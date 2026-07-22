#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#if defined(__INTELISENSE__) || !defined(USE_CPP20_MODULES)
#  include <vulkan/vulkan_raii.hpp>
#else
#  import vulkan_hpp
#endif

namespace Optim::VK {

constexpr vk::ApplicationInfo GetApplicationInfoStruct() {
  vk::ApplicationInfo appInfo;
  appInfo.pApplicationName   = "Vulkan Learning";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.pEngineName        = "Optim Engine";
  appInfo.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
  appInfo.apiVersion         = vk::ApiVersion14;
  return appInfo;
}

} // namespace Optim::VK