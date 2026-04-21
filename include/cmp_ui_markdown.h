#ifndef CMP_UI_MARKDOWN_H
#define CMP_UI_MARKDOWN_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_markdown.h
 * \brief UI Component for rendering markdown text.
 */

/**
 * \brief Opaque handle to a markdown UI component.
 */
typedef struct cmp_ui_markdown cmp_ui_markdown_t;

/**
 * \brief Creates a new markdown component.
 *
 * \param out_md Pointer to store the created markdown handle.
 * \param text The markdown text to render.
 * \param bg_color Background color (ARGB).
 * \param text_color Default text color (ARGB).
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_markdown_create(cmp_ui_markdown_t **out_md, const char *text,
                                   uint32_t bg_color, uint32_t text_color);

/**
 * \brief Destroys a markdown component.
 *
 * \param md The component to destroy.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_markdown_destroy(cmp_ui_markdown_t *md);

/**
 * \brief Retrieves the underlying UI node.
 *
 * \param md The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_markdown_get_node(cmp_ui_markdown_t *md,
                                     cmp_ui_node_t **out_node);

/**
 * \brief Updates the markdown text.
 *
 * \param md The component.
 * \param text The new markdown text.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_markdown_set_text(cmp_ui_markdown_t *md, const char *text);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_MARKDOWN_H */
