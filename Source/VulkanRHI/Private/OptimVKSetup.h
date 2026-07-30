#pragma once

#include "OptimEngine/Core/CoreMinimal.h"
#include "OptimEngine/Core/StandardTypes/String.h"
#include "OptimEngine/Core/StandardTypes/TDynamicArray.h"
#include "OptimVKTypes/OptimVKWrapperTypes.h"
#include "VulkanMinimal.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <ranges>
#include <vector>

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

/**
 * @todo
 * Improve functions implementation to allow to implement other logic if some
 * layers are not supported.
 *
 * @brief
 * Verifies if all desired layers are supported by the Vulkan implementation.
 * For now if one or more layer is not validated, simply prints a message
 * listing them.
 *
 * @return
 * A `TDynamicArray<String>` containing all unsupported layers.
 */
inline TDynamicArray<String> VerifyRequiredLayers(const TDynamicArray<String>& reqLayers, const vk::raii::Context& context)
{
  // List of all unsuported layers.
  TDynamicArray<String> unsupportedLayers;

  // Get list of all supported layers by Vulkan.
  auto vkLayerProperties = context.enumerateInstanceLayerProperties();

  // Validate all required layers and if they are supported. The ones that are
  // not will be added to the list of unsupported layers.
  for (auto& reqLayer : reqLayers) {
    auto comparaisonFn = [&reqLayer](const vk::LayerProperties& property) {
      return strcmp(property.layerName, reqLayer.GetPointer()) == 0;
    };

    if (std::ranges::none_of(vkLayerProperties, comparaisonFn)) {
      unsupportedLayers.EmplaceBack(reqLayer);
    }
  }

  return unsupportedLayers;
}

}; // namespace Optim::VK