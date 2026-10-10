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
  static constexpr int32 MAX_FRAMES_IN_FLIGHT = 2;

  vk::raii::Context                context;
  vk::raii::Instance               instance       = nullptr;
  vk::raii::SurfaceKHR             surface        = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

  RenderContext renderContext;

  SwapChainContext swapChainContext;

  vk::raii::Pipeline pipeline = nullptr;

  std::vector<FrameData>           frameResources;
  std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
  std::vector<vk::raii::Fence>     framesInFlightFences;

  int32 frameIndex = 0;
  /**
   * @brief
   * Initialized all Vulkan objects for this structure.
   */
  FORCEINLINE void Initialize(SDL_Window* pSdlWindow)
  {
    try {

      // Create VkInstance object
      instance = Optim::VK::CreateVkInstance(context);
      if (instance == nullptr) {
        throw std::runtime_error("[VulkanRHI] Error - VkInstance Initalization Status : FAILURE");
      }

      // Create VkDebugUtilsMessengerEXT object if validation layers are enabled
      if constexpr (Optim::VK::enableValidationLayers) {
        debugMessenger = Optim::VK::Debug::CreateVkDebugUtilsMessengerEXT(instance);
        if (debugMessenger == nullptr) {
          throw std::runtime_error("[VulkanRHI] Error - VkDebugUtilsMessengerEXT Initalization Status : FAILURE");
        }
      }

      // Create VKSurfaceKHR object
      surface = Optim::VK::CreateVkSurfaceKHR(instance, pSdlWindow);
      if (surface == nullptr) {
        throw std::runtime_error("[VulkanRHI] Error - VkSurfaceKHR Initalization Status : FAILURE");
      }

      // Create RenderContext object
      renderContext = RenderContext(instance, surface);
      if (!renderContext.IsValid()) {
        throw std::runtime_error("[VulkanRHI] Error - RenderContext Initalization Status : FAILURE");
      }

      // Create the initial SwapChainContext
      swapChainContext = SwapChainContext(renderContext.device, renderContext.physicalDevice, surface, pSdlWindow);
      if (!swapChainContext.IsValid()) {
        throw std::runtime_error("[VulkanRHI] Error - SwapChainContext Initalization Status : FAILURE");
      }

      // Pipeline creation
      //  * Load Shader
      //  * Create `VkShaderModule`
      //  * Crate Shader Stages Info
      //  * Use Data to create a `VkPipeline` Object
      auto shadercode       = Optim::VKPipeline::LoadCompiledShader("Shaders/sample.spv");
      auto ShaderModule     = Optim::VKPipeline::CreateVkShaderModule(shadercode, renderContext.device);
      auto shaderStagesInfo = Optim::VKPipeline::CreateVkPipelineShaderStageCreateInfoList(ShaderModule);

      pipeline = Optim::VKPipeline::CreateVulkanPipeline(swapChainContext, shaderStagesInfo, renderContext.device);
      if (pipeline == nullptr) {
        throw std::runtime_error("[VulkanRHI | Error] Failed to create VkPipeline object.");
      }

      // Create Sync objects
      for (int i = 0; i < swapChainContext.swapChainImages.size(); i++) {
        renderFinishedSemaphores.emplace_back(renderContext.device, vk::SemaphoreCreateInfo());
      }

      for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        frameResources.emplace_back(renderContext.device, renderContext);
        presentCompleteSemaphores.emplace_back(renderContext.device, vk::SemaphoreCreateInfo());
        framesInFlightFences.emplace_back(renderContext.device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
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
   * @brief
   * Draws the next frame.
   */
  FORCEINLINE void DrawFrame()
  {
    // Wait for CPU-GPU sync on the current frame slot
    auto fenceResult = renderContext.device.waitForFences(*framesInFlightFences[frameIndex], vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
      throw std::runtime_error("[VulkanRHI] Error: Failed to wait for drawFence");
    }

    renderContext.device.resetFences(*framesInFlightFences[frameIndex]);

    // Acquire next swapchain image
    auto [result, imageIndex] = swapChainContext.swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);

    // Record commands
    frameResources[frameIndex].commandBuffer.reset();
    frameResources[frameIndex].RecordCommandBuffer(swapChainContext, imageIndex, pipeline);

    // Submit to GPU
    vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    const vk::SubmitInfo submitInfo = {
      .waitSemaphoreCount   = 1,
      .pWaitSemaphores      = &(*presentCompleteSemaphores[frameIndex]),
      .pWaitDstStageMask    = &waitDstStageMask,
      .commandBufferCount   = 1,
      .pCommandBuffers      = &(*frameResources[frameIndex].commandBuffer),
      .signalSemaphoreCount = 1,
      .pSignalSemaphores    = &(*renderFinishedSemaphores[imageIndex])
    };

    renderContext.queue.submit(submitInfo, *framesInFlightFences[frameIndex]);

    // Present result
    const vk::PresentInfoKHR presentInfo = {
      .waitSemaphoreCount = 1,
      .pWaitSemaphores    = &(*renderFinishedSemaphores[imageIndex]),
      .swapchainCount     = 1,
      .pSwapchains        = &(*swapChainContext.swapChain),
      .pImageIndices      = &imageIndex
    };

    result = renderContext.queue.presentKHR(presentInfo);

    // Advance frame index
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  FORCEINLINE void Cleanup()
  {
    renderContext.device.waitIdle();
  }
};

VulkanRHI::VulkanRHI() : context(std::make_unique<VulkanRHI::VulkanContext>())
{}

VulkanRHI::~VulkanRHI()
{}

/**
 * @brief
 * Initializes the VulkanRHI object and all required Vulkan objects.
 */
void VulkanRHI::Initialize(void* pSdlWindow)
{
  context->Initialize(static_cast<SDL_Window*>(pSdlWindow));
}

void VulkanRHI::DrawFrame()
{
  context->DrawFrame();
}

void VulkanRHI::Cleanup()
{
  context->Cleanup();
}