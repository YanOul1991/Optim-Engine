/**
 * Rendering.h
 */

#pragma once

#include <memory>
#include "Core/CoreMinimal.h"
#include "RHI/IRHI.h"

namespace Internal
{

class RENDERING_API RenderModule final
{
 public:
  inline RenderModule(IRHI* pRhi) : rhi(pRhi) {}

  const std::unique_ptr<IRHI> rhi;
};

}; // namespace Internal