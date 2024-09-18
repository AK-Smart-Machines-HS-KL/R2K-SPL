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
#include "Tools/Streams/TypeRegistry.h"
#include <functional>

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
  CALLS(Stand),
  CALLS(WalkAtRelativeSpeed),
  CALLS(GoToBallAndKick),
  CALLS(WalkToPoint),
 
});

class TIExecuteImpl : public TIExecuteImplBase
{
  public:
	std::vector<SkillMapping> mappings;

  TIExecuteImpl() {
		mappings.resize(PlaybackAction::numOfSkillss);
    
    // Mappings for Skills defined in TIData.h
    MAP_EXPLICIT(PlaybackAction::Skills::Default, theStandSkill, {theStandSkill();});
    MAP_EXPLICIT(PlaybackAction::Skills::Stand, theStandSkill, {theStandSkill();});
    MAP(PlaybackAction::Skills::WalkAtRelativeSpeed, theWalkAtRelativeSpeedSkill, (action.poseParam));
    MAP(PlaybackAction::Skills::KickAtGoal, theGoToBallAndKickSkill, (0_deg, KickInfo::KickType::forwardFastRight));
 
    MAP(PlaybackAction::Skills::WalkToBall, theWalkAtRelativeSpeedSkill, (action.poseParam));
    // theWalkToPointSkill(Pose2f(0.f, 0.f, theFieldBall.intersectionPositionWithOwnYAxis.y()), 1.f, /* rough: */ false, /* disableObstacleAvoidance: */ false, /* disableAligning: */ true);
    MAP(PlaybackAction::Skills::WalkToPoint, theWalkToPointSkill, (action.poseParam, action.floatParam, true, false, false, true));
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
