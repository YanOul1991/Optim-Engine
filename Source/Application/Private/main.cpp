/**
 * main.cpp
 *
 * This file contains the prorgam entry and the global logic
 * for the program such as initialization logic, looping and
 * quit.
 */

#include "SlangCompiler.h"

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

// STD HEADERS
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

using RHIBackend = VulkanRHI;

int main(int argc, char* argv[])
{
  Timer timer;
  timer.Start();

  Optim::Shaders::SlangCompilerInterface slangSession;
  slangSession.InitalizeGlobalSession();
  slangSession.VerifyTargetSupport();
  
  RHIBackend rhiinterface;

  // Initalize the System module
  System::Initalize();

  // Load the main window with the name of the application
  Window mainWindow = Window("Vulkan Engine Project");

  rhiinterface.Initialize(mainWindow.GetSDLWindowHandle());

  uint64 loopCycles = 0;
  double average = 0;

  while (System::ProcessEvents()) {
    timer.Start();
    rhiinterface.DrawFrame();
    timer.End();

    if (loopCycles < 2048) {
      average += timer.GetMs();
      loopCycles++;
    }
  }

  average /= loopCycles;

  std::printf("Average cycle time: %lf ms (%lf fps)\n", average, 1000.0 / average);

  rhiinterface.Cleanup();
  
  mainWindow.~Window();

  System::Quit();

  return 0;
}