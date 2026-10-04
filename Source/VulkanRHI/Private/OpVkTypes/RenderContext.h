#pragma once

#include "OpVkCommon/Minimal.h"

/**
 * Encapsulates the runtime GPU interface. Owns the logical device handle, its
 * parent physical device context, and the primary queue binding used for 
 * frame rendering and command submission.
 */
struct RenderContext final
{
  vk::raii::PhysicalDevice physicalDevice   = nullptr;
  vk::raii::Device         device           = nullptr;
  vk::raii::Queue          queue            = nullptr;
  uint32                   queueIndex       = ~0u;

  [[nodiscard]]
  inline bool IsValid() const noexcept
  {
    return (physicalDevice != nullptr) &&
           (device != nullptr) &&
           (queue != nullptr) &&
           (queueIndex != ~0u);
  }

  // Null constructor
  RenderContext() = default;
  
  // Yes move
  RenderContext(RenderContext&&) noexcept            = default;
  RenderContext& operator=(RenderContext&&) noexcept = default;
  // No copy
  RenderContext(const RenderContext&)            = delete;
  RenderContext& operator=(const RenderContext&) = delete;

  // Non null default constructor
  RenderContext(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);
};
