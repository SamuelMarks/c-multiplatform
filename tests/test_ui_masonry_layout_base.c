/* clang-format off */
#include "ui_masonry_layout_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* clang-format on */

static ui_error_t test_masonry_creation(void) {
  struct ui_masonry_layout_base *masonry = NULL;
  ui_error_t rc = ui_masonry_layout_base_create(&masonry);
  assert(rc == UI_ERROR_NONE);
  assert(masonry != NULL);
  {
    ui_error_t rc_cleanup = ui_masonry_layout_base_destroy(masonry);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  printf("test_masonry_creation passed\n");
  return UI_ERROR_NONE;
}

static ui_error_t test_masonry_reflow(void) {
  struct ui_masonry_layout_base *masonry = NULL;
  ui_error_t rc;

  rc = ui_masonry_layout_base_create(&masonry);
  assert(rc == UI_ERROR_NONE);

  rc = ui_masonry_layout_base_reflow(masonry);
  assert(rc == UI_ERROR_NONE);

  {
    ui_error_t rc_cleanup = ui_masonry_layout_base_destroy(masonry);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  printf("test_masonry_reflow passed\n");
  return UI_ERROR_NONE;
}

extern int g_malloc_fail_countdown;

static void test_masonry_errors(void) {
  struct ui_masonry_layout_base *masonry = NULL;
  struct ui_component *comp = NULL;

  if (ui_masonry_layout_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;

  {
    ui_error_t rc_cleanup = ui_masonry_layout_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  if (ui_masonry_layout_base_reflow(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;

  masonry = NULL;
  if (ui_masonry_layout_base_get_component(NULL, &comp) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  if (ui_masonry_layout_base_get_component(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;

  if (ui_masonry_layout_base_bind_data(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return;

  ui_masonry_layout_base_create(&masonry);
  if (ui_masonry_layout_base_get_component(masonry, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return;
  ui_masonry_layout_base_get_component(masonry, &comp);
  ui_masonry_layout_base_bind_data(masonry, NULL);
  {
    ui_error_t rc_cleanup = ui_masonry_layout_base_destroy(masonry);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  int i;
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    ui_masonry_layout_base_create(&masonry);
    g_malloc_fail_countdown = -1;
  }
}

int main(void) {
  test_masonry_errors();
  test_masonry_creation();
  test_masonry_reflow();
  return 0;
}
