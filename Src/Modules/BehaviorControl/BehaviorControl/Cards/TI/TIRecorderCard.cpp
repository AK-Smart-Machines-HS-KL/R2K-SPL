/**
 * @file TIRecorderCard.cpp 
 * @author Wilhelm Simus (R2K)
 * @brief This file is responsible for handling the key events of a gamepad and execute the corresponding actions while keeping a recording of the performed actions when needed.
 * 
 * This Card is to control the NAO manually with the PS4 Controller and to protocol the handled actions.
 * Once a button is pressed, the Nao will start its specific action until an other one is pressed.
 * 
 * The actions are saved inside a playbackAction struct defined in TIRecorderData.h 
 * This struct is provided in with the 'void protocol()' function to the KeyLogger which saves and writes into the playback file
 * 
 * It checks the pressed key and then will handle its asociated action
 * i key == X{
 *    doSkill()
 *    activity="doSkill"
 * }
 * It handles the headMotions before the walkingMotions.
 * Skills, which are setting head and body motion, should be handled first
 * 
 * 
 * @version 1.0
 * @date 2023-04-18
 * 
 *  */

#include <map>

#include "Tools/TeachIn/KeyLogger.h"

#include "Representations/BehaviorControl/Skills.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/Math/Pose3f.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/BehaviorControl/FieldBall.h"

#include "Representations/Communication/TeamInfo.h"

#include "Representations/Communication/TeamData.h"
#include "Representations/Communication/TeamInfo.h"

#include "Representations/BehaviorControl/TI/TIData.h"
#include "Representations/BehaviorControl/TI/TIRecorderData.h"
#include "Representations/Infrastructure/FrameInfo.h"

CARD(TIRecorderCard, {
  ,
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(TIExecute),
  CALLS(Stand),
  REQUIRES(RobotInfo),
  REQUIRES(RobotPose),
  REQUIRES(FrameInfo),
  REQUIRES(TeamData),
  REQUIRES(TIRecorderData),
  DEFINES_PARAMETERS({
      ,
      (float)(0.5f) cursorActivationTreshold,
      (float)(0.05f) cursorZeroTreshold,
  		(float)(0.75f) walkSpeed,
      (int)(100) cursorActionInterval,
  }),
});

// performable actions
const PlaybackAction standAction = PlaybackAction().setSkill(PlaybackAction::Skills::Stand);
const PlaybackAction goalKickAction = PlaybackAction().setSkill(PlaybackAction::Skills::KickAtGoal);
const PlaybackAction goToBallAction = PlaybackAction().setSkill(PlaybackAction::Skills::WalkToBall);
const PlaybackAction goToPointAction = PlaybackAction().setSkill(PlaybackAction::Skills::WalkToPoint);


class TIRecorderCard : public TIRecorderCardBase
{
  KeyLogger *keyLogger = KeyLogger::getInstance();
  std::string headMotionVertical;
  std::string headMotionHorizontal;
  std::map<int, std::function<void()>> specialCallbacks;
  std::map<int, PlaybackAction> actionCallbacks;
  float speed;
  bool cursorControl = false;
  uint32_t lastCursorAction = 0;
  PlaybackAction lCursorAction = PlaybackAction().setSkill(PlaybackAction::Skills::WalkAtRelativeSpeed);

  public:
  PlaybackAction currentAction = PlaybackAction();


  /* unmapped skills (s. TIExecute.cpp
  * theWalkAtRelativeSpeedSkill, (action.poseParam));
  * ? theGoToBallAndKickSkill, (0_deg, KickInfo::KickType::forwardFastRight));
  */

  TIRecorderCard() {
	  actionCallbacks[Keys::Circle] = standAction;
    actionCallbacks[Keys::Triangle] = goalKickAction;
    actionCallbacks[Keys::Square] = goToBallAction;
    actionCallbacks[Keys::Cross] = goToPointAction;  //WalkToPoint

    specialCallbacks[Keys::Share] = [&]() {if(theTIRecorderData.recording) {theTIRecorderData.stop();} else {theTIRecorderData.start();}}; // toggle recording
    specialCallbacks[Keys::L1] = [&]() {theTIRecorderData.save();}; // save
  }

