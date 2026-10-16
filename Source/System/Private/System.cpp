/**
 * System.cpp
 */

#include "System/System.h"

#include "Core/StandardTypes/String.h"
#include "Core/Debug/Logging.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

static bool windowResized   = false;
static bool windowMinimized = false;

bool System::Initalize()
{
  if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
    // String errorMsg = String("[RUNTIME ERROR]\n").Append("  SDL has failed to initalize\n").Append("  [Error message]\n").Append(SDL_GetError());
    // fprintf(stderr, "%s", errorMsg.GetPointer());
    Debug::PrintLn("[Runtime Error]; SDL initialization status: FAILURE; Error message: [{}]", SDL_GetError());
    return false;
  }

  // OnSystemModuleInitialized.Broadcast();
  return true;
}

bool System::ProcessEvents()
{
  windowResized = false;
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      return false;
    }

    if (event.type == SDL_EVENT_WINDOW_MINIMIZED) {
      windowMinimized = true;
      windowResized   = true;
    }
    else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
      windowResized = true;
      windowMinimized = false;
    }
  }

  // SDL_Delay(1);
  return true;
}

bool System::GetWindowResized()
{
  return windowResized;
}

bool System::GetWindowMinimized() 
{
  return windowMinimized;
}

void System::Quit()
{
  SDL_Quit();
}