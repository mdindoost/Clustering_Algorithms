
#ifndef LIBLEIDENALG_EXPORT_H
#define LIBLEIDENALG_EXPORT_H

#ifdef LEIDENALG_STATIC
#  define LIBLEIDENALG_EXPORT
#  define LIBLEIDENALG_NO_EXPORT
#else
#  ifndef LIBLEIDENALG_EXPORT
#    ifdef libleidenalg_EXPORTS
        /* We are building this library */
#      define LIBLEIDENALG_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define LIBLEIDENALG_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef LIBLEIDENALG_NO_EXPORT
#    define LIBLEIDENALG_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef LIBLEIDENALG_DEPRECATED
#  define LIBLEIDENALG_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef LIBLEIDENALG_DEPRECATED_EXPORT
#  define LIBLEIDENALG_DEPRECATED_EXPORT LIBLEIDENALG_EXPORT LIBLEIDENALG_DEPRECATED
#endif

#ifndef LIBLEIDENALG_DEPRECATED_NO_EXPORT
#  define LIBLEIDENALG_DEPRECATED_NO_EXPORT LIBLEIDENALG_NO_EXPORT LIBLEIDENALG_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef LIBLEIDENALG_NO_DEPRECATED
#    define LIBLEIDENALG_NO_DEPRECATED
#  endif
#endif

#endif /* LIBLEIDENALG_EXPORT_H */
