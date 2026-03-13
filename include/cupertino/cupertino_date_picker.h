#ifndef CUPERTINO_DATE_PICKER_H
#define CUPERTINO_DATE_PICKER_H

/**
 * @file cupertino_date_picker.h
 * @brief Apple iOS style Inline Date Picker Button.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_text.h"
#include "cupertino/cupertino_color.h"
#include "cupertino/cupertino_typography.h"
/* clang-format on */

/** @brief Cupertino Date Picker Widget (Inline Button Style) */
typedef struct CupertinoDatePicker {
  CMPWidget widget;            /**< Base widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing labels. */

  CMPRect bounds; /**< Layout bounds. */

  cmp_i32 year;  /**< Selected year. */
  cmp_i32 month; /**< Selected month (1-12). */
  cmp_i32 day;   /**< Selected day (1-31). */

  cmp_i32 hour;   /**< Selected hour (0-23). */
  cmp_i32 minute; /**< Selected minute (0-59). */

  CMPBool is_time_only; /**< If true, only shows time. */
  CMPBool is_dark_mode; /**< Dark mode styling. */
  CMPBool is_pressed;   /**< True when pressed. */
} CupertinoDatePicker;

/**
 * @brief Initializes a Date Picker.
 * @param picker Pointer to the picker instance.
 * @param text_backend Pointer to the text backend interface.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_date_picker_init(
    CupertinoDatePicker *picker, const CMPTextBackend *text_backend);

/**
 * @brief Sets the date/time of the picker.
 * @param picker Pointer to the picker.
 * @param year Year (e.g. 2026).
 * @param month Month (1-12).
 * @param day Day (1-31).
 * @param hour Hour (0-23).
 * @param minute Minute (0-59).
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_date_picker_set_datetime(
    CupertinoDatePicker *picker, cmp_i32 year, cmp_i32 month, cmp_i32 day,
    cmp_i32 hour, cmp_i32 minute);

/**
 * @brief Renders the date picker inline button.
 * @param picker Pointer to the picker.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_date_picker_paint(
    const CupertinoDatePicker *picker, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_DATE_PICKER_H */
