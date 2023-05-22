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

#define SPL_MAX_MESSAGE_BYTES 128

template <typename base_T, typename key_T, typename val_T>
struct SubclassRegistry {
  public: 
  SubclassRegistry(key_T key, val_T val) {
    subclasses[key] = val;
  };
  static std::map<key_T, val_T> subclasses;
};

template <typename base_T, typename key_T, typename val_T>
std::map<key_T, val_T> SubclassRegistry<base_T, key_T, val_T>::subclasses = std::map<key_T, val_T>();


struct AbstractRobotMessageComponent {
  virtual size_t compress(char* buff) = 0;
  virtual bool decompress(char* compressed) = 0;
};

using ComponentRegistry = SubclassRegistry<AbstractRobotMessageComponent, std::string, AbstractRobotMessageComponent* (*)()>;

template<typename T>
class RobotMessageComponent : AbstractRobotMessageComponent {
  
    private:
      volatile static ComponentRegistry m;
      static std::set<void (*) (T*)> callbacks; 
      static std::set<void (*) (T*)> dataCompilers; 
    
    public:
      
      static void addCallback(void (*foo) (T*));
      static void removeCallback(void (*foo) (T*));
      void doCallbacks();

      static void addDataCompiler(void (*foo) (T*));
      static void removeDataCompiler(void (*foo) (T*));
      void compileData();

      static AbstractRobotMessageComponent* create() {return new T();}

      RobotMessageComponent() {
          (void) m; // Access m so it is not optimized out of existence by the compiler
      }
};

// m must be defined out of class so T can exist before m is set
template<typename T>
volatile ComponentRegistry RobotMessageComponent<T>::m = ComponentRegistry(typeid(T).name(),  T::create);

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

// class SubclassRegister {
//     public:
//     static std::vector<std::string> subclasses{};
//     SubclassRegister(const char *name) {
//         subclasses.push_back(name);
//     }
// };



// template<typename T>
// SubclassRegister Base<T>::r = StaticClassType(typeid(T).name());
