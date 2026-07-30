#include "OptimVKSetup.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <ranges>
#include <vector>

// List of all obligatory physical device extensions.
static std::vector<const char*> requiredDeviceExtensions = {
  vk::KHRSwapchainExtensionName
};

vk::raii::PhysicalDevice Optim::VK::SelectPhysicalDevice(const vk::raii::Instance& vkInstance)
{
  // Get a list of all GPUs found by Vulkan.
  auto physicalDevices = vkInstance.enumeratePhysicalDevices();

  // If there are no GPUs... then there is no rendering :(
  if (physicalDevices.empty()) {
    throw std::runtime_error("[VulkanRHI | Error] Failed to find GPUs with Vulkan Support.\n");
  }

  // A multimap of all potential GPUs that can be used. Each PhysicalDevice
  // will be attributed a score depending on their properties and supported
  // features. The one with the highest score will be used for rendering.
  std::multimap<int64, vk::raii::PhysicalDevice> candidates;

  for (const auto& gpu : physicalDevices) {
    auto properties = gpu.getProperties();
    auto queueFamilies       = gpu.getQueueFamilyProperties();
    auto availableExtensions = gpu.enumerateDeviceExtensionProperties();

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

    bool supportsGraphics = std::ranges::any_of(queueFamilies, [](const vk::QueueFamilyProperties& qfp) {
      return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
    });

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
                            features2.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                            features2.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

    // If all the requirements are validated then then add handle to the list of potential condidates with a score.
    if (supportsVulkan1_4 && supportsGraphics && supportsExtensions && supportsFeatures) {
      candidates.insert(std::make_pair(score, gpu));
    }
  }

  // If no valid candiate has been found then RIP.
  if (candidates.empty() || candidates.rbegin()->first <= 0) {
    // throw std::runtime_error("[VulkanRHI | Error] Failed to found suitable GPU.\n");
    return nullptr;
  }
  else {
    // Else pick the last GPU of the candidate map, since the score are in ascending order.
    return candidates.rbegin()->second;
  }
}

/**
 * Documentation for basic logical device creation:
 *
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/04_Logical_device_and_queues.htm
 *
 */
RenderDevice Optim::VK::CreateDeviceContext(const vk::raii::PhysicalDevice& physicalDevice)
{
  RenderDevice retRenderDevice;

  // List all familiy queues properties.
  auto queueFamiliyProperties = physicalDevice.getQueueFamilyProperties();

  // Find the first queue with graphics capabilities
  auto graphicsQueueFamilyProperty = std::ranges::find_if(queueFamiliyProperties, [](auto const& qfp) {
    return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
  });

  float  queuePriority = 0.5f;
  uint32 queueIndex    = static_cast<uint32>(std::distance(queueFamiliyProperties.begin(), graphicsQueueFamilyProperty));

  vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
    .queueFamilyIndex = queueIndex,
    .queueCount       = 1,
    .pQueuePriorities = &queuePriority
  };

  // Create StructureChain object of feature structures.
  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
    featureChain = {
      {},                               // vk::PhysicalDeviceFeatures2 (empty for now)
      { .shaderDrawParameters = true }, // Enable shader draw parameters from Vulkan 1.1
      { .dynamicRendering = true },     // Enable dynamic rendering from Vulkan 1.3
      { .extendedDynamicState = true }  // Enable extended dynamic state from the extension
    };

  vk::DeviceCreateInfo deviceCreateInfo{
    .pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
    .queueCreateInfoCount    = 1,
    .pQueueCreateInfos       = &deviceQueueCreateInfo,
    .enabledExtensionCount   = static_cast<uint32>(requiredDeviceExtensions.size()),
    .ppEnabledExtensionNames = requiredDeviceExtensions.data()
  };

  // Set logical first then use that member handle to create the graphics queue.
  retRenderDevice.logicalDevice            = vk::raii::Device(physicalDevice, deviceCreateInfo);
  retRenderDevice.physicalDevice           = physicalDevice;
  retRenderDevice.graphicsQueue            = vk::raii::Queue(retRenderDevice.logicalDevice, queueIndex, 0);
  retRenderDevice.graphicsQueueFamilyIndex = queueIndex;

  return retRenderDevice;
}
