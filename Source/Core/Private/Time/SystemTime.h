#pragma once

#include "OptimEngine/Core/CoreMinimal.h"

#if defined(PLATFORM_WINDOWS)
#  include <Windows.h>
#elif defined(PLATFORM_LINUX)
#  include <time.h>
#endif

namespace OptimPrivate::Time {

inline uint64 GetTime() {

#if PLATFORM_LINUX
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return static_cast<uint64_t>(ts.tv_sec) * 1000000000ull + ts.tv_nsec;
#elif PLATFORM_WINDOWS
  // Cache frequency
  static double nanosecPerTick = []() {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return 1000000000.0 / static_cast<double>(freq.QuadPart);
  }();

  LARGE_INTEGER count;
  QueryPerformanceCounter(&count);
  return static_cast<uint64_t>(count.QuadPart * nanosecPerTick);
#endif
}

}; // namespace Internal::Time