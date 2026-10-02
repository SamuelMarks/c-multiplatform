/**
 * @file cupertino_activity_rings.h
 * @brief Cupertino Activity & Health Rings Gauge conforming to Apple Watch /
 * Fitness HIG.
 */

#ifndef CUPERTINO_CUPERTINO_ACTIVITY_RINGS_H
#define CUPERTINO_CUPERTINO_ACTIVITY_RINGS_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_RINGS_DEFAULT_OUTER_RADIUS 80.0f
#define CUPERTINO_RINGS_DEFAULT_THICKNESS 18.0f
#define CUPERTINO_RINGS_DEFAULT_GAP 4.0f

/**
 * @enum cupertino_ring_id
 * @brief Identifiers for the three activity rings.
 */
enum cupertino_ring_id {
  CUPERTINO_RING_MOVE = 0,     /**< Outer red ring (Move / Active Calories). */
  CUPERTINO_RING_EXERCISE = 1, /**< Middle green ring (Exercise / Minutes). */
  CUPERTINO_RING_STAND = 2,    /**< Inner cyan ring (Stand / Hours). */
  CUPERTINO_RING_COUNT = 3
};

/**
 * @struct cupertino_activity_rings_descriptor
 * @brief Configuration descriptor for activity rings gauge.
 */
struct cupertino_activity_rings_descriptor {
  float outer_radius;        /**< Radius of outer ring centerline. */
  float ring_thickness;      /**< Stroke width of each ring. */
  float ring_gap;            /**< Gap between adjacent concentric rings. */
  float initial_progress[3]; /**< Initial progress for each ring [0.0, ...]. */
};

/**
 * @struct cupertino_activity_rings
 * @brief Activity rings instance holding geometry and state.
 */
struct cupertino_activity_rings {
  float outer_radius;
  float ring_thickness;
  float ring_gap;
  float progress[CUPERTINO_RING_COUNT];
  ui_color_t ring_colors_start[CUPERTINO_RING_COUNT];
  ui_color_t ring_colors_end[CUPERTINO_RING_COUNT];
  ui_color_t background_tracks[CUPERTINO_RING_COUNT];
};

/**
 * @brief Creates a new Cupertino activity rings instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_rings Pointer to receive newly created instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_activity_rings_create(
    struct ui_engine *engine,
    const struct cupertino_activity_rings_descriptor *desc,
    struct cupertino_activity_rings **out_rings);

/**
 * @brief Destroys a Cupertino activity rings instance.
 *
 * @param rings Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_activity_rings_destroy(struct cupertino_activity_rings *rings);

/**
 * @brief Sets the progress value of a specific ring.
 *
 * @param rings Target activity rings gauge.
 * @param ring Ring identifier (Move, Exercise, Stand).
 * @param progress Normalized progress (0.0 to 1.0+, can exceed 1.0 for
 * multi-loop).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_activity_rings_set_progress(
    struct cupertino_activity_rings *rings, enum cupertino_ring_id ring,
    float progress);

/**
 * @brief Gets the progress value of a specific ring.
 *
 * @param rings Target activity rings gauge.
 * @param ring Ring identifier (Move, Exercise, Stand).
 * @param out_progress Pointer to receive progress value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_activity_rings_get_progress(
    const struct cupertino_activity_rings *rings, enum cupertino_ring_id ring,
    float *out_progress);

/**
 * @brief Gets the centerline radius for a specific ring.
 *
 * @param rings Target activity rings gauge.
 * @param ring Ring identifier (Move, Exercise, Stand).
 * @param out_radius Pointer to receive centerline radius.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_activity_rings_get_ring_radius(
    const struct cupertino_activity_rings *rings, enum cupertino_ring_id ring,
    float *out_radius);

/**
 * @brief Calculates if an overlapping drop shadow should be cast where the ring
 * completes a 360-degree loop and crosses over its beginning.
 *
 * @param rings Target activity rings gauge.
 * @param ring Ring identifier (Move, Exercise, Stand).
 * @param out_has_shadow Pointer to receive 1 if shadow needed, 0 otherwise.
 * @param out_shadow_angle Pointer to receive angle in radians where cap
 * crosses.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_activity_rings_calculate_shadow(
    const struct cupertino_activity_rings *rings, enum cupertino_ring_id ring,
    int *out_has_shadow, float *out_shadow_angle);

/**
 * @brief Computes the center coordinates of the moving end-cap for a ring.
 *
 * @param rings Target activity rings gauge.
 * @param ring Ring identifier (Move, Exercise, Stand).
 * @param out_x Pointer to receive X offset relative to gauge center.
 * @param out_y Pointer to receive Y offset relative to gauge center.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_activity_rings_get_cap_center(
    const struct cupertino_activity_rings *rings, enum cupertino_ring_id ring,
    float *out_x, float *out_y);

/**
 * @brief Retrieves the color gradient stops and background track color for a
 * ring.
 *
 * @param rings Target activity rings gauge.
 * @param ring Ring identifier (Move, Exercise, Stand).
 * @param out_start Pointer to receive gradient start color.
 * @param out_end Pointer to receive gradient end color.
 * @param out_track Pointer to receive dark translucent track color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_activity_rings_get_colors(
    const struct cupertino_activity_rings *rings, enum cupertino_ring_id ring,
    ui_color_t *out_start, ui_color_t *out_end, ui_color_t *out_track);

/**
 * @brief Gets the total bounding dimensions of the activity rings gauge.
 *
 * @param rings Target activity rings gauge.
 * @param out_width Pointer to receive bounding width.
 * @param out_height Pointer to receive bounding height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_activity_rings_get_dimensions(
    const struct cupertino_activity_rings *rings, float *out_width,
    float *out_height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_ACTIVITY_RINGS_H */
