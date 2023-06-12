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

struct AbstractRobotMessageComponent {
  /**
   * @brief 
   * 
   * @param buff 
   * @return size_t 
   */
  virtual size_t compress(char* buff) = 0;
  virtual bool decompress(char* compressed) = 0;
  virtual void doCallbacks() = 0;
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

  private:
  static volatile ComponentRegistry registry; // = ComponentRegistry(ComponentMetadata{ T::name, T::create, T::setID });
  inline static std::list<std::function<void(T*)>> callbacks = std::list<std::function<void(T*)>>(); 
  inline static std::list<std::function<void(T*)>> dataCompilers = std::list<std::function<void(T*)>>(); 
  inline static int id = -1;

  public:
  inline static int priority = 0;

  static void addCallback(std::function<void(T*)> foo) {callbacks.push_back(foo);}
  static void removeCallback() {callbacks.pop_back();} // TODO
  void doCallbacks() final {
    for(auto callbackFunc : callbacks)
    {
      callbackFunc(static_cast<T *>(this));
    }
  }

  static void addDataCompiler(std::function<void(T*)> foo) {dataCompilers.push_back(foo);}
  static void removeDataCompiler() {dataCompilers.pop_back();} // TODO
  void compileData() final {
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
    uint32_t componentHash = 1337; // Temporary                                                        // Hash value of possible components, to ensure that messages are compatible
    uint64_t componentsIncluded;                                                    // Bitfield of included components 
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
        component->doCallbacks();
      }
    }

    void compile();

    /**
     * @brief size, in bytes of the compressed packet
     * 
     * @return size_t 
     */
};