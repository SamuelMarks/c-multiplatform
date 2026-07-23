/* clang-format off */
#include "ui_scaffold_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static enum ui_error test_scaffold_base(void) {
  struct ui_scaffold_base *scaffold;
  struct ui_component *top_bar, *main_content;
  enum ui_error err;

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
  ui_component_destroy((struct ui_component *)scaffold);
  top_bar->shadow_root = NULL;
  ui_component_destroy(top_bar);
  main_content->shadow_root = NULL;
  ui_component_destroy(main_content);
}

int main(void) {
  test_scaffold_base();
  printf("test_ui_scaffold_base passed\n");
  return 0;
}
