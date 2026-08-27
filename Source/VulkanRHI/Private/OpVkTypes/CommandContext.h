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