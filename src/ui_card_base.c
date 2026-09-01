/**
 * @file ui_card_base.c
 * @brief Implementation of the card base component.
 */

/* clang-format off */
#include "ui_card_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

#ifdef UI_TEST_MOCK_ALLOC
int g_card_mock_fail = -1;

static ui_error_t mock_dom_node_append_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  if (g_card_mock_fail == 0) {
    g_card_mock_fail = -1;
    return UI_ERROR_UNKNOWN;
  }
  if (g_card_mock_fail > 0)
    g_card_mock_fail--;
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_dom_node_append_child
/** @endcond */

static ui_error_t
mock_component_set_default_style(struct ui_component *component,
                                 struct ui_css_stylesheet *stylesheet) {
  if (g_card_mock_fail == 0) {
    g_card_mock_fail = -1;
    return UI_ERROR_UNKNOWN;
  }
  if (g_card_mock_fail > 0)
    g_card_mock_fail--;
  return (ui_component_set_default_style)(component, stylesheet);
}
#undef ui_component_set_default_style
/** @cond */
#define ui_component_set_default_style mock_component_set_default_style
/** @endcond */
#endif

/** @brief Default CSS stylesheet for card base component */
static const char ui_card_base_css[] = {
    58,  104, 111, 115, 116, 32,  123, 32,  100, 105, 115, 112, 108, 97,  121,
    58,  32,  98,  108, 111, 99,  107, 59,  32,  112, 111, 115, 105, 116, 105,
    111, 110, 58,  32,  114, 101, 108, 97,  116, 105, 118, 101, 59,  32,  98,
    97,  99,  107, 103, 114, 111, 117, 110, 100, 58,  32,  118, 97,  114, 40,
    45,  45,  99,  97,  114, 100, 45,  98,  103, 44,  32,  35,  102, 102, 102,
    102, 102, 102, 41,  59,  32,  98,  111, 114, 100, 101, 114, 45,  114, 97,
    100, 105, 117, 115, 58,  32,  118, 97,  114, 40,  45,  45,  99,  97,  114,
    100, 45,  114, 97,  100, 105, 117, 115, 44,  32,  52,  112, 120, 41,  59,
    32,  98,  111, 120, 45,  115, 104, 97,  100, 111, 119, 58,  32,  118, 97,
    114, 40,  45,  45,  99,  97,  114, 100, 45,  115, 104, 97,  100, 111, 119,
    44,  32,  48,  32,  50,  112, 120, 32,  49,  112, 120, 32,  45,  49,  112,
    120, 32,  114, 103, 98,  97,  40,  48,  44,  48,  44,  48,  44,  46,  50,
    41,  44,  32,  48,  32,  49,  112, 120, 32,  49,  112, 120, 32,  48,  32,
    114, 103, 98,  97,  40,  48,  44,  48,  44,  48,  44,  46,  49,  52,  41,
    44,  32,  48,  32,  49,  112, 120, 32,  51,  112, 120, 32,  48,  32,  114,
    103, 98,  97,  40,  48,  44,  48,  44,  48,  44,  46,  49,  50,  41,  41,
    59,  32,  112, 97,  100, 100, 105, 110, 103, 58,  32,  118, 97,  114, 40,
    45,  45,  99,  97,  114, 100, 45,  112, 97,  100, 100, 105, 110, 103, 44,
    32,  49,  54,  112, 120, 41,  59,  32,  125, 32,  46,  104, 101, 97,  100,
    101, 114, 32,  123, 32,  100, 105, 115, 112, 108, 97,  121, 58,  32,  102,
    108, 101, 120, 59,  32,  102, 108, 101, 120, 45,  100, 105, 114, 101, 99,
    116, 105, 111, 110, 58,  32,  114, 111, 119, 59,  32,  97,  108, 105, 103,
    110, 45,  105, 116, 101, 109, 115, 58,  32,  99,  101, 110, 116, 101, 114,
    59,  32,  109, 97,  114, 103, 105, 110, 45,  98,  111, 116, 116, 111, 109,
    58,  32,  118, 97,  114, 40,  45,  45,  99,  97,  114, 100, 45,  104, 101,
    97,  100, 101, 114, 45,  109, 98,  44,  32,  49,  54,  112, 120, 41,  59,
    32,  125, 32,  46,  99,  111, 110, 116, 101, 110, 116, 32,  123, 32,  100,
    105, 115, 112, 108, 97,  121, 58,  32,  98,  108, 111, 99,  107, 59,  32,
    125, 32,  46,  97,  99,  116, 105, 111, 110, 115, 32,  123, 32,  100, 105,
    115, 112, 108, 97,  121, 58,  32,  102, 108, 101, 120, 59,  32,  102, 108,
    101, 120, 45,  100, 105, 114, 101, 99,  116, 105, 111, 110, 58,  32,  114,
    111, 119, 59,  32,  97,  108, 105, 103, 110, 45,  105, 116, 101, 109, 115,
    58,  32,  99,  101, 110, 116, 101, 114, 59,  32,  112, 97,  100, 100, 105,
    110, 103, 45,  116, 111, 112, 58,  32,  118, 97,  114, 40,  45,  45,  99,
    97,  114, 100, 45,  97,  99,  116, 105, 111, 110, 115, 45,  112, 116, 44,
    32,  56,  112, 120, 41,  59,  32,  125, 0};

