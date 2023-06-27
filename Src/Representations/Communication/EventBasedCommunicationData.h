/**
 * @file EventBasedCommunicationData.h
 * @author Connor Lismore
 * @brief Representation for the Event Based Communication. Responsible for ensuring messages are send by checking for events and other changes. Reduces the amount of messages put out.
 * @version 0.1
 * @date 2022-03-08
 * 
 * Changelog: Initial Upload, added streamable functions and parameters for use between EBC and messaging
 * 
 * 
 */

#pragma once

#include "Tools/Streams/AutoStreamable.h"
#include "Tools/Function.h"


STREAMABLE(EventBasedCommunicationData,
{ 
  FUNCTION(bool()) sendThisFrame;             //custom sendThisFrame, used when ebc is enabled
  FUNCTION(int()) ebcSendMessageImportant;  // tmp return int = ebc #writes
  FUNCTION(void()) ebcMessageMonitor;
  , 
});
