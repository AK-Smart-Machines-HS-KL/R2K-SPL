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
#include <set>

template <typename base_T, typename val_T>
struct SubclassRegistry {
  public: 
  inline static std::set<val_T> subclasses = std::set<val_T>();

  SubclassRegistry(val_T val) {
    subclasses.insert(val);
  };
};