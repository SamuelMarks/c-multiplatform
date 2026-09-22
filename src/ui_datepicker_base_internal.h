/**
 * @file ui_datepicker_base_internal.h
 * @brief Internal declarations for datepicker base.
 */
#ifndef UI_DATEPICKER_BASE_INTERNAL_H
#define UI_DATEPICKER_BASE_INTERNAL_H

/* clang-format off */
#include "ui_datepicker_base.h"
#include "ui_input_base.h"
#include "ui_popover_base.h"
#include "ui_calendar_base.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct ui_datepicker_base
 * @brief Internal representation of a datepicker base widget.
 */
struct ui_datepicker_base {
  struct ui_input_base *input;       /**< Input field */
  struct ui_popover_base *popover;   /**< Popover */
  struct ui_calendar_base *calendar; /**< Calendar */
  ui_error_t (*cva_on_change)(union ui_signal_payload,
                              void *);  /**< CVA change callback */
  void *cva_on_change_user_data;        /**< CVA change user data */
  ui_error_t (*cva_on_touched)(void *); /**< CVA touched callback */
  void *cva_on_touched_user_data;       /**< CVA touched user data */
  int is_disabled;                      /**< Disabled flag */
  int is_syncing;                       /**< Syncing flag */
};

/**
 * @brief Internal calendar select callback for datepicker.
 * @param calendar Pointer to the calendar.
 * @param date Pointer to the selected date.
 * @param user_data Pointer to the datepicker instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_datepicker_on_calendar_select(struct ui_calendar_base *calendar,
                                            const struct ui_date *date,
                                            void *user_data);

/**
 * @brief Internal input change callback for datepicker.
 * @param input Pointer to the input widget.
 * @param text The input text.
 * @param user_data Pointer to the datepicker instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_datepicker_on_input_change(struct ui_input_base *input,
                                         const char *text, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* UI_DATEPICKER_BASE_INTERNAL_H */
