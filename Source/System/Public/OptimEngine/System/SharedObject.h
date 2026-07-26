// ------------------------------------------------------------------
// SharedObject.h
// ------------------------------------------------------------------

#pragma once

#include "Core/CoreMinimal.h"
#include "Core/StandardTypes/String.h"

/**
 * TODO:
 * This class must be tested for potential problems before using it.
 */
class SYSTEM_API SharedObject final
{
 public:
  SharedObject(const SharedObject&) = delete;
  SharedObject(SharedObject&&)      = delete;

  static SharedObject Load(const char* relativePath);

  operator bool() const;

  bool IsLoaded() const;

  /**
   * TODO:
   * Set return type to TFunction class once it will be implemented
   * into the core module.
   */
  template <typename RET, typename... ARGS> auto TLoadFunction(const char* functionName) -> RET (*)(ARGS...) {
    return static_cast<RET (*)(ARGS...)>(GetFunctionPointer(functionName));
  }

 private:
  SharedObject(void*);

  auto GetFunctionPointer(const char* name) -> void(*)();
  String m_location;
  void*  m_pSharedObject = nullptr;
};