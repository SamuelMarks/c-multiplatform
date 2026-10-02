/**
 * @file cupertino_tab_bar.c
 * @brief Cupertino Tab Bar component implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_tab_bar.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_tab_bar_mock_base_create_fail = 0;
int g_cupertino_tab_bar_mock_base_destroy_fail = 0;
int g_cupertino_tab_bar_mock_append_fail = 0;
int g_cupertino_tab_bar_mock_item_create_fail = 0;
int g_cupertino_tab_bar_mock_item_destroy_fail = 0;
int g_cupertino_tab_bar_mock_set_active_fail = 0;

static ui_error_t
mock_ui_bottom_nav_base_create(struct ui_bottom_nav_base **out_nav) {
  if (g_cupertino_tab_bar_mock_base_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_bottom_nav_base_create(out_nav);
}
#undef ui_bottom_nav_base_create
/** @cond */
#define ui_bottom_nav_base_create mock_ui_bottom_nav_base_create
/** @endcond */

static ui_error_t
mock_ui_bottom_nav_base_destroy(struct ui_bottom_nav_base *nav) {
  if (g_cupertino_tab_bar_mock_base_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_bottom_nav_base_destroy(nav);
}
#undef ui_bottom_nav_base_destroy
/** @cond */
#define ui_bottom_nav_base_destroy mock_ui_bottom_nav_base_destroy
/** @endcond */

static ui_error_t
mock_ui_bottom_nav_base_append_item(struct ui_bottom_nav_base *nav,
                                    struct ui_bottom_nav_item_base *item) {
  if (g_cupertino_tab_bar_mock_append_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_bottom_nav_base_append_item(nav, item);
}
#undef ui_bottom_nav_base_append_item
/** @cond */
#define ui_bottom_nav_base_append_item mock_ui_bottom_nav_base_append_item
/** @endcond */

static ui_error_t
mock_ui_bottom_nav_item_base_create(struct ui_bottom_nav_item_base **out_item) {
  if (g_cupertino_tab_bar_mock_item_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_bottom_nav_item_base_create(out_item);
}
#undef ui_bottom_nav_item_base_create
/** @cond */
#define ui_bottom_nav_item_base_create mock_ui_bottom_nav_item_base_create
/** @endcond */

static ui_error_t
mock_ui_bottom_nav_item_base_destroy(struct ui_bottom_nav_item_base *item) {
  if (g_cupertino_tab_bar_mock_item_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_bottom_nav_item_base_destroy(item);
}
#undef ui_bottom_nav_item_base_destroy
/** @cond */
#define ui_bottom_nav_item_base_destroy mock_ui_bottom_nav_item_base_destroy
/** @endcond */

static ui_error_t
mock_ui_bottom_nav_item_base_set_active(struct ui_bottom_nav_item_base *item,
                                        int active) {
  if (g_cupertino_tab_bar_mock_set_active_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_bottom_nav_item_base_set_active(item, active);
}
#undef ui_bottom_nav_item_base_set_active
/** @cond */
#define ui_bottom_nav_item_base_set_active                                     \
  mock_ui_bottom_nav_item_base_set_active
/** @endcond */
#endif

#define CUPERTINO_TAB_BAR_HEIGHT_STANDARD 49.0f
#define CUPERTINO_TAB_BAR_HEIGHT_HOME_INDICATOR 83.0f

ui_error_t
cupertino_tab_bar_create(struct ui_engine *engine,
                         const struct cupertino_tab_bar_descriptor *desc,
                         struct cupertino_tab_bar **out_tab_bar) {
  struct cupertino_tab_bar *tab_bar;
  ui_error_t rc;
  size_t i;

  if (!engine || !desc || !out_tab_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (desc->item_count > CUPERTINO_TAB_BAR_MAX_ITEMS) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tab_bar = (struct cupertino_tab_bar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_tab_bar));
  if (!tab_bar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tab_bar, 0, sizeof(*tab_bar));
  tab_bar->is_home_indicator_present = desc->is_home_indicator_present ? 1 : 0;
  tab_bar->selected_index = desc->initial_index;
  tab_bar->adaptive_sidebar_enabled = 0;
  tab_bar->sidebar_breakpoint = 768.0f;
  tab_bar->sidebar_width = 260.0f;
  tab_bar->is_sidebar_mode = 0;

  rc = ui_bottom_nav_base_create(&tab_bar->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tab_bar);
    *out_tab_bar = NULL;
    return rc;
  }

  for (i = 0; i < desc->item_count; i++) {
    size_t idx;
    rc = cupertino_tab_bar_add_item(tab_bar, &desc->items[i], &idx);
    if (rc != UI_ERROR_NONE) {
      cupertino_tab_bar_destroy(tab_bar);
      *out_tab_bar = NULL;
      return rc;
    }
  }

  *out_tab_bar = tab_bar;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_bar_destroy(struct cupertino_tab_bar *tab_bar) {
  ui_error_t rc;
  size_t i;

  if (!tab_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < tab_bar->item_count; i++) {
    if (tab_bar->items[i].base_item) {
      rc = ui_bottom_nav_item_base_destroy(tab_bar->items[i].base_item);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      tab_bar->items[i].base_item = NULL;
    }
  }

  if (tab_bar->base) {
    rc = ui_bottom_nav_base_destroy(tab_bar->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    tab_bar->base = NULL;
  }

  C_MULTIPLATFORM_FREE(tab_bar);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_bar_add_item(
    struct cupertino_tab_bar *tab_bar,
    const struct cupertino_tab_item_descriptor *item_desc, size_t *out_index) {
  struct cupertino_tab_item *item;
  ui_error_t rc;
  size_t idx;

  if (!tab_bar || !item_desc) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tab_bar->item_count >= CUPERTINO_TAB_BAR_MAX_ITEMS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = tab_bar->item_count;
  item = &tab_bar->items[idx];
  memset(item, 0, sizeof(*item));

  if (item_desc->label) {
#if defined(_MSC_VER)
    strncpy_s(item->label, sizeof(item->label), item_desc->label,
              sizeof(item->label) - 1);
#else
    strncpy(item->label, item_desc->label, sizeof(item->label) - 1);
    item->label[sizeof(item->label) - 1] = '\0';
#endif
  }

  if (item_desc->icon_name) {
#if defined(_MSC_VER)
    strncpy_s(item->icon_name, sizeof(item->icon_name), item_desc->icon_name,
              sizeof(item->icon_name) - 1);
#else
    strncpy(item->icon_name, item_desc->icon_name, sizeof(item->icon_name) - 1);
    item->icon_name[sizeof(item->icon_name) - 1] = '\0';
#endif
  }

  if (item_desc->badge_text) {
#if defined(_MSC_VER)
    strncpy_s(item->badge, sizeof(item->badge), item_desc->badge_text,
              sizeof(item->badge) - 1);
#else
    strncpy(item->badge, item_desc->badge_text, sizeof(item->badge) - 1);
    item->badge[sizeof(item->badge) - 1] = '\0';
#endif
    item->has_badge = 1;
  }

  rc = ui_bottom_nav_item_base_create(&item->base_item);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_bottom_nav_base_append_item(tab_bar->base, item->base_item);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_bottom_nav_item_base_destroy(item->base_item);
    item->base_item = NULL;
    return (destroy_rc != UI_ERROR_NONE) ? destroy_rc : rc;
  }

  rc = ui_bottom_nav_item_base_set_active(
      item->base_item, (idx == tab_bar->selected_index) ? 1 : 0);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_bottom_nav_item_base_destroy(item->base_item);
    item->base_item = NULL;
    return (destroy_rc != UI_ERROR_NONE) ? destroy_rc : rc;
  }

  tab_bar->item_count++;
  if (out_index) {
    *out_index = idx;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_bar_set_selected(struct cupertino_tab_bar *tab_bar,
                                          size_t index) {
  ui_error_t rc;
  size_t i;

  if (!tab_bar || index >= tab_bar->item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tab_bar->selected_index = index;

  for (i = 0; i < tab_bar->item_count; i++) {
    if (tab_bar->items[i].base_item) {
      rc = ui_bottom_nav_item_base_set_active(tab_bar->items[i].base_item,
                                              (i == index) ? 1 : 0);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_bar_get_selected(const struct cupertino_tab_bar *tab_bar,
                               size_t *out_index) {
  if (!tab_bar || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_index = tab_bar->selected_index;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_bar_set_badge(struct cupertino_tab_bar *tab_bar,
                                       size_t index, const char *badge_text) {
  if (!tab_bar || index >= tab_bar->item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (badge_text && badge_text[0] != '\0') {
#if defined(_MSC_VER)
    strncpy_s(tab_bar->items[index].badge, sizeof(tab_bar->items[index].badge),
              badge_text, sizeof(tab_bar->items[index].badge) - 1);
#else
    strncpy(tab_bar->items[index].badge, badge_text,
            sizeof(tab_bar->items[index].badge) - 1);
    tab_bar->items[index].badge[sizeof(tab_bar->items[index].badge) - 1] = '\0';
#endif
    tab_bar->items[index].has_badge = 1;
  } else {
    tab_bar->items[index].badge[0] = '\0';
    tab_bar->items[index].has_badge = 0;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_bar_get_badge(const struct cupertino_tab_bar *tab_bar,
                                       size_t index,
                                       const char **out_badge_text) {
  if (!tab_bar || index >= tab_bar->item_count || !out_badge_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_badge_text =
      tab_bar->items[index].has_badge ? tab_bar->items[index].badge : NULL;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_bar_set_home_indicator_present(struct cupertino_tab_bar *tab_bar,
                                             int present) {
  if (!tab_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tab_bar->is_home_indicator_present = present ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_bar_is_home_indicator_present(
    const struct cupertino_tab_bar *tab_bar, int *out_present) {
  if (!tab_bar || !out_present) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_present = tab_bar->is_home_indicator_present;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_bar_get_height(const struct cupertino_tab_bar *tab_bar,
                                        float *out_height) {
  if (!tab_bar || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_height = tab_bar->is_home_indicator_present
                    ? CUPERTINO_TAB_BAR_HEIGHT_HOME_INDICATOR
                    : CUPERTINO_TAB_BAR_HEIGHT_STANDARD;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_bar_get_item_count(const struct cupertino_tab_bar *tab_bar,
                                 size_t *out_count) {
  if (!tab_bar || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = tab_bar->item_count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_bar_get_base(struct cupertino_tab_bar *tab_bar,
                                      struct ui_bottom_nav_base **out_base) {
  if (!tab_bar || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = tab_bar->base;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_bar_set_adaptive_sidebar(struct cupertino_tab_bar *tab_bar,
                                       int enabled, float breakpoint,
                                       float sidebar_width) {
  if (!tab_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tab_bar->adaptive_sidebar_enabled = enabled ? 1 : 0;
  tab_bar->sidebar_breakpoint = (breakpoint > 0.0f) ? breakpoint : 768.0f;
  tab_bar->sidebar_width = (sidebar_width > 0.0f) ? sidebar_width : 260.0f;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_bar_get_adaptive_sidebar(const struct cupertino_tab_bar *tab_bar,
                                       int *out_enabled, float *out_breakpoint,
                                       float *out_sidebar_width) {
  if (!tab_bar || !out_enabled || !out_breakpoint || !out_sidebar_width) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_enabled = tab_bar->adaptive_sidebar_enabled;
  *out_breakpoint = tab_bar->sidebar_breakpoint;
  *out_sidebar_width = tab_bar->sidebar_width;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_bar_update_layout_for_width(struct cupertino_tab_bar *tab_bar,
                                          float viewport_width,
                                          int *out_is_sidebar) {
  if (!tab_bar || viewport_width <= 0.0f || !out_is_sidebar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tab_bar->adaptive_sidebar_enabled &&
      viewport_width >= tab_bar->sidebar_breakpoint) {
    tab_bar->is_sidebar_mode = 1;
  } else {
    tab_bar->is_sidebar_mode = 0;
  }

  *out_is_sidebar = tab_bar->is_sidebar_mode;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_tab_bar_is_in_sidebar_mode(const struct cupertino_tab_bar *tab_bar,
                                     int *out_is_sidebar) {
  if (!tab_bar || !out_is_sidebar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_sidebar = tab_bar->is_sidebar_mode;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_tab_bar_get_layout_bounds(
    const struct cupertino_tab_bar *tab_bar, float viewport_w, float viewport_h,
    float *out_x, float *out_y, float *out_w, float *out_h) {
  float h;

  if (!tab_bar || viewport_w <= 0.0f || viewport_h <= 0.0f || !out_x ||
      !out_y || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tab_bar->is_sidebar_mode) {
    *out_x = 0.0f;
    *out_y = 0.0f;
    *out_w = tab_bar->sidebar_width;
    *out_h = viewport_h;
  } else {
    h = tab_bar->is_home_indicator_present
            ? CUPERTINO_TAB_BAR_HEIGHT_HOME_INDICATOR
            : CUPERTINO_TAB_BAR_HEIGHT_STANDARD;
    *out_x = 0.0f;
    *out_y = viewport_h - h;
    *out_w = viewport_w;
    *out_h = h;
  }

  return UI_ERROR_NONE;
}
