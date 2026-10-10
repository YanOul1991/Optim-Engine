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

  SwapChainResources()                                       = default;
  SwapChainResources(SwapChainResources&&) noexcept            = default;
  SwapChainResources& operator=(SwapChainResources&&) noexcept = default;

  SwapChainResources(const SwapChainResources&)            = delete;
  SwapChainResources& operator=(const SwapChainResources&) = delete;

  /**
   * @brief
   * Constructor creating a new SwapChainContext from scratch.
   */
  SwapChainResources(
    const vk::raii::Device&         device,
    const vk::raii::PhysicalDevice& physicalDevice,
    const vk::raii::SurfaceKHR&     surface,
    SDL_Window*                     pWindow);

  /**
   * TODO:
   *  Implement...
   *
   * @brief
   * Constructor creating a new SwapChainResources from already existing
   * swapchain context.
   */
  // SwapChainResources(
  //   const vk::raii::Device&         device,
  //   const vk::raii::PhysicalDevice& physicalDevice,
  //   const vk::raii::SurfaceKHR&     surface,
  //   SDL_Window*                     pWindow,
  //   const SwapChainResources&         oldContext);

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