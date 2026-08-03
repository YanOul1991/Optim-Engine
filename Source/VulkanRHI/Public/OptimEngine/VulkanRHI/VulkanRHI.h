/**
 * FILE: VulkanRHI.h
 */

#pragma once

#include "OptimEngine/Core/CoreMinimal.h"
#include "OptimEngine/Core/Memory/TUniquePtr.h"
#include "OptimEngine/RHI/IRHI.h"

class VULKANRHI_API VulkanRHI final : public IRHI
{
 public:
  VulkanRHI();
  virtual ~VulkanRHI() override final;

  // No copy! No move!
  VulkanRHI(const VulkanRHI&)            = delete;
  VulkanRHI& operator=(const VulkanRHI&) = delete;

  virtual void Initialize(void* pWindow) override final;
  virtual void Update() override final;
  virtual void Cleanup() override final;

 private:
  struct VulkanContext;
  TUniquePtr<VulkanContext> pVkContext;
};