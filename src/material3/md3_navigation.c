/**
 * @file md3_navigation.c
 * @brief Material 3 Navigation components implementation.
 */

/* clang-format off */
#include "material3/md3_navigation.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/* =========================================================================
 * MD3 Navigation Bar
 * ========================================================================= */

ui_error_t md3_navigation_bar_create(struct ui_engine *engine,
                                     struct md3_navigation_bar **out_bar) {
  struct md3_navigation_bar *bar;
  ui_error_t rc;

  if (!engine || !out_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct md3_navigation_bar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_navigation_bar));
  if (!bar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(bar, 0, sizeof(struct md3_navigation_bar));

  rc = ui_bottom_nav_base_create(&bar->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  *out_bar = bar;
  return UI_ERROR_NONE;
}

ui_error_t md3_navigation_bar_destroy(struct md3_navigation_bar *bar) {
  ui_error_t rc;

  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_bottom_nav_base_destroy(bar->base);
  C_MULTIPLATFORM_FREE(bar);
  return rc;
}

ui_error_t md3_navigation_bar_add_item(struct md3_navigation_bar *bar,
                                       const char *label,
                                       const char *icon_name) {
  struct ui_bottom_nav_item_base *item;
  struct ui_component *comp;
  ui_error_t rc;

  if (!bar || !label) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_bottom_nav_item_base_create(&item);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_bottom_nav_item_base_get_component(item, &comp);
  rc = ui_component_set_property(comp, "--md-nav-item-label", label);
  if (rc != UI_ERROR_NONE) {
    ui_bottom_nav_item_base_destroy(item);
    return rc;
  }

  if (icon_name != NULL) {
    rc = ui_component_set_property(comp, "--md-nav-item-icon", icon_name);
    if (rc != UI_ERROR_NONE) {
      ui_bottom_nav_item_base_destroy(item);
      return rc;
    }
  }

  rc = ui_bottom_nav_base_append_item(bar->base, item);
  return rc;
}

ui_error_t md3_navigation_bar_set_selected(struct md3_navigation_bar *bar,
                                           size_t index) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->selected_index = index;
  return UI_ERROR_NONE;
}

