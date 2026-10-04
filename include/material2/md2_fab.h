/**
 * @file md2_fab.h
 * @brief Material Design 2 Floating Action Button component wrapping
 * ui_fab_base.
 */

#ifndef MATERIAL2_MD2_FAB_H
#define MATERIAL2_MD2_FAB_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_fab_base.h"
#include <stddef.h>
/* clang-format on */

struct ui_engine;
struct md2_fab;

/**
 * @brief Material 2 FAB size variants.
 */
enum md2_fab_size {
  MD2_FAB_SIZE_STANDARD, /**< 56dp FAB */
  MD2_FAB_SIZE_MINI,     /**< 40dp FAB */
  MD2_FAB_SIZE_EXTENDED  /**< Extended FAB with text label */
};

/**
 * @brief Creates a Material 2 FAB component.
 *
 * @param engine Pointer to ui_engine.
 * @param size Standard, Mini, or Extended.
 * @param icon The icon string.
 * @param label The text label (only used if Extended size).
 * @param out_fab Pointer to receive newly created FAB.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_fab_create(struct ui_engine *engine, enum md2_fab_size size,
               const char *icon, const char *label, struct md2_fab **out_fab);

/**
 * @brief Destroys a Material 2 FAB component.
 *
 * @param fab FAB to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_fab_destroy(struct md2_fab *fab);

/**
 * @brief Sets the click handler for the FAB.
 *
 * @param fab The FAB.
 * @param on_click Callback function.
 * @param user_data User data pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_fab_set_on_click(
    struct md2_fab *fab, ui_button_on_click_t on_click, void *user_data);

/**
 * @brief Retrieves the underlying ui_fab_base handle.
 *
 * @param fab The FAB.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_fab_get_base(struct md2_fab *fab, struct ui_fab_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL2_MD2_FAB_H */
