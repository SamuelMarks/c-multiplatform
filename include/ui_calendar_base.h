/**
 * @file ui_calendar_base.h
 * @brief Calendar base component definitions.
 */

#ifndef UI_CALENDAR_BASE_H
#define UI_CALENDAR_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

/** @brief Opaque handle to a calendar base component. */
struct ui_calendar_base;

/**
 * @brief Represents a simple Date.
 */
struct ui_date {
  int year;  /**< The year (e.g., 2026) */
  int month; /**< 1-12 */
  int day;   /**< 1-31 */
};

/**
 * @brief Day of week.
 */
enum ui_day_of_week {
  UI_SUNDAY = 0,    /**< Sunday */
  UI_MONDAY = 1,    /**< Monday */
  UI_TUESDAY = 2,   /**< Tuesday */
  UI_WEDNESDAY = 3, /**< Wednesday */
  UI_THURSDAY = 4,  /**< Thursday */
  UI_FRIDAY = 5,    /**< Friday */
  UI_SATURDAY = 6   /**< Saturday */
};

/**
 * @brief Creates a new calendar base component.
 *
 * @param out_calendar Pointer to receive the allocated calendar base.
 * @param out_cva Optional pointer to receive the control value accessor.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_calendar_base_create(struct ui_calendar_base **out_calendar,
                                   struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a calendar base component.
 *
 * @param calendar The calendar to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_calendar_base_destroy(struct ui_calendar_base *calendar);

/**
 * @brief Checks if a year is a leap year.
 * @param year The year to check.
 * @param out_is_leap Pointer to receive 1 if leap year, 0 otherwise.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_calendar_is_leap_year(int year, int *out_is_leap);

/**
 * @brief Gets the number of days in a month for a specific year.
 * @param year The year.
 * @param month The month (1-12).
 * @param out_days Pointer to receive the number of days (28-31) or 0 if invalid
 * month.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_calendar_days_in_month(int year, int month, int *out_days);

/**
 * @brief Gets the day of the week for a specific date using Sakamoto's
 * algorithm.
 * @param year The year.
 * @param month The month (1-12).
 * @param day The day (1-31).
 * @param out_dow Pointer to receive the day of the week, where 0 = Sunday.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_calendar_get_day_of_week(int year, int month, int day,
                                       enum ui_day_of_week *out_dow);

/**
 * @brief Sets the starting day of the week (e.g., UI_SUNDAY or UI_MONDAY).
 *
 * @param calendar The calendar.
 * @param start_day The start day.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_calendar_base_set_start_of_week(struct ui_calendar_base *calendar,
                                              enum ui_day_of_week start_day);

/**
 * @brief Sets the minimum allowed date. Pass NULL to remove the constraint.
 *
 * @param calendar The calendar.
 * @param min_date The minimum date or NULL.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_calendar_base_set_min_date(struct ui_calendar_base *calendar,
                                         const struct ui_date *min_date);

/**
 * @brief Sets the maximum allowed date. Pass NULL to remove the constraint.
 *
 * @param calendar The calendar.
 * @param max_date The maximum date or NULL.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_calendar_base_set_max_date(struct ui_calendar_base *calendar,
                                         const struct ui_date *max_date);

/**
 * @brief Sets the currently viewed month and year.
 *
 * @param calendar The calendar.
 * @param year The year.
 * @param month The month (1-12).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_calendar_base_set_view_month(struct ui_calendar_base *calendar,
                                           int year, int month);

/**
 * @brief Gets the currently viewed month and year.
 *
 * @param calendar The calendar.
 * @param out_year Pointer to receive the year.
 * @param out_month Pointer to receive the month.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_calendar_base_get_view_month(const struct ui_calendar_base *calendar,
                                int *out_year, int *out_month);

/**
 * @brief Selects a specific date. Validates against min/max and valid month
 * lengths.
 *
 * @param calendar The calendar.
 * @param date The date to select.
 * @return UI_ERROR_OUT_OF_BOUNDS if date is out of range, UI_ERROR_NONE on
 * success.
 */
ui_error_t ui_calendar_base_select_date(struct ui_calendar_base *calendar,
                                        const struct ui_date *date);

/**
 * @brief Gets the currently selected date.
 *
 * @param calendar The calendar.
 * @param out_date Pointer to receive the selected date.
 * @return UI_ERROR_NOT_FOUND if no date is selected, UI_ERROR_NONE on success.
 */
ui_error_t
ui_calendar_base_get_selected_date(const struct ui_calendar_base *calendar,
                                   struct ui_date *out_date);

/**
 * @brief Clears the current selection.
 *
 * @param calendar The calendar.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_calendar_base_clear_selection(struct ui_calendar_base *calendar);

/**
 * @brief Callback invoked when a calendar date is selected.
 */
typedef ui_error_t (*ui_calendar_on_select_t)(struct ui_calendar_base *calendar,
                                              const struct ui_date *date,
                                              void *user_data);

/**
 * @brief Sets the callback for date selection.
 *
 * @param calendar The calendar.
 * @param on_select The callback.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_calendar_base_set_on_select(struct ui_calendar_base *calendar,
                                          ui_calendar_on_select_t on_select,
                                          void *user_data);

/**
 * @brief Retrieves the calendar grid for the current view month.
 *
 * @param calendar The calendar.
 * @param out_grid Array of at least 42 struct ui_date elements to receive the
 * grid.
 * @param out_count Pointer to receive the number of generated cells (always
 * 42).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_calendar_base_get_month_grid(const struct ui_calendar_base *calendar,
                                struct ui_date *out_grid, int *out_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CALENDAR_BASE_H */