ui_error_t md3_navigation_bar_get_selected(const struct md3_navigation_bar *bar,
                                           size_t *out_index) {
  if (!bar || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_index = bar->selected_index;
  return UI_ERROR_NONE;
}

/* =========================================================================
 * MD3 Navigation Rail
 * ========================================================================= */

ui_error_t md3_navigation_rail_create(struct ui_engine *engine,
                                      struct md3_navigation_rail **out_rail) {
  struct md3_navigation_rail *rail;
  ui_error_t rc;

  if (!engine || !out_rail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rail = (struct md3_navigation_rail *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_navigation_rail));
  if (!rail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(rail, 0, sizeof(struct md3_navigation_rail));

  rc = ui_nav_rail_base_create(&rail->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(rail);
    return rc;
  }

  *out_rail = rail;
  return UI_ERROR_NONE;
}

ui_error_t md3_navigation_rail_destroy(struct md3_navigation_rail *rail) {
  ui_error_t rc;

  if (!rail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_nav_rail_base_destroy(rail->base);
  C_MULTIPLATFORM_FREE(rail);
  return rc;
}

ui_error_t md3_navigation_rail_set_expanded(struct md3_navigation_rail *rail,
                                            int expanded) {
  if (!rail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rail->expanded = expanded ? 1 : 0;
  return UI_ERROR_NONE;
}

/* =========================================================================
 * MD3 Navigation Drawer
 * ========================================================================= */

ui_error_t
md3_navigation_drawer_create(struct ui_engine *engine,
                             enum md3_navigation_drawer_variant variant,
                             struct md3_navigation_drawer **out_drawer) {
  struct md3_navigation_drawer *drawer;
  ui_error_t rc;

  if (!engine || !out_drawer ||
      (unsigned)variant > (unsigned)MD3_DRAWER_MODAL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  drawer = (struct md3_navigation_drawer *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_navigation_drawer));
  if (!drawer) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(drawer, 0, sizeof(struct md3_navigation_drawer));
  drawer->variant = variant;

  rc = ui_sidenav_base_create(&drawer->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(drawer);
    return rc;
  }

  *out_drawer = drawer;
  return UI_ERROR_NONE;
}

ui_error_t md3_navigation_drawer_destroy(struct md3_navigation_drawer *drawer) {
  ui_error_t rc;

  if (!drawer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_sidenav_base_destroy(drawer->base);
  C_MULTIPLATFORM_FREE(drawer);
  return rc;
}

ui_error_t md3_navigation_drawer_set_open(struct md3_navigation_drawer *drawer,
                                          int is_open) {
  if (!drawer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  drawer->is_open = is_open ? 1 : 0;
  return ui_sidenav_base_set_open(drawer->base, drawer->is_open);
}

/* =========================================================================
 * MD3 Navigation Suite (Adaptive navigation)
 * ========================================================================= */

ui_error_t
md3_navigation_suite_create(struct ui_engine *engine,
                            struct md3_navigation_suite **out_suite) {
  struct md3_navigation_suite *suite;
  ui_error_t rc;

  if (!engine || !out_suite) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  suite = (struct md3_navigation_suite *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_navigation_suite));
  if (!suite) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(suite, 0, sizeof(struct md3_navigation_suite));
  suite->current_class = MD3_WINDOW_SIZE_COMPACT;

  rc = md3_navigation_bar_create(engine, &suite->bar);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(suite);
    return rc;
  }

  rc = md3_navigation_rail_create(engine, &suite->rail);
  if (rc != UI_ERROR_NONE) {
    md3_navigation_bar_destroy(suite->bar);
    C_MULTIPLATFORM_FREE(suite);
    return rc;
  }

  rc =
      md3_navigation_drawer_create(engine, MD3_DRAWER_STANDARD, &suite->drawer);
  if (rc != UI_ERROR_NONE) {
    md3_navigation_rail_destroy(suite->rail);
    md3_navigation_bar_destroy(suite->bar);
    C_MULTIPLATFORM_FREE(suite);
    return rc;
  }

  *out_suite = suite;
  return UI_ERROR_NONE;
}

ui_error_t md3_navigation_suite_destroy(struct md3_navigation_suite *suite) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!suite) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = md3_navigation_drawer_destroy(suite->drawer);
  rc = md3_navigation_rail_destroy(suite->rail);
  rc = md3_navigation_bar_destroy(suite->bar);

  C_MULTIPLATFORM_FREE(suite);
  return rc;
}

ui_error_t
md3_navigation_suite_update_size_class(struct md3_navigation_suite *suite,
                                       enum md3_window_size_class size_class) {
  if (!suite || (unsigned)size_class > (unsigned)MD3_WINDOW_SIZE_EXPANDED) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  suite->current_class = size_class;
  return UI_ERROR_NONE;
}

/* =========================================================================
 * MD3 Top App Bar
 * ========================================================================= */

ui_error_t md3_top_app_bar_create(struct ui_engine *engine,
                                  enum md3_top_app_bar_variant variant,
                                  const char *title,
                                  struct md3_top_app_bar **out_bar) {
  struct md3_top_app_bar *bar;
  struct ui_top_app_bar_config config;
  ui_error_t rc;

  if (!engine || !out_bar ||
      (unsigned)variant > (unsigned)MD3_TOP_APP_BAR_LARGE) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct md3_top_app_bar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_top_app_bar));
  if (!bar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(bar, 0, sizeof(struct md3_top_app_bar));
  bar->variant = variant;

  rc = ui_arena_create(4096, &bar->arena);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  if (title) {
#if defined(_MSC_VER)
    strncpy_s(bar->title, sizeof(bar->title), title, sizeof(bar->title) - 1);
#else
    strncpy(bar->title, title, sizeof(bar->title) - 1);
    bar->title[sizeof(bar->title) - 1] = '\0';
#endif
  }

  memset(&config, 0, sizeof(config));
  config.initial_state = UI_TOP_APP_BAR_STATE_EXPANDED;
  if (variant == MD3_TOP_APP_BAR_LARGE) {
    config.expanded_height = 152.0f;
  } else if (variant == MD3_TOP_APP_BAR_MEDIUM) {
    config.expanded_height = 112.0f;
  } else {
    config.expanded_height = 64.0f;
  }
  config.collapsed_height = 64.0f;
  config.scroll_threshold = 100.0f;

  rc = ui_top_app_bar_base_create(bar->arena, &config, &bar->base);
  if (rc != UI_ERROR_NONE) {
    ui_arena_destroy(bar->arena);
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  *out_bar = bar;
  return UI_ERROR_NONE;
}

ui_error_t md3_top_app_bar_destroy(struct md3_top_app_bar *bar) {
  ui_error_t rc;

  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_top_app_bar_base_destroy(bar->base);
  ui_arena_destroy(bar->arena);

  C_MULTIPLATFORM_FREE(bar);
  return rc;
}

ui_error_t md3_top_app_bar_set_scroll_offset(struct md3_top_app_bar *bar,
                                             float scroll_offset) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->scroll_offset = scroll_offset;
  return ui_top_app_bar_base_handle_scroll(bar->base, scroll_offset,
                                           scroll_offset - bar->scroll_offset);
}

/* =========================================================================
 * MD3 Bottom App Bar
 * ========================================================================= */

ui_error_t md3_bottom_app_bar_create(struct ui_engine *engine, int has_fab,
                                     struct md3_bottom_app_bar **out_bar) {
  struct md3_bottom_app_bar *bar;
  ui_error_t rc;

  if (!engine || !out_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct md3_bottom_app_bar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_bottom_app_bar));
  if (!bar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(bar, 0, sizeof(struct md3_bottom_app_bar));
  bar->has_fab = has_fab ? 1 : 0;

  rc = ui_bottom_app_bar_base_create(&bar->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  *out_bar = bar;
  return UI_ERROR_NONE;
}

ui_error_t md3_bottom_app_bar_destroy(struct md3_bottom_app_bar *bar) {
  ui_error_t rc;

  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_bottom_app_bar_base_destroy(bar->base);
  C_MULTIPLATFORM_FREE(bar);
  return rc;
}

/* =========================================================================
 * MD3 Tabs
 * ========================================================================= */

ui_error_t md3_tabs_create(struct ui_engine *engine,
                           enum md3_tabs_variant variant,
                           struct md3_tabs **out_tabs) {
  struct md3_tabs *tabs;
  ui_error_t rc;

  if (!engine || !out_tabs ||
      (unsigned)variant > (unsigned)MD3_TABS_EXPRESSIVE_PILL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tabs = (struct md3_tabs *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_tabs));
  if (!tabs) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tabs, 0, sizeof(struct md3_tabs));
  tabs->variant = variant;

  rc = ui_tabs_base_create(&tabs->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tabs);
    return rc;
  }

  *out_tabs = tabs;
  return UI_ERROR_NONE;
}

ui_error_t md3_tabs_destroy(struct md3_tabs *tabs) {
  ui_error_t rc;

  if (!tabs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_tabs_base_destroy(tabs->base);
  C_MULTIPLATFORM_FREE(tabs);
  return rc;
}

ui_error_t md3_tabs_set_selected(struct md3_tabs *tabs, size_t index) {
  if (!tabs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tabs->selected_tab = index;
  return ui_tabs_base_set_active_index(tabs->base, (int)index);
}

ui_error_t md3_tabs_get_selected(const struct md3_tabs *tabs,
                                 size_t *out_index) {
  if (!tabs || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_index = tabs->selected_tab;
  return UI_ERROR_NONE;
}
