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
#endif

#if defined(__GNUC__) || defined(__clang__)
#if defined(__clang__)
#endif
#endif

/** @brief internal */
#define STB_TRUETYPE_IMPLEMENTATION
/** @brief internal */
#define STBTT_malloc(x,u)  ((u) ? C_MULTIPLATFORM_MALLOC(x) : C_MULTIPLATFORM_MALLOC(x))
/** @brief internal */
#define STBTT_free(x,u)    do { if (u) {} C_MULTIPLATFORM_FREE(x); } while (0)
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

/** @brief internal */
#define STB_IMAGE_IMPLEMENTATION
/** @brief internal */
#define STBI_MALLOC(x)    C_MULTIPLATFORM_MALLOC(x)
/** @brief internal */
#define STBI_REALLOC(p,x) C_MULTIPLATFORM_REALLOC(p,x)
/** @brief internal */
#define STBI_FREE(x)      C_MULTIPLATFORM_FREE(x)
#include "stb_image.h"

#if defined(_MSC_VER)
#endif

#if defined(__GNUC__) || defined(__clang__)
#endif
/* clang-format on */
