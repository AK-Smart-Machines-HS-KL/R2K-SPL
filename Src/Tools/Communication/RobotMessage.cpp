#include "RobotMessage.h"
#include <bitpacker.hpp>
#include <vector>
#include <algorithm>

bool componentListFinalized = false; // If the list of possible components has been finalized. 

// Finalize the list of possible components. This should be called once the first time the list is used. 
void finalizeComponentList() {
  
}

size_t RobotMessage::compress(std::array<uint8_t, SPL_MAX_MESSAGE_BYTES> buff)
{
    size_t bitIdx = 0;
    bitpacker::insert(buff, bitIdx, bitIdx += (sizeof(componentHash) * 8), componentHash);


    for (auto component : componentPointers) {
      // make component buffer
      // compress component into buffer
      // copy buffer data into message buffer
      // set component bit in bitfield
      // size_t newBits = component->compress();
    }
    return bitIdx;
}

bool RobotMessage::decompress(std::array<uint8_t, SPL_MAX_MESSAGE_BYTES> buff){
  size_t bitIdx = 0;
  componentHash = bitpacker::extract<uint32_t>(buff, bitIdx, bitIdx += 32);
  componentsIncluded = 0;
  componentsIncluded = bitpacker::extract<uint64_t>(buff, 32, 96);
    return false;
}

template<typename T> void RobotMessageComponent<T>::addCallback(void (*foo) (T*)) {
  callbacks.insert(foo);
}

template<typename T> void RobotMessageComponent<T>::removeCallback(void (*foo) (T*)) {
  callbacks.erase(foo);
}

template<typename T> void RobotMessageComponent<T>::doCallbacks() {
  for (void (*callbackFunc)(T *) : callbacks)
  {
    callbackFunc(static_cast<T *>(this));
  }
}

template<typename T> void RobotMessageComponent<T>::addDataCompiler(void (*foo) (T*)) {
  dataCompilers.insert(foo);
}

template<typename T> void RobotMessageComponent<T>::removeDataCompiler(void (*foo) (T*)) {
  dataCompilers.erase(foo);
}

template<typename T> void RobotMessageComponent<T>::compileData() {
  for (void (*dataCompiler)(T *) : dataCompilers)
  {
    dataCompiler(static_cast<T *>(this));
  }
}


