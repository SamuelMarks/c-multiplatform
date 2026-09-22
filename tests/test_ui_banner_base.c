/* clang-format off */
#include "ui_banner_base.h"
#include "ui_component.h"
#include "ui_signal.h"
#include "ui_arena.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

struct ui_banner_base {
  struct ui_component *base;
  int is_open;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

extern int g_malloc_fail_countdown;

#ifdef UI_TEST_MOCK_ALLOC
static ui_error_t run_mock_tests(void) {
  struct ui_banner_base *banner = NULL;
  union ui_signal_payload p;
  struct ui_signal *sig = NULL;
  ui_error_t rc;
  extern int g_banner_mock_fail;

  g_banner_mock_fail = 10;
  rc = ui_banner_base_create(&banner);
  if (rc != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_banner_mock_fail = 0;

  g_banner_mock_fail = 11;
  rc = ui_banner_base_create(&banner);
  if (rc != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_banner_mock_fail = 0;

  g_banner_mock_fail = 12;
  rc = ui_banner_base_create(&banner);
  if (rc != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_banner_mock_fail = 0;

  g_banner_mock_fail = 13;
  rc = ui_banner_base_create(&banner);
  if (rc != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_banner_mock_fail = 0;

  rc = ui_banner_base_create(&banner);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  g_banner_mock_fail = 11;
  rc = ui_banner_base_set_text(banner, "text");
  if (rc != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_banner_mock_fail = 0;

  g_banner_mock_fail = 1;
  rc = ui_banner_base_set_text(banner, "text");
  if (rc != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_banner_mock_fail = 0;

  g_banner_mock_fail = 2;
  rc = ui_banner_base_set_open(banner, 1);
  if (rc != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_banner_mock_fail = 0;

  g_banner_mock_fail = 3;
  rc = ui_banner_base_set_open(banner, 0);
  if (rc != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_banner_mock_fail = 0;

  memset(&p, 0, sizeof(p));
  p.bool_val = 1;
  rc = ui_signal_create(NULL, p, UI_SIGNAL_TYPE_BOOL, NULL, NULL, 0, &sig);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  rc = ui_banner_base_bind_open(banner, sig);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  g_banner_mock_fail = 4;
  rc = ui_banner_base_set_open(banner, 1);
  if (rc != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_banner_mock_fail = 0;

  rc = ui_signal_destroy(sig);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  rc = ui_banner_base_destroy(banner);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}
#endif

static ui_error_t test_banner_base(void) {
  struct ui_banner_base *banner = NULL;
  struct ui_component *base_comp = NULL;
  ui_error_t err;
  int is_open = 0;
  struct ui_computed *anim_sig = NULL;
  int i;
  struct ui_arena *arena = NULL;
  ui_signal_t *signal = NULL;
  union ui_signal_payload init_payload;

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
  err = ui_arena_create(1024, &arena);
  assert(err == UI_ERROR_NONE);
  memset(&init_payload, 0, sizeof(init_payload));
  err = ui_signal_create(arena, init_payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED, &signal);
  assert(err == UI_ERROR_NONE);

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
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  {
    ui_error_t rc_cleanup = ui_signal_destroy(signal);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  /* OOM Loops */
  for (i = 0; i < 15; i++) {
    struct ui_banner_base *test_banner = NULL;
    g_malloc_fail_countdown = i;
    err = ui_banner_base_create(&test_banner);
    if (err == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_banner_base_destroy(test_banner);
        assert(rc_cleanup == UI_ERROR_NONE);
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
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  /* Test destroying banner with NULL base */
  err = ui_banner_base_create(&banner);
  assert(err == UI_ERROR_NONE);
  err = ui_component_destroy(banner->base);
  assert(err == UI_ERROR_NONE);
  banner->base = NULL;
  err = ui_banner_base_destroy(banner);
  assert(err == UI_ERROR_NONE);

  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  rc = run_mock_tests();
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
#endif

  rc = test_banner_base();
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  printf("test_ui_banner_base passed\n");
  return 0;
}
