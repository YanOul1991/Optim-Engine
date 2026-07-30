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

  inline void Validate() {
    std::cout << "PhysicalDevice Initialization status: " << (physicalDevice != nullptr) << '\n';
    std::cout << "LogicalDevice Initialization status : " << (logicalDevice != nullptr) << '\n';
    std::cout << "GraphicsQueue Initialization status : " << (graphicsQueue != nullptr) << '\n';
  }
};