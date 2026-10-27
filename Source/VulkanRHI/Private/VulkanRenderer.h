#pragma once

#include "Core/CoreMinimal.h"
#include "OpVkCommon/Minimal.h"
#include "OpVkCore/OpVkCore.h"
#include "OpVkDebug/Debug.h"
#include "OpVkTypes/Buffers.h"
#include "OpVkTypes/CommandContext.h"
#include "OpVkTypes/RenderContext.h"
#include "OpVkTypes/SwapChainContext.h"

// SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

// STD
#include <algorithm>
#include <cstdint>
#include <glm/glm.hpp>
#include <iostream>
#include <limits>
#include <map>
#include <ranges>
#include <vector>

using namespace Optim;

class Renderer final
{
 public:
  static constexpr inline int32 MAX_FRAMES_IN_FLIGHT = 2;

  SDL_Window*                      pWindow = nullptr;

  vk::raii::Context                context;
  vk::raii::Instance               instance       = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

  vk::raii::SurfaceKHR             surface        = nullptr;
  RenderContext                    renderContext;
  SwapChainResources               swapChain;

  vk::raii::Pipeline               pipeline = nullptr;

  std::vector<Optim::FrameRenderer> frames;
  size_t                            frameIndex;
  bool                              frameBufferResized = false;

  VertexBuffer vertexBuffer = nullptr;
  GpuBuffer    indexBuffer  = nullptr;

