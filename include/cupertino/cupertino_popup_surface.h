/**
 * @file cupertino_popup_surface.h
 * @brief Cupertino Popup Surface (CupertinoPopupSurface) foundational modal
 * backdrop canvas conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_POPUP_SURFACE_H
#define CUPERTINO_CUPERTINO_POPUP_SURFACE_H

/* clang-format off */
#include "ui_elevation.h"
#include "ui_error.h"
#include "ui_surface_base.h"
#include "ui_vibrancy.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @brief Standard continuous squircle corner radius for Apple alert dialogs
 * (points).
 */
#define CUPERTINO_POPUP_SURFACE_RADIUS_ALERT 14.0f

/**
 * @brief Standard continuous squircle corner radius for Apple action sheets
 * (points).
 */
#define CUPERTINO_POPUP_SURFACE_RADIUS_ACTION_SHEET 12.0f

/**
 * @brief Standard continuous squircle corner radius for Apple popovers
 * (points).
 */
#define CUPERTINO_POPUP_SURFACE_RADIUS_POPOVER 13.0f

/**
 * @enum cupertino_popup_surface_preset
 * @brief Preset geometries and material styles for popup surfaces.
 */
enum cupertino_popup_surface_preset {
  CUPERTINO_POPUP_SURFACE_PRESET_ALERT =
      0, /**< Alert modal surface (14pt radius, thick blur). */
  CUPERTINO_POPUP_SURFACE_PRESET_ACTION_SHEET =
      1, /**< Action sheet surface (12pt radius, regular blur). */
  CUPERTINO_POPUP_SURFACE_PRESET_POPOVER =
      2, /**< Popover surface (13pt radius, regular blur). */
  CUPERTINO_POPUP_SURFACE_PRESET_CUSTOM =
      3 /**< Custom radius and elevation parameters. */
};

/**
 * @struct cupertino_popup_surface_descriptor
 * @brief Configuration descriptor for initializing a Cupertino popup surface.
 */
struct cupertino_popup_surface_descriptor {
  enum cupertino_popup_surface_preset preset; /**< Preset style. */
  float custom_corner_radius; /**< Radius when preset is CUSTOM. */
  int is_dark;                /**< Non-zero for dark mode styling. */
  int reduce_transparency;    /**< Non-zero if accessibility reduce transparency
                                 active. */
  float width;                /**< Surface width in points. */
  float height;               /**< Surface height in points. */
};

/**
 * @struct cupertino_popup_surface
 * @brief Foundational modal backdrop canvas instance wrapping ui_surface_base.
 */
struct cupertino_popup_surface {
  struct ui_surface_base *base;               /**< CDK surface primitive. */
  enum cupertino_popup_surface_preset preset; /**< Selected preset. */
  float corner_radius;     /**< Continuous G2 corner radius. */
  int is_dark;             /**< Dark mode state. */
  int reduce_transparency; /**< Reduce transparency state. */
  enum ui_vibrancy_material vibrancy_material; /**< Resolved blur material. */
  enum ui_elevation_level elevation;           /**< Resolved elevation level. */
  float shadow_blur;     /**< Diffuse shadow blur radius. */
  float shadow_spread;   /**< Diffuse shadow spread radius. */
  float shadow_opacity;  /**< Diffuse shadow opacity [0.0, 1.0]. */
  float shadow_offset_y; /**< Diffuse shadow Y offset. */
  float width;           /**< Width in points. */
  float height;          /**< Height in points. */
};

