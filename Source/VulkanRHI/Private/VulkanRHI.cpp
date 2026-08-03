/**
 * FILE: VulkanRHI.cpp
 */

#include "VulkanRHI/VulkanRHI.h"

#include "OpVkCommon/Minimal.h"
#include "OpVkCore/OpVkCore.h"
#include "OpVkDebug/Debug.h"
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

/**
 * @brief 
 * Structure to hold all Vulkan context objects.
 */
struct VulkanRHI::VulkanContext
{
  vk::raii::Context                context;
  vk::raii::Instance               instance       = nullptr;
  vk::raii::SurfaceKHR             surface        = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
  RenderDevice                     renderDevice;
  SwapChainContext                 swapChainContext;
};

VulkanRHI::VulkanRHI() : pVkContext(MakeUnique<VulkanContext>())
{}

VulkanRHI::~VulkanRHI()
{}


/**
 * @brief
 * Initializes the VulkanRHI object and all required Vulkan objects.
 */
void VulkanRHI::Initialize(void* param_pSDLWindow)
{
  try {
    auto& ctx = *pVkContext;

    SDL_Window* sdlwindow = static_cast<SDL_Window*>(param_pSDLWindow);

    TDynamicArray<String> reqInstanceExt;

    // Add all extensions required by SDL to create a VkSurfaceKHR object.
    Optim::VK::AddSDLRequiredExtensions(reqInstanceExt);

    // Add "VK_EXT_debug_utils" extension if validation layers are enabled
    if constexpr (Optim::VK::enableValidationLayers) {
      reqInstanceExt.EmplaceBack(vk::EXTDebugUtilsExtensionName);
    }

    if (!Optim::VK::ValidateRequiredExtensions(reqInstanceExt, ctx.context)) {
      throw std::runtime_error("[VulkanRHI | Error] Required extensions are not supported by Vulkan.");
    }

    // Required Layers Verification
    TDynamicArray<String> requiredLayers;

    // Add "VK_LAYER_KHRONOS_validation" if validation layers are enabled
    if constexpr (Optim::VK::enableValidationLayers) {
      requiredLayers.EmplaceBack("VK_LAYER_KHRONOS_validation");
    }

    if (Optim::VK::ValidateRequiredLayers(requiredLayers, ctx.context) == false) {
      throw std::runtime_error("[VulkanRHI | Error] Required layers are not supported by Vulkan.");
    }

    // Create VkInstance object
    ctx.instance = Optim::VK::CreateVkInstance(ctx.context, reqInstanceExt, requiredLayers, Optim::VK::GetApplicationInfoStruct());
    if (ctx.instance == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to initialize VkInstance object.");
    }

    // Create VkDebugUtilsMessengerEXT object if validation layers are enabled
    if constexpr (Optim::VK::enableValidationLayers) {
      ctx.debugMessenger = Optim::VK::Debug::CreateVkDebugUtilsMessengerEXT(ctx.instance);
      if (ctx.debugMessenger == nullptr) {
        throw std::runtime_error("[VulkanRHI | Error] Debug messenger FAILED to initialize.");
      }
    }

    // Create VKSurfaceKHR object
    ctx.surface = Optim::VK::CreateVkSurfaceKHR(ctx.instance, sdlwindow);
    if (ctx.surface == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to initialize VkSurfaceKHR object.");
    }

    // Select most optimal VkPhysicalDevice object
    auto physicalDevice = Optim::VK::SelectVkPhysicalDevice(ctx.instance, ctx.surface);
    if (physicalDevice == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to find usable GPU for rendering.");
    }

    // Create RenderDevice object
    ctx.renderDevice = Optim::VK::CreateRenderDevice(physicalDevice, ctx.surface);
    if (!ctx.renderDevice.IsValid()) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to create RenderDevice object.");
    }

    // Create SwapChainContext object
    ctx.swapChainContext = Optim::VK::CreateSwapChainContext(ctx.renderDevice.logicalDevice, ctx.renderDevice.physicalDevice, ctx.surface, sdlwindow);
    if (!ctx.swapChainContext.IsValid()) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to create swap chain context.");
    }
  }
  catch (const vk::SystemError& e) {
    std::cerr << "[Vulkan | Error] " << e.what() << '\n';
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