/**
 * @file md3_floating_toolbar.c
 * @brief Implementation of Material 3 Floating Toolbar component.
 */

/* clang-format off */
#include "material3/md3_floating_toolbar.h"
#include "ui_button_base.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct md3_floating_toolbar {
  struct ui_floating_toolbar_base *base;
  struct md3_fab *fab;
  enum ui_floating_toolbar_orientation orientation;
  enum md3_floating_toolbar_type type;
  int is_expanded;
};

ui_error_t
md3_floating_toolbar_create(struct ui_engine *engine,
                            enum ui_floating_toolbar_orientation orientation,
                            enum md3_floating_toolbar_type type,
                            struct md3_floating_toolbar **out_toolbar) {
  struct md3_floating_toolbar *tb = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  if (engine == NULL || out_toolbar == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tb = (struct md3_floating_toolbar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_floating_toolbar));
  if (tb == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(tb, 0, sizeof(*tb));

  tb->orientation = orientation;
  tb->type = type;
  tb->is_expanded = 1;

  rc = ui_floating_toolbar_base_create(&tb->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_floating_toolbar_base_set_orientation(tb->base, orientation);
  if (rc != UI_ERROR_NONE) {
    ui_floating_toolbar_base_destroy(tb->base);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }

  rc = ui_floating_toolbar_base_set_state(
      tb->base, (type == MD3_FLOATING_TOOLBAR_EXPANDABLE)
                    ? UI_FLOATING_TOOLBAR_COLLAPSED
                    : UI_FLOATING_TOOLBAR_EXPANDED);
  if (rc != UI_ERROR_NONE) {
    ui_floating_toolbar_base_destroy(tb->base);
    C_MULTIPLATFORM_FREE(tb);
    return rc;
  }
  tb->is_expanded = (type == MD3_FLOATING_TOOLBAR_EXPANDABLE) ? 0 : 1;

  rc = ui_floating_toolbar_base_get_component(tb->base, &comp);
  if (rc == UI_ERROR_NONE && comp != NULL && comp->shadow_root != NULL) {
    rc = ui_dom_node_set_attribute(comp->shadow_root, "class",
                                   "md3-floating-toolbar md3-elevation-2");
    if (rc != UI_ERROR_NONE) {
      ui_floating_toolbar_base_destroy(tb->base);
      C_MULTIPLATFORM_FREE(tb);
      return rc;
    }
  }

  *out_toolbar = tb;
  return UI_ERROR_NONE;
}

ui_error_t md3_floating_toolbar_destroy(struct md3_floating_toolbar *toolbar) {
  ui_error_t rc;

  if (toolbar == NULL) {
    return UI_ERROR_NONE;
  }

  rc = ui_floating_toolbar_base_destroy(toolbar->base);
  toolbar->base = NULL;

  C_MULTIPLATFORM_FREE(toolbar);
  return rc;
}

ui_error_t
md3_floating_toolbar_get_base(struct md3_floating_toolbar *toolbar,
                              struct ui_floating_toolbar_base **out_base) {
  if (toolbar == NULL || out_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = toolbar->base;
  return UI_ERROR_NONE;
}

ui_error_t md3_floating_toolbar_set_fab(struct md3_floating_toolbar *toolbar,
                                        struct md3_fab *fab) {
  struct ui_button_base *btn = NULL;
  struct ui_component *fab_comp = NULL;
  ui_error_t rc;

  if (toolbar == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->fab = fab;
  if (fab != NULL && fab->base != NULL) {
    rc = ui_fab_base_get_main_button(fab->base, &btn);
    if (rc == UI_ERROR_NONE && btn != NULL) {
      rc = ui_button_base_get_component(btn, &fab_comp);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return ui_floating_toolbar_base_set_fab_slot(toolbar->base, fab_comp);
}

ui_error_t
md3_floating_toolbar_toggle_expansion(struct md3_floating_toolbar *toolbar) {
  ui_error_t rc;

  if (toolbar == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  toolbar->is_expanded = !toolbar->is_expanded;
  rc = ui_floating_toolbar_base_set_state(
      toolbar->base, toolbar->is_expanded ? UI_FLOATING_TOOLBAR_EXPANDED
                                          : UI_FLOATING_TOOLBAR_COLLAPSED);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t
md3_floating_toolbar_is_expanded(const struct md3_floating_toolbar *toolbar,
                                 int *out_expanded) {
  if (toolbar == NULL || out_expanded == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_expanded = toolbar->is_expanded;
  return UI_ERROR_NONE;
}
