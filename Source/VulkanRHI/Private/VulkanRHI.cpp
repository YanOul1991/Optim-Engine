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

  vk::raii::Pipeline pipeline = nullptr;
  CommandContext     commandContext;
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
    
    ctx.commandContext = CommandContext(ctx.renderContext.device, ctx.renderContext);
    if (!ctx.commandContext.IsValid()) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to create CommandContext object.");
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

/**
 * Vulkan Tutorial basic demo for rendering and presentations:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/02_Rendering_and_presentation.html
 */
void VulkanRHI::DrawFrame()
{
  auto& ctx = this->pVkContext.GetRef();

  // Verify that the previous frame has successfully finished rendering.
  auto fenceResult = ctx.renderContext.device.waitForFences(*ctx.commandContext.drawFence, vk::True, UINT64_MAX);
  if (fenceResult != vk::Result::eSuccess) {
    throw std::runtime_error("[VulkanRHI] - Error: Failed to wait for VkFence");
  }
  ctx.renderContext.device.resetFences(*ctx.commandContext.drawFence);

  // Grab an image from the frambuffer, only once the presentation of the
  // previous frame is finished, use the dedicated semaphore for that purpose.
  auto [result, imageIndex] = ctx.swapChainContext.swapChain.acquireNextImage(UINT64_MAX, *ctx.commandContext.presentCompleteSemaphore, nullptr);

  // Start recording the command buffer.
  ctx.commandContext.RecordCommandBuffer(ctx.swapChainContext, imageIndex, ctx.pipeline);

  // Specify which stages of the pipeline to wait for when submiting the wait
  // semaphore. Here we wait for writing colors.
  vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

  const vk::SubmitInfo submitInfo = {
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = &*ctx.commandContext.presentCompleteSemaphore,
    .pWaitDstStageMask    = &waitDestinationStageMask,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &*ctx.commandContext.commandBuffer,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = &*ctx.commandContext.renderFinishedSemaphore
  };

  // Submit the command buffer to the queue. Signal the draw fence when the
  // command buffer finishes executing.
  ctx.renderContext.queue.submit(submitInfo, ctx.commandContext.drawFence);

  // Present the result to the swap chain

  const vk::PresentInfoKHR presentInfo = {
    .waitSemaphoreCount = 1,
    .pWaitSemaphores    = &*ctx.commandContext.renderFinishedSemaphore,
    .swapchainCount     = 1,
    .pSwapchains        = &*ctx.swapChainContext.swapChain,
    .pImageIndices      = &imageIndex,
    .pResults           = nullptr
  };

  result = ctx.renderContext.queue.presentKHR(presentInfo);

  ctx.renderContext.device.waitIdle();
}

void VulkanRHI::Cleanup()
{
}