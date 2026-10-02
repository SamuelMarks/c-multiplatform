/**
 * @file cupertino_magnifier.h
 * @brief Cupertino Text Magnifier (optical loupe) conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_MAGNIFIER_H
#define CUPERTINO_CUPERTINO_MAGNIFIER_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_MAGNIFIER_DEFAULT_WIDTH 78.0f
#define CUPERTINO_MAGNIFIER_DEFAULT_HEIGHT 38.0f
#define CUPERTINO_MAGNIFIER_DEFAULT_OFFSET_Y -48.0f
#define CUPERTINO_MAGNIFIER_DEFAULT_ZOOM 1.25f
#define CUPERTINO_MAGNIFIER_DEFAULT_CORNER_RADIUS 19.0f

/**
 * @struct cupertino_magnifier_descriptor
 * @brief Configuration descriptor for optical text magnifier loupe.
 */
struct cupertino_magnifier_descriptor {
  float width;           /**< Lens capsule width (default 78.0f). */
  float height;          /**< Lens capsule height (default 38.0f). */
  float vertical_offset; /**< Vertical float offset (default -48.0f). */
  float magnification;   /**< Zoom factor (default 1.25f). */
  float corner_radius;   /**< Corner radius (default 19.0f). */
};

/**
 * @struct cupertino_magnifier
 * @brief Optical text magnifier loupe instance.
 */
struct cupertino_magnifier {
  float width;
  float height;
  float vertical_offset;
  float magnification;
  float corner_radius;
  float touch_x;
  float touch_y;
  float lens_x;
  float lens_y;
  float current_scale; /**< Spring animation scale factor [0.0, 1.0]. */
  int is_active;       /**< 1 if active on screen, 0 if hidden. */
};

/**
 * @brief Creates a new Cupertino text magnifier loupe.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_magnifier Pointer to receive newly created magnifier instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_magnifier_create(
    struct ui_engine *engine, const struct cupertino_magnifier_descriptor *desc,
    struct cupertino_magnifier **out_magnifier);

/**
 * @brief Destroys a Cupertino text magnifier loupe.
 *
 * @param magnifier Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_magnifier_destroy(struct cupertino_magnifier *magnifier);

/**
 * @brief Shows the optical magnifier centered above the given touch contact
 * point.
 *
 * @param magnifier Target magnifier.
 * @param touch_x Touch X coordinate.
 * @param touch_y Touch Y coordinate.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_magnifier_show(
    struct cupertino_magnifier *magnifier, float touch_x, float touch_y);

/**
 * @brief Updates the contact position during finger drag.
 *
 * @param magnifier Target magnifier.
 * @param touch_x New touch X coordinate.
 * @param touch_y New touch Y coordinate.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_magnifier_update_position(
    struct cupertino_magnifier *magnifier, float touch_x, float touch_y);

/**
 * @brief Clamps lens position to remain safely within viewport boundaries.
 *
 * @param magnifier Target magnifier.
 * @param screen_width Screen/viewport width.
 * @param screen_height Screen/viewport height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_magnifier_clamp_bounds(struct cupertino_magnifier *magnifier,
                                 float screen_width, float screen_height);

/**
 * @brief Hides the magnifier with scale-down animation.
 *
 * @param magnifier Target magnifier.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_magnifier_hide(struct cupertino_magnifier *magnifier);

/**
 * @brief Advances the appearance/dissolve animation frame.
 *
 * @param magnifier Target magnifier.
 * @param delta_ms Elapsed time in milliseconds.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_magnifier_tick(struct cupertino_magnifier *magnifier, float delta_ms);

/**
 * @brief Retrieves current lens bounding rectangle.
 *
 * @param magnifier Target magnifier.
 * @param out_x Pointer to receive lens top-left X coordinate.
 * @param out_y Pointer to receive lens top-left Y coordinate.
 * @param out_w Pointer to receive lens width.
 * @param out_h Pointer to receive lens height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_magnifier_get_lens_rect(
    const struct cupertino_magnifier *magnifier, float *out_x, float *out_y,
    float *out_w, float *out_h);

/**
 * @brief Retrieves source background sampling center coordinates.
 *
 * @param magnifier Target magnifier.
 * @param out_sample_x Pointer to receive sampling center X coordinate.
 * @param out_sample_y Pointer to receive sampling center Y coordinate.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_magnifier_get_sample_center(
    const struct cupertino_magnifier *magnifier, float *out_sample_x,
    float *out_sample_y);

/**
 * @brief Retrieves current animated scale of the lens.
 *
 * @param magnifier Target magnifier.
 * @param out_scale Pointer to receive scale factor [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_magnifier_get_scale(
    const struct cupertino_magnifier *magnifier, float *out_scale);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_MAGNIFIER_H */
