/**
 * @file md3_date_time_pickers.h
 * @brief Material 3 Date and Time Pickers.
 */

#ifndef MATERIAL3_MD3_DATE_TIME_PICKERS_H
#define MATERIAL3_MD3_DATE_TIME_PICKERS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_datepicker_base.h"
#include "ui_date_range_picker_base.h"
#include "ui_timepicker_base.h"
#include <stddef.h>
/* clang-format on */

struct ui_engine;

/**
 * @struct md3_date_picker
 * @brief Material 3 Calendar Date Picker modal dialog.
 */
struct md3_date_picker;

/**
 * @struct md3_date_range_picker
 * @brief Material 3 Date Range Picker.
 */
struct md3_date_range_picker;

/**
 * @struct md3_time_picker
 * @brief Material 3 Time Picker (Clock dial or Input mode).
 */
struct md3_time_picker;

/**
 * @brief Material 3 Time Picker modes.
 */
enum md3_time_picker_mode {
  MD3_TIME_PICKER_MODE_DIAL, /**< Interactive circular clock dial. */
  MD3_TIME_PICKER_MODE_INPUT /**< Input fields with AM/PM toggle pill. */
};

/* -------------------------------------------------------------------------
 * MD3 Date Picker
 * ------------------------------------------------------------------------- */

/**
 * @brief Creates a Material 3 Date Picker.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_picker Pointer to receive newly created Date Picker.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_date_picker_create(
    struct ui_engine *engine, struct md3_date_picker **out_picker);

/**
 * @brief Destroys a Material 3 Date Picker.
 *
 * @param picker Date picker to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_date_picker_destroy(struct md3_date_picker *picker);

/* -------------------------------------------------------------------------
 * MD3 Date Range Picker
 * ------------------------------------------------------------------------- */

/**
 * @brief Creates a Material 3 Date Range Picker.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_picker Pointer to receive newly created Date Range Picker.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_date_range_picker_create(
    struct ui_engine *engine, struct md3_date_range_picker **out_picker);

/**
 * @brief Destroys a Material 3 Date Range Picker.
 *
 * @param picker Date range picker to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_date_range_picker_destroy(struct md3_date_range_picker *picker);

/**
 * @brief Selects a date in the date range picker.
 * @param picker The picker.
 * @param date The date to select.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_date_range_picker_select_date(
    struct md3_date_range_picker *picker, const struct ui_date *date);

/**
 * @brief Sets the hover date in the date range picker.
 * @param picker The picker.
 * @param date The date to hover.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_date_range_picker_set_hover_date(
    struct md3_date_range_picker *picker, const struct ui_date *date);

/**
 * @brief Gets the selected range from the date range picker.
 * @param picker The picker.
 * @param out_range Pointer to store the range.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_date_range_picker_get_range(const struct md3_date_range_picker *picker,
                                struct ui_date_range *out_range);

/**
 * @brief Clears the selected range from the date range picker.
 * @param picker The picker.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_date_range_picker_clear(struct md3_date_range_picker *picker);

/* -------------------------------------------------------------------------
 * MD3 Time Picker

 * ------------------------------------------------------------------------- */

/**
 * @brief Creates a Material 3 Time Picker.
 *
 * @param engine Pointer to ui_engine instance.
 * @param mode Dial or input mode.
 * @param format 12-hour or 24-hour format.
 * @param out_picker Pointer to receive newly created Time Picker.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_time_picker_create(
    struct ui_engine *engine, enum md3_time_picker_mode mode,
    enum ui_timepicker_format format, struct md3_time_picker **out_picker);

/**
 * @brief Destroys a Material 3 Time Picker.
 *
 * @param picker Time picker to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_time_picker_destroy(struct md3_time_picker *picker);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_DATE_TIME_PICKERS_H */
