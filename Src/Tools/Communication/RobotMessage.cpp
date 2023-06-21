#include "RobotMessage.h"
#include "Tools/Settings.h"
#include <bitpacker.hpp>
#include <vector>
#include <algorithm>
#include "Platform/Time.h"


bool idsAssigned = false;
std::vector<ComponentMetadata> metadataById = std::vector<ComponentMetadata>();

void assignComponentIDs() {
  
  int currentID = 0;
  metadataById.reserve(ComponentRegistry::subclasses.size());

  for (auto &subclass : ComponentRegistry::subclasses)
  {
    subclass.setID(currentID);
    metadataById.push_back(subclass);
    
    currentID ++;
  }
  idsAssigned = true;
}

size_t RobotMessage::compress(std::array<uint8_t, SPL_MAX_MESSAGE_BYTES>& outBuff)
{
  if (!idsAssigned) {
    assignComponentIDs();
  }

  // Sort Components by ID 
  struct IDSortPredicate 
  {
    inline bool operator() (const std::shared_ptr<AbstractRobotMessageComponent>& a, const std::shared_ptr<AbstractRobotMessageComponent>& b)
    {
      return a->getID() < b->getID();
    }
  };

  std::sort(componentPointers.begin(), componentPointers.end(), IDSortPredicate());

  size_t byteOffset = 0; // current offset in bytes from the beginning of Buff
  std::array<uint8_t, SPL_MAX_MESSAGE_BYTES> componentBuff; // buffer reused for component compression

  // reserve space for bitfield
  size_t bitfieldBytes = MAX_NUM_COMPONENTS / 8 + (bool) (MAX_NUM_COMPONENTS % 8);
  byteOffset += bitfieldBytes; 

  // Copy component hash
  memcpy(outBuff.data() + byteOffset, &header.componentHash, sizeof(header.componentHash)); 
  byteOffset += sizeof(header.componentHash);

  memcpy(outBuff.data() + byteOffset, &header.senderID, sizeof(header.senderID)); 
  byteOffset += sizeof(header.senderID);

  memcpy(outBuff.data() + byteOffset, &header.timestamp, sizeof(header.timestamp)); 
  byteOffset += sizeof(header.timestamp);

  // Compress and Copy Individual Components
  for (auto component : componentPointers) {

    // compress component into componentBuff
    size_t len = component->compress((char*) componentBuff.data()); 

    // Copy component into buffer
    memcpy(outBuff.data() + byteOffset, componentBuff.data(), len);

    // Adjust ByteOffset
    byteOffset += len;

    // set component bit in bitfield
    bitpacker::insert(outBuff, component->getID(), 1, (uint8_t) 1);
  }
  
  return byteOffset;
}

bool RobotMessage::decompress(std::array<uint8_t, SPL_MAX_MESSAGE_BYTES>& buff){
  if (!idsAssigned) {
    assignComponentIDs();
  }

  size_t byteOffset = 0;

  std::list<int> includedComponents = std::list<int>();
  for (int id = 0; id < ComponentRegistry::subclasses.size() ; id++)
  {
    bool included = (bool) bitpacker::extract<uint8_t>(buff, id, 1);
    if(included) {
      includedComponents.push_back(id);
    }
  }

  size_t bitfieldBytes = MAX_NUM_COMPONENTS / 8 + (bool) (MAX_NUM_COMPONENTS % 8);
  byteOffset += bitfieldBytes; 

  // Copy component hash out of the buffer
  memcpy(&header.componentHash, buff.data() + byteOffset, sizeof(header.componentHash));
  byteOffset += sizeof(header.componentHash);

  // TODO: Compare Hash
  
  // Copy Sender
  memcpy(&header.senderID, buff.data() + byteOffset, sizeof(header.senderID));
  byteOffset += sizeof(header.senderID);

  // Copy Timestamp
  memcpy(&header.timestamp, buff.data() + byteOffset, sizeof(header.timestamp));
  byteOffset += sizeof(header.timestamp);

  for (auto &componentID : includedComponents)
  {
    auto component = metadataById[componentID].createNew();
    bool success = component->decompress((char*) buff.data() + byteOffset);
    if (!success) {
      return false;
    }
    componentPointers.push_back(component);
    byteOffset += component->getSize();
  }
  
  return true;
}

void RobotMessage::compile() {
  if (!idsAssigned) {
    assignComponentIDs();
  }

  header.componentHash = 1337; //TODO: Generate and use hash
  header.senderID = Global::getSettings().playerNumber;
  header.timestamp = Time::getCurrentSystemTime();

  static std::vector<ComponentMetadata> metadataByPriority(metadataById); // list for prioritization

  // Sorts ComponenentMetadata by Component priority
  struct PrioritySortPredicate {
    inline bool operator() (const ComponentMetadata& a, const ComponentMetadata& b)
    {
      return *a.priority > *b.priority;
    }
  };

  std::sort(metadataByPriority.begin(), metadataByPriority.end(), PrioritySortPredicate());

  size_t bytesRemaining = 128;
  bytesRemaining -= MAX_NUM_COMPONENTS / 8 + (bool) (MAX_NUM_COMPONENTS % 8);
  bytesRemaining -= sizeof(header.componentHash);

  for(auto metadata : metadataByPriority) {

    // Components with a priority 0 or lower are not compiled
    if(!(*metadata.priority > 0)) {
      break;
    }

    auto component = metadata.createNew();
    component->compileData();
    int size = component->getSize();
    if (size <= bytesRemaining) {
      bytesRemaining -= size;
      componentPointers.push_back(component);
    } else {
      break;
    }
  }
}



