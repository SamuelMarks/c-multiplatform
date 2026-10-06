/**
 * @file m2_home.c
 * @brief Implementation of Material 2 Catalog Home Screen.
 */

/* clang-format off */
#include "sampler/m2/m2_home.h"
#include "material2/md2_top_app_bar.h"
#include "material2/md2_card.h"
#include "sampler/sampler_models.h"
#include "ui_card_base.h"
#include "ui_top_app_bar_base.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct m2_home_screen {
  struct ui_dom_node *root_node;
  struct md2_top_app_bar *top_bar;
  struct ui_dom_node *grid_node;
  struct md2_card **cards;
  size_t num_cards;
};

static sampler_error_t create_m2_component_card(
    struct ui_engine *engine, const struct sampler_component *comp_model,
    struct md2_card **out_card, struct ui_dom_node **out_node) {
  struct md2_card *card = NULL;
  struct ui_card_base *card_base = NULL;
  struct ui_component *comp = NULL;
  struct ui_dom_node *card_root = NULL;
  ui_error_t u_rc;

  u_rc = md2_card_create(
      engine, 0 /* MD2_CARD_ELEVATED or OUTLINED, assuming 0 is elevated */,
      &card);
  if (u_rc != UI_ERROR_NONE) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  u_rc = md2_card_set_title(card, comp_model->name);
  if (u_rc != UI_ERROR_NONE) {
    md2_card_destroy(card);
    return SAMPLER_ERROR_STYLE_PROP_FAILED;
  }

  u_rc = md2_card_get_base(card, &card_base);
  if (u_rc == UI_ERROR_NONE && card_base != NULL) {
    u_rc = ui_card_base_get_component(card_base, &comp);
    if (u_rc == UI_ERROR_NONE && comp != NULL) {
      card_root = comp->shadow_root;
    }
  }

  if (card_root == NULL) {
    md2_card_destroy(card);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  *out_card = card;
  *out_node = card_root;
  return SAMPLER_SUCCESS;
}

sampler_error_t m2_home_create(struct ui_engine *engine,
                               struct sampler_nav *nav,
                               struct m2_home_screen **out_screen) {
  struct m2_home_screen *screen = NULL;
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

  screen = (struct m2_home_screen *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct m2_home_screen));
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
                                   "m2-catalog-screen m2-home-screen");
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->root_node);
    C_MULTIPLATFORM_FREE(screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  /* 2. M2 Top App Bar */
  u_rc = md2_top_app_bar_create(engine, 0 /* variant */, "Material Design",
                                &screen->top_bar);
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->root_node);
    C_MULTIPLATFORM_FREE(screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  {
    struct ui_top_app_bar_base *base_bar = NULL;
    struct ui_component *base_comp = NULL;
    struct ui_dom_node *top_bar_elem = NULL;

    u_rc = md2_top_app_bar_get_base(screen->top_bar, &base_bar);
    if (u_rc == UI_ERROR_NONE && base_bar != NULL) {
      u_rc = ui_top_app_bar_base_get_component(base_bar, &base_comp);
      if (u_rc == UI_ERROR_NONE && base_comp != NULL) {
        top_bar_elem = base_comp->shadow_root;
      }
    }

    if (top_bar_elem != NULL) {
      u_rc = ui_dom_node_append_child(screen->root_node, top_bar_elem);
      if (u_rc != UI_ERROR_NONE) {
        m2_home_destroy(&screen);
        return SAMPLER_ERROR_DOM_ATTACH_FAILED;
      }
    } else {
      m2_home_destroy(&screen);
      return SAMPLER_ERROR_DOM_ATTACH_FAILED;
    }
  }

  /* 3. Grid: Adaptive vertical grid hosting md2_card */
  u_rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &screen->grid_node);
  if (u_rc != UI_ERROR_NONE) {
    m2_home_destroy(&screen);
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  u_rc = ui_dom_node_set_tag_name(screen->grid_node, "main");
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->grid_node);
    m2_home_destroy(&screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  u_rc = ui_dom_node_set_attribute(screen->grid_node, "class", "m2-home-grid");
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->grid_node);
    m2_home_destroy(&screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  u_rc = ui_dom_node_append_child(screen->root_node, screen->grid_node);
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->grid_node);
    m2_home_destroy(&screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  screen->num_cards = num_components;
  screen->cards = (struct md2_card **)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md2_card *) * num_components);

  if (screen->cards == NULL) {
    m2_home_destroy(&screen);
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  memset(screen->cards, 0, sizeof(struct md2_card *) * num_components);

  for (i = 0; i < num_components; i++) {
    struct md2_card *card = NULL;
    struct ui_dom_node *card_node = NULL;

    rc = create_m2_component_card(engine, components[i], &card, &card_node);
    if (rc != SAMPLER_SUCCESS) {
      m2_home_destroy(&screen);
      return rc;
    }

    screen->cards[i] = card;

    u_rc = ui_dom_node_append_child(screen->grid_node, card_node);
    if (u_rc != UI_ERROR_NONE) {
      m2_home_destroy(&screen);
      return SAMPLER_ERROR_DOM_ATTACH_FAILED;
    }
  }

  *out_screen = screen;
  return SAMPLER_SUCCESS;
}

sampler_error_t m2_home_get_root(const struct m2_home_screen *screen,
                                 struct ui_dom_node **out_root) {
  if (screen == NULL || out_root == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  *out_root = screen->root_node;
  return SAMPLER_SUCCESS;
}

sampler_error_t m2_home_destroy(struct m2_home_screen **screen) {
  size_t i;
  if (screen == NULL || *screen == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  if ((*screen)->cards != NULL) {
    for (i = 0; i < (*screen)->num_cards; i++) {
      if ((*screen)->cards[i] != NULL) {
        md2_card_destroy((*screen)->cards[i]);
      }
    }
    C_MULTIPLATFORM_FREE((*screen)->cards);
  }

  if ((*screen)->top_bar != NULL) {
    md2_top_app_bar_destroy((*screen)->top_bar);
  }

  /* Nodes managed by widgets or parent destroy. */
  C_MULTIPLATFORM_FREE(*screen);
  *screen = NULL;

  return SAMPLER_SUCCESS;
}
