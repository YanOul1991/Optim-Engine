#pragma once

#include "OpVkCommon/Minimal.h"

struct RenderContext;
struct SwapChainContext;

/**
 * Vulkan Documentation:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 *
 * Structure to manage drawing instructions
 */
struct CommandContext final
{
  vk::raii::CommandPool   commandPool   = nullptr;
  vk::raii::CommandBuffer commandBuffer = nullptr;

  vk::raii::Semaphore presentCompleteSemaphore = nullptr;
  vk::raii::Semaphore renderFinishedSemaphore  = nullptr;
  vk::raii::Fence     drawFence                = nullptr;

  [[nodiscard]]
  inline bool IsValid() const noexcept
  {
    return commandPool != nullptr &&
           commandBuffer != nullptr &&
           presentCompleteSemaphore != nullptr &&
           renderFinishedSemaphore != nullptr &&
           drawFence != nullptr;
  }

  CommandContext() = default;

  // Yes move :D
  CommandContext(CommandContext&&) noexcept            = default;
  CommandContext& operator=(CommandContext&&) noexcept = default;
  // No copy >:(
  CommandContext(const CommandContext&)            = delete;
  CommandContext& operator=(const CommandContext&) = delete;

  CommandContext(const vk::raii::Device& device, RenderContext& renderContext);

  void RecordCommandBuffer(const SwapChainContext& swapChainContext, const uint32 swapChainImageIndex, const vk::raii::Pipeline& vkPipeline);

  /*
  void CreateCommandPool(const vk::raii::Device& device, uint32 queueIndex)
  {
    vk::CommandPoolCreateInfo poolInfo{
      .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = queueIndex
    };
    commandPool = vk::raii::CommandPool(device, poolInfo);
  }

  void CreateCommandBuffer(const vk::raii::Device& device)
  {
    vk::CommandBufferAllocateInfo allocInfo{
      .commandPool        = commandPool,
      .level              = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1
    };
    // Contructor create vector of VkCommandBuffer objects.
    // Only take the first one (using .front() function) and
    // use std::move to make sure that the command buffer holds the object
    // and that it wont be destroyed once the vector is being destroyed.
    commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
  };

  */

  /*
  void RecordCommandBuffer(
    const std::vector<vk::Image>&     swapChainImages,
    const std::vector<vk::ImageView>& swapChainImageViews,
    uint32                            imageIndex,
    const vk::Extent2D&               swapChainExtent,
    vk::raii::Pipeline&               graphicsPipeline)
  {
    commandBuffer.begin({});

    TransitionImageLayout(
      swapChainImages,
      imageIndex,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal,
      {},
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput);

    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

    vk::RenderingAttachmentInfo attachementInfo = {
      .imageView   = swapChainImageViews[imageIndex],
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp      = vk::AttachmentLoadOp::eClear,
      .storeOp     = vk::AttachmentStoreOp::eStore,
      .clearValue  = clearColor
    };

    vk::RenderingInfo renderingInfo = {
      .renderArea           = { .offset = { 0, 0 }, .extent = swapChainExtent },
      .layerCount           = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments    = &attachementInfo
    };

    commandBuffer.beginRendering(renderingInfo);

    // Basic Drawing Commands
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);

    commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));
    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRendering();

    TransitionImageLayout(
      swapChainImages,
      imageIndex,
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::ePresentSrcKHR,
      {},
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eBottomOfPipe);

    commandBuffer.end();
  }
  */

  void TransitionImageLayout(
    const std::vector<vk::Image>& swapChainImages,
    uint32                        imageIndex,
    vk::ImageLayout               oldLayout,
    vk::ImageLayout               newLayout,
    vk::AccessFlags2              srcAcesssMask,
    vk::AccessFlags2              dstAcessMask,
    vk::PipelineStageFlags2       srcStageMask,
    vk::PipelineStageFlags2       dstStageMask)
  {
    vk::ImageMemoryBarrier2 barrier = {
      .srcStageMask        = srcStageMask,
      .srcAccessMask       = srcAcesssMask,
      .dstStageMask        = dstStageMask,
      .dstAccessMask       = dstAcessMask,
      .oldLayout           = oldLayout,
      .newLayout           = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image               = swapChainImages[imageIndex],
      .subresourceRange    = {
                              .aspectMask     = vk::ImageAspectFlagBits::eColor,
                              .baseMipLevel   = 0,
                              .levelCount     = 1,
                              .baseArrayLayer = 0,
                              .layerCount     = 1 }
    };

    vk::DependencyInfo dependencyInfo = {
      .dependencyFlags         = {},
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers    = &barrier
    };

    commandBuffer.pipelineBarrier2(dependencyInfo);
  }
};