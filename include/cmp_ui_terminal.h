#ifndef CMP_UI_TERMINAL_H
#define CMP_UI_TERMINAL_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_terminal.h
 * \brief UI Component for rendering an ANSI terminal display.
 */

/**
 * \brief Opaque handle to a terminal UI component.
 */
typedef struct cmp_ui_terminal cmp_ui_terminal_t;

/**
 * \brief Creates a new terminal component.
 *
 * \param out_terminal Pointer to store the created terminal handle.
 * \param bg_color Background color of the terminal (ARGB).
 * \param fg_color Default foreground text color (ARGB).
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_terminal_create(cmp_ui_terminal_t **out_terminal,
                                   uint32_t bg_color, uint32_t fg_color);

/**
 * \brief Destroys a terminal component.
 *
 * \param terminal The component to destroy.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_terminal_destroy(cmp_ui_terminal_t *terminal);

/**
 * \brief Retrieves the underlying UI node.
 *
 * \param terminal The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_terminal_get_node(cmp_ui_terminal_t *terminal,
                                     cmp_ui_node_t **out_node);

/**
 * \brief Appends output text to the terminal.
 *
 * \param terminal The component.
 * \param output The string to append.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_terminal_append_output(cmp_ui_terminal_t *terminal,
                                          const char *output);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_TERMINAL_H */
