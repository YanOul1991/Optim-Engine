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

// Graphics related modules
#include "OptimEngine/RHI/RHI.h"
#include "OptimEngine/Rendering/Rendering.h"
#include "OptimEngine/VulkanRHI/VulkanRHI.h"

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

class Bar
{
 public:
  void Foo()
  {
    fmt::println(fg(fmt::color::green) | fmt::emphasis::bold, "System Module has been initialized");
  }
};

int main(int argc, char* argv[])
{
  Bar bar;
  System::OnSystemModuleInitialized.SubscribeMemberFunction<&Bar::Foo>(&bar);
  
  // Initalize the System module
  System::Initalize();

  // Load the main window with the name of the application
  Window mainWindow = Window("Vulkan Engine Project");

  Internal::RenderModule rhiBackend(new VulkanRHI());

  rhiBackend.rhi.GetPtr()->Initialize(mainWindow.GetSDLWindowHandle());

  uint64 loopCycles = 0;

  while (System::ProcessEvents()) {
    loopCycles++;
  }

  std::cout << "Cycles count: " << loopCycles << '\n';

  mainWindow.~Window();

  System::Quit();

  return 0;
}