#ifndef UI_AVATAR_GROUP_BASE_H
#define UI_AVATAR_GROUP_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_avatar_base.h"
/* clang-format on */

struct ui_avatar_group_base;

/**
 * @brief Creates a new unstyled avatar group base component.
 *
 * @param out_group Pointer to receive the allocated avatar group base.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_avatar_group_base_create(struct ui_avatar_group_base **out_group);

/**
 * @brief Destroys an avatar group base component.
 *
 * @param group The group to destroy.
 */
void ui_avatar_group_base_destroy(struct ui_avatar_group_base *group);

/**
 * @brief Gets the underlying component for the avatar group.
 *
 * @param group The group.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_avatar_group_base_get_component(struct ui_avatar_group_base *group,
                                   struct ui_component **out_component);

/**
 * @brief Appends an avatar to the group.
 *
 * @param group The group.
 * @param avatar The avatar to append.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_avatar_group_base_append_avatar(struct ui_avatar_group_base *group,
                                   struct ui_avatar_base *avatar);

/**
 * @brief Sets the maximum number of avatars to visually display before
 * truncating.
 *
 * E.g., if set to 3, and 5 avatars are appended, only the first 3 will be
 * rendered individually, typically followed by a "+2" indicator (handled
 * visually by the layout engine).
 *
 * @param group The group.
 * @param max_avatars The maximum number to show. Set to 0 for unlimited.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_avatar_group_base_set_max_avatars(struct ui_avatar_group_base *group,
                                     unsigned int max_avatars);

/**
 * @brief Gets the current max avatars limit.
 *
 * @param group The group.
 * @param out_max_avatars Pointer to receive the max limit.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_avatar_group_base_get_max_avatars(struct ui_avatar_group_base *group,
                                     unsigned int *out_max_avatars);

/**
 * @brief Calculates the truncation remainder.
 *
 * This calculates (Total Avatars) - (Max Avatars) if max_avatars > 0.
 * Returns 0 if max_avatars is 0 or if the total is less than max_avatars.
 *
 * @param group The group.
 * @param out_remainder Pointer to receive the remainder count (e.g., the "N" in
 * "+N").
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_avatar_group_base_get_truncation_remainder(
    struct ui_avatar_group_base *group, unsigned int *out_remainder);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_avatar_group_base_bind_data(struct ui_avatar_group_base *widget,
                               struct ui_computed *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_AVATAR_GROUP_BASE_H */
