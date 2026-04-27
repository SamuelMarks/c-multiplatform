#ifndef CMP_UI_SEGMENTED_BUTTON_H
#define CMP_UI_SEGMENTED_BUTTON_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_segmented_button.h
 * \brief UI Component for rendering a Segmented Button (M3).
 */

/**
 * \brief Opaque handle to a Segmented Button UI component.
 */
typedef struct cmp_ui_segmented_button cmp_ui_segmented_button_t;

/**
 * \brief Creates a new Segmented Button component.
 *
 * \param out_btn Pointer to store the created component handle.
 * \param multi_select 1 to allow multiple selections, 0 for single selection.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_segmented_button_create(cmp_ui_segmented_button_t **out_btn,
                                           int multi_select);

/**
 * \brief Destroys a Segmented Button component.
 *
 * \param btn The component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_segmented_button_destroy(cmp_ui_segmented_button_t *btn);

/**
 * \brief Retrieves the underlying UI node for the segmented button group.
 *
 * \param btn The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_segmented_button_get_node(cmp_ui_segmented_button_t *btn,
                                             cmp_ui_node_t **out_node);

/**
 * \brief Adds a segment to the button.
 *
 * \param btn The segmented button component.
 * \param label The text label of the segment.
 * \param icon_name Optional icon name (can be NULL).
 * \param out_segment_index Receives the index of the newly added segment.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_segmented_button_add_segment(cmp_ui_segmented_button_t *btn,
                                                const char *label,
                                                const char *icon_name,
                                                int *out_segment_index);

/**
 * \brief Selects a segment.
 *
 * \param btn The segmented button component.
 * \param index The index of the segment to select.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_segmented_button_set_selected(cmp_ui_segmented_button_t *btn,
                                                 int index);

/**
 * \brief Deselects a segment.
 *
 * \param btn The segmented button component.
 * \param index The index of the segment to deselect.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_segmented_button_set_deselected(
    cmp_ui_segmented_button_t *btn, int index);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_segmented_button_bind_a11y(cmp_ui_segmented_button_t *widget,
                                              cmp_a11y_tree_t *tree);
#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_SEGMENTED_BUTTON_H */