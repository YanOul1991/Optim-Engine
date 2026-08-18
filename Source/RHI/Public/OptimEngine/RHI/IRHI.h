/**
 * IRHI.h
 */

#pragma once

#include "Core/CoreMinimal.h"
#include "Core/StandardTypes/String.h"
#include "Core/StandardTypes/TDynamicArray.h"

class Window;

class IRHI
{
 public:
  inline virtual ~IRHI()
  {}
  virtual void Initialize(void*) = 0;
  virtual void DrawFrame()       = 0;
  virtual void Cleanup()         = 0;
};