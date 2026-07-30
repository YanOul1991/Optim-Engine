/**
 * Rendering.cpp
 */

#include "Rendering/Rendering.h"

#include "RHI/RHI.h"
#include "VulkanRHI/VulkanRHI.h"

#include <cstdio>
#include <cstdlib>

IRHI* activeModule;

void Internal::Rendering::Initialize()
{
  Internal::RHI::InitializeModule();
}

IRHI* Internal::Rendering::InstanciateRenderInterface()
{
  activeModule = new VulkanRHI();
  return activeModule;
}

void Internal::Rendering::Setup(TDynamicArray<String>& settings)
{
}