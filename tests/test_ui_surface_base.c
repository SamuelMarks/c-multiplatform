/* clang-format off */
#include "ui_surface_base.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define ASSERT_SUCCESS(expr)                                                   \
  do {                                                                         \
    ui_error_t err = (expr);                                                   \
    if (err != UI_ERROR_NONE) {                                                \
      printf("Failed at line %d: %d\n", __LINE__, err);                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)
#define ASSERT_EQ(expr, expected)                                              \
  do {                                                                         \
    ui_error_t err = (expr);                                                   \
    if (err != (expected)) {                                                   \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             err);                                                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)

static int run_normal_tests(void) {
  struct ui_surface_base *surface = NULL;

  ASSERT_EQ(ui_surface_base_create(NULL), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_surface_base_set_elevation(NULL, UI_ELEVATION_LEVEL_1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_surface_base_bind_data(NULL, NULL), UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_surface_base_create(&surface));
  if (!surface)
    return 1;

  ASSERT_SUCCESS(ui_surface_base_set_elevation(surface, UI_ELEVATION_LEVEL_3));
  ASSERT_SUCCESS(ui_surface_base_bind_data(surface, NULL));

  /* Missing destroy fn in api! Just free for tests */
  if (surface) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(&surface->base);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_surface_base *surface = NULL;

  g_malloc_fail_countdown = 0;
  if (ui_surface_base_create(&surface) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 1;
  if (ui_surface_base_create(&surface) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

#endif
  return 0;
}

int main(void) {
  if (run_normal_tests() != 0)
    return 1;
  struct ui_surface_base *s2;
  g_malloc_fail_countdown = 2;
  ui_surface_base_create(&s2);
  g_malloc_fail_countdown = 3;
  ui_surface_base_create(&s2);
  g_malloc_fail_countdown = -1;

  if (run_oom_tests() != 0)
    return 1;
  printf("All ui_surface_base tests passed.\n");
  return 0;
}
