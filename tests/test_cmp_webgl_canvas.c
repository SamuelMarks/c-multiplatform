/* clang-format off */
#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define ASSERT_EQ(expected, actual)                                            \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      printf("%s:%d: Assertion failed: expected %d, got %d\n", __FILE__,       \
             __LINE__, (int)(expected), (int)(actual));                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_NEQ_PTR(expected, actual)                                       \
  do {                                                                         \
    if ((expected) == (actual)) {                                              \
      printf("%s:%d: Assertion failed: expected not %p, got %p\n", __FILE__,   \
             __LINE__, (void *)(expected), (void *)(actual));                  \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main(void) {
  cmp_webgl_canvas_t *canvas = NULL;
  int rc;

  /* Test creation with invalid arg */
  rc = cmp_webgl_canvas_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  /* Test normal creation */
  rc = cmp_webgl_canvas_create(&canvas);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_NEQ_PTR(NULL, canvas);

  /* Test bind with invalid args */
  rc = cmp_webgl_canvas_bind(NULL, "#canvas");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_webgl_canvas_bind(canvas, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  /* Test normal bind */
  rc = cmp_webgl_canvas_bind(canvas, "#canvas");
  ASSERT_EQ(CMP_SUCCESS, rc);

  /* Test destroy with invalid arg */
  rc = cmp_webgl_canvas_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  /* Test normal destroy */
  rc = cmp_webgl_canvas_destroy(canvas);
  ASSERT_EQ(CMP_SUCCESS, rc);

  printf("test_cmp_webgl_canvas passed.\n");
  return 0;
}
