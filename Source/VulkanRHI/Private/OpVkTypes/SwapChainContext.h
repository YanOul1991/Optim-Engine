#pragma once

#include "OpVkCommon/Minimal.h"
#include <tuple>

// Forward declare SDL_Window struct.
struct SDL_Window;

namespace Optim
{

class SwapChainResource final
{
 public:
  SwapChainResource()                                    = default;
  ~SwapChainResource()                                   = default;
  SwapChainResource(SwapChainResource&&)                 = default;
  SwapChainResource& operator=(SwapChainResource&&)      = default;
  SwapChainResource(const SwapChainResource&)            = delete;
  SwapChainResource& operator=(const SwapChainResource&) = delete;
  SwapChainResource(std::nullptr_t) {};

  SwapChainResource(vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice, vk::raii::SurfaceKHR& surface, SDL_Window* pWindow);

  /**
   * @brief
   * Wrapper function for `vk::raii::SwapchainKHR::acquireNextImage()`
   * 
   * @param signalSemaphore 
   * Semaphore to signal once the requested image is ready to be used and can
   * start rendering into it. Most likely for command recording.
   * 
   * @param singalFence
   * Fence to signal once the requested image is ready to be used and can start.
   */
  inline uint32 GetNextImageIndex(const vk::raii::Semaphore& signalSemaphore, const vk::raii::Fence& signalFence = nullptr) const noexcept
  {
    auto[result, index] = swapChain.acquireNextImage(UINT64_MAX, *signalSemaphore, nullptr);
    if (result != vk::Result::eSuccess) {
      return ~0u;
    }
    return index;
  }

  inline vk::raii::SwapchainKHR&                 SwapChain()               noexcept       { return swapChain;} 
  inline std::vector<vk::Image>&                 Images()                  noexcept       { return images;} 
  inline std::vector<vk::raii::ImageView>&       ImageViews()              noexcept       { return imageViews;} 
  inline std::vector<vk::raii::Semaphore>&       RenderFinishedSemaphore() noexcept       { return renderFinishedSemaphores;} 
  inline vk::SurfaceFormatKHR&                   ImageFormat()             noexcept       { return imageFormat;} 
  inline vk::Extent2D&                           ImageExtent()             noexcept       { return extent;} 

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

 private:
  vk::raii::SwapchainKHR           swapChain = nullptr;
  std::vector<vk::Image>           images;
  std::vector<vk::raii::ImageView> imageViews;
  std::vector<vk::ImageLayout>     imageLayouts;
  std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
  vk::SurfaceFormatKHR             imageFormat;
  vk::Extent2D                     extent;
};

}; // namespace Optim