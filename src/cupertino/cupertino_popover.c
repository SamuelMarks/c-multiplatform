/**
 * @file cupertino_popover.c
 * @brief Cupertino Popover implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_popover.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t
cupertino_popover_create(struct ui_engine *engine,
                         const struct cupertino_popover_descriptor *desc,
                         struct cupertino_popover **out_popover) {
  struct cupertino_popover *popover;
  ui_error_t rc;

  if (!engine || !desc || !out_popover) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->permitted_arrows < 0 || (int)desc->permitted_arrows > 4) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  popover = (struct cupertino_popover *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_popover));
  if (!popover) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(popover, 0, sizeof(*popover));
  popover->content_width = (desc->content_width > 0.0f)
                               ? desc->content_width
                               : CUPERTINO_POPOVER_DEFAULT_WIDTH;
  popover->content_height = (desc->content_height > 0.0f)
                                ? desc->content_height
                                : CUPERTINO_POPOVER_DEFAULT_HEIGHT;
  popover->permitted_arrows = desc->permitted_arrows;
  popover->actual_arrow_direction = CUPERTINO_ARROW_DIRECTION_UP;
  popover->is_presented = 0;

  rc = ui_popover_base_create(&popover->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(popover);
    return rc;
  }

  *out_popover = popover;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_popover_destroy(struct cupertino_popover *popover) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!popover) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (popover->base) {
    rc = ui_popover_base_destroy(popover->base);
    popover->base = NULL;
  }

  C_MULTIPLATFORM_FREE(popover);
  return rc;
}

ui_error_t cupertino_popover_present(struct cupertino_popover *popover,
                                     float anchor_x, float anchor_y,
                                     float anchor_w, float anchor_h,
                                     float screen_w, float screen_h) {
  float anchor_center_x;
  float total_h;
  float min_offset;
  float max_offset;
  enum cupertino_arrow_direction direction;

  if (!popover || anchor_w < 0.0f || anchor_h < 0.0f || screen_w <= 0.0f ||
      screen_h <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  anchor_center_x = anchor_x + (anchor_w * 0.5f);
  total_h = popover->content_height + CUPERTINO_POPOVER_ARROW_HEIGHT;

  /* Check vertical placement */
  if (anchor_y + anchor_h + total_h <= screen_h - 10.0f) {
    /* Below anchor: arrow points UP */
    direction = CUPERTINO_ARROW_DIRECTION_UP;
    popover->bounds_y = anchor_y + anchor_h;
    popover->bounds_h = total_h;
  } else if (anchor_y - total_h >= 10.0f) {
    /* Above anchor: arrow points DOWN */
    direction = CUPERTINO_ARROW_DIRECTION_DOWN;
    popover->bounds_y = anchor_y - total_h;
    popover->bounds_h = total_h;
  } else {
    /* Default below */
    direction = CUPERTINO_ARROW_DIRECTION_UP;
    popover->bounds_y = 10.0f;
    popover->bounds_h = total_h;
  }

  /* Horizontal placement */
  popover->bounds_w = popover->content_width;
  popover->bounds_x = anchor_center_x - (popover->bounds_w * 0.5f);
  if (popover->bounds_x < 10.0f) {
    popover->bounds_x = 10.0f;
  } else if (popover->bounds_x + popover->bounds_w > screen_w - 10.0f) {
    popover->bounds_x = screen_w - 10.0f - popover->bounds_w;
  }

  /* Arrow offset calculation */
  popover->arrow_offset = anchor_center_x - popover->bounds_x;
  min_offset = CUPERTINO_POPOVER_CORNER_RADIUS +
               (CUPERTINO_POPOVER_ARROW_BASE_WIDTH * 0.5f);
  max_offset = popover->bounds_w - min_offset;
  if (popover->arrow_offset < min_offset) {
    popover->arrow_offset = min_offset;
  } else if (popover->arrow_offset > max_offset) {
    popover->arrow_offset = max_offset;
  }

  popover->actual_arrow_direction = direction;
  popover->is_presented = 1;

  return UI_ERROR_NONE;
}

ui_error_t cupertino_popover_dismiss(struct cupertino_popover *popover) {
  if (!popover) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  popover->is_presented = 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_popover_is_presented(const struct cupertino_popover *popover,
                               int *out_is_presented) {
  if (!popover || !out_is_presented) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_presented = popover->is_presented;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_popover_get_bounds(const struct cupertino_popover *popover,
                                        float *out_x, float *out_y,
                                        float *out_w, float *out_h) {
  if (!popover || !out_x || !out_y || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_x = popover->bounds_x;
  *out_y = popover->bounds_y;
  *out_w = popover->bounds_w;
  *out_h = popover->bounds_h;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_popover_get_arrow_position(
    const struct cupertino_popover *popover,
    enum cupertino_arrow_direction *out_direction, float *out_offset) {
  if (!popover || !out_direction || !out_offset) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_direction = popover->actual_arrow_direction;
  *out_offset = popover->arrow_offset;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_popover_get_base(struct cupertino_popover *popover,
                                      struct ui_popover_base **out_base) {
  if (!popover || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = popover->base;
  return UI_ERROR_NONE;
}
