#include "./Setup.h"

#include "Core/Debug/Debug.h"
#include "Core/StandardTypes/String.h"
// #include "Core/StandardTypes/TDynamicArray.h"
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

/**
 * @brief
 * Validates if all required layers are supported by the Vulkan implementation.
 * If one or more layer is not supported, a message will be printed listing them.
 */
[[nodiscard]]
static bool ValidateExtensions(const std::vector<const char*>& reqExtensions, const vk::raii::Context& context)
{
  // List of all unsuported extensions.
  std::vector<const char*> unsupportedExtensions;

  // Get list of all supported extensions by Vulkan.
  auto vkExtensionProperties = context.enumerateInstanceExtensionProperties();

  // Validate all required extensions and if they are supported. The ones that are
  // not will be added to the list of unsupported extensions.
  for (const auto& reqExtension : reqExtensions) {
    bool isSupported = false;

    for (const auto& extensionProperty : vkExtensionProperties) {
      if (strcmp(extensionProperty.extensionName, reqExtension) == 0) {
        isSupported = true;
        break;
      }
    }

    if (!isSupported) {
      unsupportedExtensions.push_back(reqExtension);
    }
  }

  if (unsupportedExtensions.size() > 0) {
    std::cout << "[VulkanRHI] The following extensions are not supported by the Vulkan implementation:\n";
    for (const auto& extension : unsupportedExtensions) {
      // std::cout << "   " << extension.GetPointer() << "\n";
      Debug::PrintLn("[Vulkan RHI - Error] Unsupported extension: {}", extension);
    }
    return false;
  }
  return true;
}

/**
 * @brief
 * Validates if all required extensions are supported by the Vulkan implementation.
 * If one or more extension is not supported, a message will be printed listing them.
 */
[[nodiscard]]
bool ValidateLayers(const std::vector<const char*>& reqLayers, const vk::raii::Context& context)
{
  std::vector<const char*> unsupportedLayers;

  // Get list of all supported layers by Vulkan.
  auto supportedLayers = context.enumerateInstanceLayerProperties();

  // Validate all required layers and if they are supported. The ones that are
  // not will be added to the list of unsupported layers.
  for (const auto& reqLayer : reqLayers) {
    bool isSupported = false;

    for (const auto& layerProperty : supportedLayers) {
      if (strcmp(layerProperty.layerName, reqLayer) == 0) {
        isSupported = true;
        break;
      }
    }

    if (!isSupported) {
      unsupportedLayers.push_back(reqLayer);
    }
  }

  if (unsupportedLayers.size() > 0) {
    std::cout << "[VulkanRHI] The following layers are not supported by the Vulkan implementation:\n";
    for (const auto& layer : unsupportedLayers) {
      // std::cout << "   " << layer.GetPointer() << "\n";
      Debug::PrintLn("[Vulkan RHI - Error] Unsupported layer: {}", layer);
    }
    return false;
  }

  return true;
}
} // namespace

/**
 * @brief
 * Creates a Vulkan surface object using the SDL function 
 * `SDL_Vulkan_CreateSurface`.This function abstracts the need to perform 
 * platform specific operations, figuring out the display service, defining the
 * correct macros, vulkan functions, etc.
 */
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

/**
 * @brief 
 * Create a `vk::raii::Instance` object. This function, validates all required
 * extensions such as the ones from SDL, and verifies all required Layers.
 * 
 * May throw a `std::runtime_error` if anu required extension or layer is not
 * supported.
 */
vk::raii::Instance Optim::VK::CreateVkInstance(const vk::raii::Context& context)
{
  // ----------- EXTENSIONS VALIDATION -----------
  // TDynamicArray<String> requiredExtensions;
  std::vector<const char*> requiredExtensions;

  // Add the extensions required by SDL.
  uint32             vkInstanceExtCount = 0;
  const char* const* ppVkInstanceExt    = SDL_Vulkan_GetInstanceExtensions(&vkInstanceExtCount);

  if (ppVkInstanceExt == nullptr) {
    throw std::runtime_error(SDL_GetError());
  }

  for (size_t i = 0; i < vkInstanceExtCount; i++) {
    requiredExtensions.push_back(ppVkInstanceExt[i]);
  }

  // Add "VK_EXT_debug_utils" extension if validation layers are enabled
  if constexpr (Optim::VK::enableValidationLayers) {
    requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
  }

  // Validate the required extensions.
  if (!ValidateExtensions(requiredExtensions, context)) {
    throw std::runtime_error("[Vulkan RHI] Error - Unsupported required extensions");
  }

  // ----------- LAYERS VALIDATION -----------

  // Required Layers Verification
  std::vector<const char*> requiredLayers;
  // Add "VK_LAYER_KHRONOS_validation" if validation layers are enabled
  if constexpr (Optim::VK::enableValidationLayers) {
    requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
  }

  if (!ValidateLayers(requiredLayers, context)) {
    throw std::runtime_error("[Vulkan RHI] Error - Unsupported required layers.");
  }

  // ----------- INSTANCE CREATION -----------
  // std::vector<const char*> ppExtensionNames;
  // std::vector<const char*> ppLayerNames;

  // List of raw c style pointers for extensions names
  // for (const auto& extension : requiredExtensions) {
  //   ppExtensionNames.push_back(extension.GetPointer());
  // }

  // // List of raw c style pointers for layers names
  // for (const auto& layer : requiredLayers) {
  //   ppLayerNames.push_back(layer.GetPointer());
  // }

  // Simple debug to check generated list objects.
  if constexpr (true) {
    std::cout << "[VulkanRHI] Creating VkInstance object with the following (" << requiredExtensions.size() << ") extensions:\n";
    for (const auto& extension : requiredExtensions) {
      std::cout << "   " << extension << "\n";
    }
    std::cout << "[VulkanRHI] Creating VkInstance object with the following (" << requiredLayers.size() << ") layers:\n";
    for (const auto& layer : requiredLayers) {
      std::cout << "   " << layer << "\n";
    }
  }

  auto appInfo = Optim::VK::GetApplicationInfoStruct();
  // Once the list for extension and layer have been generated, the appropriate
  // data can be passed to the `vk::InstanceCreateInfo` struct.
  vk::InstanceCreateInfo createInfo = {
    .pApplicationInfo        = &appInfo,
    .enabledLayerCount       = static_cast<uint32>(requiredLayers.size()),
    .ppEnabledLayerNames     = requiredLayers.data(),
    .enabledExtensionCount   = static_cast<uint32>(requiredExtensions.size()),
    .ppEnabledExtensionNames = requiredExtensions.data()
  };

  return vk::raii::Instance(context, createInfo);
}