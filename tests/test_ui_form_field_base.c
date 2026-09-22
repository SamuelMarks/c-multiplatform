/* clang-format off */
#include "../include/ui_form_field_base.h"
#include "../include/ui_form_control.h"
#include "../include/ui_reactor.h"
#include "../include/ui_signal.h"
#include "../include/ui_arena.h"
#include <stdio.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

struct test_field_effect_data {
  struct ui_form_field_base *field;
  ui_signal_t *errors_signal;
};

struct test_field_mirror {
  struct ui_component *component;
  char *label_text;
  char *hint_text;
  char *error_text;
  struct ui_dom_node *root_node;
  struct ui_dom_node *wrapper_node;
  struct ui_dom_node *label_node;
  struct ui_dom_node *control_container;
  struct ui_dom_node *prefix_container;
  struct ui_dom_node *suffix_container;
  struct ui_dom_node *subscript_node;
  struct ui_dom_node *hint_node;
  struct ui_dom_node *error_node;
  struct ui_component *control;
  struct ui_component *prefix;
  struct ui_component *suffix;
  int is_focused;
  int has_value;
  struct ui_arena *binding_arena;
  struct ui_effect *binding_effect;
  struct test_field_effect_data *binding_data;
  struct ui_signal *data_signal;
};

static ui_error_t run_basic_tests(void) {
  struct ui_form_field_base *field = NULL;
  struct ui_component *comp = NULL;
  struct ui_component *prefix = NULL;
  struct ui_component *suffix = NULL;
  struct ui_component *control_comp = NULL;
  ui_error_t rc;

  /* Invalid args */
  rc = ui_form_field_base_create(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_form_field_base_create(&field);
  assert(rc == UI_ERROR_NONE);

  /* Get component */
  rc = ui_form_field_base_get_component(NULL, &comp);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_get_component(field, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_get_component(field, &comp);
  assert(rc == UI_ERROR_NONE);

  /* Set label */
  rc = ui_form_field_base_set_label(NULL, "Label");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_set_label(field, "Label");
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_field_base_set_label(field, NULL);
  assert(rc == UI_ERROR_NONE);

  /* Set hint */
  rc = ui_form_field_base_set_hint(NULL, "Hint");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_set_hint(field, "Hint");
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_field_base_set_hint(field, NULL);
  assert(rc == UI_ERROR_NONE);

  /* Set error */
  rc = ui_form_field_base_set_error(NULL, "Error");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_set_error(field, "");
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_field_base_set_error(field, "Error");
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_field_base_set_error(field, NULL);
  assert(rc == UI_ERROR_NONE);

  /* Set components */
  {
    ui_error_t rc_cleanup = ui_component_create(&prefix);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_create(&suffix);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_create(&control_comp);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }

  rc = ui_form_field_base_set_prefix(NULL, prefix);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_set_prefix(field, prefix);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_field_base_set_prefix(field, NULL);
  assert(rc == UI_ERROR_NONE);

  rc = ui_form_field_base_set_suffix(NULL, suffix);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_set_suffix(field, suffix);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_field_base_set_suffix(field, NULL);
  assert(rc == UI_ERROR_NONE);

  rc = ui_form_field_base_set_control(NULL, control_comp);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_set_control(field, control_comp);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_field_base_set_control(field, NULL);
  assert(rc == UI_ERROR_NONE);

  /* States */
  rc = ui_form_field_base_set_focused(NULL, 1);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_set_focused(field, 1);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_field_base_set_focused(field, 0);
  assert(rc == UI_ERROR_NONE);

  rc = ui_form_field_base_set_has_value(NULL, 1);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_set_has_value(field, 1);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_field_base_set_has_value(field, 0);
  assert(rc == UI_ERROR_NONE);

  rc = ui_form_field_base_bind_data(NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_bind_data(field, NULL);
  assert(rc == UI_ERROR_NONE);

  {
    ui_error_t rc_cleanup = ui_form_field_base_destroy(field);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    struct ui_form_field_base *f_hint = NULL;
    rc = ui_form_field_base_create(&f_hint);
    assert(rc == UI_ERROR_NONE);
    rc = ui_form_field_base_set_hint(f_hint, "retained hint");
    assert(rc == UI_ERROR_NONE);
    rc = ui_form_field_base_destroy(f_hint);
    assert(rc == UI_ERROR_NONE);
  }
  {
    struct ui_form_field_base *f_err = NULL;
    rc = ui_form_field_base_create(&f_err);
    assert(rc == UI_ERROR_NONE);
    rc = ui_form_field_base_set_error(f_err, "retained error");
    assert(rc == UI_ERROR_NONE);
    rc = ui_form_field_base_destroy(f_err);
    assert(rc == UI_ERROR_NONE);
  }
  {
    struct ui_form_field_base *f_nocomp = NULL;
    struct test_field_mirror *tf = NULL;
    struct ui_component *saved_comp = NULL;
    rc = ui_form_field_base_create(&f_nocomp);
    assert(rc == UI_ERROR_NONE);
    tf = (struct test_field_mirror *)f_nocomp;
    saved_comp = tf->component;
    tf->component = NULL;
    rc = ui_form_field_base_destroy(f_nocomp);
    assert(rc == UI_ERROR_NONE);
    rc = ui_component_destroy(saved_comp);
    assert(rc == UI_ERROR_NONE);
  }
  {
    ui_error_t rc_cleanup = ui_form_field_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(prefix);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(suffix);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(control_comp);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t run_binding_tests(void) {
  struct ui_form_field_base *field = NULL;
  struct ui_form_control *control = NULL;
  struct ui_reactor *reactor = NULL;
  struct ui_arena *arena = NULL;
  union ui_signal_payload val = {0};
  ui_error_t rc;

  {
    ui_error_t rc_cleanup = ui_arena_create(1024, &arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_reactor_create(&reactor);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_form_control_create(arena, val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                               UI_SIGNAL_MODE_SINGLE_THREADED, &control);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_form_field_base_create(&field);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }

  rc = ui_form_field_base_bind_form_control(NULL, control, reactor);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_bind_form_control(field, NULL, reactor);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_form_field_base_bind_form_control(field, control, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_form_field_base_bind_form_control(field, control, reactor);
  assert(rc == UI_ERROR_NONE);

  /* Poll initially so effect subscribes to control->errors_signal */
  {
    ui_error_t rc_cleanup = ui_reactor_poll(reactor, 100);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }

  /* Trigger error signal */
  {
    ui_error_t rc_cleanup = ui_form_control_set_error(control, "Test error");
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  g_malloc_fail_countdown =
      0; /* fail ui_signal_get or ui_form_field_base_set_error inside effect */
  {
    ui_error_t rc_cleanup = ui_reactor_poll(reactor, 100);
    assert(rc_cleanup == UI_ERROR_OUT_OF_MEMORY);
  }

  g_malloc_fail_countdown = -1;
  {
    ui_error_t rc_cleanup = ui_form_control_set_error(control, "Test error 2");
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_reactor_poll(reactor, 100);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }

  /* Second bind does nothing but returns UI_ERROR_NONE */
  rc = ui_form_field_base_bind_form_control(field, control, reactor);
  assert(rc == UI_ERROR_NONE);

  /* Cover error_effect_runner rc != UI_ERROR_NONE */
  {
    struct test_field_mirror *tf = (struct test_field_mirror *)field;
    ui_signal_t *saved_sig = tf->binding_data->errors_signal;
    tf->binding_data->errors_signal = NULL;
    ui_form_control_set_error(control, "Trigger runner error");
    ui_reactor_poll(reactor, 100);
    tf->binding_data->errors_signal = saved_sig;
  }

  {
    ui_error_t rc_cleanup = ui_form_field_base_destroy(field);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_reactor_destroy(reactor);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t run_oom_tests_form_field_base(void) {
  struct ui_form_field_base *field;
  struct ui_form_control *control;
  struct ui_reactor *reactor;
  struct ui_arena *arena;
  union ui_signal_payload val = {0};
  int i;
  ui_error_t rc;

  for (i = 0; i < 2000; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_form_field_base_create(&field);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_form_field_base_destroy(field);
        if (rc_cleanup != UI_ERROR_NONE) {
          return rc_cleanup;
        }
      }
      break;
    }
  }

  {
    ui_error_t rc_cleanup = ui_form_field_base_create(&field);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }

  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_form_field_base_set_label(field, "Label");
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE)
      break;
  }
  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_form_field_base_set_hint(field, "Hint");
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE)
      break;
  }
  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_form_field_base_set_error(field, "Error");
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE)
      break;
  }
  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_form_field_base_set_focused(field, 1);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE)
      break;
  }
  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_form_field_base_set_has_value(field, 1);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE)
      break;
  }

  {
    ui_error_t rc_cleanup = ui_arena_create(1024, &arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_reactor_create(&reactor);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_form_control_create(arena, val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                               UI_SIGNAL_MODE_SINGLE_THREADED, &control);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }

  for (i = 0; i < 10; i++) {
    ui_error_t bind_rc;
    g_malloc_fail_countdown = i;
    bind_rc = ui_form_field_base_bind_form_control(field, control, reactor);
    g_malloc_fail_countdown = -1;

    if (bind_rc == UI_ERROR_NONE)
      break;
  }

  {
    ui_error_t rc_cleanup = ui_form_field_base_destroy(field);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_form_control_destroy(control);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_reactor_destroy(reactor);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  return UI_ERROR_NONE;
}

int main(void) {
  assert(run_basic_tests() == UI_ERROR_NONE);
  assert(run_binding_tests() == UI_ERROR_NONE);
  assert(run_oom_tests_form_field_base() == UI_ERROR_NONE);
  printf("All ui_form_field_base tests passed.\n");
  return 0;
}
