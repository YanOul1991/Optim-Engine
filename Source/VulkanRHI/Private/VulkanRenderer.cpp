#include "VulkanRenderer.h"

void Renderer::InitalizeVertexBuffer()
{
  static constexpr vk::BufferUsageFlags stageBufferUsage{
    vk::BufferUsageFlagBits::eTransferSrc
  };
  static constexpr vk::MemoryPropertyFlags stageBufferMemoryProperties{
    vk::MemoryPropertyFlagBits::eHostVisible |
    vk::MemoryPropertyFlagBits::eHostCoherent
  };

  GpuBuffer stagingBuffer(
    device,
    physicalDevice,
    BinSize(testVertices),
    stageBufferUsage,
    stageBufferMemoryProperties,
    testVertices.data());

  stagingBuffer.Map(testVertices.data(), BinSize(testVertices));

  // Create temporary command pool & command buffer for buffer transfer
  vk::CommandPoolCreateInfo poolInfo{
    .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
    .queueFamilyIndex = renderQueueIndex
  };

  auto cmdPool = vk::raii::CommandPool(device, poolInfo);

  vk::CommandBufferAllocateInfo allocInfo{
    .commandPool        = cmdPool,
    .level              = vk::CommandBufferLevel::ePrimary,
    .commandBufferCount = 1
  };

  auto cmdBuf = std::move(vk::raii::CommandBuffers(device, allocInfo).front());

  // Record command to copy from staging buffer to vertex buffer
  cmdBuf.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
  cmdBuf.copyBuffer(*stagingBuffer.Buffer(), *vertexBuffer.Buffer(), vk::BufferCopy(0, 0, BinSize(testVertices)));
  cmdBuf.end();

  // Submit work to queue then, simply wait for it to be idle. (Could use a fence as well)
  renderQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*cmdBuf }, nullptr);

  renderQueue.waitIdle();
}

void Renderer::InitializeIndexBuffer()
{
  static constexpr vk::BufferUsageFlags stageBufferUsage{
    vk::BufferUsageFlagBits::eTransferSrc
  };
  static constexpr vk::MemoryPropertyFlags stageBufferMemoryProperties{
    vk::MemoryPropertyFlagBits::eHostVisible |
    vk::MemoryPropertyFlagBits::eHostCoherent
  };

  GpuBuffer stagingBuffer(
    device,
    physicalDevice,
    BinSize(testVertices),
    stageBufferUsage,
    stageBufferMemoryProperties,
    testVertices.data());

  // Map index buffer data
  stagingBuffer.Map(indices.data(), BinSize(indices));

  // Create temporary command pool & command buffer for buffer transfer
  vk::CommandPoolCreateInfo poolInfo{
    .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
    .queueFamilyIndex = renderQueueIndex
  };

  auto cmdPool = vk::raii::CommandPool(device, poolInfo);

  vk::CommandBufferAllocateInfo allocInfo{
    .commandPool        = cmdPool,
    .level              = vk::CommandBufferLevel::ePrimary,
    .commandBufferCount = 1
  };

  auto cmdBuf = std::move(vk::raii::CommandBuffers(device, allocInfo).front());

  // Record command to copy from staging buffer to vertex buffer
  cmdBuf.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
  cmdBuf.copyBuffer(*stagingBuffer.Buffer(), *indexBuffer.Buffer(), vk::BufferCopy(0, 0, BinSize(indices)));
  cmdBuf.end();

  // Submit work to queue then, simply wait for it to be idle. (Could use a fence as well)
  renderQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*cmdBuf }, nullptr);
  renderQueue.waitIdle();
}
