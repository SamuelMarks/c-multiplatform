/**
 * @file md3_radio_button.h
 * @brief Material 3 Radio Button component wrapping ui_radio_group_base.
 */

#ifndef MATERIAL3_MD3_RADIO_BUTTON_H
#define MATERIAL3_MD3_RADIO_BUTTON_H

/* clang-format off */
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_radio_group_base.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @struct md3_radio_group
 * @brief Material 3 Radio Group wrapper managing selection with CVA support.
 */
struct md3_radio_group {
  struct ui_radio_group_base *base;
  struct ui_control_value_accessor cva;
  int selected_index;
  int is_disabled;
};

/**
 * @brief Creates a Material 3 radio group wrapping ui_radio_group_base.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_group Pointer to receive newly created radio group.
 * @param out_cva Optional pointer to receive CVA interface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_radio_group_create(
    struct ui_engine *engine, struct md3_radio_group **out_group,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Material 3 radio group.
 *
 * @param group Radio group to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_radio_group_destroy(struct md3_radio_group *group);

/**
 * @brief Sets selected index of the radio group.
 *
 * @param group The radio group.
 * @param index Zero-based selected index (-1 for none).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_radio_group_set_selected_index(struct md3_radio_group *group, int index);

/**
 * @brief Gets selected index of the radio group.
 *
 * @param group The radio group.
 * @param out_index Pointer to receive selected index (-1 if none).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_radio_group_get_selected_index(
    const struct md3_radio_group *group, int *out_index);

/**
 * @brief Sets disabled state of the radio group.
 *
 * @param group The radio group.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_radio_group_set_disabled(struct md3_radio_group *group, int disabled);

/**
 * @brief Retrieves underlying ui_radio_group_base handle.
 *
 * @param group The radio group.
 * @param out_base Pointer to receive ui_radio_group_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_radio_group_get_base(
    struct md3_radio_group *group, struct ui_radio_group_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_RADIO_BUTTON_H */
