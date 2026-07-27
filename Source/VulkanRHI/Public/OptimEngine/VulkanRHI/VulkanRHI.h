/**
 * FILE: VulkanRHI.h
 */

#pragma once

#include "Core/CoreMinimal.h"
#include "RHI/IRHI.h"

class VULKANRHI_API VulkanRHI final : public IRHI
{
 public:
  VulkanRHI();

  virtual ~VulkanRHI() override final;
  virtual void Initialize(TDynamicArray<String>& extraData) override final;
  virtual void Update() override final;
  virtual void Cleanup() override final;
};