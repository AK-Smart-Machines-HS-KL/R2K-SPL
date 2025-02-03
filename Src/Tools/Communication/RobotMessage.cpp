#include "RobotMessage.h"
#include "Tools/Settings.h"
#include <bitpacker.hpp>
#include <vector>
#include <algorithm>
#include "Platform/Time.h"


bool idsAssigned = false;
std::vector<ComponentMetadata> metadataById = std::vector<ComponentMetadata>();
uint32_t messageVersionHash = 1337;

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

  // HEADER
  memcpy(outBuff.data() + byteOffset, &header, sizeof(header)); 
  byteOffset += sizeof(header);

  // COMPONENT BITFIELD
  const size_t bitfieldOffset = byteOffset; // offset for the beginning of the bitfield;

  // reserve space for bitfield
  memset(outBuff.data() + byteOffset, 0, COMPONENT_BITFIELD_SIZE);
  byteOffset += COMPONENT_BITFIELD_SIZE; 

  // COMPONENTS
  std::array<uint8_t, SPL_MAX_MESSAGE_BYTES> componentBuff; // buffer reused for component compression

  for (auto component : componentPointers) {

    // compress component into componentBuff
    size_t len = component->compress(componentBuff.data()); // TODO: Memory safety needed! (Use std::span maybe?)

    // TODO: Verify that component fits

    // Copy component into buffer
    memcpy(outBuff.data() + byteOffset, componentBuff.data(), len);

    // Adjust ByteOffset
    byteOffset += len;

    // set component bit in bitfield
    // offset is `bitfieldOffset * 8 + component->getID()`, as in the nth bit in the bitfield
    bitpacker::insert(outBuff, bitfieldOffset * 8 + component->getID(), 1, (uint8_t) 1);
  }
  
  return byteOffset;
}

bool RobotMessage::decompress(std::array<uint8_t, SPL_MAX_MESSAGE_BYTES>& buff){
  if (!idsAssigned) {
    assignComponentIDs();
  }

  size_t byteOffset = 0;

  // Copy header
  memcpy(&header, buff.data() + byteOffset, sizeof(header));
  byteOffset += sizeof(header);

  // Verify component hash
  if(header.componentHash != messageVersionHash) {
    return false;
  }

  // Read which components are included
  std::list<int> includedComponents = std::list<int>(); // List of IDs of included components, 
  for (int id = 0; id < ComponentRegistry::subclasses.size() ; id++)
  {
    bool included = (bool) bitpacker::extract<uint8_t>(buff, byteOffset * 8 + id, 1);
    if(included) {
      includedComponents.push_back(id);
    }
  }
  byteOffset += COMPONENT_BITFIELD_SIZE; 

  

  // decompress components
  // `includedComponents` is inherently sorted from lowest to highest, so this is OK 
  for (auto &componentID : includedComponents)
  {
    auto component = metadataById[componentID].createNew();
    bool success = component->decompress(buff.data() + byteOffset); // TODO: Memory safety needed! (Use std::span maybe?)
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

  // Have a copy of the component metadata list. Thread-local for paralellism support
  thread_local std::vector<ComponentMetadata> metadataByPriority(metadataById); // list for prioritization

  // Sorts ComponenentMetadata by Component priority
  struct PrioritySortPredicate {
    inline bool operator() (const ComponentMetadata& a, const ComponentMetadata& b)
    {
      return *a.priority > *b.priority;
    }
  };

  std::sort(metadataByPriority.begin(), metadataByPriority.end(), PrioritySortPredicate());

  size_t bytesRemaining = 128; // Number of bytes remaining for the next component
  bytesRemaining -= sizeof(header); // Reserve Header Space
  bytesRemaining -= COMPONENT_BITFIELD_SIZE; // Reserve Component Bitfield

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
      // Log which component we stopped at & which ones were left out
      break;
    }
  }
}



