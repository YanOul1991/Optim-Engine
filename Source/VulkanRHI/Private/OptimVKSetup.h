#pragma once

#include "OptimEngine/Core/CoreMinimal.h"
#include "OptimEngine/Core/StandardTypes/String.h"
#include "OptimEngine/Core/StandardTypes/TDynamicArray.h"
#include "OptimVKTypes/OptimVKWrapperTypes.h"
#include "VulkanMinimal.h"
// SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
// STD
#include <algorithm>
#include <iostream>
#include <map>
#include <ranges>
#include <vector>

namespace Optim::VK
{

/**
 * Creates VkSurfaceKHR object.
 */
inline vk::raii::SurfaceKHR CreateVkSurfaceKHR(const vk::raii::Instance& instance, SDL_Window*& sdlwindow)
{
  // Use the SDL function to create the surface as it abstracts the need 
  // to perform platform specific operations, figuring out the display service,
  // defining the correct macros, calling the correct vulkan functions, etc.
  VkSurfaceKHR rawSurface = VK_NULL_HANDLE;

  if (!SDL_Vulkan_CreateSurface(sdlwindow, *instance, nullptr, &rawSurface)) {
    throw std::runtime_error(SDL_GetError());
  }

  return vk::raii::SurfaceKHR(instance, rawSurface);
}

/**
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
  for (auto& layer : reqLayers) {
    auto comparaisonFn = [&layer](const vk::LayerProperties& property) {
      return strcmp(property.layerName, layer.GetPointer()) == 0;
    };

    if (std::ranges::none_of(vkLayerProperties, comparaisonFn)) {
      unsupportedLayers.EmplaceBack(layer);
    }
  }

  return unsupportedLayers;
}

inline vk::raii::Instance CreateVkInstance(
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

  // List of raw c style pointers for layers names
  for (String& layer : enabledLayerNames) {
    ppLayerNames.Push(layer.GetPointer());
  }

  // Simple debug to check generated list objects.
  if constexpr (true) {
    std::cout << "[VulkanRHI] Creating VkInstance object with the following (" << ppExtensionNames.GetCount() << ") extensions:\n";
    for (auto&& extension : ppExtensionNames) {
      std::cout << "   " << extension << "\n";
    }
    std::cout << "[VulkanRHI] Creating VkInstance object with the following (" << ppLayerNames.GetCount() << ") layers:\n";
    for (auto&& layer : ppLayerNames) {
      std::cout << "   " << layer << "\n";
    }
  }

  // Once the list for extension and layer have been generated, the appropriate
  // data can be passed to the `vk::InstanceCreateInfo` struct.
  vk::InstanceCreateInfo createInfo = {
    .pApplicationInfo        = &appInfo,
    .enabledLayerCount       = static_cast<uint32>(ppLayerNames.GetCount()),
    .ppEnabledLayerNames     = ppLayerNames.GetData(),
    .enabledExtensionCount   = static_cast<uint32>(ppExtensionNames.GetCount()),
    .ppEnabledExtensionNames = ppExtensionNames.GetData()
  };
  
  return vk::raii::Instance(context, createInfo);
}

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
vk::raii::PhysicalDevice SelectVkPhysicalDevice(const vk::raii::Instance& vkInstance, const vk::raii::SurfaceKHR& vkSurface);

/**
 * Documentation for basic logical device creation:
 *
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/04_Logical_device_and_queues.htm
 *
 */
RenderDevice CreateRenderDevice(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& vkSurface);

}; // namespace Optim::VK