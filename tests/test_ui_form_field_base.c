/* clang-format off */
#include "ui_form_field_base.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  struct ui_form_field_base *field = NULL;
  enum ui_error rc;

  printf("Testing invalid arguments...\n");
  if (ui_form_field_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_form_field_base_destroy(NULL);

  /* removed */

  if (ui_form_field_base_set_label(NULL, "a") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_form_field_base_set_hint(NULL, "a") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_form_field_base_set_error(NULL, "a") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_form_field_base_set_focused(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_form_field_base_set_has_value(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_form_field_base_bind_form_control(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_form_field_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_form_field_base_destroy(NULL); /* Should not crash */
  if (ui_form_field_base_set_label(NULL, "a") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_form_field_base_set_hint(NULL, "a") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_form_field_base_set_error(NULL, "a") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_form_field_base_set_prefix(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_form_field_base_set_suffix(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_form_field_base_set_control(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_form_field_base_set_focused(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_form_field_base_set_has_value(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    struct ui_component *tmp_comp;
    if (ui_form_field_base_get_component(NULL, &tmp_comp) == UI_ERROR_NONE)
      return 1;
  }

  rc = ui_form_field_base_create(&field);

  if (rc != UI_ERROR_NONE || field == NULL) {
    printf("Failed to create form field.\n");
    return 1;
  }

  {
    struct ui_component *tmp_comp;
    if (ui_form_field_base_get_component(field, &tmp_comp) != UI_ERROR_NONE ||
        tmp_comp == NULL)
      return 1;
  }

  /* Test states */
  if (ui_form_field_base_set_label(field, "Username") != UI_ERROR_NONE)
    return 1;
  if (ui_form_field_base_set_label(field, NULL) != UI_ERROR_NONE)
    return 1;

  if (ui_form_field_base_set_hint(field, "Hint text") != UI_ERROR_NONE)
    return 1;
  if (ui_form_field_base_set_hint(field, NULL) != UI_ERROR_NONE)
    return 1;

  if (ui_form_field_base_set_error(field, "Error text") != UI_ERROR_NONE)
    return 1;
  if (ui_form_field_base_set_error(field, NULL) != UI_ERROR_NONE)
    return 1;

  if (ui_form_field_base_set_focused(field, 1) != UI_ERROR_NONE)
    return 1;
  if (ui_form_field_base_set_focused(field, 0) != UI_ERROR_NONE)
    return 1;

  if (ui_form_field_base_set_has_value(field, 1) != UI_ERROR_NONE)
    return 1;
  if (ui_form_field_base_set_has_value(field, 0) != UI_ERROR_NONE)
    return 1;

  ui_form_field_base_set_label(field, "Label");
  ui_form_field_base_set_hint(field, "Hint");
  ui_form_field_base_set_error(field, "Error");
  ui_form_field_base_destroy(field);
  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_form_field_base *field = NULL;
  enum ui_error rc;
  int i;

  printf("Testing OOM...\n");

  /* Test component creation OOM */
  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_form_field_base_create(&field);

    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      continue;
    } else if (rc == UI_ERROR_NONE) {

      {
        struct ui_form_control *control;
        union ui_signal_payload dummy = {0};
        struct ui_arena *form_arena;
        struct ui_reactor *reactor;
        int j;
        ui_arena_create(1024, &form_arena);
        ui_reactor_create(&reactor);
        ui_form_control_create(form_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                               NULL, UI_SIGNAL_MODE_SINGLE_THREADED, &control);

        for (j = 0; j < 50; j++) {
          g_malloc_fail_countdown = j;
          rc = ui_form_field_base_bind_form_control(field, control, reactor);
          g_malloc_fail_countdown = -1;
          if (rc == UI_ERROR_NONE) {
            break;
          }
        }

        /* Hit the error branch by passing a NULL reactor, since
         * ui_effect_create requires it */
        ui_form_field_base_bind_form_control(field, control, NULL);

        ui_form_control_destroy(control);
        ui_reactor_destroy(reactor);
        ui_arena_destroy(form_arena);
      }

      ui_form_field_base_set_label(field, "Label");
      ui_form_field_base_set_hint(field, "Hint");
      ui_form_field_base_set_error(field, "Error");
      ui_form_field_base_destroy(field);
      break;
    } else {
      return 1;
    }
  }

  /* Set text OOM */
  ui_form_field_base_create(&field);

  g_malloc_fail_countdown = 0;
  rc = ui_form_field_base_set_label(field, "A");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  g_malloc_fail_countdown = 0;
  rc = ui_form_field_base_set_hint(field, "H");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  g_malloc_fail_countdown = 0;
  rc = ui_form_field_base_set_error(field, "E");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  ui_form_field_base_set_label(field, "Label");
  ui_form_field_base_set_hint(field, "Hint");
  ui_form_field_base_set_error(field, "Error");
  ui_form_field_base_destroy(field);
#endif
  return 0;
}

static int run_extra_field(void);

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_extra_field() != 0)
    return 1;

  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All ui_form_field_base tests passed.\n");
  return 0;
}

static int run_extra_field(void) {
  struct ui_form_field_base *field;
  struct ui_component *comp;
  struct ui_signal *dummy_sig = (struct ui_signal *)1;

  ui_form_field_base_create(&field);

  ui_component_create(&comp);

  {
    struct ui_form_control *control;
    union ui_signal_payload dummy = {0};
    struct ui_arena *form_arena;
    struct ui_reactor *reactor;
    ui_arena_create(1024, &form_arena);
    ui_reactor_create(&reactor);
    ui_form_control_create(form_arena, dummy, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                           UI_SIGNAL_MODE_SINGLE_THREADED, &control);

    ui_form_field_base_bind_form_control(field, control, reactor);
    ui_reactor_poll(reactor, 10); /* Process initial effect scheduling */
    ui_form_field_base_bind_form_control(field, control, reactor);

    ui_form_control_set_error(control, "Invalid value");
    ui_reactor_poll(reactor, 10);

    ui_form_control_destroy(control);
    ui_reactor_destroy(reactor);
    ui_arena_destroy(form_arena);
  }

  ui_form_field_base_set_label(field, "Label 1");
  ui_form_field_base_set_hint(field, "Hint 1");
  ui_form_field_base_set_error(field, "Error 1");

  ui_form_field_base_set_label(field, NULL);
  ui_form_field_base_set_hint(field, NULL);
  ui_form_field_base_set_error(field, NULL);

  ui_form_field_base_set_prefix(field, NULL);
  ui_form_field_base_set_suffix(field, NULL);
  ui_form_field_base_set_control(field, NULL);

  ui_form_field_base_set_prefix(field, comp);
  ui_form_field_base_set_suffix(field, comp);
  ui_form_field_base_set_control(field, comp);

  ui_form_field_base_bind_data(NULL, dummy_sig);
  ui_form_field_base_bind_data(field, dummy_sig);

  /* form control bind is tricky because it takes ui_form_control and reactor.
   * We can test NULLs. */
  if (ui_form_field_base_bind_form_control(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_form_field_base_set_label(field, "Label");
  ui_form_field_base_set_hint(field, "Hint");
  ui_form_field_base_set_error(field, "Error");
  ui_form_field_base_destroy(field);
  ui_component_destroy(comp);

  return 0;
}
