/**
 * @file cupertino_page_scaffold.h
 * @brief Cupertino Page Scaffold (CupertinoPageScaffold) screen host container
 * conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_PAGE_SCAFFOLD_H
#define CUPERTINO_CUPERTINO_PAGE_SCAFFOLD_H

/* clang-format off */
#include "ui_error.h"
#include "ui_scaffold_base.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum cupertino_scaffold_background
 * @brief Background fill style for the page scaffold.
 */
enum cupertino_scaffold_background {
  CUPERTINO_SCAFFOLD_BG_SYSTEM =
      0, /**< Primary SystemBackground canvas fill. */
  CUPERTINO_SCAFFOLD_BG_GROUPED =
      1 /**< Grouped SystemGroupedBackground fill. */
};

/**
 * @struct cupertino_safe_area_insets
 * @brief Edge insets representing notch, Dynamic Island, and home indicator
 * boundaries.
 */
struct cupertino_safe_area_insets {
  float top;    /**< Top safe area inset (e.g. notch / island). */
  float bottom; /**< Bottom safe area inset (e.g. home indicator, ~34pt). */
  float left;   /**< Left safe area inset (e.g. landscape sensor cutout). */
  float right;  /**< Right safe area inset. */
};

/**
 * @struct cupertino_page_scaffold_descriptor
 * @brief Configuration descriptor for creating a Cupertino Page Scaffold.
 */
struct cupertino_page_scaffold_descriptor {
  enum cupertino_scaffold_background
      background_style;             /**< Canvas fill style. */
  int resize_to_avoid_bottom_inset; /**< Non-zero to avoid keyboard / bottom
                                       sheets. */
  struct cupertino_safe_area_insets
      safe_area_insets; /**< Initial safe area padding. */
  float bottom_inset;   /**< Transient bottom inset (e.g. active keyboard). */
  int is_dark;          /**< Non-zero for dark mode styling. */
};

/**
 * @struct cupertino_page_scaffold
 * @brief Cupertino Page Scaffold instance wrapping ui_scaffold_base.
 */
struct cupertino_page_scaffold {
  struct ui_scaffold_base *base; /**< CDK scaffold primitive. */
  enum cupertino_scaffold_background
      background_style;             /**< Canvas fill style. */
  int resize_to_avoid_bottom_inset; /**< Avoid keyboard flag. */
  struct cupertino_safe_area_insets
      safe_area_insets; /**< Hardware safe area insets. */
  float bottom_inset;   /**< Current bottom inset (e.g. keyboard height). */
  struct ui_component
      *nav_bar;              /**< Placed navigation bar component (or NULL). */
  struct ui_component *body; /**< Placed main body component (or NULL). */
  int is_dark;               /**< Dark mode appearance state. */
};

