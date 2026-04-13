#ifndef CMP_UI_TEXT_FIELD_H
#define CMP_UI_TEXT_FIELD_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_text_field.h
 * \brief UI Component for rendering a Text Field (with optional floating
 * label).
 */

/**
 * \brief Opaque handle to a Text Field UI component.
 */
typedef struct cmp_ui_text_field cmp_ui_text_field_t;

/**
 * \brief Creates a new Text Field component.
 *
 * \param out_field Pointer to store the created handle.
 * \param label The label text (acts as placeholder, floating label when
 * focused/filled).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_text_field_create(cmp_ui_text_field_t **out_field,
                                     const char *label);

/**
 * \brief Destroys a Text Field component.
 *
 * \param field The component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_text_field_destroy(cmp_ui_text_field_t *field);

/**
 * \brief Retrieves the underlying UI node for the Text Field container.
 *
 * \param field The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_text_field_get_node(cmp_ui_text_field_t *field,
                                       cmp_ui_node_t **out_node);

/**
 * \brief Updates the text value of the Text Field.
 *
 * \param field The component.
 * \param value The new value text.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_text_field_set_value(cmp_ui_text_field_t *field,
                                        const char *value);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_text_field_bind_a11y(cmp_ui_text_field_t *widget,
                                        cmp_a11y_tree_t *tree);

#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_TEXT_FIELD_H */