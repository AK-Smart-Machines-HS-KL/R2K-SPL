#include "ShotPredictor.h"
#include "Tools/Math/Geometry.h"
#include "Tools/Math/Probabilistics.h"
#include "Tools/Framework/Configuration.h"

#define drawID "module:ShotPredictor"

MAKE_MODULE(ShotPredictor, behaviorControl);

ShotPredictor::ShotPredictor() {
    goalLeft = Vector2f(theFieldDimensions.xPosOpponentGoalPost, theFieldDimensions.yPosLeftGoal);
    goalRight = Vector2f(theFieldDimensions.xPosOpponentGoalPost, theFieldDimensions.yPosRightGoal);
}

void ShotPredictor::update(Shots& shotData) {
    DECLARE_DEBUG_DRAWING(drawID, "drawingOnField"); // Shot Predictor Debug drawing

    Vector2f goalLeftRelative = theRobotPose.toRelative(goalLeft);
    Vector2f goalRightRelative = theRobotPose.toRelative(goalRight);
    LINE(drawID, goalLeftRelative.x(), goalLeftRelative.y(), goalRightRelative.x(), goalRightRelative.y(), 50, Drawings::solidPen, ColorRGBA::green);

    shotData.goalShot = getBestThruShot(goalLeft, goalRight);
};

float ShotPredictor::estimateTimeToKick(Shot& shot) {
    Angle angleToTarget = (theRobotPose.toRelative(shot.target) - theFieldBall.positionRelative).angle();
    return estimateTimeToAlign(angleToTarget, shot) + shot.kickType.duration;
}

float ShotPredictor::estimateTimeToAlign(Angle& direction, Shot& shot) {
    Angle angleDiff = -direction;
    Vector2f posDiff = -(theFieldBall.positionRelative - shot.kickType.offset.rotated(angleDiff));

    float t = 
         (abs(angleDiff / maxSpeed.rotation)) // Time To Rotate to target direction, in seconds
        + (abs(posDiff.norm() / maxSpeed.translation.x())); // Time to Move to ball, in seconds
    return t;
}

SectorList ShotPredictor::getMissSectors(Shot& shot, Vector2f& targetLineStartRelative, Vector2f& targetLineEndRelative) {
  SectorList miss = SectorList();
  Vector2f lineStartBallRelative = targetLineStartRelative - theFieldBall.positionRelative;
  Vector2f lineEndBallRelative = targetLineEndRelative - theFieldBall.positionRelative;
  Sector goal = Sector(lineStartBallRelative.angle(), lineEndBallRelative.angle(), (lineStartBallRelative + lineEndBallRelative).angle());
  goal.invert();
  miss.addSector(goal);
  return miss;
}

SectorList ShotPredictor::getInsterceptSectors(Shot& shot){
  auto prediction = predictObstacles(shot.executionTime);
  SectorList blocked;
  for(auto obstacle : prediction)
  {
      Vector2f posBallRelative = obstacle.pos - theFieldBall.positionRelative;
      Vector2f tp1, tp2;
      if(Geometry::getTangentPoints(Vector2f::Zero(), Geometry::Circle(posBallRelative, obstacle.range), tp1, tp2)) {
          blocked.addSector(Sector(tp1.angle(), tp2.angle(), posBallRelative.angle()));   
      } 
      else 
      {
          blocked.addSector(Sector(-pi, pi - 0.00001f));
          break;
      }

      tp1 += theFieldBall.positionRelative;
      tp2 += theFieldBall.positionRelative;
      LINE(drawID, tp1.x(), tp1.y(), tp2.x(), tp2.y(), 10, Drawings::dottedPen, ColorRGBA::blue);
  }
  return blocked;
}