/**
 * @brief Creates a new Cupertino popup surface.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_surface Pointer to receive newly created popup surface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popup_surface_create(
    struct ui_engine *engine,
    const struct cupertino_popup_surface_descriptor *desc,
    struct cupertino_popup_surface **out_surface);

/**
 * @brief Destroys a Cupertino popup surface.
 *
 * @param surface Surface instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_popup_surface_destroy(struct cupertino_popup_surface *surface);

/**
 * @brief Sets the continuous corner radius of the popup surface.
 *
 * @param surface Target surface.
 * @param radius Corner radius in points (must be >= 0.0f).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_popup_surface_set_corner_radius(
    struct cupertino_popup_surface *surface, float radius);

/**
 * @brief Gets the continuous corner radius of the popup surface.
 *
 * @param surface Target surface.
 * @param out_radius Pointer to receive corner radius in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_popup_surface_get_corner_radius(
    const struct cupertino_popup_surface *surface, float *out_radius);

/**
 * @brief Updates dark mode appearance of the popup surface.
 *
 * @param surface Target surface.
 * @param is_dark Non-zero for dark mode, 0 for light mode.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popup_surface_set_dark_mode(
    struct cupertino_popup_surface *surface, int is_dark);

/**
 * @brief Gets current dark mode state.
 *
 * @param surface Target surface.
 * @param out_is_dark Pointer to receive dark mode flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popup_surface_get_dark_mode(
    const struct cupertino_popup_surface *surface, int *out_is_dark);

/**
 * @brief Updates reduce transparency accessibility override.
 *
 * @param surface Target surface.
 * @param reduce Non-zero to enforce opaque backdrop, 0 for frosted blur.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_popup_surface_set_reduce_transparency(
    struct cupertino_popup_surface *surface, int reduce);

/**
 * @brief Gets reduce transparency accessibility state.
 *
 * @param surface Target surface.
 * @param out_reduce Pointer to receive reduce transparency flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_popup_surface_get_reduce_transparency(
    const struct cupertino_popup_surface *surface, int *out_reduce);

/**
 * @brief Gets resolved vibrancy blur material.
 *
 * @param surface Target surface.
 * @param out_material Pointer to receive ui_vibrancy_material enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_popup_surface_get_vibrancy_material(
    const struct cupertino_popup_surface *surface,
    enum ui_vibrancy_material *out_material);

/**
 * @brief Gets current elevation level.
 *
 * @param surface Target surface.
 * @param out_elevation Pointer to receive elevation level.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popup_surface_get_elevation(
    const struct cupertino_popup_surface *surface,
    enum ui_elevation_level *out_elevation);

/**
 * @brief Sets elevation level and recalculates diffuse shadow falloff
 * parameters.
 *
 * @param surface Target surface.
 * @param elevation New elevation level.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popup_surface_set_elevation(
    struct cupertino_popup_surface *surface, enum ui_elevation_level elevation);

/**
 * @brief Gets diffuse elevation shadow parameters (low-spread soft falloff).
 *
 * @param surface Target surface.
 * @param out_blur Pointer to receive shadow blur radius.
 * @param out_spread Pointer to receive shadow spread radius.
 * @param out_opacity Pointer to receive shadow opacity [0.0, 1.0].
 * @param out_offset_y Pointer to receive vertical shadow offset.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_popup_surface_get_shadow_params(
    const struct cupertino_popup_surface *surface, float *out_blur,
    float *out_spread, float *out_opacity, float *out_offset_y);

/**
 * @brief Sets surface dimensions in points.
 *
 * @param surface Target surface.
 * @param width Width in points (must be >= 0.0f).
 * @param height Height in points (must be >= 0.0f).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popup_surface_set_dimensions(
    struct cupertino_popup_surface *surface, float width, float height);

/**
 * @brief Gets surface dimensions in points.
 *
 * @param surface Target surface.
 * @param out_width Pointer to receive width in points.
 * @param out_height Pointer to receive height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popup_surface_get_dimensions(
    const struct cupertino_popup_surface *surface, float *out_width,
    float *out_height);

/**
 * @brief Retrieves underlying CDK surface base primitive.
 *
 * @param surface Target surface.
 * @param out_base Pointer to receive ui_surface_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popup_surface_get_base(
    struct cupertino_popup_surface *surface, struct ui_surface_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_POPUP_SURFACE_H */
