/**
 * @file cupertino_page_control.h
 * @brief Cupertino Page Control (dot indicator) conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_PAGE_CONTROL_H
#define CUPERTINO_CUPERTINO_PAGE_CONTROL_H

/* clang-format off */
#include "ui_page_control_base.h"
#include "ui_color_space.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_PAGE_CONTROL_DOT_DIAMETER 7.0f
#define CUPERTINO_PAGE_CONTROL_DOT_SPACING 9.0f
#define CUPERTINO_PAGE_CONTROL_HEIGHT 28.0f
#define CUPERTINO_PAGE_CONTROL_MAX_UNCOMPRESSED_DOTS 10

/**
 * @struct cupertino_page_control_descriptor
 * @brief Configuration descriptor for creating a Cupertino page control.
 */
struct cupertino_page_control_descriptor {
  int number_of_pages;       /**< Total count of pages (>= 0). */
  int current_page;          /**< Initial active page index (0-based). */
  int hides_for_single_page; /**< 1 to automatically hide when pages <= 1. */
  ui_color_t page_indicator_tint_color;         /**< Inactive dot color. */
  ui_color_t current_page_indicator_tint_color; /**< Active dot color. */
};

/**
 * @struct cupertino_page_control
 * @brief Cupertino page control instance wrapping ui_page_control_base.
 */
struct cupertino_page_control {
  struct ui_page_control_base *base; /**< Wrapped CDK page control primitive. */
  int number_of_pages;               /**< Total page count. */
  int current_page;                  /**< Active page index (0-based). */
  int hides_for_single_page;         /**< Auto-hide on single page flag. */
  int is_scrubbing;                  /**< 1 during drag/scrub gesture. */
  float scrub_touch_x;               /**< Last touch X coordinate. */
  ui_color_t page_indicator_tint_color;         /**< Inactive dot color. */
  ui_color_t current_page_indicator_tint_color; /**< Active dot color. */
};

/**
 * @brief Creates a new Cupertino page control instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_control Pointer to receive newly created page control.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_page_control_create(
    struct ui_engine *engine,
    const struct cupertino_page_control_descriptor *desc,
    struct cupertino_page_control **out_control);

/**
 * @brief Destroys a Cupertino page control instance.
 *
 * @param control Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_control_destroy(struct cupertino_page_control *control);

/**
 * @brief Sets the total number of pages.
 *
 * @param control Target page control.
 * @param count Number of pages (>= 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_control_set_number_of_pages(
    struct cupertino_page_control *control, int count);

/**
 * @brief Gets the total number of pages.
 *
 * @param control Target page control.
 * @param out_count Pointer to receive count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_control_get_number_of_pages(
    const struct cupertino_page_control *control, int *out_count);

/**
 * @brief Sets the currently active page index.
 *
 * @param control Target page control.
 * @param page Active page index (0-based).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_control_set_current_page(struct cupertino_page_control *control,
                                        int page);

/**
 * @brief Gets the currently active page index.
 *
 * @param control Target page control.
 * @param out_page Pointer to receive active page index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_control_get_current_page(
    const struct cupertino_page_control *control, int *out_page);

/**
 * @brief Interactive touch scrubbing across page dots.
 *
 * @param control Target page control.
 * @param touch_x Touch X coordinate relative to page control origin.
 * @param out_page_changed Pointer to receive 1 if page changed, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_control_scrub(struct cupertino_page_control *control,
                             float touch_x, int *out_page_changed);

/**
 * @brief Ends the interactive scrubbing gesture.
 *
 * @param control Target page control.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_control_end_scrub(struct cupertino_page_control *control);

/**
 * @brief Computes scale factor for a given dot (supports page limit
 * compression).
 *
 * @param control Target page control.
 * @param dot_index Index of the dot (0 to number_of_pages - 1).
 * @param out_scale Pointer to receive dot scale multiplier [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_page_control_get_dot_scale(
    const struct cupertino_page_control *control, int dot_index,
    float *out_scale);

/**
 * @brief Retrieves bounding dimensions for the page control container.
 *
 * @param control Target page control.
 * @param out_width Pointer to receive total width.
 * @param out_height Pointer to receive total height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_page_control_get_dimensions(
    const struct cupertino_page_control *control, float *out_width,
    float *out_height);

/**
 * @brief Retrieves the underlying CDK page control base.
 *
 * @param control Target page control.
 * @param out_base Pointer to receive ui_page_control_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_page_control_get_base(struct cupertino_page_control *control,
                                struct ui_page_control_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_PAGE_CONTROL_H */
