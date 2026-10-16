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
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

using RHIBackend = VulkanRHI;

int main(int argc, char* argv[])
{
  Timer timer;
  timer.Start();

  try {
    // Path starts at running directory as default.
    std::filesystem::path dir("Shaders");

    std::vector<std::filesystem::directory_entry> slangModules;
    // std::vector<std::filesystem::directory_entry> sprivBinaries; 

    if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
      // slangModules.reserve(32);

      for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().extension().string() == ".slang") {
          slangModules.push_back(entry);
        }
        // else if (entry.path().extension().string() == ".spv") {
        //   sprivBinaries.push_back(entry);
        // }
      }
    }
    else {
      fmt::println("Failed to get dir :(");
    }

    Optim::Shaders::SlangCompilerInterface slangSession;
    slangSession.InitalizeGlobalSession();
    slangSession.VerifyTargetSupport();

    fmt::println("Compiling Slang Modules:");
    for (const auto& mod : slangModules) {
      Optim::Shaders::ShaderModule shaderModule;

      auto result = shaderModule.LoadFromModule(slangSession.GetGlobalSession(), mod.path().stem().string().c_str());

      if (Optim::Shaders::Success(result)) {
        fmt::println("  [{}] : Sucess", mod.path().stem().string());
      }
      else {
        fmt::println("  [{}] : Failure", mod.path().stem().string());
      }
    }
    // fmt::println("SPRIV Binaries:");
    // for (const auto& bin : sprivBinaries) {
    //   fmt::println("  {}", bin.path().string());
    // }
  }
  catch (const std::filesystem::filesystem_error& e) {
    fmt::println(stderr, "Error: {}", e.what());
    return 1;
  }

  return 0;

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

    if (loopCycles < 2048) {
      average += timer.GetMs();
      loopCycles++;
    }
  }

  average /= loopCycles;
  Debug::PrintLn("Average cycle time: {} ms ({} fps)", average, 1000.0 / average);
  // fmt::println("Avergae cycle time: {} ms ({} fps)", average, 1000.0 / average);

  rhiinterface.Cleanup();

  mainWindow.~Window();

  System::Quit();

  return 0;
}