/* clang-format off */
#include "ui_link_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

enum ui_error ui_link_base_create(struct ui_link_base **out_link) {
  struct ui_link_base *link;
  struct ui_component *base_comp;
  enum ui_error err;

  if (!out_link) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  link = (struct ui_link_base *)UI_MALLOC(sizeof(struct ui_link_base));
  if (!link) {
    ui_component_destroy(base_comp);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  link->base = *base_comp;
  UI_FREE(base_comp);

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &link->base.shadow_root);
  if (err != UI_ERROR_NONE) {
    UI_FREE(link);
    return err;
  }

  err = ui_dom_node_set_tag_name(link->base.shadow_root, "ui-link");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(link->base.shadow_root);
    UI_FREE(link);
    return err;
  }

  /* Links are focusable by default */
  ui_dom_node_set_attribute(link->base.shadow_root, "tabindex", "0");
  ui_dom_node_set_attribute(link->base.shadow_root, "role", "link");

  *out_link = link;
  return UI_ERROR_NONE;
}

enum ui_error ui_link_base_set_href(struct ui_link_base *link,
                                    const char *url) {
  if (!link || !url) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_dom_node_set_attribute(link->base.shadow_root, "href", url);
}

enum ui_error ui_link_base_set_text(struct ui_link_base *link,
                                    const char *text) {
  struct ui_dom_node *text_node;
  enum ui_error err;

  if (!link || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Create or find the text node */
  if (!link->base.shadow_root->first_child) {
    err = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    ui_dom_node_append_child(link->base.shadow_root, text_node);
  } else {
    text_node = link->base.shadow_root->first_child;
  }

  return ui_dom_node_set_text_content(text_node, text);
}

enum ui_error ui_link_base_bind_disabled(struct ui_link_base *widget,
                                         struct ui_signal *disabled_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->disabled_signal = disabled_signal;
  return UI_ERROR_NONE;
}

enum ui_error ui_link_base_bind_text(struct ui_link_base *widget,
                                     struct ui_signal *text_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = text_signal;
  return UI_ERROR_NONE;
}
