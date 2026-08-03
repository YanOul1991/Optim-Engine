#pragma once

#include "OptimEngine/Core/CoreMinimal.h"
#include "OptimEngine/Core/StandardTypes/String.h"
#include "OptimEngine/Core/StandardTypes/TDynamicArray.h"
#include "OptimEngine/Core/Events/TDelegate.h"

namespace System {
  
SYSTEM_API bool Initalize();

SYSTEM_API TDynamicArray<String> GetVulkanRequiredExtensions();

SYSTEM_API bool ProcessEvents();

SYSTEM_API void Quit();

// SYSTEM_API TDelegate<> OnSystemModuleInitialized;

}; // namespace System
