#pragma once

#include "OpVkCommon/Minimal.h"

struct SwapChainContext
{
  vk::raii::SwapchainKHR swapChain = nullptr;
  std::vector<vk::Image> swapChainImages;
  vk::SurfaceFormatKHR   swapChainImageFormat;
  vk::Extent2D           swapChainExtent;

  inline bool IsValid() const
  {
    return (swapChain != nullptr) && (!swapChainImages.empty());
  }
};