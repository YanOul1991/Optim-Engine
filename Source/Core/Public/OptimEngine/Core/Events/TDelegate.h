#pragma once

#include "OptimEngine/Core/CoreMinimal.h"
#include <iostream>


/**
 * Example usage: 
 * ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 *  Delegate<float, int> someDelegate;
 *  ExampleClass e;
 *  someDelegate.SubscribeMemberFunction<&ExampleClass::Foo>(&e);
 *  someDelegate.Broadcast(35.3478f, 999);
 * ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 */
template <typename... ARGS> class TDelegate final
{
 public:
  template <auto memberFunctionOffset> void SubscribeMemberFunction(auto* instance) {
    subscriber = instance;
    call       = [](void* pSubscriberInstance, ARGS... args) -> void {
      (static_cast<decltype(instance)>(pSubscriberInstance)->*memberFunctionOffset)(args...);
    };
  }

  void Broadcast(ARGS... args) {
    if (subscriber) {
      call(subscriber, args...);
    }
  }

 private:
  void* subscriber = nullptr;
  void (*call)(void* subscriberInstance, ARGS...);
};