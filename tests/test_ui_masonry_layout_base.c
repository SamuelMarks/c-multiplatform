/* clang-format off */
#include "ui_masonry_layout_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* clang-format on */

static enum ui_error test_masonry_creation(void) {
  struct ui_masonry_layout_base *masonry = NULL;
  enum ui_error rc = ui_masonry_layout_base_create(&masonry);
  assert(rc == UI_ERROR_NONE);
  assert(masonry != NULL);
  ui_masonry_layout_base_destroy(masonry);
  printf("test_masonry_creation passed\n");
  return UI_ERROR_NONE;
}

static enum ui_error test_masonry_reflow(void) {
  struct ui_masonry_layout_base *masonry = NULL;
  enum ui_error rc;

  rc = ui_masonry_layout_base_create(&masonry);
  assert(rc == UI_ERROR_NONE);

  rc = ui_masonry_layout_base_reflow(masonry);
  assert(rc == UI_ERROR_NONE);

  ui_masonry_layout_base_destroy(masonry);
  printf("test_masonry_reflow passed\n");
  return UI_ERROR_NONE;
}

extern int g_malloc_fail_countdown;

static void test_masonry_errors(void) {
  struct ui_masonry_layout_base *masonry = NULL;
  struct ui_component *comp = NULL;

  if (ui_masonry_layout_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;

  ui_masonry_layout_base_destroy(NULL);

  if (ui_masonry_layout_base_reflow(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;

  if (ui_masonry_layout_base_get_component(NULL, &comp) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_masonry_layout_base_get_component(masonry, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;

  if (ui_masonry_layout_base_bind_data(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;

  ui_masonry_layout_base_create(&masonry);
  ui_masonry_layout_base_get_component(masonry, &comp);
  ui_masonry_layout_base_bind_data(masonry, NULL);
  ui_masonry_layout_base_destroy(masonry);

#ifdef UI_TEST_MOCK_ALLOC
  int i;
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    ui_masonry_layout_base_create(&masonry);
    g_malloc_fail_countdown = -1;
  }
#endif
}

int main(void) {
  test_masonry_errors();
  test_masonry_creation();
  test_masonry_reflow();
  return 0;
}
