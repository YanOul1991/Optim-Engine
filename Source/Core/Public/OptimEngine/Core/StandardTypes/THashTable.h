#pragma once

#include "Core/CoreMinimal.h"
#include <cstdint>
#include <functional>

inline uint64 Mixer64(uint64 x) {
  x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9;
  x = (x ^ (x >> 27)) * 0x94D049BB133111EB;
  x = x ^ (x >> 31);
  return x & (128 - 1);
}

inline uint64 MixerPointer(uintptr_t x) {
  x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9;
  x = (x ^ (x >> 27)) * 0x94D049BB133111EB;
  x = x ^ (x >> 31);
  return x & (128 - 1);
}

namespace __OptimInternal {

CORE_API size_t HashMixer(uint64 x);
CORE_API size_t HashMixer(uintptr_t x);
CORE_API size_t HashMixer(void* x);

}; // namespace __OptimInternal

template <typename T> inline size_t GetHash(T x) {
  return __OptimInternal::HashMixer(x);
}

uint64 GetHash(const char* str) {
  uint64 hash = 0xCBF29CE484222325; // FNV offset basis

  while (*str) {
    hash ^= (uint64)((unsigned char)*str++);
    hash *= 0x100000001B3; // FNV prime
  }

  return hash & (64 - 1);
}

template <typename T> struct THashEntry final {
  T    value;
  byte meta = 0; // bit 0 = occupied; bit 1 = deleted;
};

template <typename KEY, typename VAL>
class THashTable final
{
 public:
 private:
  KEY*             m_keys;
  THashEntry<VAL>* m_entries;
  size_t           m_capacity;
  size_t           m_count;
};