/**
 * @file TIExecute.cpp
 * @author Andy Hobelsberger
 * @brief This Skill executes  
 * @version 1.0
 * @date 2022-01-01
 * 
 * This Skill Executes Supported Subskills for the TI System.
 */

#include "Representations/BehaviorControl/Skills.h"
#include "Representations/BehaviorControl/TI/TIData.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Modeling/RobotPose.h"
#include "Tools/Streams/TypeRegistry.h"
#include <functional>
#include <cmath>

struct SkillMapping {
  bool mapped = false;
  std::function<void(const PlaybackAction& action)> call;
  std::function<bool()> isDone;
  std::function<bool()> isAborted;
};

/**
 * @brief Default Mapping macro for Enum -> skill calls. This must always be called first
 * Maps a skill call with args and maps through isDone and isAborted. Customize isDone and isAborted with the MAP_DONE and MAP_ABORT macros
 */
#define MAP(_ENUM, _SKILL, _ARGS) MAP_EXPLICIT(_ENUM, _SKILL, {_SKILL _ARGS;})

#define MAP_EXPLICIT(_ENUM, _SKILL, _CALL) \
  mappings[_ENUM].call = [&](const PlaybackAction& action)->void _CALL; \
  mappings[_ENUM].isDone = [&]()->bool {return _SKILL.isDone();}; \
  mappings[_ENUM].isAborted = [&]()->bool {return _SKILL.isAborted();}; \
  mappings[_ENUM].mapped = true

#define MAP_DONE(_ENUM, _CALL) \
  mappings[_ENUM].isDone = [&]()->bool _CALL

#define MAP_ABORT(_ENUM, _CALL) \
  mappings[_ENUM].isAborted = [&]()->bool _CALL

SKILL_IMPLEMENTATION(TIExecuteImpl,
{,
  IMPLEMENTS(TIExecute),
  REQUIRES(FrameInfo),
  REQUIRES(RobotPose),
  CALLS(Stand),
  CALLS(WalkAtRelativeSpeed),
  CALLS(GoToBallAndKick),
  CALLS(WalkToPoint),
  CALLS(WalkToBall),
  CALLS(GoToBallAndDribble),
});

class TIExecuteImpl : public TIExecuteImplBase
{
  public:
	std::vector<SkillMapping> mappings;

  TIExecuteImpl() {
		mappings.resize(PlaybackAction::numOfSkillss);
    
    // Mappings for Skills defined in TIData.h
    MAP_EXPLICIT(PlaybackAction::Skills::Default, theStandSkill, {theStandSkill();});
    MAP_DONE(PlaybackAction::Skills::Default, { return false; });  // continuous: use maxTime only

    MAP_EXPLICIT(PlaybackAction::Skills::Stand, theStandSkill, {theStandSkill();});
    MAP_DONE(PlaybackAction::Skills::Stand, { return false; });  // continuous: use maxTime only

    MAP(PlaybackAction::Skills::WalkAtRelativeSpeed, theWalkAtRelativeSpeedSkill, (action.poseParam));
    MAP_DONE(PlaybackAction::Skills::WalkAtRelativeSpeed, { return false; });  // continuous: use maxTime only

    MAP_EXPLICIT(PlaybackAction::Skills::KickAtGoal, theGoToBallAndKickSkill, {
      // intParam > 0: explicit KickInfo::KickType enum value (see KickInfo.h for values):
      //   0 = fallback to boolParam foot selection (forwardFastRight or forwardFastLeft)
      //   1 = forwardFastLeft        (-185 mm standoff, Default cfg)
      //   2 = forwardFastRightLong   (-190 mm standoff)
      //   3 = forwardFastLeftLong    (-190 mm standoff — more space, recommended for corners)
      //   5 = walkForwardsLeft       (-210 mm standoff — maximum standoff)
      // boolParam (fallback when intParam=0): true = forwardFastLeft, false = forwardFastRight
      KickInfo::KickType kickType;
      if(action.intParam > 0 && action.intParam < KickInfo::numOfKickTypes)
        kickType = static_cast<KickInfo::KickType>(action.intParam);
      else
        kickType = action.boolParam ? KickInfo::forwardFastLeft : KickInfo::forwardFastRight;
      // poseParam encodes the field-absolute kick target (x, y in mm).
      // Transform it into robot-relative bearing — same pattern as calcAngleToOppPenaltyMark().
      // This ensures the kick direction is always correct regardless of the robot's heading.
      const Angle targetAngle = (theRobotPose.inversePose * action.poseParam.translation).angle();
      // alignPrecisely=true: robot carefully positions itself before kicking — prevents
      // accidentally nudging the ball during approach (critical for corner/boundary kicks)
      theGoToBallAndKickSkill(targetAngle, kickType, true);
    });
    // KickAtGoal: isDone() from GoToBallAndKick is reliable (true when kick is performed)

    MAP_EXPLICIT(PlaybackAction::Skills::WalkToBall, theWalkToBallSkill, {theWalkToBallSkill();});
    // WalkToBall: isDone() is reliable (CABSL target_state when ball is reached)

    MAP_EXPLICIT(PlaybackAction::Skills::WalkToPoint, theWalkToPointSkill, {
      // poseParam stores field-absolute coordinates; WalkToPoint expects robot-relative.
      const Pose2f relTarget = theRobotPose.inversePose * action.poseParam;
      theWalkToPointSkill(relTarget, action.floatParam > 0.f ? action.floatParam : 1.f, true, false, false, true);
    });
    // WalkToPoint: isDone() is reliable (done when destination reached)

    MAP(PlaybackAction::Skills::Dribble, theGoToBallAndDribbleSkill,
        (action.angleParam1, false, action.floatParam > 0.f ? action.floatParam : 1.f));
    MAP_DONE(PlaybackAction::Skills::Dribble, { return false; });  // continuous: use maxTime only
   // MAP_DONE(PlaybackAction::Skills::GoToTarget, { return theWalkToTargetSkill.isDone(); });
    //MAP_ABORT(PlaybackAction::Skills::GoToTarget, { return theWalkToTargetSkill.isAborted(); });

   // MAP(PlaybackAction::Skills::WalkToTarget, theWalkToTargetSkill, (Pose2f(180_deg, 1000.0f, 1000.0f), action.poseParam));
   // MAP_DONE(PlaybackAction::Skills::WalkToTarget, {return false;});

    for(size_t i = 0; i < mappings.size(); i++) {
      //ASSERT(mappings[i].mapped); // If this Trips, A function of the Enum PlaybackAction::Skills has no mapping. Add it above
      
      if (!mappings[i].mapped) {
        mappings[i] = mappings[PlaybackAction::Default];
        OUTPUT_TEXT("Warning: TI Skill `" << std::string(TypeRegistry::getEnumName(typeid(PlaybackAction::Skills).name(), int(i))) << "` is not mapped. It has been remapped to Default");
      }
    }  
  }

  void execute(const TIExecute& p) override{
    mappings[p.pAction.skill].call(p.pAction);
  }

  bool isDone(const TIExecute& p) const override{
    return mappings[p.pAction.skill].isDone();
  }

  bool isAborted(const TIExecute& p) const override{
    return mappings[p.pAction.skill].isAborted();
  }
};

MAKE_SKILL_IMPLEMENTATION(TIExecuteImpl);
