/**
 * FILE: VulkanRHI.cpp
 */

#include "VulkanRHI/VulkanRHI.h"

#include "OptVulkanDebug.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <ranges>
#include <vector>

// Local namespace
namespace VulkanObj {

static vk::raii::Context                context;                  // Context Handle
static vk::raii::Instance               instance       = nullptr; // VkInstance Handle
static vk::raii::PhysicalDevice         physicalDevice = nullptr; // Physical Device (GPU) handle
static vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
static vk::raii::Device                 device         = nullptr; // Logical Device Handle

}; // namespace VulkanObj

static std::vector<char const*> requiredLayers = {};

static std::vector<const char*> requiredDeviceExtensions = {
  vk::KHRSwapchainExtensionName
};

/**
 * @todo
 * Improve functions implementation to allow to implement other logic if
 * some layers are not supported.
 *
 * @brief
 * Verifies if all desired layers are supported by the Vulkan implementations.
 * For now if one or more layer is not validated, simply prints a message
 * listing them.
 *
 * @return
 * A `TDynamicArray<String>` containing all validated layers.
 */
static TDynamicArray<String> VerifyRequiredLayers() {
  // Create a list of validated and unsupported layers.
  TDynamicArray<String>      validatedLayers;
  TDynamicArray<const char*> unsupportedLayers;

  // Add the validation layers if enabled
  if constexpr (Optim::VK::enableValidationLayers) {
    requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
  }

  // Get list of all supported layers by Vulkan implementation.
  auto vkLayerProperties = VulkanObj::context.enumerateInstanceLayerProperties();

  // Validate all required layers and if they are supported.
  for (const char* const& reqLayer : requiredLayers) {
    auto comparaisonFn = [&reqLayer](const vk::LayerProperties& property) {
      return strcmp(property.layerName, reqLayer) == 0;
    };

    // Add the layer to the appropriate list whether they are supported
    // or not.
    if (std::ranges::none_of(vkLayerProperties, comparaisonFn)) {
      unsupportedLayers.EmplaceBack(reqLayer);
    }
    else {
      validatedLayers.EmplaceBack(reqLayer);
    }
  }

  if (unsupportedLayers.GetCount() > 0) {
    std::cout << "Warning the following layers properties are not supported:\n";
    for (auto&& layer : unsupportedLayers) {
      std::cout << "---(" << layer << ")\n";
    }
  }

  return validatedLayers;
};

/**
 * TODO: Move this function into an external file.
 *
 * @brief
 * In order to make `vk::InstanceCreateInfo` and `vk::raii::Instance()` work
 * as intended, must first transform the `TDynamicArray<String>` class into a
 * `TDynamicArray<const char*>` to pass in the pointer to pointer required by
 * Vulkan.
 *
 * Use the `TDynamicArray::GetData()` function to get a raw constant pointer
 * to the internal buffer of raw strings (`const char* const*`) required
 * by the `vk::InstanceCreateInfo` struct.
 *
 * This function performs that operation then returns
 * a `vk::raii::Instance` object.
 */
vk::raii::Instance CreateVulkanInstanceObject(TDynamicArray<String>& enabledExtensionNames, TDynamicArray<String>& enabledLayerNames, vk::ApplicationInfo&& appInfo) {
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

  return vk::raii::Instance(VulkanObj::context, createInfo);
};

