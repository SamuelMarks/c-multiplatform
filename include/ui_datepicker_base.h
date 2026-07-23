#ifndef UI_DATEPICKER_BASE_H
#define UI_DATEPICKER_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_calendar_base.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

struct ui_datepicker_base;
struct ui_input_base;
struct ui_popover_base;

/**
 * @brief Creates a datepicker coordinator linking an input field and a popover
 * containing a calendar.
 *
 * @param out_datepicker Pointer to receive the allocated datepicker base.
 * @param input The text input field.
 * @param popover The popover to display the calendar.
 * @param calendar The calendar instance inside the popover.
 * @param out_cva Optional pointer to receive the control value accessor.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_datepicker_base_create(
    struct ui_datepicker_base **out_datepicker, struct ui_input_base *input,
    struct ui_popover_base *popover, struct ui_calendar_base *calendar,
    struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a datepicker coordinator. Does NOT destroy the linked
 * input/popover/calendar.
 *
 * @param datepicker The datepicker to destroy.
 */
void ui_datepicker_base_destroy(struct ui_datepicker_base *datepicker);

/**
 * @brief Parses a text string in YYYY-MM-DD format into a ui_date.
 *
 * @param text The input string.
 * @param out_date Pointer to receive the parsed date.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on parse failure.
 */
enum ui_error ui_datepicker_parse_date(const char *text,
                                       struct ui_date *out_date);

/**
 * @brief Formats a ui_date into a YYYY-MM-DD string.
 *
 * @param date The input date.
 * @param out_text The output buffer.
 * @param max_len Maximum length of the output buffer.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_datepicker_format_date(const struct ui_date *date,
                                        char *out_text, int max_len);

/**
 * @brief Synchronizes the input text and the calendar selection manually.
 *
 * @param datepicker The datepicker.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_datepicker_base_sync(struct ui_datepicker_base *datepicker);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DATEPICKER_BASE_H */
