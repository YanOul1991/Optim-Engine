#pragma once

#include "OpVkCommon/Minimal.h"

// Forward declare SDL_Window struct.
struct SDL_Window;

/**
 * SwapChainContext Objects wraps
 */
struct SwapChainResources final
{
  vk::raii::SwapchainKHR           swapChain = nullptr;
  std::vector<vk::Image>           images;
  std::vector<vk::raii::ImageView> imageViews;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
  vk::SurfaceFormatKHR             imageFormat;
  vk::Extent2D                     extent;

  SwapChainResources()                                         = default;
  ~SwapChainResources()                                        = default;
  SwapChainResources(SwapChainResources&&) noexcept            = default;
  SwapChainResources& operator=(SwapChainResources&&) noexcept = default;
  SwapChainResources(const SwapChainResources&)                = delete;
  SwapChainResources& operator=(const SwapChainResources&)     = delete;

  SwapChainResources(vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice, vk::raii::SurfaceKHR& surface, SDL_Window* pWindow);

  [[nodiscard]]
  inline bool IsValid() const noexcept
  {
    return swapChain != nullptr &&
           !images.empty() &&
           !imageViews.empty() &&
           !renderFinishedSemaphores.empty() &&
           imageFormat.format != vk::Format::eUndefined &&
           extent.width > 0 &&
           extent.height > 0;
  }
};