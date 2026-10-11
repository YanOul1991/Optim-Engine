#pragma once

#include "Core/CoreMinimal.h"
#include "Core/StandardTypes/String.h"
#include "Core/StandardTypes/TDynamicArray.h"
#include "Core/Events/TDelegate.h"

namespace System {
  
SYSTEM_API bool Initalize();

SYSTEM_API bool ProcessEvents();

SYSTEM_API bool GetWindowResized();

SYSTEM_API bool GetWindowMinimized();

SYSTEM_API void Quit();

}; // namespace System
