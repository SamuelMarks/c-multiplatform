/* clang-format off */
#include "ui_menubar_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static enum ui_error test_menubar_base(void) {
  struct ui_menubar_base *menubar;
  struct ui_component *menu_item;
  enum ui_error err;
  const char *attr_val;

  err = ui_menubar_base_create(&menubar);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_menubar_base_create failed\n");
    exit(1);
    return UI_ERROR_NONE;
  }

  err = ui_dom_node_get_attribute(menubar->base.shadow_root, "role", &attr_val);
  if (err != UI_ERROR_NONE || strcmp(attr_val, "menubar") != 0) {
    fprintf(stderr, "menubar role not set correctly\n");
    exit(1);
  }

  ui_component_create(&menu_item);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &menu_item->shadow_root);
  ui_dom_node_set_tag_name(menu_item->shadow_root, "ui-menu-item");

  err = ui_menubar_base_append_item(menubar, menu_item);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_menubar_base_append_item failed\n");
    exit(1);
  }

  if (menubar->base.shadow_root->first_child != menu_item->shadow_root) {
    fprintf(stderr, "menu item not appended correctly\n");
    exit(1);
  }
  ui_component_destroy((struct ui_component *)menubar);
  menu_item->shadow_root = NULL;
  ui_component_destroy(menu_item);
  return UI_ERROR_NONE;
}

static int test_edge_cases(void) {
  struct ui_menubar_base *menubar;
  struct ui_component item;
  int i;
  enum ui_error err;

  ui_menubar_base_create(NULL);
  ui_menubar_base_append_item(NULL, &item);
  ui_menubar_base_append_item((struct ui_menubar_base *)&item, NULL);
  ui_menubar_base_bind_active_index(NULL, NULL);

  ui_menubar_base_create(&menubar);
  ui_menubar_base_bind_active_index(menubar, (struct ui_signal *)1);
  ui_component_destroy((struct ui_component *)menubar);

  for (i = 0; i < 4; i++) {
    g_malloc_fail_countdown = i;
    err = ui_menubar_base_create(&menubar);
    if (err == UI_ERROR_NONE) {
      ui_component_destroy((struct ui_component *)menubar);
    }
  }
  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  test_menubar_base();
  test_edge_cases();
  printf("test_ui_menubar_base passed\n");
  return 0;
}
