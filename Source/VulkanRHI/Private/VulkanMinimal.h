#pragma once

#include "OptimEngine/Core/CoreMinimal.h"

#if defined(PLATFORM_WINDOWS)
#  define VK_USE_PLATFORM_WIN32_KHR
#elif defined(PLATFORM_LINUX)
#  define VK_USE_PLATFORM_X11_KHR
#  define VK_USE_PLATFORM_WAYLAND_KHR
#endif

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#if defined(__INTELISENSE__) || !defined(USE_CPP20_MODULES)
#  include <vulkan/vulkan_raii.hpp>
#else
#  import vulkan_hpp
#endif

namespace Optim::VK
{

constexpr uint32 ApiVersion = vk::ApiVersion14;

#if defined(NDEBUG)
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr vk::ApplicationInfo GetApplicationInfoStruct()
{
  vk::ApplicationInfo appInfo;
  appInfo.pApplicationName   = "Vulkan Learning";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.pEngineName        = "Optim Engine";
  appInfo.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
  appInfo.apiVersion         = Optim::VK::ApiVersion;
  return appInfo;
}

} // namespace Optim::VK
