#pragma once

#include "OpVkCommon/Minimal.h"
#include "OpVkTypes/RenderDevice.h"
#include "OptimEngine/Core/CoreMinimal.h"
#include "OptimEngine/Core/StandardTypes/String.h"
#include "OptimEngine/Core/StandardTypes/TDynamicArray.h"
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
 *
 [[deprecated("This function may be removed in the future. Use `ValidateRequiredLayers` instead.")]]
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
 */

/**
 * @brief
 * Validates if all required layers are supported by the Vulkan implementation.
 * If one or more layer is not supported, a message will be printed listing them.
 */
inline bool ValidateRequiredLayers(const TDynamicArray<String>& reqLayers, const vk::raii::Context& context)
{
  TDynamicArray<String> unsupportedLayers;

  // Get list of all supported layers by Vulkan.
  auto supportedLayers = context.enumerateInstanceLayerProperties();

  // Validate all required layers and if they are supported. The ones that are
  // not will be added to the list of unsupported layers.
  for (const auto& reqLayer : reqLayers) {
    // auto comparaisonFn = [&reqLayer](const vk::LayerProperties& layerProperty) {
    //   return strcmp(layerProperty.layerName, reqLayer.GetPointer()) == 0;
    // };
    // if (std::ranges::none_of(supportedLayers, comparaisonFn)) {
    //   unsupportedLayers.EmplaceBack(reqLayer);
    // }

    // Switching to idiomatic for loop for better readability and performance.
    bool isSupported = false;

    for (const auto& layerProperty : supportedLayers) {
      if (strcmp(layerProperty.layerName, reqLayer.GetPointer()) == 0) {
        isSupported = true;
        break;
      }
    }

    if (!isSupported) {
      unsupportedLayers.EmplaceBack(reqLayer);
    }
  }

  if (unsupportedLayers.GetCount() > 0) {
    std::cout << "[VulkanRHI] The following layers are not supported by the Vulkan implementation:\n";
    for (const auto& layer : unsupportedLayers) {
      std::cout << "   " << layer.GetPointer() << "\n";
    }
    return false;
  }

  return true;
}

inline bool ValidateRequiredExtensions(TDynamicArray<String>& reqExtensions, const vk::raii::Context& context)
{
  // List of all unsuported extensions.
  TDynamicArray<String> unsupportedExtensions;

  // Get list of all supported extensions by Vulkan.
  auto vkExtensionProperties = context.enumerateInstanceExtensionProperties();

  // Validate all required extensions and if they are supported. The ones that are
  // not will be added to the list of unsupported extensions.
  for (const auto& reqExtension : reqExtensions) {
    // auto comparaisonFn = [&extension](const vk::ExtensionProperties& property) {
    //   return strcmp(property.extensionName, extension.GetPointer()) == 0;
    // };

    // if (std::ranges::none_of(vkExtensionProperties, comparaisonFn)) {
    //   unsupportedExtensions.EmplaceBack(extension);
    // }

    // Switching to idiomatic for loop for better readability and performance.

    bool isSupported = false;

    for (const auto& extensionProperty : vkExtensionProperties) {
      if (strcmp(extensionProperty.extensionName, reqExtension.GetPointer()) == 0) {
        isSupported = true;
        break;
      }
    }

    if (!isSupported) {
      unsupportedExtensions.EmplaceBack(reqExtension);
    }
  }

  if (unsupportedExtensions.GetCount() > 0) {
    std::cout << "[VulkanRHI] The following extensions are not supported by the Vulkan implementation:\n";
    for (const auto& extension : unsupportedExtensions) {
      std::cout << "   " << extension.GetPointer() << "\n";
    }
    return false;
  }

  return true;
}

inline void AddSDLRequiredExtensions(TDynamicArray<String>& reqExtensions)
{
  uint32             vkInstanceExtCount = 0;
  const char* const* ppVkInstanceExt    = SDL_Vulkan_GetInstanceExtensions(&vkInstanceExtCount);

  if (ppVkInstanceExt == nullptr) {
    throw std::runtime_error(SDL_GetError());
  }

  for (size_t i = 0; i < vkInstanceExtCount; i++) {
    reqExtensions.EmplaceBack(ppVkInstanceExt[i]);
  }
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
  for (const auto& extension : enabledExtensionNames) {
    ppExtensionNames.Push(extension.GetPointer());
  }

  // List of raw c style pointers for layers names
  for (const auto& layer : enabledLayerNames) {
    ppLayerNames.Push(layer.GetPointer());
  }

  // Simple debug to check generated list objects.
  if constexpr (true) {
    std::cout << "[VulkanRHI] Creating VkInstance object with the following (" << ppExtensionNames.GetCount() << ") extensions:\n";
    for (const auto& extension : ppExtensionNames) {
      std::cout << "   " << extension << "\n";
    }
    std::cout << "[VulkanRHI] Creating VkInstance object with the following (" << ppLayerNames.GetCount() << ") layers:\n";
    for (const auto& layer : ppLayerNames) {
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