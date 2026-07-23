/* clang-format off */
#include "ui_button_group_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static enum ui_error test_button_group_base(void) {
  struct ui_button_group_base *group;
  struct ui_component *dummy_btn;
  enum ui_error err;
  const char *attr_val;
  int dummy_signal = 0;

  if (ui_button_group_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    exit(1);
  if (ui_button_group_base_set_orientation(NULL, 1) !=
      UI_ERROR_INVALID_ARGUMENT)
    exit(1);
  if (ui_button_group_base_append_button(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    exit(1);
  if (ui_button_group_base_append_button((struct ui_button_group_base *)1,
                                         NULL) != UI_ERROR_INVALID_ARGUMENT)
    exit(1);
  if (ui_button_group_base_bind_disabled(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    exit(1);
  if (ui_button_group_base_bind_text(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    exit(1);

  err = ui_button_group_base_create(&group);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_button_group_base_create failed\n");
    exit(1);
    return UI_ERROR_NONE;
  }

  err = ui_button_group_base_set_orientation(group, 1);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_button_group_base_set_orientation failed\n");
    exit(1);
  }

  err = ui_button_group_base_set_orientation(group, 0);
  if (err != UI_ERROR_NONE)
    exit(1);

  err = ui_dom_node_get_attribute(group->base.shadow_root, "data-orientation",
                                  &attr_val);
  if (err != UI_ERROR_NONE || strcmp(attr_val, "horizontal") != 0)
    exit(1);

  /* Dummy append */
  ui_component_create(&dummy_btn);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dummy_btn->shadow_root);
  ui_dom_node_set_tag_name(dummy_btn->shadow_root, "button");

  err = ui_button_group_base_append_button(group, dummy_btn);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_button_group_base_append_button failed\n");
    exit(1);
  }

  if (group->base.shadow_root->first_child != dummy_btn->shadow_root) {
    fprintf(stderr, "ui_button_group_base_append_button did not attach node\n");
    exit(1);
  }

  if (ui_button_group_base_bind_disabled(
          group, (struct ui_signal *)&dummy_signal) != UI_ERROR_NONE)
    exit(1);
  if (ui_button_group_base_bind_text(
          group, (struct ui_signal *)&dummy_signal) != UI_ERROR_NONE)
    exit(1);

  ui_component_destroy((struct ui_component *)group);
  dummy_btn->shadow_root = NULL;
  ui_component_destroy(dummy_btn);

  return UI_ERROR_NONE;
}

static int run_oom_tests(void) {
  struct ui_button_group_base *group = NULL;
  enum ui_error rc;
  int i;

  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_button_group_base_create(&group);
    if (rc == UI_ERROR_NONE) {
      ui_component_destroy((struct ui_component *)group);
      break;
    }
  }
  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  test_button_group_base();
  run_oom_tests();
  printf("test_ui_button_group_base passed\n");
  return 0;
}
