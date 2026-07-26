/**
 * Rendering.h
 */

#pragma once

#include "OptimEngine/Core/CoreMinimal.h"
#include "OptimEngine/Core/StandardTypes/String.h"
#include "OptimEngine/Core/StandardTypes/TDynamicArray.h"
#include "OptimEngine/RHI/IRHI.h"

namespace Internal::Rendering {
  
RENDERING_API void Initialize();

RENDERING_API IRHI* InstanciateRenderInterface();

RENDERING_API void Setup(TDynamicArray<String>& settings);

} // namespace Internal::Rendering