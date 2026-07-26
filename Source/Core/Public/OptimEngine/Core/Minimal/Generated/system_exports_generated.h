
#ifndef SYSTEM_API_H
#define SYSTEM_API_H

#ifdef SYSTEM_STATIC_DEFINE
#  define SYSTEM_API
#  define SYSTEM_NO_EXPORT
#else
#  ifndef SYSTEM_API
#    ifdef system_EXPORTS
        /* We are building this library */
#      define SYSTEM_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define SYSTEM_API __declspec(dllimport)
#    endif
#  endif

#  ifndef SYSTEM_NO_EXPORT
#    define SYSTEM_NO_EXPORT 
#  endif
#endif

#ifndef SYSTEM_DEPRECATED
#  define SYSTEM_DEPRECATED __declspec(deprecated)
#endif

#ifndef SYSTEM_DEPRECATED_EXPORT
#  define SYSTEM_DEPRECATED_EXPORT SYSTEM_API SYSTEM_DEPRECATED
#endif

#ifndef SYSTEM_DEPRECATED_NO_EXPORT
#  define SYSTEM_DEPRECATED_NO_EXPORT SYSTEM_NO_EXPORT SYSTEM_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SYSTEM_NO_DEPRECATED
#    define SYSTEM_NO_DEPRECATED
#  endif
#endif

#endif /* SYSTEM_API_H */
