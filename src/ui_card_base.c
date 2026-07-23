/* clang-format off */
#include "ui_card_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_card_base_css =
    ":host { "
    "display: block; "
    "position: relative; "
    "background: var(--card-bg, #ffffff); "
    "border-radius: var(--card-radius, 4px); "
    "box-shadow: var(--card-shadow, 0 2px 1px -1px rgba(0,0,0,.2), 0 1px 1px 0 "
    "rgba(0,0,0,.14), 0 1px 3px 0 rgba(0,0,0,.12)); "
    "padding: var(--card-padding, 16px); "
    "} "
    ".header { "
    "display: flex; "
    "flex-direction: row; "
    "align-items: center; "
    "margin-bottom: var(--card-header-mb, 16px); "
    "} "
    ".content { "
    "display: block; "
    "} "
    ".actions { "
    "display: flex; "
    "flex-direction: row; "
    "align-items: center; "
    "padding-top: var(--card-actions-pt, 8px); "
    "}";

/** \brief ui_card_base */
struct ui_card_base {
  struct ui_component *component;
  struct ui_dom_node *root_node;
  struct ui_dom_node *header_node;
  struct ui_dom_node *content_node;
  struct ui_dom_node *actions_node;

  struct ui_component *header_content;
  struct ui_component *main_content;
  struct ui_component *actions_content;
  struct ui_signal *data_signal;
};

enum ui_error ui_card_base_create(struct ui_card_base **out_card) {
  struct ui_card_base *card;
  struct ui_css_stylesheet *default_style = NULL;
  enum ui_error rc;

  if (!out_card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  card = (struct ui_card_base *)UI_MALLOC(sizeof(struct ui_card_base));
  if (!card) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(card, 0, sizeof(struct ui_card_base));

  rc = ui_component_create(&card->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &card->root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_tag_name(card->root_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &card->header_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_tag_name(card->header_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(card->header_node, "class", "header");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &card->content_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_tag_name(card->content_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(card->content_node, "class", "content");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &card->actions_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_tag_name(card->actions_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(card->actions_node, "class", "actions");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  (void)ui_dom_node_append_child(card->root_node, card->header_node);
  (void)ui_dom_node_append_child(card->root_node, card->content_node);
  (void)ui_dom_node_append_child(card->root_node, card->actions_node);

  rc = ui_css_parse_stylesheet(ui_card_base_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  (void)ui_component_set_default_style(card->component, default_style);

  card->component->shadow_root = card->root_node;

  *out_card = card;
  return UI_ERROR_NONE;

cleanup:
  if (card->header_node && card->header_node->parent == NULL) {
    ui_dom_node_destroy(card->header_node);
  }
  if (card->content_node && card->content_node->parent == NULL) {
    ui_dom_node_destroy(card->content_node);
  }
  if (card->actions_node && card->actions_node->parent == NULL) {
    ui_dom_node_destroy(card->actions_node);
  }
  if (card->root_node) {
    ui_dom_node_destroy(card->root_node);
  }
  if (card->component)
    ui_component_destroy(card->component);
  UI_FREE(card);
  return rc;
}

void ui_card_base_destroy(struct ui_card_base *card) {
  if (!card) {
    return;
  }
  if (card->component) {
    ui_component_destroy(card->component);
  }
  UI_FREE(card);
}

enum ui_error ui_card_base_set_header(struct ui_card_base *card,
                                      struct ui_component *header_content) {
  if (!card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  card->header_content = header_content;
  if (header_content) {
    return ui_component_mount(header_content, card->header_node);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_card_base_set_content(struct ui_card_base *card,
                                       struct ui_component *content) {
  if (!card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  card->main_content = content;
  if (content) {
    return ui_component_mount(content, card->content_node);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_card_base_set_actions(struct ui_card_base *card,
                                       struct ui_component *actions) {
  if (!card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  card->actions_content = actions;
  if (actions) {
    return ui_component_mount(actions, card->actions_node);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_card_base_get_component(struct ui_card_base *card,
                                         struct ui_component **out_component) {
  if (!card || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = card->component;
  return UI_ERROR_NONE;
}

enum ui_error ui_card_base_bind_data(struct ui_card_base *widget,
                                     struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
