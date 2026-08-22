#include "./CommandContext.h"

#include "./RenderContext.h"
#include "./SwapChainContext.h"

CommandContext::CommandContext(const vk::raii::Device& device, RenderContext& renderContext)
{
  // Create the command pool
  vk::CommandPoolCreateInfo poolInfo{
    .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
    .queueFamilyIndex = renderContext.queueIndex
  };

  this->commandPool = vk::raii::CommandPool(device, poolInfo);

  // Create the command buffer
  vk::CommandBufferAllocateInfo allocInfo{
    .commandPool        = commandPool,
    .level              = vk::CommandBufferLevel::ePrimary,
    .commandBufferCount = 1
  };

  // Contructor create vector of VkCommandBuffer objects. Only take the first
  // one (using .front() function) and use std::move to move the object as
  // front return an lvalue ref which is a delete function.
  this->commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());

  // Create the sync objects
  this->presentCompleteSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
  this->renderFinishedSemaphore  = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
  this->drawFence                = vk::raii::Fence(device, { .flags = vk::FenceCreateFlagBits::eSignaled });
}

void CommandContext::RecordCommandBuffer(const SwapChainContext& swapChainContext, const uint32 imageIndex, const vk::raii::Pipeline& vkPipeline)
{
  commandBuffer.begin({});

  TransitionImageLayout(
    swapChainContext.swapChainImages,
    imageIndex,
    vk::ImageLayout::eUndefined,
    vk::ImageLayout::eColorAttachmentOptimal,
    {},
    vk::AccessFlagBits2::eColorAttachmentWrite,
    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
    vk::PipelineStageFlagBits2::eColorAttachmentOutput);

  vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

  vk::RenderingAttachmentInfo attachementInfo = {
    .imageView   = swapChainContext.swapChainImageViews[imageIndex],
    .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
    .loadOp      = vk::AttachmentLoadOp::eClear,
    .storeOp     = vk::AttachmentStoreOp::eStore,
    .clearValue  = clearColor
  };

  vk::RenderingInfo renderingInfo = {
    .renderArea           = { .offset = { 0, 0 }, .extent = swapChainContext.swapChainExtent },
    .layerCount           = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments    = &attachementInfo
  };

  // Start rendering objects
  commandBuffer.beginRendering(renderingInfo);

  // Basic Drawing Commands
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *vkPipeline);

  commandBuffer.setViewport(
    0,
    vk::Viewport(
      0.0f,
      0.0f,
      static_cast<float>(swapChainContext.swapChainExtent.width),
      static_cast<float>(swapChainContext.swapChainExtent.height),
      0.0f,
      1.0f));

  commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainContext.swapChainExtent));
  commandBuffer.draw(3, 1, 0, 0);

  commandBuffer.endRendering();

  TransitionImageLayout(
    swapChainContext.swapChainImages,
    imageIndex,
    vk::ImageLayout::eColorAttachmentOptimal,
    vk::ImageLayout::ePresentSrcKHR,
    {},
    vk::AccessFlagBits2::eColorAttachmentWrite,
    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
    vk::PipelineStageFlagBits2::eBottomOfPipe);

  commandBuffer.end();
}