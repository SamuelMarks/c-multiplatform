#ifndef CMP_UI_BADGE_H
#define CMP_UI_BADGE_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_badge.h
 * \brief UI Component for rendering a small badge (e.g., notification count).
 */

/**
 * \brief Opaque handle to a badge UI component.
 */
typedef struct cmp_ui_badge cmp_ui_badge_t;

/**
 * \brief Creates a new badge component.
 *
 * \param out_badge Pointer to store the created badge handle.
 * \param text The initial text to display in the badge.
 * \param bg_color Background color of the badge (ARGB).
 * \param text_color Text color of the badge (ARGB).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_badge_create(cmp_ui_badge_t **out_badge, const char *text,
                                uint32_t bg_color, uint32_t text_color);

/**
 * \brief Destroys a badge component.
 *
 * \param badge The badge component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_badge_destroy(cmp_ui_badge_t *badge);

/**
 * \brief Retrieves the underlying UI node for the badge.
 *
 * \param badge The badge component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_badge_get_node(cmp_ui_badge_t *badge,
                                  cmp_ui_node_t **out_node);

/**
 * \brief Updates the text displayed in the badge.
 *
 * \param badge The badge component.
 * \param text The new text to display.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_badge_set_text(cmp_ui_badge_t *badge, const char *text);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_BADGE_H */
