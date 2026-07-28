
#ifndef RENDERING_API_H
#define RENDERING_API_H

#ifdef RENDERING_STATIC_DEFINE
#  define RENDERING_API
#  define RENDERING_NO_EXPORT
#else
#  ifndef RENDERING_API
#    ifdef rendering_EXPORTS
        /* We are building this library */
#      define RENDERING_API __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define RENDERING_API __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef RENDERING_NO_EXPORT
#    define RENDERING_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef RENDERING_DEPRECATED
#  define RENDERING_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef RENDERING_DEPRECATED_EXPORT
#  define RENDERING_DEPRECATED_EXPORT RENDERING_API RENDERING_DEPRECATED
#endif

#ifndef RENDERING_DEPRECATED_NO_EXPORT
#  define RENDERING_DEPRECATED_NO_EXPORT RENDERING_NO_EXPORT RENDERING_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef RENDERING_NO_DEPRECATED
#    define RENDERING_NO_DEPRECATED
#  endif
#endif

#endif /* RENDERING_API_H */
