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
#include <chrono>
#include <cstdint>
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

  // Could decapsulate, but keeping it as its encapsulated objets
  // are tightly coupled. Could even add SurfaceKHR.
  Optim::SwapChainResource swapChain;

  vk::raii::DescriptorSetLayout        descriptorSetLayout = nullptr; // NEW
  vk::raii::PipelineLayout             pipelineLayout      = nullptr; // NEW
  vk::raii::DescriptorPool             descriptorPool      = nullptr; // NEW
  std::vector<vk::raii::DescriptorSet> descriptorSets;
  vk::raii::Pipeline                   pipeline = nullptr;

  std::vector<GpuBuffer> uniformBuffers;  // NEW List of uniform buffers, one per frame in flight

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

  // NEW Function
  // Must be called before creating the actual pipeline object.
  inline void CreateDescriptorSetLayoutAndPipelineLayout()
  {
    // Note:
    //  Slang refelection API could help implementing a flexible layout
    //  binding for different pipeline.
    vk::DescriptorSetLayoutBinding uBufferLayoutBinding = {
      .binding            = 0,                                  // Binding slot: 0 by default
      .descriptorType     = vk::DescriptorType::eUniformBuffer, // Resource type = uniform buffer / constant buffer
      .descriptorCount    = 1,                                  // Only one value to assign
      .stageFlags         = vk::ShaderStageFlagBits::eVertex,   // Used in the vertex shade stage only.
      .pImmutableSamplers = nullptr                             // "relevant for image sampling related descriptors,"
    };

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {
      .bindingCount = 1,
      .pBindings    = &uBufferLayoutBinding
    };

    descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {
      .setLayoutCount         = 1,
      .pSetLayouts            = &*descriptorSetLayout,
      .pushConstantRangeCount = 0
    };

    pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);
  }

  // NEW 
  inline void CreateDescriptorPool()
  {
    vk::DescriptorPoolSize poolSize = {
      .type            = vk::DescriptorType::eUniformBuffer,  //
      .descriptorCount = MAX_FRAMES_IN_FLIGHT                 // Allocate for each frame
    };

    vk::DescriptorPoolCreateInfo poolInfo = {
      .flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets       = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = 1,
      .pPoolSizes    = &poolSize
    };

    descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
  }

  // NEW
  inline void CreateDescriptorSets()
  {
#ifndef NDEBUG
    // Assert descriptor pool is valid at this point. 
    assert(descriptorPool != nullptr && "[VulkanRHI - Renderer::CreateDescriptorSets()] assertion failed: the 'descriptorPool' object is null");
#endif
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);

    vk::DescriptorSetAllocateInfo allocInfo = {
      .descriptorPool     = descriptorPool,
      .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts        = layouts.data()
    };

    descriptorSets = device.allocateDescriptorSets(allocInfo);
  }

  // NEW
  inline void UpdateUniformBuffer(int32 index, const vk::Extent2D& imageExtent) 
  {
    static auto startTime   = std::chrono::high_resolution_clock::now();
    auto        currentTime = std::chrono::high_resolution_clock::now();
    float       time        = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject cBuffer{};
    // Model rotates clockwise on the Z axis.
    cBuffer.model = glm::rotate(glm::mat4(1.0f), time * (glm::pi<float>() / 2), glm::vec3(0.0f, 0.0f, 1.0f));
    cBuffer.view  = glm::lookAt(glm::vec3(0.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    cBuffer.proj  = glm::perspective(glm::radians(50.0f), static_cast<float>(imageExtent.width) / static_cast<float>(imageExtent.height), 0.1f, 100.0f);

    // NOTE:
    //  GLM uses column-major layout, but Slang uses row-major like HLSL.
    //  Could transpose before updating the buffer data, I opted to update the
    //  slang compiler's settings to make matices layout as column-major to 
    //  match glm's. This avoids extra transpose overhead on the host side.

    // cBuffer.model = glm::transpose(cBuffer.model);
    // cBuffer.view  = glm::transpose(cBuffer.view);
    // cBuffer.proj  = glm::transpose(cBuffer.proj);

    memcpy(uniformBuffers[index].pMappedMemory, &cBuffer, sizeof(cBuffer));
  };

  /**
   * Performs main initializations for Renderer members.
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
    auto shadercode       = Optim::LoadCompiledShader("Shaders/uniformBuffers.spv");
    auto ShaderModule     = Optim::CreateVkShaderModule(shadercode, device);
    auto shaderStagesInfo = Optim::CreateVkPipelineShaderStageCreateInfoList(ShaderModule);

    CreateDescriptorSetLayoutAndPipelineLayout();
    
    pipeline = Optim::CreateVkPipeline(device, swapChain.ImageFormat().format, shaderStagesInfo, pipelineLayout);
    if (pipeline == nullptr) {
      throw std::runtime_error("VkPipeline Initalization FAILURE");
    }

    // Vertex buffer creation
    vertexBuffer = VertexBuffer(device, physicalDevice, testVertices);

    // Create the index buffer from generic GpuBuffer class
    indexBuffer = GpuBuffer(
      device,
      physicalDevice,
      BinSize(indices),
      vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal);

    InitalizeVertexBuffer(); 
    InitializeIndexBuffer();
    
    // Create FrameResources objects
    for (int32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      auto frame = Optim::FrameRenderer(device);
      frame.CmdResources() = Optim::CommandResources(device, renderQueueIndex);
      frames.emplace_back(std::move(frame));
    }

    // Create Uniform buffers
    for (int32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      GpuBuffer tempUniformBuffer(
        device,
        physicalDevice,
        sizeof(UniformBufferObject),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        // Cache mapped memory in pointer. Less expensive than per frame mapping.
        tempUniformBuffer.pMappedMemory = tempUniformBuffer.deviceMemory.mapMemory(0, sizeof(UniformBufferObject));
        uniformBuffers.emplace_back(std::move(tempUniformBuffer));
    }

    // Create then populate descriptors sets
    CreateDescriptorPool();
    CreateDescriptorSets();

    for (int32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vk::DescriptorBufferInfo bufferInfo = {
        .buffer = uniformBuffers[i].buffer,
        .offset = 0,
        .range  = sizeof(UniformBufferObject)
      };

      vk::WriteDescriptorSet descriptorWrite = {
        .dstSet          = descriptorSets[i],
        .dstBinding      = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType  = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo     = &bufferInfo
      };

      device.updateDescriptorSets(descriptorWrite, {});
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

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
    
    commandBuffer.bindVertexBuffers(0, *vertexBuffer.Buffer(), {0});
    
    commandBuffer.bindIndexBuffer(*indexBuffer.Buffer(), 0, vk::IndexType::eUint32);
    
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, *descriptorSets[frameIndex], nullptr);
    
    commandBuffer.drawIndexed(static_cast<uint32>(indices.size()), 1, 0, 0, 0);

    currentFrame.End(swapChain.Images()[imageIndex]);

    // Only wait for the presentCompleteSemaphore during the color attachement
    // output stage.
    vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    // Update uniform buffer before submitting work to gpu
    UpdateUniformBuffer(frameIndex, swapChain.ImageExtent());

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