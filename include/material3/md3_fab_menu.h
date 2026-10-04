/**
 * @file md3_fab_menu.h
 * @brief Material 3 FAB Menu / Speed Dial component.
 */

#ifndef MATERIAL3_MD3_FAB_MENU_H
#define MATERIAL3_MD3_FAB_MENU_H

/* clang-format off */
#include "ui_error.h"
#include "ui_speed_dial_base.h"
#include "material3/md3_fab.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

struct md3_fab_menu;

/**
 * @brief Direction for fan-out action items.
 */
enum md3_fab_menu_direction {
  MD3_FAB_MENU_DIRECTION_UP,
  MD3_FAB_MENU_DIRECTION_DOWN,
  MD3_FAB_MENU_DIRECTION_LEFT,
  MD3_FAB_MENU_DIRECTION_RIGHT,
  MD3_FAB_MENU_DIRECTION_COUNT
};

/**
 * @brief Creates a Material 3 FAB Menu (Speed Dial) component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param direction Fan-out direction for action items.
 * @param out_menu Pointer to receive newly created FAB Menu.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_fab_menu_create(
    struct ui_engine *engine, enum md3_fab_menu_direction direction,
    struct md3_fab_menu **out_menu);

/**
 * @brief Destroys a Material 3 FAB Menu.
 *
 * @param menu The FAB Menu to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_fab_menu_destroy(struct md3_fab_menu *menu);

/**
 * @brief Sets the primary FAB for the menu.
 *
 * @param menu The FAB Menu.
 * @param fab The primary FAB.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_fab_menu_set_primary_fab(struct md3_fab_menu *menu, struct md3_fab *fab);

/**
 * @brief Adds an action item to the FAB Menu.
 *
 * @param menu The FAB Menu.
 * @param id Action identifier.
 * @param fab The action FAB (usually small).
 * @param label The text label chip content (optional).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_fab_menu_add_action(
    struct md3_fab_menu *menu, int id, struct md3_fab *fab, const char *label);

/**
 * @brief Retrieves the underlying ui_speed_dial_base handle.
 *
 * @param menu The FAB Menu.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_fab_menu_get_base(
    struct md3_fab_menu *menu, struct ui_speed_dial_base **out_base);

/**
 * @brief Toggles menu expansion state with spring transitions.
 *
 * @param menu The FAB menu.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_fab_menu_toggle(struct md3_fab_menu *menu);

/**
 * @brief Sets menu expansion state explicitly.
 *
 * @param menu The FAB menu.
 * @param expanded 1 to expand, 0 to collapse.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_fab_menu_set_expanded(struct md3_fab_menu *menu, int expanded);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_FAB_MENU_H */
