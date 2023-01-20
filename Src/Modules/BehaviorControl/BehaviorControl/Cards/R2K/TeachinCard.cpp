/**
 * @file TeachinCard.cpp
 * @author Felix Mayer (R2K)
 * @brief This file is responsible for handling the key events of a gamepad and execute the corresponding actions.
 *
 * This Card is to control the NAO manuel with the PS4 Controller and to protocol the handled actions.
 * Once a button got pressed, the Nao will start it's specific action until an other one got pressed.
 *
 * The actions are saved inside a playbackAction struct defined in TIData.h
 * This struct is provided in with the 'void protocol()' function to the keyLogger which saves and writes the playback File
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
 * @version 1.1
 * @date 2021-10-05
 *
 *  */

#include <map>

#include "Tools/KeyLogger/KeyLogger.h"
#include "Representations/BehaviorControl/Skills.h"
#include "Tools/BehaviorControl/Framework/Card/Card.h"
#include "Tools/BehaviorControl/Framework/Card/CabslCard.h"
#include "Tools/Math/Pose3f.h"
#include "Representations/Communication/RobotInfo.h"
#include "Representations/BehaviorControl/FieldBall.h"

#include "Representations/BehaviorControl/TIData.h"
#include "Representations/BehaviorControl/TIRecorder.h"
#include "Representations/Infrastructure/FrameInfo.h"

CARD(TeachinCard, {
  ,
  CALLS(Activity),
  CALLS(LookForward),
  CALLS(TIExecute),
  CALLS(Stand),
  REQUIRES(RobotInfo),
  REQUIRES(FrameInfo),
  REQUIRES(TIRecorder),
  DEFINES_PARAMETERS({
      ,
      (float)(0.5f) cursorActivationTreshold,
      (float)(0.05f) cursorZeroTreshold,
        (float)(0.75f) walkSpeed,
      (int)(100) cursorActionInterval,
  }),
    });

const playbackAction standAction = playbackAction().setSkill(playbackAction::Skills::Stand);

class TeachinCard : public TeachinCardBase
{
    KeyLogger* keyLogger = KeyLogger::getInstance();
    std::string headMotionVertical;
    std::string headMotionHorizontal;
    std::map<int, std::function<void()>> specialCallbacks;
    std::map<int, playbackAction> actionCallbacks;
    float speed;
    bool cursorControl = false;
    uint32_t lastCursorAction = 0;
    playbackAction lCursorAction = playbackAction().setSkill(playbackAction::Skills::WalkAtRelativeSpeed);

public:
    playbackAction currentAction = playbackAction();

    TeachinCard() {

        actionCallbacks[Keys::Circle] = standAction;
        actionCallbacks[Keys::Triangle] = playbackAction().setSkill(playbackAction::Skills::GoToBallAndKick);
        actionCallbacks[Keys::Square] = playbackAction().setSkill(playbackAction::Skills::WalkToPoint);
        actionCallbacks[Keys::Cross] = playbackAction().setSkill(playbackAction::Skills::GoToBallAndDribble);

        specialCallbacks[Keys::Share] = [&]() {if (theTIRecorder.recording) { theTIRecorder.stop(); } else { theTIRecorder.start(); }}; // toggle recording
        specialCallbacks[Keys::L1] = [&]() {theTIRecorder.save(); }; // save
    }

    //always active
    bool preconditions() const override
    {
        if (theRobotInfo.number == keyLogger->getRobotNumber() && keyLogger->isStartPressed()) {
            keyLogger->clearEvents();
            return true;
        }
        return false;
    }

    bool postconditions() const override
    {
        if (!keyLogger->isStartPressed()) {
            if (theTIRecorder.recording) {
                theTIRecorder.stop();
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

        theLookForwardSkill(); //TI does not currently support head motion requests.
        theTIExecuteSkill(currentAction);
    }

    void setAction(const playbackAction& setAction) {
        currentAction = setAction;
        theTIRecorder.changeAction(currentAction);
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
            if (!cursorControl) {
                OUTPUT_TEXT("Enabling cursor Control!");
            }
            cursorControl = true;
        }

        if (keyLogger->hasEvent()) {
            int e = keyLogger->nextEvent(); // only handle first key press this frame. Should be fine. Change if it's an issue

            if (specialCallbacks.count(e)) {
                specialCallbacks.at(e)();
            }
            else if (actionCallbacks.count(e)) {
                setAction(actionCallbacks.at(e));
                cursorControl = false;
            }
            else {
                OUTPUT_TEXT("Key " << e << " not bound");
            }
            return;
        }

        // If No events
        if (cursorControl) {

            if (theFrameInfo.getTimeSince(lastCursorAction) > cursorActionInterval) { // so we don't write actions every frame
                lastCursorAction = theFrameInfo.time;
                if (lCursor.norm() < cursorZeroTreshold) { // if values are small enough, interpret as zero and stand
                    setAction(standAction);
                }
                else {
                    setAction(lCursorAction.setPose(Pose2f(lCursor.x(), lCursor.y(), 0)));
                }
            }
        }
    }
};

MAKE_CARD(TeachinCard);