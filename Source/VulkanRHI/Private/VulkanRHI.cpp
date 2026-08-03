/**
 * FILE: VulkanRHI.cpp
 */

#include "VulkanRHI/VulkanRHI.h"

#include "OpVkCommon/Minimal.h"
#include "OpVkDebug/Debug.h"
#include "OpVkCore/OpVkCore.h"
#include "OpVkTypes/RenderDevice.h"
#include "OpVkTypes/SwapChainContext.h"

// SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

// STD
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <ranges>
#include <vector>

struct VulkanRHI::VulkanContext
{
  vk::raii::Context    context;
  vk::raii::Instance   instance = nullptr;
  vk::raii::SurfaceKHR surface  = nullptr;

  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

  RenderDevice renderDevice;

  vk::raii::SwapchainKHR swapChain = nullptr;
  std::vector<vk::Image> swapChainImages;
};

VulkanRHI::VulkanRHI() : pVkContext(MakeUnique<VulkanContext>())
{}

VulkanRHI::~VulkanRHI()
{}

/**
 * Start by verifying required extensions. For now since only the SDL
 * required are used, the list containing them is being directly verified.
 *
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
 * `Optim::VK::CreateVkInstance()` function is being called to
 * finally create a vk::Instance object.
 *
 * With the instance being created and validated, this function can now start
 * checking for a GPU (vk::PhysicalDevice) to be used.
 *
 * First call the `Optim::VK::SelectVkPhysicalDevice` function to get the most
 * appropriate GPU to use for rendering. The prerequestists for the physical
 * device selection are all defined inside the function. All properties,
 * extensions, queue families being used as prerequisits for GPU selection,
 * must ALL be supported, if nothing is found then the function returns
 * nullptr, then we throw an error.
 *
 * Once that is confirmed we finally call `Optim::VK::CreateRenderDevice`
 * which will create the actual logical device object (vk::Device).
 */
void VulkanRHI::Initialize(void* param_pSDLWindow)
{
  try {
    auto& ctx = *pVkContext;

    SDL_Window* sdlwindow = static_cast<SDL_Window*>(param_pSDLWindow);

    TDynamicArray<String> reqInstanceExt;

    // Get list of all extensions required by SDL to create an vkInstance
    // object, then add them to the required extensions list.
    uint32             vkInstanceExtCount = 0;
    const char* const* ppVkInstanceExt    = SDL_Vulkan_GetInstanceExtensions(&vkInstanceExtCount);

    if (ppVkInstanceExt == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }

    for (size_t i = 0; i < vkInstanceExtCount; i++) {
      reqInstanceExt.EmplaceBack(ppVkInstanceExt[i]);
    }

    if constexpr (Optim::VK::enableValidationLayers) {
      reqInstanceExt.EmplaceBack(vk::EXTDebugUtilsExtensionName);
    }

    // Get list of all supported extension by current Vulkan API.
    auto extensionProperties = ctx.context.enumerateInstanceExtensionProperties();

    for (auto&& str : reqInstanceExt) {
      auto comparaisonFn = [str](vk::ExtensionProperties const& extentionProperty) {
        return strcmp(extentionProperty.extensionName, str.GetPointer()) == 0;
      };
      if (std::ranges::none_of(extensionProperties, comparaisonFn)) {
        std::cout << "[VulkanRHI | Error]-The required SDL extension: (" << str.GetPointer() << ") is not supported by Vulkan.\n";
      }
    }

    // Required Layers Verification
    TDynamicArray<String> requiredLayers;

    // Add "VK_LAYER_KHRONOS_validation" if validation layers are enabled
    if constexpr (Optim::VK::enableValidationLayers) {
      requiredLayers.EmplaceBack("VK_LAYER_KHRONOS_validation");
    }

    // Validate required layers.
    TDynamicArray<String> unsupportedLayers = Optim::VK::VerifyRequiredLayers(requiredLayers, ctx.context);

    if (unsupportedLayers.GetCount() > 0) {
      String errorMsg = String("[VulkanRHI | Error] The following layers are not supported by Vulkan:\n");
      for (auto& layer : unsupportedLayers) {
        errorMsg.Append(" * ").Append(layer);
      }
      throw std::runtime_error(errorMsg.GetPointer());
    }

    // Create VkInstance object
    ctx.instance = Optim::VK::CreateVkInstance(pVkContext.GetRef().context, reqInstanceExt, requiredLayers, Optim::VK::GetApplicationInfoStruct());
    if (pVkContext.GetRef().instance == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to initialize VkInstance object.");
    }

    // Create VkDebugUtilsMessengerEXT object if validation layers are enabled
    if constexpr (Optim::VK::enableValidationLayers) {
      ctx.debugMessenger = Optim::VK::Debug::CreateVkDebugUtilsMessengerEXT(pVkContext.GetRef().instance);
      if (pVkContext.GetRef().debugMessenger == nullptr) {
        throw std::runtime_error("[VulkanRHI | Error] Debug messenger FAILED to initialize.");
      }
    }

    // Create VKSurfaceKHR object
    ctx.surface = Optim::VK::CreateVkSurfaceKHR(pVkContext.GetRef().instance, sdlwindow);
    if (pVkContext.GetRef().surface == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to initialize VkSurfaceKHR object.");
    }

    // Select most optimal VkPhysicalDevice object
    auto physicalDevice = Optim::VK::SelectVkPhysicalDevice(pVkContext.GetRef().instance, ctx.surface);
    if (physicalDevice == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to find usable GPU for rendering.");
    }

    // Create RenderDevice object
    ctx.renderDevice = Optim::VK::CreateRenderDevice(physicalDevice, ctx.surface);

    if (pVkContext.GetRef().renderDevice.physicalDevice == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] vk::PhyscialDevice object is null.");
    }
    if (pVkContext.GetRef().renderDevice.logicalDevice == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] vk::Device object is null.");
    }
    if (pVkContext.GetRef().renderDevice.graphicsQueue == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] vk::Queue object is null.");
    }

    // Swap chain creation
    ctx.swapChain = Optim::VK::CreateVkSwapChainKHR(ctx.renderDevice.logicalDevice, ctx.renderDevice.physicalDevice, ctx.surface, sdlwindow);

    if (pVkContext.GetRef().swapChain == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to create VkSwapChainKHR object.");
    }

    // Get swap chain images
    ctx.swapChainImages = ctx.swapChain.getImages();
    if (ctx.swapChainImages.empty()) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to get swap chain images.");
    }

    // Check if the swap chain images are valid
    for (const auto& image : ctx.swapChainImages) {
      if (image == nullptr) {
        throw std::runtime_error("[VulkanRHI | Error] Swap chain image is null.");
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