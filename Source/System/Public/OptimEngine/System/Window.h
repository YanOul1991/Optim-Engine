#pragma once

#include "OptimEngine/Core/CoreMinimal.h"

class SYSTEM_API Window final
{
 public:
  Window(const char* windowName);
  ~Window();
  
  uint32 GetWindowID() const { return windowId; }
  void* GetSDLWindowHandle() const { return pSDLWindow; }
  bool IsValid() const { return pSDLWindow != nullptr; }

 private:
  uint32 windowId;
  void*  pSDLWindow;
};