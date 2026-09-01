/* clang-format off */
#include "ui_button_group_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_button_group_mock_fail;

static ui_error_t test_button_group_base(void) {
  struct ui_button_group_base *group;
  struct ui_component *dummy_btn;
  ui_error_t err;
  ui_error_t rc = UI_ERROR_NONE;
  const char *attr_val;
  int dummy_signal = 0;

  g_button_group_mock_fail = 1;
  rc = ui_button_group_base_create(&group);
  if (rc != UI_ERROR_UNKNOWN)
    return UI_ERROR_UNKNOWN;
  g_button_group_mock_fail = 0;

  rc = ui_button_group_base_create(NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_button_group_base_set_orientation(NULL, 1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_button_group_base_append_button(NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_button_group_base_append_button((struct ui_button_group_base *)1,
                                          NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_button_group_base_bind_disabled(NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_button_group_base_bind_text(NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_button_group_base_create(&group);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_button_group_base_create failed\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    return UI_ERROR_NONE;
  }

  rc = ui_button_group_base_set_orientation(group, 1);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_button_group_base_set_orientation failed\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  rc = ui_button_group_base_set_orientation(group, 0);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_dom_node_get_attribute(group->base.shadow_root, "data-orientation",
                                 &attr_val);
  if (rc != UI_ERROR_NONE || strcmp(attr_val, "horizontal") != 0)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Dummy append */
  rc = ui_component_create(&dummy_btn);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dummy_btn->shadow_root);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_tag_name(dummy_btn->shadow_root, "button");
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_button_group_base_append_button(group, dummy_btn);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "ui_button_group_base_append_button failed\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  if (group->base.shadow_root->first_child != dummy_btn->shadow_root) {
    fprintf(stderr, "ui_button_group_base_append_button did not attach node\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  rc = ui_button_group_base_bind_disabled(group,
                                          (struct ui_signal *)&dummy_signal);

  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_button_group_base_bind_text(group, (struct ui_signal *)&dummy_signal);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_component_destroy((struct ui_component *)group);
  if (rc != UI_ERROR_NONE)
    return rc;
  dummy_btn->shadow_root = NULL;
  rc = ui_component_destroy(dummy_btn);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

static ui_error_t
run_oom_test_create_step(int i, struct ui_button_group_base **out_group,
                         int *out_break) {
  ui_error_t rc;
  g_malloc_fail_countdown = i;
  rc = ui_button_group_base_create(out_group);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }
  if (rc == UI_ERROR_NONE) {
    g_malloc_fail_countdown = -1;
    {
      ui_error_t d_rc = ui_component_destroy((struct ui_component *)*out_group);
      if (d_rc != UI_ERROR_NONE)
        return d_rc;
    }
    *out_break = 1;
    return UI_ERROR_NONE;
  }
  g_malloc_fail_countdown = -1;
  return UI_ERROR_NONE;
}

static ui_error_t run_oom_tests(void) {
  struct ui_button_group_base *group = NULL;
  ui_error_t rc;
  int i;

  for (i = 0; i < 5; i++) {
    int do_break = 0;
    rc = run_oom_test_create_step(i, &group, &do_break);
    if (rc != UI_ERROR_NONE && rc != UI_ERROR_UNKNOWN)
      return rc;
    if (do_break)
      break;
  }
  g_malloc_fail_countdown = -1;
  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t rc;
  rc = test_button_group_base();
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = run_oom_tests();
  if (rc != UI_ERROR_NONE)
    return 1;
  printf("test_ui_button_group_base passed\n");
  return 0;
}