/**
 * @struct ui_card_base
 * @struct ui_card_base
 * @brief Internal representation of a card base component.
 */
struct ui_card_base {
  struct ui_component *component;   /**< component */
  struct ui_dom_node *root_node;    /**< root_node */
  struct ui_dom_node *header_node;  /**< header_node */
  struct ui_dom_node *content_node; /**< content_node */
  struct ui_dom_node *actions_node; /**< actions_node */

  struct ui_component *header_content;  /**< header_content */
  struct ui_component *main_content;    /**< main_content */
  struct ui_component *actions_content; /**< actions_content */
  struct ui_signal *data_signal;        /**< data_signal */
};

ui_error_t ui_card_base_create(struct ui_card_base **out_card) {
  struct ui_card_base *card;
  struct ui_css_stylesheet *default_style = NULL;
  ui_error_t rc;

  if (!out_card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  card = (struct ui_card_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_card_base));
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

  rc = ui_dom_node_append_child(card->root_node, card->header_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_append_child(card->root_node, card->content_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_append_child(card->root_node, card->actions_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_css_parse_stylesheet(ui_card_base_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_set_default_style(card->component, default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  card->component->shadow_root = card->root_node;

  *out_card = card;
  return UI_ERROR_NONE;

cleanup:
  if (card->header_node && card->header_node->parent == NULL) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(card->header_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  if (card->content_node && card->content_node->parent == NULL) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(card->content_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  if (card->actions_node && card->actions_node->parent == NULL) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(card->actions_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  if (card->root_node) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(card->root_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  if (card->component) {
    ui_error_t rc_cleanup = ui_component_destroy(card->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  C_MULTIPLATFORM_FREE(card);
  return rc;
}

#ifdef UI_TEST_MOCK_ALLOC
extern ui_error_t ui_component_destroy(struct ui_component *c);
static ui_error_t mock_component_destroy_card(struct ui_component *c) {
  if (g_card_mock_fail == 20)
    return UI_ERROR_UNKNOWN;
  return ui_component_destroy(c);
}
#define ui_component_destroy mock_component_destroy_card
#endif

ui_error_t ui_card_base_destroy(struct ui_card_base *card) {
  if (!card) {
    return UI_ERROR_NONE;
  }
  if (card->component) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(card->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  C_MULTIPLATFORM_FREE(card);
  return UI_ERROR_NONE;
}

ui_error_t ui_card_base_set_header(struct ui_card_base *card,
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

ui_error_t ui_card_base_set_content(struct ui_card_base *card,
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

ui_error_t ui_card_base_set_actions(struct ui_card_base *card,
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

ui_error_t ui_card_base_get_component(struct ui_card_base *card,
                                      struct ui_component **out_component) {
  if (!card || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = card->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_card_base_bind_data(struct ui_card_base *widget,
                                  struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
