/* clang-format off */
#include "cmp_ui_fab.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_fab {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_icon;
  char *icon_name;
};

int cmp_ui_fab_create(cmp_ui_fab_t **out_fab, const char *icon_name) {
  cmp_ui_fab_t *fab;
  int err;
  size_t len;

  if (!out_fab) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_fab_t), (void **)&fab);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(fab, 0, sizeof(cmp_ui_fab_t));

  if (icon_name) {
    len = strlen(icon_name);
    err = CMP_MALLOC(len + 1, (void **)&fab->icon_name);
    if (err == CMP_SUCCESS) {
      memcpy(fab->icon_name, icon_name, len + 1);
    }
  }

  err = cmp_ui_button_create(&fab->node_root, "", 0);
  if (err != CMP_SUCCESS) {
    CMP_FREE(fab->icon_name);
    CMP_FREE(fab);
    return err;
  }

  /* Assign standard FAB style - e.g. circular, floating */
  /* This relies on the theming engine later, but we set a role/type. */
  fab->node_root->type = 3; /* Button */

  err = cmp_ui_text_create(&fab->node_icon,
                           fab->icon_name ? fab->icon_name : "", -1);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(fab->node_root);
    CMP_FREE(fab->icon_name);
    CMP_FREE(fab);
    return err;
  }

  cmp_ui_node_add_child(fab->node_root, fab->node_icon);

  *out_fab = fab;
  return CMP_SUCCESS;
}

int cmp_ui_fab_destroy(cmp_ui_fab_t *fab) {
  if (!fab) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(fab->icon_name);
  CMP_FREE(fab);
  return CMP_SUCCESS;
}

int cmp_ui_fab_get_node(cmp_ui_fab_t *fab, cmp_ui_node_t **out_node) {
  if (!fab || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = fab->node_root;
  return CMP_SUCCESS;
}

int cmp_ui_fab_set_icon(cmp_ui_fab_t *fab, const char *icon_name) {
  size_t len;
  int err;

  if (!fab) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (fab->icon_name) {
    CMP_FREE(fab->icon_name);
    fab->icon_name = NULL;
  }

  if (icon_name) {
    len = strlen(icon_name);
    err = CMP_MALLOC(len + 1, (void **)&fab->icon_name);
    if (err != CMP_SUCCESS) {
      return err;
    }
    memcpy(fab->icon_name, icon_name, len + 1);
  }

  return CMP_SUCCESS;
}
int cmp_ui_fab_bind_a11y(cmp_ui_fab_t *widget, cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "button",
                         "Floating Action Button");
  cmp_a11y_tree_set_node_traits(tree, widget->node_root->layout->id,
                                CMP_A11Y_TRAIT_BUTTON);
  return CMP_SUCCESS;
}
