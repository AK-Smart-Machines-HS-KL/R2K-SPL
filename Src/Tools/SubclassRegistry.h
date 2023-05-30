/**
 * @file SubclassRegistry.h
 * @author Andy Hobelsberger <mailto:anho1001@stud.hs-kl.de>
 * @brief 
 * @version 1.0
 * @date 2023-05-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <map>

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