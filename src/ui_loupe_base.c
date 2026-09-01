/**
 * @file ui_loupe_base.c
 * @brief ui_loupe_base.c implementation.
 */
/*
 * @file ui_loupe_base.c
 * @brief Implementation of the UI loupe (magnifying glass) component.
 */
/* clang-format off */
#include "ui_loupe_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

/**
 * @struct ui_loupe_base
 * @struct ui_loupe_base
 * @brief Context for a loupe widget, including focal and overlay coordinates.
 */
struct ui_loupe_base {
  struct ui_arena *arena;    /**< arena */
  float magnification_level; /**< magnification_level */
  float loupe_width;         /**< loupe_width */
  float loupe_height;        /**< loupe_height */
  float y_offset;            /**< y_offset */
  ui_bool_t is_visible;      /**< is_visible */

  struct ui_dom_point focal_point;    /**< focal_point */
  struct ui_dom_point overlay_origin; /**< overlay_origin */

  ui_signal_t *overlay_origin_signal; /**< overlay_origin_signal */
};

/**
 * @brief Signal equality callback for comparing two UI points.
 * @param[in] a The first point payload.
 * @param[in] b The second point payload.
 * @param[out] out_equal Pointer to store the boolean equality result.
 * @return UI_ERROR_NONE on success.
 */
/**
 * @brief point_equality.
 * @param a Parameter a.
 * @param b Parameter b.
 * @param out_equal Parameter out_equal.
 * @return Return value.
 */
static ui_error_t point_equality(union ui_signal_payload a,
                                 union ui_signal_payload b,
                                 ui_bool_t *out_equal) {
  const struct ui_dom_point *pa = (const struct ui_dom_point *)a.ptr_val;
  const struct ui_dom_point *pb = (const struct ui_dom_point *)b.ptr_val;

  /* Simple float equality since this is an abstract state carrier,
     in a strict environment epsilon comparison would be used. */
  *out_equal = (pa->x == pb->x && pa->y == pb->y) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

/**
 * @brief Creates a new loupe base widget.
 * @param[in,out] arena The memory arena.
 * @param[in] config The initial configuration (size, magnification).
 * @param[out] out_loupe Pointer to store the created loupe widget.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_loupe_base_create(struct ui_arena *arena,
                                const struct ui_loupe_config *config,
                                struct ui_loupe_base **out_loupe) {
  ui_error_t err;
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !config || !out_loupe) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_loupe_base), 8, &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  memset(ptr, 0, sizeof(struct ui_loupe_base));

  *out_loupe = (struct ui_loupe_base *)ptr;
  (*out_loupe)->arena = arena;
  (*out_loupe)->magnification_level = config->magnification_level;
  (*out_loupe)->loupe_width = config->loupe_width;
  (*out_loupe)->loupe_height = config->loupe_height;
  (*out_loupe)->y_offset = config->y_offset;
  (*out_loupe)->is_visible = UI_FALSE;

  (*out_loupe)->focal_point.x = 0.0;
  (*out_loupe)->focal_point.y = 0.0;
  (*out_loupe)->focal_point.z = 0.0;
  (*out_loupe)->focal_point.w = 1.0;

  (*out_loupe)->overlay_origin.x = 0.0;
  (*out_loupe)->overlay_origin.y = -config->y_offset;
  (*out_loupe)->overlay_origin.z = 0.0;
  (*out_loupe)->overlay_origin.w = 1.0;

  initial_payload.ptr_val = &(*out_loupe)->overlay_origin;
  err = ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_POINTER,
                         point_equality, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                         &(*out_loupe)->overlay_origin_signal);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a loupe base widget.
 * @param[in,out] loupe The loupe to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_loupe_base_destroy(struct ui_loupe_base *loupe) {
  if (!loupe) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (loupe->overlay_origin_signal) {
    {
      ui_error_t rc_cleanup = ui_signal_destroy(loupe->overlay_origin_signal);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Sets the focal point of the loupe, updating the overlay origin signal.
 * @param[in,out] loupe The loupe widget.
 * @param[in] focal_point The new focal coordinates.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_loupe_base_set_focal_point(struct ui_loupe_base *loupe,
                              const struct ui_dom_point *focal_point) {

  union ui_signal_payload payload;

  if (!loupe || !focal_point) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  loupe->focal_point = *focal_point;

  /* Calculate overlay origin by centering horizontally and applying Y offset */
  loupe->overlay_origin.x = focal_point->x - (loupe->loupe_width / 2.0f);
  loupe->overlay_origin.y =
      focal_point->y - loupe->y_offset - loupe->loupe_height;
  loupe->overlay_origin.z = focal_point->z;
  loupe->overlay_origin.w = focal_point->w;

  payload.ptr_val = &loupe->overlay_origin;
  return ui_signal_set(loupe->overlay_origin_signal, payload);
}

/**
 * @brief Gets the signal representing the computed top-left origin for the
 * loupe overlay.
 * @param[in] loupe The loupe widget.
 * @param[out] out_signal Pointer to store the signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_loupe_base_get_overlay_origin_signal(struct ui_loupe_base *loupe,
                                                   ui_signal_t **out_signal) {
  if (!loupe || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = loupe->overlay_origin_signal;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the current focal point of the loupe.
 * @param[in] loupe The loupe widget.
 * @param[out] out_focal_point Pointer to store the focal point.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_loupe_base_get_focal_point(const struct ui_loupe_base *loupe,
                                         struct ui_dom_point *out_focal_point) {
  if (!loupe || !out_focal_point) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_focal_point = loupe->focal_point;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the visibility state of the loupe.
 * @param[in,out] loupe The loupe widget.
 * @param[in] visible The boolean visibility state.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_loupe_base_set_visible(struct ui_loupe_base *loupe,
                                     ui_bool_t visible) {
  if (!loupe) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  loupe->is_visible = visible;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the magnification scale level of the loupe.
 * @param[in,out] loupe The loupe widget.
 * @param[in] magnification_level The new magnification level.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_loupe_base_set_magnification_level(struct ui_loupe_base *loupe,
                                                 float magnification_level) {
  if (!loupe) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (magnification_level <= 0.0f) {
    magnification_level = 1.0f; /* Guard against invalid zoom */
  }
  loupe->magnification_level = magnification_level;
  return UI_ERROR_NONE;
}
