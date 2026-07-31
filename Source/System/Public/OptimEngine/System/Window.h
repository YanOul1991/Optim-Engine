#pragma once

#include "OptimEngine/Core/CoreMinimal.h"

class SYSTEM_API Window final
{
 public:
  Window(const char* windowName);
  ~Window();

  uint32 GetWindowID() const { return m_WindowID; }

  bool IsValid() const;

 private:
  uint32 m_WindowID;
};