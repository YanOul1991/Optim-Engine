#pragma once

#include "OptimEngine/Core/CoreMinimal.h"
#include <iostream>

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