/**
 * FILE: VulkanRHI.cpp
 */

#include "VulkanRHI/VulkanRHI.h"

#include "OptimVKDebug.h"
#include "OptimVKSetup.h"

// SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

// STD
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <ranges>
#include <vector>

struct VulkanRHI::Impl
{
  vk::raii::Context    context;
  vk::raii::Instance   instance = nullptr;
  vk::raii::SurfaceKHR surface  = nullptr;

  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

  RenderDevice renderDevice;

  vk::raii::SwapchainKHR swapChain = nullptr;
  std::vector<vk::Image> swapChainImages;
};

namespace Optim::VK
{
vk::SurfaceFormatKHR SelectSwapChainVkSurfaceKHRFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
  assert(!availableFormats.empty());
  for (auto& surfaceFormat : availableFormats) {
    if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      std::cout << "  Found best Format(color space: " << to_string(surfaceFormat.colorSpace) << " | format: " << to_string(surfaceFormat.format) << ")\n";
      return surfaceFormat;
    }
  }

  return availableFormats[0];
}

/**
 * Vulkan presentation modes
 *
 * Vulkan Tutorial
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html#_presentation_mode
 */
vk::PresentModeKHR SelectSwapChainVkPresentModeKHR(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
  assert(std::ranges::any_of(availablePresentModes, [](const vk::PresentModeKHR& presentMode) {
    return presentMode == vk::PresentModeKHR::eFifo;
  }));

  for (auto& presentMode : availablePresentModes) {
    if (presentMode == vk::PresentModeKHR::eMailbox) {
      return presentMode;
    }
  }

  return vk::PresentModeKHR::eFifo;
}

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
vk::Extent2D SelectSwapChainVkExtend2D(const vk::SurfaceCapabilitiesKHR& capabilities, SDL_Window*& pWindow)
{
  // std::cout << "Current Surface Capabilities info\n";
  // std::cout << "  Current extent width " << capabilities.currentExtent.width << '\n';
  // std::cout << "  Current extent height " << capabilities.currentExtent.width << '\n';

  if (capabilities.currentExtent.width != (std::numeric_limits<uint32>::max)()) {
    return capabilities.currentExtent;
  }

  int32 width, height;
  SDL_GetWindowSizeInPixels(pWindow, &width, &height);

  return {
    std::clamp<uint32>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
    std::clamp<uint32>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
  };
}

/**
 * Defines how many images will be in the swap chain. Because Vulkan implemention
 * has a minimal and a maximal amount this function helps select a correct amount.
 */
uint32 SelectSwapChainMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities)
{
  uint32 minImgCount = std::max(3u, capabilities.minImageCount);

  // If maxImageCount is more that 0 and smaller than the desired minImgCount
  // then use the maxImageCount for the swap chain.
  if ((0 < capabilities.maxImageCount) && (capabilities.maxImageCount < minImgCount)) {
    return capabilities.maxImageCount;
  }

  return minImgCount;
}

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
vk::raii::SwapchainKHR CreateVkSwapChainKHR(const vk::raii::Device& device, const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface, SDL_Window*& pWindow)
{
  std::cout << "Querying QueryVkSurfaceKHRCapabilities\n";

  // Get surface available basic capabilities
  auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
  // Get surface available formats
  auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
  // Get surface available present modes
  auto surfacePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);

  // Select optimal settings
  auto swapChainFormat        = SelectSwapChainVkSurfaceKHRFormat(surfaceFormats);
  auto swapChainPresentMode   = SelectSwapChainVkPresentModeKHR(surfacePresentModes);
  auto swapChainExtent        = SelectSwapChainVkExtend2D(surfaceCapabilities, pWindow);
  auto swapChainMinImageCount = SelectSwapChainMinImageCount(surfaceCapabilities);

  std::cout << "Swap Chain Selection info:\n"
            << "  Format: " << to_string(swapChainFormat.colorSpace) << " " << to_string(swapChainFormat.format) << '\n'
            << "  Present Mode: " << to_string(swapChainPresentMode) << '\n'
            << "  Extent: " << swapChainExtent.width << ", " << swapChainExtent.height << '\n'
            << "  Min image count: " << swapChainMinImageCount << '\n';

  // Swap Chain create info
  vk::SwapchainCreateInfoKHR swapChainCreateInfo{
    .surface          = *surface,
    .minImageCount    = swapChainMinImageCount,
    .imageFormat      = swapChainFormat.format,
    .imageColorSpace  = swapChainFormat.colorSpace,
    .imageExtent      = swapChainExtent,
    .imageArrayLayers = 1,
    .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
    .imageSharingMode = vk::SharingMode::eExclusive,
    .preTransform     = surfaceCapabilities.currentTransform,
    .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
    .presentMode      = swapChainPresentMode,
    .clipped          = true
  };

  return vk::raii::SwapchainKHR(device, swapChainCreateInfo);
}

} // namespace Optim::VK

VulkanRHI::VulkanRHI() : impl(MakeUnique<Impl>())
{}

VulkanRHI::~VulkanRHI()
{}

