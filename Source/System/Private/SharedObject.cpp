// ------------------------------------------------------------------
// SharedObject.cpp
// ------------------------------------------------------------------

#include "System/SharedObject.h"

#include <SDL3/SDL.h>

SharedObject::SharedObject(void* pSharedObject) {
  m_pSharedObject = pSharedObject;
}

auto SharedObject::GetFunctionPointer(const char *name)->void (*)() {
  if (m_pSharedObject == nullptr) {
    return nullptr;
  }
  return SDL_LoadFunction(static_cast<SDL_SharedObject*>(m_pSharedObject), name);
}

SharedObject SharedObject::Load(const char* relativePath) {
  String soFile = String(SDL_GetBasePath()).Append(relativePath);
  return SharedObject(SDL_LoadObject(soFile.GetPointer()));
}

SharedObject::operator bool() const {
  return IsLoaded();
}

bool SharedObject::IsLoaded() const {
  return m_pSharedObject != nullptr;
}
