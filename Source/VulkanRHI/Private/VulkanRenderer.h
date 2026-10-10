#pragma once

#include "Core/CoreMinimal.h"
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

class Renderer final
{
 public:
  static constexpr inline int32 MAX_FRAMES_IN_FLIGHT = 2;

  vk::raii::Context                context;
  vk::raii::Instance               instance       = nullptr;
  vk::raii::SurfaceKHR             surface        = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
  RenderContext                    renderContext;
  SwapChainResources               swapChain;
  vk::raii::Pipeline               pipeline = nullptr;
  std::vector<FrameResource>       frames;
  size_t                           frameIndex;

  /**
   * MIGHT THROW
   */
  FORCEINLINE void Initialize(SDL_Window* pSDLWindow)
  {

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
    surface = Optim::VK::CreateVkSurfaceKHR(instance, pSDLWindow);
    if (surface == nullptr) {
      throw std::runtime_error("[VulkanRHI] Error - VkSurfaceKHR Initalization Status : FAILURE");
    }

    // Create RenderContext object
    renderContext = RenderContext(instance, surface);
    if (!renderContext.IsValid()) {
      throw std::runtime_error("[VulkanRHI] Error - RenderContext Initalization Status : FAILURE");
    }

    // Create the initial SwapChainResources
    swapChain = SwapChainResources(renderContext.device, renderContext.physicalDevice, surface, pSDLWindow);
    if (!swapChain.IsValid()) {
      throw std::runtime_error("[VulkanRHI] Error - SwapChainResources Initalization Status : FAILURE");
    }

    // Pipeline creation
    //  * Load Shader
    //  * Create `VkShaderModule`
    //  * Crate Shader Stages Info
    //  * Use Data to create a `VkPipeline` Object
    auto shadercode       = Optim::VKPipeline::LoadCompiledShader("Shaders/sample.spv");
    auto ShaderModule     = Optim::VKPipeline::CreateVkShaderModule(shadercode, renderContext.device);
    auto shaderStagesInfo = Optim::VKPipeline::CreateVkPipelineShaderStageCreateInfoList(ShaderModule);

    pipeline = Optim::VKPipeline::CreateVulkanPipeline(swapChain, shaderStagesInfo, renderContext.device);
    if (pipeline == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to create VkPipeline object.");
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      frames.emplace_back(renderContext.device, renderContext);
    }
  }

  FORCEINLINE void DrawFrame()
  {
    auto& currentFrame = frames[frameIndex];

    /**
     * Wait for frame slot to be signaled and free to be used. This fence gets
     * signaled when the last frame commands have finished being computed by
     * the GPU queue.
     */
    auto fenceResult = renderContext.device.waitForFences(*currentFrame.inFlightFence, vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
      throw std::runtime_error("[VulkanRHI] Error: Failed to wait for inFlightFence");
    }
    renderContext.device.resetFences(*currentFrame.inFlightFence);
    
    /**
     * Aquire a new image index from the swap chain. Signal the presentCompleteSemaphore
     * of the active frame resource.
     */
    auto [result, imageIndex] = swapChain.swapChain.acquireNextImage(UINT64_MAX, *currentFrame.presentCompleteSemaphore, nullptr);

    // Reset command buffer then start recodring commands for next frame.
    currentFrame.commandBuffer.reset();
    currentFrame.RecordCommandBuffer(swapChain, imageIndex, pipeline);

    // Only wait for the presentCompleteSemaphore during the color attachement
    // output stage.
    vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    /**
     * Commands submit:
     * When submitting wait for the last inf lsight frame to have finished
     * presenting before starting to record into the swap chain image.
     *
     * When the queue finishes, signal for the rendering finished semaphore
     * of the acquired image in the swap chain, not on the frame in flight.
     *
     * Additionaly, signal for the frame's in flight fence, so that the next
     * CPU cycle can already start recording the next frame's command without
     * waiting for the presentation to be finished, as the current frame
     * already waits for the last one's present operation to be completed when
     * submitting commands one the GPU queue.
     */
    const vk::SubmitInfo submitInfo = {
      .waitSemaphoreCount   = 1,
      .pWaitSemaphores      = &(*currentFrame.presentCompleteSemaphore),
      .pWaitDstStageMask    = &waitDstStageMask,
      .commandBufferCount   = 1,
      .pCommandBuffers      = &(*currentFrame.commandBuffer),
      .signalSemaphoreCount = 1,
      .pSignalSemaphores    = &(*swapChain.renderFinishedSemaphores[imageIndex])
    };
    renderContext.queue.submit(submitInfo, *currentFrame.inFlightFence);

    /**
     * Presentation:
     *
     * Tell queue to present the buffer content of the acquired image index in
     * this frame.
     *
     * And wait for the swap chain image's render finished semaphore's to be
     * signaled before starting presentation operations.
     */
    const vk::PresentInfoKHR presentInfo = {
      .waitSemaphoreCount = 1,
      .pWaitSemaphores    = &(*swapChain.renderFinishedSemaphores[imageIndex]),
      .swapchainCount     = 1,
      .pSwapchains        = &(*swapChain.swapChain),
      .pImageIndices      = &imageIndex
    };

    result = renderContext.queue.presentKHR(presentInfo);

    // Advance frame index
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  FORCEINLINE void WaitForDeviceIdle()
  {
    renderContext.device.waitIdle();
  }
};