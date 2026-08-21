#pragma once

#include "OpVkCommon/Minimal.h"
#include "OptimEngine/Core/CoreMinimal.h"

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
vk::raii::SurfaceKHR CreateVkSurfaceKHR(const vk::raii::Instance& instance, SDL_Window* sdlwindow);

/**
 * @brief 
 * Create a `vk::raii::Instance` object. This function, validates all required
 * extensions such as the ones from SDL, and verifies all required Layers.
 * 
 * May through a `std::runtime_error` if anu required extension or layer is not
 * supported.
 */
vk::raii::Instance CreateVkInstance(const vk::raii::Context& context);

}; // namespace Optim::VK