  // BufferData _test_vertexBuffer;
  // TEST FIELD
  std::vector<Vertex> testVertices = {
    { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    {  { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    {   { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
    {  { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
  };

  std::vector<uint32> indices = {
    0, 1, 2, 2, 3, 0
  };

  /**
   * @brief
   * Transfers the vertex data from a staging buffer into the dedicated
   * device local vertex buffer.
   */
  void InitalizeVertexBuffer();

  void InitializeIndexBuffer();

  /**
   * MIGHT THROW
   */
  void Initialize(SDL_Window* pSDLWindow)
  {
    pWindow = pSDLWindow;

    // Create VkInstance object
    instance = Optim::VK::CreateVkInstance(context);
    if (instance == nullptr) {
      throw std::runtime_error("VkInstance Initalization FAILURE");
    }

    // Create VkDebugUtilsMessengerEXT object if validation layers are enabled
    if constexpr (Optim::VK::enableValidationLayers) {
      debugMessenger = Optim::VK::Debug::CreateVkDebugUtilsMessengerEXT(instance);
      if (debugMessenger == nullptr) {
        throw std::runtime_error("VkDebugUtilsMessengerEXT Initalization FAILURE");
      }
    }

    // Create VKSurfaceKHR object
    surface = Optim::VK::CreateVkSurfaceKHR(instance, pSDLWindow);
    if (surface == nullptr) {
      throw std::runtime_error("VkSurfaceKHR Initalization FAILURE");
    }

    // Create RenderContext object
    renderContext = RenderContext(instance, surface);
    if (!renderContext.IsValid()) {
      throw std::runtime_error("RenderContext Initalization FAILURE");
    }

    // Create the initial SwapChainResources
    swapChain = SwapChainResources(renderContext.device, renderContext.physicalDevice, surface, pSDLWindow);
    if (!swapChain.IsValid()) {
      throw std::runtime_error("SwapChainResources Initalization FAILURE");
    }

    // Pipeline creation
    //  * Load Shader
    //  * Create `VkShaderModule`
    //  * Crate Shader Stages Info
    //  * Use Data to create a `VkPipeline` Object
    auto shadercode       = Optim::VKPipeline::LoadCompiledShader("Shaders/dynVert.spv");
    auto ShaderModule     = Optim::VKPipeline::CreateVkShaderModule(shadercode, renderContext.device);
    auto shaderStagesInfo = Optim::VKPipeline::CreateVkPipelineShaderStageCreateInfoList(ShaderModule);

    pipeline = Optim::VKPipeline::CreateVulkanPipeline(swapChain, shaderStagesInfo, renderContext.device);
    if (pipeline == nullptr) {
      throw std::runtime_error("VkPipeline Initalization FAILURE");
    }

    // Vertex buffer creation
    vertexBuffer = VertexBuffer(renderContext.device, renderContext.physicalDevice, testVertices);

    // Create the index buffer from generic GpuBuffer class
    indexBuffer = GpuBuffer(renderContext.device,
                            renderContext.physicalDevice,
                            BinSize(indices),
                            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                            vk::MemoryPropertyFlagBits::eDeviceLocal,
                            nullptr);

    InitalizeVertexBuffer(); 
    InitializeIndexBuffer();
    
    // Create FrameResources objects
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      auto frame = Optim::FrameRenderer(renderContext.device);
      frame.CmdResources() = Optim::CommandResources(renderContext.device, renderContext.queueIndex);
      frames.emplace_back(std::move(frame));
    }
  }

  void DrawFrame()
  {
    if (frameBufferResized) {
      frameBufferResized = false;
      RecreateSwapChain();
      return;
    }

    auto& currentFrame = frames[frameIndex];

    /**
     * Wait for frame slot to be signaled and free to be used. This fence gets
     * signaled when the last frame commands have finished being computed by
     * the GPU queue.
     */
    auto fenceResult = renderContext.device.waitForFences(*currentFrame.InFlightFence(), vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
      throw std::runtime_error("Failed to wait for inFlightFence");
    }

    /**
     * Aquire a new image index from the swap chain. Signal the presentCompleteSemaphore
     * of the active frame resource.
     */
    auto [result, imageIndex] = swapChain.swapChain.acquireNextImage(UINT64_MAX, *currentFrame.PresentCompleteSemaphore(), nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || frameBufferResized) {
      frameBufferResized = false;
      RecreateSwapChain();
      return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
      assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
      throw std::runtime_error("Failed to acquire swap chain image");
    }

    renderContext.device.resetFences(*currentFrame.InFlightFence());

    auto& commandBuffer = currentFrame.CmdResources().CmdBuffer();

    // Reset command buffer then start recodring commands for next frame.
    commandBuffer.reset();
    currentFrame.Start(swapChain.images[imageIndex], swapChain.imageViews[imageIndex], swapChain.extent);

    // currentFrame.RecordCommandBuffer(swapChain, imageIndex, pipeline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
    commandBuffer.bindVertexBuffers(0, *vertexBuffer.Buffer(), {0});
    commandBuffer.bindIndexBuffer(*indexBuffer.Buffer(), 0, vk::IndexType::eUint32);
    // commandBuffer.draw(testVertices.size(), 1, 0, 0);
    commandBuffer.drawIndexed(static_cast<uint32>(indices.size()), 1, 0, 0, 0);
    currentFrame.End(swapChain.images[imageIndex]);

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
      .pWaitSemaphores      = &(*currentFrame.PresentCompleteSemaphore()),
      .pWaitDstStageMask    = &waitDstStageMask,
      .commandBufferCount   = 1,
      .pCommandBuffers      = &(*commandBuffer),
      .signalSemaphoreCount = 1,
      .pSignalSemaphores    = &(*swapChain.renderFinishedSemaphores[imageIndex])
    };
    renderContext.queue.submit(submitInfo, *currentFrame.InFlightFence());

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

    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || frameBufferResized) {
      frameBufferResized = false;
      RecreateSwapChain();
    }
    
    // Advance frame index
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  inline void WaitForDeviceIdle()
  {
    renderContext.device.waitIdle();
  }

  inline void RecreateSwapChain()
  {
    WaitForDeviceIdle();
    swapChain.imageViews.clear();
    swapChain.swapChain = nullptr;
    swapChain           = std::move(SwapChainResources(renderContext.device, renderContext.physicalDevice, surface, pWindow));
  }
};