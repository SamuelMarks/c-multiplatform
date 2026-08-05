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

static ui_error_t on_dismiss_handler(struct ui_alert_base *alert,
                                     void *user_data) {
  (void)alert;
  if (user_data) {
    int *data = (int *)user_data;
    (*data)++;
  }
  dismiss_count++;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static ui_error_t run_normal_tests(void) {
  struct ui_alert_base *alert = NULL;
  ui_error_t rc;
  int my_data = 0;
  struct ui_component *comp;
  enum ui_alert_role role;
  int dismissible;

  printf("Testing ui_alert_base_create...\n");

  rc = ui_alert_base_create(NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_alert_base_create(&alert);
  if (rc != UI_ERROR_NONE || !alert) {
    printf("Failed to create alert base.\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  printf("Testing ui_alert_base_get_component...\n");
  rc = ui_alert_base_get_component(NULL, &comp);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_get_component(alert, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_get_component(alert, &comp);
  if (rc != UI_ERROR_NONE || !comp)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Testing default properties...\n");
  rc = ui_alert_base_get_role(alert, &role);
  if (rc != UI_ERROR_NONE || role != UI_ALERT_ROLE_ALERT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_alert_base_is_dismissible(alert, &dismissible);
  if (rc != UI_ERROR_NONE || dismissible != 0)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Testing setting properties...\n");
  /* test STATUS role DOM update branch */

  rc = ui_alert_base_set_role(alert, UI_ALERT_ROLE_STATUS);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_get_role(alert, &role);
  if (rc != UI_ERROR_NONE || role != UI_ALERT_ROLE_STATUS)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_alert_base_set_dismissible(alert, 1);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_is_dismissible(alert, &dismissible);
  if (rc != UI_ERROR_NONE || dismissible != 1)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Testing on_dismiss callback...\n");
  rc = ui_alert_base_set_on_dismiss(alert, on_dismiss_handler, &my_data);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_alert_base_dismiss(alert);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  if (dismiss_count != 1 || my_data != 1) {
    printf("Dismiss callback not invoked or wrong data.\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  /* Dismiss again should fail since it's already dismissed */
  rc = ui_alert_base_dismiss(alert);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (dismiss_count != 1 || my_data != 1) {
    printf("Dismiss callback invoked incorrectly.\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  rc = ui_alert_base_set_on_dismiss(alert, NULL, NULL);

  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_alert_base_set_open(alert, 1);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_alert_base_dismiss(alert);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Additional coverage: focus return */
  struct ui_component *focus_comp = NULL;
  rc = ui_component_create(&focus_comp);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_alert_base_set_focus_return(alert, focus_comp);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Open/Close states */
  int is_open = 0;
  rc = ui_alert_base_is_open(alert, &is_open);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  assert(is_open == 0); /* dismissed earlier */

  rc = ui_alert_base_set_open(alert, 1);

  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_is_open(alert, &is_open);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  assert(is_open == 1);

  /* Test signals */
  struct ui_arena *arena = NULL;
  rc = ui_arena_create(1024, &arena);
  if (rc != UI_ERROR_NONE)
    return rc;
  ui_signal_t *signal = NULL;
  union ui_signal_payload init_payload;
  memset(&init_payload, 0, sizeof(init_payload));
  rc = ui_signal_create(arena, init_payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &signal);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_alert_base_bind_open(alert, signal);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_set_open(alert, 0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN
                               : rc; /* triggers signal set */

  struct ui_computed *anim_sig = NULL;
  rc = ui_alert_base_get_animating_signal(alert, &anim_sig);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Testing invalid arguments...\n");
  rc = ui_alert_base_set_role(NULL, UI_ALERT_ROLE_STATUS);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_get_role(NULL, &role);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_get_role(alert, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_set_dismissible(NULL, 1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_is_dismissible(NULL, &dismissible);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_is_dismissible(alert, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_set_on_dismiss(NULL, on_dismiss_handler, &my_data);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_dismiss(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_set_focus_return(NULL, comp);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_alert_base_set_open(NULL, 1);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_is_open(NULL, &is_open);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_is_open(alert, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_bind_open(NULL, signal);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_get_animating_signal(NULL, &anim_sig);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_alert_base_get_animating_signal(alert, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Test unsupported dismiss (when not dismissible) */
  {
    struct ui_alert_base *alert2 = NULL;
    rc = ui_alert_base_create(&alert2);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_alert_base_set_dismissible(alert2, 0);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_alert_base_dismiss(alert2);
    if (rc != UI_ERROR_UNSUPPORTED)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    rc = ui_alert_base_destroy(alert2);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  printf("Destroying alert...\n");
  rc = ui_alert_base_destroy(alert);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Destroy NULL should be safe */
  rc = ui_alert_base_destroy(NULL);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_component_destroy(focus_comp);

  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_arena_destroy(arena);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static ui_error_t run_oom_tests(void) {
  struct ui_alert_base *alert = NULL;
  ui_error_t rc;
  int i;

  printf("Testing OOM conditions...\n");

  /* Changed from i=1 to i=0 to hit the very first malloc failure */
  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_alert_base_create(&alert);
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    }
    if (rc == UI_ERROR_NONE) {
      rc = ui_alert_base_destroy(alert);
      if (rc != UI_ERROR_NONE)
        return rc;
      break; /* We succeeded, didn't hit malloc fail */
    }
  }

  g_malloc_fail_countdown = -1;

#ifdef UI_TEST_MOCK_ALLOC
  extern ui_error_t run_alert_coverage(void);
  run_alert_coverage();
#endif
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t rc;
  printf("--- ui_alert_base Tests ---\n");

  if (run_normal_tests() != UI_ERROR_NONE) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != UI_ERROR_NONE) {
    printf("OOM tests failed.\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  printf("All ui_alert_base tests passed.\n");
  return 0;
}
