/**
 * @file cupertino_spatial_ornament.h
 * @brief visionOS & iPadOS/macOS Floating Spatial Glass Ornaments conforming to
 * Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_SPATIAL_ORNAMENT_H
#define CUPERTINO_CUPERTINO_SPATIAL_ORNAMENT_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum cupertino_ornament_placement
 * @brief Floating placement edges relative to the host window canvas.
 */
enum cupertino_ornament_placement {
  CUPERTINO_ORNAMENT_PLACEMENT_TOP = 0, /**< Floating above the top frame. */
  CUPERTINO_ORNAMENT_PLACEMENT_BOTTOM,  /**< Floating below the bottom frame. */
  CUPERTINO_ORNAMENT_PLACEMENT_LEADING, /**< Floating outside leading edge. */
  CUPERTINO_ORNAMENT_PLACEMENT_TRAILING /**< Floating outside trailing edge. */
};

/**
 * @struct cupertino_spatial_ornament_descriptor
 * @brief Initialization descriptor for a spatial glass ornament.
 */
struct cupertino_spatial_ornament_descriptor {
  enum cupertino_ornament_placement placement; /**< Anchored edge placement. */
  float width;           /**< Desired ornament width in points. */
  float height;          /**< Desired ornament height in points. */
  float corner_radius;   /**< Continuous G2 corner radius in points. */
  float depth_offset_pt; /**< 3D spatial depth offset in points (e.g. 24pt). */
  float ambient_occlusion; /**< Ambient shadow occlusion coefficient [0.0,
                              1.0]. */
  float edge_gap; /**< Physical distance between window edge and ornament. */
};

/**
 * @struct cupertino_spatial_ornament
 * @brief Instance managing a floating spatial accessory bar with depth & AO.
 */
struct cupertino_spatial_ornament {
  enum cupertino_ornament_placement placement; /**< Edge placement. */
  float width;                                 /**< Ornament width in points. */
  float height;            /**< Ornament height in points. */
  float corner_radius;     /**< Continuous corner radius in points. */
  float depth_offset_pt;   /**< Spatial depth separation in points. */
  float ambient_occlusion; /**< Occlusion shading strength in [0.0, 1.0]. */
  float edge_gap;          /**< Spacing gap between frame and ornament. */
  int is_visible;          /**< Visibility flag. */
};

/**
 * @brief Creates a new spatial glass ornament instance.
 *
 * @param engine Pointer to ui_engine instance.
 * @param desc Configuration descriptor.
 * @param out_ornament Pointer to receive allocated ornament.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spatial_ornament_create(
    struct ui_engine *engine,
    const struct cupertino_spatial_ornament_descriptor *desc,
    struct cupertino_spatial_ornament **out_ornament);

/**
 * @brief Destroys a spatial glass ornament instance.
 *
 * @param ornament Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_spatial_ornament_destroy(struct cupertino_spatial_ornament *ornament);

/**
 * @brief Updates edge placement of the ornament.
 *
 * @param ornament Target ornament.
 * @param placement New edge placement.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_spatial_ornament_set_placement(
    struct cupertino_spatial_ornament *ornament,
    enum cupertino_ornament_placement placement);

/**
 * @brief Gets current edge placement.
 *
 * @param ornament Target ornament.
 * @param out_placement Pointer to receive placement.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_spatial_ornament_get_placement(
    const struct cupertino_spatial_ornament *ornament,
    enum cupertino_ornament_placement *out_placement);

/**
 * @brief Sets spatial depth offset and ambient occlusion factor.
 *
 * @param ornament Target ornament.
 * @param depth_offset_pt Distance in points along Z axis.
 * @param ambient_occlusion Occlusion factor in [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spatial_ornament_set_depth(
    struct cupertino_spatial_ornament *ornament, float depth_offset_pt,
    float ambient_occlusion);

/**
 * @brief Gets spatial depth and ambient occlusion.
 *
 * @param ornament Target ornament.
 * @param out_depth Pointer to receive depth offset in points.
 * @param out_occlusion Pointer to receive occlusion factor.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spatial_ornament_get_depth(
    const struct cupertino_spatial_ornament *ornament, float *out_depth,
    float *out_occlusion);

/**
 * @brief Sets ornament size.
 *
 * @param ornament Target ornament.
 * @param width Width in points.
 * @param height Height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spatial_ornament_set_size(
    struct cupertino_spatial_ornament *ornament, float width, float height);

/**
 * @brief Gets ornament size.
 *
 * @param ornament Target ornament.
 * @param out_width Pointer to receive width in points.
 * @param out_height Pointer to receive height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spatial_ornament_get_size(
    const struct cupertino_spatial_ornament *ornament, float *out_width,
    float *out_height);

/**
 * @brief Sets visibility state.
 *
 * @param ornament Target ornament.
 * @param is_visible 1 to display, 0 to hide.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spatial_ornament_set_visible(
    struct cupertino_spatial_ornament *ornament, int is_visible);

/**
 * @brief Gets visibility state.
 *
 * @param ornament Target ornament.
 * @param out_visible Pointer to receive visibility flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spatial_ornament_is_visible(
    const struct cupertino_spatial_ornament *ornament, int *out_visible);

/**
 * @brief Computes screen layout coordinates relative to host window bounds.
 *
 * @param ornament Target ornament.
 * @param window_x Window top-left X in points.
 * @param window_y Window top-left Y in points.
 * @param window_w Window width in points.
 * @param window_h Window height in points.
 * @param out_x Pointer to receive computed ornament X in points.
 * @param out_y Pointer to receive computed ornament Y in points.
 * @param out_w Pointer to receive computed ornament width in points.
 * @param out_h Pointer to receive computed ornament height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_spatial_ornament_compute_bounds(
    const struct cupertino_spatial_ornament *ornament, float window_x,
    float window_y, float window_w, float window_h, float *out_x, float *out_y,
    float *out_w, float *out_h);

/**
 * @brief Computes ambient occlusion shadow parameters based on spatial depth.
 *
 * @param ornament Target ornament.
 * @param out_shadow_offset_y Pointer to receive shadow Y offset in points.
 * @param out_shadow_radius Pointer to receive shadow blur radius in points.
 * @param out_shadow_alpha Pointer to receive shadow alpha fraction in [0.0,
 * 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_spatial_ornament_compute_shadow(
    const struct cupertino_spatial_ornament *ornament,
    float *out_shadow_offset_y, float *out_shadow_radius,
    float *out_shadow_alpha);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SPATIAL_ORNAMENT_H */
