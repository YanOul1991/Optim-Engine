/**
 * TUniquePtr.h
 */

#pragma once

#include "Core/CoreMinimal.h"
#include "Core/Debug/Debug.h"

#include <cassert>
#include <cstddef>

template <typename T> class TUniquePtr final
{
  public:
  TUniquePtr() = default;
  explicit TUniquePtr(T* ptr) noexcept : pRefObject(ptr) {}
  ~TUniquePtr() { Reset(); }
  
  TUniquePtr(const TUniquePtr&) = delete;
  TUniquePtr operator=(const TUniquePtr&) = delete;

  TUniquePtr(TUniquePtr&& other) noexcept 
  {
    if (this != &other) {
      Reset(other.Release());
    }
  }

  TUniquePtr& operator=(TUniquePtr&& other) noexcept
  {
    if (this != &other) {
      Reset(other.Release());
    }
    return *this;
  }

  void Reset(T* ptr = nullptr)
  {
    if (pRefObject != nullptr) {
      delete pRefObject;
    }
    pRefObject = ptr;
  }

  T* Release()
  {
    T* temp = pRefObject;
    pRefObject = nullptr;
    return temp;
  }

  T& GetRef() { return *pRefObject; }
  T* GetPtr() const { return pRefObject; }
  bool operator==(nullptr_t) { return pRefObject == nullptr; }
  bool IsValid() const { return pRefObject != nullptr; }
  T* operator->() const { return pRefObject; }
  T& operator*() const { return *pRefObject; }

 private:
  T* pRefObject = nullptr;
};

template <typename T> TUniquePtr<T> MakeUnique()
{
  return TUniquePtr<T>(new T());
}
