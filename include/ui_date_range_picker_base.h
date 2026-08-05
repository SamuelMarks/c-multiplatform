#ifndef UI_DATE_RANGE_PICKER_BASE_H
#define UI_DATE_RANGE_PICKER_BASE_H

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
#include "ui_calendar_base.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_date_range_picker_base;

/**
 * @brief State of the date range selection process.
 */
enum ui_date_range_picker_state {
  UI_DATE_RANGE_PICKER_STATE_IDLE = 0,
  UI_DATE_RANGE_PICKER_STATE_SELECTING_END_DATE = 1
};

/**
 * @brief Represents a date range.
 */
struct ui_date_range {
  struct ui_date start_date;
  struct ui_date end_date;
};

/**
 * @brief Predicate callback to determine if a date is disabled.
 *
 * @param date The date to check.
 * @param user_data Opaque user data.
 * @return 1 (UI_TRUE) if disabled, 0 (UI_FALSE) otherwise.
 */
typedef ui_bool_t (*ui_date_predicate_cb)(const struct ui_date *date,
                                          void *user_data);

/**
 * @brief Callback invoked when the range selection is completed.
 *
 * @param picker The picker base.
 * @param range The selected range.
 * @param user_data Opaque user data.
 */
typedef ui_error_t (*ui_date_range_on_change_cb)(
    struct ui_date_range_picker_base *picker, const struct ui_date_range *range,
    void *user_data);

/**
 * @brief Creates a new date range picker base.
 *
 * @param out_picker Pointer to receive the allocated base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_date_range_picker_base_create(struct ui_date_range_picker_base **out_picker);

/**
 * @brief Destroys a date range picker base.
 *
 * @param picker The picker base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_date_range_picker_base_destroy(struct ui_date_range_picker_base *picker);

/**
 * @brief Sets a predicate to disable specific dates.
 *
 * @param picker The picker base.
 * @param predicate The callback function.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_date_range_picker_base_set_disable_predicate(
    struct ui_date_range_picker_base *picker, ui_date_predicate_cb predicate,
    void *user_data);

/**
 * @brief Sets the change callback.
 *
 * @param picker The picker base.
 * @param on_change The callback function.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_date_range_picker_base_set_on_change(
    struct ui_date_range_picker_base *picker,
    ui_date_range_on_change_cb on_change, void *user_data);

/**
 * @brief Handles a user click/selection on a specific date.
 * Steps the internal state machine.
 *
 * @param picker The picker base.
 * @param date The selected date.
 * @return UI_ERROR_NONE on success, or an error if invalid date.
 */
ui_error_t
ui_date_range_picker_base_select_date(struct ui_date_range_picker_base *picker,
                                      const struct ui_date *date);

/**
 * @brief Sets the hovered date during end-date selection to calculate the
 * active range span.
 *
 * @param picker The picker base.
 * @param date The hovered date.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_date_range_picker_base_set_hover_date(
    struct ui_date_range_picker_base *picker, const struct ui_date *date);

/**
 * @brief Retrieves the current state of the selection process.
 *
 * @param picker The picker base.
 * @param out_state Pointer to receive the state.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_date_range_picker_base_get_state(
    const struct ui_date_range_picker_base *picker,
    enum ui_date_range_picker_state *out_state);

/**
 * @brief Retrieves the currently selected range (or partial range).
 *
 * @param picker The picker base.
 * @param out_range Pointer to receive the range. If selecting end date,
 * end_date will match hover date or start date.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_date_range_picker_base_get_range(
    const struct ui_date_range_picker_base *picker,
    struct ui_date_range *out_range);

/**
 * @brief Clears the current selection and resets the state machine.
 *
 * @param picker The picker base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_date_range_picker_base_clear(struct ui_date_range_picker_base *picker);

/**
 * @brief Compares two dates.
 *
 * @param a First date.
 * @param b Second date.
 * @param out_result <0 if a < b, 0 if a == b, >0 if a > b.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_date_compare(const struct ui_date *a, const struct ui_date *b,
                           int *out_result);

/**
 * @brief Validates if a date is logically valid.
 *
 * @param date The date to validate.
 * @param out_is_valid Pointer to receive validity.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_date_is_valid(const struct ui_date *date,
                            ui_bool_t *out_is_valid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DATE_RANGE_PICKER_BASE_H */
