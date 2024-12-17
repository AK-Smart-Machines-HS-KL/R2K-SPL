/**
 * @file RobotMessage.h
 * @author Andy Hobelsberger <mailto:anho1001@stud.hs-kl.de>
 * @brief 
 * @version 1.0
 * @date 2023-05-19
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <functional> // lambda abstraction for callbacks
#include <memory>     // shared_ptr
#include "Tools/SubclassRegistry.h"
#include "CallbackRegistry.h"

using namespace std::placeholders;

#define SPL_MAX_MESSAGE_BYTES 128
#define COMPONENT_BITFIELD_SIZE 4

struct RobotMessageHeader {
  uint32_t componentHash = 1337; // Temporary     
  uint16_t senderID;
  uint32_t timestamp;
};

struct AbstractRobotMessageComponent {
  /**
   * @brief 
   * 
   * @param buff 
   * @return size_t 
   */
  virtual ~AbstractRobotMessageComponent() = default;

  virtual size_t compress(uint8_t* buff) = 0;
  virtual bool decompress(uint8_t* compressed) = 0;
  virtual void doCallbacks(RobotMessageHeader& header) = 0;
  virtual void compileData() = 0;
  virtual size_t getSize() = 0;
  virtual int getID() = 0;
};

struct ComponentMetadata {
  std::string name;
  std::shared_ptr<AbstractRobotMessageComponent> (*createNew)();
  int* priority;
  void (*setID)(int);

  bool operator<(const ComponentMetadata& b) const {
    return name < b.name; 
  };
};

using ComponentRegistry = SubclassRegistry<AbstractRobotMessageComponent, ComponentMetadata>;

template<typename T>
class RobotMessageComponent : public AbstractRobotMessageComponent {
  public:
  using CallbackFunc_t = std::function<void(T*, RobotMessageHeader&)>;
  using CompilerFunc_t = std::function<void(T*)>;

  private:
  static volatile ComponentRegistry registry;
  inline static int id = -1;


  public: 
  inline thread_local static CallbackRegistry<CompilerFunc_t> onCompile = CallbackRegistry<CompilerFunc_t>();
  inline thread_local static CallbackRegistry<CallbackFunc_t> onRecieve = CallbackRegistry<CallbackFunc_t>();

  typedef typename CallbackRegistry<CallbackFunc_t>::Callback Callback;
  typedef typename CallbackRegistry<CompilerFunc_t>::Callback Compiler;

  inline static int priority = 0;

  void doCallbacks(RobotMessageHeader& header) {
    onRecieve.call(static_cast<T *>(this), header);
  }
  
  void compileData() {
    onCompile.call(static_cast<T *>(this));
  }

  static std::shared_ptr<AbstractRobotMessageComponent> create() {
    return std::shared_ptr<AbstractRobotMessageComponent>(new T());
  }

  int getID() final {
    return id;
  }

  /**
   * @brief Sets the ID for this component. Is called Implicitly and can only be called once
   * 
   * @param id 
   */
  static void setID(int newID) {
    if (id == -1) {
      id = newID;
    } else {
      // TODO: Throw error
    }
  }

  RobotMessageComponent() {
      (void) registry; // Access registry so it is not optimized out of existence by the compiler
  }
};

// registry must be defined out of class so T can exist before registry is set
template<typename T>
volatile ComponentRegistry RobotMessageComponent<T>::registry = ComponentRegistry(ComponentMetadata{T::name, T::create, &T::priority, T::setID});

class RobotMessage
{

  public:
  RobotMessageHeader header; 
  std::vector<std::shared_ptr<AbstractRobotMessageComponent>> componentPointers;  // Pointers to the included components. With smart pointers we don't need to worry about deleting them!
  inline thread_local static CallbackRegistry<std::function<void(RobotMessageHeader&)>> onRecieve = CallbackRegistry<std::function<void(RobotMessageHeader&)>>();

    /**
     * @brief 
     * 
     * @param compressed 
     * @return true decompression success
     * @return false 
     */
    bool decompress(std::array<uint8_t, SPL_MAX_MESSAGE_BYTES>& buff);

    /**
     * @brief generates a compressed version of the message into buff
     * 
     * @return size_t number of significant Bytes written to buffer
     */
    size_t compress(std::array<uint8_t, SPL_MAX_MESSAGE_BYTES>& buff);

    /**
     * @brief Runs all callbacks for components
     * 
     */
    void doCallbacks() {
      for( auto component : componentPointers) {
        onRecieve.call(header);
        component->doCallbacks(header);
      }
    }

    
    void compile();

    /**
     * @brief size, in bytes of the compressed packet
     * 
     * @return size_t 
     */
};