#pragma once

// #define ENABLE_LOG

#if defined(ENABLE_LOG)
#  define LOG(...) printf(__VA_ARGS__);
#else
#  define LOG(...)
#endif

#include "Core/CoreMinimal.h"
#include "Core/Debug/Debug.h"
#include "Core/Memory/Allocation.h"
#include "Core/System/LowLevel.h"

#include <iostream>

template <typename T> class TDynamicArray final
{
 public:
  TDynamicArray() : m_buffer(Memory::Alloc<T>(32)), m_count(0), m_capacity(32) {
  }

  ~TDynamicArray() {
    Memory::Dealloc<T>(m_buffer, m_count, m_capacity);
    m_buffer = nullptr;
  }

  void Reserve(uint64 _newCapacity) {
    if (m_capacity >= _newCapacity) {
      return;
    }

    T* __newBuffer = Memory::Alloc<T>(_newCapacity);

    if constexpr (Concepts::TrivialCopy<T>) {
      Memory::Memcpy<T>(__newBuffer, m_buffer, m_count);
    }
    else if constexpr (Concepts::IsMoveConstructible<T>) {
      Memory::ObjectMoveConstruct<T>(__newBuffer, m_buffer, m_count);
    }
    else if constexpr (Concepts::IsCopyConstructible<T>) {
      Memory::ObjectCopyConstruct<T>(__newBuffer, m_buffer, m_count);
    }

    Memory::Dealloc<T>(m_buffer, m_count, m_capacity);

    m_buffer   = __newBuffer;
    m_capacity = _newCapacity;
  }

  uint64 GetCount() const {
    return m_count;
  }

  uint64 GetCapacity() const {
    return m_capacity;
  }

  T const* GetData() const {
    return m_buffer;
  }

  void Push(const T& _newElement) {
    if (m_count >= m_capacity) {
      Reserve(m_capacity * 2);
    }

    if constexpr (Concepts::TrivialCopy<T>) {
      m_buffer[m_count] = _newElement;
      m_count++;
    }
    else {
      new (m_buffer + m_count) T(_newElement);
    }
  }

  void Push(T&& _newElement) {
    static_assert(Concepts::TrivialCopy<T> | Concepts::IsMoveConstructible<T>);

    if (m_count >= m_capacity) {
      Reserve(m_capacity * 2);
    }

    if constexpr (Concepts::TrivialCopy<T>) {
      m_buffer[m_count] = _newElement;
      m_count++;
    }
    else {
      new (m_buffer + m_count) T(_newElement);
      m_count++;
    }
  }

  void RemoveAt(uint64 index) {
    if (index >= m_count) {
      // TODO Add Warning
      return;
    }

    if constexpr (Concepts::TrivialCopy<T>) {
      Memory::Memmove(m_buffer + index, m_buffer + index + 1, m_count - (index + 1));
      m_count--;
    }
  };

  /**
   * Removes an element at a given index, but replaces
   * it by the last element. Faster than the RemoveAt
   * function, if element order doesnt matter, better
   * for complex objects.
   */
  void RemoveAtAndSwap(uint64 index) {
    if (index >= m_count) {
      // TODO Add Warning
      return;
    }

    if constexpr (Concepts::TrivialCopy<T>) {
      m_buffer[index] = m_buffer[m_count - 1];
      m_count--;
    }
  }

  // Removes the last element of the array
  void Pop() {
    if constexpr (Concepts::TrivialDestruct<T>) {
      if (m_count > 0) {
        m_count--;
      }
    }
  }

  void Shrink(uint64 padding = 0) {
    uint64 __newCapacity = m_count + padding;

    // Allocate new buffer with new capacity + extra padding
    T* __newBuffer = Memory::Alloc<T>(__newCapacity);

    if constexpr (Concepts::TrivialCopy<T>) {
      Memory::Memcpy<T>(__newBuffer, m_buffer, m_count);
    }

    Memory::Dealloc<T>(m_buffer, m_count, m_capacity);

    m_buffer   = __newBuffer;
    m_capacity = __newCapacity;
  }

  void Clear() {
    if constexpr (Concepts::TrivialDestruct<T>) {
      m_count = 0;
    }
  }

  void Sort() {
  }

  bool ContainsSorted(const T& value) const {
    size_t _start = 0;
    size_t _end   = m_count - 1;
    size_t _center;

    int confirm;

    while (_end - _start > 1) {
      // Separate into two partitions the buffer at
      // the center (middle value);
      _center = (0.5 * (_end - _start)) + _start;

      // printf("Searching through indices %lu : %lu\n", _start, _end);
      // If out of range then automatically false.

      if (value < m_buffer[_start] || value > m_buffer[_end]) {
        // printf("Searching value out of bounds.\n");
        return false;
      }

      if (value == _center) {
        return true;
      }

      // if the searching value is lower than the
      // center value then next partition will search lower
      // part. If high higher part.
      if (value < m_buffer[_center]) {
        _end = _center;
      }
      else {
        _start = _center;
      }
    }

    return value == m_buffer[_end] || value == m_buffer[_start];
  };

  /*
  bool Contains(const T& value) const {
    uint64 cursor = 0;

    for (; cursor + ((32 * 4) / sizeof(T)) <= m_count; cursor += ((32 * 4) / sizeof(T))) {
      int32 m1 = LowLevel::FindSimdAvx2(value, m_buffer + (cursor + ((32 * 0) / sizeof(T))));
      int32 m2 = LowLevel::FindSimdAvx2(value, m_buffer + (cursor + ((32 * 1) / sizeof(T))));
      int32 m3 = LowLevel::FindSimdAvx2(value, m_buffer + (cursor + ((32 * 2) / sizeof(T))));
      int32 m4 = LowLevel::FindSimdAvx2(value, m_buffer + (cursor + ((32 * 3) / sizeof(T))));

      if ((m1 | m2 | m3 | m4) != 0) {
        return true;
      }
    }
    for (; cursor + (32 / sizeof(T)) <= m_count; cursor += (32 / sizeof(T))) {
      if (LowLevel::FindSimdAvx2(value, m_buffer + cursor) != 0) {
        return true;
      }
    }
    for (; cursor < m_count; cursor++) {
      if (m_buffer[cursor] == value) {
        return true;
      }
    }
    return false;
  };
  */

  int64 Find(const T& value) const {
    for (int64 i = 0; i < m_count; i++) {
      if (m_buffer[i] == value) {
        return i;
      }
    }
    return -1;
  }

  template <typename... Args> T& EmplaceBack(Args&&... args) {
    if (m_count >= m_capacity) {
      Reserve(m_capacity * 2);
    }

    T* item = new (&m_buffer[m_count]) T(static_cast<Args&&>(args)...);
    m_count++;
    return *item;
  };

  T& operator[](const uint64 i) {
    return m_buffer[i];
  }

  //////////////////////////////////// STD IDIOMATIC LOOP

  constexpr T* begin() noexcept {
    return m_buffer;
  }
  constexpr T* end() noexcept {
    return m_buffer + m_count;
  }
  constexpr const T* begin() const noexcept {
    return m_buffer;
  }
  constexpr const T* end() const noexcept {
    return m_buffer + m_count;
  }
  constexpr const T* cbegin() const noexcept {
    return m_buffer;
  }
  constexpr const T* cend() const noexcept {
    return m_buffer + m_count;
  }
  constexpr T* rbegin() noexcept {
    return m_buffer + m_count - 1;
  }
  constexpr T* rend() noexcept {
    return m_buffer - 1;
  }
  constexpr const T* rbegin() const noexcept {
    return m_buffer + m_count - 1;
  }
  constexpr const T* rend() const noexcept {
    return m_buffer - 1;
  }

 private:
  T*     m_buffer;
  size_t m_count;
  size_t m_capacity;
};