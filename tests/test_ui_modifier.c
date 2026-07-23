/* clang-format off */
#include "ui_modifier.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  struct ui_modifier *modifier = NULL;
  struct ui_component *component = NULL;
  struct ui_dom_node *root = NULL;
  enum ui_error err;
  const char *val;

  printf("Testing invalid arguments...\n");
  if (ui_modifier_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_modifier_destroy(NULL); /* should not crash */
  if (ui_modifier_add_class(NULL, "c") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_modifier_add_style(NULL, "p", "v") != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_modifier_create(&modifier);
  if (err != UI_ERROR_NONE)
    return 1;

  if (ui_modifier_add_class(modifier, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_modifier_add_style(modifier, NULL, "v") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_modifier_add_style(modifier, "p", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_component_create(&component);
  if (ui_modifier_apply(NULL, component) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_modifier_apply(modifier, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_modifier_apply(modifier, component) != UI_ERROR_INVALID_ARGUMENT)
    return 1; /* Shadow root missing */

  ui_modifier_destroy(modifier);
  ui_component_destroy(component);
  modifier = NULL;
  component = NULL;

  printf("Testing add class and style...\n");
  err = ui_modifier_create(&modifier);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_modifier_add_class(modifier, "btn-primary");
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_modifier_add_class(modifier, "large");
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_modifier_add_style(modifier, "background-color", "red");
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_modifier_add_style(modifier, "padding", "10px");
  if (err != UI_ERROR_NONE)
    return 1;

  /* Create component */
  err = ui_component_create(&component);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  if (err != UI_ERROR_NONE)
    return 1;
  ui_dom_node_set_tag_name(root, "div");
  component->shadow_root = root;

  /* Pre-existing class/style */
  ui_dom_node_set_attribute(root, "class", "existing-class");
  ui_dom_node_set_attribute(root, "style", "margin: 5px;");

  /* Apply modifier */
  err = ui_modifier_apply(modifier, component);
  if (err != UI_ERROR_NONE) {
    printf("Failed to apply modifier\n");
    return 1;
  }

  /* Verify class */
  err = ui_dom_node_get_attribute(root, "class", &val);
  if (err != UI_ERROR_NONE ||
      strcmp(val, "existing-class btn-primary large") != 0) {
    printf("Class mismatch: %s\n", val ? val : "NULL");
    return 1;
  }

  /* Verify style */
  err = ui_dom_node_get_attribute(root, "style", &val);
  if (err != UI_ERROR_NONE ||
      strcmp(val, "margin: 5px; background-color: red; padding: 10px;") != 0) {
    printf("Style mismatch: %s\n", val ? val : "NULL");
    return 1;
  }

  /* Apply again to test append without semicolon on existing */
  ui_dom_node_set_attribute(root, "style",
                            "margin: 5px"); /* Missing trailing ; */
  err = ui_modifier_apply(modifier, component);
  if (err != UI_ERROR_NONE)
    return 1;

  ui_component_destroy(component);
  ui_modifier_destroy(modifier);

  /* Test apply empty */
  ui_modifier_create(&modifier);
  ui_component_create(&component);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  component->shadow_root = root;

  err = ui_modifier_apply(modifier, component);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_dom_node_get_attribute(root, "class", &val);
  if (err != UI_ERROR_NOT_FOUND)
    return 1;

  err = ui_dom_node_get_attribute(root, "style", &val);
  if (err != UI_ERROR_NOT_FOUND)
    return 1;

  ui_component_destroy(component);
  ui_modifier_destroy(modifier);

  /* Test apply without existing */
  ui_modifier_create(&modifier);
  ui_modifier_add_class(modifier, "test-class");
  ui_modifier_add_style(modifier, "color", "blue");

  ui_component_create(&component);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  component->shadow_root = root;

  err = ui_modifier_apply(modifier, component);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_dom_node_get_attribute(root, "class", &val);
  if (err != UI_ERROR_NONE || strcmp(val, "test-class") != 0)
    return 1;

  err = ui_dom_node_get_attribute(root, "style", &val);
  if (err != UI_ERROR_NONE || strcmp(val, "color: blue;") != 0)
    return 1;

  ui_component_destroy(component);
  ui_modifier_destroy(modifier);

  return 0;
}

static int run_oom_tests(void) {
  struct ui_modifier *modifier = NULL;
  struct ui_component *component = NULL;
  struct ui_dom_node *root = NULL;
  enum ui_error err;
  int i;

  printf("Running ui_modifier OOM tests...\n");

  /* Creation OOM */
  g_malloc_fail_countdown = 0;
  err = ui_modifier_create(&modifier);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  ui_modifier_create(&modifier);

  /* Add class OOM */
  for (i = 0; i < 2; i++) {
    g_malloc_fail_countdown = i;
    err = ui_modifier_add_class(modifier, "btn");
    g_malloc_fail_countdown = -1;
    if (err != UI_ERROR_OUT_OF_MEMORY)
      return 1;
  }

  /* Add style OOM */
  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    err = ui_modifier_add_style(modifier, "color", "red");
    g_malloc_fail_countdown = -1;
    if (err != UI_ERROR_OUT_OF_MEMORY)
      return 1;
  }

  /* Prepare for apply OOM */
  ui_modifier_add_class(modifier, "btn");
  ui_modifier_add_style(modifier, "color", "red");

  ui_component_create(&component);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_tag_name(root, "div");
  component->shadow_root = root;

  /* Apply OOM for new string allocations */
  /* new_class_str allocation */
  g_malloc_fail_countdown = 0;
  err = ui_modifier_apply(modifier, component);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  /* set_attribute class OOM */
  g_malloc_fail_countdown = 1; /* strdup inside set_attribute */
  err = ui_modifier_apply(modifier, component);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  /* new_style_str allocation */
  g_malloc_fail_countdown = 4; /* 1 for class string, 3 for class attribute set,
                                  next is style string */
  err = ui_modifier_apply(modifier, component);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  /* style set_attribute OOM */
  g_malloc_fail_countdown = 5;
  err = ui_modifier_apply(modifier, component);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  ui_component_destroy(component);
  ui_modifier_destroy(modifier);
  return 0;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All ui_modifier tests passed.\n");
  return 0;
}
