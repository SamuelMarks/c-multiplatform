/* clang-format off */
#include "ui_banner_base.h"
#include "ui_signal.h"
#include "ui_arena.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static void test_banner_base(void) {
  struct ui_banner_base *banner;
  struct ui_component *base_comp;
  ui_error_t err;
  const char *attr_val;
  int is_open = 0;
  struct ui_computed *anim_sig = NULL;

  /* Invalid arguments */
  assert(ui_banner_base_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  ui_banner_base_destroy(NULL); /* Should not crash */

  err = ui_banner_base_create(&banner);
  assert(err == UI_ERROR_NONE);

  assert(ui_banner_base_get_component(NULL, &base_comp) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_banner_base_get_component(banner, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  err = ui_banner_base_get_component(banner, &base_comp);
  assert(err == UI_ERROR_NONE && base_comp != NULL);

  assert(ui_banner_base_set_text(NULL, "Test") == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_banner_base_set_text(banner, NULL) == UI_ERROR_INVALID_ARGUMENT);

  err = ui_banner_base_set_text(banner, "Warning: Connection Lost");
  assert(err == UI_ERROR_NONE);

  assert(ui_banner_base_set_dismissible(NULL, 1) == UI_ERROR_INVALID_ARGUMENT);

  err = ui_banner_base_set_dismissible(banner, 1);
  assert(err == UI_ERROR_NONE);

  err = ui_banner_base_set_dismissible(banner, 0);
  assert(err == UI_ERROR_NONE);

  assert(base_comp->shadow_root->first_child != NULL);
  assert(strcmp(base_comp->shadow_root->first_child->text_content,
                "Warning: Connection Lost") == 0);

  /* Open / Close */
  assert(ui_banner_base_is_open(NULL, &is_open) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_banner_base_is_open(banner, NULL) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_banner_base_set_open(NULL, 1) == UI_ERROR_INVALID_ARGUMENT);

  assert(ui_banner_base_set_open(banner, 1) == UI_ERROR_NONE);
  assert(ui_banner_base_is_open(banner, &is_open) == UI_ERROR_NONE);
  assert(is_open == 1);

  assert(ui_banner_base_set_open(banner, 1) ==
         UI_ERROR_NONE); /* Already open */

  assert(ui_banner_base_set_open(banner, 0) == UI_ERROR_NONE);
  assert(ui_banner_base_is_open(banner, &is_open) == UI_ERROR_NONE);
  assert(is_open == 0);

  /* Signals */
  struct ui_arena *arena = NULL;
  ui_arena_create(1024, &arena);
  ui_signal_t *signal = NULL;
  union ui_signal_payload init_payload;
  memset(&init_payload, 0, sizeof(init_payload));
  ui_signal_create(arena, init_payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &signal);

  assert(ui_banner_base_bind_open(NULL, signal) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_banner_base_bind_open(banner, signal) == UI_ERROR_NONE);

  /* Trigger signal set */
  assert(ui_banner_base_set_open(banner, 1) == UI_ERROR_NONE);

  assert(ui_banner_base_get_animating_signal(NULL, &anim_sig) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_banner_base_get_animating_signal(banner, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_banner_base_get_animating_signal(banner, &anim_sig) ==
         UI_ERROR_NONE);

  {
    ui_error_t rc_cleanup = ui_banner_base_destroy(banner);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_signal_destroy(signal);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* OOM Loops */
  int i;
  for (i = 0; i < 15; i++) {
    struct ui_banner_base *test_banner = NULL;
    g_malloc_fail_countdown = i;
    err = ui_banner_base_create(&test_banner);
    if (err == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_banner_base_destroy(test_banner);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      break;
    } else {
      assert(err == UI_ERROR_OUT_OF_MEMORY);
    }
  }
  g_malloc_fail_countdown = -1;

  /* OOM loop for set_text */
  err = ui_banner_base_create(&banner);
  assert(err == UI_ERROR_NONE);
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    err = ui_banner_base_set_text(banner, "Hello OOM");
    if (err == UI_ERROR_NONE) {
      break;
    } else {
      assert(err == UI_ERROR_OUT_OF_MEMORY);
    }
  }
  g_malloc_fail_countdown = -1;
  {
    ui_error_t rc_cleanup = ui_banner_base_destroy(banner);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}

int main(void) {
#ifdef UI_TEST_MOCK_ALLOC
  extern ui_error_t run_banner_coverage(void);
  run_banner_coverage();
#endif

  test_banner_base();
  printf("test_ui_banner_base passed\n");
  return 0;
}
