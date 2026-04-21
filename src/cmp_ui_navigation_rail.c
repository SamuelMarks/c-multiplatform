/* clang-format off */
#include "cmp_ui_navigation_rail.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct cmp_ui_navigation_rail_dest {
  cmp_ui_node_t *node;
  int is_selected;
} cmp_ui_navigation_rail_dest_t;

struct cmp_ui_navigation_rail {
  cmp_ui_node_t *node_root;
  cmp_ui_navigation_rail_dest_t *destinations;
  int dest_count;
  int dest_capacity;
  int selected_index;
};

int cmp_ui_navigation_rail_create(cmp_ui_navigation_rail_t **out_rail) {
  cmp_ui_navigation_rail_t *rail;
  int err;

  if (!out_rail) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_navigation_rail_t), (void **)&rail);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(rail, 0, sizeof(cmp_ui_navigation_rail_t));

  rail->dest_capacity = 4;
  err = CMP_MALLOC(sizeof(cmp_ui_navigation_rail_dest_t) * rail->dest_capacity,
                   (void **)&rail->destinations);
  if (err != CMP_SUCCESS) {
    CMP_FREE(rail);
    return err;
  }

  err = cmp_ui_box_create(&rail->node_root);
  if (err != CMP_SUCCESS) {
    CMP_FREE(rail->destinations);
    CMP_FREE(rail);
    return err;
  }

  rail->node_root->layout->direction = CMP_FLEX_COLUMN;
  rail->node_root->bg_color = 0xFFF5F5F5;
  rail->selected_index = -1;

  *out_rail = rail;
  return CMP_SUCCESS;
}

int cmp_ui_navigation_rail_destroy(cmp_ui_navigation_rail_t *rail) {
  if (!rail) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (rail->destinations) {
    CMP_FREE(rail->destinations);
  }
  CMP_FREE(rail);
  return CMP_SUCCESS;
}

int cmp_ui_navigation_rail_get_node(cmp_ui_navigation_rail_t *rail,
                                    cmp_ui_node_t **out_node) {
  if (!rail || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = rail->node_root;
  return CMP_SUCCESS;
}

int cmp_ui_navigation_rail_add_destination(cmp_ui_navigation_rail_t *rail,
                                           const char *icon_name,
                                           const char *label, int *out_index) {
  cmp_ui_node_t *dest_node;
  int err;

  if (!rail) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (rail->dest_count >= rail->dest_capacity) {
    int new_cap = rail->dest_capacity * 2;
    cmp_ui_navigation_rail_dest_t *new_dests;
    err = CMP_MALLOC(sizeof(cmp_ui_navigation_rail_dest_t) * new_cap,
                     (void **)&new_dests);
    if (err != CMP_SUCCESS) {
      return err;
    }
    memcpy(new_dests, rail->destinations,
           sizeof(cmp_ui_navigation_rail_dest_t) * rail->dest_count);
    CMP_FREE(rail->destinations);
    rail->destinations = new_dests;
    rail->dest_capacity = new_cap;
  }

  err = cmp_ui_box_create(&dest_node);
  if (err != CMP_SUCCESS) {
    return err;
  }

  dest_node->layout->direction = CMP_FLEX_COLUMN;

  (void)icon_name;
  if (label) {
    cmp_ui_node_t *lbl_node;
    if (cmp_ui_text_create(&lbl_node, label, -1) == CMP_SUCCESS) {
      cmp_ui_node_add_child(dest_node, lbl_node);
    }
  }

  cmp_ui_node_add_child(rail->node_root, dest_node);

  rail->destinations[rail->dest_count].node = dest_node;
  rail->destinations[rail->dest_count].is_selected = 0;

  if (out_index) {
    *out_index = rail->dest_count;
  }

  rail->dest_count++;
  return CMP_SUCCESS;
}

int cmp_ui_navigation_rail_set_selected(cmp_ui_navigation_rail_t *rail,
                                        int index) {
  int i;
  if (!rail || index < 0 || index >= rail->dest_count) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < rail->dest_count; ++i) {
    rail->destinations[i].is_selected = (i == index);
    /* visual updates via CSS classes/properties would go here */
  }

  rail->selected_index = index;
  return CMP_SUCCESS;
}
int cmp_ui_navigation_rail_bind_a11y(cmp_ui_navigation_rail_t *widget,
                                     cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "navigation",
                         "Navigation Rail");
  return CMP_SUCCESS;
}
