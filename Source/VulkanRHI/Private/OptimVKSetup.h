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

using StringList = TDynamicArray<String>;

inline vk::raii::Instance CreateVulkanInstanceObject(
  const vk::raii::Context& context,
  TDynamicArray<String>&   enabledExtensionNames,
  TDynamicArray<String>&   enabledLayerNames,
  vk::ApplicationInfo&&    appInfo)
{
    TDynamicArray<const char*> ppExtensionNames;
  TDynamicArray<const char*> ppLayerNames;

  // List of raw c style pointers for extensions names
  for (String& extention : enabledExtensionNames) {
    ppExtensionNames.Push(extention.GetPointer());
  }

  // If using validation layers than add it to the extension list.
  // Since the Vulkan API contains already `char*` to some
  // extension names, it can simply be directly added here to
  // the extension names list.
  if constexpr (Optim::VK::enableValidationLayers) {
    ppExtensionNames.Push(vk::EXTDebugUtilsExtensionName);
  }

  // List of raw c style pointers for layers names
  for (String& layer : enabledLayerNames) {
    ppLayerNames.Push(layer.GetPointer());
  }

  // Simple debug to check generated list objects.
  if constexpr (false) {
    std::cout << "[VulkanRHI] Creating Vulkan instance with the following (" << ppExtensionNames.GetCount() << ") extensions:\n";
    for (auto&& extension : ppExtensionNames) {
      std::cout << "   " << extension << "\n";
    }
    std::cout << "[VulkanRHI] Creating Vulkan instance with the following (" << ppLayerNames.GetCount() << ") layers:\n";
    for (auto&& layer : ppLayerNames) {
      std::cout << "   " << layer << "\n";
    }
  }

  // Once the list for extension and layer have been generated, the appropriate
  // data can be passed to the `vk::InstanceCreateInfo` struct.
  vk::InstanceCreateInfo createInfo;
  createInfo.pApplicationInfo        = &appInfo;
  createInfo.enabledLayerCount       = ppLayerNames.GetCount();
  createInfo.ppEnabledLayerNames     = ppLayerNames.GetData();
  createInfo.enabledExtensionCount   = ppExtensionNames.GetCount();
  createInfo.ppEnabledExtensionNames = ppExtensionNames.GetData();

  return vk::raii::Instance(context, createInfo);
}

}; // namespace Optim::VK