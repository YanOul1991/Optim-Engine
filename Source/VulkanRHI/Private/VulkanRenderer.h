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
#include <glm/glm.hpp>
#include <iostream>
#include <limits>
#include <map>
#include <ranges>
#include <vector>

namespace Optim
{

template <typename T>
inline constexpr size_t BinSize(const std::vector<T>& v) {
  return sizeof(T) * std::size(v);
}

template <typename T, size_t N>
inline constexpr size_t BinSize(const std::array<T, N>& v) {
  return sizeof(T) * std::size(v);
}

class BufferData final
{
 public:
  OPTIM_VK_RAII_TYPE(BufferData)
  // BufferData()                             = default;
  // ~BufferData()                            = default;
  // BufferData(BufferData&&)                 = default;
  // BufferData& operator=(BufferData&&)      = default;
  // BufferData(const BufferData&)            = delete;
  // BufferData& operator=(const BufferData&) = delete;

  BufferData(
    vk::raii::Device&         device,
    vk::raii::PhysicalDevice& physicalDevice,
    vk::DeviceSize            deviceSize,
    vk::BufferUsageFlagBits   usage,
    vk::MemoryPropertyFlags   memoryPorperties)
  {
    // Buffer creation
    // https://docs.vulkan.org/refpages/latest/refpages/source/VkBufferCreateInfo.html#_document_notes
    vk::BufferCreateInfo bufferInfo{
      .size        = deviceSize,
      .usage       = usage,
      .sharingMode = vk::SharingMode::eExclusive
    };

    buffer = vk::raii::Buffer(device, bufferInfo);

    if (buffer == nullptr) {
      return;
    }

    // Get buffer's memory requirements
    vk::MemoryRequirements memoryReqs = buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo memAllocInfo {
      .allocationSize  = memoryReqs.size,
      .memoryTypeIndex = FindMemoryTypeIndex(physicalDevice, memoryReqs.memoryTypeBits, memoryPorperties)
    };

    deviceMemory = vk::raii::DeviceMemory(device, memAllocInfo);

    if(deviceMemory == nullptr) {
      return;
    }

    buffer.bindMemory(*deviceMemory, 0);
  }

  void Map(const void* pInData, vk::DeviceSize size)
  {
    void* data = deviceMemory.mapMemory(0, size);
    memcpy(data, pInData, size);
    deviceMemory.unmapMemory();
  }

  bool IsValid() const noexcept { return buffer != nullptr && deviceMemory != nullptr; }
  vk::raii::Buffer& GetBuffer() { return buffer; }
  vk::raii::DeviceMemory& GetMemory() { return deviceMemory; }

 private:
  inline uint32 FindMemoryTypeIndex(const vk::raii::PhysicalDevice& physicalDevice, uint32 typeFilter, vk::MemoryPropertyFlags properties)
  {
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
      }
    }
    throw std::runtime_error("failed to find suitable memory type!");
  }

  vk::raii::Buffer       buffer       = nullptr;
  vk::raii::DeviceMemory deviceMemory = nullptr;
};


/**
 * 
 */
class VertexBuffer final
{
 private:
  BufferData gpuBuffer;
  uint32     vertexCount = 0;

 public:
  OPTIM_VK_RAII_TYPE(VertexBuffer)

  static constexpr const auto usage       = vk::BufferUsageFlagBits::eVertexBuffer;
  static constexpr const auto memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                                            vk::MemoryPropertyFlagBits::eHostCoherent;

  // VertexBuffer()                               = default;
  // VertexBuffer(VertexBuffer&&)                 = default;
  // VertexBuffer& operator=(VertexBuffer&&)      = default;
  // VertexBuffer(const VertexBuffer&)            = delete;
  // VertexBuffer& operator=(const VertexBuffer&) = delete;

  VertexBuffer(vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice)
  {
    // Cannot directly call member initalizer list for BufferData constructor because the vertices
    // buffer will not be instanciated in time, therefore the size value being passed is of 0 which
    // causes a runtime error.
    // For now because of testing, it will be fine, but in the future the list of vertices may
    // not be owner directly by this class, in which case we will see again about initalized list.
    gpuBuffer = Optim::BufferData(device, physicalDevice, BinSize(vertices), usage, memoryFlags);
    if(!gpuBuffer.IsValid()) {
      throw std::runtime_error("Failed to create vertex buffer");
    }
    gpuBuffer.Map(vertices.data(), BinSize(vertices));
  }

