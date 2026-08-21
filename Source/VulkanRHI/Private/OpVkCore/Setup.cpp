#include "./Setup.h"

#include "OptimEngine/Core/StandardTypes/String.h"
#include "OptimEngine/Core/StandardTypes/TDynamicArray.h"
// SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
// std
#include <algorithm>
#include <iostream>
#include <map>
#include <ranges>
#include <vector>

namespace
{

/*
 * @brief
 * Validates if all required extensions are supported by the Vulkan implementation.
 * If one or more extension is not supported, a message will be printed listing them.
 */
[[nodiscard]]
static bool ValidateExtensions(const TDynamicArray<String>& reqExtensions, const vk::raii::Context& context)
{
  // List of all unsuported extensions.
  TDynamicArray<String> unsupportedExtensions;

  // Get list of all supported extensions by Vulkan.
  auto vkExtensionProperties = context.enumerateInstanceExtensionProperties();

  // Validate all required extensions and if they are supported. The ones that are
  // not will be added to the list of unsupported extensions.
  for (const auto& reqExtension : reqExtensions) {
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

[[nodiscard]]
bool ValidateLayers(const TDynamicArray<String>& reqLayers, const vk::raii::Context& context)
{
  TDynamicArray<String> unsupportedLayers;

  // Get list of all supported layers by Vulkan.
  auto supportedLayers = context.enumerateInstanceLayerProperties();

  // Validate all required layers and if they are supported. The ones that are
  // not will be added to the list of unsupported layers.
  for (const auto& reqLayer : reqLayers) {
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
} // namespace

vk::raii::SurfaceKHR Optim::VK::CreateVkSurfaceKHR(const vk::raii::Instance& instance, SDL_Window* sdlwindow)
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

vk::raii::Instance Optim::VK::CreateVkInstance(const vk::raii::Context& context)
{
  // ----------- EXTENSIONS VALIDATION -----------
  TDynamicArray<String> requiredExtensions;

  // Add the extensions required by SDL.
  uint32             vkInstanceExtCount = 0;
  const char* const* ppVkInstanceExt    = SDL_Vulkan_GetInstanceExtensions(&vkInstanceExtCount);

  if (ppVkInstanceExt == nullptr) {
    throw std::runtime_error(SDL_GetError());
  }

  for (size_t i = 0; i < vkInstanceExtCount; i++) {
    requiredExtensions.EmplaceBack(ppVkInstanceExt[i]);
  }

  // Add "VK_EXT_debug_utils" extension if validation layers are enabled
  if constexpr (Optim::VK::enableValidationLayers) {
    requiredExtensions.EmplaceBack(vk::EXTDebugUtilsExtensionName);
  }

  // Validate the required extensions.
  if (!ValidateExtensions(requiredExtensions, context)) {
    throw std::runtime_error("[Vulkan RHI] Error - Unsupported required extensions");
  }

  // ----------- LAYERS VALIDATION -----------

  // Required Layers Verification
  TDynamicArray<String> requiredLayers;
  // Add "VK_LAYER_KHRONOS_validation" if validation layers are enabled
  if constexpr (Optim::VK::enableValidationLayers) {
    requiredLayers.EmplaceBack("VK_LAYER_KHRONOS_validation");
  }

  if (!ValidateLayers(requiredLayers, context)) {
    throw std::runtime_error("[Vulkan RHI] Error - Unsupported required layers.");
  }

  // ----------- INSTANCE CREATION -----------
  TDynamicArray<const char*> ppExtensionNames;
  TDynamicArray<const char*> ppLayerNames;

  // List of raw c style pointers for extensions names
  for (const auto& extension : requiredExtensions) {
    ppExtensionNames.Push(extension.GetPointer());
  }

  // List of raw c style pointers for layers names
  for (const auto& layer : requiredLayers) {
    ppLayerNames.Push(layer.GetPointer());
  }

  // Simple debug to check generated list objects.
  if constexpr (false) {
    std::cout << "[VulkanRHI] Creating VkInstance object with the following (" << ppExtensionNames.GetCount() << ") extensions:\n";
    for (const auto& extension : ppExtensionNames) {
      std::cout << "   " << extension << "\n";
    }
    std::cout << "[VulkanRHI] Creating VkInstance object with the following (" << ppLayerNames.GetCount() << ") layers:\n";
    for (const auto& layer : ppLayerNames) {
      std::cout << "   " << layer << "\n";
    }
  }

  auto appInfo = Optim::VK::GetApplicationInfoStruct();
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