/**
 * Start by verifying required extensions. For now since only the SDL
 * required are used, the list containing them is being directly verified.
 *
 * When more extensions will be need then the required extensions verification
 * logic will be updated accordingly.
 *
 * Make a list of string containing all required layers to be used which will
 * be passed as an argument for the `VerifyRequiredLayers` function. It will
 * return a list containing all the unsupported layers.
 *
 * If it is empty, everything is well. Else throw a runtime error listing
 * all the unsupported layers.
 *
 * Once the extensions and layers have been validated, the
 * `Optim::VK::CreateVkInstance()` function is being called to
 * finally create a vk::Instance object.
 *
 * With the instance being created and validated, this function can now start
 * checking for a GPU (vk::PhysicalDevice) to be used.
 *
 * First call the `Optim::VK::SelectVkPhysicalDevice` function to get the most
 * appropriate GPU to use for rendering. The prerequestists for the physical
 * device selection are all defined inside the function. All properties,
 * extensions, queue families being used as prerequisits for GPU selection,
 * must ALL be supported, if nothing is found then the function returns
 * nullptr, then we throw an error.
 *
 * Once that is confirmed we finally call `Optim::VK::CreateRenderDevice`
 * which will create the actual logical device object (vk::Device).
 */
void VulkanRHI::Initialize(void* param_pSDLWindow)
{
  try {
    SDL_Window* sdlwindow = static_cast<SDL_Window*>(param_pSDLWindow);

    TDynamicArray<String> reqInstanceExt;

    // Get list of all extensions required by SDL to create an vkInstance
    // object, then add them to the required extensions list.
    uint32             vkInstanceExtCount = 0;
    const char* const* ppVkInstanceExt    = SDL_Vulkan_GetInstanceExtensions(&vkInstanceExtCount);

    if (ppVkInstanceExt == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }

    for (size_t i = 0; i < vkInstanceExtCount; i++) {
      reqInstanceExt.EmplaceBack(ppVkInstanceExt[i]);
    }

    if constexpr (Optim::VK::enableValidationLayers) {
      reqInstanceExt.EmplaceBack(vk::EXTDebugUtilsExtensionName);
    }

    // Get list of all supported extension by current Vulkan API.
    auto extensionProperties = impl.Get().context.enumerateInstanceExtensionProperties();

    for (auto&& str : reqInstanceExt) {
      auto comparaisonFn = [str](vk::ExtensionProperties const& extentionProperty) {
        return strcmp(extentionProperty.extensionName, str.GetPointer()) == 0;
      };
      if (std::ranges::none_of(extensionProperties, comparaisonFn)) {
        std::cout << "[VulkanRHI | Error]-The required SDL extension: (" << str.GetPointer() << ") is not supported by Vulkan.\n";
      }
    }

    // Required Layers Verification
    TDynamicArray<String> requiredLayers;

    // Add "VK_LAYER_KHRONOS_validation" if validation layers are enabled
    if constexpr (Optim::VK::enableValidationLayers) {
      requiredLayers.EmplaceBack("VK_LAYER_KHRONOS_validation");
    }

    // Validate required layers.
    TDynamicArray<String> unsupportedLayers = Optim::VK::VerifyRequiredLayers(requiredLayers, impl.Get().context);

    if (unsupportedLayers.GetCount() > 0) {
      String errorMsg = String("[VulkanRHI | Error] The following layers are not supported by Vulkan:\n");
      for (auto& layer : unsupportedLayers) {
        errorMsg.Append(" * ").Append(layer);
      }
      throw std::runtime_error(errorMsg.GetPointer());
    }

    // Create VkInstance object
    impl.Get().instance = Optim::VK::CreateVkInstance(impl.Get().context, reqInstanceExt, requiredLayers, Optim::VK::GetApplicationInfoStruct());
    if (impl.Get().instance == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to initialize VkInstance object.");
    }

    // Create VkDebugUtilsMessengerEXT object if validation layers are enabled
    if constexpr (Optim::VK::enableValidationLayers) {
      impl.Get().debugMessenger = Optim::VK::Debug::CreateVkDebugUtilsMessengerEXT(impl.Get().instance);
      if (impl.Get().debugMessenger == nullptr) {
        throw std::runtime_error("[VulkanRHI | Error] Debug messenger FAILED to initialize.");
      }
    }

    // Create VKSurfaceKHR object
    impl.Get().surface = Optim::VK::CreateVkSurfaceKHR(impl.Get().instance, sdlwindow);
    if (impl.Get().surface == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to initialize VkSurfaceKHR object.");
    }

    // Select most optimal VkPhysicalDevice object
    auto physicalDevice = Optim::VK::SelectVkPhysicalDevice(impl.Get().instance, impl.Get().surface);
    if (physicalDevice == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to find usable GPU for rendering.");
    }

    // Create RenderDevice object
    impl.Get().renderDevice = Optim::VK::CreateRenderDevice(physicalDevice, impl.Get().surface);

    if (impl.Get().renderDevice.physicalDevice == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] vk::PhyscialDevice object is null.");
    }
    if (impl.Get().renderDevice.logicalDevice == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] vk::Device object is null.");
    }
    if (impl.Get().renderDevice.graphicsQueue == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] vk::Queue object is null.");
    }

    // Swap chain creation
    impl.Get().swapChain = Optim::VK::CreateVkSwapChainKHR(
      impl.Get().renderDevice.logicalDevice,
      impl.Get().renderDevice.physicalDevice,
      impl.Get().surface,
      sdlwindow);

    if (impl.Get().swapChain == nullptr) {
      throw std::runtime_error("[VulkanRHI | Error] Failed to create VkSwapChainKHR object.");
    }
  }
  catch (const vk::SystemError& e) {
    std::cerr << "[Vulkan System Error] " << e.what() << '\n';
    return;
  }
  catch (const std::exception& e) {
    std::cerr << "[VulkanRHI | Error] " << e.what() << '\n';
    return;
  }
}

void VulkanRHI::Update()
{
}
void VulkanRHI::Cleanup()
{
}