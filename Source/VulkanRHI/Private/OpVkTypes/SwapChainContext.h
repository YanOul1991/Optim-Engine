#pragma once

#include "OpVkCommon/Minimal.h"

// Forward declare SDL_Window struct.
struct SDL_Window;

/**
 * SwapChainContext Objects wraps
 */
struct SwapChainContext final
{
  vk::raii::SwapchainKHR           swapChain = nullptr;
  std::vector<vk::Image>           swapChainImages;
  std::vector<vk::raii::ImageView> swapChainImageViews;
  vk::SurfaceFormatKHR             swapChainImageFormat;
  vk::Extent2D                     swapChainExtent;

  SwapChainContext() = default;
  // Yes move :D
  SwapChainContext(SwapChainContext&&) noexcept            = default;
  SwapChainContext& operator=(SwapChainContext&&) noexcept = default;
  // No copy >:(
  SwapChainContext(const SwapChainContext&)            = delete;
  SwapChainContext& operator=(const SwapChainContext&) = delete;

  /**
   * @brief
   * Constructor creating a new SwapChainContext from scratch.
   */
  SwapChainContext(
    const vk::raii::Device&         device,
    const vk::raii::PhysicalDevice& physicalDevice,
    const vk::raii::SurfaceKHR&     surface,
    SDL_Window*                     pWindow);

  /**
   * TODO: 
   *  Implement...
   * 
   * @brief
   * Constructor creating a new SwapChainContext from already existing 
   * swapchain context.
   */
  // SwapChainContext(
  //   const vk::raii::Device&         device,
  //   const vk::raii::PhysicalDevice& physicalDevice,
  //   const vk::raii::SurfaceKHR&     surface,
  //   SDL_Window*                     pWindow,
  //   const SwapChainContext&         oldContext);

  [[nodiscard]]
  inline bool IsValid() const noexcept
  {
    return swapChain != nullptr &&
           !swapChainImages.empty() &&
           !swapChainImageViews.empty() &&
           swapChainImageFormat.format != vk::Format::eUndefined &&
           swapChainExtent.width > 0 &&
           swapChainExtent.height > 0;
  }
};