vk::raii::PhysicalDevice SelectPhysicalDevice() {
  /**
   * vk::PhysicalDeviceProperties struct Documentation:
   * https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceProperties.html
   *
   * Device Limits struct (vk::PhysicalDeviceLimits)
   * https://docs.vulkan.org/spec/latest/chapters/limits.html
   */

  // Get a list of all found GPUs.
  auto physicalDevices = VulkanObj::instance.enumeratePhysicalDevices();

  std::cout << "[VulkanRHI] Selecting GPU:\n";

  // If there are no GPUs... then there is no rendering :(
  if (physicalDevices.empty()) {
    throw std::runtime_error("[VulkanRHI | Error] Failed to find GPUs with Vulkan Support.\n");
  }

  // A multimap of all potential GPUs that can be used.
  // Each PhysicalDevice will be attributed a score depending
  // on their properties and supported features.
  // The one with the highest score will be used for rendering.
  std::multimap<int64, vk::raii::PhysicalDevice> candidates;

  for (const auto& gpu : physicalDevices) {
    auto properties          = gpu.getProperties();
    auto features            = gpu.getFeatures();
    auto queueFamilies       = gpu.getQueueFamilyProperties();
    auto availableExtensions = gpu.enumerateDeviceExtensionProperties();

    std::cout << "  Device: " << properties.deviceName << '\n';

    int64 score = 0;

    // Prioritize discreet GPU (dedicated graphics card) as they offer
    // a peformance advantage.
    if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
      score += 1000;
    }

    score += properties.limits.maxImageDimension2D;

    // If the GPU does not support geometry shaders
    // then the application simply cannot run.
    if (features.geometryShader == false) {
      continue;
    }

    // Verify support for Vulkan 1.4 API.
    // If the API is not supported then the GPU
    // cannot be used.
    bool supportsRequiredVulkanAPIVersion = (properties.apiVersion >= Optim::VK::ApiVersion);

    if (supportsRequiredVulkanAPIVersion == false) {
      continue;
    }

    bool supportsGraphics = std::ranges::any_of(queueFamilies, [](const vk::QueueFamilyProperties& qfp) {
      return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
    });

    if (supportsGraphics == true) {
      std::cout << "    Supports graphics: true\n";
    }
    else {
      std::cout << "    Supports graphics: false\n";
      continue;
    }

    // Check if all required extensions are supported
    // by the GPU.
    bool supportsAllExt = std::ranges::all_of(requiredDeviceExtensions, [&availableExtensions](const auto& requiredExt) {
      return std::ranges::any_of(availableExtensions, [requiredExt](const vk::ExtensionProperties& availableDeviceExt) {
        return strcmp(availableDeviceExt.extensionName, requiredExt);
      });
    });

    if (supportsAllExt == true) {
      std::cout << "    Supports all required extensions: true\n";
    }
    else {
      std::cout << "    Supports all required extensions: false\n";
      continue;
    }

    // Check required features.

    auto testFeatures = gpu.getFeatures2();

    /**
     * TODO:
     * Search more on `vk::raii::PhysicalDevice::getFeatures2`
     */

    auto features2 = gpu.getFeatures2<vk::PhysicalDeviceFeatures2,
                                      vk::PhysicalDeviceVulkan11Features,
                                      vk::PhysicalDeviceVulkan13Features,
                                      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

    bool supportsReqFeatures = features2.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                               features2.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                               features2.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

    if (supportsReqFeatures == false) {
      continue;
    }

    candidates.insert(std::make_pair(score, gpu));
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

vk::raii::Device CreateLogicalDevice() {
  /**
   * Documentation for basic logical device creation:
   *
   * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/04_Logical_device_and_queues.htm
   *
   */
  auto queueFamilyProperties = VulkanObj::physicalDevice.getQueueFamilyProperties();

  // Find queue with graphics capabilities
  auto graphicsQueueFamilyProperty = std::ranges::find_if(queueFamilyProperties, [](auto const& qfp) {
    return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
  });

  float queuePriority = 0.5f;

  auto graphicsIndex = static_cast<uint32>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

  vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
    .queueFamilyIndex = graphicsIndex,
    .queueCount       = 1,
    .pQueuePriorities = &queuePriority
  };

  // vk::PhysicalDeviceFeatures deviceFeatures;
  // // Additional device features enabling
  // vk::StructureChain<vk::PhysicalDeviceFeatures2,
  //                    vk::PhysicalDeviceVulkan11Features,
  //                    vk::PhysicalDeviceVulkan13Features,
  //                    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
  //   featureChaine;
  //   featureChaine.get<vk::PhysicalDeviceVulkan11Features>().setShaderDrawParameters(true);
  //   featureChaine.get<vk::PhysicalDeviceVulkan13Features>().setDynamicRendering(true);
  //   featureChaine.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().setExtendedDynamicState(true);

  // Create a chain of feature structures
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

  return vk::raii::Device(VulkanObj::physicalDevice, deviceCreateInfo);
}

