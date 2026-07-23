#ifndef UI_INTERNAL_MEM_H
#define UI_INTERNAL_MEM_H

/* clang-format off */
#include <stdlib.h>
#include "ui_internal_mem.h"

#ifdef UI_TEST_MOCK_ALLOC
extern void* ui_mock_malloc(size_t size);
extern void* ui_mock_realloc(void* ptr, size_t size);
extern void ui_mock_free(void* ptr);
#define UI_MALLOC ui_mock_malloc
#define UI_REALLOC ui_mock_realloc
#define UI_FREE ui_mock_free
#else
#define UI_MALLOC malloc
#define UI_REALLOC realloc
#define UI_FREE free
#endif

#if defined(_MSC_VER)
#include <stdio.h>
#include <string.h>
#define UI_STRCPY(dest, destsz, src) strcpy_s((dest), (destsz), (src))
#define UI_STRNCPY(dest, destsz, src, count) strncpy_s((dest), (destsz), (src), (count))
#define UI_STRCAT(dest, destsz, src) strcat_s((dest), (destsz), (src))
#else
#include <stdio.h>
#include <string.h>
/* clang-format on */
#define UI_STRCPY(dest, destsz, src) (strcpy((dest), (src)), 0)
#define UI_STRNCPY(dest, destsz, src, count)                                   \
  (((destsz) > 0)                                                              \
       ? (strncpy((dest), (src),                                               \
                  (count) < (destsz) ? (count) : ((destsz) - 1)),              \
          (dest)[(count) < (destsz) ? (count) : ((destsz) - 1)] = '\0', 0)     \
       : 0)
#define UI_STRCAT(dest, destsz, src) (strcat((dest), (src)), 0)
#endif

#endif /* UI_INTERNAL_MEM_H */
