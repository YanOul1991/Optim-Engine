/**
 * Rendering.h
 */

#pragma once

#include "OptimEngine/Core/CoreMinimal.h"
#include "OptimEngine/Core/Memory/TUniquePtr.h"
#include "OptimEngine/RHI/IRHI.h"

namespace Internal
{

class RENDERING_API RenderModule final
{
 public:
  inline RenderModule(IRHI* pRhi) : rhi(pRhi) {}

  const TUniquePtr<IRHI> rhi;
};

}; // namespace Internal