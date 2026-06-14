/* bimg: amalgamated dav1d 16-bit DSP templates (one translation unit per bitdepth). */

/* Platform feature macros must be set before any system header is included. */
#if !defined(_WIN32)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE       // posix_memalign, PTHREAD_STACK_MIN (glibc), etc.
#	endif
#	ifndef _FILE_OFFSET_BITS
#		define _FILE_OFFSET_BITS 64 // 64-bit off_t on 32-bit POSIX (e.g. Android)
#	endif
#endif

/* macOS <sys/param.h> defines a 1-arg ALIGN() that clashes with dav1d's 2-arg
 * ALIGN(); include it first (header-guarded) and drop its macro so dav1d's wins. */
#if defined(__APPLE__)
#	include <sys/param.h>
#	undef ALIGN
#endif

#if defined(_MSC_VER)
#	pragma warning(push)
#	pragma warning(disable: 4057) // warning C4057: indirection to slightly different base types
#	pragma warning(disable: 4090) // warning C4090: different 'const' qualifiers
#	pragma warning(disable: 4100) // warning C4100: unreferenced formal parameter
#	pragma warning(disable: 4127) // warning C4127: conditional expression is constant
#	pragma warning(disable: 4152) // warning C4152: function/data pointer conversion in expression
#	pragma warning(disable: 4189) // warning C4189: local variable is initialized but not referenced
#	pragma warning(disable: 4200) // warning C4200: zero-sized array in struct/union
#	pragma warning(disable: 4201) // warning C4201: nonstandard extension: nameless struct/union
#	pragma warning(disable: 4244) // warning C4244: conversion, possible loss of data
#	pragma warning(disable: 4245) // warning C4245: conversion, signed/unsigned mismatch
#	pragma warning(disable: 4324) // warning C4324: structure was padded due to alignment specifier
#	pragma warning(disable: 4389) // warning C4389: signed/unsigned mismatch
#	pragma warning(disable: 4456) // warning C4456: declaration hides previous local declaration
#	pragma warning(disable: 4457) // warning C4457: declaration hides function parameter
#	pragma warning(disable: 4701) // warning C4701: potentially uninitialized local variable used
#	pragma warning(disable: 4702) // warning C4702: unreachable code
#	pragma warning(disable: 4703) // warning C4703: potentially uninitialized local pointer variable used
#	pragma warning(disable: 5287) // warning C5287: operands are different enum types
#elif defined(__clang__)
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wunused-parameter"
#	pragma clang diagnostic ignored "-Wunused-function"
#	pragma clang diagnostic ignored "-Wunused-variable"
#	pragma clang diagnostic ignored "-Wsign-compare"
#	pragma clang diagnostic ignored "-Wmissing-field-initializers"
#	pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#elif defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-parameter"
#	pragma GCC diagnostic ignored "-Wunused-function"
#	pragma GCC diagnostic ignored "-Wunused-variable"
#	pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#	pragma GCC diagnostic ignored "-Wsign-compare"
#	pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#	pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#	pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#	pragma GCC diagnostic ignored "-Wtype-limits"
#endif

#define BITDEPTH 16

#include "src/cdef_apply_tmpl.c"
#include "src/cdef_tmpl.c"
#include "src/fg_apply_tmpl.c"
#include "src/filmgrain_tmpl.c"
#include "src/ipred_prepare_tmpl.c"
#include "src/ipred_tmpl.c"
#include "src/itx_tmpl.c"
#include "src/lf_apply_tmpl.c"
#include "src/loopfilter_tmpl.c"
#include "src/looprestoration_tmpl.c"
#include "src/lr_apply_tmpl.c"
#include "src/mc_tmpl.c"
#include "src/recon_tmpl.c"

#if defined(_MSC_VER)
#	pragma warning(pop)
#elif defined(__clang__)
#	pragma clang diagnostic pop
#elif defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif
