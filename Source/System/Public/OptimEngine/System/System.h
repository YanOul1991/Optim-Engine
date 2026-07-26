#pragma once

#include "Core/CoreMinimal.h"
#include "Core/StandardTypes/String.h"
#include "Core/StandardTypes/TDynamicArray.h"

namespace System {
  
SYSTEM_API bool Initalize();

SYSTEM_API TDynamicArray<String> GetVulkanRequiredExtensions();

SYSTEM_API bool ProcessEvents();

SYSTEM_API void Quit();

}; // namespace System