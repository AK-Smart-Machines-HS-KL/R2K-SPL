/**
 * @file Representations/BehaviorControl/BehaviorStatus.h
 * The file declares a struct that contains data about the current behavior state.
 * @author Andreas Stolpmann
 */

#pragma once

#include "Tools/Communication/BHumanTeamMessageParts/BHumanMessageParticle.h"
#include "Tools/Math/Eigen.h"
#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Streams/Enum.h"

/**
 * @struct BehaviorStatus
 * A struct that contains data about the current behavior state.
 */
STREAMABLE(BehaviorStatus, COMMA public BHumanCompressedMessageParticle<BehaviorStatus>
{
  ENUM(Activity,
  {,
    unknown,

    afterInterceptBall,
    fallen,
    finished,
    initial,
    lookAroundAfterPenalty,
    set,

    testingBehavior,
    goalShot,
    defaultBehavior,
    codeReleaseKickAtGoal,
    codeReleasePositionForKickOff,
    
    // R2K stuff
    searchingForBall,
    blocking,
    defenseChaseBallCard,      
    offenseChaseBallCard,      
    clearOwnHalfCard,  
    clearOwnHalfCardGoalie,
    defenseCard,        
    defenseLongShotCard,
    dive,
    goalShotCard,
    goalieLongShotCard, 
    offenseFastGoalKick,
    offenseForwardPassCard,
    offenseReceivePassCard,
    
    //Game States
    ownFreeKick,
    ownPenaltyKick,
    oppFreeKick,

    ownKickoff,
    oppKickoff,

    // end R2K stuff

    calibrationFinished,

    referenceCard,      // R2K
    searchForBall,  // R2K
  });

  /**
   * Checks whether the activity indicates that the robot is searching for the ball.
   * @return Whether the activity indicates that the robot is searching for the ball.
   */
  bool searchesForBall() const
  {
    return false;
  },

  (Activity)(unknown) activity, /**< What is the robot doing in general? */
  (int)(-1) passTarget,
  (Vector2f)(Vector2f::Zero()) walkingTo,
  (float) speed, /**< The absolute speed in mm/s. */
  (Vector2f)(Vector2f::Zero()) shootingTo,
});
