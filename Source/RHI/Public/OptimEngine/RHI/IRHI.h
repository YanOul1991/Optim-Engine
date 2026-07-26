/**
 * IRHI.h
 */

#pragma once

#include "Core/CoreMinimal.h"
#include "Core/StandardTypes/String.h"
#include "Core/StandardTypes/TDynamicArray.h"

class IRHI
{
 public:
  inline virtual ~IRHI() {
  }
  virtual void Initialize(TDynamicArray<String>& extraData) = 0;
  virtual void Update()                                     = 0;
  virtual void Cleanup()                                    = 0;
};