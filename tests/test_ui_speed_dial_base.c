/* clang-format off */
#include <assert.h>
#include <stddef.h>

#include "ui_speed_dial_base.h"
#include "ui_error.h"
/* clang-format on */

struct ui_component {
  int id;
};

/* Dummy struct to satisfy pointer */
struct ui_fab_base {
  int id;
};

static enum ui_error test_speed_dial_init(void) {
  struct ui_speed_dial_base sd;
  struct ui_component comp;
  enum ui_error err;

  err = ui_speed_dial_base_init(NULL, &comp);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_speed_dial_base_init(&sd, NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_speed_dial_base_init(&sd, &comp);
  assert(err == UI_ERROR_NONE);
  assert(sd.component == &comp);
  assert(sd.is_open == 0);
  assert(sd.actions == NULL);
  return UI_ERROR_NONE;
}

static enum ui_error test_speed_dial_operations(void) {
  struct ui_speed_dial_base sd;
  struct ui_component comp;
  struct ui_fab_base fab1 = {1};
  struct ui_fab_base fab2 = {2};
  enum ui_error err;

  ui_speed_dial_base_init(&sd, &comp);

  err = ui_speed_dial_base_add_action(NULL, 100, &fab1);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_speed_dial_base_add_action(&sd, 100, NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_speed_dial_base_add_action(&sd, 100, &fab1);
  assert(err == UI_ERROR_NONE);
  assert(sd.actions != NULL);
  assert(sd.actions->id == 100);

  err = ui_speed_dial_base_add_action(&sd, 200, &fab2);
  assert(err == UI_ERROR_NONE);
  assert(sd.actions->id == 200);

  assert(sd.is_open == 0);

  err = ui_speed_dial_base_toggle(NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  ui_speed_dial_base_toggle(&sd);
  assert(sd.is_open == 1);
  ui_speed_dial_base_toggle(&sd);
  assert(sd.is_open == 0);

  err = ui_speed_dial_base_cleanup(NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_speed_dial_base_cleanup(&sd);
  assert(err == UI_ERROR_NONE);
  assert(sd.actions == NULL);

  {
    extern int g_malloc_fail_countdown;
    g_malloc_fail_countdown = 0;
    err = ui_speed_dial_base_add_action(&sd, 300, &fab1);
    assert(err == UI_ERROR_OUT_OF_MEMORY);
    g_malloc_fail_countdown = -1;
  }

  return UI_ERROR_NONE;
}

static enum ui_error test_speed_dial_bindings(void) {
  struct ui_speed_dial_base sd;
  struct ui_component comp;
  enum ui_error err;

  ui_speed_dial_base_init(&sd, &comp);

  err = ui_speed_dial_base_bind_disabled(NULL, NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_speed_dial_base_bind_disabled(&sd, NULL);
  assert(err == UI_ERROR_NONE);

  err = ui_speed_dial_base_bind_text(NULL, NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_speed_dial_base_bind_text(&sd, NULL);
  assert(err == UI_ERROR_NONE);

  return UI_ERROR_NONE;
}

int main(void) {
  test_speed_dial_init();
  test_speed_dial_operations();
  test_speed_dial_bindings();
  return 0;
}
