#ifndef CMP_UI_AVATAR_H
#define CMP_UI_AVATAR_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * \file cmp_ui_avatar.h
 * \brief UI Component for rendering a user avatar with initials.
 */

/**
 * \brief Opaque handle to an avatar UI component.
 */
typedef struct cmp_ui_avatar cmp_ui_avatar_t;

/**
 * \brief Creates a new avatar component.
 *
 * \param out_avatar Pointer to store the created avatar handle.
 * \param initials The initial text to display.
 * \param bg_color Background color of the avatar (ARGB).
 * \param text_color Text color of the avatar (ARGB).
 *
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_avatar_create(cmp_ui_avatar_t **out_avatar,
                                 const char *initials, uint32_t bg_color,
                                 uint32_t text_color);

/**
 * \brief Destroys an avatar component.
 *
 * \param avatar The avatar component to destroy.
 *
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_avatar_destroy(cmp_ui_avatar_t *avatar);

/**
 * \brief Retrieves the underlying UI node for the avatar.
 *
 * \param avatar The avatar component.
 * \param out_node Pointer to store the UI node.
 *
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_avatar_get_node(cmp_ui_avatar_t *avatar,
                                   cmp_ui_node_t **out_node);

/**
 * \brief Updates the initials displayed in the avatar.
 *
 * \param avatar The avatar component.
 * \param initials The new initials to display.
 *
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_avatar_set_initials(cmp_ui_avatar_t *avatar,
                                       const char *initials);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_AVATAR_H */
