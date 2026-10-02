/**
 * @file cupertino_list_tile.c
 * @brief Cupertino List Tile component implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_list_tile.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_list_tile_mock_create_fail = 0;
int g_cupertino_list_tile_mock_destroy_fail = 0;

static ui_error_t
mock_list_item_base_create(struct ui_list_item_base **out_base) {
  if (g_cupertino_list_tile_mock_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_list_item_base_create(out_base);
}
#undef ui_list_item_base_create
/** @cond */
#define ui_list_item_base_create mock_list_item_base_create
/** @endcond */

static ui_error_t mock_list_item_base_destroy(struct ui_list_item_base *base) {
  if (g_cupertino_list_tile_mock_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_list_item_base_destroy(base);
}
#undef ui_list_item_base_destroy
/** @cond */
#define ui_list_item_base_destroy mock_list_item_base_destroy
/** @endcond */
#endif

#define CUPERTINO_LIST_TILE_HEIGHT_DEFAULT 44.0f
#define CUPERTINO_LIST_TILE_HEIGHT_SUBTITLE 54.0f
#define CUPERTINO_LIST_TILE_HIGHLIGHT_MAX_OPACITY 0.15f

ui_error_t
cupertino_list_tile_create(struct ui_engine *engine,
                           const struct cupertino_list_tile_descriptor *desc,
                           struct cupertino_list_tile **out_tile) {
  struct cupertino_list_tile *tile;
  ui_error_t rc;

  if (!engine || !desc || !out_tile) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tile = (struct cupertino_list_tile *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_list_tile));
  if (!tile) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tile, 0, sizeof(*tile));
  tile->dynamic_type_scale = 1.0f;
  tile->style = desc->style;
  tile->accessory = desc->accessory;
  tile->is_rtl = desc->is_rtl ? 1 : 0;

  if (desc->title) {
#if defined(_MSC_VER)
    strncpy_s(tile->title, sizeof(tile->title), desc->title,
              sizeof(tile->title) - 1);
#else
    strncpy(tile->title, desc->title, sizeof(tile->title) - 1);
    tile->title[sizeof(tile->title) - 1] = '\0';
#endif
  }

  if (desc->subtitle) {
#if defined(_MSC_VER)
    strncpy_s(tile->subtitle, sizeof(tile->subtitle), desc->subtitle,
              sizeof(tile->subtitle) - 1);
#else
    strncpy(tile->subtitle, desc->subtitle, sizeof(tile->subtitle) - 1);
    tile->subtitle[sizeof(tile->subtitle) - 1] = '\0';
#endif
  }

  if (desc->additional_info) {
#if defined(_MSC_VER)
    strncpy_s(tile->additional_info, sizeof(tile->additional_info),
              desc->additional_info, sizeof(tile->additional_info) - 1);
#else
    strncpy(tile->additional_info, desc->additional_info,
            sizeof(tile->additional_info) - 1);
    tile->additional_info[sizeof(tile->additional_info) - 1] = '\0';
#endif
  }

  if (desc->leading_icon) {
#if defined(_MSC_VER)
    strncpy_s(tile->leading_icon, sizeof(tile->leading_icon),
              desc->leading_icon, sizeof(tile->leading_icon) - 1);
#else
    strncpy(tile->leading_icon, desc->leading_icon,
            sizeof(tile->leading_icon) - 1);
    tile->leading_icon[sizeof(tile->leading_icon) - 1] = '\0';
#endif
  }

  rc = ui_list_item_base_create(&tile->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tile);
    return rc;
  }

  *out_tile = tile;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_list_tile_destroy(struct cupertino_list_tile *tile) {
  ui_error_t rc;

  if (!tile) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tile->base) {
    rc = ui_list_item_base_destroy(tile->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    tile->base = NULL;
  }

  C_MULTIPLATFORM_FREE(tile);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_list_tile_set_title(struct cupertino_list_tile *tile,
                                         const char *title) {
  if (!tile || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(tile->title, sizeof(tile->title), title, sizeof(tile->title) - 1);
#else
  strncpy(tile->title, title, sizeof(tile->title) - 1);
  tile->title[sizeof(tile->title) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t cupertino_list_tile_get_title(const struct cupertino_list_tile *tile,
                                         const char **out_title) {
  if (!tile || !out_title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_title = tile->title;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_list_tile_set_subtitle(struct cupertino_list_tile *tile,
                                            const char *subtitle) {
  if (!tile) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (subtitle) {
#if defined(_MSC_VER)
    strncpy_s(tile->subtitle, sizeof(tile->subtitle), subtitle,
              sizeof(tile->subtitle) - 1);
#else
    strncpy(tile->subtitle, subtitle, sizeof(tile->subtitle) - 1);
    tile->subtitle[sizeof(tile->subtitle) - 1] = '\0';
#endif
  } else {
    tile->subtitle[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_tile_get_subtitle(const struct cupertino_list_tile *tile,
                                 const char **out_subtitle) {
  if (!tile || !out_subtitle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_subtitle = (tile->subtitle[0] != '\0') ? tile->subtitle : NULL;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_tile_set_additional_info(struct cupertino_list_tile *tile,
                                        const char *info) {
  if (!tile) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (info) {
#if defined(_MSC_VER)
    strncpy_s(tile->additional_info, sizeof(tile->additional_info), info,
              sizeof(tile->additional_info) - 1);
#else
    strncpy(tile->additional_info, info, sizeof(tile->additional_info) - 1);
    tile->additional_info[sizeof(tile->additional_info) - 1] = '\0';
#endif
  } else {
    tile->additional_info[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_tile_get_additional_info(const struct cupertino_list_tile *tile,
                                        const char **out_info) {
  if (!tile || !out_info) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_info = (tile->additional_info[0] != '\0') ? tile->additional_info : NULL;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_list_tile_set_accessory(
    struct cupertino_list_tile *tile,
    enum cupertino_list_tile_accessory accessory) {
  int acc_val = (int)accessory;
  if (!tile || acc_val < 0 ||
      acc_val > (int)CUPERTINO_LIST_TILE_ACCESSORY_CHECKMARK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tile->accessory = accessory;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_list_tile_get_accessory(
    const struct cupertino_list_tile *tile,
    enum cupertino_list_tile_accessory *out_accessory) {
  if (!tile || !out_accessory) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_accessory = tile->accessory;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_list_tile_set_pressed(struct cupertino_list_tile *tile,
                                           int pressed) {
  if (!tile) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tile->is_pressed = pressed ? 1 : 0;
  tile->highlight_opacity =
      tile->is_pressed ? CUPERTINO_LIST_TILE_HIGHLIGHT_MAX_OPACITY : 0.0f;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_tile_is_pressed(const struct cupertino_list_tile *tile,
                               int *out_pressed) {
  if (!tile || !out_pressed) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_pressed = tile->is_pressed;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_list_tile_get_highlight_opacity(
    const struct cupertino_list_tile *tile, float *out_opacity) {
  if (!tile || !out_opacity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_opacity = tile->highlight_opacity;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_tile_get_height(const struct cupertino_list_tile *tile,
                               float *out_height) {
  if (!tile || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_height = (tile->style == CUPERTINO_LIST_TILE_SUBTITLE)
                    ? CUPERTINO_LIST_TILE_HEIGHT_SUBTITLE
                    : CUPERTINO_LIST_TILE_HEIGHT_DEFAULT;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_list_tile_get_base(struct cupertino_list_tile *tile,
                                        struct ui_list_item_base **out_base) {
  if (!tile || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = tile->base;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_tile_set_scale_factor(struct cupertino_list_tile *tile,
                                     float scale_factor) {
  if (!tile || scale_factor <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tile->dynamic_type_scale = scale_factor;
  if (scale_factor >= 1.5f && tile->additional_info[0] != '\0') {
    tile->is_stacked_layout = 1;
  } else {
    tile->is_stacked_layout = 0;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_tile_is_stacked(const struct cupertino_list_tile *tile,
                               int *out_is_stacked) {
  if (!tile || !out_is_stacked) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_stacked = tile->is_stacked_layout;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_list_tile_compute_height(const struct cupertino_list_tile *tile,
                                   float *out_height) {
  float base_h;

  if (!tile || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  base_h = (tile->style == CUPERTINO_LIST_TILE_SUBTITLE)
               ? CUPERTINO_LIST_TILE_HEIGHT_SUBTITLE
               : CUPERTINO_LIST_TILE_HEIGHT_DEFAULT;

  if (tile->is_stacked_layout) {
    *out_height = (base_h + 24.0f) * tile->dynamic_type_scale;
  } else {
    *out_height = base_h * tile->dynamic_type_scale;
  }

  return UI_ERROR_NONE;
}