void ShotPredictor::calcShotFailProbability(Shot& shot, Vector2f& targetLineStartRelative, Vector2f& targetLineEndRelative){
    shot.failureProbability = 0;

    if ((theFieldBall.positionOnField - shot.target).norm() > shot.kickType.range) {
        shot.failureProbability = 1;
        return;
    }

    SectorList shotFailureSectors; // Must be BALL Relative
    shotFailureSectors.join(getMissSectors(shot, targetLineStartRelative, targetLineEndRelative));
    shotFailureSectors.join(getInsterceptSectors(shot));
    // NOTE: There is a bug in the addSectorLogic!!!!!

    Angle targetDirection = (theRobotPose.toRelative(shot.target) - theFieldBall.positionRelative).angle(); 
    for(auto sector : shotFailureSectors)
    {
        DRAW_SECTOR(drawID, sector, Pose2f(theFieldBall.positionRelative));

        // Note about probability of intervals on circles 
        // Mathematically you need to do an infinite sum over the intervals offset by 2 pi repeatedly in both directions
        // But for smaller SD it's enough to do 3 intervals: the base interval and +/- 2 pi
        // Imagine unrolling the Sectors onto a flat number line, since that's where probabilities operate.
      
        // Convert to float to allow for negative Values required for proper intervals
        float angleMax = sector.max.normalize();
        float angleMin = sector.min.normalize();
        if(angleMax <= angleMin) { // if sector overlaps the 0 Angle, convert min to negative value
            angleMin -= pi2; 
        } 
        // ASSERT(angleMin < angleMax); // Fails due to Compiler optimization where angles are NaN Values

        shot.failureProbability += probabilityOfInterval(targetDirection, shot.kickType.angleAccSD, angleMin, angleMax);
        shot.failureProbability += probabilityOfInterval(targetDirection, shot.kickType.angleAccSD, angleMin + pi2, angleMax + pi2);
        shot.failureProbability += probabilityOfInterval(targetDirection, shot.kickType.angleAccSD, angleMin - pi2, angleMax - pi2);
    }
}

std::vector<ObstaclePrediction> ShotPredictor::predictObstacles(float time) {
    std::vector<ObstaclePrediction> prediction;
    for(auto o : theObstacleModel.obstacles)
    {
        if(o.isOpponent()) { // Obstacle is Opponent
            Vector2f toBall = (theFieldBall.positionRelative - o.center);
            float distTravelled = maxSpeed.translation.x() * opponentSpeedModifier * time;
            
            // Enemy will reach the ball before me!
            if(distTravelled > toBall.norm()) {
                prediction.clear();
                ObstaclePrediction p = {theFieldBall.positionRelative, (o.left - o.right).norm() / 2 + ballRadius};
                prediction.push_back(p);
                break;
            }

            Vector2f posRelative = o.center + toBall.normalized() * distTravelled;
            ObstaclePrediction p = {posRelative, (o.left - o.right).norm() / 2 + ballRadius};
            prediction.push_back(p);
            CIRCLE(drawID, posRelative.x(), posRelative.y(), p.range, 10, Drawings::dottedPen, ColorRGBA::red, Drawings::noBrush, ColorRGBA::blue);
            
        } else if (!o.isTeammate()) // Obstacle is random object
        {
            ObstaclePrediction p = {o.center, (o.left - o.right).norm() / 2 + ballRadius}; 
            prediction.push_back(p);
            CIRCLE(drawID, o.center.x(), o.center.y(), p.range, 10, Drawings::dottedPen, ColorRGBA::red, Drawings::noBrush, ColorRGBA::blue);
        } 
    }
    return prediction;
}

Shot ShotPredictor::getBestThruShot(const Vector2f& lineStart, const Vector2f& lineEnd, const size_t scanFidelity) {
    Shot best;
    best.failureProbability = 10;
    Shot current;
    Vector2f dir = (lineEnd - lineStart) / (float) scanFidelity;

    current.power = 1;
    
    for (size_t i = 0; i <= scanFidelity; i++)
    {
        current.target = lineStart + dir * i;

        current.executionTime = estimateTimeToKick(current);
        Vector2f startRelative = theRobotPose.toRelative(lineStart);
        Vector2f endRelative = theRobotPose.toRelative(lineEnd);

        for (auto &kickType : kicks)
        {
            current.kickType = kickType;
            calcShotFailProbability(current, startRelative, endRelative);

            if (current.failureProbability < best.failureProbability) 
            {
                best = current;
            }
            else if (current.failureProbability == best.failureProbability) {
                if ((best.target - theFieldBall.positionOnField).norm() > (current.target - theFieldBall.positionOnField).norm())
                {
                    best = current;
                }            
            }
        }
        
        // Draw Considered Lines
        LINE(drawID, theFieldBall.positionRelative.x(), theFieldBall.positionRelative.y(), theRobotPose.toRelative(current.target).x(), theRobotPose.toRelative(current.target).y(), 5, Drawings::solidPen, ColorRGBA((char)(current.failureProbability * 255), (char)((1 - current.failureProbability) * 255), 0));
        DRAW_TEXT(drawID, theRobotPose.toRelative(current.target).x(), theRobotPose.toRelative(current.target).y(), 40, ColorRGBA::black, current.failureProbability);
    }

    // Draw best
    CIRCLE(drawID, theRobotPose.toRelative(best.target).x(), theRobotPose.toRelative(best.target).y(), 50, 10, Drawings::dottedPen, ColorRGBA::black, Drawings::noBrush, ColorRGBA::blue);
    return best;

}