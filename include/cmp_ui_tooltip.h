#ifndef CMP_UI_TOOLTIP_H
#define CMP_UI_TOOLTIP_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_tooltip.h
 * \brief UI Component for rendering a hover tooltip.
 */

/**
 * \brief Opaque handle to a tooltip UI component.
 */
typedef struct cmp_ui_tooltip cmp_ui_tooltip_t;

/**
 * \brief Creates a new tooltip component.
 *
 * \param out_tooltip Pointer to store the created tooltip handle.
 * \param text The tooltip text.
 * \param bg_color Background color of the tooltip (ARGB).
 * \param text_color Text color of the tooltip (ARGB).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_tooltip_create(cmp_ui_tooltip_t **out_tooltip,
                                  const char *text, uint32_t bg_color,
                                  uint32_t text_color);

/**
 * \brief Destroys a tooltip component.
 *
 * \param tooltip The tooltip component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_tooltip_destroy(cmp_ui_tooltip_t *tooltip);

/**
 * \brief Retrieves the underlying UI node for the tooltip.
 *
 * \param tooltip The tooltip component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_tooltip_get_node(cmp_ui_tooltip_t *tooltip,
                                    cmp_ui_node_t **out_node);

/**
 * \brief Updates the text displayed in the tooltip.
 *
 * \param tooltip The tooltip component.
 * \param text The new text to display.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_tooltip_set_text(cmp_ui_tooltip_t *tooltip,
                                    const char *text);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_TOOLTIP_H */
