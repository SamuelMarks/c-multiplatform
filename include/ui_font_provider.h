/**
 * @file ui_font_provider.h
 * @brief System font discovery interface for native platforms.
 */

#ifndef UI_FONT_PROVIDER_H
#define UI_FONT_PROVIDER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "ui_error.h"
#include "ui_font_manager.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Locates and loads a native system font into the font manager.
 *
 * Discovers system-installed fonts matching the requested family name
 * (e.g. "San Francisco", "Arial", "Segoe UI", "Roboto", "sans-serif").
 *
 * @param[in,out] manager Target font manager.
 * @param[in] family_name Desired font family or generic CSS family name.
 * @param[in] weight Font weight (100 to 900, e.g. 400 for regular, 700 for
 * bold).
 * @param[in] is_italic Non-zero if italic font variant is requested.
 * @param[out] out_font Pointer to receive the loaded font handle.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if font not available,
 *         or an appropriate error code.
 */
C_MULTIPLATFORM_EXPORT ui_error_t ui_font_provider_load_system_font(
    struct ui_font_manager *manager, const char *family_name, int weight,
    int is_italic, struct ui_font **out_font);

/**
 * @brief Dummy function for Linux font provider fallback.
 * @return UI_ERROR_NONE.
 */
C_MULTIPLATFORM_EXPORT ui_error_t ui_font_provider_linux_dummy(void);

/**
 * @brief Dummy function for Win32 font provider fallback.
 * @return UI_ERROR_NONE.
 */
C_MULTIPLATFORM_EXPORT ui_error_t ui_font_provider_win32_dummy(void);

/**
 * @brief Dummy function for macOS font provider fallback.
 * @return UI_ERROR_NONE.
 */
C_MULTIPLATFORM_EXPORT ui_error_t ui_font_provider_macos_dummy(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FONT_PROVIDER_H */
