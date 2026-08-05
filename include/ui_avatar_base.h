#ifndef UI_AVATAR_BASE_H
#define UI_AVATAR_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_icon_base.h"
/* clang-format on */

/**
 * @brief Represents the active display mode of the avatar.
 */
enum ui_avatar_type {
  UI_AVATAR_TYPE_IMAGE,        /**< Displaying an image */
  UI_AVATAR_TYPE_INITIALS,     /**< Displaying extracted initials from name */
  UI_AVATAR_TYPE_FALLBACK_ICON /**< Displaying a fallback icon */
};

struct ui_avatar_base;

/**
 * @brief Creates a new unstyled avatar base component.
 *
 * @param out_avatar Pointer to receive the allocated avatar base.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY on allocation
 * failure.
 */
ui_error_t ui_avatar_base_create(struct ui_avatar_base **out_avatar);

/**
 * @brief Destroys an avatar base component.
 *
 * @param avatar The avatar to destroy.
 */
ui_error_t ui_avatar_base_destroy(struct ui_avatar_base *avatar);

/**
 * @brief Sets the name for the avatar and extracts initials.
 *
 * Safe string handling with UTF-8 awareness is used to extract up to two
 * initials (first and last word). Sets the type to UI_AVATAR_TYPE_INITIALS if
 * no image is set.
 *
 * @param avatar The avatar.
 * @param name The full name.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer,
 * UI_ERROR_OUT_OF_MEMORY on allocation failure.
 */
ui_error_t ui_avatar_base_set_name(struct ui_avatar_base *avatar,
                                   const char *name);

/**
 * @brief Gets the full name of the avatar.
 *
 * @param avatar The avatar.
 * @param out_name Pointer to receive the name.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_avatar_base_get_name(const struct ui_avatar_base *avatar,
                                   const char **out_name);

/**
 * @brief Gets the extracted initials of the avatar.
 *
 * @param avatar The avatar.
 * @param out_initials Pointer to receive the initials string.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_avatar_base_get_initials(const struct ui_avatar_base *avatar,
                                       const char **out_initials);

/**
 * @brief Sets the image URL for the avatar.
 *
 * Automatically updates the type to UI_AVATAR_TYPE_IMAGE.
 *
 * @param avatar The avatar.
 * @param image_url The URL or path to the image.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer,
 * UI_ERROR_OUT_OF_MEMORY on allocation failure.
 */
ui_error_t ui_avatar_base_set_image_url(struct ui_avatar_base *avatar,
                                        const char *image_url);

/**
 * @brief Gets the image URL of the avatar.
 *
 * @param avatar The avatar.
 * @param out_image_url Pointer to receive the image URL.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_avatar_base_get_image_url(const struct ui_avatar_base *avatar,
                                        const char **out_image_url);

/**
 * @brief Sets the fallback icon for the avatar.
 *
 * The avatar takes ownership of the icon component.
 * Sets the type to UI_AVATAR_TYPE_FALLBACK_ICON if no name or image is set.
 *
 * @param avatar The avatar.
 * @param icon The fallback icon component.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_avatar_base_set_fallback_icon(struct ui_avatar_base *avatar,
                                            struct ui_icon_base *icon);

/**
 * @brief Gets the fallback icon.
 *
 * @param avatar The avatar.
 * @param out_icon Pointer to receive the icon.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_avatar_base_get_fallback_icon(const struct ui_avatar_base *avatar,
                                            struct ui_icon_base **out_icon);

/**
 * @brief Gets the current active display type of the avatar.
 *
 * Prioritizes Image > Initials > Fallback Icon.
 *
 * @param avatar The avatar.
 * @param out_type Pointer to receive the type.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_avatar_base_get_type(const struct ui_avatar_base *avatar,
                                   enum ui_avatar_type *out_type);

/**
 * @brief Binds the src property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_avatar_base_bind_src(struct ui_avatar_base *widget,
                                   struct ui_signal *signal);

#ifdef __cplusplus
}
#endif

#endif /* UI_AVATAR_BASE_H */
