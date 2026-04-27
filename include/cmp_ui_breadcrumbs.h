#ifndef CMP_UI_BREADCRUMBS_H
#define CMP_UI_BREADCRUMBS_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_breadcrumbs.h
 * \brief UI Component for rendering navigation breadcrumbs.
 */

/**
 * \brief Opaque handle to a breadcrumbs UI component.
 */
typedef struct cmp_ui_breadcrumbs cmp_ui_breadcrumbs_t;

/**
 * \brief Creates a new breadcrumbs component.
 *
 * \param out_breadcrumbs Pointer to store the created breadcrumbs handle.
 * \param bg_color Background color of the breadcrumbs (ARGB).
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_breadcrumbs_create(cmp_ui_breadcrumbs_t **out_breadcrumbs,
                                      uint32_t bg_color);

/**
 * \brief Destroys a breadcrumbs component.
 *
 * \param breadcrumbs The breadcrumbs component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_breadcrumbs_destroy(cmp_ui_breadcrumbs_t *breadcrumbs);

/**
 * \brief Retrieves the underlying UI node for the breadcrumbs.
 *
 * \param breadcrumbs The breadcrumbs component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_breadcrumbs_get_node(cmp_ui_breadcrumbs_t *breadcrumbs,
                                        cmp_ui_node_t **out_node);

/**
 * \brief Appends a path segment to the breadcrumbs.
 *
 * \param breadcrumbs The breadcrumbs component.
 * \param segment The segment label text.
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_breadcrumbs_add_segment(cmp_ui_breadcrumbs_t *breadcrumbs,
                                           const char *segment);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_BREADCRUMBS_H */
