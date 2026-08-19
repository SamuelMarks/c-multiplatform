/**
 * @file ui_skeleton_base.h
 * @brief Skeleton loader base implementation.
 *
 * @defgroup SkeletonBase Skeleton Base
 * @brief Base implementation for skeleton loader components.
 * @{
 */

#ifndef UI_SKELETON_BASE_H
#define UI_SKELETON_BASE_H

/**
 * @brief Opaque structure for a computed signal.
 */
struct ui_computed;

/**
 * @brief Opaque structure for a signal.
 */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Represents the shape of the skeleton loader.
 */
enum ui_skeleton_shape {
  UI_SKELETON_SHAPE_RECTANGLE,        /**< Rectangular skeleton shape. */
  UI_SKELETON_SHAPE_CIRCLE,           /**< Circular skeleton shape. */
  UI_SKELETON_SHAPE_ROUNDED_RECTANGLE /**< Rounded rectangle skeleton shape. */
};

/**
 * @brief Configuration for the skeleton animation effect.
 */
struct ui_skeleton_animation_config {
  float duration_ms;       /**< Duration of one pulse/shimmer cycle in ms. */
  float base_opacity;      /**< Minimum opacity of the background. */
  float highlight_opacity; /**< Maximum opacity of the highlight wave. */
  float highlight_width;   /**< Width of the highlight wave relative to the
                              element (0.0 to 1.0). */
};

/**
 * @brief Opaque structure for the skeleton base component.
 */
struct ui_skeleton_base;

/**
 * @brief Creates a new skeleton loader component.
 *
 * @param out_skeleton Pointer to receive the allocated skeleton base.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY on allocation
 * failure.
 */
ui_error_t ui_skeleton_base_create(struct ui_skeleton_base **out_skeleton);

/**
 * @brief Destroys a skeleton loader component.
 *
 * @param skeleton The skeleton to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_skeleton_base_destroy(struct ui_skeleton_base *skeleton);

/**
 * @brief Sets the requested shape of the skeleton loader.
 *
 * @param skeleton The skeleton.
 * @param shape The shape to set.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_skeleton_base_set_shape(struct ui_skeleton_base *skeleton,
                                      enum ui_skeleton_shape shape);

/**
 * @brief Gets the requested shape of the skeleton loader.
 *
 * @param skeleton The skeleton.
 * @param out_shape Pointer to receive the shape.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_skeleton_base_get_shape(const struct ui_skeleton_base *skeleton,
                                      enum ui_skeleton_shape *out_shape);

/**
 * @brief Sets the dimensions of the skeleton loader.
 *
 * @param skeleton The skeleton.
 * @param width Requested width in pixels.
 * @param height Requested height in pixels.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_skeleton_base_set_dimensions(struct ui_skeleton_base *skeleton,
                                           int width, int height);

/**
 * @brief Gets the dimensions of the skeleton loader.
 *
 * @param skeleton The skeleton.
 * @param out_width Pointer to receive the width.
 * @param out_height Pointer to receive the height.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t
ui_skeleton_base_get_dimensions(const struct ui_skeleton_base *skeleton,
                                int *out_width, int *out_height);

/**
 * @brief Updates the internal animation state of the skeleton loader.
 *
 * Useful for calculating the shader uniform offset based on elapsed time.
 *
 * @param skeleton The skeleton.
 * @param delta_time_ms Delta time since last frame in milliseconds.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_skeleton_base_tick(struct ui_skeleton_base *skeleton,
                                 float delta_time_ms);

/**
 * @brief Gets the current animation phase offset [0.0, 1.0].
 *
 * @param skeleton The skeleton.
 * @param out_phase Pointer to receive the phase offset.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t
ui_skeleton_base_get_animation_phase(const struct ui_skeleton_base *skeleton,
                                     float *out_phase);

/**
 * @brief Retrieves the current animation configuration.
 *
 * @param skeleton The skeleton.
 * @param out_config Pointer to receive the configuration.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_skeleton_base_get_animation_config(
    const struct ui_skeleton_base *skeleton,
    struct ui_skeleton_animation_config **out_config);

/**
 * @brief Binds the active property.
 *
 * @param widget The skeleton widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_skeleton_base_bind_active(struct ui_skeleton_base *widget,
                                        struct ui_signal *signal);

#ifdef __cplusplus
}
#endif

#endif /* UI_SKELETON_BASE_H */

/** @} */
