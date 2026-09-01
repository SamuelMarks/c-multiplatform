/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "ui_css_highlight_api.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_highlight_create(void) {
  struct ui_css_highlight *hl = NULL;

  if (ui_css_highlight_create(UI_CSS_HIGHLIGHT_TYPE_CUSTOM, "test", NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_highlight_create(UI_CSS_HIGHLIGHT_TYPE_CUSTOM, NULL, &hl) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_highlight_create(UI_CSS_HIGHLIGHT_TYPE_SELECTION, NULL, &hl) !=
      UI_ERROR_NONE)
    return 1;
  ui_css_highlight_destroy(hl);

  if (ui_css_highlight_create(UI_CSS_HIGHLIGHT_TYPE_CUSTOM, "my-hl", &hl) !=
      UI_ERROR_NONE)
    return 1;
  ui_css_highlight_destroy(hl);
  ui_css_highlight_destroy(NULL); /* no-op */

  /* OOM */
  g_malloc_fail_countdown = 0;
  if (ui_css_highlight_create(UI_CSS_HIGHLIGHT_TYPE_SELECTION, NULL, &hl) !=
      UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 1;
  if (ui_css_highlight_create(UI_CSS_HIGHLIGHT_TYPE_CUSTOM, "my-hl", &hl) !=
      UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  return 0;
}

static int test_highlight_range(void) {
  struct ui_css_highlight *hl = NULL;

  if (ui_css_highlight_add_range(NULL, 0, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_css_highlight_create(UI_CSS_HIGHLIGHT_TYPE_SELECTION, NULL, &hl);

  if (ui_css_highlight_add_range(hl, 0, 5) != UI_ERROR_NONE)
    return 1;
  if (ui_css_highlight_add_range(hl, 10, 15) != UI_ERROR_NONE)
    return 1;
  if (ui_css_highlight_add_range(hl, 20, 25) != UI_ERROR_NONE)
    return 1;
  if (ui_css_highlight_add_range(hl, 30, 35) != UI_ERROR_NONE)
    return 1;

  /* At capacity 4, adding next will trigger realloc */
  g_malloc_fail_countdown = 0;
  if (ui_css_highlight_add_range(hl, 40, 45) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  if (ui_css_highlight_add_range(hl, 40, 45) != UI_ERROR_NONE)
    return 1; /* this time it works */

  ui_css_highlight_destroy(hl);

  /* fresh one to OOM on first alloc */
  ui_css_highlight_create(UI_CSS_HIGHLIGHT_TYPE_SELECTION, NULL, &hl);
  g_malloc_fail_countdown = 0;
  if (ui_css_highlight_add_range(hl, 0, 5) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;
  ui_css_highlight_destroy(hl);

  return 0;
}

static int test_highlight_cleanup_coverage(void) {
  /* We want to hit the cleanup goto when hl is allocated but custom_name fails
   * to allocate */
  struct ui_css_highlight *hl = NULL;

  g_malloc_fail_countdown = 1; /* first malloc succeeds (for the struct), second
                                  fails (for the string) */
  ui_css_highlight_create(UI_CSS_HIGHLIGHT_TYPE_CUSTOM, "my-hl", &hl);
  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  int failures = 0;

  failures += test_highlight_create();
  failures += test_highlight_range();
  failures += test_highlight_cleanup_coverage();

  if (failures == 0) {
    printf("test_ui_css_highlight_api passed\n");
  } else {
    printf("test_ui_css_highlight_api failed with %d errors\n", failures);
  }

  return failures;
}
