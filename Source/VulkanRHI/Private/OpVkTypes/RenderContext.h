#pragma once

#include "OpVkCommon/Minimal.h"

struct RenderContext final
{
  vk::raii::PhysicalDevice physicalDevice   = nullptr;
  vk::raii::Device         device           = nullptr;
  vk::raii::Queue          queueFamily      = nullptr;
  uint32                   queueFamilyIndex = ~0u;


  RenderContext() = default;

  RenderContext(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);

  // Enable Move Semantics
  RenderContext(RenderContext&&) = default;
  RenderContext& operator=(RenderContext&&) = default;
  // No copy semantics
  RenderContext(const RenderContext&) = delete;
  RenderContext& operator=(const RenderContext&) = delete;

  inline bool IsValid() const
  {
    return (physicalDevice != nullptr) &&
           (device != nullptr) &&
           (queueFamily != nullptr) &&
           (queueFamilyIndex != ~0u);
  }
};