  //always active
  bool preconditions() const override
  {
    if(theRobotInfo.number == keyLogger->getRobotNumber() && keyLogger->isStartPressed()) {
      OUTPUT_TEXT("TIRecording triggered for robot " << theRobotInfo.number);
      keyLogger->clearEvents();
      return true;
    }
    return false; 
  }

  bool postconditions() const override
  {
    if (!keyLogger->isStartPressed()) {
      if (theTIRecorderData.recording) {
        OUTPUT_TEXT("TIRecording halted");
        theTIRecorderData.stop();
      }
      return true;
    }
    return false;
  }

  void execute() override
  { 
    theActivitySkill(BehaviorStatus::teachin);

    speed = walkSpeed;
    //get the last pressed key
    // int key = keyLogger->getKey();
    // //get the Cursor Values

    // Key handling:
    handleController();

    // OUTPUT_TEXT("target " << theTeamData.teammates[5].theRobotPose.translation.x() << theTeamData.teammates[5].theRobotPose.translation.y());
    /*
    if(currentAction.skill == PlaybackAction::Skills::Default || 
       currentAction.skill == PlaybackAction::Skills::Stand ||
       currentAction.skill == PlaybackAction::Skills::WalkAtRelativeSpeed ||
       currentAction.skill == PlaybackAction::Skills::KickAtGoal)
       */
    if (currentAction.skill != PlaybackAction::Skills::KickAtGoal)
      theLookForwardSkill(); //TI does not currently support head motion requests.
    
    if (currentAction.skill == PlaybackAction::Skills::WalkToPoint) {

      float dest_x = theTeamData.teammates[4].theRobotPose.translation.x();
      float dest_y = theTeamData.teammates[4].theRobotPose.translation.y();
      // OUTPUT_TEXT("x " << dest_x << " " << dest_y);

      Pose2f targetAbsolute = Pose2f(calcAngleToDest(dest_x,dest_y),dest_x,dest_y);
      const Pose2f targetRelative = theRobotPose.inversePose * targetAbsolute;
      setAction(currentAction.setPose(targetRelative).setFloat(1.f));
    }
    
    theTIExecuteSkill(currentAction);
  }

  void setAction(const PlaybackAction& setAction) {
    currentAction = setAction;
    theTIRecorderData.changeAction(currentAction);
  }

  /**
   * @brief Requests input from the user 
   * 
   * @return Vector2f Position on field
   */
  Vector2f requestInput() {
    return Vector2f();
  }

  void handleController() {

    Vector2f lCursor = keyLogger->getLeftCursor();
    if (lCursor.norm() > cursorActivationTreshold) { // check if Left cursor should take over control (walking)
      if(!cursorControl) {
        OUTPUT_TEXT("Enabling cursor Control!");
      }
      cursorControl = true;
    }

    if (keyLogger->hasEvent()) {
      int e = keyLogger->nextEvent(); // only handle first key press this frame. Should be fine. Change if it's an issue

      if (specialCallbacks.count(e)) {
        specialCallbacks.at(e)();
      } else if (actionCallbacks.count(e)) {
        setAction(actionCallbacks.at(e));
        cursorControl = false;
      } else {
        OUTPUT_ERROR("Key " << e << " not bound");
      }
      return;
    }    

    // If No events
    if(cursorControl) {
      
      if (theFrameInfo.getTimeSince(lastCursorAction) > cursorActionInterval) { // so we don't write actions every frame
        lastCursorAction = theFrameInfo.time;
        if (lCursor.norm() < cursorZeroTreshold) { // if values are small enough, interpret as zero and stand
          setAction(standAction);
        } else {
          setAction(lCursorAction.setPose(Pose2f(lCursor.x(), lCursor.y(), 0)));
        }
      }
    }
  }


  //  Pose2f targetAbsolute = Pose2f(theRobotPose.inversePose * Vector2f(theTeamData.teammates[5].theRobotPose.translation.x(), theTeamData.teammates[5].theRobotPose.translation.y())).angle();
  Angle calcAngleToDest(float x, float y) const
  {
    // return (theRobotPose.inversePose * Vector2f(theTeamData.teammates[4].theRobotPose.translation.x(), theTeamData.teammates[4].theRobotPose.translation.y())).angle();
    return  (theRobotPose.inversePose * Vector2f(x, y)).angle();
  }
};

MAKE_CARD(TIRecorderCard);