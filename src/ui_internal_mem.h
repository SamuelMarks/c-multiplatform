/**
 * @file ui_internal_mem.h
 * @brief ui_internal_mem.h implementation.
 */
#ifndef UI_INTERNAL_MEM_H
#define UI_INTERNAL_MEM_H

/* clang-format off */
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief ui_mock_malloc.
 * @param size Parameter size.
 * @return Return value.
 */
extern void *ui_mock_malloc(size_t size);
/**
 * @brief ui_mock_realloc.
 * @param ptr Parameter ptr.
 * @param size Parameter size.
 * @return Return value.
 */
extern void *ui_mock_realloc(void *ptr, size_t size);
/**
 * @brief ui_mock_free.
 * @param ptr Parameter ptr.
 * @return Return value.
 */
extern void ui_mock_free(void *ptr);
/**
 * @brief ui_mock_strdup.
 * @param src Parameter src.
 * @return Return value.
 */
extern char *ui_mock_strdup(const char *src);

/** @def C_MULTIPLATFORM_MALLOC
 * @brief Platform malloc
 */
#ifndef C_MULTIPLATFORM_MALLOC
#define C_MULTIPLATFORM_MALLOC malloc
#endif

/** @def C_MULTIPLATFORM_FREE
 * @brief Platform free
 */
#ifndef C_MULTIPLATFORM_FREE
#define C_MULTIPLATFORM_FREE free
#endif

/** @def C_MULTIPLATFORM_REALLOC
 * @brief Platform realloc
 */
#ifndef C_MULTIPLATFORM_REALLOC
#define C_MULTIPLATFORM_REALLOC realloc
#endif

/** @def C_MULTIPLATFORM_CALLOC
 * @brief Platform calloc
 */
#ifndef C_MULTIPLATFORM_CALLOC
#define C_MULTIPLATFORM_CALLOC calloc
#endif

/** @def C_MULTIPLATFORM_STRDUP
 * @brief Platform strdup
 */
#ifndef C_MULTIPLATFORM_STRDUP
#if defined(_MSC_VER)
#define C_MULTIPLATFORM_STRDUP _strdup
#else
#if defined(__CYGWIN__)
char *strdup(const char *);
#endif
#define C_MULTIPLATFORM_STRDUP strdup
#endif
#endif

#ifdef UI_TEST_MOCK_ALLOC
extern int g_mock_strcpy_fail;
#if defined(__GNUC__) || defined(__clang__)
static int mock_strcpy_fail_check(void) __attribute__((unused));
#endif
/**
 * @brief mock_strcpy_fail_check.
 * @return Return value.
 */
static int mock_strcpy_fail_check(void) {
  if (g_mock_strcpy_fail == 1) {
    g_mock_strcpy_fail = 0;
    return 1;
  }
  if (g_mock_strcpy_fail == 2) {
    /* fail on the second call */
    static int calls = 0;
    if (++calls == 2) {
      calls = 0;
      g_mock_strcpy_fail = 0;
      return 1;
    }
    return 0;
  }
  return 0;
}
#if defined(_MSC_VER)
/** @def UI_STRCPY
 * @brief Safe strcpy wrapper
 */
#define UI_STRCPY(dest, destsz, src)                                           \
  (mock_strcpy_fail_check() ? -1 : strcpy_s((dest), (destsz), (src)))
/** @def UI_STRNCPY
 * @brief Safe strncpy wrapper
 */
#define UI_STRNCPY(dest, destsz, src, count)                                   \
  strncpy_s((dest), (destsz), (src), (count))
#define UI_STRCAT(dest, destsz, src) strcat_s((dest), (destsz), (src))
#else
/** @def UI_STRCPY
 * @brief Safe strcpy wrapper
 */
#define UI_STRCPY(dest, destsz, src)                                           \
  (mock_strcpy_fail_check() ? -1 : (strcpy((dest), (src)) == NULL ? -1 : 0))
/** @def UI_STRNCPY
 * @brief Safe strncpy wrapper
 */
#define UI_STRNCPY(dest, destsz, src, count) strncpy((dest), (src), (count))
#define UI_STRCAT(dest, destsz, src) (strcat((dest), (src)) == NULL ? -1 : 0)
#endif
#else
#if defined(_MSC_VER)
/** @def UI_STRCPY
 * @brief Safe strcpy wrapper
 */
#define UI_STRCPY(dest, destsz, src) strcpy_s((dest), (destsz), (src))
/** @def UI_STRNCPY
 * @brief Safe strncpy wrapper
 */
#define UI_STRNCPY(dest, destsz, src, count)                                   \
  strncpy_s((dest), (destsz), (src), (count))
#define UI_STRCAT(dest, destsz, src) strcat_s((dest), (destsz), (src))
#else
/** @def UI_STRCPY
 * @brief Safe strcpy wrapper
 */
#define UI_STRCPY(dest, destsz, src) (strcpy((dest), (src)) == NULL ? -1 : 0)
/** @def UI_STRNCPY
 * @brief Safe strncpy wrapper
 */
#define UI_STRNCPY(dest, destsz, src, count) strncpy((dest), (src), (count))
#define UI_STRCAT(dest, destsz, src) (strcat((dest), (src)) == NULL ? -1 : 0)
#endif
#endif

#endif /* UI_INTERNAL_MEM_H */
