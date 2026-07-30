/**
 * FILE: VulkanRHI.cpp
 */

#include "VulkanRHI/VulkanRHI.h"

#include "OptVulkanDebug.h"
#include "OptimVKSetup.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <ranges>
#include <vector>

struct VulkanRHI::Impl
{
  vk::raii::Context                context;
  vk::raii::Instance               instance       = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

  RenderDevice renderDevice;
};

// static RenderDevice renderDevice;

// Local namespace
namespace VulkanObj
{

// static vk::raii::Context                context;                  // Context Handle
// static vk::raii::Instance               instance       = nullptr; // VkInstance Handle
// static vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

}; // namespace VulkanObj

static std::vector<char const*> requiredLayers = {};

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
static TDynamicArray<String> VerifyRequiredLayers(const vk::raii::Context& context)
{
  // Create a list of validated and unsupported layers.
  TDynamicArray<String>      validatedLayers;
  TDynamicArray<const char*> unsupportedLayers;

  // Add the validation layers if enabled
  if constexpr (Optim::VK::enableValidationLayers) {
    requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
  }

  // Get list of all supported layers by Vulkan implementation.
  auto vkLayerProperties = context.enumerateInstanceLayerProperties();

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
vk::raii::Instance CreateVulkanInstanceObject(
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
};

VulkanRHI::VulkanRHI() : impl(MakeUnique<Impl>())
{
  if (!impl.IsValid()) {
    std::cout << "Impl is nullptr,\n";
  }
}

VulkanRHI::~VulkanRHI()
{
}

void VulkanRHI::Initialize(TDynamicArray<String>& paramSDLExt)
{
  // std::cout << "[VulkanRHI] Vulkan initialization...\n";
  // std::cout << "[VulkanRHI] Vulkan API version min support: " << VK_API_VERSION_1_4 << '\n';

  try {
    // Verify SDL required extensions first.
    // impl.Get().context.enumerateInstanceExtensionProperties();

    auto extensionProperties = impl.Get().context.enumerateInstanceExtensionProperties();
    // auto extensionProperties = VulkanObj::context.enumerateInstanceExtensionProperties();

    for (auto&& str : paramSDLExt) {
      auto comparaisonFn = [str](vk::ExtensionProperties const& extentionProperty) {
        return strcmp(extentionProperty.extensionName, str.GetPointer()) == 0;
      };

      if (std::ranges::none_of(extensionProperties, comparaisonFn)) {
        std::cout << "[VulkanRHI | Error]-The required SDL extension: (" << str.GetPointer() << ") is not supported by Vulkan.\n";
      }
    }

    // Validate required layers.
    // TDynamicArray<String> validatedLayers = VerifyRequiredLayers();
    TDynamicArray<String> validatedLayers = VerifyRequiredLayers(impl.Get().context);

    // Create a vk::raii:instance with the given Extensions, Layers and
    // `vk::ApplicationInfo` struct.
    impl.Get().instance = CreateVulkanInstanceObject(impl.Get().context, paramSDLExt, validatedLayers, Optim::VK::GetApplicationInfoStruct());

    // Simple verification to see if the `vk::raii::Instance` object
    // is valid/initialized properly.
    //
    // According to Vulkan documentation, if no `vk::Result::eErrorLayerNotPresent`
    // have been error thrown, then the instanciation is probably successful.

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

      // VulkanObj::debugMessenger = VulkanObj::instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
      impl.Get().debugMessenger = impl.Get().instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);

      if (impl.Get().debugMessenger == nullptr) {
        throw std::runtime_error("[VulkanRHI | Error] Debug messenger FAILED to initialize.");
      }
    }

    // Get a handle to the most optimal physical device to use for rendering
    auto physicalDevice = Optim::VK::SelectPhysicalDevice(impl.Get().instance);

    if (physicalDevice == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to find usable GPU for rendering.");
    }

    // Initialize the RenderDevice object
    impl.Get().renderDevice = Optim::VK::CreateDeviceContext(physicalDevice);

    impl.Get().renderDevice.Validate();
  }
  catch (const vk::SystemError& e) {
    std::cerr << "[Vulkan System Error] " << e.what() << '\n';
    return;
  }
  catch (const std::exception& e) {
    std::cerr << "[VulkanRHI | Error] " << e.what() << '\n';
    return;
  }
}

void VulkanRHI::Update()
{
}
void VulkanRHI::Cleanup()
{
}