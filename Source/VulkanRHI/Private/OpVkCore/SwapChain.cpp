#pragma once

#include "OpVkCore/SwapChain.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

vk::SurfaceFormatKHR Optim::VK::SelectSwapChainVkSurfaceKHRFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
  assert(!availableFormats.empty());
  for (auto& surfaceFormat : availableFormats) {
    if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return surfaceFormat;
    }
  }

  return availableFormats[0];
}

/**
 * Vulkan presentation modes
 *
 * Vulkan Tutorial
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html#_presentation_mode
 */
vk::PresentModeKHR Optim::VK::SelectSwapChainVkPresentModeKHR(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
  assert(std::ranges::any_of(availablePresentModes, [](const vk::PresentModeKHR& presentMode) {
    return presentMode == vk::PresentModeKHR::eFifo;
  }));

  for (auto& presentMode : availablePresentModes) {
    if (presentMode == vk::PresentModeKHR::eMailbox) {
      return presentMode;
    }
  }

  return vk::PresentModeKHR::eFifo;
}

/**
 * Swap chain extent:
 *  Resolution of swap chain images.
 *
 * Vulkan Tutorial:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html#_swap_extent
 *
 * struct `VkSurfaceCapabilitiesKHR` documentation def:
 * https://docs.vulkan.org/refpages/latest/refpages/source/VkSurfaceCapabilitiesKHR.html
 */
vk::Extent2D Optim::VK::SelectSwapChainVkExtend2D(const vk::SurfaceCapabilitiesKHR& capabilities, SDL_Window* pWindow)
{
  if (capabilities.currentExtent.width != (std::numeric_limits<uint32>::max)()) {
    return capabilities.currentExtent;
  }

  int32 width, height;
  SDL_GetWindowSizeInPixels(pWindow, &width, &height);

  return {
    std::clamp<uint32>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
    std::clamp<uint32>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
  };
}

/**
 * Defines how many images will be in the swap chain. Because Vulkan implemention
 * has a minimal and a maximal amount this function helps select a correct amount.
 */
uint32 Optim::VK::SelectSwapChainMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities)
{
  uint32 minImgCount = std::max(3u, capabilities.minImageCount);

  // If maxImageCount is more that 0 and smaller than the desired minImgCount
  // then use the maxImageCount for the swap chain.
  if ((0 < capabilities.maxImageCount) && (capabilities.maxImageCount < minImgCount)) {
    return capabilities.maxImageCount;
  }

  return minImgCount;
}

/**
 * Querying details of swap chain support.
 *
 * Settings to determine:
 *   * Surface format (color depth)
 *   * Presentation mode (conditions for "swapping" images to the screen)
 *   * Swap extent (resolution of images in swapchain)
 *
 * Vulkan Tutorial Example:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html#_querying_details_of_swap_chain_support
 */
vk::raii::SwapchainKHR Optim::VK::CreateVkSwapChainKHR(const vk::raii::Device&         device,
                                                       const vk::raii::PhysicalDevice& physicalDevice,
                                                       const vk::raii::SurfaceKHR&     surface,
                                                       SDL_Window*                     pWindow)
{
  // Get surface available basic capabilities
  auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
  // Get surface available formats
  auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
  // Get surface available present modes
  auto surfacePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);

  // Select optimal settings
  auto swapChainFormat        = Optim::VK::SelectSwapChainVkSurfaceKHRFormat(surfaceFormats);
  auto swapChainPresentMode   = Optim::VK::SelectSwapChainVkPresentModeKHR(surfacePresentModes);
  auto swapChainExtent        = Optim::VK::SelectSwapChainVkExtend2D(surfaceCapabilities, pWindow);
  auto swapChainMinImageCount = Optim::VK::SelectSwapChainMinImageCount(surfaceCapabilities);

  // Swap Chain create info
  vk::SwapchainCreateInfoKHR swapChainCreateInfo{
    .surface          = *surface,
    .minImageCount    = swapChainMinImageCount,
    .imageFormat      = swapChainFormat.format,
    .imageColorSpace  = swapChainFormat.colorSpace,
    .imageExtent      = swapChainExtent,
    .imageArrayLayers = 1,
    .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
    .imageSharingMode = vk::SharingMode::eExclusive,
    .preTransform     = surfaceCapabilities.currentTransform,
    .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
    .presentMode      = swapChainPresentMode,
    .clipped          = true
  };

  return vk::raii::SwapchainKHR(device, swapChainCreateInfo);
}