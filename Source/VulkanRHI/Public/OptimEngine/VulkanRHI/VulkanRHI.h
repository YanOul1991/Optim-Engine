/**
 * FILE: VulkanRHI.h
 */

#pragma once

#include "Core/CoreMinimal.h"
#include "Core/Memory/TUniquePtr.h"
#include "RHI/IRHI.h"

#include <memory>

struct Renderer;

class VULKANRHI_API VulkanRHI final : public IRHI
{
 public:
  VulkanRHI();
  virtual ~VulkanRHI() override final;

  // No copy! No move!
  VulkanRHI(const VulkanRHI&)            = delete;
  VulkanRHI& operator=(const VulkanRHI&) = delete;

  virtual void Initialize(void* pWindow) override final;
  virtual void DrawFrame() override final;
  virtual void Cleanup() override final;
  
 private:
  std::unique_ptr<Renderer> renderer;
};