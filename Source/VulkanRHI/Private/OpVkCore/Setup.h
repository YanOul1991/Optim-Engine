#pragma once

#include "OpVkCommon/Minimal.h"
#include "Core/CoreMinimal.h"

struct SDL_Window;

namespace Optim
{
  
struct ResultCreateVkDeviceAndQueue final
{
  vk::raii::Device device     = nullptr;
  vk::raii::Queue  queue      = nullptr;
  uint32           queueIndex = ~0u;
};


/**
 * @brief
 * Validates all required extensions such as the ones from SDL, and verifies all required Layers.
 * 
 * @param context
 * The Vulkan context used to enumerate available layers and extensions.
 * 
 * @return 
 * `vk::raii::Instance`
 * 
 * @throws 
 * `std::runtime_error` if one or more required extensions or layers are not supported
 */
[[nodiscard]]
vk::raii::Instance CreateVkInstance(const vk::raii::Context& context);

/**
 * @brief
 * Creates a Vulkan surface for the given SDL window.
 * 
 * @param instance 
 * The Vulkan instance used to create the surface.
 * 
 * @param sdlwindow 
 * The SDL window for which to create the Vulkan surface.
 * 
 * @return `vk::raii::SurfaceKHR` The created Vulkan surface.
 */
[[nodiscard]]
vk::raii::SurfaceKHR CreateVkSurfaceKHR(const vk::raii::Instance& instance, SDL_Window* sdlwindow);

/**
 * @brief
 * Selects the most suitable physical device (GPU) for rendering based on the
 * available devices and their capabilities. The selection criteria may include
 * factors such as device type, supported features, and compatibility with the
 * application's requirements.
 * 
 * @param instance 
 * The Vulkan instance used to enumerate physical devices.
 * 
 * @param surface
 * The Vulkan surface associated with the window, used to determine the appropriate queue family for presentation.
 * 
 * @return
 * `vk::raii::PhysicalDevice` The selected physical device (GPU) that meets
 */
[[nodiscard]]
vk::raii::PhysicalDevice SelectVkPhysicalDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);

/**
 * @brief
 * Because the `vk::raii::Device` and `vk::raii::Queue` objects are tightly
 * coupled, this function creates a struct containing both objects and the 
 * queue index. This approach ensures that the logical device and its 
 * associated queue are created together, reducing the risk of mismanagement.
 * 
 * @param physicalDevice 
 * The selected physical device (GPU) for which the logical device and queue will be created.
 * 
 * @param surface 
 * The Vulkan surface associated with the window, used to determine the appropriate queue family for presentation.
 * 
 * @return 
 * `ResultCreateVkDeviceAndQueue` A struct containing the created logical device, the graphics queue, and the index of the queue family.
 */
[[nodiscard]]
Optim::ResultCreateVkDeviceAndQueue CreateVkDeviceAndQueue(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface);

};