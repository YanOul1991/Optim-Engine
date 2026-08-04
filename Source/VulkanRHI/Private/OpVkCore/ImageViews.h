#pragma once

#include "OpVkCommon/Minimal.h"
#include "OpVkTypes/SwapChainContext.h"

namespace Optim::VK
{

/**
 * Creates image views for each image in the swap chain and stores them in the provided SwapChainContext.
 *
 * Vulkan Tutorial:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/02_Image_views.html
 *
 * Documentation for VkImageSubresourceRange:
 * https://docs.vulkan.org/refpages/latest/refpages/source/VkImageSubresourceRange.html
 */
inline void CreateImageViews(SwapChainContext& swapChainContext, const vk::raii::Device& device)
{
  assert(swapChainContext.swapChainImageViews.empty() && "Image views have already been created.");

  vk::ImageViewCreateInfo imageViewCreateInfo{
    .viewType         = vk::ImageViewType::e2D,
    .format           = swapChainContext.swapChainImageFormat.format,
    .components       = { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity },
    .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
  };

  for (auto& image : swapChainContext.swapChainImages) {
    imageViewCreateInfo.image = image;

    // Create the image view and add it to the vector
    swapChainContext.swapChainImageViews.emplace_back(device.createImageView(imageViewCreateInfo));
  }
}

}; // namespace Optim::VK