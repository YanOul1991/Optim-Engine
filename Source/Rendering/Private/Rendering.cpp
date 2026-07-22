/**
 * Rendering.cpp
 */

#include "Rendering/Rendering.h"

#include "VulkanRHI/VulkanRHI.h"

#include <cstdio>
#include <cstdlib>

static struct RenderingGlobalData {
  IRHI* interfaceInstance = nullptr;
} globalRendering;

void Internal::Rendering::Initialize() {
  
}

IRHI* Internal::Rendering::InstanciateRenderInterface() {
  // Only create a new instance of a RHI if no instance
  // exists already
  if (globalRendering.interfaceInstance == nullptr) {
    printf("Creating instance of VulkanRHI\n");
    return new VulkanRHI();
  }
  return nullptr;
}

void Internal::Rendering::Setup(TDynamicArray<String>& settings) {
}