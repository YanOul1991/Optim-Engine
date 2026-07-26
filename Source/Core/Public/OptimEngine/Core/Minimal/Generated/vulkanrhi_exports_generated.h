
#ifndef VULKANRHI_API_H
#define VULKANRHI_API_H

#ifdef VULKANRHI_STATIC_DEFINE
#  define VULKANRHI_API
#  define VULKANRHI_NO_EXPORT
#else
#  ifndef VULKANRHI_API
#    ifdef vulkanrhi_EXPORTS
        /* We are building this library */
#      define VULKANRHI_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define VULKANRHI_API __declspec(dllimport)
#    endif
#  endif

#  ifndef VULKANRHI_NO_EXPORT
#    define VULKANRHI_NO_EXPORT 
#  endif
#endif

#ifndef VULKANRHI_DEPRECATED
#  define VULKANRHI_DEPRECATED __declspec(deprecated)
#endif

#ifndef VULKANRHI_DEPRECATED_EXPORT
#  define VULKANRHI_DEPRECATED_EXPORT VULKANRHI_API VULKANRHI_DEPRECATED
#endif

#ifndef VULKANRHI_DEPRECATED_NO_EXPORT
#  define VULKANRHI_DEPRECATED_NO_EXPORT VULKANRHI_NO_EXPORT VULKANRHI_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef VULKANRHI_NO_DEPRECATED
#    define VULKANRHI_NO_DEPRECATED
#  endif
#endif

#endif /* VULKANRHI_API_H */
