/**
 * @file md3_carousel.c
 * @brief Material 3 Expressive Carousel component implementation wrapping
 * ui_carousel_base.
 */

/* clang-format off */
#include "material3/md3_carousel.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t md3_carousel_create(struct ui_engine *engine,
                               enum md3_carousel_layout_type layout_type,
                               const struct ui_carousel_config *config,
                               struct md3_carousel **out_carousel) {
  struct md3_carousel *carousel;
  ui_error_t rc;

  if (!engine || !config || !out_carousel ||
      (unsigned)layout_type >= MD3_CAROUSEL_LAYOUT_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  carousel = (struct md3_carousel *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_carousel));
  if (!carousel) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(carousel, 0, sizeof(struct md3_carousel));
  carousel->layout_type = layout_type;
  carousel->corner_radius = 28.0f; /* M3 Extra Large shape default */

  rc = ui_carousel_base_create(&carousel->base, config);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(carousel);
    return rc;
  }

  *out_carousel = carousel;
  return UI_ERROR_NONE;
}

ui_error_t md3_carousel_destroy(struct md3_carousel *carousel) {
  ui_error_t rc;

  if (!carousel) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_carousel_base_destroy(carousel->base);
  C_MULTIPLATFORM_FREE(carousel);
  return rc;
}

ui_error_t md3_carousel_set_mask_morphing(struct md3_carousel *carousel,
                                          int enable) {
  if (!carousel) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  carousel->dynamic_mask_morphing = enable ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_carousel_set_corner_radius(struct md3_carousel *carousel,
                                          float radius) {
  if (!carousel || radius < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  carousel->corner_radius = radius;
  return UI_ERROR_NONE;
}

ui_error_t md3_carousel_scroll_to(struct md3_carousel *carousel, size_t index,
                                  int smooth) {
  if (!carousel || !carousel->base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_carousel_base_scroll_to_index(carousel->base, index, smooth);
}

ui_error_t md3_carousel_get_component(struct md3_carousel *carousel,
                                      struct ui_component **out_component) {
  if (!carousel || !out_component || !carousel->base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_carousel_base_get_component(carousel->base, out_component);
}
