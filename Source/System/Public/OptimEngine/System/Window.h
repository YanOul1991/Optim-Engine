#pragma once

#include "Core/CoreMinimal.h"

class SYSTEM_API Window final
{
 public:
  Window(const char* windowName);

  uint32 GetWindowID() const;

  bool IsValid() const;

  operator bool() const;

 private:
  uint32 m_WindowID;
};