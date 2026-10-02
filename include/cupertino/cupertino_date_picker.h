/**
 * @file cupertino_date_picker.h
 * @brief Cupertino Date and Time Pickers conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_DATE_PICKER_H
#define CUPERTINO_CUPERTINO_DATE_PICKER_H

/* clang-format off */
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_DATE_PICKER_COMPACT_WIDTH 130.0f
#define CUPERTINO_DATE_PICKER_COMPACT_HEIGHT 36.0f
#define CUPERTINO_DATE_PICKER_WHEELS_WIDTH 320.0f
#define CUPERTINO_DATE_PICKER_WHEELS_HEIGHT 216.0f
#define CUPERTINO_DATE_PICKER_INLINE_WIDTH 320.0f
#define CUPERTINO_DATE_PICKER_INLINE_HEIGHT 380.0f

/**
 * @enum cupertino_date_picker_mode
 * @brief Pick mode (time, date, both, countdown).
 */
enum cupertino_date_picker_mode {
  CUPERTINO_DATE_PICKER_MODE_TIME = 0,       /**< Hour, Minute, AM/PM. */
  CUPERTINO_DATE_PICKER_MODE_DATE,           /**< Month, Day, Year. */
  CUPERTINO_DATE_PICKER_MODE_DATE_AND_TIME,  /**< Date and Time combined. */
  CUPERTINO_DATE_PICKER_MODE_COUNTDOWN_TIMER /**< Hours and Minutes countdown.
                                              */
};

/**
 * @enum cupertino_date_picker_style
 * @brief Visual presentation style.
 */
enum cupertino_date_picker_style {
  CUPERTINO_DATE_PICKER_STYLE_COMPACT =
      0, /**< Compact pill button triggering popover. */
  CUPERTINO_DATE_PICKER_STYLE_INLINE, /**< Full monthly calendar grid. */
  CUPERTINO_DATE_PICKER_STYLE_WHEELS  /**< 3D cylinder slot machine wheels. */
};

/**
 * @struct cupertino_date_picker_descriptor
 * @brief Configuration descriptor for creating a Cupertino date/time picker.
 */
struct cupertino_date_picker_descriptor {
  enum cupertino_date_picker_mode mode;   /**< Picker mode. */
  enum cupertino_date_picker_style style; /**< Visual style. */
  int year;                               /**< Initial year (e.g. 2026). */
  int month;                              /**< Initial month (1-12). */
  int day;                                /**< Initial day (1-31). */
  int hour;                               /**< Initial hour (0-23). */
  int minute;                             /**< Initial minute (0-59). */
  int is_24h;                             /**< 1 for 24h, 0 for 12h AM/PM. */
  int minute_interval; /**< Interval steps (1, 5, 10, 15, etc.). */
};

/**
 * @struct cupertino_date_picker
 * @brief Cupertino date/time picker instance.
 */
struct cupertino_date_picker {
  enum cupertino_date_picker_mode mode;
  enum cupertino_date_picker_style style;
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int is_24h;
  int minute_interval;
  char formatted_text[64];
  struct ui_control_value_accessor cva;
  int is_popover_open;
  float width;
  float height;
};

/**
 * @brief Creates a new Cupertino date/time picker.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_picker Pointer to receive newly created picker instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_date_picker_create(
    struct ui_engine *engine,
    const struct cupertino_date_picker_descriptor *desc,
    struct cupertino_date_picker **out_picker);

/**
 * @brief Destroys a Cupertino date/time picker.
 *
 * @param picker Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_date_picker_destroy(struct cupertino_date_picker *picker);

/**
 * @brief Sets current date and time values.
 *
 * @param picker Target picker.
 * @param year Year value (1900-2100).
 * @param month Month value (1-12).
 * @param day Day value (1-31).
 * @param hour Hour value (0-23).
 * @param minute Minute value (0-59).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_date_picker_set_date(struct cupertino_date_picker *picker, int year,
                               int month, int day, int hour, int minute);

/**
 * @brief Gets current date and time values.
 *
 * @param picker Target picker.
 * @param out_year Pointer to receive year.
 * @param out_month Pointer to receive month.
 * @param out_day Pointer to receive day.
 * @param out_hour Pointer to receive hour.
 * @param out_minute Pointer to receive minute.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_date_picker_get_date(
    const struct cupertino_date_picker *picker, int *out_year, int *out_month,
    int *out_day, int *out_hour, int *out_minute);

/**
 * @brief Sets picker mode.
 *
 * @param picker Target picker.
 * @param mode New picker mode.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_date_picker_set_mode(
    struct cupertino_date_picker *picker, enum cupertino_date_picker_mode mode);

/**
 * @brief Gets current picker mode.
 *
 * @param picker Target picker.
 * @param out_mode Pointer to receive mode enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_date_picker_get_mode(const struct cupertino_date_picker *picker,
                               enum cupertino_date_picker_mode *out_mode);

/**
 * @brief Sets visual style (Compact, Inline, Wheels).
 *
 * @param picker Target picker.
 * @param style New style enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_date_picker_set_style(struct cupertino_date_picker *picker,
                                enum cupertino_date_picker_style style);

/**
 * @brief Gets current visual style.
 *
 * @param picker Target picker.
 * @param out_style Pointer to receive style enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_date_picker_get_style(const struct cupertino_date_picker *picker,
                                enum cupertino_date_picker_style *out_style);

/**
 * @brief Gets localized formatted text string (e.g. "Oct 1, 2026").
 *
 * @param picker Target picker.
 * @param out_text Pointer to receive const string pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_date_picker_get_formatted_text(
    const struct cupertino_date_picker *picker, const char **out_text);

/**
 * @brief Toggles compact popover display state.
 *
 * @param picker Target picker.
 * @param is_open 1 to open popover, 0 to close.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_date_picker_set_popover_open(
    struct cupertino_date_picker *picker, int is_open);

/**
 * @brief Gets dimensions of the picker container.
 *
 * @param picker Target picker.
 * @param out_width Pointer to receive width.
 * @param out_height Pointer to receive height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_date_picker_get_dimensions(const struct cupertino_date_picker *picker,
                                     float *out_width, float *out_height);

/**
 * @brief Retrieves Control Value Accessor for form integration.
 *
 * @param picker Target picker.
 * @param out_cva Pointer to receive CVA handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_date_picker_get_cva(struct cupertino_date_picker *picker,
                              struct ui_control_value_accessor **out_cva);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_DATE_PICKER_H */
