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
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#pragma GCC diagnostic ignored "-Wendif-labels"
#pragma GCC diagnostic ignored "-Wcomment"
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,u)  ((void)(u),C_MULTIPLATFORM_MALLOC(x))
#define STBTT_free(x,u)    ((void)(u),C_MULTIPLATFORM_FREE(x))
#include "../include/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_MALLOC(x)    C_MULTIPLATFORM_MALLOC(x)
#define STBIW_REALLOC(p,x) C_MULTIPLATFORM_REALLOC(p,x)
#define STBIW_FREE(x)      C_MULTIPLATFORM_FREE(x)
#include "../include/stb_image_write.h"

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
/* clang-format on */
