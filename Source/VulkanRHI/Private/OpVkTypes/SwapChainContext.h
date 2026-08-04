#pragma once

#include "OpVkCommon/Minimal.h"

struct SwapChainContext
{
  vk::raii::SwapchainKHR           swapChain = nullptr;
  std::vector<vk::Image>           swapChainImages;
  std::vector<vk::raii::ImageView> swapChainImageViews;
  vk::SurfaceFormatKHR             swapChainImageFormat;
  vk::Extent2D                     swapChainExtent;

  inline bool IsValid() const
  {
    return (swapChain != nullptr) &&
           (!swapChainImages.empty()) &&
           (!swapChainImageViews.empty()) &&
           (swapChainImageFormat.format != vk::Format::eUndefined) &&
           (swapChainExtent.width > 0) &&
           (swapChainExtent.height > 0);
  }
};