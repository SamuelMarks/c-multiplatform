/* clang-format off */
#include "ui_bottom_app_bar_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

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

/** \brief ui_bottom_app_bar_base */
struct ui_bottom_app_bar_base {
  struct ui_component *component;
  struct ui_fab_base *fab;
  enum ui_bottom_app_bar_fab_alignment alignment;
  struct ui_signal *active_index_signal;
};

/** \brief ui_error */
enum ui_error
ui_bottom_app_bar_base_create(struct ui_bottom_app_bar_base **out_bar) {
  struct ui_bottom_app_bar_base *bar;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct ui_bottom_app_bar_base *)UI_MALLOC(
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

  ui_component_set_default_style(bar->component, default_style);

  bar->component->shadow_root = root_node;
  root_node = NULL;

  *out_bar = bar;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    ui_dom_node_destroy(root_node);
  }
  if (bar->component) {
    ui_component_destroy(bar->component);
  }
  UI_FREE(bar);
  return rc;
}

void ui_bottom_app_bar_base_destroy(struct ui_bottom_app_bar_base *bar) {
  if (!bar) {
    return;
  }
  ui_component_destroy(bar->component);
  UI_FREE(bar);
}

/** \brief ui_error */
enum ui_error
ui_bottom_app_bar_base_get_component(struct ui_bottom_app_bar_base *bar,
                                     struct ui_component **out_component) {
  if (!bar || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = bar->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_bottom_app_bar_base_set_fab(struct ui_bottom_app_bar_base *bar,
                               struct ui_fab_base *fab,
                               enum ui_bottom_app_bar_fab_alignment alignment) {
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
      ui_dom_node_set_attribute(
          bar->component->shadow_root, "style",
          "--bottom-app-bar-cutout: circle(50% at 50% 0);");
    } else {
      ui_dom_node_set_attribute(
          bar->component->shadow_root, "style",
          "--bottom-app-bar-cutout: circle(50% at 90% 0);");
    }
  } else {
    ui_dom_node_set_attribute(bar->component->shadow_root, "style",
                              "--bottom-app-bar-cutout: none;");
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_bottom_app_bar_base_bind_active_index(struct ui_bottom_app_bar_base *widget,
                                         struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
