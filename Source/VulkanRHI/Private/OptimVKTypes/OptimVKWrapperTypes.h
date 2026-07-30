#pragma once

#include "VulkanMinimal.h"

#include <iostream>

/**
 * Wrapper struct for device context
 */
struct RenderDevice
{
  vk::raii::PhysicalDevice physicalDevice = nullptr;
  vk::raii::Device         logicalDevice  = nullptr;
  vk::raii::Queue          graphicsQueue  = nullptr;

  uint32 graphicsQueueFamilyIndex = 0;
};