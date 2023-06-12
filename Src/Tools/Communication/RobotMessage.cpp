#include "RobotMessage.h"
#include <bitpacker.hpp>
#include <vector>
#include <algorithm>

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
  byteOffset += (MAX_NUM_COMPONENTS / 8); 

  // Copy component hash
  memcpy(outBuff.data() + byteOffset, &componentHash, sizeof(componentHash)); 
  byteOffset += sizeof(componentHash);

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

  byteOffset += MAX_NUM_COMPONENTS / 8;

  // Copy component hash out of the buffer
  memcpy(&componentHash, buff.data() + byteOffset, sizeof(componentHash));
  byteOffset += sizeof(componentHash);

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

  static std::vector<ComponentMetadata> metadataByPriority(metadataById); // list for prioritization

  struct PrioritySortPredicate
  {
    inline bool operator() (const ComponentMetadata& a, const ComponentMetadata& b)
    {
      return *a.priority < *b.priority;
    }
  };

  std::sort(metadataByPriority.begin(), metadataByPriority.end(), PrioritySortPredicate());

  size_t bytesRemaining = 128;
  bytesRemaining -= MAX_NUM_COMPONENTS / 8;
  bytesRemaining -= sizeof(componentHash);

  for( auto metadata : metadataByPriority) {
    auto component = metadata.createNew();
    component->compileData();
    int size = component->getSize();
    if (size <= bytesRemaining) {
      bytesRemaining -= size;
      componentPointers.push_back(component);
    } else {
      return;
    }
  }
}



