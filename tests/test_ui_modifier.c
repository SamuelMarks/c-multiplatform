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
  ui_error_t err;
  const char *val;

  printf("Testing invalid arguments...\n");
  if (ui_modifier_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    ui_error_t _ign = ui_modifier_destroy(NULL);
    (void)_ign;
  } /* should not crash */
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

  {
    ui_error_t _ign = ui_component_create(&component);
    (void)_ign;
  }
  if (ui_modifier_apply(NULL, component) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_modifier_apply(modifier, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_modifier_apply(modifier, component) != UI_ERROR_INVALID_ARGUMENT)
    return 1; /* Shadow root missing */

  {
    ui_error_t _ign = ui_modifier_destroy(modifier);
    (void)_ign;
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
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
  {
    ui_error_t _ign = ui_dom_node_set_tag_name(root, "div");
    (void)_ign;
  }
  component->shadow_root = root;

  /* Pre-existing class/style */
  {
    ui_error_t _ign =
        ui_dom_node_set_attribute(root, "class", "existing-class");
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_dom_node_set_attribute(root, "style", "margin: 5px;");
    (void)_ign;
  }

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
  {
    ui_error_t _ign = ui_dom_node_set_attribute(root, "style", "margin: 5px");
    (void)_ign;
  }
  err = ui_modifier_apply(modifier, component);
  if (err != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_component_destroy(component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t _ign = ui_modifier_destroy(modifier);
    (void)_ign;
  }

  /* Test apply empty */
  {
    ui_error_t _ign = ui_modifier_create(&modifier);
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_component_create(&component);
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
    (void)_ign;
  }
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

  {
    ui_error_t rc_cleanup = ui_component_destroy(component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t _ign = ui_modifier_destroy(modifier);
    (void)_ign;
  }

  /* Test apply without existing */
  {
    ui_error_t _ign = ui_modifier_create(&modifier);
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_modifier_add_class(modifier, "test-class");
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_modifier_add_style(modifier, "color", "blue");
    (void)_ign;
  }

  {
    ui_error_t _ign = ui_component_create(&component);
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
    (void)_ign;
  }
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

  /* Apply modifier with empty pre-existing strings and EMPTY modifier lists to
   * hit the other side of the branch */
  {
    struct ui_modifier *empty_mod;
    {
      ui_error_t _ign = ui_modifier_create(&empty_mod);
      (void)_ign;
    }
    {
      ui_error_t _ign = ui_dom_node_set_attribute(root, "class", "");
      (void)_ign;
    }
    {
      ui_error_t _ign = ui_dom_node_set_attribute(root, "style", "");
      (void)_ign;
    }
    {
      ui_error_t _ign = ui_modifier_apply(empty_mod, component);
      (void)_ign;
    }
    {
      ui_error_t _ign = ui_modifier_destroy(empty_mod);
      (void)_ign;
    }
  }

  /* Apply modifier with empty pre-existing strings to hit the \0 checks */
  {
    ui_error_t _ign = ui_modifier_add_class(modifier, "test");
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_modifier_add_style(modifier, "color", "blue");
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_dom_node_set_attribute(root, "class", "");
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_dom_node_set_attribute(root, "style", "");
    (void)_ign;
  }
  err = ui_modifier_apply(modifier, component);
  if (err != UI_ERROR_NONE) {
    printf("ui_modifier_apply (empty) failed: %d\n", err);
    return 1;
  }

  {
    ui_error_t rc_cleanup = ui_component_destroy(component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t _ign = ui_modifier_destroy(modifier);
    (void)_ign;
  }

  return 0;
}

static int run_oom_tests(void) {
  struct ui_modifier *modifier = NULL;
  struct ui_component *component = NULL;
  struct ui_dom_node *root = NULL;
  ui_error_t err;
  int i;

  printf("Running ui_modifier OOM tests...\n");

  /* Creation OOM */
  g_malloc_fail_countdown = 0;
  err = ui_modifier_create(&modifier);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  {
    ui_error_t _ign = ui_modifier_create(&modifier);
    (void)_ign;
  }

  /* Add class OOM */
  for (i = 0; i < 2; i++) {
    g_malloc_fail_countdown = i;
    err = ui_modifier_add_class(modifier, "btn");
    g_malloc_fail_countdown = -1;
    if (err != UI_ERROR_OUT_OF_MEMORY)
      return 1;
  }

  /* Simulate NULL name/property on destroy */
  {
    struct ui_modifier_class {
      char *name;
      void *next;
    };
    struct ui_modifier_style {
      char *property;
      char *value;
      void *next;
    };
    struct ui_modifier_internal {
      struct ui_modifier_class *first_class;
      void *last_class;
      struct ui_modifier_style *first_style;
      void *last_style;
    };

    struct ui_modifier *null_test_mod = NULL;
    {
      ui_error_t _ign = ui_modifier_create(&null_test_mod);
      (void)_ign;
    }

    struct ui_modifier_internal *internal =
        (struct ui_modifier_internal *)null_test_mod;
    internal->first_class = malloc(sizeof(struct ui_modifier_class));
    internal->first_class->name = NULL;
    internal->first_class->next = NULL;

    internal->first_style = malloc(sizeof(struct ui_modifier_style));
    internal->first_style->property = NULL;
    internal->first_style->value = NULL;
    internal->first_style->next = NULL;

    {
      ui_error_t _ign = ui_modifier_destroy(null_test_mod);
      (void)_ign;
    }
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
  {
    ui_error_t _ign = ui_modifier_add_class(modifier, "btn");
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_modifier_add_style(modifier, "color", "red");
    (void)_ign;
  }

  {
    ui_error_t _ign = ui_component_create(&component);
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_dom_node_set_tag_name(root, "div");
    (void)_ign;
  }
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
  g_malloc_fail_countdown = 4; /* 1 class str, 3 for class attribute set */
  err = ui_modifier_apply(modifier, component);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  /* style set_attribute OOM */
  g_malloc_fail_countdown =
      5; /* 1 class str, 3 class attr, 1 style str, 1 style attr */
  err = ui_modifier_apply(modifier, component);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  {
    ui_error_t rc_cleanup = ui_component_destroy(component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t _ign = ui_modifier_destroy(modifier);
    (void)_ign;
  }
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
