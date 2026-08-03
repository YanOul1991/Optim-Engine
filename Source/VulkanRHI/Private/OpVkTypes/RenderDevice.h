#pragma once

#include "OpVkCommon/Minimal.h"

struct QueueFamilyIndices
{
  uint32 graphicsQueueFamilyIndex = ~0u;
  uint32 presentQueueFamilyIndex  = ~0u;

  inline bool IsValid() const
  {
    return (graphicsQueueFamilyIndex != ~0u) &&
           (presentQueueFamilyIndex != ~0u);
  }
};

struct RenderDevice
{
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device         logicalDevice  = nullptr;
  vk::raii::Queue          graphicsQueue  = nullptr;

  uint32 graphicsQueueFamilyIndex = ~0u;

  inline bool IsValid() const
  {
    return (physicalDevice != nullptr) &&
           (logicalDevice != nullptr) &&
           (graphicsQueue != nullptr) &&
           (graphicsQueueFamilyIndex != ~0u);
  }
};
