/**
 * Rendering.h
 */

#pragma once

#include "Core/CoreMinimal.h"
#include "Core/StandardTypes/String.h"
#include "Core/StandardTypes/TDynamicArray.h"
#include "RHI/Interface/IRHI.h"

namespace Internal::Rendering {
RENDERING_API void Initialize();

RENDERING_API IRHI* InstanciateRenderInterface();

RENDERING_API void Setup(TDynamicArray<String>& settings);

} // namespace Internal::Rendering