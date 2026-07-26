/**
 * Allocator.h
 *
 * TODO:
 *    - Implement logic to have warnings for handled error/potential errors
 *      such as passing invalid pointers to functions.
 *      Those functions make sure that passed pointers are valid, and in
 *      the case one is not the error is silently handled (in this case simply
 *      returns and does nothing), but does not give a warning that it has
 *      failed making debugging tricky.
 */

#pragma once

#include "Core/CoreMinimal.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Memory {

template <typename T>
inline T* Alloc(uint64 count) {
  return static_cast<T*>(::operator new(sizeof(T) * count));
};

template <typename T>
inline void Dealloc(T* buffer, uint64 count, uint64 capacity) {
  if (!buffer) {
    return;
  }
  
  if constexpr (!Concepts::TrivialDestruct<T>) {
    for (uint64 i = count; i > 0; i--) {
      buffer[i - 1].~T();
    }
  }

  ::operator delete(buffer, sizeof(T) * capacity);
};

template <typename T>
inline void ObjectCopyConstruct(T* destination, T* source, uint64 count) {
  static_assert(Concepts::IsCopyConstructible<T>, "The type does not support copy constructor\n");
  for (size_t i = 0; i < count; i++) {
    new (destination + i) T(source[i]);
  }
}

/**
 * @brief
 * Instanciates objects from a source buffer into a destination buffer by move constrcutor.
 *
 * @param destination
 * A pointer to a destination buffer to hold the instanciated elements.
 *
 * @param source
 * A source buffer to use for the move constructors.
 *
 * @param count
 * Number of elements in the source buffer to move
 */
template <typename T>
inline void ObjectMoveConstruct(T* destination, T* source, uint64 count) {
  static_assert(Concepts::IsMoveConstructible<T>, "The type does not support move constructor\n");
  if (destination && source) {
    for (size_t i = 0; i < count; i++) {
      new (destination + i) T(static_cast<T&&>(source[i]));
    }
  }
}

/**
 * @brief
 * Moves a memory block from a source to a destination buffer.
 *
 * This function only works with types that are trivially copyable,
 * using it on a non trivially copyable type will cause a compile time
 * assertion failure.
 *
 * @param destination
 * A pointer to destination buffer to hold the data.
 *
 * @param source
 * A pointer to a source buffer to get the data from.
 *
 * @param count
 * The number of ELEMENTS to copy from the source into the destination.
 *
 * @warning
 * This function can cause undefined behaviour if the buffer's range
 * overlap.
 *
 * If it is the case use `Memory::Memmove` function instead.
 */
template <typename T>
inline void Memcpy(T* destination, T* source, uint64 count) {
  static_assert(Concepts::TrivialCopy<T>, "Type must be trivially copyable to use Memory::Memcpy function\n");
  if (destination && source) {
    memcpy(destination, source, sizeof(T) * count);
  }
}

// template <typename T>
// inline void Copy(T* destinationBuffer, T* sourceBuffer, uint64 count) {
//   static_assert(Concepts::TrivialCopy || Concepts::IsCopyConstructible<T> || Concepts::IsMoveConstructible,
//                 "The type does not support copy constructor\n");

//   // TODO add warnings if any pointer is null.
//   if (!destinationBuffer || !sourceBuffer) {
//     return;
//   }

//   if constexpr (Concepts::TrivialCopy<T>) {
//     memccpy(destinationBuffer, sourceBuffer, sizeof(T) * count);
//   }
//   else if constexpr (Concepts::IsMoveConstructible<T>) {
//     for (size_t i = 0; i < count; i++) {
//       new (destinationBuffer + i) T(static_cast<T&&>(sourceBuffer[i]));
//     }
//   }
//   else if constexpr (Concepts::IsCopyConstructible<T>) {
//     for (size_t i = 0; i < count; i++) {
//       new (destinationBuffer + i) T(sourceBuffer[i]);
//     }
//   }
// }

template <typename T>
inline void Memmove(T* destination, T* source, uint64 count) {
  static_assert(Concepts::TrivialCopy<T>, "Type must be trivially copyable to use Memory::Memmove function\n");
  if (destination && source) {
    memmove(destination, source, sizeof(T) * count);
  }
}

template <>
inline void Memcpy<char>(char* destination, char* source, uint64 count) {
  static_assert(Concepts::TrivialCopy<char>, "Type must be trivially copyable to use Memory::Memcpy function\n");
  if (destination && source) {
    memcpy(destination, source, sizeof(char) * count);
  }
}

}; // namespace Memory