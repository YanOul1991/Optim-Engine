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
  Optim::Shaders::SlangCompilerInterface slangSession;
  slangSession.InitalizeGlobalSession();
  slangSession.VerifyTargetSupport();

  // Slang::ComPtr<slang::IGlobalSession> globalSession;
  // slang::createGlobalSession(globalSession.writeRef());

  // // Optim::Shaders::SlangCompiler::InitalizeGlobalSession();
  // Optim::Shaders::SlangCompiler::VerifyTargetSupport(globalSession.get());

  RHIBackend rhiinterface;

  // Initalize the System module
  System::Initalize();

  // Load the main window with the name of the application
  Window mainWindow = Window("Vulkan Engine Project");

  rhiinterface.Initialize(mainWindow.GetSDLWindowHandle());

  uint64 loopCycles = 0;

  while (System::ProcessEvents()) {
    loopCycles++;
    // TODO: 
    //  Process Input Events.

    // Draw Rendering Frames.
    rhiinterface.DrawFrame();
  }

  std::cout << "Cycles count: " << loopCycles << '\n';

  mainWindow.~Window();

  System::Quit();

  return 0;
}