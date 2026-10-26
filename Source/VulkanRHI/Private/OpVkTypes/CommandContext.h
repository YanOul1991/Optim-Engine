#pragma once

#include "OpVkCommon/Minimal.h"

/**
 * Vulkan Documentation:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 *
 * VULKAN OBJECTS
 *
 * SEMAPHORES:
 * https://docs.vulkan.org/spec/latest/chapters/synchronization.html#synchronization-semaphores
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
 * FENCES:
 * https://docs.vulkan.org/spec/latest/chapters/synchronization.html#VkFence
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

namespace Optim
{

/**
 * @brief
 * Wrapper class for command resources wrapping`vk::raii::CommandPool` 
 * and `vk::raii::CommandBuffer` objects.
 * 
 * This object should be used instead of single command buffers, as it is 
 * easier to manage a 1 pool per buffer, than to manage multiple buffers
 * with a restricted pool count, unless necessary. Might change during 
 * implementation.
 */
class CommandResources final
{
 private:
  vk::raii::CommandPool   commandPool   = nullptr;
  vk::raii::CommandBuffer commandBuffer = nullptr;

 public:
  OPTIM_VK_RAII_TYPE(CommandResources)
  // CommandResources()                                   = default;
  // ~CommandResources()                                  = default;
  // CommandResources(CommandResources&&)                 = default;
  // CommandResources& operator=(CommandResources&&)      = default;
  // CommandResources(const CommandResources&)            = delete;
  // CommandResources& operator=(const CommandResources&) = delete;
  // CommandResources(std::nullptr_t) {};

  inline CommandResources(vk::raii::Device& device, uint32 queueIndex)
  {
    // Create the command pool
    vk::CommandPoolCreateInfo poolInfo{
      .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = queueIndex
    };

    commandPool = vk::raii::CommandPool(device, poolInfo);

    // Create the command buffer
    vk::CommandBufferAllocateInfo allocInfo{
      .commandPool        = commandPool,
      .level              = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1
    };

    // Contructor create vector of VkCommandBuffer objects. Only take the first
    // one (using .front() function) and use std::move to move the object as
    // front return an lvalue ref which is a delete function.
    commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
  }

  vk::raii::CommandPool&   CmdPool()   noexcept { return commandPool; }
  vk::raii::CommandBuffer& CmdBuffer() noexcept { return commandBuffer; }

  inline bool IsValid() const noexcept { return commandPool != nullptr && commandBuffer != nullptr; }
};

/**
 * TODO: 
 *  - Add description for frame renderer. 
 *  - Review object's implementation and functionalities, as currently it is 
 *    quite verbose for no reason.
 */
class FrameRenderer final
{
 private:
  CommandResources    cmdResources             = nullptr;
  vk::raii::Semaphore presentCompleteSemaphore = nullptr;
  vk::raii::Fence     inFlightFence            = nullptr;

  // Keep track of active image layout for image transitions.
  // Must test to make sure it doesnt cause sync issues with command recording
  vk::ImageLayout currentImageLayout = vk::ImageLayout::eUndefined;

 public:
  OPTIM_VK_RAII_TYPE(FrameRenderer)
  // FrameRenderer()                                = default;
  // ~FrameRenderer()                               = default;
  // FrameRenderer(FrameRenderer&&)                 = default;
  // FrameRenderer& operator=(FrameRenderer&&)      = default;
  // FrameRenderer(const FrameRenderer&)            = delete;
  // FrameRenderer& operator=(const FrameRenderer&) = delete;
  // FrameRenderer(std::nullptr_t) {};

  FrameRenderer(const vk::raii::Device& device) :
      presentCompleteSemaphore(vk::raii::Semaphore(device, vk::SemaphoreCreateInfo())),
      inFlightFence(vk::raii::Fence(device, { .flags = vk::FenceCreateFlagBits::eSignaled }))
  {}

  inline CommandResources&        CmdResources()             noexcept { return cmdResources; }
  inline vk::raii::Semaphore&     PresentCompleteSemaphore() noexcept { return presentCompleteSemaphore; }
  inline vk::raii::Fence&         InFlightFence()            noexcept { return inFlightFence; }
  // Maybe find a more efficient way of getting underlying CommandResources members ???
  inline vk::raii::CommandPool&   CommandPool()              noexcept { return cmdResources.CmdPool(); }
  inline vk::raii::CommandBuffer& CommandBuffer()            noexcept { return cmdResources.CmdBuffer(); }

  [[nodiscard]]
  inline bool IsValid() const noexcept { return cmdResources.IsValid() && *presentCompleteSemaphore && *inFlightFence; }

  inline void Start(vk::Image& image, vk::raii::ImageView& imageView, vk::Extent2D& extent)
  {
    auto& cmd = cmdResources.CmdBuffer();
    cmd.begin({});

    // Transition image layout to optimize for color attachement.
    TransitionImageLayout(
      image,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal,
      {},
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput);

    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

    vk::RenderingAttachmentInfo attachementInfo = {
      .imageView   = imageView,
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp      = vk::AttachmentLoadOp::eClear,
      .storeOp     = vk::AttachmentStoreOp::eStore,
      .clearValue  = clearColor
    };

    vk::RenderingInfo renderingInfo = {
      .renderArea           = { .offset = { 0, 0 }, .extent = extent },
      .layerCount           = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments    = &attachementInfo
    };

    cmd.beginRendering(renderingInfo);

    // Basic Drawing Commands
    cmd.setViewport(
      0,
      vk::Viewport(
        0.0f,
        0.0f,
        static_cast<float>(extent.width),
        static_cast<float>(extent.height),
        0.0f,
        1.0f));

    cmd.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), extent));
  }

  inline void End(vk::Image& image)
  {
    auto& cmdBuffer = cmdResources.CmdBuffer();
    cmdBuffer.endRendering();

    TransitionImageLayout(
      image,
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::ePresentSrcKHR,
      {},
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eBottomOfPipe | vk::PipelineStageFlagBits2::eColorAttachmentOutput);

    cmdBuffer.end();
  }

  inline void TransitionImageLayout(
    vk::Image&              vkImage,
    vk::ImageLayout         oldLayout,
    vk::ImageLayout         newLayout,
    vk::AccessFlags2        srcAcesssMask,
    vk::AccessFlags2        dstAcessMask,
    vk::PipelineStageFlags2 srcStageMask,
    vk::PipelineStageFlags2 dstStageMask)
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
      .image               = vkImage,
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

    cmdResources.CmdBuffer().pipelineBarrier2(dependencyInfo);
  }
};

}; // namespace Optim
