#ifndef CMP_UI_CODE_BLOCK_H
#define CMP_UI_CODE_BLOCK_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_code_block.h
 * \brief UI Component for rendering a block of source code.
 */

/**
 * \brief Opaque handle to a code block UI component.
 */
typedef struct cmp_ui_code_block cmp_ui_code_block_t;

/**
 * \brief Creates a new code block component.
 *
 * \param out_block Pointer to store the created code block handle.
 * \param code The source code text.
 * \param language The programming language for syntax highlighting (can be
 * NULL).
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_code_block_create(cmp_ui_code_block_t **out_block,
                                     const char *code, const char *language);

/**
 * \brief Destroys a code block component.
 *
 * \param block The component to destroy.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_code_block_destroy(cmp_ui_code_block_t *block);

/**
 * \brief Retrieves the underlying UI node.
 *
 * \param block The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_code_block_get_node(cmp_ui_code_block_t *block,
                                       cmp_ui_node_t **out_node);

/**
 * \brief Updates the code text.
 *
 * \param block The component.
 * \param code The new source code text.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_code_block_set_code(cmp_ui_code_block_t *block,
                                       const char *code);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_CODE_BLOCK_H */
