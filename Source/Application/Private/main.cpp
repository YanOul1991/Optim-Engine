/**
 * main.cpp
 *
 * This file contains the prorgam entry and the global logic
 * for the program such as initialization logic, looping and
 * quit.
 */

// Slang modules compilation
#include "SlangCompiler.h"

// Core module
#include "Core/Debug/Debug.h"
#include "Core/StandardTypes/String.h"
#include "Core/Time/Time.h"

// Graphics related modules
#include "RHI/RHI.h"
#include "Rendering/Rendering.h"
#include "VulkanRHI/VulkanRHI.h"

// System module
#include "System/System.h"
#include "System/Window.h"

// STD HEADERS
#include <filesystem>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

using RHIBackend = VulkanRHI;

static void CompileSlangModules()
{
  Optim::Shaders::SlangSession slangSession;
  slangSession.InitGlobal();
  slangSession.CreateSession();

  // Path starts at running directory as default.
  std::filesystem::path dir("Shaders");

  std::vector<std::filesystem::directory_entry> slangModules;

  if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
    Debug::PrintLn("Compiling Slang Modules:");
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
      if (entry.path().extension().string() == ".slang") {
        Debug::Print("  {}... ", entry.path().stem().string());
        Optim::Shaders::ShaderModule shaderModule;
        auto result = shaderModule.LoadFromModule(slangSession.GetGlobalSession(), entry.path().stem().string().c_str());
        Debug::PrintLn("{}", (Optim::Shaders::Success(result) ? "SUCCESS" : "FAILURE"));
      }
    }
  }
  else {
    Debug::PrintLn("Failed to get dir :(");
  }
}

int main(int argc, char* argv[])
{
  Timer timer;
  timer.Start();

  try {
    CompileSlangModules();
  }
  catch (const std::filesystem::filesystem_error& e) {
    Debug::PrintError("Error: {}", e.what());
    return 1;
  }

  timer.End();
  Debug::PrintLn("Compiled slang modules in {} ms", timer.GetMs());

  // return 0;

  RHIBackend rhiinterface;

  // Initalize the System module
  System::Initalize();

  // Load the main window with the name of the application
  Window mainWindow = Window("Vulkan Engine Project");

  rhiinterface.Initialize(mainWindow.GetSDLWindowHandle());
  
  uint64 loopCycles = 0;
  double average    = 0;

  while (System::ProcessEvents()) {
    timer.Start();
    if (System::GetWindowResized()) {
      rhiinterface.ResizeWindow();
    }
    // Only draw if the window has pixels to draw onto.
    if (!System::GetWindowMinimized()) {
      rhiinterface.DrawFrame();
    }

    timer.End();

    average += timer.GetMs();
    loopCycles++;
  }

  average /= loopCycles;
  Debug::PrintLn("Average cycle time: {} ms ({} fps)", average, 1000.0 / average);

  rhiinterface.Cleanup();
  mainWindow.~Window();
  System::Quit();
  return 0;
}