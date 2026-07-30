#pragma once

#include "OptimEngine/Core/CoreMinimal.h"
#include "OptimVKTypes/OptimVKWrapperTypes.h"
#include "VulkanMinimal.h"

namespace Optim::VK
{

/**
 * @brief
 * Check all available GPUs on system and gets the most appropriate one
 * from predefined prerequisists such as properties, supported features,
 * queueFamilies and extensions.
 *
 * vk::PhysicalDeviceProperties struct Documentation:
 * https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceProperties.html
 *
 * Device Limits struct (vk::PhysicalDeviceLimits)
 * https://docs.vulkan.org/spec/latest/chapters/limits.html
 */
vk::raii::PhysicalDevice SelectPhysicalDevice(const vk::raii::Instance& vkInstance);

/**
 * Documentation for basic logical device creation:
 *
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/04_Logical_device_and_queues.htm
 *
 */
RenderDevice CreateDeviceContext(const vk::raii::PhysicalDevice& physicalDevice);

}; // namespace Optim::VK