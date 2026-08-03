// THIS FILE IS AUTO-GENERATED.
#pragma once

// Export macros for vulkanrhi module
#if !defined(VULKANRHI_API)
#  if defined(USE_STATIC_LIBS)
#    define VULKANRHI_API
#  else
#    if defined(EXPORT_VULKANRHI)
#      if defined(_MSC_VER) || defined(__CYGWIN__)
#        define VULKANRHI_API __declspec(dllexport)
#      else
#        define VULKANRHI_API __attribute__((visibility("default")))
#      endif
#    else
#      if defined(_MSC_VER) || defined(__CYGWIN__)
#        define VULKANRHI_API __declspec(dllimport)
#      else
#        define VULKANRHI_API
#      endif
#    endif
#  endif
#endif



// Export macros for core module
#if !defined(CORE_API)
#  if defined(USE_STATIC_LIBS)
#    define CORE_API
#  else
#    if defined(EXPORT_CORE)
#      if defined(_MSC_VER) || defined(__CYGWIN__)
#        define CORE_API __declspec(dllexport)
#      else
#        define CORE_API __attribute__((visibility("default")))
#      endif
#    else
#      if defined(_MSC_VER) || defined(__CYGWIN__)
#        define CORE_API __declspec(dllimport)
#      else
#        define CORE_API
#      endif
#    endif
#  endif
#endif



// Export macros for rhi module
#if !defined(RHI_API)
#  if defined(USE_STATIC_LIBS)
#    define RHI_API
#  else
#    if defined(EXPORT_RHI)
#      if defined(_MSC_VER) || defined(__CYGWIN__)
#        define RHI_API __declspec(dllexport)
#      else
#        define RHI_API __attribute__((visibility("default")))
#      endif
#    else
#      if defined(_MSC_VER) || defined(__CYGWIN__)
#        define RHI_API __declspec(dllimport)
#      else
#        define RHI_API
#      endif
#    endif
#  endif
#endif



// Export macros for rendering module
#if !defined(RENDERING_API)
#  if defined(USE_STATIC_LIBS)
#    define RENDERING_API
#  else
#    if defined(EXPORT_RENDERING)
#      if defined(_MSC_VER) || defined(__CYGWIN__)
#        define RENDERING_API __declspec(dllexport)
#      else
#        define RENDERING_API __attribute__((visibility("default")))
#      endif
#    else
#      if defined(_MSC_VER) || defined(__CYGWIN__)
#        define RENDERING_API __declspec(dllimport)
#      else
#        define RENDERING_API
#      endif
#    endif
#  endif
#endif



// Export macros for system module
#if !defined(SYSTEM_API)
#  if defined(USE_STATIC_LIBS)
#    define SYSTEM_API
#  else
#    if defined(EXPORT_SYSTEM)
#      if defined(_MSC_VER) || defined(__CYGWIN__)
#        define SYSTEM_API __declspec(dllexport)
#      else
#        define SYSTEM_API __attribute__((visibility("default")))
#      endif
#    else
#      if defined(_MSC_VER) || defined(__CYGWIN__)
#        define SYSTEM_API __declspec(dllimport)
#      else
#        define SYSTEM_API
#      endif
#    endif
#  endif
#endif


