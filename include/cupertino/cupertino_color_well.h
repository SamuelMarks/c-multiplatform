/**
 * @file cupertino_color_well.h
 * @brief macOS/iPadOS Color Well Swatch Control (NSColorWell) conforming to
 * Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_COLOR_WELL_H
#define CUPERTINO_CUPERTINO_COLOR_WELL_H

/* clang-format off */
#include "ui_color_picker_base.h"
#include "ui_color_space.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_COLOR_WELL_DEFAULT_WIDTH 44.0f
#define CUPERTINO_COLOR_WELL_DEFAULT_HEIGHT 24.0f
#define CUPERTINO_COLOR_WELL_CORNER_RADIUS 5.0f

/**
 * @struct cupertino_color_well_descriptor
 * @brief Configuration descriptor for Color Well control.
 */
struct cupertino_color_well_descriptor {
  ui_color_t initial_color; /**< Starting selected color. */
  float width;              /**< Swatch width (default 44pt). */
  float height;             /**< Swatch height (default 24pt). */
};

/**
 * @struct cupertino_color_well
 * @brief Cupertino Color Well instance.
 */
struct cupertino_color_well {
  ui_color_t current_color;
  int is_panel_open;
  int is_dragging;
  float width;
  float height;
  struct ui_color_picker_base *base;
  struct ui_control_value_accessor cva;
};

/**
 * @brief Creates a new Cupertino Color Well control.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_well Pointer to receive newly created color well instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_color_well_create(struct ui_engine *engine,
                            const struct cupertino_color_well_descriptor *desc,
                            struct cupertino_color_well **out_well);

/**
 * @brief Destroys a Cupertino Color Well instance.
 *
 * @param well Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_color_well_destroy(struct cupertino_color_well *well);

/**
 * @brief Sets current selected color in the color well.
 *
 * @param well Target color well.
 * @param color New color in ARGB format.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_color_well_set_color(
    struct cupertino_color_well *well, ui_color_t color);

/**
 * @brief Gets current selected color in the color well.
 *
 * @param well Target color well.
 * @param out_color Pointer to receive color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_color_well_get_color(
    const struct cupertino_color_well *well, ui_color_t *out_color);

/**
 * @brief Opens floating popover color palette panel.
 *
 * @param well Target color well.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_color_well_open_panel(struct cupertino_color_well *well);

/**
 * @brief Closes floating popover color palette panel.
 *
 * @param well Target color well.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_color_well_close_panel(struct cupertino_color_well *well);

/**
 * @brief Checks if color panel popover is open.
 *
 * @param well Target color well.
 * @param out_is_open Pointer to receive 1 if open, 0 if closed.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_color_well_is_panel_open(
    const struct cupertino_color_well *well, int *out_is_open);

/**
 * @brief Initiates drag-and-drop operation of color swatch.
 *
 * @param well Target color well.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_color_well_start_drag(struct cupertino_color_well *well);

/**
 * @brief Ends drag-and-drop operation.
 *
 * @param well Target color well.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_color_well_end_drag(struct cupertino_color_well *well);

/**
 * @brief Gets dimensions of the color well swatch button.
 *
 * @param well Target color well.
 * @param out_width Pointer to receive width.
 * @param out_height Pointer to receive height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_color_well_get_dimensions(const struct cupertino_color_well *well,
                                    float *out_width, float *out_height);

/**
 * @brief Gets Control Value Accessor for form integration.
 *
 * @param well Target color well.
 * @param out_cva Pointer to receive CVA handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_color_well_get_cva(struct cupertino_color_well *well,
                             struct ui_control_value_accessor **out_cva);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_COLOR_WELL_H */
