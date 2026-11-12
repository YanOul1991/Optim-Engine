#pragma once

#include "Core/CoreMinimal.h"
#include "OpVkCommon/Minimal.h"
#include "OpVkCore/OpVkCore.h"
#include "OpVkDebug/Debug.h"
#include "OpVkTypes/Buffers.h"
#include "OpVkTypes/CommandContext.h"
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

/**
 * Review global uses of classes and encapsulation.
 */

class Renderer final
{
 public:
  static constexpr inline int32 MAX_FRAMES_IN_FLIGHT = 2;
  
  // SDL_Window pointer, only focusing on single window application so we make 
  // it a simple member variable.
  SDL_Window*                      pWindow = nullptr;

  // Global Vulkan Objects
  vk::raii::Context                context;
  vk::raii::Instance               instance         = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger   = nullptr;
  vk::raii::SurfaceKHR             surface          = nullptr;
  vk::raii::PhysicalDevice         physicalDevice   = nullptr;
  vk::raii::Device                 device           = nullptr;
  vk::raii::Queue                  renderQueue      = nullptr;
  uint32                           renderQueueIndex = ~0;
  
  Optim::SwapChainResource         swapChain;

  vk::raii::Pipeline                pipeline = nullptr;
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
    instance = Optim::CreateVkInstance(context);
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
    surface = Optim::CreateVkSurfaceKHR(instance, pSDLWindow);
    if (surface == nullptr) {
      throw std::runtime_error("VkSurfaceKHR Initalization FAILURE");
    }

    // Select the most suitable physical device (GPU) for rendering
    physicalDevice = Optim::SelectVkPhysicalDevice(instance, surface);

    auto [outDevice, outRenderQueue, outRenderQueueIndex] = CreateVkDeviceAndQueue(physicalDevice, surface);
    device           = std::move(outDevice);
    renderQueue      = std::move(outRenderQueue);
    renderQueueIndex = outRenderQueueIndex;

    // Create the initial SwapChainResources
    swapChain = Optim::SwapChainResource(device, physicalDevice, surface, pSDLWindow);
    if (!swapChain.IsValid()) {
      throw std::runtime_error("SwapChainResources Initalization FAILURE");
    }

    // Pipeline creation
    //  * Load Shader
    //  * Create `VkShaderModule`
    //  * Crate Shader Stages Info
    //  * Use Data to create a `VkPipeline` Object
    auto shadercode       = Optim::VKPipeline::LoadCompiledShader("Shaders/dynVert.spv");
    auto ShaderModule     = Optim::VKPipeline::CreateVkShaderModule(shadercode, device);
    auto shaderStagesInfo = Optim::VKPipeline::CreateVkPipelineShaderStageCreateInfoList(ShaderModule);

    // pipeline = Optim::VKPipeline::CreateVulkanPipeline(swapChain, shaderStagesInfo, renderContext.device);
    pipeline = Optim::VK::CreateVulkanPipeline(device, swapChain.ImageFormat().format, shaderStagesInfo);
    if (pipeline == nullptr) {
      throw std::runtime_error("VkPipeline Initalization FAILURE");
    }

    // Vertex buffer creation
    vertexBuffer = VertexBuffer(device, physicalDevice, testVertices);

    // Create the index buffer from generic GpuBuffer class
    indexBuffer = GpuBuffer(device,
                            physicalDevice,
                            BinSize(indices),
                            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                            vk::MemoryPropertyFlagBits::eDeviceLocal,
                            nullptr);

    InitalizeVertexBuffer(); 
    InitializeIndexBuffer();
    
    // Create FrameResources objects
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      auto frame = Optim::FrameRenderer(device);
      frame.CmdResources() = Optim::CommandResources(device, renderQueueIndex);
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
    auto fenceResult = device.waitForFences(*currentFrame.InFlightFence(), vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
      throw std::runtime_error("Failed to wait for inFlightFence");
    }

    /**
     * Aquire a new image index from the swap chain. Signal the presentCompleteSemaphore
     * of the active frame resource.
     */
    auto [result, imageIndex] = swapChain.SwapChain().acquireNextImage(UINT64_MAX, *currentFrame.PresentCompleteSemaphore(), nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || frameBufferResized) {
      frameBufferResized = false;
      RecreateSwapChain();
      return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
      assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
      throw std::runtime_error("Failed to acquire swap chain image");
    }

    device.resetFences(*currentFrame.InFlightFence());

    auto& commandBuffer = currentFrame.CmdResources().CmdBuffer();

    // Reset command buffer then start recodring commands for next frame.
    commandBuffer.reset();
    currentFrame.Start(swapChain.Images()[imageIndex], swapChain.ImageViews()[imageIndex], swapChain.ImageExtent());

    // currentFrame.RecordCommandBuffer(swapChain, imageIndex, pipeline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
    commandBuffer.bindVertexBuffers(0, *vertexBuffer.Buffer(), {0});
    commandBuffer.bindIndexBuffer(*indexBuffer.Buffer(), 0, vk::IndexType::eUint32);
    commandBuffer.drawIndexed(static_cast<uint32>(indices.size()), 1, 0, 0, 0);
    currentFrame.End(swapChain.Images()[imageIndex]);

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
      .pSignalSemaphores    = &(*swapChain.RenderFinishedSemaphore()[imageIndex])
    };
    renderQueue.submit(submitInfo, *currentFrame.InFlightFence());

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
      .pWaitSemaphores    = &(*swapChain.RenderFinishedSemaphore()[imageIndex]),
      .swapchainCount     = 1,
      .pSwapchains        = &(*swapChain.SwapChain()),
      .pImageIndices      = &imageIndex
    };
    result = renderQueue.presentKHR(presentInfo);

    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || frameBufferResized) {
      frameBufferResized = false;
      RecreateSwapChain();
    }
    
    // Advance frame index
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  inline void WaitForDeviceIdle()
  {
    device.waitIdle();
  }

  inline void RecreateSwapChain()
  {
    WaitForDeviceIdle();
    swapChain.ImageViews().clear();
    swapChain.SwapChain() = nullptr;
    swapChain = std::move(Optim::SwapChainResource(device, physicalDevice, surface, pWindow));
  }
};