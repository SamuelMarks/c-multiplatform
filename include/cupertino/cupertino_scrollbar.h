/**
 * @file cupertino_scrollbar.h
 * @brief Cupertino Scrollbar component conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_SCROLLBAR_H
#define CUPERTINO_CUPERTINO_SCROLLBAR_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_SCROLLBAR_THICKNESS_RESTING 3.0f
#define CUPERTINO_SCROLLBAR_THICKNESS_ACTIVE 6.0f
#define CUPERTINO_SCROLLBAR_MIN_THUMB_LENGTH 36.0f
#define CUPERTINO_SCROLLBAR_FADE_TIMEOUT_MS 1500.0f
#define CUPERTINO_SCROLLBAR_FADE_ANIMATION_MS 300.0f

/**
 * @struct cupertino_scrollbar_descriptor
 * @brief Configuration descriptor for creating a Cupertino scrollbar.
 */
struct cupertino_scrollbar_descriptor {
  float safe_area_top; /**< Top safe area inset to avoid nav bar collision. */
  float safe_area_bottom; /**< Bottom safe area inset to avoid tab bar
                             collision. */
};

/**
 * @struct cupertino_scrollbar
 * @brief Cupertino Scrollbar instance.
 */
struct cupertino_scrollbar {
  float safe_area_top;       /**< Top inset margin. */
  float safe_area_bottom;    /**< Bottom inset margin. */
  float content_offset;      /**< Current vertical scroll offset. */
  float content_length;      /**< Total content height. */
  float viewport_length;     /**< Visible viewport height. */
  float thumb_offset;        /**< Computed thumb Y position in track. */
  float thumb_length;        /**< Computed thumb height in points. */
  float current_thickness;   /**< 3.0pt resting -> 6.0pt dragging. */
  float current_opacity;     /**< 0.0f invisible to 1.0f fully visible. */
  float inactivity_timer_ms; /**< Inactivity timer tracking fade-out. */
  int is_dragging;           /**< 1 if user is actively dragging thumb. */
  int is_visible;            /**< 1 if currently visible. */
};

/**
 * @brief Creates a new Cupertino scrollbar instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_scrollbar Pointer to receive newly created scrollbar.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_scrollbar_create(
    struct ui_engine *engine, const struct cupertino_scrollbar_descriptor *desc,
    struct cupertino_scrollbar **out_scrollbar);

/**
 * @brief Destroys a Cupertino scrollbar instance.
 *
 * @param scrollbar Scrollbar instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_scrollbar_destroy(struct cupertino_scrollbar *scrollbar);

/**
 * @brief Updates scroll metrics and triggers instant fade-in of scrollbar.
 *
 * @param scrollbar Target scrollbar.
 * @param content_offset Current scroll offset in content points.
 * @param content_length Total scrollable height.
 * @param viewport_length Height of visible viewport.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_scrollbar_handle_scroll(
    struct cupertino_scrollbar *scrollbar, float content_offset,
    float content_length, float viewport_length);

/**
 * @brief Advances scrollbar timer and fade animation.
 *
 * @param scrollbar Target scrollbar.
 * @param delta_ms Elapsed time in milliseconds.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_scrollbar_tick(struct cupertino_scrollbar *scrollbar, float delta_ms);

/**
 * @brief Updates direct thumb dragging state.
 *
 * When dragging is active, thickness expands from 3pt to 6pt.
 *
 * @param scrollbar Target scrollbar.
 * @param dragging 1 if dragging, 0 if released.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_scrollbar_set_dragging(
    struct cupertino_scrollbar *scrollbar, int dragging);

/**
 * @brief Queries whether thumb is currently being dragged.
 *
 * @param scrollbar Target scrollbar.
 * @param out_dragging Pointer to receive dragging state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_scrollbar_is_dragging(
    const struct cupertino_scrollbar *scrollbar, int *out_dragging);

/**
 * @brief Retrieves current thickness of scrollbar capsule (3.0pt to 6.0pt).
 *
 * @param scrollbar Target scrollbar.
 * @param out_thickness Pointer to receive thickness in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_scrollbar_get_thickness(
    const struct cupertino_scrollbar *scrollbar, float *out_thickness);

/**
 * @brief Retrieves current opacity of scrollbar capsule (0.0 to 1.0).
 *
 * @param scrollbar Target scrollbar.
 * @param out_opacity Pointer to receive opacity.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_scrollbar_get_opacity(
    const struct cupertino_scrollbar *scrollbar, float *out_opacity);

/**
 * @brief Retrieves computed thumb offset and length within track.
 *
 * @param scrollbar Target scrollbar.
 * @param out_offset Pointer to receive vertical offset from top.
 * @param out_size Pointer to receive vertical length of thumb.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_scrollbar_get_thumb_geometry(
    const struct cupertino_scrollbar *scrollbar, float *out_offset,
    float *out_size);

/**
 * @brief Updates safe area insets to prevent overlapping with bars.
 *
 * @param scrollbar Target scrollbar.
 * @param top Inset from screen top in points.
 * @param bottom Inset from screen bottom in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_scrollbar_set_safe_area_insets(struct cupertino_scrollbar *scrollbar,
                                         float top, float bottom);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SCROLLBAR_H */
