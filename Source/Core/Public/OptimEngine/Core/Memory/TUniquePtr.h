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
  TUniquePtr()
  {}

  ~TUniquePtr()
  {
    FreeData();
  }

  TUniquePtr(nullptr_t)
  {}

  TUniquePtr(T*& pointer)
  {
    pRefObject = pointer;
    pointer    = nullptr;
  }

  TUniquePtr(T*&& pointer)
  {
    pRefObject = pointer;
  }

  TUniquePtr(const TUniquePtr&) = delete;

  TUniquePtr(TUniquePtr&& other)
  {
    if (&other != this) {
      pRefObject       = other.pRefObject;
      other.pRefObject = nullptr;
    }
  }

  void SetPtr(T* initialValue)
  {
    if (pRefObject != nullptr) {
      delete pRefObject;
    }
    pRefObject = initialValue;
  }

  void TransferOwnershipTo(TUniquePtr& other)
  {
    if (&other == this) {
      return;
    }

    if (other.pRefObject != nullptr) {
      other.FreeData();
      other.pRefObject = pRefObject;
      pRefObject       = nullptr;
    }
  }

  TUniquePtr operator=(const TUniquePtr&) = delete;

  TUniquePtr& operator=(TUniquePtr&& other)
  {
    if (&other != this) {
      pRefObject       = other.pRefObject;
      other.pRefObject = nullptr;
    }
    return *this;
  }

  void FreeData()
  {
    if (pRefObject != nullptr) {
      delete pRefObject;
    }
    pRefObject = nullptr;
  }

  operator bool() const
  {
    return pRefObject != nullptr;
  }

  bool operator==(nullptr_t)
  {
    return pRefObject == nullptr;
  }

  bool IsValid() const
  {
    return pRefObject != nullptr;
  }

  T* GetPtr()
  {
    return pRefObject;
  }

  T& Get()
  {
    assert(pRefObject != nullptr && "[Assertion failure] TUniquePtr | Trying to dereference a null pointer.\n");
    return *pRefObject;
  }

 private:
  T* pRefObject = nullptr;
};

template <typename T> TUniquePtr<T> MakeUnique()
{
  return TUniquePtr<T>(new T());
}
