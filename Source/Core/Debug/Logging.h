// Logging.h
//
// In non debug mode template functions are empty. Compilers
// should remove these when compiling. For link-time optimizations
// enable the appropriate flag for the used compiler.
// Ideally, the flag should not be used by default to avoid extra
// build time.
//
// In debug mode templates functions behave as expected.

#pragma once

#include "Core/CoreMinimal.h"
#include "Core/StandardTypes/String.h"

namespace Internal::Debug {
CORE_API void InternalDebugPrintF(const char* format, ...);
}

namespace Debug {
#if defined(NDEBUG) || defined(NOLOGS)
template <typename... ARGS> inline void Log(ARGS&&...) {
}
#else
template <typename... ARGS> inline void Log(const char* format, ARGS&&... args) {
  Internal::Debug::InternalDebugPrintF(format, args...);
}
#endif
} // namespace Debug