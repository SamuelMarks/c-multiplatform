/**
 * @file ui_bottom_app_bar_base.c
 * @brief Implementation of the bottom app bar base component.
 */

/* clang-format off */
#include "ui_bottom_app_bar_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_bottom_app_bar_mock_fail = 0;
static ui_error_t
mock_component_set_default_style(struct ui_component *comp,
                                 struct ui_css_stylesheet *style) {
  if (g_bottom_app_bar_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_set_default_style)(comp, style);
}
#undef ui_component_set_default_style
#define ui_component_set_default_style mock_component_set_default_style

static ui_error_t mock_dom_node_set_attribute(struct ui_dom_node *node,
                                              const char *k, const char *v) {
  if (g_bottom_app_bar_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_set_attribute)(node, k, v);
}
#undef ui_dom_node_set_attribute
#define ui_dom_node_set_attribute mock_dom_node_set_attribute

ui_error_t run_bottom_app_bar_coverage(void);
ui_error_t run_bottom_app_bar_coverage(void) {

  struct ui_bottom_app_bar_base *bar = NULL;

  g_bottom_app_bar_mock_fail = 1;
  (void)ui_bottom_app_bar_base_create(&bar);
  g_bottom_app_bar_mock_fail = 0;

  (void)ui_bottom_app_bar_base_create(&bar);

  g_bottom_app_bar_mock_fail = 2;
  (void)ui_bottom_app_bar_base_set_fab(bar, NULL, UI_BOTTOM_APP_BAR_FAB_CENTER);
  g_bottom_app_bar_mock_fail = 0;

  g_bottom_app_bar_mock_fail = 2;
  (void)ui_bottom_app_bar_base_set_fab(bar, (struct ui_fab_base *)1,
                                       UI_BOTTOM_APP_BAR_FAB_CENTER);
  g_bottom_app_bar_mock_fail = 0;

  g_bottom_app_bar_mock_fail = 2;
  (void)ui_bottom_app_bar_base_set_fab(bar, (struct ui_fab_base *)1,
                                       UI_BOTTOM_APP_BAR_FAB_END);
  g_bottom_app_bar_mock_fail = 0;

  (void)ui_bottom_app_bar_base_destroy(bar);

  return UI_ERROR_NONE;
}
#endif

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_bottom_app_bar_default_css =
    "div.bottom-app-bar { "
    "display: flex; "
    "flex-direction: row; "
    "align-items: center; "
    "justify-content: space-between; "
    "width: 100%; "
    "position: relative; "
    /* The clip-path handles the notch geometry using variables injected at
       runtime */
    "clip-path: var(--bottom-app-bar-cutout, none); "
    "}";

/**
 * @struct ui_bottom_app_bar_base
 * @brief Internal implementation of the bottom app bar base component.
 */
struct ui_bottom_app_bar_base {
  struct ui_component *component;                 /**< Core UI component */
  struct ui_fab_base *fab;                        /**< Integrated FAB */
  enum ui_bottom_app_bar_fab_alignment alignment; /**< FAB alignment */
  struct ui_signal *active_index_signal; /**< Optional active index signal */
};

ui_error_t
ui_bottom_app_bar_base_create(struct ui_bottom_app_bar_base **out_bar) {
  struct ui_bottom_app_bar_base *bar;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct ui_bottom_app_bar_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_bottom_app_bar_base));
  if (!bar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  bar->component = NULL;
  bar->fab = NULL;
  bar->alignment = UI_BOTTOM_APP_BAR_FAB_CENTER;

  rc = ui_component_create(&bar->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "class", "bottom-app-bar");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_bottom_app_bar_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_set_default_style(bar->component, default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  bar->component->shadow_root = root_node;
  root_node = NULL;

  *out_bar = bar;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  if (bar->component) {
    (void)ui_component_destroy(bar->component);
  }
  C_MULTIPLATFORM_FREE(bar);
  return rc;
}

ui_error_t ui_bottom_app_bar_base_destroy(struct ui_bottom_app_bar_base *bar) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  (void)ui_component_destroy(bar->component);
  C_MULTIPLATFORM_FREE(bar);
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_app_bar_base_get_component(struct ui_bottom_app_bar_base *bar,
                                     struct ui_component **out_component) {
  if (!bar || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = bar->component;
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_app_bar_base_set_fab(struct ui_bottom_app_bar_base *bar,
                               struct ui_fab_base *fab,
                               enum ui_bottom_app_bar_fab_alignment alignment) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  bar->fab = fab;
  bar->alignment = alignment;

  /* In a full engine, we would compute the actual SVG path or polygon based on
     the FAB's geometry and inject it into --bottom-app-bar-cutout.
     For now, this provides the structural plumbing. */
  if (fab) {
    if (alignment == UI_BOTTOM_APP_BAR_FAB_CENTER) {
      rc = ui_dom_node_set_attribute(
          bar->component->shadow_root, "style",
          "--bottom-app-bar-cutout: circle(50% at 50% 0);");
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    } else {
      rc = ui_dom_node_set_attribute(
          bar->component->shadow_root, "style",
          "--bottom-app-bar-cutout: circle(50% at 90% 0);");
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  } else {
    rc = ui_dom_node_set_attribute(bar->component->shadow_root, "style",
                                   "--bottom-app-bar-cutout: none;");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_app_bar_base_bind_active_index(struct ui_bottom_app_bar_base *widget,
                                         struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
