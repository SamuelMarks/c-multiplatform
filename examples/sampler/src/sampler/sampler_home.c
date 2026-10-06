/**
 * @file sampler_home.c
 * @brief Implementation of sampler home screen.
 */

/* clang-format off */
#include "sampler/sampler_home.h"
#include "sampler/sampler_top_app_bar.h"
#include "sampler/sampler_models.h"
#include "ui_card_base.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct sampler_home {
  struct ui_dom_node *root_node;
  struct ui_dom_node *top_bar_node;
  struct ui_dom_node *grid_node;
  struct md3_card **cards;
  size_t num_cards;
};

static sampler_error_t create_component_card(
    struct ui_engine *engine, const struct sampler_component *comp_model,
    struct md3_card **out_card, struct ui_dom_node **out_node) {
  struct md3_card *card = NULL;
  struct ui_card_base *card_base = NULL;
  struct ui_component *comp = NULL;
  struct ui_dom_node *card_root = NULL;
  ui_error_t u_rc;

  u_rc = md3_card_create(engine, MD3_CARD_OUTLINED, &card);
  if (u_rc != UI_ERROR_NONE) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  u_rc = md3_card_set_title(card, comp_model->name);
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

sampler_error_t sampler_home_create(struct ui_engine *engine,
                                    struct sampler_nav *nav,
                                    struct sampler_home **out_screen) {
  struct sampler_home *screen = NULL;
  struct sampler_top_app_bar_config top_bar_config;
  const struct sampler_component *const *components = NULL;
  size_t num_components = 0;
  size_t i;
  sampler_error_t rc;
  ui_error_t u_rc;

  if (engine == NULL || nav == NULL || out_screen == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  rc = sampler_catalog_get_components(&components, &num_components);
  if (rc != SAMPLER_SUCCESS) {
    return rc;
  }

  screen = (struct sampler_home *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct sampler_home));
  if (screen == NULL) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }
  memset(screen, 0, sizeof(*screen));

  /* 1. Create root container */
  u_rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &screen->root_node);
  if (u_rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(screen);
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  u_rc = ui_dom_node_set_tag_name(screen->root_node, "div");
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->root_node);
    C_MULTIPLATFORM_FREE(screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  u_rc = ui_dom_node_set_attribute(screen->root_node, "class",
                                   "m3-catalog-screen m3-home-screen");
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->root_node);
    C_MULTIPLATFORM_FREE(screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  /* 2. Top App Bar */
  memset(&top_bar_config, 0, sizeof(top_bar_config));
  top_bar_config.title = "Compose Material 3";
  top_bar_config.show_back_button = 0;
  top_bar_config.show_search_field = 1;
  top_bar_config.show_favorite_pin = 1;
  top_bar_config.is_favorite_pinned = 0;
  top_bar_config.show_theme_button = 1;
  top_bar_config.show_more_menu = 1;

  rc = sampler_top_app_bar_create(engine, &top_bar_config,
                                  &screen->top_bar_node);
  if (rc != SAMPLER_SUCCESS) {
    ui_dom_node_destroy(screen->root_node);
    C_MULTIPLATFORM_FREE(screen);
    return rc;
  }

  u_rc = ui_dom_node_append_child(screen->root_node, screen->top_bar_node);
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->root_node);
    ui_dom_node_destroy(screen->top_bar_node);
    C_MULTIPLATFORM_FREE(screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  /* 3. Grid: Adaptive vertical grid hosting md3_card */
  u_rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &screen->grid_node);
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->root_node);
    C_MULTIPLATFORM_FREE(screen);
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  u_rc = ui_dom_node_set_tag_name(screen->grid_node, "main");
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->grid_node);
    ui_dom_node_destroy(screen->root_node);
    C_MULTIPLATFORM_FREE(screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  u_rc = ui_dom_node_set_attribute(screen->grid_node, "class", "m3-home-grid");
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->grid_node);
    ui_dom_node_destroy(screen->root_node);
    C_MULTIPLATFORM_FREE(screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  u_rc = ui_dom_node_append_child(screen->root_node, screen->grid_node);
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->grid_node);
    ui_dom_node_destroy(screen->root_node);
    C_MULTIPLATFORM_FREE(screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  screen->num_cards = num_components;
  screen->cards = (struct md3_card **)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_card *) * num_components);

  if (screen->cards == NULL) {
    ui_dom_node_destroy(screen->root_node);
    C_MULTIPLATFORM_FREE(screen);
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  memset(screen->cards, 0, sizeof(struct md3_card *) * num_components);

  for (i = 0; i < num_components; i++) {
    struct md3_card *card = NULL;
    struct ui_dom_node *card_node = NULL;

    rc = create_component_card(engine, components[i], &card, &card_node);
    if (rc != SAMPLER_SUCCESS) {
      sampler_home_destroy(&screen);
      return rc;
    }

    screen->cards[i] = card;

    u_rc = ui_dom_node_append_child(screen->grid_node, card_node);
    if (u_rc != UI_ERROR_NONE) {
      sampler_home_destroy(&screen);
      return SAMPLER_ERROR_DOM_ATTACH_FAILED;
    }
  }

  *out_screen = screen;
  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_home_get_root(const struct sampler_home *screen,
                                      struct ui_dom_node **out_root) {
  if (screen == NULL || out_root == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  *out_root = screen->root_node;
  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_home_destroy(struct sampler_home **screen) {
  size_t i;
  if (screen == NULL || *screen == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  if ((*screen)->cards != NULL) {
    for (i = 0; i < (*screen)->num_cards; i++) {
      if ((*screen)->cards[i] != NULL) {
        md3_card_destroy((*screen)->cards[i]);
      }
    }
    C_MULTIPLATFORM_FREE((*screen)->cards);
  }

  /* Nodes managed by widgets or parent destroy. */
  C_MULTIPLATFORM_FREE(*screen);
  *screen = NULL;

  return SAMPLER_SUCCESS;
}
