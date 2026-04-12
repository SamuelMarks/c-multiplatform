#ifndef CMP_UI_ACCORDION_H
#define CMP_UI_ACCORDION_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_accordion.h
 * \brief UI Component for rendering a collapsible accordion panel.
 */

/**
 * \brief Opaque handle to an accordion UI component.
 */
typedef struct cmp_ui_accordion cmp_ui_accordion_t;

/**
 * \brief Creates a new accordion component.
 *
 * \param out_accordion Pointer to store the created accordion handle.
 * \param title The title text.
 * \param bg_color Background color of the accordion (ARGB).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_accordion_create(cmp_ui_accordion_t **out_accordion,
                                    const char *title, uint32_t bg_color);

/**
 * \brief Destroys an accordion component.
 *
 * \param accordion The accordion component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_accordion_destroy(cmp_ui_accordion_t *accordion);

/**
 * \brief Retrieves the underlying UI node for the accordion.
 *
 * \param accordion The accordion component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_accordion_get_node(cmp_ui_accordion_t *accordion,
                                      cmp_ui_node_t **out_node);

/**
 * \brief Toggles the accordion's expanded state.
 *
 * \param accordion The accordion component.
 * \param is_expanded Non-zero to expand, zero to collapse.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_accordion_set_expanded(cmp_ui_accordion_t *accordion,
                                          int is_expanded);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_ACCORDION_H */
