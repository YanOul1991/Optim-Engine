#include "./SwapChainContext.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

/**
 * @brief Selects the appropriate Vulkan surface format for the swap chain.
 */
static inline vk::SurfaceFormatKHR SelectSwapChainVkSurfaceKHRFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
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
static inline vk::PresentModeKHR SelectSwapChainVkPresentModeKHR(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
  // Check that the swap chain supports a FIFO presentation mode, if so
  // automatically pick it as the presentation mode for the swap chain.
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
vk::Extent2D SelectSwapChainVkExtend2D(const vk::SurfaceCapabilitiesKHR& capabilities, SDL_Window* pWindow)
{
  if (capabilities.currentExtent.width != (std::numeric_limits<uint32>::max)()) {
    return capabilities.currentExtent;
  }

  //  Get the size of the active SDL window in pixels.
  //  TODO:
  //    - Implement external function in swap chain to
  //      update the size of the window when swap chain
  //      resize will be implemented.
  int32 width, height;
  SDL_GetWindowSizeInPixels(pWindow, &width, &height);

  // Return the desired swap chain size while making sure it stays withing the
  // minimum and maximum supported range.
  return {
    std::clamp<uint32>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
    std::clamp<uint32>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
  };
}

/**
 * Defines how many images will be in the swap chain. Because Vulkan implemention
 * has a minimal and a maximal amount this function helps select a correct amount.
 */
uint32 SelectSwapChainMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities)
{
  //
  uint32 minImgCount = std::max(3u, capabilities.minImageCount);

  // If maxImageCount is more that 0 and smaller than the desired minImgCount
  // then use the maxImageCount for the swap chain.
  if ((0 < capabilities.maxImageCount) && (capabilities.maxImageCount < minImgCount)) {
    return capabilities.maxImageCount;
  }

  return minImgCount;
}

SwapChainContext::SwapChainContext(
  const vk::raii::Device&         device,
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
  auto selectedSwapChainFormat        = SelectSwapChainVkSurfaceKHRFormat(surfaceFormats);
  auto selectedSwapChainPresentMode   = SelectSwapChainVkPresentModeKHR(surfacePresentModes);
  auto selectedSwapChainExtent        = SelectSwapChainVkExtend2D(surfaceCapabilities, pWindow);
  auto selectedSwapChainMinImageCount = SelectSwapChainMinImageCount(surfaceCapabilities);

  // Swap Chain create info
  vk::SwapchainCreateInfoKHR swapChainCreateInfo{
    .surface          = *surface,
    .minImageCount    = selectedSwapChainMinImageCount,
    .imageFormat      = selectedSwapChainFormat.format,
    .imageColorSpace  = selectedSwapChainFormat.colorSpace,
    .imageExtent      = selectedSwapChainExtent,
    .imageArrayLayers = 1,
    .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
    .imageSharingMode = vk::SharingMode::eExclusive,
    .preTransform     = surfaceCapabilities.currentTransform,
    .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
    .presentMode      = selectedSwapChainPresentMode,
    .clipped          = true
  };

  this->swapChain            = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
  this->swapChainImages      = this->swapChain.getImages();
  this->swapChainImageFormat = selectedSwapChainFormat;
  this->swapChainExtent      = selectedSwapChainExtent;

  // Create the Image views
  vk::ImageViewCreateInfo imageViewCreateInfo{
    .viewType         = vk::ImageViewType::e2D,
    .format           = this->swapChainImageFormat.format,
    .components       = { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity },
    .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
  };

  for (auto& image : this->swapChainImages) {
    imageViewCreateInfo.image = image;
    this->swapChainImageViews.emplace_back(device.createImageView(imageViewCreateInfo));
  }
}

/**
 * Initilize the SwapChainContext struct and its underlying Vulkan objects.
 */
void SwapChainContext::CreateSwapChainContext(
  const vk::raii::Device&         device,
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
  auto selectedSwapChainFormat        = SelectSwapChainVkSurfaceKHRFormat(surfaceFormats);
  auto selectedSwapChainPresentMode   = SelectSwapChainVkPresentModeKHR(surfacePresentModes);
  auto selectedSwapChainExtent        = SelectSwapChainVkExtend2D(surfaceCapabilities, pWindow);
  auto selectedSwapChainMinImageCount = SelectSwapChainMinImageCount(surfaceCapabilities);

  // Swap Chain create info
  vk::SwapchainCreateInfoKHR swapChainCreateInfo{
    .surface          = *surface,
    .minImageCount    = selectedSwapChainMinImageCount,
    .imageFormat      = selectedSwapChainFormat.format,
    .imageColorSpace  = selectedSwapChainFormat.colorSpace,
    .imageExtent      = selectedSwapChainExtent,
    .imageArrayLayers = 1,
    .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
    .imageSharingMode = vk::SharingMode::eExclusive,
    .preTransform     = surfaceCapabilities.currentTransform,
    .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
    .presentMode      = selectedSwapChainPresentMode,
    .clipped          = true
  };

  this->swapChain            = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
  this->swapChainImages      = this->swapChain.getImages();
  this->swapChainImageFormat = selectedSwapChainFormat;
  this->swapChainExtent      = selectedSwapChainExtent;

  // Create the Image views
  vk::ImageViewCreateInfo imageViewCreateInfo{
    .viewType         = vk::ImageViewType::e2D,
    .format           = this->swapChainImageFormat.format,
    .components       = { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity },
    .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
  };

  for (auto& image : this->swapChainImages) {
    imageViewCreateInfo.image = image;
    this->swapChainImageViews.emplace_back(device.createImageView(imageViewCreateInfo));
  }
}