#include "RobotMessage.h"
#include <bitpacker.hpp>
#include <vector>
#include <algorithm>

size_t RobotMessage::compress(std::array<uint8_t, SPL_MAX_MESSAGE_BYTES> outBuff)
{
    size_t byteOffset = 0; // current offset in bytes from the beginning of Buff
    std::array<uint8_t, SPL_MAX_MESSAGE_BYTES> componentBuff; // buffer reused for component compression

    // reserve space for bitfield
    byteOffset += (sizeof(componentsIncluded)); 

    // Copy component hash
    memcpy(outBuff.data() + byteOffset, &componentHash, sizeof(componentHash)); 
    byteOffset += sizeof(componentHash);

    // Copy Individual Components
    for (auto component : componentPointers) {

      // compress component into componentBuff
      size_t len = component->compress((char*) componentBuff.data()); 
      
      // check if length of component exceeds max Message Bytes
      if (byteOffset + len > SPL_MAX_MESSAGE_BYTES) {
        // TODO: Handle
        return 0;
      }

      // Copy component into buffer
      memcpy(outBuff.data() + byteOffset, componentBuff.data(), len);

      // set component bit in bitfield
      // size_t newBits = component->compress();
    }
    return byteOffset;
}

bool RobotMessage::decompress(std::array<uint8_t, SPL_MAX_MESSAGE_BYTES> buff){
  size_t bitIdx = 0;
  componentHash = bitpacker::extract<uint32_t>(buff, bitIdx, bitIdx += 32);
  componentsIncluded = 0;
  componentsIncluded = bitpacker::extract<uint64_t>(buff, 32, 96);
    return false;
}



