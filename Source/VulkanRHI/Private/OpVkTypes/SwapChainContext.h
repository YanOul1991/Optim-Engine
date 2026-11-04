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

  inline vk::ImageMemoryBarrier2 TransitionImageLayout(
    uint32 inImageIndex,
    // vk::ImageLayout         oldLayout,
    vk::ImageLayout         newLayout,
    vk::AccessFlags2        srcAcesssMask,
    vk::AccessFlags2        dstAcessMask,
    vk::PipelineStageFlags2 srcStageMask,
    vk::PipelineStageFlags2 dstStageMask)
  {
    return vk::ImageMemoryBarrier2{
      .srcStageMask        = srcStageMask,
      .srcAccessMask       = srcAcesssMask,
      .dstStageMask        = dstStageMask,
      .dstAccessMask       = dstAcessMask,
      .oldLayout           = imageLayouts[inImageIndex],
      .newLayout           = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image               = images[inImageIndex],
      .subresourceRange    = {
                              .aspectMask     = vk::ImageAspectFlagBits::eColor,
                              .baseMipLevel   = 0,
                              .levelCount     = 1,
                              .baseArrayLayer = 0,
                              .layerCount     = 1 }
    };
  }

  inline vk::raii::SwapchainKHR&                 SwapChain()               noexcept       { return swapChain;} 
  inline std::vector<vk::Image>&                 Images()                  noexcept       { return images;} 
  inline std::vector<vk::raii::ImageView>&       ImageViews()              noexcept       { return imageViews;} 
  inline std::vector<vk::raii::Semaphore>&       RenderFinishedSemaphore() noexcept       { return renderFinishedSemaphores;} 
  inline vk::SurfaceFormatKHR&                   ImageFormat()             noexcept       { return imageFormat;} 
  inline vk::Extent2D&                           ImageExtent()             noexcept       { return extent;} 

  inline const vk::raii::SwapchainKHR&           SwapChain()               const noexcept { return swapChain;} 
  inline const std::vector<vk::Image>&           Images()                  const noexcept { return images;} 
  inline const std::vector<vk::raii::ImageView>& ImageViews()              const noexcept { return imageViews;} 
  inline const std::vector<vk::raii::Semaphore>& RenderFinishedSemaphore() const noexcept { return renderFinishedSemaphores;} 
  inline const vk::SurfaceFormatKHR&             ImageFormat()             const noexcept { return imageFormat;} 
  inline const vk::Extent2D&                     ImageExtent()             const noexcept { return extent;}

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

/**
 * This struct will be deprecated
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */
// struct SwapChainResources final
// {
//   vk::raii::SwapchainKHR           swapChain = nullptr;
//   std::vector<vk::Image>           images;
//   std::vector<vk::raii::ImageView> imageViews;
//   std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
//   vk::SurfaceFormatKHR             imageFormat;
//   vk::Extent2D                     extent;

//   SwapChainResources()                                         = default;
//   ~SwapChainResources()                                        = default;
//   SwapChainResources(SwapChainResources&&) noexcept            = default;
//   SwapChainResources& operator=(SwapChainResources&&) noexcept = default;
//   SwapChainResources(const SwapChainResources&)                = delete;
//   SwapChainResources& operator=(const SwapChainResources&)     = delete;
//   SwapChainResources(std::uintptr_t){};

//   SwapChainResources(vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice, vk::raii::SurfaceKHR& surface, SDL_Window* pWindow);

//   [[nodiscard]]
//   inline bool IsValid() const noexcept
//   {
//     return swapChain != nullptr &&
//            !images.empty() &&
//            !imageViews.empty() &&
//            !renderFinishedSemaphores.empty() &&
//            imageFormat.format != vk::Format::eUndefined &&
//            extent.width > 0 &&
//            extent.height > 0;
//   }
// };