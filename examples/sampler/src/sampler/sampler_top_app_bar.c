/**
 * @file sampler_top_app_bar.c
 * @brief Implementation of Top App Bar for Compose Material Catalog.
 */

/* clang-format off */
#include "sampler/sampler_top_app_bar.h"
#include "material3/md3_top_app_bar.h"
#include "material3/md3_icon_button.h"
#include "material3/md3_search.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "ui_engine.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

sampler_error_t
sampler_top_app_bar_create(struct ui_engine *engine,
                           const struct sampler_top_app_bar_config *config,
                           struct ui_dom_node **out_node) {
  struct md3_top_app_bar_config m3_cfg;
  struct md3_top_app_bar *m3_bar = NULL;
  struct ui_top_app_bar_base *base_bar = NULL;
  struct ui_component *base_comp = NULL;
  struct ui_dom_node *root = NULL;
  struct md3_icon_button *nav_btn = NULL;
  struct md3_icon_button *pin_btn = NULL;
  struct md3_icon_button *theme_btn = NULL;
  struct md3_icon_button *more_btn = NULL;
  struct md3_search_bar *search_bar = NULL;
  struct ui_search_bar_base *search_base = NULL;
  struct ui_component *search_comp = NULL;
  ui_error_t u_rc;

  if (engine == NULL || config == NULL || out_node == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  /* 1. Configure and create M3 Top App Bar */
  m3_cfg.variant = MD3_TOP_APP_BAR_SMALL;
  m3_cfg.scroll_behavior = MD3_TOP_APP_BAR_SCROLL_PINNED;
  m3_cfg.title = config->title;
  m3_cfg.subtitle = NULL;

  u_rc = md3_top_app_bar_create(engine, &m3_cfg, &m3_bar);
  if (u_rc != UI_ERROR_NONE) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  /* 2. Navigation Icon */
  if (config->show_back_button) {
    u_rc = md3_icon_button_create(engine, MD3_ICON_BUTTON_STANDARD,
                                  "arrow_back", &nav_btn);
    if (u_rc == UI_ERROR_NONE) {
      /* Assume on_back would be wired via md3_icon_button_set_on_click if
       * provided */
      u_rc = md3_top_app_bar_set_navigation_icon(m3_bar, nav_btn);
      if (u_rc != UI_ERROR_NONE) {
        md3_icon_button_destroy(nav_btn);
      }
    }
  }

  /* 3. Action Items */
  if (config->show_favorite_pin) {
    u_rc = md3_icon_button_create(
        engine, MD3_ICON_BUTTON_STANDARD,
        config->is_favorite_pinned ? "pin_drop" : "push_pin", &pin_btn);
    if (u_rc == UI_ERROR_NONE) {
      u_rc = md3_top_app_bar_add_action_item(m3_bar, pin_btn);
      if (u_rc != UI_ERROR_NONE) {
        md3_icon_button_destroy(pin_btn);
      }
    }
  }

  if (config->show_theme_button) {
    u_rc = md3_icon_button_create(engine, MD3_ICON_BUTTON_STANDARD, "palette",
                                  &theme_btn);
    if (u_rc == UI_ERROR_NONE) {
      u_rc = md3_top_app_bar_add_action_item(m3_bar, theme_btn);
      if (u_rc != UI_ERROR_NONE) {
        md3_icon_button_destroy(theme_btn);
      }
    }
  }

  if (config->show_more_menu) {
    u_rc = md3_icon_button_create(engine, MD3_ICON_BUTTON_STANDARD, "more_vert",
                                  &more_btn);
    if (u_rc == UI_ERROR_NONE) {
      u_rc = md3_top_app_bar_add_action_item(m3_bar, more_btn);
      if (u_rc != UI_ERROR_NONE) {
        md3_icon_button_destroy(more_btn);
      }
    }
  }

  /* 4. Extract base DOM node from Top App Bar */
  u_rc = md3_top_app_bar_get_base(m3_bar, &base_bar);
  if (u_rc != UI_ERROR_NONE) {
    md3_top_app_bar_destroy(m3_bar);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  u_rc = ui_top_app_bar_base_get_component(base_bar, &base_comp);
  if (u_rc != UI_ERROR_NONE) {
    md3_top_app_bar_destroy(m3_bar);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  if (base_comp != NULL) {
    root = base_comp->shadow_root;
  }
  if (root == NULL) {
    md3_top_app_bar_destroy(m3_bar);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  /* 5. Attach Search Field to root (as a child or sibling depending on catalog
     design) For now, append it to root node. */
  if (config->show_search_field) {
    u_rc =
        md3_search_bar_create(engine, "Search components", &search_bar, NULL);
    if (u_rc == UI_ERROR_NONE) {
      u_rc = md3_search_bar_get_base(search_bar, &search_base);
      if (u_rc == UI_ERROR_NONE) {
        u_rc = ui_search_bar_base_get_component(search_base, &search_comp);
        if (u_rc == UI_ERROR_NONE) {
          struct ui_dom_node *search_node = NULL;
          if (search_comp != NULL) {
            search_node = search_comp->shadow_root;
          }
          if (search_node != NULL) {
            u_rc = ui_dom_node_append_child(root, search_node);
            if (u_rc != UI_ERROR_NONE) {
              /* Ignore attach fail for search, or handle gracefully */
            }
          }
        }
      }
    }
  }

  *out_node = root;
  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_top_app_bar_set_pinned(struct ui_dom_node *top_bar_node,
                                               int is_pinned) {
  ui_error_t u_rc;

  if (top_bar_node == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  u_rc = ui_dom_node_set_attribute(top_bar_node, "data-pinned",
                                   is_pinned ? "1" : "0");
  if (u_rc != UI_ERROR_NONE) {
    return SAMPLER_ERROR_STYLE_PROP_FAILED;
  }

  return SAMPLER_SUCCESS;
}
