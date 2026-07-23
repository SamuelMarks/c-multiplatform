/* clang-format off */
#include "ui_platform_detector.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  struct ui_platform_detector *detector = NULL;
  unsigned int caps = 0;
  int has_cap = 0;
  enum ui_error rc;

  printf("Testing invalid arguments...\n");
  if (ui_platform_detector_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_platform_detector_destroy(NULL);
  if (ui_platform_detector_get_capabilities(NULL, &caps) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_platform_detector_create(&detector);
  if (rc != UI_ERROR_NONE || detector == NULL)
    return 1;

  if (ui_platform_detector_get_capabilities(detector, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_platform_detector_has_capability(
          NULL, UI_PLATFORM_CAP_MOUSE, &has_cap) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_platform_detector_has_capability(detector, UI_PLATFORM_CAP_MOUSE,
                                          NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_platform_detector_get_capabilities(detector, &caps);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_platform_detector_has_capability(detector, UI_PLATFORM_CAP_MOUSE,
                                           &has_cap);
  if (rc != UI_ERROR_NONE || has_cap != 1)
    return 1;

  rc = ui_platform_detector_has_capability(detector, UI_PLATFORM_CAP_KEYBOARD,
                                           &has_cap);
  if (rc != UI_ERROR_NONE || has_cap != 1)
    return 1;

  rc = ui_platform_detector_has_capability(detector, 0xFFFFFFFF, &has_cap);
  if (rc != UI_ERROR_NONE || has_cap != 0) {
    return 1;
  }

  ui_platform_detector_destroy(detector);
  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_platform_detector *detector = NULL;
  enum ui_error rc;

  printf("Testing OOM...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_platform_detector_create(&detector);
  if (rc != UI_ERROR_OUT_OF_MEMORY || detector != NULL) {
    printf("Failed to catch OOM in create.\n");
    return 1;
  }
#endif
  return 0;
}

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All ui_platform_detector tests passed.\n");
  return 0;
}
