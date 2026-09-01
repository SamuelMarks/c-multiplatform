/* clang-format off */
#include "ui_scaffold_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_mock_append_child_fail;

static ui_error_t test_scaffold_base(void) {
  struct ui_scaffold_base *scaffold;
  struct ui_component *top_bar, *main_content;
  ui_error_t err;

  err = ui_scaffold_base_create(&scaffold);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_scaffold_base_create failed\n");
    exit(1);
    return UI_ERROR_NONE;
  }

  ui_component_create(&top_bar);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &top_bar->shadow_root);
  ui_dom_node_set_tag_name(top_bar->shadow_root, "nav");

  ui_component_create(&main_content);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &main_content->shadow_root);
  ui_dom_node_set_tag_name(main_content->shadow_root, "main");

  err = ui_scaffold_base_set_top_bar(scaffold, top_bar);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_scaffold_base_set_top_bar failed\n");
    exit(1);
  }

  err = ui_scaffold_base_set_main_content(scaffold, main_content);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_scaffold_base_set_main_content failed\n");
    exit(1);
  }

  if (scaffold->slot_top_bar->first_child != top_bar->shadow_root) {
    fprintf(stderr, "top bar not mounted to slot\n");
    exit(1);
  }

  if (scaffold->slot_main_content->first_child != main_content->shadow_root) {
    fprintf(stderr, "main content not mounted to slot\n");
    exit(1);
  }
  {
    ui_error_t rc_cleanup =
        ui_component_destroy((struct ui_component *)scaffold);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  top_bar->shadow_root = NULL;
  {
    ui_error_t rc_cleanup = ui_component_destroy(top_bar);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  main_content->shadow_root = NULL;
  {
    ui_error_t rc_cleanup = ui_component_destroy(main_content);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

int test_scaffold_base_extra(void);
int test_scaffold_base_oom(void);
int main(void) {
  test_scaffold_base();
  test_scaffold_base_extra();
  test_scaffold_base_oom();
  printf("test_ui_scaffold_base passed\n");
  return 0;
}
int test_scaffold_base_extra(void) {
  struct ui_scaffold_base *scaffold = NULL;
  struct ui_dom_node *dummy_slot = NULL;

  ui_scaffold_base_create(NULL);
  ui_scaffold_base_create(&scaffold);
  if (scaffold) {
    ui_scaffold_base_set_top_bar(NULL, NULL);
    ui_scaffold_base_set_top_bar(scaffold, NULL);
    ui_scaffold_base_set_main_content(NULL, NULL);
    ui_scaffold_base_set_main_content(scaffold, NULL);
    ui_scaffold_base_bind_data(NULL, NULL);
    ui_scaffold_base_bind_data(scaffold, NULL);
    {
      ui_error_t rc_cleanup =
          ui_component_destroy((struct ui_component *)scaffold);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  return 0;
}

int test_scaffold_base_oom(void) {
  extern int g_malloc_fail_countdown;
  struct ui_scaffold_base *scaffold = NULL;
  int i;
  for (i = 0; i < 20; ++i) {
    g_malloc_fail_countdown = i;
    ui_scaffold_base_create(&scaffold);
  }
  g_malloc_fail_countdown = -1;
  return 0;
}
