#pragma once

#include "OpVkCommon/Minimal.h"
#include "Core/CoreMinimal.h"

struct SDL_Window;

namespace Optim::VK
{

/**
 * @brief
 * Creates a Vulkan surface object using the SDL function 
 * `SDL_Vulkan_CreateSurface`.This function abstracts the need to perform 
 * platform specific operations, figuring out the display service, defining the
 * correct macros, vulkan functions, etc.
 */
[[deprecated("Use Optim::CreateVkSurfaceKHR instead.")]]
vk::raii::SurfaceKHR CreateVkSurfaceKHR(const vk::raii::Instance& instance, SDL_Window* sdlwindow);

/**
 * @brief 
 * Create a `vk::raii::Instance` object. This function, validates all required
 * extensions such as the ones from SDL, and verifies all required Layers.
 * 
 * May through a `std::runtime_error` if anu required extension or layer is not
 * supported.
 */
[[deprecated("Use Optim::CreateVkInstance instead.")]]
vk::raii::Instance CreateVkInstance(const vk::raii::Context& context);

}; // namespace Optim::VK

namespace Optim
{

[[nodiscard]]
vk::raii::Instance CreateVkInstance(const vk::raii::Context& context);

[[nodiscard]]
vk::raii::SurfaceKHR CreateVkSurfaceKHR(const vk::raii::Instance& instance, SDL_Window* sdlwindow);

[[nodiscard]]
vk::raii::PhysicalDevice SelectVkPhysicalDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);

/**
 * Because the `vk::raii::Device` and `vk::raii::Queue` objects are tightly 
 * coupled, this function creates both objects at the same time. The function
 *  will also return the index of the queue family that was used to create the 
 * queue.
 */
bool CreateVkDeviceAndQueue(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface, vk::raii::Device& outDevice, vk::raii::Queue& outQueue, uint32& outQueueIndex);

[[nodiscard]]
vk::raii::Queue CreateVkQueue(const vk::raii::PhysicalDevice& physicalDevice, uint32& outQueueIndex);

[[nodiscard]]
vk::raii::Device CreateVkDevice(const vk::raii::PhysicalDevice& physicalDevice, uint32& queueIndex);

};