/**
 * @brief Creates a new Cupertino Page Scaffold.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_scaffold Pointer to receive newly created page scaffold.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_page_scaffold_create(
    struct ui_engine *engine,
    const struct cupertino_page_scaffold_descriptor *desc,
    struct cupertino_page_scaffold **out_scaffold);

/**
 * @brief Destroys a Cupertino Page Scaffold.
 *
 * @param scaffold Scaffold instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_scaffold_destroy(struct cupertino_page_scaffold *scaffold);

/**
 * @brief Sets navigation bar component in top slot.
 *
 * @param scaffold Target scaffold.
 * @param nav_bar Navigation bar component (or NULL to remove).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_page_scaffold_set_nav_bar(
    struct cupertino_page_scaffold *scaffold, struct ui_component *nav_bar);

/**
 * @brief Gets currently placed navigation bar component.
 *
 * @param scaffold Target scaffold.
 * @param out_nav_bar Pointer to receive navigation bar component pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_page_scaffold_get_nav_bar(
    const struct cupertino_page_scaffold *scaffold,
    struct ui_component **out_nav_bar);

/**
 * @brief Sets main body component in content slot.
 *
 * @param scaffold Target scaffold.
 * @param body Main content component (or NULL to remove).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_page_scaffold_set_body(
    struct cupertino_page_scaffold *scaffold, struct ui_component *body);

/**
 * @brief Gets currently placed main body component.
 *
 * @param scaffold Target scaffold.
 * @param out_body Pointer to receive body component pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_scaffold_get_body(const struct cupertino_page_scaffold *scaffold,
                                 struct ui_component **out_body);

/**
 * @brief Sets background fill style (System vs Grouped).
 *
 * @param scaffold Target scaffold.
 * @param background_style Background style.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_scaffold_set_background_style(
    struct cupertino_page_scaffold *scaffold,
    enum cupertino_scaffold_background background_style);

/**
 * @brief Gets current background fill style.
 *
 * @param scaffold Target scaffold.
 * @param out_style Pointer to receive background style.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_scaffold_get_background_style(
    const struct cupertino_page_scaffold *scaffold,
    enum cupertino_scaffold_background *out_style);

/**
 * @brief Sets keyboard avoidance preference (resizeToAvoidBottomInset).
 *
 * @param scaffold Target scaffold.
 * @param resize Non-zero to avoid keyboard inset, 0 to allow behind-keyboard
 * extension.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_scaffold_set_resize_to_avoid_bottom_inset(
    struct cupertino_page_scaffold *scaffold, int resize);

/**
 * @brief Gets keyboard avoidance preference.
 *
 * @param scaffold Target scaffold.
 * @param out_resize Pointer to receive flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_scaffold_get_resize_to_avoid_bottom_inset(
    const struct cupertino_page_scaffold *scaffold, int *out_resize);

/**
 * @brief Updates hardware safe area insets.
 *
 * @param scaffold Target scaffold.
 * @param insets Pointer to safe area insets structure.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_scaffold_set_safe_area_insets(
    struct cupertino_page_scaffold *scaffold,
    const struct cupertino_safe_area_insets *insets);

/**
 * @brief Gets current safe area insets.
 *
 * @param scaffold Target scaffold.
 * @param out_insets Pointer to receive safe area insets.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_scaffold_get_safe_area_insets(
    const struct cupertino_page_scaffold *scaffold,
    struct cupertino_safe_area_insets *out_insets);

/**
 * @brief Updates transient bottom inset (e.g. keyboard height).
 *
 * @param scaffold Target scaffold.
 * @param bottom_inset Inset height in points (must be >= 0.0f).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_scaffold_set_bottom_inset(
    struct cupertino_page_scaffold *scaffold, float bottom_inset);

/**
 * @brief Gets transient bottom inset.
 *
 * @param scaffold Target scaffold.
 * @param out_bottom_inset Pointer to receive bottom inset.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_scaffold_get_bottom_inset(
    const struct cupertino_page_scaffold *scaffold, float *out_bottom_inset);

/**
 * @brief Computes effective content padding for the body, accounting for safe
 * areas, navigation bar height, and keyboard avoidance.
 *
 * @param scaffold Target scaffold.
 * @param out_top_padding Pointer to receive effective top padding.
 * @param out_bottom_padding Pointer to receive effective bottom padding.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_scaffold_get_effective_padding(
    const struct cupertino_page_scaffold *scaffold, float *out_top_padding,
    float *out_bottom_padding);

/**
 * @brief Updates dark mode appearance state.
 *
 * @param scaffold Target scaffold.
 * @param is_dark Non-zero for dark mode, 0 for light mode.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_page_scaffold_set_dark_mode(
    struct cupertino_page_scaffold *scaffold, int is_dark);

/**
 * @brief Gets current dark mode state.
 *
 * @param scaffold Target scaffold.
 * @param out_is_dark Pointer to receive dark mode flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_page_scaffold_get_dark_mode(
    const struct cupertino_page_scaffold *scaffold, int *out_is_dark);

/**
 * @brief Retrieves underlying CDK scaffold base primitive.
 *
 * @param scaffold Target scaffold.
 * @param out_base Pointer to receive ui_scaffold_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_scaffold_get_base(struct cupertino_page_scaffold *scaffold,
                                 struct ui_scaffold_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_PAGE_SCAFFOLD_H */
