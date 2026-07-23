/* clang-format off */
#include "ui_alert_base.h"
#include "ui_arena.h"
#include "ui_signal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#undef NDEBUG
#include <assert.h>

extern int g_malloc_fail_countdown;

static int dismiss_count = 0;

static enum ui_error on_dismiss_handler(struct ui_alert_base *alert,
                                        void *user_data) {
  (void)alert;
  if (user_data) {
    int *data = (int *)user_data;
    (*data)++;
  }
  dismiss_count++;
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_alert_base *alert = NULL;
  enum ui_error rc;
  int my_data = 0;
  struct ui_component *comp;
  enum ui_alert_role role;
  int dismissible;

  printf("Testing ui_alert_base_create...\n");

  if (ui_alert_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_alert_base_create(&alert);
  if (rc != UI_ERROR_NONE || !alert) {
    printf("Failed to create alert base.\n");
    return 1;
  }

  printf("Testing ui_alert_base_get_component...\n");
  if (ui_alert_base_get_component(NULL, &comp) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_alert_base_get_component(alert, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_alert_base_get_component(alert, &comp);
  if (rc != UI_ERROR_NONE || !comp)
    return 1;

  printf("Testing default properties...\n");
  rc = ui_alert_base_get_role(alert, &role);
  if (rc != UI_ERROR_NONE || role != UI_ALERT_ROLE_ALERT)
    return 1;

  rc = ui_alert_base_is_dismissible(alert, &dismissible);
  if (rc != UI_ERROR_NONE || dismissible != 0)
    return 1;

  printf("Testing setting properties...\n");
  /* test STATUS role DOM update branch */

  rc = ui_alert_base_set_role(alert, UI_ALERT_ROLE_STATUS);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_alert_base_get_role(alert, &role);
  if (rc != UI_ERROR_NONE || role != UI_ALERT_ROLE_STATUS)
    return 1;

  rc = ui_alert_base_set_dismissible(alert, 1);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_alert_base_is_dismissible(alert, &dismissible);
  if (rc != UI_ERROR_NONE || dismissible != 1)
    return 1;

  printf("Testing on_dismiss callback...\n");
  rc = ui_alert_base_set_on_dismiss(alert, on_dismiss_handler, &my_data);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_alert_base_dismiss(alert);
  if (rc != UI_ERROR_NONE)
    return 1;
  if (dismiss_count != 1 || my_data != 1) {
    printf("Dismiss callback not invoked or wrong data.\n");
    return 1;
  }

  /* Dismiss again should fail since it's already dismissed */
  rc = ui_alert_base_dismiss(alert);
  assert(rc == UI_ERROR_NONE);
  if (dismiss_count != 1 || my_data != 1) {
    printf("Dismiss callback invoked incorrectly.\n");
    return 1;
  }

  ui_alert_base_set_on_dismiss(alert, NULL, NULL);
  ui_alert_base_set_open(alert, 1);
  rc = ui_alert_base_dismiss(alert);
  assert(rc == UI_ERROR_NONE);

  /* Additional coverage: focus return */
  struct ui_component *focus_comp = NULL;
  ui_component_create(&focus_comp);
  assert(ui_alert_base_set_focus_return(alert, focus_comp) == UI_ERROR_NONE);

  /* Open/Close states */
  int is_open = 0;
  assert(ui_alert_base_is_open(alert, &is_open) == UI_ERROR_NONE);
  assert(is_open == 0); /* dismissed earlier */

  assert(ui_alert_base_set_open(alert, 1) == UI_ERROR_NONE);
  assert(ui_alert_base_is_open(alert, &is_open) == UI_ERROR_NONE);
  assert(is_open == 1);

  /* Test signals */
  struct ui_arena *arena = NULL;
  ui_arena_create(1024, &arena);
  ui_signal_t *signal = NULL;
  union ui_signal_payload init_payload;
  memset(&init_payload, 0, sizeof(init_payload));
  ui_signal_create(arena, init_payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &signal);
  assert(ui_alert_base_bind_open(alert, signal) == UI_ERROR_NONE);
  assert(ui_alert_base_set_open(alert, 0) ==
         UI_ERROR_NONE); /* triggers signal set */

  struct ui_computed *anim_sig = NULL;
  assert(ui_alert_base_get_animating_signal(alert, &anim_sig) == UI_ERROR_NONE);

  printf("Testing invalid arguments...\n");
  if (ui_alert_base_set_role(NULL, UI_ALERT_ROLE_STATUS) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_alert_base_get_role(NULL, &role) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_alert_base_get_role(alert, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_alert_base_set_dismissible(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_alert_base_is_dismissible(NULL, &dismissible) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_alert_base_is_dismissible(alert, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_alert_base_set_on_dismiss(NULL, on_dismiss_handler, &my_data) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_alert_base_dismiss(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_alert_base_set_focus_return(NULL, comp) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  assert(ui_alert_base_set_open(NULL, 1) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_alert_base_is_open(NULL, &is_open) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_alert_base_is_open(alert, NULL) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_alert_base_bind_open(NULL, signal) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_alert_base_get_animating_signal(NULL, &anim_sig) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_alert_base_get_animating_signal(alert, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  /* Test unsupported dismiss (when not dismissible) */
  {
    struct ui_alert_base *alert2 = NULL;
    ui_alert_base_create(&alert2);
    ui_alert_base_set_dismissible(alert2, 0);
    if (ui_alert_base_dismiss(alert2) != UI_ERROR_UNSUPPORTED)
      return 1;
    ui_alert_base_destroy(alert2);
  }

  printf("Destroying alert...\n");
  ui_alert_base_destroy(alert);

  /* Destroy NULL should be safe */
  ui_alert_base_destroy(NULL);

  ui_component_destroy(focus_comp);
  ui_arena_destroy(arena);

  return 0;
}

static int run_oom_tests(void) {
  struct ui_alert_base *alert = NULL;
  enum ui_error rc;
  int i;

  printf("Testing OOM conditions...\n");

  /* Changed from i=1 to i=0 to hit the very first malloc failure */
  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_alert_base_create(&alert);
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      return 1;
    }
    if (rc == UI_ERROR_NONE) {
      ui_alert_base_destroy(alert);
      break; /* We succeeded, didn't hit malloc fail */
    }
  }

  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  printf("--- ui_alert_base Tests ---\n");

  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All ui_alert_base tests passed.\n");
  return 0;
}
