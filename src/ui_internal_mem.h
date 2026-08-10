#ifndef UI_INTERNAL_MEM_H
#define UI_INTERNAL_MEM_H

/* clang-format off */
#include <stdlib.h>
#include <string.h>

extern void *ui_mock_malloc(size_t size);
extern void *ui_mock_realloc(void *ptr, size_t size);
extern void ui_mock_free(void *ptr);
extern char *ui_mock_strdup(const char *src);

#ifndef C_MULTIPLATFORM_MALLOC
#define C_MULTIPLATFORM_MALLOC malloc
#endif

#ifndef C_MULTIPLATFORM_FREE
#define C_MULTIPLATFORM_FREE free
#endif

#ifndef C_MULTIPLATFORM_REALLOC
#define C_MULTIPLATFORM_REALLOC realloc
#endif

#ifndef C_MULTIPLATFORM_CALLOC
#define C_MULTIPLATFORM_CALLOC calloc
#endif

#ifndef C_MULTIPLATFORM_STRDUP
#if defined(_MSC_VER)
#define C_MULTIPLATFORM_STRDUP _strdup
#else
#define C_MULTIPLATFORM_STRDUP strdup
#endif
#endif






#ifdef UI_TEST_MOCK_ALLOC
extern int g_mock_strcpy_fail;
static int mock_strcpy_fail_check(void) __attribute__((unused));
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
#define UI_STRCPY(dest, destsz, src) (mock_strcpy_fail_check() ? -1 : strcpy_s((dest), (destsz), (src)))
#define UI_STRNCPY(dest, destsz, src, count) strncpy_s((dest), (destsz), (src), (count))
#else
#define UI_STRCPY(dest, destsz, src) (mock_strcpy_fail_check() ? -1 : (strcpy((dest), (src)), 0))
#define UI_STRNCPY(dest, destsz, src, count) strncpy((dest), (src), (count))
#endif
#else
#if defined(_MSC_VER)
#define UI_STRCPY(dest, destsz, src) strcpy_s((dest), (destsz), (src))
#define UI_STRNCPY(dest, destsz, src, count) strncpy_s((dest), (destsz), (src), (count))
#else
#define UI_STRCPY(dest, destsz, src) (strcpy((dest), (src)), 0)
#define UI_STRNCPY(dest, destsz, src, count) strncpy((dest), (src), (count))
#endif
#endif

#endif /* UI_INTERNAL_MEM_H */
