#include "./RenderContext.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <ranges>
#include <vector>

namespace 
{

static std::vector<const char*> requiredDeviceExtensions = {
  vk::KHRSwapchainExtensionName
};

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
static vk::raii::PhysicalDevice SelectVkPhysicalDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface)
{
  // Get a list of all GPUs found by Vulkan.
  auto physicalDevices = instance.enumeratePhysicalDevices();

  // If there are no GPUs... then there is no rendering :(
  if (physicalDevices.empty()) {
    throw std::runtime_error("[VulkanRHI | Error] Failed to find GPUs with Vulkan Support.\n");
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
                            features2.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
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

} // namespace

/**
 * Documentation for basic logical device creation:
 *
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/04_Logical_device_and_queues.htm
 */
RenderContext::RenderContext(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface)
{
  // Select the most apporpriate physical device
  auto selectedPhysicalDevice = SelectVkPhysicalDevice(instance, surface);

  // List all familiy queues properties.
  auto queueFamiliyProperties = selectedPhysicalDevice.getQueueFamilyProperties();

  // Get Queue Family that supports both graphics and present.
  uint32 queueIndex = ~0;

  for (uint32 qfpIndex = 0; qfpIndex < queueFamiliyProperties.size(); qfpIndex++) {
    auto const& qfp = queueFamiliyProperties[qfpIndex];
    if ((qfp.queueFlags & vk::QueueFlagBits::eGraphics) && selectedPhysicalDevice.getSurfaceSupportKHR(qfpIndex, *surface)) {
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
      { .dynamicRendering = true },     // Enable dynamic rendering from Vulkan 1.3
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

  // assign the queue index and selected physical device.
  // Then use those to create the vkDevice and vkQueue objects.
  this->physicalDevice   = selectedPhysicalDevice;
  this->queueFamilyIndex = queueIndex;
  this->device           = vk::raii::Device(this->physicalDevice, deviceCreateInfo);
  this->queueFamily      = vk::raii::Queue(this->device, this->queueFamilyIndex, 0);
}
