#include "ObstacleModel.h"
#include "Platform/SystemCall.h"
#include "Representations/Communication/TeamInfo.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Debugging/DebugDrawings3D.h"
#include "Tools/Math/Approx.h"
#include "Tools/Modeling/Obstacle.h"
#include "Tools/Module/Blackboard.h"

void ObstacleModel::operator>>(BHumanMessage& m) const
{
  std::sort(const_cast<std::vector<Obstacle>&>(obstacles).begin(), const_cast<std::vector<Obstacle>&>(obstacles).end(), [](const Obstacle& a, const Obstacle& b) {return a.center.squaredNorm() < b.center.squaredNorm(); });

  Streaming::streamIt(*m.theBHumanStandardMessage.out, "theObstacleModel",  *this);
}

void ObstacleModel::operator<<(const BHumanMessage& m)
{
  Streaming::streamIt(*m.theBHumanStandardMessage.in, "theObstacleModel",  *this);
}

// need to clarify: opponent detection
bool ObstacleModel::opponentIsClose(float min) const{
  for (const Obstacle& ob : obstacles)
  {
    if (ob.isOpponent()) // || ob.isTeammate())
      if (ob.center.norm() <= min) return true;
  }
  return false;
}

//returns an int value depending on how far the ball is away from the opponent
  KickInfo::LongShotType ObstacleModel::opponentIsTooClose(Vector2f ballPosRelative, float min, float preciseMin) const {
    KickInfo::LongShotType lowestProximityIndex = KickInfo::LongShotType::precise;
    float distanceToBall = ballPosRelative.norm();

    for (const Obstacle& ob : obstacles)
    {

      // need to clarify: opponent detection

      if (ob.isOpponent()) { //tbd: check for teammate, add sector wheel)
        float distanceOpponentToBall = (ob.center - ballPosRelative).norm();



        //       (float)(1500) minOppDistance,
        //       (float)(2500) preciseKickDistance,
        //is any opponent closer to ball than me or is too close to ball
        if (distanceToBall >= distanceOpponentToBall || distanceOpponentToBall <= min) {
          return KickInfo::LongShotType::noKick;
        }

        if (distanceOpponentToBall <= preciseMin) { //close enemy found
          lowestProximityIndex = KickInfo::LongShotType::fast;
        }
      }
    }
    return lowestProximityIndex;
  }

void ObstacleModel::verify() const
{
  DECLARE_DEBUG_RESPONSE("representation:ObstacleModel:verify");
  for(const auto& obstacle : obstacles)
  {
    ASSERT(std::isfinite(obstacle.center.x()));
    ASSERT(std::isfinite(obstacle.center.y()));

    ASSERT(std::isfinite(obstacle.left.x()));
    ASSERT(std::isfinite(obstacle.left.y()));

    ASSERT(std::isfinite(obstacle.right.x()));
    ASSERT(std::isfinite(obstacle.right.y()));

    ASSERT(std::isfinite(obstacle.velocity.x()));
    ASSERT(std::isfinite(obstacle.velocity.y()));

    DEBUG_RESPONSE("representation:ObstacleModel:verify")
      if((obstacle.left - obstacle.right).squaredNorm() < sqr(2000.f))
        OUTPUT_WARNING("Obstacle too big!");

    ASSERT(std::isnormal(obstacle.covariance(0, 0)));
    ASSERT(std::isnormal(obstacle.covariance(1, 1)));
    ASSERT(std::isfinite(obstacle.covariance(0, 1)));
    ASSERT(std::isfinite(obstacle.covariance(1, 0)));
    ASSERT(Approx::isEqual(obstacle.covariance(0, 1), obstacle.covariance(1, 0), 1e-20f));
  }
}

void ObstacleModel::draw() const
{
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:rectangle", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:centerCross", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:leftRight", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:circle", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:covariance", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:velocity", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:fallen", "drawingOnField");
  DECLARE_DEBUG_DRAWING3D("representation:ObstacleModel", "robot");

  const ColorRGBA ownColor = ColorRGBA::fromTeamColor(Blackboard::getInstance().exists("OwnTeamInfo") ?
      static_cast<const OwnTeamInfo&>(Blackboard::getInstance()["OwnTeamInfo"]).fieldPlayerColour : TEAM_RED);

  const ColorRGBA opponentColor = ColorRGBA::fromTeamColor(Blackboard::getInstance().exists("OpponentTeamInfo") ?
      static_cast<const OpponentTeamInfo&>(Blackboard::getInstance()["OpponentTeamInfo"]).fieldPlayerColour : TEAM_BLACK);

  ColorRGBA color;
  for(const auto& obstacle : obstacles)
  {
    switch(obstacle.type)
    {
      case Obstacle::goalpost:
      {
        color = ColorRGBA::white;
        break;
      }
      case Obstacle::fallenTeammate:
      case Obstacle::teammate:
      {
        color = ownColor;
        break;
      }
      case Obstacle::fallenOpponent:
      case Obstacle::opponent:
      {
        color = opponentColor;
        break;
      }
      case Obstacle::fallenSomeRobot:
      case Obstacle::someRobot:
      {
        color = ColorRGBA(200, 200, 200); // gray
        break;
      }
      default:
      {
        color = ColorRGBA::violet;
        break;
      }
    }
    const Vector2f& center = obstacle.center;
    const Vector2f& left = obstacle.left;
    const Vector2f& right = obstacle.right;

    CYLINDER3D("representation:ObstacleModel", center.x(), center.y(), -210, 0, 0, 0, (left - right).norm(), 10, color);
    CROSS("representation:ObstacleModel:centerCross", center.x(), center.y(), Obstacle::getRobotDepth(), 10, Drawings::solidPen, color);

    float obstacleRadius = (left - right).norm() * .5f;
    Angle robotRotation = Blackboard::getInstance().exists("RobotPose") ? static_cast<const RobotPose&>(Blackboard::getInstance()["RobotPose"]).rotation : Angle();
    Vector2f frontRight(-Obstacle::getRobotDepth(), -obstacleRadius);
    frontRight = center + frontRight;
    RECTANGLE2("representation:ObstacleModel:rectangle", frontRight, obstacleRadius * 2, obstacleRadius * 2, -robotRotation, 16, Drawings::PenStyle::solidPen, ColorRGBA::black, Drawings::solidBrush, color);

    LINE("representation:ObstacleModel:leftRight", center.x(), center.y(), left.x(), left.y(), 20, Drawings::dottedPen, color);
    LINE("representation:ObstacleModel:leftRight", center.x(), center.y(), right.x(), right.y(), 20, Drawings::dottedPen, color);
    CIRCLE("representation:ObstacleModel:circle", center.x(), center.y(), obstacleRadius, 10, Drawings::dottedPen, color, Drawings::noBrush, color);
    COVARIANCE_ELLIPSES_2D("representation:ObstacleModel:covariance", obstacle.covariance, center);

    if(obstacle.velocity.squaredNorm() > 0)
      ARROW("representation:ObstacleModel:velocity", center.x(), center.y(),
            center.x() + 2 * obstacle.velocity.x(), center.y() + 2 * obstacle.velocity.y(), 10, Drawings::solidPen, ColorRGBA::black);

    if(obstacle.type >= Obstacle::fallenSomeRobot)
      DRAW_TEXT("representation:ObstacleModel:fallen", center.x(), center.y(), 100, color, "FALLEN");
  }
}
