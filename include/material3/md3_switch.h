/**
 * @file md3_switch.h
 * @brief Material 3 Switch component wrapping ui_slide_toggle_base.
 */

#ifndef MATERIAL3_MD3_SWITCH_H
#define MATERIAL3_MD3_SWITCH_H

/* clang-format off */
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_slide_toggle_base.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @struct md3_switch
 * @brief Material 3 Switch skin wrapping ui_slide_toggle_base with CVA export.
 */
struct md3_switch {
  struct ui_slide_toggle_base *base;
  struct ui_control_value_accessor cva;
  int show_icon;
};

/**
 * @brief Creates a Material 3 switch wrapping ui_slide_toggle_base.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_switch Pointer to receive newly created switch.
 * @param out_cva Optional pointer to receive CVA interface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_switch_create(struct ui_engine *engine, struct md3_switch **out_switch,
                  struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Material 3 switch and its underlying base.
 *
 * @param sw Switch to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_switch_destroy(struct md3_switch *sw);

/**
 * @brief Sets checked state of the switch.
 *
 * @param sw The switch.
 * @param checked 1 if selected, 0 if unselected.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_switch_set_checked(struct md3_switch *sw, int checked);

/**
 * @brief Gets checked state of the switch.
 *
 * @param sw The switch.
 * @param out_checked Pointer to receive checked state (1 or 0).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_switch_get_checked(const struct md3_switch *sw, int *out_checked);

/**
 * @brief Sets disabled state of the switch.
 *
 * @param sw The switch.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_switch_set_disabled(struct md3_switch *sw, int disabled);

/**
 * @brief Enables or disables thumb icon display (checkmark / cross).
 *
 * @param sw The switch.
 * @param show_icon 1 to show thumb icon, 0 to hide.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_switch_set_show_icon(struct md3_switch *sw, int show_icon);

/**
 * @brief Retrieves underlying ui_slide_toggle_base handle.
 *
 * @param sw The switch.
 * @param out_base Pointer to receive ui_slide_toggle_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_switch_get_base(
    struct md3_switch *sw, struct ui_slide_toggle_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_SWITCH_H */
