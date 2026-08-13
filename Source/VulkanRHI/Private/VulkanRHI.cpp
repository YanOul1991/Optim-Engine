/**
 * FILE: VulkanRHI.cpp
 */

#include "VulkanRHI/VulkanRHI.h"

#include "OpVkCommon/Minimal.h"
#include "OpVkCore/OpVkCore.h"
#include "OpVkDebug/Debug.h"
#include "OpVkTypes/RenderDevice.h"
#include "OpVkTypes/SwapChainContext.h"

// SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

// STD
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <ranges>
#include <vector>

/**
 * Vualk Documentation:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html
 *
 */

struct DrawContext
{
  vk::raii::CommandPool   commandPool   = nullptr;
  vk::raii::CommandBuffer commandBuffer = nullptr;

  /**
   * Verfiy member vk objects status
   */
  inline bool IsValid() const
  {
    return commandPool != nullptr && commandBuffer != nullptr;
  }

  void CreateCommandPool(const vk::raii::Device& device, uint32 queueFamilyIndex)
  {
    vk::CommandPoolCreateInfo poolInfo{
      .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = queueFamilyIndex
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

  void RecordCommandBuffer(const SwapChainContext& swapChainContext, uint32 imageIndex, vk::raii::Pipeline& graphicsPipeline)
  {}

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

struct VulkanRHI::VulkanContext
{
  vk::raii::Context                context;
  vk::raii::Instance               instance       = nullptr;
  vk::raii::SurfaceKHR             surface        = nullptr;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

  RenderDevice     renderContext;
  SwapChainContext swapChainContext;

  vk::raii::Pipeline      pipeline      = nullptr;
  vk::raii::CommandPool   commandPool   = nullptr;
  vk::raii::CommandBuffer commandBuffer = nullptr;

  DrawContext drawContext;
};

VulkanRHI::VulkanRHI() : pVkContext(MakeUnique<VulkanContext>())
{}

VulkanRHI::~VulkanRHI()
{}

/**
 * @brief
 * Initializes the VulkanRHI object and all required Vulkan objects.
 */
void VulkanRHI::Initialize(void* param_pSDLWindow)
{
  try {
    auto& ctx = *pVkContext;

    SDL_Window* sdlwindow = static_cast<SDL_Window*>(param_pSDLWindow);

    TDynamicArray<String> reqInstanceExt;

    // Add all extensions required by SDL to create a VkSurfaceKHR object.
    Optim::VK::AddSDLRequiredExtensions(reqInstanceExt);

    // Add "VK_EXT_debug_utils" extension if validation layers are enabled
    if constexpr (Optim::VK::enableValidationLayers) {
      reqInstanceExt.EmplaceBack(vk::EXTDebugUtilsExtensionName);
    }

    if (!Optim::VK::ValidateRequiredExtensions(reqInstanceExt, ctx.context)) {
      throw std::runtime_error("[VulkanRHI | Error] Required extensions are not supported by Vulkan.");
    }

    // Required Layers Verification
    TDynamicArray<String> requiredLayers;

    // Add "VK_LAYER_KHRONOS_validation" if validation layers are enabled
    if constexpr (Optim::VK::enableValidationLayers) {
      requiredLayers.EmplaceBack("VK_LAYER_KHRONOS_validation");
    }

    if (Optim::VK::ValidateRequiredLayers(requiredLayers, ctx.context) == false) {
      throw std::runtime_error("[VulkanRHI | Error] Required layers are not supported by Vulkan.");
    }

    // Create VkInstance object
    ctx.instance = Optim::VK::CreateVkInstance(ctx.context, reqInstanceExt, requiredLayers, Optim::VK::GetApplicationInfoStruct());
    if (ctx.instance == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to initialize VkInstance object.");
    }

    // Create VkDebugUtilsMessengerEXT object if validation layers are enabled
    if constexpr (Optim::VK::enableValidationLayers) {
      ctx.debugMessenger = Optim::VK::Debug::CreateVkDebugUtilsMessengerEXT(ctx.instance);
      if (ctx.debugMessenger == nullptr) {
        throw std::runtime_error("[VulkanRHI | Error] Debug messenger FAILED to initialize.");
      }
    }

    // Create VKSurfaceKHR object
    ctx.surface = Optim::VK::CreateVkSurfaceKHR(ctx.instance, sdlwindow);
    if (ctx.surface == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to initialize VkSurfaceKHR object.");
    }

    // Select most optimal VkPhysicalDevice object
    auto physicalDevice = Optim::VK::SelectVkPhysicalDevice(ctx.instance, ctx.surface);
    if (physicalDevice == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to find usable GPU for rendering.");
    }

    // Create RenderDevice object
    ctx.renderContext = Optim::VK::CreateRenderDevice(physicalDevice, ctx.surface);
    if (!ctx.renderContext.IsValid()) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to create RenderDevice object.");
    }

    // Create SwapChainContext object
    ctx.swapChainContext = Optim::VK::CreateSwapChainContext(ctx.renderContext.device, ctx.renderContext.physicalDevice, ctx.surface, sdlwindow);

    // Create Image Views for each image in the swap chain
    Optim::VK::CreateImageViews(ctx.swapChainContext, ctx.renderContext.device);
    if (!ctx.swapChainContext.IsValid()) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to create swap chain context.");
    }

    // Pipeline creation
    //  * Load Shader
    //  * Create `VkShaderModule`
    //  * Crate Shader Stages Info
    //  * Use Data to create a `VkPipeline` Object

    auto shadercode       = Optim::VKPipeline::LoadCompiledShader("Shaders/sample.spv");
    auto ShaderModule     = Optim::VKPipeline::CreateVkShaderModule(shadercode, ctx.renderContext.device);
    auto shaderStagesInfo = Optim::VKPipeline::CreateVkPipelineShaderStageCreateInfoList(ShaderModule);

    ctx.pipeline = Optim::VKPipeline::CreateVulkanPipeline(ctx.swapChainContext, shaderStagesInfo, ctx.renderContext.device);
    if (ctx.pipeline == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to create VkPipeline object.");
    }
  }
  catch (const vk::SystemError& e) {
    std::cerr << "[Vulkan | Error] " << e.what() << '\n';
    return;
  }
  catch (const std::exception& e) {
    std::cerr << "[VulkanRHI | Error] " << e.what() << '\n';
    return;
  }
}

void VulkanRHI::Update()
{
}
void VulkanRHI::Cleanup()
{
}