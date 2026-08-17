#ifndef UI_RICH_TEXT_EDITOR_BASE_H
#define UI_RICH_TEXT_EDITOR_BASE_H

/**
 * \file ui_rich_text_editor_base.h
 * \brief UI Rich Text Editor Base component.
 *
 * This file contains definitions for a rich text editor component.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

/**
 * @brief Opaque handle representing a rich text editor component.
 */
struct ui_rich_text_editor_base;

/**
 * @brief Creates a new unstyled rich text editor base component.
 *
 * @param out_rte Pointer to receive the allocated component.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_rich_text_editor_base_create(struct ui_rich_text_editor_base **out_rte,
                                struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a rich text editor base component.
 *
 * @param rte The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_rich_text_editor_base_destroy(struct ui_rich_text_editor_base *rte);

/**
 * @brief Gets the underlying component instance.
 *
 * @param rte The rich text editor component.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_rich_text_editor_base_get_component(struct ui_rich_text_editor_base *rte,
                                       struct ui_component **out_component);

/**
 * @brief Handles text insertion, updating the AST and caret position.
 *
 * @param rte The rich text editor component.
 * @param text The text to insert.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_rich_text_editor_base_insert_text(struct ui_rich_text_editor_base *rte,
                                     const char *text);

/**
 * @brief Maps screen coordinates to an exact caret position.
 *
 * @param rte The rich text editor component.
 * @param x Screen X coordinate.
 * @param y Screen Y coordinate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_editor_base_set_caret_from_point(
    struct ui_rich_text_editor_base *rte, float x, float y);

/**
 * @brief Performs an undo operation.
 *
 * @param rte The rich text editor component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_editor_base_undo(struct ui_rich_text_editor_base *rte);

/**
 * @brief Performs a redo operation.
 *
 * @param rte The rich text editor component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_editor_base_redo(struct ui_rich_text_editor_base *rte);

/**
 * @brief Starts IME composition.
 *
 * @param rte The rich text editor component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_rich_text_editor_base_ime_start(struct ui_rich_text_editor_base *rte);

/**
 * @brief Updates IME composition.
 *
 * @param rte The rich text editor component.
 * @param composition The ongoing composition text.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_rich_text_editor_base_ime_update(struct ui_rich_text_editor_base *rte,
                                    const char *composition);

/**
 * @brief Ends IME composition, confirming the final text.
 *
 * @param rte The rich text editor component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_rich_text_editor_base_ime_end(struct ui_rich_text_editor_base *rte);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RICH_TEXT_EDITOR_BASE_H */