  // TEST FIELD
  std::vector<Vertex> vertices = {
    { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    {  { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
    { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
  };
};

}; // namespace Optim::RHI

class Renderer final
{
 public:
  static constexpr inline int32 MAX_FRAMES_IN_FLIGHT = 2;

  SDL_Window*                      pWindow = nullptr;
  vk::raii::Context                context;
  vk::raii::Instance               instance       = nullptr;
  vk::raii::SurfaceKHR             surface        = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
  RenderContext                    renderContext;
  SwapChainResources               swapChain;
  vk::raii::Pipeline               pipeline = nullptr;
  std::vector<FrameResource>       frames;
  size_t                           frameIndex;
  bool                             frameBufferResized = false;

  Optim::VertexBuffer vertexBuffer;

  /**
   * MIGHT THROW
   */
  FORCEINLINE void Initialize(SDL_Window* pSDLWindow)
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
    auto shadercode       = Optim::VKPipeline::LoadCompiledShader("Shaders/sample.spv");
    auto ShaderModule     = Optim::VKPipeline::CreateVkShaderModule(shadercode, renderContext.device);
    auto shaderStagesInfo = Optim::VKPipeline::CreateVkPipelineShaderStageCreateInfoList(ShaderModule);

    pipeline = Optim::VKPipeline::CreateVulkanPipeline(swapChain, shaderStagesInfo, renderContext.device);
    if (pipeline == nullptr) {
      throw std::runtime_error("VkPipeline Initalization FAILURE");
    }

    vertexBuffer = Optim::VertexBuffer(renderContext.device, renderContext.physicalDevice);

    // Create FrameResources objects
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      frames.emplace_back(renderContext.device, renderContext);
    }
  }

  // void CreateVertexBuffer()
  // {
  //   // Create Vertex buffer
  //   // https://docs.vulkan.org/refpages/latest/refpages/source/VkBufferCreateInfo.html#_document_notes
  //   vk::BufferCreateInfo bufferInfo{
  //     .size        = sizeof(vertices[0]) * vertices.size(),
  //     .usage       = vk::BufferUsageFlagBits::eVertexBuffer,
  //     .sharingMode = vk::SharingMode::eExclusive
  //   };
  //   vertexBuffer = vk::raii::Buffer(renderContext.device, bufferInfo);
  //   if (vertexBuffer == nullptr) {
  //     throw std::runtime_error("Vertex Buffer initialization failure");
  //   }
  //   // Get memory requirements
  //   vk::MemoryRequirements memReq = vertexBuffer.getMemoryRequirements();
  //   auto findMemoryType = [&](uint32 typeFilter, vk::MemoryPropertyFlags properties) -> uint32 {
  //     vk::PhysicalDeviceMemoryProperties memProperties = renderContext.physicalDevice.getMemoryProperties();
  //     for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
  //       if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
  //         return i;
  //       }
  //     }
  //     throw std::runtime_error("failed to find suitable memory type!");
  //   };
  //   constexpr auto reqMemPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
  //   vk::MemoryAllocateInfo memAllocInfo{
  //     .allocationSize  = memReq.size,
  //     .memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, reqMemPropertyFlags)
  //   };
  //   vertexBufferMemory = vk::raii::DeviceMemory(renderContext.device, memAllocInfo);
  //   if (vertexBufferMemory == nullptr) {
  //     throw std::runtime_error("Failed to create vertex buffer memory");
  //   }
  //   vertexBuffer.bindMemory(*vertexBufferMemory, 0);
  //   void* data = vertexBufferMemory.mapMemory(0, bufferInfo.size);
  //   memcpy(data, vertices.data(), bufferInfo.size);
  //   vertexBufferMemory.unmapMemory();
  // }

  FORCEINLINE void DrawFrame()
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
    auto fenceResult = renderContext.device.waitForFences(*currentFrame.inFlightFence, vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
      throw std::runtime_error("Failed to wait for inFlightFence");
    }

    /**
     * Aquire a new image index from the swap chain. Signal the presentCompleteSemaphore
     * of the active frame resource.
     */
    auto [result, imageIndex] = swapChain.swapChain.acquireNextImage(UINT64_MAX, *currentFrame.presentCompleteSemaphore, nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || frameBufferResized) {
      frameBufferResized = false;
      RecreateSwapChain();
      return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
      assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
      throw std::runtime_error("Failed to acquire swap chain image");
    }

    renderContext.device.resetFences(*currentFrame.inFlightFence);

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