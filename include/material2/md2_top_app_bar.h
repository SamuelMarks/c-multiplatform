/**
 * @file md2_top_app_bar.h
 * @brief Material Design 2 Top App Bar component wrapping ui_top_app_bar_base.
 */

#ifndef MATERIAL2_MD2_TOP_APP_BAR_H
#define MATERIAL2_MD2_TOP_APP_BAR_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_top_app_bar_base.h"
#include <stddef.h>
/* clang-format on */

struct ui_engine;
struct md2_top_app_bar;

/**
 * @brief Material 2 Top App Bar visual variants.
 */
enum md2_top_app_bar_variant {
  MD2_TOP_APP_BAR_REGULAR,  /**< Regular 56dp Top App Bar. */
  MD2_TOP_APP_BAR_PROMINENT /**< Prominent 128dp Top App Bar. */
};

/**
 * @brief Creates a Material 2 Top App Bar component.
 *
 * @param engine Pointer to ui_engine.
 * @param variant Regular or Prominent.
 * @param title The title text.
 * @param out_bar Pointer to receive newly created Top App Bar.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_top_app_bar_create(
    struct ui_engine *engine, enum md2_top_app_bar_variant variant,
    const char *title, struct md2_top_app_bar **out_bar);

/**
 * @brief Destroys a Material 2 Top App Bar component.
 *
 * @param bar Top App Bar to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_top_app_bar_destroy(struct md2_top_app_bar *bar);

/**
 * @brief Sets the title of the Top App Bar.
 *
 * @param bar The Top App Bar.
 * @param title The title text to set.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_top_app_bar_set_title(struct md2_top_app_bar *bar, const char *title);

/**
 * @brief Retrieves the underlying ui_top_app_bar_base handle.
 *
 * @param bar The Top App Bar.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_top_app_bar_get_base(
    struct md2_top_app_bar *bar, struct ui_top_app_bar_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL2_MD2_TOP_APP_BAR_H */
