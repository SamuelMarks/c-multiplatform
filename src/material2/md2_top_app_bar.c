/**
 * @file md2_top_app_bar.c
 * @brief Implementation of Material Design 2 Top App Bar component.
 */

/* clang-format off */
#include "material2/md2_top_app_bar.h"
#include "ui_internal_mem.h"
#include "ui_component.h"
#include "ui_arena.h"
#include <string.h>
/* clang-format on */

struct md2_top_app_bar {
  struct ui_component *component;
  struct ui_top_app_bar_base *base;
  struct ui_arena *arena;
  enum md2_top_app_bar_variant variant;
  char *title;
};

ui_error_t md2_top_app_bar_create(struct ui_engine *engine,
                                  enum md2_top_app_bar_variant variant,
                                  const char *title,
                                  struct md2_top_app_bar **out_bar) {
  struct md2_top_app_bar *bar;
  struct ui_top_app_bar_config config;
  ui_error_t rc;
  size_t len;

  if (!engine || !out_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct md2_top_app_bar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md2_top_app_bar));
  if (!bar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(bar, 0, sizeof(*bar));

  bar->variant = variant;

  rc = ui_component_create(&bar->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  rc = ui_arena_create(4096, &bar->arena);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(bar->component);
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  config.initial_state = UI_TOP_APP_BAR_STATE_EXPANDED;
  config.scroll_threshold = 50.0f;
  if (variant == MD2_TOP_APP_BAR_PROMINENT) {
    config.expanded_height = 128.0f;
    config.collapsed_height = 56.0f;
  } else {
    config.expanded_height = 56.0f;
    config.collapsed_height = 56.0f;
  }

  rc = ui_top_app_bar_base_create(bar->arena, &config, &bar->base);
  if (rc != UI_ERROR_NONE) {
    ui_arena_destroy(bar->arena);
    ui_component_destroy(bar->component);
    C_MULTIPLATFORM_FREE(bar);
    return rc;
  }

  if (title) {
    len = strlen(title);
    bar->title = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
    if (!bar->title) {
      ui_top_app_bar_base_destroy(bar->base);
      ui_arena_destroy(bar->arena);
      ui_component_destroy(bar->component);
      C_MULTIPLATFORM_FREE(bar);
      return UI_ERROR_OUT_OF_MEMORY;
    }
#if defined(_MSC_VER)
    strcpy_s(bar->title, len + 1, title);
#else
    strcpy(bar->title, title);
#endif
  }

  *out_bar = bar;
  return UI_ERROR_NONE;
}

ui_error_t md2_top_app_bar_destroy(struct md2_top_app_bar *bar) {
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t temp_rc;

  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (bar->title) {
    C_MULTIPLATFORM_FREE(bar->title);
  }

  if (bar->base) {
    temp_rc = ui_top_app_bar_base_destroy(bar->base);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }

  if (bar->arena) {
    temp_rc = ui_arena_destroy(bar->arena);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }

  if (bar->component) {
    temp_rc = ui_component_destroy(bar->component);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }

  C_MULTIPLATFORM_FREE(bar);
  return rc;
}

ui_error_t md2_top_app_bar_set_title(struct md2_top_app_bar *bar,
                                     const char *title) {
  size_t len;

  if (!bar || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (bar->title) {
    C_MULTIPLATFORM_FREE(bar->title);
    bar->title = NULL;
  }

  len = strlen(title);
  bar->title = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!bar->title) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  strcpy_s(bar->title, len + 1, title);
#else
  strcpy(bar->title, title);
#endif

  return UI_ERROR_NONE;
}

ui_error_t md2_top_app_bar_get_base(struct md2_top_app_bar *bar,
                                    struct ui_top_app_bar_base **out_base) {
  if (!bar || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = bar->base;
  return UI_ERROR_NONE;
}
