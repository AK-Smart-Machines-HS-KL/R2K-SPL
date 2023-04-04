/**
 * @file TIPlayback.cpp
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
#include "Representations/BehaviorControl/TI/TIPlaybackData.h"
#include "Tools/BehaviorControl/Framework/Skill/CabslSkill.h"

SKILL_IMPLEMENTATION(TIPlaybackImpl,
                     {
                         ,
                         IMPLEMENTS(TIPlayback),
                         CALLS(TIExecute),
                         CALLS(Stand),
                         REQUIRES(TIPlaybackSequences),
                         REQUIRES(FrameInfo),
                     });

class TIPlaybackImpl : public TIPlaybackImplBase
{
    int lastSeqIdx = 0;
    bool _reset = false;
    unsigned int actStartTime = 0;
    std::vector<PlaybackAction>::const_iterator actIter;

    option(TIPlayback)
    {         
        _reset = lastSeqIdx != p.idx || p.reset;   
        lastSeqIdx = p.idx;

        initial_state(init)
        {
            OUTPUT_TEXT(theTIPlaybackSequences.data[p.idx].fileName);
            actStartTime = theFrameInfo.time;
            actIter = theTIPlaybackSequences.data[p.idx].actions.begin();
            transition
            {
                goto exec;
            }
        }

        state(exec)
        {
            if (isActionDone())
            {
                actIter++;
                actStartTime = theFrameInfo.time;
            }
            
            transition
            {
                if (_reset)
                {
                    goto init;
                } 
                else if (actIter == theTIPlaybackSequences.data[p.idx].actions.end())
                {
                    goto done;
                }
                else if (theTIExecuteSkill.isAborted())
                {
                    goto error;
                }
            }
            action
            {
                theTIExecuteSkill(*actIter);
            }
        }

        target_state(done)
        {
            transition
            {
                if (_reset) {
                    goto init;
                }
            }
            action
            {
                theStandSkill();
            }
        }

        aborted_state(error)
        {
            action
            {
                theStandSkill();
            }
        }
    }

    bool isActionDone() {
        return theFrameInfo.getTimeSince(actStartTime) >= actIter->maxTime
           // || (theTIExecuteSkill.isDone() && theFrameInfo.getTimeSince(actStartTime) != 0)
        ;
    }
};

MAKE_SKILL_IMPLEMENTATION(TIPlaybackImpl);
