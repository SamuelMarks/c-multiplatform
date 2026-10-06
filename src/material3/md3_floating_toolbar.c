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

#ifdef UI_TEST_MOCK_ALLOC
/** @brief Mock failure flag for md3_floating_toolbar testing. */
int g_md3_floating_toolbar_mock_fail = 0;
/** @brief Mock destroy failure flag for md3_floating_toolbar testing. */
int g_md3_floating_toolbar_destroy_mock_fail = 0;

/**
 * @brief Mock implementation of ui_fab_base_get_main_button for failure
 * testing.
 * @param fab The FAB instance.
 * @param out_btn Output pointer for the main button.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_fab_base_get_main_button.
 */
static ui_error_t
mock_md3_fab_base_get_main_button(const struct ui_fab_base *fab,
                                  struct ui_button_base **out_btn) {
  if (g_md3_floating_toolbar_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_fab_base_get_main_button(fab, out_btn);
}
#undef ui_fab_base_get_main_button
/** @cond */
#define ui_fab_base_get_main_button mock_md3_fab_base_get_main_button
/** @endcond */

/**
 * @brief Mock implementation of ui_button_base_get_component for failure
 * testing.
 * @param btn The button instance.
 * @param out_comp Output pointer for the component.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_button_base_get_component.
 */
static ui_error_t
mock_md3_button_base_get_component(struct ui_button_base *btn,
                                   struct ui_component **out_comp) {
  if (g_md3_floating_toolbar_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_button_base_get_component(btn, out_comp);
}
#undef ui_button_base_get_component
/** @cond */
#define ui_button_base_get_component mock_md3_button_base_get_component
/** @endcond */

/**
 * @brief Mock implementation of ui_floating_toolbar_base_destroy for failure
 * testing.
 * @param tb The floating toolbar base instance.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_floating_toolbar_base_destroy.
 */
static ui_error_t
mock_md3_floating_toolbar_base_destroy(struct ui_floating_toolbar_base *tb) {
  if (g_md3_floating_toolbar_destroy_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_floating_toolbar_base_destroy(tb);
}
#undef ui_floating_toolbar_base_destroy
/** @cond */
#define ui_floating_toolbar_base_destroy mock_md3_floating_toolbar_base_destroy
/** @endcond */

/**
 * @brief Mock implementation of ui_floating_toolbar_base_set_orientation for
 * failure testing.
 * @param tb The floating toolbar base instance.
 * @param o The orientation.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_floating_toolbar_base_set_orientation.
 */
static ui_error_t
mock_md3_set_orientation(struct ui_floating_toolbar_base *tb,
                         enum ui_floating_toolbar_orientation o) {
  if (g_md3_floating_toolbar_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_floating_toolbar_base_set_orientation(tb, o);
}
#undef ui_floating_toolbar_base_set_orientation
/** @cond */
#define ui_floating_toolbar_base_set_orientation mock_md3_set_orientation
/** @endcond */

/**
 * @brief Mock implementation of ui_floating_toolbar_base_get_component for
 * failure testing.
 * @param tb The floating toolbar base instance.
 * @param out_comp Output component pointer.
 * @return UI_ERROR_UNKNOWN on injected mock failure, or result of
 * ui_floating_toolbar_base_get_component.
 */
static ui_error_t mock_md3_get_component(struct ui_floating_toolbar_base *tb,
                                         struct ui_component **out_comp) {
  if (g_md3_floating_toolbar_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_floating_toolbar_base_get_component(tb, out_comp);
}
#undef ui_floating_toolbar_base_get_component
/** @cond */
#define ui_floating_toolbar_base_get_component mock_md3_get_component
/** @endcond */
#endif

/**
 * @struct md3_floating_toolbar
 * @brief Internal representation of Material 3 Floating Toolbar component.
 */
struct md3_floating_toolbar {
  struct ui_floating_toolbar_base
      *base;           /**< Underlying CDK floating toolbar base. */
  struct md3_fab *fab; /**< Mounted Material 3 FAB instance. */
  enum ui_floating_toolbar_orientation orientation; /**< Orientation layout. */
  enum md3_floating_toolbar_type type;              /**< Toolbar visual type. */
  int is_expanded; /**< 1 if expanded, 0 if collapsed. */
};

ui_error_t
md3_floating_toolbar_create(struct ui_engine *engine,
                            enum ui_floating_toolbar_orientation orientation,
                            enum md3_floating_toolbar_type type,
                            struct md3_floating_toolbar **out_toolbar) {
  struct md3_floating_toolbar *tb = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;
  ui_error_t rc_cleanup;

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
    goto cleanup;
  }

  rc = ui_floating_toolbar_base_set_state(
      tb->base, (type == MD3_FLOATING_TOOLBAR_EXPANDABLE)
                    ? UI_FLOATING_TOOLBAR_COLLAPSED
                    : UI_FLOATING_TOOLBAR_EXPANDED);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  tb->is_expanded = (type == MD3_FLOATING_TOOLBAR_EXPANDABLE) ? 0 : 1;

  rc = ui_floating_toolbar_base_get_component(tb->base, &comp);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(comp->shadow_root, "class",
                                 "md3-floating-toolbar md3-elevation-2");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  *out_toolbar = tb;
  return UI_ERROR_NONE;

cleanup:
  rc_cleanup = ui_floating_toolbar_base_destroy(tb->base);
  if (rc_cleanup != UI_ERROR_NONE) {
    rc = rc_cleanup;
  }
  C_MULTIPLATFORM_FREE(tb);
  return rc;
}

ui_error_t md3_floating_toolbar_destroy(struct md3_floating_toolbar *toolbar) {
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t rc_cleanup;

  if (toolbar == NULL) {
    return UI_ERROR_NONE;
  }

  if (toolbar->base != NULL) {
    rc_cleanup = ui_floating_toolbar_base_destroy(toolbar->base);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
    toolbar->base = NULL;
  }

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
  if (fab != NULL) {
    if (fab->base != NULL) {
      rc = ui_fab_base_get_main_button(fab->base, &btn);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      if (btn != NULL) {
        rc = ui_button_base_get_component(btn, &fab_comp);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
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
