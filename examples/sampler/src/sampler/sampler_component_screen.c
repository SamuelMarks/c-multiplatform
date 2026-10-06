/**
 * @file sampler_component_screen.c
 * @brief Implementation of component details screen.
 */

/* clang-format off */
#include "sampler/sampler_component_screen.h"
#include "sampler/sampler_top_app_bar.h"
#include "material3/md3_card.h"
#include "ui_card_base.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct sampler_component_screen {
  struct ui_dom_node *root_node;
  struct ui_dom_node *top_bar_node;
  struct ui_dom_node *content_node;
  struct md3_card **example_cards;
  size_t num_examples;
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

static sampler_error_t
create_example_card(struct ui_engine *engine,
                    const struct sampler_example *example_model,
                    struct md3_card **out_card, struct ui_dom_node **out_node) {
  struct md3_card *card = NULL;
  struct ui_card_base *card_base = NULL;
  struct ui_component *comp = NULL;
  struct ui_dom_node *card_root = NULL;
  ui_error_t u_rc;

  u_rc = md3_card_create(engine, MD3_CARD_ELEVATED, &card);
  if (u_rc != UI_ERROR_NONE) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  u_rc = md3_card_set_title(card, example_model->name);
  if (u_rc != UI_ERROR_NONE) {
    md3_card_destroy(card);
    return SAMPLER_ERROR_STYLE_PROP_FAILED;
  }

  u_rc = md3_card_get_base(card, &card_base);
  if (u_rc == UI_ERROR_NONE && card_base != NULL) {
    u_rc = ui_card_base_get_component(card_base, &comp);
    if (u_rc == UI_ERROR_NONE && comp != NULL) {
      card_root = comp->shadow_root;
    }
  }

  if (card_root == NULL) {
    md3_card_destroy(card);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  *out_card = card;
  *out_node = card_root;
  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_component_screen_create(struct ui_engine *engine,
                                struct sampler_nav *nav,
                                const struct sampler_component *component,
                                struct sampler_component_screen **out_screen) {
  struct sampler_component_screen *screen = NULL;
  struct sampler_top_app_bar_config top_bar_config;
  struct ui_dom_node *hero_node = NULL;
  struct ui_dom_node *desc_node = NULL;
  size_t i;
  sampler_error_t rc;
  ui_error_t u_rc;

  if (engine == NULL || nav == NULL || component == NULL ||
      out_screen == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  screen = (struct sampler_component_screen *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct sampler_component_screen));
  if (screen == NULL) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }
  memset(screen, 0, sizeof(*screen));

  /* 1. Create root screen container */
  rc = sampler_create_element_with_class(
      "div", "m3-catalog-screen m3-component-screen", &screen->root_node);
  if (rc != SAMPLER_SUCCESS) {
    C_MULTIPLATFORM_FREE(screen);
    return rc;
  }

  /* 2. Instantiate Material 3 Top App Bar */
  memset(&top_bar_config, 0, sizeof(top_bar_config));
  top_bar_config.title = component->name;
  top_bar_config.show_back_button = 1;
  top_bar_config.show_search_field = 0;
  top_bar_config.show_favorite_pin = 0;
  top_bar_config.show_theme_button = 0;
  top_bar_config.show_more_menu = 0;

  rc = sampler_top_app_bar_create(engine, &top_bar_config,
                                  &screen->top_bar_node);
  if (rc != SAMPLER_SUCCESS) {
    sampler_component_screen_destroy(&screen);
    return rc;
  }

  u_rc = ui_dom_node_append_child(screen->root_node, screen->top_bar_node);
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->top_bar_node);
    sampler_component_screen_destroy(&screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  /* 3. Create content area */
  rc = sampler_create_element_with_class("main", "m3-component-content",
                                         &screen->content_node);
  if (rc != SAMPLER_SUCCESS) {
    sampler_component_screen_destroy(&screen);
    return rc;
  }

  rc = sampler_append_child_checked(screen->root_node, screen->content_node);
  if (rc != SAMPLER_SUCCESS) {
    sampler_component_screen_destroy(&screen);
    return rc;
  }

  /* 4. Hero icon */
  rc =
      sampler_create_element_with_class("div", "m3-component-hero", &hero_node);
  if (rc != SAMPLER_SUCCESS) {
    sampler_component_screen_destroy(&screen);
    return rc;
  }

  /* Assuming there is an img or icon here. */
  {
    struct ui_dom_node *icon_node = NULL;
    rc = sampler_create_element_with_class(
        "i", "material-symbols-outlined m3-hero-icon", &icon_node);
    if (rc == SAMPLER_SUCCESS) {
      /* Dummy text for icon ID */
      sampler_create_text_child(icon_node, "widgets");
      sampler_append_child_checked(hero_node, icon_node);
    }
  }

  rc = sampler_append_child_checked(screen->content_node, hero_node);
  if (rc != SAMPLER_SUCCESS) {
    sampler_component_screen_destroy(&screen);
    return rc;
  }

  /* 5. Description */
  rc = sampler_create_element_with_class("p", "m3-component-description",
                                         &desc_node);
  if (rc != SAMPLER_SUCCESS) {
    sampler_component_screen_destroy(&screen);
    return rc;
  }

  rc = sampler_create_text_child(desc_node, component->description);
  if (rc != SAMPLER_SUCCESS) {
    sampler_component_screen_destroy(&screen);
    return rc;
  }

  rc = sampler_append_child_checked(screen->content_node, desc_node);
  if (rc != SAMPLER_SUCCESS) {
    sampler_component_screen_destroy(&screen);
    return rc;
  }

  /* 6. Examples list */
  if (component->examples_count > 0) {
    struct ui_dom_node *examples_header = NULL;

    rc = sampler_create_element_with_class("h2", "m3-examples-heading",
                                           &examples_header);
    if (rc == SAMPLER_SUCCESS) {
      sampler_create_text_child(examples_header, "Examples");
      sampler_append_child_checked(screen->content_node, examples_header);
    }

    screen->num_examples = component->examples_count;
    screen->example_cards = (struct md3_card **)C_MULTIPLATFORM_MALLOC(
        sizeof(struct md3_card *) * component->examples_count);

    if (screen->example_cards == NULL) {
      sampler_component_screen_destroy(&screen);
      return SAMPLER_ERROR_OUT_OF_MEMORY;
    }

    memset(screen->example_cards, 0,
           sizeof(struct md3_card *) * component->examples_count);

    for (i = 0; i < component->examples_count; i++) {
      struct md3_card *card = NULL;
      struct ui_dom_node *card_node = NULL;

      rc = create_example_card(engine, &component->examples[i], &card,
                               &card_node);
      if (rc != SAMPLER_SUCCESS) {
        sampler_component_screen_destroy(&screen);
        return rc;
      }

      screen->example_cards[i] = card;

      u_rc = ui_dom_node_append_child(screen->content_node, card_node);
      if (u_rc != UI_ERROR_NONE) {
        sampler_component_screen_destroy(&screen);
        return SAMPLER_ERROR_DOM_ATTACH_FAILED;
      }
    }
  }

  *out_screen = screen;
  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_component_screen_get_root(const struct sampler_component_screen *screen,
                                  struct ui_dom_node **out_root) {
  if (screen == NULL || out_root == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  *out_root = screen->root_node;
  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_component_screen_destroy(struct sampler_component_screen **screen) {
  size_t i;
  if (screen == NULL || *screen == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  if ((*screen)->example_cards != NULL) {
    for (i = 0; i < (*screen)->num_examples; i++) {
      if ((*screen)->example_cards[i] != NULL) {
        md3_card_destroy((*screen)->example_cards[i]);
      }
    }
    C_MULTIPLATFORM_FREE((*screen)->example_cards);
  }

  /* Nodes managed by widgets or parent destroy. */
  C_MULTIPLATFORM_FREE(*screen);
  *screen = NULL;

  return SAMPLER_SUCCESS;
}
