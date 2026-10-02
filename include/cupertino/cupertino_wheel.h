/**
 * @file cupertino_wheel.h
 * @brief Cupertino 3D Cylinder Wheel Picker conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_WHEEL_H
#define CUPERTINO_CUPERTINO_WHEEL_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_WHEEL_DEFAULT_HEIGHT 216.0f
#define CUPERTINO_WHEEL_DEFAULT_ITEM_HEIGHT 32.0f
#define CUPERTINO_WHEEL_PERSPECTIVE_DISTANCE 300.0f
#define CUPERTINO_WHEEL_DECELERATION_RATE 0.990f

/**
 * @struct cupertino_wheel_descriptor
 * @brief Configuration descriptor for creating a 3D wheel picker.
 */
struct cupertino_wheel_descriptor {
  int item_count;        /**< Total count of selectable items. */
  int selected_index;    /**< Initially selected item index (0-based). */
  float viewport_height; /**< Height of wheel container (default 216pt). */
  float item_height;     /**< Height per item row (default 32pt). */
};

/**
 * @struct cupertino_wheel
 * @brief Cupertino 3D cylinder wheel picker instance.
 */
struct cupertino_wheel {
  int item_count;
  int selected_index;
  float viewport_height;
  float item_height;
  float scroll_offset_y; /**< Current vertical scroll position in points. */
  float velocity_y;      /**< Fling velocity in points per millisecond. */
  float target_offset_y; /**< Target magnetic snap detent offset. */
  int is_dragging;       /**< 1 if finger / pointer drag active. */
  int is_settling;       /**< 1 if magnetic spring snap animation active. */
  int last_ticked_index; /**< Tracking for selection haptic ticks. */
};

/**
 * @brief Creates a new Cupertino 3D wheel picker instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_wheel Pointer to receive newly created wheel instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_wheel_create(
    struct ui_engine *engine, const struct cupertino_wheel_descriptor *desc,
    struct cupertino_wheel **out_wheel);

/**
 * @brief Destroys a Cupertino 3D wheel picker instance.
 *
 * @param wheel Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_wheel_destroy(struct cupertino_wheel *wheel);

/**
 * @brief Sets the total item count.
 *
 * @param wheel Target wheel picker.
 * @param count Number of items (>= 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_wheel_set_item_count(struct cupertino_wheel *wheel, int count);

/**
 * @brief Gets the total item count.
 *
 * @param wheel Target wheel picker.
 * @param out_count Pointer to receive item count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_wheel_get_item_count(
    const struct cupertino_wheel *wheel, int *out_count);

/**
 * @brief Sets the currently selected index.
 *
 * @param wheel Target wheel picker.
 * @param index New index (0 to item_count - 1).
 * @param animated 1 for smooth magnetic snap animation, 0 for immediate jump.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_wheel_set_selected_index(
    struct cupertino_wheel *wheel, int index, int animated);

/**
 * @brief Gets the currently selected index.
 *
 * @param wheel Target wheel picker.
 * @param out_index Pointer to receive selected index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_wheel_get_selected_index(
    const struct cupertino_wheel *wheel, int *out_index);

/**
 * @brief Initiates interactive touch drag gesture.
 *
 * @param wheel Target wheel picker.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_wheel_drag_start(struct cupertino_wheel *wheel);

/**
 * @brief Updates vertical scroll displacement during drag.
 *
 * @param wheel Target wheel picker.
 * @param delta_y Displacement change in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_wheel_drag_update(struct cupertino_wheel *wheel, float delta_y);

/**
 * @brief Concludes interactive drag and releases into fling or detent snap.
 *
 * @param wheel Target wheel picker.
 * @param release_velocity_y Velocity in points/sec at moment of release.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_wheel_drag_end(
    struct cupertino_wheel *wheel, float release_velocity_y);

/**
 * @brief Advances physical simulation and detent magnetic snap animation.
 *
 * @param wheel Target wheel picker.
 * @param delta_ms Elapsed time in milliseconds.
 * @param out_haptic_tick Pointer to receive 1 if a selection detent was
 * crossed.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_wheel_tick(
    struct cupertino_wheel *wheel, float delta_ms, int *out_haptic_tick);

/**
 * @brief Computes 3D cylinder perspective transform and opacity for an item.
 *
 * @param wheel Target wheel picker.
 * @param item_index Index of the item.
 * @param out_y Pointer to receive projected screen Y coordinate.
 * @param out_scale Pointer to receive perspective scale factor.
 * @param out_opacity Pointer to receive progressive alpha fade [0.1, 1.0].
 * @param out_angle_rad Pointer to receive cylinder rotation angle in radians.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_wheel_calculate_item_transform(const struct cupertino_wheel *wheel,
                                         int item_index, float *out_y,
                                         float *out_scale, float *out_opacity,
                                         float *out_angle_rad);

/**
 * @brief Retrieves bounding dimensions for the wheel container.
 *
 * @param wheel Target wheel picker.
 * @param out_width Pointer to receive width (typically full parent width).
 * @param out_height Pointer to receive height (viewport height).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_wheel_get_dimensions(
    const struct cupertino_wheel *wheel, float *out_width, float *out_height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_WHEEL_H */
