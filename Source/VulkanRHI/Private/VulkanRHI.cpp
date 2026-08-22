/**
 * FILE: VulkanRHI.cpp
 */

#include "VulkanRHI/VulkanRHI.h"

#include "OpVkCommon/Minimal.h"
#include "OpVkCore/OpVkCore.h"
#include "OpVkDebug/Debug.h"
#include "OpVkTypes/CommandContext.h"
#include "OpVkTypes/RenderContext.h"
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
  vk::raii::Context                context;
  vk::raii::Instance               instance       = nullptr;
  vk::raii::SurfaceKHR             surface        = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

  RenderContext    renderContext;
  SwapChainContext swapChainContext;

  vk::raii::Pipeline      pipeline      = nullptr;
  vk::raii::CommandPool   commandPool   = nullptr;
  vk::raii::CommandBuffer commandBuffer = nullptr;

  CommandContext commandContext;
};

VulkanRHI::VulkanRHI() : pVkContext(MakeUnique<VulkanContext>())
{
}

VulkanRHI::~VulkanRHI()
{}

/**
 * @brief
 * Initializes the VulkanRHI object and all required Vulkan objects.
 */
void VulkanRHI::Initialize(void* param_pSDLWindow)
{
  try {
    auto&       ctx       = *pVkContext;
    SDL_Window* sdlwindow = static_cast<SDL_Window*>(param_pSDLWindow);

    // Create VkInstance object
    ctx.instance = Optim::VK::CreateVkInstance(ctx.context);
    if (ctx.instance == nullptr) {
      throw std::runtime_error("[VulkanRHI] Error - VkInstance Initalization Status : FAILURE");
    }

    // Create VkDebugUtilsMessengerEXT object if validation layers are enabled
    if constexpr (Optim::VK::enableValidationLayers) {
      ctx.debugMessenger = Optim::VK::Debug::CreateVkDebugUtilsMessengerEXT(ctx.instance);
      if (ctx.debugMessenger == nullptr) {
        throw std::runtime_error("[VulkanRHI] Error - VkDebugUtilsMessengerEXT Initalization Status : FAILURE");
      }
    }

    // Create VKSurfaceKHR object
    ctx.surface = Optim::VK::CreateVkSurfaceKHR(ctx.instance, sdlwindow);
    if (ctx.surface == nullptr) {
      throw std::runtime_error("[VulkanRHI] Error - VkSurfaceKHR Initalization Status : FAILURE");
    }

    // Create RenderContext object
    ctx.renderContext = RenderContext(ctx.instance, ctx.surface);
    if (!ctx.renderContext.IsValid()) {
      throw std::runtime_error("[VulkanRHI] Error - RenderContext Initalization Status : FAILURE");
    }

    // Create the initial SwapChainContext
    ctx.swapChainContext = SwapChainContext(ctx.renderContext.device, ctx.renderContext.physicalDevice, ctx.surface, sdlwindow);
    if (!ctx.swapChainContext.IsValid()) {
      throw std::runtime_error("[VulkanRHI] Error - SwapChainContext Initalization Status : FAILURE");
    }

    // Pipeline creation
    //  * Load Shader
    //  * Create `VkShaderModule`
    //  * Crate Shader Stages Info
    //  * Use Data to create a `VkPipeline` Object

    auto shadercode       = Optim::VKPipeline::LoadCompiledShader("Shaders/sample.spv");
    auto ShaderModule     = Optim::VKPipeline::CreateVkShaderModule(shadercode, ctx.renderContext.device);
    auto shaderStagesInfo = Optim::VKPipeline::CreateVkPipelineShaderStageCreateInfoList(ShaderModule);

    ctx.pipeline = Optim::VKPipeline::CreateVulkanPipeline(ctx.swapChainContext, shaderStagesInfo, ctx.renderContext.device);
    if (ctx.pipeline == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to create VkPipeline object.");
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

void VulkanRHI::DrawFrame()
{
}
void VulkanRHI::Cleanup()
{
}