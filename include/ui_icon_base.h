#ifndef UI_ICON_BASE_H
#define UI_ICON_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_font_manager.h"
#include "ui_svg.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Represents the internal backing type of an icon.
 */
enum ui_icon_type {
  UI_ICON_TYPE_UNSET,   /**< Icon is empty / unset */
  UI_ICON_TYPE_FONT,    /**< Icon backed by a font glyph */
  UI_ICON_TYPE_SVG_PATH /**< Icon backed by raw SVG path data */
};

struct ui_icon_base;

/**
 * @brief Creates a new unstyled icon base component.
 *
 * @param out_icon Pointer to receive the allocated icon base.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY on allocation
 * failure.
 */
ui_error_t ui_icon_base_create(struct ui_icon_base **out_icon);

/**
 * @brief Destroys an icon base component and frees all resources.
 *
 * @param icon The icon to destroy.
 */
ui_error_t ui_icon_base_destroy(struct ui_icon_base *icon);

/**
 * @brief Sets the icon to use a font glyph (e.g., Material Icons).
 *
 * Copies the identifier string safely.
 *
 * @param icon The icon.
 * @param font The loaded font handle containing the glyph.
 * @param glyph_name_or_code The identifier for the glyph (e.g., "home" or
 * "\e800").
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointers,
 * UI_ERROR_OUT_OF_MEMORY on allocation failure.
 */
ui_error_t ui_icon_base_set_font_glyph(struct ui_icon_base *icon,
                                       struct ui_font *font,
                                       const char *glyph_name_or_code);

/**
 * @brief Sets the icon to use raw SVG path data.
 *
 * Copies the path data string safely.
 *
 * @param icon The icon.
 * @param svg_path_data The raw SVG path data string (e.g., "M10 20v-6h4v6...").
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer,
 * UI_ERROR_OUT_OF_MEMORY on allocation failure.
 */
ui_error_t ui_icon_base_set_svg_path(struct ui_icon_base *icon,
                                     const char *svg_path_data);

/**
 * @brief Gets the current backing type of the icon.
 *
 * @param icon The icon.
 * @param out_type Pointer to receive the type.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_icon_base_get_type(const struct ui_icon_base *icon,
                                 enum ui_icon_type *out_type);

/**
 * @brief Gets the string identifier data configured for the icon.
 *
 * For font icons, this is the glyph name/code.
 * For SVG icons, this is the raw path data string.
 *
 * @param icon The icon.
 * @param out_data Pointer to receive the string data (owned by the icon).
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_icon_base_get_data(const struct ui_icon_base *icon,
                                 const char **out_data);

/**
 * @brief Binds the name property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_icon_base_bind_name(struct ui_icon_base *widget,
                                  struct ui_signal *signal);

#ifdef __cplusplus
}
#endif

#endif /* UI_ICON_BASE_H */
