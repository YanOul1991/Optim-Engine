/**
 * FILE: VulkanRHI.cpp
 */

#include "VulkanRHI/VulkanRHI.h"

#include "OptimVKDebug.h"
#include "OptimVKSetup.h"

// SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

// STD
#include <algorithm>
#include <iostream>
#include <map>
#include <ranges>
#include <vector>

struct VulkanRHI::Impl
{
  vk::raii::Context                context;
  vk::raii::Instance               instance       = nullptr;
  vk::raii::SurfaceKHR             surface        = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

  RenderDevice renderDevice;
};

VulkanRHI::VulkanRHI() : impl(MakeUnique<Impl>())
{}

VulkanRHI::~VulkanRHI()
{}

/**
 * Start by verifying required extensions. For now since only the SDL
 * required are used, the list containing them is being directly verified.
 *
 * @todo
 * When more extensions will be need then the required extensions verification
 * logic will be updated accordingly.
 *
 * Make a list of string containing all required layers to be used which will
 * be passed as an argument for the `VerifyRequiredLayers` function. It will
 * return a list containing all the unsupported layers.
 *
 * If it is empty, everything is well. Else throw a runtime error listing
 * all the unsupported layers.
 *
 * Once the extensions and layers have been validated, the
 * `Optim::VK::CreateVulkanInstanceObject()` function is being called to
 * finally create a vk::Instance object.
 *
 * With the instance being created and validated, this function can now start
 * checking for a GPU (vk::PhysicalDevice) to be used.
 *
 * First call the `Optim::VK::SelectPhysicalDevice` function to get the most
 * appropriate GPU to use for rendering. The prerequestists for the physical
 * device selection are all defined inside the function. All properties,
 * extensions, queue families being used as prerequisits for GPU selection,
 * must ALL be supported, if nothing is found then the function returns
 * nullptr, then we throw an error.
 *
 * Once that is confirmed we finally call `Optim::VK::CreateDeviceContext`
 * which will create the actual logical device object (vk::Device).
 */
void VulkanRHI::Initialize(void* pWindow)
{
  try {
    SDL_Window* targetWindow = static_cast<SDL_Window*>(pWindow);

    /**
     * REQUIRED EXTENSIONS VERIFICATION
     */
    // Get list of all extensions required by SDL to create an vkInstance object.
    uint32             vkInstanceExtCount = 0;
    const char* const* ppVkInstanceExt    = SDL_Vulkan_GetInstanceExtensions(&vkInstanceExtCount);

    TDynamicArray<String> reqInstanceExt;

    if (ppVkInstanceExt) {
      for (size_t i = 0; i < vkInstanceExtCount; i++) {
        reqInstanceExt.EmplaceBack(ppVkInstanceExt[i]);
      }
    }

    // Get list of all supported extension by current Vulkan API.
    auto extensionProperties = impl.Get().context.enumerateInstanceExtensionProperties();

    for (auto&& str : reqInstanceExt) {
      auto comparaisonFn = [str](vk::ExtensionProperties const& extentionProperty) {
        return strcmp(extentionProperty.extensionName, str.GetPointer()) == 0;
      };

      if (std::ranges::none_of(extensionProperties, comparaisonFn)) {
        std::cout << "[VulkanRHI | Error]-The required SDL extension: (" << str.GetPointer() << ") is not supported by Vulkan.\n";
      }
    }

    /**
     * REQUIRED LAYERS VERIFICATION
     */

    TDynamicArray<String> requiredLayers;

    if constexpr (Optim::VK::enableValidationLayers) {
      requiredLayers.EmplaceBack("VK_LAYER_KHRONOS_validation");
    }

    // Validate required layers.
    TDynamicArray<String> unsupportedLayers = Optim::VK::VerifyRequiredLayers(requiredLayers, impl.Get().context);

    if (unsupportedLayers.GetCount() > 0) {
      String errorMsg = String("[VulkanRHI | Error] The following layers are not supported by Vulkan:\n");

      for (auto& layer : unsupportedLayers) {
        errorMsg.Append(" * ").Append(layer);
      }

      throw std::runtime_error(errorMsg.GetPointer());
    }

    // Initialize vkInstance object
    impl.Get().instance = Optim::VK::CreateVulkanInstanceObject(impl.Get().context, reqInstanceExt, requiredLayers, Optim::VK::GetApplicationInfoStruct());

    VkSurfaceKHR rawSurface = VK_NULL_HANDLE;

    if (!SDL_Vulkan_CreateSurface(targetWindow, *impl.Get().instance, nullptr, &rawSurface)) {
      throw std::runtime_error(SDL_GetError());
    }

    impl.Get().surface = vk::raii::SurfaceKHR(impl.Get().instance, rawSurface);

    if (impl.Get().surface == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to initialize VkSurfaceKHR object.");
    }

    // PHYSCIAL DEVICE SELECTION AND LOGICAL DEVICE INITIALIZATION

    // Get a handle to the most optimal physical device to use for rendering
    auto physicalDevice = Optim::VK::SelectPhysicalDevice(impl.Get().instance, impl.Get().surface);

    if (physicalDevice == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to find usable GPU for rendering.");
    }

    // Initialize the RenderDevice object
    impl.Get().renderDevice = Optim::VK::CreateDeviceContext(physicalDevice, impl.Get().surface);

    // Verify RenderDevice handles.
    if (impl.Get().renderDevice.physicalDevice == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] vk::PhyscialDevice object is null.");
    }
    if (impl.Get().renderDevice.logicalDevice == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] vk::Device object is null.");
    }
    if (impl.Get().renderDevice.graphicsQueue == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] vk::Queue object is null.");
    }

    // Setup Debug messenger
    if constexpr (Optim::VK::enableValidationLayers) {
      impl.Get().debugMessenger = Optim::VK::Debug::CreateDebugMessenger(impl.Get().instance);

      if (impl.Get().debugMessenger == nullptr) {
        throw std::runtime_error("[VulkanRHI | Error] Debug messenger FAILED to initialize.");
      }
    }
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