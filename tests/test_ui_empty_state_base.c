/* clang-format off */
#include "ui_empty_state_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static void test_empty_state_base(void) {
  struct ui_empty_state_base *state = NULL;
  ui_error_t err;
  const char *attr_val;
  struct ui_signal *dummy_signal = (struct ui_signal *)0x1234;

  /* Null parameter */
  err = ui_empty_state_base_create(NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_empty_state_base_create NULL failed\n");
    exit(1);
  }

  err = ui_empty_state_base_create(&state);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_empty_state_base_create failed\n");
    exit(1);
  }

  /* Null parameters */
  err = ui_empty_state_base_set_title(NULL, "No Results");
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_empty_state_base_set_title NULL state failed\n");
    exit(1);
  }

  err = ui_empty_state_base_set_title(state, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_empty_state_base_set_title NULL text failed\n");
    exit(1);
  }

  err =
      ui_empty_state_base_set_description(NULL, "Try adjusting your filters.");
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_empty_state_base_set_description NULL state failed\n");
    exit(1);
  }

  err = ui_empty_state_base_set_description(state, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_empty_state_base_set_description NULL text failed\n");
    exit(1);
  }

  err = ui_empty_state_base_bind_data(NULL, dummy_signal);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_empty_state_base_bind_data NULL widget failed\n");
    exit(1);
  }

  err = ui_empty_state_base_set_title(state, "No Results");
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_empty_state_base_set_title failed\n");
    exit(1);
  }

  err =
      ui_empty_state_base_set_description(state, "Try adjusting your filters.");
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_empty_state_base_set_description failed\n");
    exit(1);
  }

  err = ui_empty_state_base_bind_data(state, dummy_signal);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_empty_state_base_bind_data failed\n");
    exit(1);
  }

  err = ui_dom_node_get_attribute(state->base.shadow_root, "data-title",
                                  &attr_val);
  if (err != UI_ERROR_NONE || strcmp(attr_val, "No Results") != 0) {
    fprintf(stderr,
            "ui_empty_state_base_set_title did not set attribute correctly\n");
    exit(1);
  }

  err = ui_dom_node_get_attribute(state->base.shadow_root, "data-description",
                                  &attr_val);
  if (err != UI_ERROR_NONE ||
      strcmp(attr_val, "Try adjusting your filters.") != 0) {
    fprintf(stderr, "ui_empty_state_base_set_description did not set attribute "
                    "correctly\n");
    exit(1);
  }

  (void)ui_component_destroy((struct ui_component *)state);
}

static void test_oom(void) {
  extern int g_malloc_fail_countdown;
  struct ui_empty_state_base *state;
  int i;
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    if (ui_empty_state_base_create(&state) == UI_ERROR_NONE) {
      (void)ui_component_destroy((struct ui_component *)state);
    }
    g_malloc_fail_countdown = -1;
  }
}

int main(void) {
  test_empty_state_base();
  test_oom();
  printf("test_ui_empty_state_base passed\n");
  return 0;
}
