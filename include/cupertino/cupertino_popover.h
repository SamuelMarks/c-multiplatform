/**
 * @file cupertino_popover.h
 * @brief Cupertino Popover with anchored pointer arrow conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_POPOVER_H
#define CUPERTINO_CUPERTINO_POPOVER_H

/* clang-format off */
#include "ui_error.h"
#include "ui_popover_base.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_POPOVER_ARROW_HEIGHT 12.0f
#define CUPERTINO_POPOVER_ARROW_BASE_WIDTH 24.0f
#define CUPERTINO_POPOVER_CORNER_RADIUS 13.0f
#define CUPERTINO_POPOVER_DEFAULT_WIDTH 280.0f
#define CUPERTINO_POPOVER_DEFAULT_HEIGHT 200.0f

/**
 * @enum cupertino_arrow_direction
 * @brief Direction the popover arrow points towards the anchor.
 */
enum cupertino_arrow_direction {
  CUPERTINO_ARROW_DIRECTION_UP =
      0, /**< Popover is below anchor, arrow points UP. */
  CUPERTINO_ARROW_DIRECTION_DOWN,  /**< Popover is above anchor, arrow points
                                      DOWN. */
  CUPERTINO_ARROW_DIRECTION_LEFT,  /**< Popover is right of anchor, arrow points
                                      LEFT. */
  CUPERTINO_ARROW_DIRECTION_RIGHT, /**< Popover is left of anchor, arrow points
                                      RIGHT. */
  CUPERTINO_ARROW_DIRECTION_ANY    /**< Automatic best fit direction. */
};

/**
 * @struct cupertino_popover_descriptor
 * @brief Configuration descriptor for creating a popover.
 */
struct cupertino_popover_descriptor {
  float content_width;  /**< Width of popover body. */
  float content_height; /**< Height of popover body. */
  enum cupertino_arrow_direction permitted_arrows; /**< Permitted directions. */
};

/**
 * @struct cupertino_popover
 * @brief Cupertino popover instance.
 */
struct cupertino_popover {
  struct ui_popover_base *base;
  float content_width;
  float content_height;
  enum cupertino_arrow_direction permitted_arrows;
  enum cupertino_arrow_direction actual_arrow_direction;
  float arrow_offset; /**< Offset of arrow center along popover edge. */
  float bounds_x;
  float bounds_y;
  float bounds_w;
  float bounds_h;
  int is_presented;
};

/**
 * @brief Creates a new Cupertino Popover instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_popover Pointer to receive newly created popover instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popover_create(
    struct ui_engine *engine, const struct cupertino_popover_descriptor *desc,
    struct cupertino_popover **out_popover);

/**
 * @brief Destroys a Cupertino Popover instance.
 *
 * @param popover Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_popover_destroy(struct cupertino_popover *popover);

/**
 * @brief Presents popover anchored to a source rectangle with screen collision
 * math.
 *
 * @param popover Target popover.
 * @param anchor_x Top-left X of originating view/button.
 * @param anchor_y Top-left Y of originating view/button.
 * @param anchor_w Width of originating view/button.
 * @param anchor_h Height of originating view/button.
 * @param screen_w Screen width for boundary collision clamping.
 * @param screen_h Screen height for boundary collision clamping.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popover_present(
    struct cupertino_popover *popover, float anchor_x, float anchor_y,
    float anchor_w, float anchor_h, float screen_w, float screen_h);

/**
 * @brief Dismisses the popover.
 *
 * @param popover Target popover.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_popover_dismiss(struct cupertino_popover *popover);

/**
 * @brief Checks if popover is currently presented on screen.
 *
 * @param popover Target popover.
 * @param out_is_presented Pointer to receive presentation state (1 or 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popover_is_presented(
    const struct cupertino_popover *popover, int *out_is_presented);

/**
 * @brief Gets calculated bounding rectangle of the popover (including arrow).
 *
 * @param popover Target popover.
 * @param out_x Pointer to receive top-left X coordinate.
 * @param out_y Pointer to receive top-left Y coordinate.
 * @param out_w Pointer to receive bounding width.
 * @param out_h Pointer to receive bounding height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popover_get_bounds(
    const struct cupertino_popover *popover, float *out_x, float *out_y,
    float *out_w, float *out_h);

/**
 * @brief Gets calculated arrow pointing direction and edge offset.
 *
 * @param popover Target popover.
 * @param out_direction Pointer to receive actual arrow direction.
 * @param out_offset Pointer to receive offset along the arrow edge.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popover_get_arrow_position(
    const struct cupertino_popover *popover,
    enum cupertino_arrow_direction *out_direction, float *out_offset);

/**
 * @brief Retrieves underlying CDK popover base.
 *
 * @param popover Target popover.
 * @param out_base Pointer to receive ui_popover_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_popover_get_base(
    struct cupertino_popover *popover, struct ui_popover_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_POPOVER_H */
