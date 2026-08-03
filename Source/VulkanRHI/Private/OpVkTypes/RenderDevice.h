#pragma once

#include "OpVkCommon/Minimal.h"

struct RenderDevice
{
  vk::raii::PhysicalDevice physicalDevice   = nullptr;
  vk::raii::Device         logicalDevice    = nullptr;
  vk::raii::Queue          queueFamily      = nullptr;
  uint32                   queueFamilyIndex = ~0u;

  inline bool IsValid() const
  {
    return (physicalDevice != nullptr) &&
           (logicalDevice != nullptr) &&
           (queueFamily != nullptr) &&
           (queueFamilyIndex != ~0u);
  }
};
