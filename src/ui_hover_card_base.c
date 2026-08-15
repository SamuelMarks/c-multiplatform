/* clang-format off */
#include "ui_hover_card_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

static const char *ui_hover_card_base_default_css =
    ".hover-card-container { "
    "position: absolute; "
    "display: none; "
    "} "
    ".hover-card-container[data-state=\"open\"] { "
    "display: block; "
    "}";

/** \brief ui_hover_card_base */
struct ui_hover_card_base {
  struct ui_component *component;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

/** \brief ui_error */
ui_error_t
ui_hover_card_base_create(struct ui_hover_card_base **out_hover_card) {
  struct ui_hover_card_base *hover_card;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_hover_card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  hover_card = (struct ui_hover_card_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_hover_card_base));
  if (!hover_card) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  hover_card->component = NULL;

  rc = ui_component_create(&hover_card->component);
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

  rc = ui_dom_node_set_attribute(root_node, "class", "hover-card-container");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "data-state", "closed");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_hover_card_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  (void)ui_component_set_default_style(hover_card->component, default_style);

  hover_card->component->shadow_root = root_node;
  root_node = NULL;

  *out_hover_card = hover_card;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  if (hover_card->component) {
    (void)ui_component_destroy(hover_card->component);
  }
  C_MULTIPLATFORM_FREE(hover_card);
  return rc;
}

ui_error_t ui_hover_card_base_destroy(struct ui_hover_card_base *hover_card) {
  if (!hover_card) {
    return UI_ERROR_NONE;
  }
  if (hover_card->component) {
    (void)ui_component_destroy(hover_card->component);
  }
  C_MULTIPLATFORM_FREE(hover_card);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_hover_card_base_get_component(struct ui_hover_card_base *hover_card,
                                 struct ui_component **out_component) {
  if (!hover_card || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = hover_card->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_hover_card_base_on_mouse_enter(struct ui_hover_card_base *hover_card) {
  if (!hover_card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* In a real implementation, start open delay timer. */
  if (hover_card->component && hover_card->component->shadow_root) {
    return ui_dom_node_set_attribute(hover_card->component->shadow_root,
                                     "data-state", "open");
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_hover_card_base_on_mouse_leave(struct ui_hover_card_base *hover_card,
                                  float cursor_x, float cursor_y) {
  (void)cursor_x;
  (void)cursor_y;
  if (!hover_card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* In a real implementation, check if cursor is in safe polygon (triangle
   * towards hover card). */
  /* If not, start close delay timer or close immediately. */
  if (hover_card->component && hover_card->component->shadow_root) {
    return ui_dom_node_set_attribute(hover_card->component->shadow_root,
                                     "data-state", "closed");
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_hover_card_base_bind_open(struct ui_hover_card_base *widget,
                                        struct ui_signal *open_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_hover_card_base_get_animating_signal(struct ui_hover_card_base *widget,
                                        struct ui_computed **out_animating) {
  if (!widget || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = widget->animating_signal;
  return UI_ERROR_NONE;
}
