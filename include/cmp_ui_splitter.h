#ifndef CMP_UI_SPLITTER_H
#define CMP_UI_SPLITTER_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_splitter.h
 * \brief UI Component for rendering a resizable splitter pane.
 */

/**
 * \brief Opaque handle to a splitter UI component.
 */
typedef struct cmp_ui_splitter cmp_ui_splitter_t;

/**
 * \brief Creates a new splitter component.
 *
 * \param out_splitter Pointer to store the created splitter handle.
 * \param is_vertical Non-zero if the splitter divides vertically.
 * \param color Background color of the splitter divider (ARGB).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_splitter_create(cmp_ui_splitter_t **out_splitter,
                                   int is_vertical, uint32_t color);

/**
 * \brief Destroys a splitter component.
 *
 * \param splitter The splitter component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_splitter_destroy(cmp_ui_splitter_t *splitter);

/**
 * \brief Retrieves the underlying UI node for the splitter.
 *
 * \param splitter The splitter component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_splitter_get_node(cmp_ui_splitter_t *splitter,
                                     cmp_ui_node_t **out_node);

/**
 * \brief Updates the splitter position.
 *
 * \param splitter The splitter component.
 * \param position The new position (0.0f to 1.0f).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_splitter_set_position(cmp_ui_splitter_t *splitter,
                                         float position);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_SPLITTER_H */
