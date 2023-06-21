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

#define SPL_MAX_MESSAGE_BYTES 128
#define MAX_NUM_COMPONENTS 64

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
  virtual size_t compress(char* buff) = 0;
  virtual bool decompress(char* compressed) = 0;
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
  static volatile ComponentRegistry registry; // = ComponentRegistry(ComponentMetadata{ T::name, T::create, T::setID });
  inline static int id = -1;
  inline static std::list<CallbackFunc_t> callbacks = std::list<CallbackFunc_t>(); 
  inline static std::list<CompilerFunc_t> dataCompilers = std::list<CompilerFunc_t>(); 

  public: 
  class CallbackRef {
    private:
    typename std::list<CallbackFunc_t>::iterator element;
    
    public:
    CallbackRef(CallbackRef&&) = default;
    CallbackRef(const CallbackRef&) = delete;
    CallbackRef() = default;
    CallbackRef(typename std::list<CallbackFunc_t>::iterator element) : element(element) {}
    ~CallbackRef() {callbacks.erase(element);}
  };

  class CompilerRef {
    private:
    typename std::list<CompilerFunc_t>::iterator element;
    
    public:
    CompilerRef(CompilerRef&&) = default;
    CompilerRef(const CompilerRef&) = delete;
    CompilerRef() = default;
    CompilerRef(typename std::list<CompilerFunc_t>::iterator element) : element(element) {}
    ~CompilerRef() {dataCompilers.erase(element);}
  };

  inline static int priority = 0;

  static CallbackRef addCallback(CallbackFunc_t foo) {
    callbacks.push_back(foo);
    return CallbackRef(--callbacks.end());
  }

  void doCallbacks(RobotMessageHeader& header) {
    for(auto callbackFunc : callbacks) {
      callbackFunc(static_cast<T *>(this), header);
    }
  }

  static CompilerRef addDataCompiler(CompilerFunc_t foo) {
    dataCompilers.push_back(foo);
    return CompilerRef(--dataCompilers.end());
  }
  
  void compileData() {
    for (auto dataCompiler : dataCompilers)
    {
      dataCompiler(static_cast<T *>(this));
    }
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