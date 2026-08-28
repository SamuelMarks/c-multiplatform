/**
 * @file ui_stb_impl.c
 * @brief ui_stb_impl.c implementation.
 */
/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_internal_mem.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4204) /* nonstandard extension used: non-constant aggregate initializer */
#pragma warning(disable : 4244) /* possible loss of data */
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wc++-compat"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wimplicit-int-float-conversion"
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#pragma GCC diagnostic ignored "-Wendif-labels"
#pragma GCC diagnostic ignored "-Wcomment"
#endif

/** @brief internal */
#define STB_TRUETYPE_IMPLEMENTATION
/** @brief internal */
#define STBTT_malloc(x,u)  ((void)(u),C_MULTIPLATFORM_MALLOC(x))
/** @brief internal */
#define STBTT_free(x,u)    ((void)(u),C_MULTIPLATFORM_FREE(x))
#include "stb_truetype.h"

/** @brief internal */
#define STB_IMAGE_WRITE_IMPLEMENTATION
/** @brief internal */
#define STBIW_MALLOC(x)    C_MULTIPLATFORM_MALLOC(x)
/** @brief internal */
#define STBIW_REALLOC(p,x) C_MULTIPLATFORM_REALLOC(p,x)
/** @brief internal */
#define STBIW_FREE(x)      C_MULTIPLATFORM_FREE(x)
#include "stb_image_write.h"

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
/* clang-format on */
