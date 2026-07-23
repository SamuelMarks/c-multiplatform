/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include "../include/ui_types.h"
/* clang-format on */

typedef char static_assert_ui_int8[sizeof(ui_int8) == 1 ? 1 : -1];
typedef char static_assert_ui_uint8[sizeof(ui_uint8) == 1 ? 1 : -1];
typedef char static_assert_ui_int16[sizeof(ui_int16) == 2 ? 1 : -1];
typedef char static_assert_ui_uint16[sizeof(ui_uint16) == 2 ? 1 : -1];
typedef char static_assert_ui_int32[sizeof(ui_int32) == 4 ? 1 : -1];
typedef char static_assert_ui_uint32[sizeof(ui_uint32) == 4 ? 1 : -1];
typedef char static_assert_ui_int64[sizeof(ui_int64) == 8 ? 1 : -1];
typedef char static_assert_ui_uint64[sizeof(ui_uint64) == 8 ? 1 : -1];
typedef char
    static_assert_ui_intptr[sizeof(ui_intptr) == sizeof(void *) ? 1 : -1];
typedef char
    static_assert_ui_uintptr[sizeof(ui_uintptr) == sizeof(void *) ? 1 : -1];

extern int g_malloc_fail_countdown;
extern int g_malloc_called;

void *ui_mock_malloc(size_t size);
void *ui_mock_realloc(void *ptr, size_t size);
void ui_mock_free(void *ptr);

int main(void) {
  void *p;
  void *p2;

  g_malloc_fail_countdown = 0;
  p = ui_mock_malloc(10);
  if (p != NULL)
    return 1;

  g_malloc_fail_countdown = 1;
  p = ui_mock_malloc(10);
  if (p == NULL)
    return 1;

  g_malloc_fail_countdown = 0;
  p2 = ui_mock_realloc(p, 20);
  if (p2 != NULL)
    return 1;

  g_malloc_fail_countdown = 1;
  p2 = ui_mock_realloc(p, 20);
  if (p2 == NULL)
    return 1;

  ui_mock_free(p2);

  printf("All types tests passed.\n");
  return 0;
}
