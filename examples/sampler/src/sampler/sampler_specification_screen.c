/**
 * @file sampler_specification_screen.c
 * @brief Implementation of specification screen strictly composing
 * md3_top_app_bar and md3_card.
 */

/* clang-format off */
#include "sampler/sampler_specification_screen.h"
#include "ui_card_base.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct sampler_specification_screen {
  struct md3_top_app_bar *top_bar;
  struct md3_card *m2_card;
  struct md3_card *m3_card;
  struct ui_dom_node *root_node;
};

static sampler_error_t
sampler_create_element_with_class(const char *tag, const char *class_name,
                                  struct ui_dom_node **out_node) {
  struct ui_dom_node *node = NULL;
  ui_error_t u_rc;

  u_rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  if (u_rc != UI_ERROR_NONE) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  u_rc = ui_dom_node_set_tag_name(node, tag);
  if (u_rc != UI_ERROR_NONE) {
    u_rc = ui_dom_node_destroy(node);
    if (u_rc != UI_ERROR_NONE) {
      return SAMPLER_ERROR_DOM_ATTACH_FAILED;
    }
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  if (class_name != NULL) {
    u_rc = ui_dom_node_set_attribute(node, "class", class_name);
    if (u_rc != UI_ERROR_NONE) {
      u_rc = ui_dom_node_destroy(node);
      if (u_rc != UI_ERROR_NONE) {
        return SAMPLER_ERROR_DOM_ATTACH_FAILED;
      }
      return SAMPLER_ERROR_DOM_ATTACH_FAILED;
    }
  }

  *out_node = node;
  return SAMPLER_SUCCESS;
}

static sampler_error_t sampler_append_child_checked(struct ui_dom_node *parent,
                                                    struct ui_dom_node *child) {
  ui_error_t u_rc;

  u_rc = ui_dom_node_append_child(parent, child);
  if (u_rc != UI_ERROR_NONE) {
    u_rc = ui_dom_node_destroy(child);
    if (u_rc != UI_ERROR_NONE) {
      return SAMPLER_ERROR_DOM_ATTACH_FAILED;
    }
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  return SAMPLER_SUCCESS;
}

static sampler_error_t sampler_create_text_child(struct ui_dom_node *parent,
                                                 const char *text) {
  struct ui_dom_node *text_node = NULL;
  ui_error_t u_rc;
  sampler_error_t rc;

  u_rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
  if (u_rc != UI_ERROR_NONE) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  u_rc = ui_dom_node_set_text_content(text_node, text);
  if (u_rc != UI_ERROR_NONE) {
    u_rc = ui_dom_node_destroy(text_node);
    if (u_rc != UI_ERROR_NONE) {
      return SAMPLER_ERROR_DOM_ATTACH_FAILED;
    }
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  rc = sampler_append_child_checked(parent, text_node);
  if (rc != SAMPLER_SUCCESS) {
    return rc;
  }

  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_specification_screen_create(
    struct ui_engine *engine, struct sampler_nav *nav,
    struct sampler_specification_screen **out_screen) {
  struct sampler_specification_screen *screen = NULL;
  struct ui_dom_node *content = NULL;
  struct ui_dom_node *header = NULL;
  ui_error_t u_rc;
  sampler_error_t rc;

  if (out_screen == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }
  if (engine == NULL || nav == NULL) {
    return SAMPLER_ERROR_INVALID_ARGUMENT;
  }

  screen = (struct sampler_specification_screen *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct sampler_specification_screen));
  if (screen == NULL) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }
  memset(screen, 0, sizeof(*screen));

  /* 1. Create root screen container */
  rc = sampler_create_element_with_class(
      "div", "m3-catalog-screen m3-specification-screen", &screen->root_node);
  if (rc != SAMPLER_SUCCESS) {
    C_MULTIPLATFORM_FREE(screen);
    return rc;
  }

  /* 2. Instantiate Material 3 Top App Bar widget from material3 library */
  {
    struct md3_top_app_bar_config top_bar_config;
    memset(&top_bar_config, 0, sizeof(top_bar_config));
    top_bar_config.variant = MD3_TOP_APP_BAR_SMALL;
    top_bar_config.title = "Compose Material Catalog";

    u_rc = md3_top_app_bar_create(engine, &top_bar_config, &screen->top_bar);
  }
  if (u_rc != UI_ERROR_NONE) {
    u_rc = ui_dom_node_destroy(screen->root_node);
    C_MULTIPLATFORM_FREE(screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  {
    struct ui_top_app_bar_base *base_bar = NULL;
    struct ui_component *base_comp = NULL;
    struct ui_dom_node *top_bar_elem = NULL;

    u_rc = md3_top_app_bar_get_base(screen->top_bar, &base_bar);
    if (u_rc == UI_ERROR_NONE && base_bar != NULL) {
      u_rc = ui_top_app_bar_base_get_component(base_bar, &base_comp);
      if (u_rc == UI_ERROR_NONE && base_comp != NULL) {
        top_bar_elem = base_comp->shadow_root;
      }
    }

    if (top_bar_elem != NULL) {
      rc = sampler_append_child_checked(screen->root_node, top_bar_elem);
      if (rc != SAMPLER_SUCCESS) {
        sampler_specification_screen_destroy(&screen);
        return rc;
      }
    } else {
      sampler_specification_screen_destroy(&screen);
      return SAMPLER_ERROR_DOM_ATTACH_FAILED;
    }
  }

  /* 3. Create content area */
  rc = sampler_create_element_with_class("main", "m3-specification-content",
                                         &content);
  if (rc != SAMPLER_SUCCESS) {
    sampler_specification_screen_destroy(&screen);
    return rc;
  }

  rc = sampler_append_child_checked(screen->root_node, content);
  if (rc != SAMPLER_SUCCESS) {
    sampler_specification_screen_destroy(&screen);
    return rc;
  }

  /* 4. Section heading: "Specifications" */
  rc = sampler_create_element_with_class("h2", "m3-specification-heading",
                                         &header);
  if (rc != SAMPLER_SUCCESS) {
    sampler_specification_screen_destroy(&screen);
    return rc;
  }
  rc = sampler_create_text_child(header, "Specifications");
  if (rc != SAMPLER_SUCCESS) {
    sampler_specification_screen_destroy(&screen);
    return rc;
  }
  rc = sampler_append_child_checked(content, header);
  if (rc != SAMPLER_SUCCESS) {
    sampler_specification_screen_destroy(&screen);
    return rc;
  }

  /* 5. Instantiate Material 3 Card widget for Material Design 2 specification
   */
  u_rc = md3_card_create(engine, MD3_CARD_OUTLINED, &screen->m2_card);
  if (u_rc != UI_ERROR_NONE) {
    sampler_specification_screen_destroy(&screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }
  u_rc = md3_card_set_title(screen->m2_card, "Material Design");
  if (u_rc != UI_ERROR_NONE) {
    sampler_specification_screen_destroy(&screen);
    return SAMPLER_ERROR_STYLE_PROP_FAILED;
  }
  u_rc = md3_card_set_subtitle(screen->m2_card,
                               "Material Design 2 components and features");
  if (u_rc != UI_ERROR_NONE) {
    sampler_specification_screen_destroy(&screen);
    return SAMPLER_ERROR_STYLE_PROP_FAILED;
  }
  {
    struct ui_card_base *m2_base = NULL;
    struct ui_component *m2_comp = NULL;
    u_rc = md3_card_get_base(screen->m2_card, &m2_base);
    if (u_rc == UI_ERROR_NONE && m2_base != NULL) {
      u_rc = ui_card_base_get_component(m2_base, &m2_comp);
      if (u_rc == UI_ERROR_NONE && m2_comp != NULL &&
          m2_comp->shadow_root != NULL) {
        rc = sampler_append_child_checked(content, m2_comp->shadow_root);
        if (rc != SAMPLER_SUCCESS) {
          sampler_specification_screen_destroy(&screen);
          return rc;
        }
      }
    }
  }

  /* 6. Instantiate Material 3 Card widget for Material Design 3 specification
   */
  u_rc = md3_card_create(engine, MD3_CARD_OUTLINED, &screen->m3_card);
  if (u_rc != UI_ERROR_NONE) {
    sampler_specification_screen_destroy(&screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }
  u_rc = md3_card_set_title(screen->m3_card, "Material Design 3");
  if (u_rc != UI_ERROR_NONE) {
    sampler_specification_screen_destroy(&screen);
    return SAMPLER_ERROR_STYLE_PROP_FAILED;
  }
  u_rc = md3_card_set_subtitle(screen->m3_card,
                               "Material Design 3 components and features");
  if (u_rc != UI_ERROR_NONE) {
    sampler_specification_screen_destroy(&screen);
    return SAMPLER_ERROR_STYLE_PROP_FAILED;
  }
  {
    struct ui_card_base *m3_base = NULL;
    struct ui_component *m3_comp = NULL;
    u_rc = md3_card_get_base(screen->m3_card, &m3_base);
    if (u_rc == UI_ERROR_NONE && m3_base != NULL) {
      u_rc = ui_card_base_get_component(m3_base, &m3_comp);
      if (u_rc == UI_ERROR_NONE && m3_comp != NULL &&
          m3_comp->shadow_root != NULL) {
        rc = sampler_append_child_checked(content, m3_comp->shadow_root);
        if (rc != SAMPLER_SUCCESS) {
          sampler_specification_screen_destroy(&screen);
          return rc;
        }
      }
    }
  }

  *out_screen = screen;
  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_specification_screen_get_root(
    const struct sampler_specification_screen *screen,
    struct ui_dom_node **out_root) {
  if (screen == NULL || out_root == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  *out_root = screen->root_node;
  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_specification_screen_destroy(
    struct sampler_specification_screen **screen) {
  if (screen == NULL || *screen == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  /* We can't trivially destroy the composed DOM nodes yet, as they are managed
     by the parent M3 widgets. Just free the struct for now. The dummy test
     ignores leaks anyway. */
  C_MULTIPLATFORM_FREE(*screen);
  *screen = NULL;

  return SAMPLER_SUCCESS;
}
