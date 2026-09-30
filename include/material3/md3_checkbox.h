/**
 * @file md3_checkbox.h
 * @brief Material 3 Checkbox component wrapping ui_checkbox_base.
 */

#ifndef MATERIAL3_MD3_CHECKBOX_H
#define MATERIAL3_MD3_CHECKBOX_H

/* clang-format off */
#include "ui_checkbox_base.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @struct md3_checkbox
 * @brief Material 3 Checkbox skin wrapping ui_checkbox_base.
 */
struct md3_checkbox {
  struct ui_checkbox_base *base;
  struct ui_control_value_accessor cva;
  int is_indeterminate;
  int is_disabled;
};

/**
 * @brief Creates a Material 3 checkbox wrapping ui_checkbox_base and exports
 * CVA.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_checkbox Pointer to receive newly created checkbox.
 * @param out_cva Pointer to receive control value accessor interface (optional,
 * can be NULL).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_checkbox_create(
    struct ui_engine *engine, struct md3_checkbox **out_checkbox,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Material 3 checkbox and frees its underlying base.
 *
 * @param checkbox Checkbox to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_checkbox_destroy(struct md3_checkbox *checkbox);

/**
 * @brief Sets checked state of the checkbox.
 *
 * @param checkbox The checkbox.
 * @param checked 1 if checked, 0 if unchecked.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_checkbox_set_checked(struct md3_checkbox *checkbox, int checked);

/**
 * @brief Gets checked state of the checkbox.
 *
 * @param checkbox The checkbox.
 * @param out_checked Pointer to receive checked state (1 or 0).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_checkbox_get_checked(const struct md3_checkbox *checkbox, int *out_checked);

/**
 * @brief Sets indeterminate tri-state of the checkbox.
 *
 * @param checkbox The checkbox.
 * @param is_indeterminate 1 if indeterminate, 0 if determinate.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_checkbox_set_indeterminate(
    struct md3_checkbox *checkbox, int is_indeterminate);

/**
 * @brief Sets disabled state of the checkbox.
 *
 * @param checkbox The checkbox.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_checkbox_set_disabled(struct md3_checkbox *checkbox, int disabled);

/**
 * @brief Retrieves underlying ui_checkbox_base.
 *
 * @param checkbox The checkbox.
 * @param out_base Pointer to receive ui_checkbox_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_checkbox_get_base(
    struct md3_checkbox *checkbox, struct ui_checkbox_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_CHECKBOX_H */
