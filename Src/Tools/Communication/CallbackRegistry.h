/**
 * @file CallbackRegistry.h
 * @author Andy Hobelsberger <mailto:anho1001@stud.hs-kl.de>
 * @brief
 * @version 1.0
 * @date 2023-07-02
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <functional>
#include <list>
#include <memory>

template <typename Func_t>
class CallbackRegistry
{
private:
  std::shared_ptr<std::list<Func_t>> callbacks = std::make_shared<std::list<Func_t>>();

public:
  class _CallbackRef
  {
  private:
    typename std::weak_ptr<std::list<Func_t>> list_ptr = nullptr;
    typename std::list<Func_t>::iterator element;

  public:
    _CallbackRef(typename std::weak_ptr<std::list<Func_t>> list_ptr, typename std::list<Func_t>::iterator element) : list_ptr(list_ptr), element(element) {}
    ~_CallbackRef()
    {
      if (!list_ptr.expired())
      {
        std::shared_ptr<std::list<Func_t>> locked_ptr = list_ptr.lock();
        locked_ptr->erase(element);
      }
    }
  };

  typedef std::shared_ptr<_CallbackRef> Callback;

  Callback add(Func_t foo)
  {
    callbacks->push_back(foo);
    auto ref = std::make_shared<_CallbackRef>(callbacks, --callbacks->end());
    return ref;
  }

  template <class... Args>
  void call(Args&&... args)
  {
    for (auto &foo : *callbacks)
    {
      foo(args...);
    }
  }
};
