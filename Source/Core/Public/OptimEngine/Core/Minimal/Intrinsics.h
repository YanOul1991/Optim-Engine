#pragma once

#if defined(_WIN32) || defined(_WIN64)
#  define PLATFORM_WINDOWS
#endif

#if defined(_MSC_VER)
#  define FORCEINLINE __forceinline
#  define DEPRECATED  __declpspec(deprecated)
#  define EXPORT      __declspec(dllexport)
#  define IMPORT      __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#  define FORCEINLINE __attribute__((always_inline))
#  define DEPRECATED  __attribute__((deprecated))
#  define EXPORT      __attribute__((visibility("default")))
#  define IMPORT
#endif