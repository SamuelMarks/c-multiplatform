/* clang-format off */
#include "cmp.h"
#include <stdio.h>
/* clang-format on */

#define ASSERT_EQ(expected, actual)                                            \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      printf("%s:%d: Assertion failed: expected %d, got %d\n", __FILE__,       \
             __LINE__, (int)(expected), (int)(actual));                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main(void) {
  cmp_materials_t *materials = NULL;
  cmp_window_t *window = NULL;
  cmp_window_config_t config;
  int rc;

  /* Test with invalid args */
  rc = cmp_win32_request_windows_material(materials, NULL,
                                          CMP_WINDOWS_MATERIAL_MICA);
#if defined(_WIN32)
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);
#else
  ASSERT_EQ(CMP_SUCCESS, rc);
#endif

  /* Create mock window to test real invocation */
  memset(&config, 0, sizeof(config));
  config.title = "Test Window";
  config.width = 800;
  config.height = 600;

  rc = cmp_window_create(&config, &window);
  if (rc == CMP_SUCCESS) {
    /* Test valid arguments */
    rc = cmp_win32_request_windows_material(materials, window,
                                            CMP_WINDOWS_MATERIAL_NONE);
    ASSERT_EQ(CMP_SUCCESS, rc);

    rc = cmp_win32_request_windows_material(materials, window,
                                            CMP_WINDOWS_MATERIAL_MICA);
    ASSERT_EQ(CMP_SUCCESS, rc);

    cmp_window_destroy(window);
  }

  printf("test_cmp_win32_window passed.\n");
  return 0;
}
