#pragma once

#include "OpVkCommon/Minimal.h"

struct RenderContext;
struct SwapChainContext;

/**
 * VULKAN OBJECTS
 * 
 * SEMAPHORES: https://docs.vulkan.org/spec/latest/chapters/synchronization.html#synchronization-semaphores
 * 
 * The CPU submits commands to GPU queues without waiting for execution to 
 * complete. This allows multiple command submissions and queues to run 
 * concurrently on the GPU, but it requires explicit synchronization when data 
 * is shared between them.
 * 
 * When submitting commands to a queue, we can instruct the queue to wait for a 
 * designated semaphore to be signaled before executing dependent pipeline 
 * stages. A queue submission can also signal a semaphore upon completion, 
 * enabling precise control over GPU-side synchronization.
 * 
 * FENCES: https://docs.vulkan.org/spec/latest/chapters/synchronization.html#VkFence
 * 
 * Fences synchronize CPU and GPU execution by allowing the CPU (Host) to wait
 * for a GPU queue submission to complete.
 * 
 * When submitting command buffers to a GPU queue, we can provide a fence to be
 * signaled upon task completion. The CPU thread can then continue executing
 * independent tasks concurrently before explicitly blocking on the fence
 * (vkWaitForFences). 
 * 
 * Once signaled, the CPU resets the fence (vkResetFences)
 * for future reuse and safe resource recycling.
 */

/**
 * Vulkan Documentation:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 *
 * Structure to manage drawing context
 */
struct CommandContext final
{
  vk::raii::CommandPool   commandPool          = nullptr;
  vk::raii::CommandBuffer commandBuffer        = nullptr;
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

  /**
   * @param swapChainContext
   * A reference to the active swap chain context.
   * 
   * @param swapChainImageIndex
   * The index of the swap chain image that we will draw to. This is aquired
   * by calling `vk::raii::SwapchainKHR::acquireNextImage`. This is managed 
   * in the `DrawFrame` function of the `VulkanRHI` class.
   * 
   * @param vkPipeline
   * A reference to the VkPipeline object defining the current render settings.
   */
  void RecordCommandBuffer(const SwapChainContext& swapChainContext, const uint32 swapChainImageIndex, const vk::raii::Pipeline& vkPipeline);

  /**
   * @brief
   * Helper function to change the image layout of a given `VkImage` contained 
   * in a given `SwapChainContext` object.
   * 
   * REFERENCE: https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html#_image_layout_transitions
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