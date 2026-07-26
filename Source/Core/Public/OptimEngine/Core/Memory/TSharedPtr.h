/**
 * TSharedPtr.h
 */

#pragma once

#include "Core/CoreMinimal.h"
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstdlib>

template <typename T> class TWeakPtr;
template <typename T> class TSharedPtr;

/**
 * @brief
 * ControlBlock object
 */
template <typename T> struct ControlBlock {
  std::atomic<uint64_t> strong = 0;
  std::atomic<uint64_t> weak   = 0;
  T*                    ptr    = nullptr;
};

/**
 * @brief
 * TSharedPtr object.
 */
template <typename T> class TSharedPtr final
{
 public:
  explicit TSharedPtr() {
    // printf("null shared pointer");
  };

  explicit TSharedPtr(const TWeakPtr<T>& weak) {
    if (weak.getCtrl() != nullptr) {
      pCtrl      = weak.getCtrl();
      pRefObject = pCtrl->ptr;
      pCtrl->strong.fetch_add(1);
      // printf("[TSharedPtr] INIT by TWeakPtr updgrade (%lu | %lu)\n", pCtrl->strong.load(), pCtrl->weak.load());
      return;
    }
    // printf("[TSharedPtr] Could not initalize by TWeakPtr upgrade\n");
  }

  explicit TSharedPtr(T* pointer) :
      pRefObject{ pointer },
      pCtrl{ new ControlBlock<T> } {
    pCtrl->strong.fetch_add(1);
    pCtrl->ptr = pRefObject;
    // printf("[TSharedPtr] INIT by raw pointer (%lu | %lu)\n", pCtrl->strong.load(), pCtrl->weak.load());
  }

  explicit TSharedPtr(const TSharedPtr&& other) :
      pRefObject{ other.pRefObject },
      pCtrl{ other.pCtrl } {
    pCtrl->strong.fetch_add(1);
    // printf("[TSharedPtr] INIT by RVALUE (%lu | %lu)\n", pCtrl->strong.load(), pCtrl->weak.load());
  }

  /**
   * @brief
   * TSharedPtr object should not use copy assignement directly.
   * @
   * They should alway use the share function.
   */
  explicit TSharedPtr(const TSharedPtr&) = delete;

  ~TSharedPtr() {
    release();
  }

  /**
   * @brief
   * Removes this TSharedPtr object's shared ownership over the underlying object.
   * If it was the last one with a reference to the object then, the object gets deleted.
   */
  void release() {
    if (pRefObject == nullptr) {
      // printf("[TSharedPtr] release() callback | object already null\n");
      return;
    }

    if (pCtrl->strong.fetch_sub(1) == 1) {
      delete pRefObject;
      pRefObject = nullptr;
      // printf("[TSharedPtr] release() callback | (%lu | %lu)\n", pCtrl->strong.load(), pCtrl->weak.load());
      if (pCtrl->weak.load() == 0) {
        delete pCtrl;
        pCtrl = nullptr;
      }
    }
  }

  TSharedPtr& operator=(const TSharedPtr&& other) {
    if (&other != this) {
      pRefObject = other.pRefObject;
      pCtrl      = other.pCtrl;
      pCtrl->strong.fetch_add(1);
    }
    return *this;
  }

  TSharedPtr operator=(const TSharedPtr&) = delete;

  bool operator==(nullptr_t) {
    return pRefObject == nullptr;
  }

  operator bool() const {
    return pRefObject != nullptr;
  }

  /**
   * @brief
   * Share this TSharedPtr object underlying pointer with another TSharedPtr object,
   * so they both now refer to the same one.
   */
  TSharedPtr<T>&& share() {
    return static_cast<TSharedPtr<T>&&>(*this);
  }

  T* operator->() const {
    return pRefObject;
  }

  ControlBlock<T>* getCtrl() const {
    return pCtrl;
  }

 private:
  T*               pRefObject{ nullptr };
  ControlBlock<T>* pCtrl{ nullptr };
};

/**
 * @brief
 * TWeakPtr object.
 */
template <typename T> class TWeakPtr final
{
 public:
  explicit TWeakPtr() :
      pCtrl{ nullptr } {
  }

  explicit TWeakPtr(const TSharedPtr<T>& sr) :
      pCtrl{ sr.getCtrl() } {
    if (pCtrl) {
      pCtrl->weak.fetch_add(1);
      // printf("[TWeakPtr] Contructor pass with TSharedPtr (%lu | %lu)\n", pCtrl->strong.load(), pCtrl->weak.load());
    }
  }

  TWeakPtr(const TWeakPtr<T>& other) :
      pCtrl{ other.pCtrl } {
    if (&other == this) {
      // printf("[TWeakPtr] Trying to initalize WeakPointer with itself.\n");
      return;
    }

    if (pCtrl) {
      pCtrl->weak.fetch_add(1, std::memory_order_relaxed);
      // printf("[TWeakPtr] copy constructor to other TWeakPtr: (%lu | %lu)\n", pCtrl->strong.load(), pCtrl->weak.load());
    }
  }

  ~TWeakPtr() {
    release();
  }

  TWeakPtr& operator=(const TWeakPtr&) = delete;

  TWeakPtr& operator=(TWeakPtr&& other) {
    if (&other != this) {
      release();
      pCtrl       = other.pCtrl;
      other.pCtrl = nullptr;
      // printf("[TWeakPtr] move: (%lu | %lu)\n", pCtrl->strong.load(), pCtrl->weak.load());
    }
    return *this;
  }

  TWeakPtr&& move() {
    return static_cast<TWeakPtr<T>&&>(*this);
  }

  void release() {
    if (!pCtrl) {
      return;
    }
    // printf("[TWeakPtr] release() callback: (%lu | %lu)\n", pCtrl->strong.load(), pCtrl->weak.load());

    if (pCtrl->weak.fetch_sub(1) == 1 && pCtrl->strong.load() == 0) {
      delete pCtrl;
      pCtrl = nullptr;
      printf("[TWeakPtr] Released reference: (0| 0)\n");
      return;
    }
    // printf("[TWeakPtr] Released reference: (%lu | %lu)\n", pCtrl->strong.load(), pCtrl->weak.load());
  }

  TSharedPtr<T> lock() const {
    if (!pCtrl || pCtrl->strong.load() == 0) {
      return TSharedPtr<T>();
    }
    return TSharedPtr<T>(*this);
  }

  bool expired() const {
    return !pCtrl || pCtrl->strong.load() == 0;
  }

  ControlBlock<T>* const getCtrl() const {
    return pCtrl;
  }

 private:
  ControlBlock<T>* pCtrl{ nullptr };
};