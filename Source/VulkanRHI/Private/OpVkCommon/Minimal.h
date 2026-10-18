#pragma once

#include "Core/CoreMinimal.h"
#include "Core/Debug/Debug.h"
#include <glm/glm.hpp>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#if defined(__INTELISENSE__) || !defined(USE_CPP20_MODULES)
#  include <vulkan/vulkan_raii.hpp>
#else
#  import vulkan_hpp
#endif

// Since using vk::raii all wrapper objects always define the same constructors and 
// assignement opeators, wrap them in helper macro.
#define OPTIM_VK_RAII_TYPE(_CLASS_)             \
  _CLASS_()                          = default; \
  ~_CLASS_()                         = default; \
  _CLASS_(_CLASS_&&)                 = default; \
  _CLASS_& operator=(_CLASS_&&)      = default; \
  _CLASS_(const _CLASS_&)            = delete;  \
  _CLASS_& operator=(const _CLASS_&) = delete;

namespace Optim::VK
{

constexpr uint32 ApiVersion = vk::ApiVersion14;

#if defined(NDEBUG)
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr int32 maxFramesInFlight = 2;

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

struct Vertex
{
  glm::vec2 position;
  glm::vec3 color;

  static vk::VertexInputBindingDescription GetBindingDescription()
  {
    return {.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex };
  }

  static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescription()
  {
    return{{
      {.location = 0, .binding =0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, position)},
      {.location = 1, .binding =0 , .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)}
    }};
  }
};
