#pragma once

#include "OpVkCommon/Minimal.h"

struct SDL_Window;

namespace Optim::VK
{
    
vk::SurfaceFormatKHR SelectSwapChainVkSurfaceKHRFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

/**
 * Vulkan presentation modes
 *
 * Vulkan Tutorial
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html#_presentation_mode
 */
vk::PresentModeKHR SelectSwapChainVkPresentModeKHR(const std::vector<vk::PresentModeKHR>& availablePresentModes);

/**
 * Swap chain extent:
 *  Resolution of swap chain images.
 *
 * Vulkan Tutorial:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html#_swap_extent
 *
 * struct `VkSurfaceCapabilitiesKHR` documentation def:
 * https://docs.vulkan.org/refpages/latest/refpages/source/VkSurfaceCapabilitiesKHR.html
 */
vk::Extent2D SelectSwapChainVkExtend2D(const vk::SurfaceCapabilitiesKHR& capabilities, SDL_Window* pWindow);

/**
 * Defines how many images will be in the swap chain. Because Vulkan implemention
 * has a minimal and a maximal amount this function helps select a correct amount.
 */
uint32 SelectSwapChainMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities);

/**
 * Querying details of swap chain support.
 *
 * Settings to determine:
 *   * Surface format (color depth)
 *   * Presentation mode (conditions for "swapping" images to the screen)
 *   * Swap extent (resolution of images in swapchain)
 *
 * Vulkan Tutorial Example:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html#_querying_details_of_swap_chain_support
 */
vk::raii::SwapchainKHR CreateVkSwapChainKHR(const vk::raii::Device&         device,
                                            const vk::raii::PhysicalDevice& physicalDevice,
                                            const vk::raii::SurfaceKHR&     surface,
                                            SDL_Window*                     pWindow);

} // namespace Optim::VK