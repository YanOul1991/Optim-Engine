/**
 * FILE: VulkanRHI.cpp
 */

#include "VulkanRHI/VulkanRHI.h"

#include "OpVkCommon/Minimal.h"
#include "OpVkCore/OpVkCore.h"
#include "OpVkDebug/Debug.h"
#include "OpVkTypes/CommandContext.h"
#include "OpVkTypes/RenderContext.h"
#include "OpVkTypes/SwapChainContext.h"
#include "VulkanRenderer.h"

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

VulkanRHI::VulkanRHI() : renderer(std::make_unique<Renderer>())
{}

VulkanRHI::~VulkanRHI()
{}

/**
 * @brief
 * Initializes the VulkanRHI object and all required Vulkan objects.
 */
void VulkanRHI::Initialize(void* pSdlWindow)
{
  try {
    renderer->Initialize(static_cast<SDL_Window*>(pSdlWindow));
  }
  catch (const vk::SystemError& e) {
    std::cerr << "[Vulkan | Error] " << e.what() << '\n';
    return;
  }
  catch (const std::exception& e) {
    std::cerr << "[VulkanRHI | Error] " << e.what() << '\n';
    return;
  }
}

void VulkanRHI::DrawFrame()
{
  try {
    renderer->DrawFrame();
  }
  catch (const vk::SystemError& e) {
    std::cerr << "[Vulkan | Error] " << e.what() << '\n';
    return;
  }
  catch (const std::exception& e) {
    std::cerr << "[VulkanRHI | Error] " << e.what() << '\n';
    return;
  }
}

void VulkanRHI::Cleanup()
{
  renderer->WaitForDeviceIdle();
}