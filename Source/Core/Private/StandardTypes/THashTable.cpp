#include "Core/StandardTypes/THashTable.h"

size_t __OptimInternal::HashMixer(uint64 x) {
  x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9;
  x = (x ^ (x >> 27)) * 0x94D049BB133111EB;
  x = x ^ (x >> 31);
  return x & (128 - 1);
}

// size_t __OptimInternal::HashMixer(uintptr_t x) {
//   x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9;
//   x = (x ^ (x >> 27)) * 0x94D049BB133111EB;
//   x = x ^ (x >> 31);
//   return x & (128 - 1);
// }

size_t __OptimInternal::HashMixer(void *x) {
  return 0;
}