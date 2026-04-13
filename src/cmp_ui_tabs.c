/* clang-format off */
#include "cmp_ui_tabs.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct cmp_ui_tab_item {
  cmp_ui_node_t *node;
  int is_selected;
} cmp_ui_tab_item_t;

struct cmp_ui_tabs {
  cmp_ui_node_t *node_root;
  cmp_ui_tab_item_t *tabs;
  int tab_count;
  int tab_capacity;
  int selected_index;
};

int cmp_ui_tabs_create(cmp_ui_tabs_t **out_tabs) {
  cmp_ui_tabs_t *tabs;
  int err;

  if (!out_tabs) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_tabs_t), (void **)&tabs);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(tabs, 0, sizeof(cmp_ui_tabs_t));

  tabs->tab_capacity = 4;
  err = CMP_MALLOC(sizeof(cmp_ui_tab_item_t) * tabs->tab_capacity,
                   (void **)&tabs->tabs);
  if (err != CMP_SUCCESS) {
    CMP_FREE(tabs);
    return err;
  }

  err = cmp_ui_box_create(&tabs->node_root);
  if (err != CMP_SUCCESS) {
    CMP_FREE(tabs->tabs);
    CMP_FREE(tabs);
    return err;
  }

  tabs->node_root->layout->direction = CMP_FLEX_ROW;
  tabs->node_root->bg_color = 0xFFFFFFFF;
  tabs->selected_index = -1;

  *out_tabs = tabs;
  return CMP_SUCCESS;
}

int cmp_ui_tabs_destroy(cmp_ui_tabs_t *tabs) {
  if (!tabs) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (tabs->tabs) {
    CMP_FREE(tabs->tabs);
  }
  CMP_FREE(tabs);
  return CMP_SUCCESS;
}

int cmp_ui_tabs_get_node(cmp_ui_tabs_t *tabs, cmp_ui_node_t **out_node) {
  if (!tabs || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = tabs->node_root;
  return CMP_SUCCESS;
}

int cmp_ui_tabs_add_tab(cmp_ui_tabs_t *tabs, const char *title,
                        int *out_index) {
  cmp_ui_node_t *tab_node;
  int err;

  if (!tabs || !title) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (tabs->tab_count >= tabs->tab_capacity) {
    int new_cap = tabs->tab_capacity * 2;
    cmp_ui_tab_item_t *new_tabs;
    err = CMP_MALLOC(sizeof(cmp_ui_tab_item_t) * new_cap, (void **)&new_tabs);
    if (err != CMP_SUCCESS) {
      return err;
    }
    memcpy(new_tabs, tabs->tabs, sizeof(cmp_ui_tab_item_t) * tabs->tab_count);
    CMP_FREE(tabs->tabs);
    tabs->tabs = new_tabs;
    tabs->tab_capacity = new_cap;
  }

  err = cmp_ui_button_create(&tab_node, title, -1);
  if (err != CMP_SUCCESS) {
    return err;
  }

  tab_node->type = 3; /* Button structure for click events */
  cmp_ui_node_add_child(tabs->node_root, tab_node);

  tabs->tabs[tabs->tab_count].node = tab_node;
  tabs->tabs[tabs->tab_count].is_selected = 0;

  if (out_index) {
    *out_index = tabs->tab_count;
  }

  tabs->tab_count++;
  return CMP_SUCCESS;
}

int cmp_ui_tabs_set_selected(cmp_ui_tabs_t *tabs, int index) {
  int i;
  if (!tabs || index < 0 || index >= tabs->tab_count) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < tabs->tab_count; ++i) {
    tabs->tabs[i].is_selected = (i == index);
    /* Update CSS styles internally for active/inactive state */
  }

  tabs->selected_index = index;
  return CMP_SUCCESS;
}
int cmp_ui_tabs_bind_a11y(cmp_ui_tabs_t *widget, cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "tablist",
                         "Tabs");
  return CMP_SUCCESS;
}
