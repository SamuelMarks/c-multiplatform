/* clang-format off */
#include "cmp_ui_modal_drawer.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct cmp_ui_modal_drawer_item {
  cmp_ui_node_t *node;
  int is_selected;
} cmp_ui_modal_drawer_item_t;

struct cmp_ui_modal_drawer {
  cmp_ui_node_t *node_root;   /* The scrim/overlay container */
  cmp_ui_node_t *node_drawer; /* The actual sliding sheet */
  cmp_ui_node_t *node_items;  /* Container for the items */
  cmp_ui_modal_drawer_item_t *items;
  int item_count;
  int item_capacity;
  int selected_index;
  int is_open;
};

int cmp_ui_modal_drawer_create(cmp_ui_modal_drawer_t **out_drawer) {
  cmp_ui_modal_drawer_t *drawer;
  int err;

  if (!out_drawer) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_modal_drawer_t), (void **)&drawer);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(drawer, 0, sizeof(cmp_ui_modal_drawer_t));

  drawer->item_capacity = 8;
  err = CMP_MALLOC(sizeof(cmp_ui_modal_drawer_item_t) * drawer->item_capacity,
                   (void **)&drawer->items);
  if (err != CMP_SUCCESS) {
    CMP_FREE(drawer);
    return err;
  }

  err = cmp_ui_box_create(&drawer->node_root);
  if (err != CMP_SUCCESS) {
    CMP_FREE(drawer->items);
    CMP_FREE(drawer);
    return err;
  }

  err = cmp_ui_box_create(&drawer->node_drawer);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(drawer->node_root);
    CMP_FREE(drawer->items);
    CMP_FREE(drawer);
    return err;
  }

  err = cmp_ui_box_create(&drawer->node_items);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(drawer->node_drawer);
    cmp_ui_node_destroy(drawer->node_root);
    CMP_FREE(drawer->items);
    CMP_FREE(drawer);
    return err;
  }

  /* Scrim properties */
  drawer->node_root->layout->position_type = CMP_POSITION_ABSOLUTE;
  drawer->node_root->bg_color = 0x80000000; /* Semi-transparent black */

  /* Drawer properties */
  drawer->node_drawer->layout->position_type = CMP_POSITION_ABSOLUTE;
  drawer->node_drawer->layout->direction = CMP_FLEX_COLUMN;
  drawer->node_drawer->bg_color = 0xFFFFFFFF; /* Solid white */

  drawer->node_items->layout->direction = CMP_FLEX_COLUMN;

  cmp_ui_node_add_child(drawer->node_drawer, drawer->node_items);
  cmp_ui_node_add_child(drawer->node_root, drawer->node_drawer);

  drawer->selected_index = -1;
  drawer->is_open = 0;

  *out_drawer = drawer;
  return CMP_SUCCESS;
}

int cmp_ui_modal_drawer_destroy(cmp_ui_modal_drawer_t *drawer) {
  if (!drawer) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (drawer->items) {
    CMP_FREE(drawer->items);
  }
  CMP_FREE(drawer);
  return CMP_SUCCESS;
}

int cmp_ui_modal_drawer_get_node(cmp_ui_modal_drawer_t *drawer,
                                 cmp_ui_node_t **out_node) {
  if (!drawer || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = drawer->node_root;
  return CMP_SUCCESS;
}

int cmp_ui_modal_drawer_set_open(cmp_ui_modal_drawer_t *drawer, int open) {
  if (!drawer) {
    return CMP_ERROR_INVALID_ARG;
  }

  drawer->is_open = open;
  /* Apply layout or CSS classes to animate sliding in/out */
  return CMP_SUCCESS;
}

int cmp_ui_modal_drawer_add_item(cmp_ui_modal_drawer_t *drawer,
                                 const char *icon_name, const char *label,
                                 int *out_index) {
  cmp_ui_node_t *item_node;
  int err;

  if (!drawer || !label) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (drawer->item_count >= drawer->item_capacity) {
    int new_cap = drawer->item_capacity * 2;
    cmp_ui_modal_drawer_item_t *new_items;
    err = CMP_MALLOC(sizeof(cmp_ui_modal_drawer_item_t) * new_cap,
                     (void **)&new_items);
    if (err != CMP_SUCCESS) {
      return err;
    }
    memcpy(new_items, drawer->items,
           sizeof(cmp_ui_modal_drawer_item_t) * drawer->item_count);
    CMP_FREE(drawer->items);
    drawer->items = new_items;
    drawer->item_capacity = new_cap;
  }

  err = cmp_ui_button_create(&item_node, label, -1);
  if (err != CMP_SUCCESS) {
    return err;
  }

  item_node->type = 3; /* Button / List Item */
  (void)icon_name;

  cmp_ui_node_add_child(drawer->node_items, item_node);

  drawer->items[drawer->item_count].node = item_node;
  drawer->items[drawer->item_count].is_selected = 0;

  if (out_index) {
    *out_index = drawer->item_count;
  }

  drawer->item_count++;
  return CMP_SUCCESS;
}

int cmp_ui_modal_drawer_set_selected(cmp_ui_modal_drawer_t *drawer, int index) {
  int i;
  if (!drawer || index < 0 || index >= drawer->item_count) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < drawer->item_count; ++i) {
    drawer->items[i].is_selected = (i == index);
    /* Update CSS styles internally for active/inactive state */
  }

  drawer->selected_index = index;
  return CMP_SUCCESS;
}
int cmp_ui_modal_drawer_bind_a11y(cmp_ui_modal_drawer_t *widget,
                                  cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "dialog",
                         "Modal Drawer");
  return CMP_SUCCESS;
}
