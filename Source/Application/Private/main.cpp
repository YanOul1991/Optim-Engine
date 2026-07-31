/**
 * main.cpp
 *
 * This file contains the prorgam entry and the global logic
 * for the program such as initialization logic, looping and
 * quit.
 */

// Core module

#include "OptimEngine/Core/Memory/TUniquePtr.h"
#include "OptimEngine/Core/StandardTypes/String.h"
#include "OptimEngine/Core/Time/Time.h"

// Rendering module
#include "OptimEngine/Rendering/Rendering.h"

// System module
#include "OptimEngine/System/System.h"
#include "OptimEngine/System/Window.h"

// FMT test
#include <fmt/color.h>

// STD HEADERS
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

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

class Bar
{
 public:
  void Foo() {
    fmt::println(fg(fmt::color::green) | fmt::emphasis::bold, "System Module has been initialized");
  }
};

int main(int argc, char* argv[]) {
  
  Bar bar;
  System::OnSystemModuleInitialized.SubscribeMemberFunction<&Bar::Foo>(&bar);
  
  // Initalize the System module
  System::Initalize();

  // Load the main window with the name of the application
  Window mainWindow = Window("Vulkan Engine Project");

  // Create an instance of Rendering RHI.
  TUniquePtr<IRHI> pGraphicsRHI(Internal::Rendering::InstanciateRenderInterface());

  if (true) {
    auto requiredVulkanExtensions = System::GetVulkanRequiredExtensions();
    pGraphicsRHI.Get().Initialize(mainWindow.GetSDLWindowHandle());
  }

  uint64 loopCycles = 0;

  while (System::ProcessEvents()) {
    loopCycles++;
  }

  std::cout << "Cycles count: " << loopCycles << '\n';

  System::Quit();

  return 0;
}