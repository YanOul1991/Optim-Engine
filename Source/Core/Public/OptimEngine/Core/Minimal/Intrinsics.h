#pragma once

#if defined(_MSC_VER)
#  define COMPILER_MSVC 1
#elif defined(__clang__)
#  define COMPILER_CLANG 1
#elif defined(__GNUC__)
#  define COMPILER_GCC 1
#else
#  error "Unknown compiler!"
#endif

#if defined(_WIN32) || defined(_WIN64)
#  define PLATFORM_WINDOWS 1
#elif defined(__linux__)
#  define PLATFORM_LINUX 1
#else
#  error "Unsupported platform"
#endif

#if !defined(NDEBUG) || defined(DEBUG)
#  if defined(COMPILER_MSVC)
#    define DEBUG_BREAK() __debugbreak()
#  elif defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#    define DEBUG_BREAK() __builtin_trap()
#  else
#    define DEBUG_BREAK()
#  endif
#endif

#if defined(COMPILER_MSVC)
#  define FORCEINLINE __forceinline
#  define DEPRECATED  __declpspec(deprecated)
#  define EXPORT      __declspec(dllexport)
#  define IMPORT      __declspec(dllimport)
#  define ALIGN(x)    __declspec(align(x))
#elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#  define FORCEINLINE __attribute__((always_inline))
#  define DEPRECATED  __attribute__((deprecated))
#  define EXPORT      __attribute__((visibility("default")))
#  define IMPORT
#  define ALIGN(x) __attribute__((aligned(x)))
#endif