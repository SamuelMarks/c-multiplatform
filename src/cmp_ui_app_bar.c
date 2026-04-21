/* clang-format off */
#include "cmp_ui_app_bar.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_app_bar {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_title;
  cmp_ui_app_bar_placement_t placement;
  char *title;
};

int cmp_ui_app_bar_create(cmp_ui_app_bar_t **out_bar,
                          cmp_ui_app_bar_placement_t placement) {
  cmp_ui_app_bar_t *bar;
  int err;

  if (!out_bar) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_app_bar_t), (void **)&bar);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(bar, 0, sizeof(cmp_ui_app_bar_t));

  bar->placement = placement;

  err = cmp_ui_box_create(&bar->node_root);
  if (err != CMP_SUCCESS) {
    CMP_FREE(bar);
    return err;
  }

  bar->node_root->layout->direction = CMP_FLEX_ROW;
  /* Specific styling (shadows, background color based on placement) */
  bar->node_root->bg_color = 0xFFF5F5F5;

  err = cmp_ui_text_create(&bar->node_title, "", 0);
  if (err == CMP_SUCCESS) {
    cmp_ui_node_add_child(bar->node_root, bar->node_title);
  }

  *out_bar = bar;
  return CMP_SUCCESS;
}

int cmp_ui_app_bar_destroy(cmp_ui_app_bar_t *bar) {
  if (!bar) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (bar->title) {
    CMP_FREE(bar->title);
  }
  CMP_FREE(bar);
  return CMP_SUCCESS;
}

int cmp_ui_app_bar_get_node(cmp_ui_app_bar_t *bar, cmp_ui_node_t **out_node) {
  if (!bar || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = bar->node_root;
  return CMP_SUCCESS;
}

int cmp_ui_app_bar_set_title(cmp_ui_app_bar_t *bar, const char *title) {
  size_t len;
  int err;

  if (!bar) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (bar->title) {
    CMP_FREE(bar->title);
    bar->title = NULL;
  }

  if (title) {
    len = strlen(title);
    err = CMP_MALLOC(len + 1, (void **)&bar->title);
    if (err != CMP_SUCCESS) {
      return err;
    }
    memcpy(bar->title, title, len + 1);

    if (bar->node_title->properties) {
      CMP_FREE(bar->node_title->properties);
    }
    err = CMP_MALLOC(len + 1, (void **)&bar->node_title->properties);
    if (err == CMP_SUCCESS) {
      memcpy(bar->node_title->properties, title, len + 1);
    }
  } else {
    if (bar->node_title->properties) {
      CMP_FREE(bar->node_title->properties);
    }
    err = CMP_MALLOC(1, (void **)&bar->node_title->properties);
    if (err == CMP_SUCCESS) {
      ((char *)bar->node_title->properties)[0] = '\0';
    }
  }

  return CMP_SUCCESS;
}

int cmp_ui_app_bar_add_action(cmp_ui_app_bar_t *bar,
                              cmp_ui_node_t *action_node) {
  if (!bar || !action_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  return cmp_ui_node_add_child(bar->node_root, action_node);
}
int cmp_ui_app_bar_bind_a11y(cmp_ui_app_bar_t *widget, cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "group",
                         "App Bar");
  return CMP_SUCCESS;
}
