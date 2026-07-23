#ifndef UI_DATAGRID_BASE_H
#define UI_DATAGRID_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

struct ui_datagrid_base;

/**
 * @brief Creates a new unstyled datagrid base component.
 *
 * @param out_datagrid Pointer to receive the allocated component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_datagrid_base_create(struct ui_datagrid_base **out_datagrid);

/**
 * @brief Destroys a datagrid base component.
 *
 * @param datagrid The component to destroy.
 */
void ui_datagrid_base_destroy(struct ui_datagrid_base *datagrid);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param datagrid The datagrid component.
 * @return The underlying component.
 */
enum ui_error
ui_datagrid_base_get_component(struct ui_datagrid_base *datagrid,
                               struct ui_component **out_component);

/**
 * @brief Handles manual column resizing via drag events.
 *
 * @param datagrid The datagrid component.
 * @param col_index The index of the column being resized.
 * @param new_width The new calculated width in pixels.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_datagrid_base_resize_column(struct ui_datagrid_base *datagrid,
                                             int col_index, float new_width);

/**
 * @brief Moves the 2D roving tabindex focus cell.
 *
 * @param datagrid The datagrid component.
 * @param row_delta Vertical movement (-1 for up, 1 for down, 0 for none).
 * @param col_delta Horizontal movement (-1 for left, 1 for right, 0 for none).
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_datagrid_base_move_focus(struct ui_datagrid_base *datagrid,
                                          int row_delta, int col_delta);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_datagrid_base_bind_data(struct ui_datagrid_base *widget,
                                         struct ui_computed *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DATAGRID_BASE_H */
