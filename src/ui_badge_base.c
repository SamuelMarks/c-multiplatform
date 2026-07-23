/* clang-format off */
#include "ui_badge_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

struct ui_badge_base {
  struct ui_component *component;
  struct ui_signal *text_signal;
};

enum ui_error ui_badge_base_create(struct ui_badge_base **out_badge) {
  struct ui_badge_base *badge;
  struct ui_dom_node *root_node = NULL;
  enum ui_error rc;

  if (!out_badge) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  badge = (struct ui_badge_base *)UI_MALLOC(sizeof(struct ui_badge_base));
  if (!badge) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&badge->component);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(badge);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(badge->component);
    UI_FREE(badge);
    return rc;
  }

  ui_dom_node_set_tag_name(root_node, "span");
  ui_dom_node_set_attribute(root_node, "role", "status");

  /* Text content is stored in a child text node */
  {
    struct ui_dom_node *text_node;
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (rc == UI_ERROR_NONE) {
      ui_dom_node_append_child(root_node, text_node);
    }
  }

  badge->component->shadow_root = root_node;

  *out_badge = badge;
  return UI_ERROR_NONE;
}

void ui_badge_base_destroy(struct ui_badge_base *badge) {
  if (badge) {
    if (badge->component) {
      ui_component_destroy(badge->component);
    }
    UI_FREE(badge);
  }
}

enum ui_error ui_badge_base_set_value(struct ui_badge_base *badge, int value,
                                      int max_value) {
  char buf[32];

  if (!badge || !badge->component || !badge->component->shadow_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (value > max_value) {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%d+", max_value);
#else
    sprintf(buf, "%d+", max_value);
#endif
  } else {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%d", value);
#else
    sprintf(buf, "%d", value);
#endif
  }

  if (badge->component->shadow_root->first_child) {
    return ui_dom_node_set_text_content(
        badge->component->shadow_root->first_child, buf);
  }
  return UI_ERROR_INVALID_ARGUMENT;
}

enum ui_error ui_badge_base_set_text(struct ui_badge_base *badge,
                                     const char *text) {
  if (!badge || !badge->component || !badge->component->shadow_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (badge->component->shadow_root->first_child) {
    return ui_dom_node_set_text_content(
        badge->component->shadow_root->first_child, text ? text : "");
  }
  return UI_ERROR_INVALID_ARGUMENT;
}

enum ui_error ui_badge_base_set_hidden(struct ui_badge_base *badge,
                                       int is_hidden) {
  if (!badge || !badge->component || !badge->component->shadow_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_hidden) {
    return ui_dom_node_set_attribute(badge->component->shadow_root,
                                     "aria-hidden", "true");
  } else {
    return ui_dom_node_remove_attribute(badge->component->shadow_root,
                                        "aria-hidden");
  }
}

enum ui_error ui_badge_base_get_component(struct ui_badge_base *badge,
                                          struct ui_component **out_component) {
  if (!badge || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = badge->component;
  return UI_ERROR_NONE;
}

enum ui_error ui_badge_base_bind_text(struct ui_badge_base *widget,
                                      struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = signal;
  return UI_ERROR_NONE;
}
