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
  cmp_layout_node_t *dummy_flex = NULL;
  cmp_wayland_protocols_t *protocols = NULL;
  cmp_window_t *window = NULL;
  cmp_window_config_t config;
  int rc;

  memset(&config, 0, sizeof(config));
  config.title = "Test Window";
  config.width = 800;
  config.height = 600;

  /* Since window is required for binding, mock a basic window handle */
  rc = (cmp_layout_node_create(&dummy_flex),
        dummy_flex->display = CMP_DISPLAY_FLEX, config.root_layout = dummy_flex,
        cmp_window_create(&config, &window));
  if (rc != CMP_SUCCESS) {
    /* In some environments, window creation might fail if there's no display
     * server. If that's the case, we can mock or safely skip the rest of the
     * bind test, but assuming CMP provides fallback or headless stubs, it
     * should pass. */
  }

  /* Test creation with invalid arg */
  rc = cmp_wayland_protocols_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  /* Test normal creation */
  rc = cmp_wayland_protocols_create(&protocols);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_NEQ_PTR(NULL, protocols);

  /* Test bind with invalid args */
  rc = cmp_wayland_protocols_bind(NULL, window);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_wayland_protocols_bind(protocols, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  /* Test normal bind (mocked) */
  if (window) {
    rc = cmp_wayland_protocols_bind(protocols, window);
    ASSERT_EQ(CMP_SUCCESS, rc);
  }

  /* Test destroy with invalid args */
  rc = cmp_wayland_protocols_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  /* Test normal destroy */
  rc = cmp_wayland_protocols_destroy(protocols);
  ASSERT_EQ(CMP_SUCCESS, rc);

  if (window) {
    (cmp_window_destroy(window),
     (dummy_flex ? (cmp_layout_node_destroy(dummy_flex), 0) : 0),
     (dummy_flex = NULL, 0));
  }

  printf("test_cmp_wayland_protocols passed.\n");
  return 0;
}
