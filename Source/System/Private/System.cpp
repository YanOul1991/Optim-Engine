/**
 * System.cpp
 */

#include "System/System.h"

#include "OptimEngine/Core/StandardTypes/String.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

bool System::Initalize() {
  if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
    String errorMsg = String("[RUNTIME ERROR]\n").Append("  SDL has failed to initalize\n").Append("  [Error message]\n").Append(SDL_GetError());
    fprintf(stderr, "%s", errorMsg.GetPointer());
    return false;
  }

  // OnSystemModuleInitialized.Broadcast();
  return true;
}

TDynamicArray<String> System::GetVulkanRequiredExtensions() {
  
  uint32             vkInstanceCount = 0;
  const char* const* vkInstanceExtensions = SDL_Vulkan_GetInstanceExtensions(&vkInstanceCount);

  TDynamicArray<String> vulkanExtensions;
   
  if (vkInstanceExtensions) {
    for (size_t i = 0; i < vkInstanceCount; i++) {
      vulkanExtensions.EmplaceBack(vkInstanceExtensions[i]);
    }
  }

  return vulkanExtensions;
}

bool System::ProcessEvents() {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      return false;
    }
  }
  
  SDL_Delay(1);
  return true;
}

void System::Quit() {
  SDL_Quit();
}