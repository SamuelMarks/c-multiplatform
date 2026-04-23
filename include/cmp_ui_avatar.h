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
 * ile cmp_ui_avatar.h
 * rief UI Component for rendering a user avatar with initials.
 */

/**
 * rief Opaque handle to an avatar UI component.
 */
typedef struct cmp_ui_avatar cmp_ui_avatar_t;

/**
 * rief Creates a new avatar component.
 *
 * \param out_avatar Pointer to store the created avatar handle.
 * \param initials The initial text to display.
 * \param bg_color Background color of the avatar (ARGB).
 * \param text_color Text color of the avatar (ARGB).
 *
eturn 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_avatar_create(cmp_ui_avatar_t **out_avatar,
                                 const char *initials, uint32_t bg_color,
                                 uint32_t text_color);

/**
 * rief Destroys an avatar component.
 *
 * \param avatar The avatar component to destroy.
 *
eturn 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_avatar_destroy(cmp_ui_avatar_t *avatar);

/**
 * rief Retrieves the underlying UI node for the avatar.
 *
 * \param avatar The avatar component.
 * \param out_node Pointer to store the UI node.
 *
eturn 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_avatar_get_node(cmp_ui_avatar_t *avatar,
                                   cmp_ui_node_t **out_node);

/**
 * rief Updates the initials displayed in the avatar.
 *
 * \param avatar The avatar component.
 * \param initials The new initials to display.
 *
eturn 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_avatar_set_initials(cmp_ui_avatar_t *avatar,
                                       const char *initials);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_AVATAR_H */
