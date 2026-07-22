// Time.cpp

#include "Core/System/Time.h"

#include "Core/Debug/Debug.h"

#ifdef PLATFORM_WINDOWS
#  include <windows.h>
#else
#  include <time.h>
#endif

// Get time in nanoseconds
uint64 Time::GetTime() {
#ifdef PLATFORM_WINDOWS
  // Cache frequency for windows
  static double nanosecPerTick = []() {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return 1000000000.0 / static_cast<double>(freq.QuadPart);
  }();

  LARGE_INTEGER count;
  QueryPerformanceCounter(&count);
  return static_cast<uint64_t>(count.QuadPart * nanosecPerTick);
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return static_cast<uint64_t>(ts.tv_sec) * 1000000000ull + ts.tv_nsec;
#endif
}