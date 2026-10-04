/**
 * @file sampler_example_screen.c
 * @brief Implementation of interactive example canvas screen.
 */

/* clang-format off */
#include "sampler/sampler_example_screen.h"
#include "sampler/sampler_top_app_bar.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct sampler_example_screen {
  struct ui_dom_node *root_node;
  struct ui_dom_node *top_bar_node;
  struct ui_dom_node *canvas_node;
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

sampler_error_t
sampler_example_screen_create(struct ui_engine *engine, struct sampler_nav *nav,
                              const struct sampler_example *example,
                              struct sampler_example_screen **out_screen) {
  struct sampler_example_screen *screen = NULL;
  struct sampler_top_app_bar_config top_bar_config;
  sampler_error_t rc;
  ui_error_t u_rc;

  if (engine == NULL || nav == NULL || example == NULL || out_screen == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  screen = (struct sampler_example_screen *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct sampler_example_screen));
  if (screen == NULL) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }
  memset(screen, 0, sizeof(*screen));

  /* 1. Create root screen container */
  rc = sampler_create_element_with_class(
      "div", "m3-catalog-screen m3-example-screen", &screen->root_node);
  if (rc != SAMPLER_SUCCESS) {
    C_MULTIPLATFORM_FREE(screen);
    return rc;
  }

  /* 2. Instantiate Material 3 Top App Bar */
  memset(&top_bar_config, 0, sizeof(top_bar_config));
  top_bar_config.title = example->name;
  top_bar_config.show_back_button = 1;
  top_bar_config.show_search_field = 0;
  top_bar_config.show_favorite_pin = 0;
  top_bar_config.show_theme_button = 0;
  top_bar_config.show_more_menu = 1; /* For GitHub link maybe */

  rc = sampler_top_app_bar_create(engine, &top_bar_config,
                                  &screen->top_bar_node);
  if (rc != SAMPLER_SUCCESS) {
    sampler_example_screen_destroy(&screen);
    return rc;
  }

  u_rc = ui_dom_node_append_child(screen->root_node, screen->top_bar_node);
  if (u_rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(screen->top_bar_node);
    sampler_example_screen_destroy(&screen);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  /* 3. Create canvas area */
  rc = sampler_create_element_with_class("main", "m3-example-canvas",
                                         &screen->canvas_node);
  if (rc != SAMPLER_SUCCESS) {
    sampler_example_screen_destroy(&screen);
    return rc;
  }

  rc = sampler_append_child_checked(screen->root_node, screen->canvas_node);
  if (rc != SAMPLER_SUCCESS) {
    sampler_example_screen_destroy(&screen);
    return rc;
  }

  /* 4. Invoke example content callback */
  if (example->content != NULL) {
    rc = example->content(engine, screen->canvas_node);
    if (rc != SAMPLER_SUCCESS) {
      sampler_example_screen_destroy(&screen);
      return rc;
    }
  }

  *out_screen = screen;
  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_example_screen_get_root(const struct sampler_example_screen *screen,
                                struct ui_dom_node **out_root) {
  if (screen == NULL || out_root == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  *out_root = screen->root_node;
  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_example_screen_destroy(struct sampler_example_screen **screen) {
  if (screen == NULL || *screen == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  /* Nodes managed by widgets or parent destroy. */
  C_MULTIPLATFORM_FREE(*screen);
  *screen = NULL;

  return SAMPLER_SUCCESS;
}
