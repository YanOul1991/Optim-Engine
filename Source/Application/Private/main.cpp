/**
 * main.cpp
 *
 * This file contains the prorgam entry and the global logic
 * for the program such as initialization logic, looping and
 * quit.
 */

// Core module

#include "Core/Memory/TUniquePtr.h"
#include "Core/StandardTypes/String.h"

// Rendering module
#include "OptimEngine/Rendering/Rendering.h"

// System module
#include "System/System.h"
#include "System/Window.h"

// STD HEADERS
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

template <typename... ARGS> class Delegate final
{
 public:
  template <auto memberFunctionOffset> void SubscribeMemberFunction(auto* instance) {
    subscriber = instance;
    call       = [](void* pSubscriberInstance, ARGS... args) -> void {
      (static_cast<decltype(instance)>(pSubscriberInstance)->*memberFunctionOffset)(args...);
    };
    std::cout << "Subscriber has.. subscribed!\n";
  }

  void Broadcast(ARGS... args) {
    if (subscriber) {
      call(subscriber, args...);
      std::cout << "[Delegate] Broadcast function called\n";
    }
  }

 private:
  void* subscriber = nullptr;
  void (*call)(void* subscriberInstance, ARGS...);
};

class ExampleClass final
{
 public:
  void Foo(float paramFloat, int paramInt) {
    std::cout << "Foo function from ExampleClass class has been called\n";
    std::cout << "Passed params: " << paramFloat << " & " << paramInt << '\n';
  }
};

/**
 * TODO:
 * Move the TFunction class template into the core module.
 */
template <typename T, typename... ARGS> class TFunction final
{
 public:
  TFunction() {
  }

  TFunction(auto pFunction) {
    p_func = pFunction;
  }

  auto operator=(auto pFunction) {
    p_func = pFunction;
    return this;
  }

  bool IsValid() const {
    return p_func != nullptr;
  }

  operator bool() const {
    return p_func != nullptr;
  }

  T operator()(ARGS... args) const {
    return p_func(args...);
  }

 private:
  T (*p_func)(ARGS...);
};
// Delegate<float, int> someDelegate;
// ExampleClass e;
// someDelegate.SubscribeMemberFunction<&ExampleClass::Foo>(&e);
// someDelegate.Broadcast(35.3478f, 999);
// printf("Current C++ version: %s", GetCPPVersion());

int main(int argc, char* argv[]) {
  
  // Initalize the System module
  System::Initalize();

  // Load the main window with the name of the application
  Window           mainwindow = Window("Vulkan Engine Project");
  
  TUniquePtr<IRHI> pGraphicsRHI;
  pGraphicsRHI.SetPtr(Internal::Rendering::InstanciateRenderInterface());

  if (true) {
    auto requiredVulkanExtensions = System::GetVulkanRequiredExtensions();
    pGraphicsRHI.GetRef().Initialize(requiredVulkanExtensions);
  }
  
  std::cout << "[APPLICATION] Starting Processing events\n";

  uint64 loopCycles = 0;

  while (System::ProcessEvents()) {
    loopCycles++;
  }

  std::cout << "Cycles count: " << loopCycles << '\n';

  System::Quit();

  return 0;
}