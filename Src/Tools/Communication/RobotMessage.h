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
#include <functional>
#include "Tools/SubclassRegistry.h"

#define SPL_MAX_MESSAGE_BYTES 128

struct AbstractRobotMessageComponent {
  virtual size_t compress(char* buff) = 0;
  virtual bool decompress(char* compressed) = 0;
  virtual void doCallbacks() = 0;
  virtual void compileData() = 0;
};

using ComponentRegistry = SubclassRegistry<AbstractRobotMessageComponent, std::string, AbstractRobotMessageComponent* (*)()>;

template<typename T>
class RobotMessageComponent : public AbstractRobotMessageComponent {

  protected:
  volatile static ComponentRegistry registry;
  inline static std::set<std::function<void(T*)>> callbacks = std::set<std::function<void(T*)>>(); 
  inline static std::set<std::function<void(T*)>> dataCompilers = std::set<std::function<void(T*)>>(); 
  
  public:
  static void addCallback(std::function<void(T*)> foo) {callbacks.insert(foo);}
  static void removeCallback(std::function<void(T*)> foo) {callbacks.erase(foo);}
  void doCallbacks() {
    for(auto callbackFunc : callbacks)
    {
      callbackFunc(static_cast<T *>(this));
    }
  }

  static void addDataCompiler(std::function<void(T*)> foo) {dataCompilers.insert(foo);}
  static void removeDataCompiler(std::function<void(T*)> foo) {dataCompilers.erase(foo);}
  void compileData() {
    for (auto dataCompiler : dataCompilers)
    {
      dataCompiler(static_cast<T *>(this));
    }
  }

  static AbstractRobotMessageComponent* create() {return new T();}

  RobotMessageComponent() {
      (void) registry; // Access registry so it is not optimized out of existence by the compiler
  }
};

// registry must be defined out of class so T can exist before registry is set
template<typename T>
volatile ComponentRegistry RobotMessageComponent<T>::registry = ComponentRegistry(T::name, T::create);

class RobotMessage
{
    public: 
    uint32_t componentHash;                                 // Hash value of possible components, to ensure that messages are compatible
    uint64_t componentsIncluded;                            // Bitfield of included components 
    std::vector<AbstractRobotMessageComponent*> componentPointers;  // Pointers to the included components

    /**
     * @brief 
     * 
     * @param compressed 
     * @return true decompression success
     * @return false 
     */
    bool decompress(std::array<uint8_t, SPL_MAX_MESSAGE_BYTES> buff);

    /**
     * @brief generates a compressed version of the message into buff
     * 
     * @return size_t number of significant bits written to buffer
     */
    size_t compress(std::array<uint8_t, SPL_MAX_MESSAGE_BYTES> buff);

    /**
     * @brief size, in bytes of the compressed packet
     * 
     * @return size_t 
     */
    size_t size() {
        return sizeof(RobotMessage);
    }
};