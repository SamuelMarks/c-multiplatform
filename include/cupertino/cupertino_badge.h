/**
 * @file cupertino_badge.h
 * @brief Cupertino Notification Badge conforming to Apple HIG specifications.
 */

#ifndef CUPERTINO_CUPERTINO_BADGE_H
#define CUPERTINO_CUPERTINO_BADGE_H

/* clang-format off */
#include "ui_badge_base.h"
#include "ui_color_space.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_BADGE_DEFAULT_MAX_COUNT 99
#define CUPERTINO_BADGE_STANDARD_HEIGHT 18.0f
#define CUPERTINO_BADGE_DOT_SIZE 8.0f

/**
 * @enum cupertino_badge_style
 * @brief Badge presentation style.
 */
enum cupertino_badge_style {
  CUPERTINO_BADGE_STYLE_STANDARD = 0, /**< Capsule pill with text or count. */
  CUPERTINO_BADGE_STYLE_DOT           /**< Minimal circular notification dot. */
};

/**
 * @struct cupertino_badge_descriptor
 * @brief Configuration descriptor for creating a Cupertino badge.
 */
struct cupertino_badge_descriptor {
  enum cupertino_badge_style style; /**< Visual badge style. */
  int count;               /**< Initial numeric count (0 for dot or empty). */
  int max_count;           /**< Threshold before '+' suffix (e.g. 99). */
  const char *custom_text; /**< Custom text override (or NULL). */
  int is_hidden;           /**< 1 if initially hidden, 0 if visible. */
};

/**
 * @struct cupertino_badge
 * @brief Cupertino badge instance wrapping ui_badge_base.
 */
struct cupertino_badge {
  struct ui_badge_base *base;       /**< Underlying CDK badge primitive. */
  enum cupertino_badge_style style; /**< Visual badge style. */
  int count;                        /**< Numeric count value. */
  int max_count;                    /**< Max numeric threshold. */
  char text[32];                    /**< Displayed text string. */
  int is_hidden;                    /**< Hidden visibility flag. */
  float width;                      /**< Computed bounding width in points. */
  float height;                     /**< Computed bounding height in points. */
  ui_color_t background_color;      /**< Pill background (SystemRed). */
  ui_color_t text_color;            /**< Text foreground (White). */
};

/**
 * @brief Creates a new Cupertino badge instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_badge Pointer to receive newly created badge instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_badge_create(
    struct ui_engine *engine, const struct cupertino_badge_descriptor *desc,
    struct cupertino_badge **out_badge);

/**
 * @brief Destroys a Cupertino badge instance.
 *
 * @param badge Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_badge_destroy(struct cupertino_badge *badge);

/**
 * @brief Sets the numeric count of the badge.
 *
 * @param badge Target badge.
 * @param count Numeric count value (>= 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_badge_set_count(struct cupertino_badge *badge, int count);

/**
 * @brief Gets the current numeric count of the badge.
 *
 * @param badge Target badge.
 * @param out_count Pointer to receive count value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_badge_get_count(const struct cupertino_badge *badge, int *out_count);

/**
 * @brief Sets a custom text string in the badge, overriding numeric formatting.
 *
 * @param badge Target badge.
 * @param text Custom string (or NULL to revert to empty).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_badge_set_text(struct cupertino_badge *badge, const char *text);

/**
 * @brief Gets the currently displayed text string of the badge.
 *
 * @param badge Target badge.
 * @param out_text Pointer to receive const pointer to cached text.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_badge_get_text(
    const struct cupertino_badge *badge, const char **out_text);

/**
 * @brief Sets the visibility of the badge.
 *
 * @param badge Target badge.
 * @param is_hidden 1 to hide, 0 to show.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_badge_set_hidden(struct cupertino_badge *badge, int is_hidden);

/**
 * @brief Retrieves the visibility state of the badge.
 *
 * @param badge Target badge.
 * @param out_hidden Pointer to receive hidden flag (1 or 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_badge_is_hidden(const struct cupertino_badge *badge, int *out_hidden);

/**
 * @brief Computes physical layout dimensions for the badge.
 *
 * @param badge Target badge.
 * @param out_width Pointer to receive bounding width.
 * @param out_height Pointer to receive bounding height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_badge_get_dimensions(
    const struct cupertino_badge *badge, float *out_width, float *out_height);

/**
 * @brief Retrieves the background and foreground colors of the badge.
 *
 * @param badge Target badge.
 * @param out_bg Pointer to receive background color.
 * @param out_fg Pointer to receive text foreground color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_badge_get_colors(const struct cupertino_badge *badge,
                           ui_color_t *out_bg, ui_color_t *out_fg);

/**
 * @brief Retrieves the underlying CDK badge primitive.
 *
 * @param badge Target badge.
 * @param out_base Pointer to receive ui_badge_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_badge_get_base(
    struct cupertino_badge *badge, struct ui_badge_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_BADGE_H */
