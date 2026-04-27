#ifndef CMP_UI_DIFF_H
#define CMP_UI_DIFF_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_diff.h
 * \brief UI Component for rendering a side-by-side or inline diff.
 */

/**
 * \brief Opaque handle to a diff UI component.
 */
typedef struct cmp_ui_diff cmp_ui_diff_t;

/**
 * \brief Creates a new diff component.
 *
 * \param out_diff Pointer to store the created diff handle.
 * \param old_text The original text.
 * \param new_text The updated text.
 * \return 0 on success, or a non-zero error code.
 */
int CMP_API cmp_ui_diff_create(cmp_ui_diff_t **out_diff, const char *old_text,
                               const char *new_text);

/**
 * \brief Destroys a diff component.
 *
 * \param diff The component to destroy.
 * \return 0 on success, or a non-zero error code.
 */
int CMP_API cmp_ui_diff_destroy(cmp_ui_diff_t *diff);

/**
 * \brief Retrieves the underlying UI node.
 *
 * \param diff The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code.
 */
int CMP_API cmp_ui_diff_get_node(cmp_ui_diff_t *diff, cmp_ui_node_t **out_node);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_DIFF_H */
