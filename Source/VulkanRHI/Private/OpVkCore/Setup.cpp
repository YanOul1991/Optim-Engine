#include "./Setup.h"

#include "Core/Debug/Debug.h"
#include "Core/StandardTypes/String.h"
// SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
// std
#include <algorithm>
#include <iostream>
#include <map>
#include <ranges>
#include <vector>

// Extent of the physical device selection process .
static std::vector<const char*> requiredDeviceExtensions = {
  vk::KHRSwapchainExtensionName,
  vk::KHRSynchronization2ExtensionName
};

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
    std::cout << "[VulkanRHI - Error] The following extensions are not supported by the Vulkan implementation:\n";
    for (const auto& extension : unsupportedExtensions) {
      Debug::PrintLn("  {}", extension);
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
    std::cout << "[VulkanRHI - Error] The following layers are not supported by the Vulkan implementation:\n";
    for (const auto& layer : unsupportedLayers) {
      Debug::PrintLn("  {}", layer);
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
[[deprecated("Use Optim::VK::CreateVkSurfaceKHR instead.")]]
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
 * May throw a `std::runtime_error` if any required extension or layer is not
 * supported.
 */
[[deprecated("Use Optim::CreateVkInstance instead.")]]
vk::raii::Instance Optim::VK::CreateVkInstance(const vk::raii::Context& context) {
    // ----------- EXTENSIONS VALIDATION -----------
  std::vector<const char*> requiredExtensions;

  // Add the extensions required by SDL.
  uint32             vkInstanceExtCount = 0;
  const char* const* ppVkInstanceExt    = SDL_Vulkan_GetInstanceExtensions(&vkInstanceExtCount);

  if (ppVkInstanceExt == nullptr) {
    throw std::runtime_error(SDL_GetError());
  }

  // Add extensions required by SDL.
  for (size_t i = 0; i < vkInstanceExtCount; i++) {
    requiredExtensions.push_back(ppVkInstanceExt[i]);
  }

  // Add "VK_EXT_debug_utils" extension if validation layers are enabled
  if constexpr (Optim::VK::enableValidationLayers) {
    requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
  }

  // Validate the required extensions.
  if (!ValidateExtensions(requiredExtensions, context)) {
    throw std::runtime_error("[VulkanRHI - Error] - Unsupported required extensions");
  }

  // ----------- LAYERS VALIDATION -----------

  // Required Layers Verification
  std::vector<const char*> requiredLayers;

  // Add "VK_LAYER_KHRONOS_validation" if validation layers are enabled
  if constexpr (Optim::VK::enableValidationLayers) {
    requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
  }

  if (!ValidateLayers(requiredLayers, context)) {
    throw std::runtime_error("[VulkanRHI - Error] - Unsupported required layers.");
  }

  // Simple debug to check generated list objects.
  if constexpr (false) {
    Debug::PrintLn("Creating vk::raii::Instance with {} extension:", requiredExtensions.size());
    for (const auto& extension : requiredExtensions) {
      Debug::PrintLn("  {}", extension);
    }
    Debug::PrintLn("Creating vk::raii::Instance with {} layers:", requiredLayers.size());
    for (const auto& layer : requiredLayers) {
      Debug::PrintLn("  {}", layer);
    }
  }

  // auto appInfo = Optim::VK::GetApplicationInfoStruct();
  vk::ApplicationInfo appInfo{
    .pApplicationName   = "Vulkan Learning",
    .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
    .pEngineName        = "Optim Engine",
    .engineVersion      = VK_MAKE_VERSION(0, 0, 1),
    .apiVersion         = Optim::VK::ApiVersion
  };

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

vk::raii::Instance Optim::CreateVkInstance(const vk::raii::Context& context)
{
  // ----------- EXTENSIONS VALIDATION -----------
  std::vector<const char*> requiredExtensions;

  // Add the extensions required by SDL.
  uint32             vkInstanceExtCount = 0;
  const char* const* ppVkInstanceExt    = SDL_Vulkan_GetInstanceExtensions(&vkInstanceExtCount);

  if (ppVkInstanceExt == nullptr) {
    throw std::runtime_error(SDL_GetError());
  }

  // Add extensions required by SDL.
  for (size_t i = 0; i < vkInstanceExtCount; i++) {
    requiredExtensions.push_back(ppVkInstanceExt[i]);
  }

  // Add "VK_EXT_debug_utils" extension if validation layers are enabled
  if constexpr (Optim::VK::enableValidationLayers) {
    requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
  }

  // Validate the required extensions.
  if (!ValidateExtensions(requiredExtensions, context)) {
    throw std::runtime_error("[VulkanRHI - Error] - Unsupported required extensions");
  }

  // ----------- LAYERS VALIDATION -----------

  // Required Layers Verification
  std::vector<const char*> requiredLayers;

  // Add "VK_LAYER_KHRONOS_validation" if validation layers are enabled
  if constexpr (Optim::VK::enableValidationLayers) {
    requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
  }

  if (!ValidateLayers(requiredLayers, context)) {
    throw std::runtime_error("[VulkanRHI - Error] - Unsupported required layers.");
  }

  // Simple debug to check generated list objects.
  if constexpr (false) {
    Debug::PrintLn("Creating vk::raii::Instance with {} extension:", requiredExtensions.size());
    for (const auto& extension : requiredExtensions) {
      Debug::PrintLn("  {}", extension);
    }
    Debug::PrintLn("Creating vk::raii::Instance with {} layers:", requiredLayers.size());
    for (const auto& layer : requiredLayers) {
      Debug::PrintLn("  {}", layer);
    }
  }

  // auto appInfo = Optim::VK::GetApplicationInfoStruct();
  vk::ApplicationInfo appInfo{
    .pApplicationName   = "Vulkan Learning",
    .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
    .pEngineName        = "Optim Engine",
    .engineVersion      = VK_MAKE_VERSION(0, 0, 1),
    .apiVersion         = Optim::VK::ApiVersion
  };

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

vk::raii::SurfaceKHR Optim::CreateVkSurfaceKHR(const vk::raii::Instance& instance, SDL_Window* sdlwindow)
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

vk::raii::PhysicalDevice Optim::SelectVkPhysicalDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface)
{
  // Get a list of all GPUs found by Vulkan.
  auto physicalDevices = instance.enumeratePhysicalDevices();

  // If there are no GPUs... then there is no rendering :(
  if (physicalDevices.empty()) {
    throw std::runtime_error("[VulkanRHI - Error] Failed to find GPUs with Vulkan Support.\n");
  }

  // A multimap of all potential GPUs that can be used. Each PhysicalDevice
  // will be attributed a score depending on their properties and supported
  // features. The one with the highest score will be used for rendering.
  std::multimap<int64, vk::raii::PhysicalDevice> candidates;
  for (const auto& gpu : physicalDevices) {
    auto properties             = gpu.getProperties();
    auto queueFamiliyProperties = gpu.getQueueFamilyProperties();
    auto availableExtensions    = gpu.enumerateDeviceExtensionProperties();

    int64 score = 0;

    // Prioritize discreet GPU (dedicated graphics card) as they offer
    // a peformance advantage.
    if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
      score += 1000;
    }

    score += properties.limits.maxImageDimension2D;

    // Verify support for Vulkan 1.4 API. If the API is not supported then the
    // GPU cannot be used.
    bool supportsVulkan1_4 = (properties.apiVersion >= Optim::VK::ApiVersion);

    bool supportsReqQueueFamily = false;
    for (uint32 qfpIndex = 0; qfpIndex < queueFamiliyProperties.size(); qfpIndex++) {
      auto const& qfp = queueFamiliyProperties[qfpIndex];

      // Get Queue Family that supports both graphics and present.
      if ((qfp.queueFlags & vk::QueueFlagBits::eGraphics) && gpu.getSurfaceSupportKHR(qfpIndex, *surface)) {
        supportsReqQueueFamily = true;
        break;
      }
    }

    // Check if all required extensions are supported by the GPU.
    bool supportsExtensions = std::ranges::all_of(requiredDeviceExtensions, [&availableExtensions](const auto& requiredExt) {
      return std::ranges::any_of(availableExtensions, [requiredExt](const vk::ExtensionProperties& availableDeviceExt) {
        return strcmp(availableDeviceExt.extensionName, requiredExt);
      });
    });

    auto features2 = gpu.getFeatures2<vk::PhysicalDeviceFeatures2,
                                      vk::PhysicalDeviceVulkan11Features,
                                      vk::PhysicalDeviceVulkan13Features,
                                      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

    bool supportsFeatures = features2.get<vk::PhysicalDeviceFeatures2>().features.geometryShader &&
                            features2.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                            features2.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering && // <- use dynamic rendering
                            features2.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

    // If all the requirements are validated then then add handle to the list of potential condidates with a score.
    if (supportsVulkan1_4 && supportsReqQueueFamily && supportsExtensions && supportsFeatures) {
      candidates.insert(std::make_pair(score, gpu));
    }
  }

  // If no valid candiate has been found then RIP.
  // Else pick the last GPU of the candidate map, since the score are in ascending order.
  if (candidates.empty() || candidates.rbegin()->first <= 0) {
    return nullptr;
  }
  else {
    return candidates.rbegin()->second;
  }
}


bool Optim::CreateVkDeviceAndQueue(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface, vk::raii::Device &outDevice, vk::raii::Queue &outQueue, uint32 &outQueueIndex)
{
  // List all familiy queues properties.
  auto queueFamiliyProperties = physicalDevice.getQueueFamilyProperties();

  // Get Queue Family that supports both graphics and present.
  uint32 queueIndex = ~0;

  for (uint32 qfpIndex = 0; qfpIndex < queueFamiliyProperties.size(); qfpIndex++) {
    auto const& qfp = queueFamiliyProperties[qfpIndex];
    if ((qfp.queueFlags & vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface)) {
      queueIndex = qfpIndex;
      break;
    }
  }

  if (queueIndex == ~0) {
    throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
  }

    // Create StructureChain object of feature structures.
  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
    featureChain = {
      {},                               // vk::PhysicalDeviceFeatures2 (empty for now)
      { .shaderDrawParameters = true }, // Enable shader draw parameters from Vulkan 1.1
      { 
        .synchronization2 = true,
        .dynamicRendering = true 
      },    // Enable dynamic rendering from Vulkan 1.3
      { .extendedDynamicState = true }  // Enable extended dynamic state from the extension
    };

  // Create the device.
  float queuePriority = 0.5f;

  vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
    .queueFamilyIndex = queueIndex,
    .queueCount       = 1,
    .pQueuePriorities = &queuePriority
  };

  vk::DeviceCreateInfo deviceCreateInfo{
    .pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
    .queueCreateInfoCount    = 1,
    .pQueueCreateInfos       = &deviceQueueCreateInfo,
    .enabledExtensionCount   = static_cast<uint32>(requiredDeviceExtensions.size()),
    .ppEnabledExtensionNames = requiredDeviceExtensions.data()
  };

  outDevice     = vk::raii::Device(physicalDevice, deviceCreateInfo);
  outQueue      = vk::raii::Queue(outDevice, queueIndex, 0);
  outQueueIndex = queueIndex;

  return *outDevice && *outQueue && outQueueIndex != ~0;
}

// vk::raii::Queue Optim::CreateVkQueue(const vk::raii::PhysicalDevice &physicalDevice, uint32 &outQueueIndex)
// {
//   // List all familiy queues properties.
//   auto queueFamiliyProperties = physicalDevice.getQueueFamilyProperties();
// }

// vk::raii::Device Optim::CreateVkDevice(const vk::raii::PhysicalDevice &physicalDevice, uint32 queueIndex)
// {

// }