VulkanRHI::VulkanRHI() {
}

VulkanRHI::~VulkanRHI() {
}

void VulkanRHI::Initialize(TDynamicArray<String>& paramSDLExt) {
  std::cout << "[VulkanRHI] Vulkan initialization...\n";
  std::cout << "[VulkanRHI] Vulkan API version min support: " << VK_API_VERSION_1_4 << '\n';

  try {
    /**
     * Verify SDL required extensions first.
     *
     * If they are valid the create a vulkan Instance object.
     *
     * TODO:
     * Consider putting extension validation into a function like
     * `VerifyRequiredLayers()`
     */
    auto extensionProperties = VulkanObj::context.enumerateInstanceExtensionProperties();

    for (auto&& str : paramSDLExt) {
      auto comparaisonFn = [str](vk::ExtensionProperties const& extentionProperty) {
        return strcmp(extentionProperty.extensionName, str.GetPointer()) == 0;
      };

      if (std::ranges::none_of(extensionProperties, comparaisonFn)) {
        std::cout << "[VulkanRHI | Error]-The required SDL extension: (" << str.GetPointer() << ") is not supported by Vulkan.\n";
      }
    }

    // Validate required layers.
    TDynamicArray<String> validatedLayers = VerifyRequiredLayers();

    // Create a vk::raii:instance with the given Extensions, Layers and
    // `vk::ApplicationInfo` struct.
    VulkanObj::instance = CreateVulkanInstanceObject(paramSDLExt, validatedLayers, Optim::VK::GetApplicationInfoStruct());

    // Simple verification to see if the `vk::raii::Instance` object
    // is valid/initialized properly.
    //
    // According to Vulkan documentation, if no `vk::Result::eErrorLayerNotPresent`
    // have been error thrown, then the instanciation is probably successful.
    std::cout << "[VulkanRHI] " << (VulkanObj::instance != nullptr ? "VulkanObj::instance is valid!\n" : "Instance is NOT valid :(\n");
    std::cout << "[VulkanRHI] Vulkan initialized\n";

    // Setup Debug messenger
    if constexpr (Optim::VK::enableValidationLayers) {
      constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

      constexpr vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                                   vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                                   vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

      constexpr vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
        .messageSeverity = severityFlags,
        .messageType     = messageTypeFlags,
        .pfnUserCallback = &Optim::VK::DebugCallback
      };

      VulkanObj::debugMessenger = VulkanObj::instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);

      if (VulkanObj::debugMessenger != nullptr) {
        std::cout << "[VulkanRHI] Debug messenger successfully initialized.\n";
      }
      else {
        std::cout << "[VulkanRHI] Debug messenger FAILED to initialize.\n";
      }
    }

    // Select the physical device
    VulkanObj::physicalDevice = SelectPhysicalDevice();

    if (VulkanObj::physicalDevice != nullptr) {
      std::cout << "[VulkanRHI] GPU Sucessfully selected: " << VulkanObj::physicalDevice.getProperties().deviceName << '\n';
    }
    
    // Create the logical device
    VulkanObj::device = CreateLogicalDevice();

    if (VulkanObj::device != nullptr) {
      std::cout << "[VulkanRHI] Logical Device Sucessfully created " << '\n';
    }
  }
  catch (const vk::SystemError& e) {
    std::cerr << "[Vulkan System Error]\n"
              << e.what() << '\n';
    return;
  }
  catch (const std::exception& e) {
    std::cerr << "[VulkanRHI | Error] " << e.what() << '\n';
    return;
  }
}

void VulkanRHI::Update() {
}
void VulkanRHI::Cleanup() {
}