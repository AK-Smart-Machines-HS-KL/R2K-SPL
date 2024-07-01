/**
 * @file TeamCommBufferManager.h
 * @author Andy Hobelsberger
 */

#include "TeamCommBufferManager.h"

MAKE_MODULE(TeamCommBufferManager, communication); 

void TeamCommBufferManager::update(TeamCommBuffer& buffer) { 
    buffer.pose = theRobotPose;
    buffer.model = theBallModel;
}
