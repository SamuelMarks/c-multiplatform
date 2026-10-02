/**
 * @file cupertino_bidi.h
 * @brief Cupertino & Apple Human Interface Guidelines (HIG) Bidirectional
 * (BiDi) layout mirroring, localized numerals, calendar conversions, and
 * text shaping utilities.
 */

#ifndef CUPERTINO_CUPERTINO_BIDI_H
#define CUPERTINO_CUPERTINO_BIDI_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_bidi_manager.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @enum cupertino_calendar_type
 * @brief Apple HIG calendar system identifiers.
 */
enum cupertino_calendar_type {
  CUPERTINO_CALENDAR_GREGORIAN = 0, /**< Standard Western Gregorian calendar */
  CUPERTINO_CALENDAR_BUDDHIST, /**< Buddhist solar calendar (BE = CE + 543) */
  CUPERTINO_CALENDAR_JAPANESE, /**< Japanese Imperial era calendar */
  CUPERTINO_CALENDAR_ISLAMIC_HIJRI, /**< Islamic lunar Hijri calendar */
  CUPERTINO_CALENDAR_HEBREW         /**< Hebrew lunisolar calendar */
};

/**
 * @struct cupertino_calendar_date
 * @brief Representation of a date in a specified calendar system.
 */
struct cupertino_calendar_date {
  int year;                          /**< Year number in specified calendar */
  int month;                         /**< 1-based month (1-12 or 1-13) */
  int day;                           /**< 1-based day */
  enum cupertino_calendar_type type; /**< Calendar system type */
  char era_name[32];                 /**< Optional era name (e.g., "Reiwa") */
};

/**
 * @enum cupertino_numeral_system
 * @brief Localized numeral representation systems.
 */
enum cupertino_numeral_system {
  CUPERTINO_NUMERAL_WESTERN = 0,    /**< Western Arabic digits 0-9 */
  CUPERTINO_NUMERAL_EASTERN_ARABIC, /**< Eastern Arabic-Indic digits ٠-٩ */
  CUPERTINO_NUMERAL_PERSIAN,        /**< Persian digits ۰-۹ */
  CUPERTINO_NUMERAL_DEVANAGARI      /**< Devanagari digits ०-९ */
};

/**
 * @enum cupertino_plural_category
 * @brief CLDR plural categories for Apple .stringsdict translation.
 */
enum cupertino_plural_category {
  CUPERTINO_PLURAL_ZERO = 0, /**< Exactly zero */
  CUPERTINO_PLURAL_ONE,      /**< Singular one */
  CUPERTINO_PLURAL_TWO,      /**< Dual two */
  CUPERTINO_PLURAL_FEW,      /**< Paucal few */
  CUPERTINO_PLURAL_MANY,     /**< Many */
  CUPERTINO_PLURAL_OTHER     /**< General plural form */
};

/**
 * @brief Sets the global layout direction for Cupertino components.
 *
 * @param direction Global direction (UI_BIDI_DIR_LTR or UI_BIDI_DIR_RTL).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_bidi_set_direction(enum ui_bidi_direction direction);

/**
 * @brief Gets the current global layout direction.
 *
 * @param out_dir Pointer to receive the direction.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_bidi_get_direction(enum ui_bidi_direction *out_dir);

/**
 * @brief Resolves logical CSS margins (margin-inline-start / margin-inline-end)
 * to physical left/right margins based on direction.
 *
 * @param start Start margin in points.
 * @param end End margin in points.
 * @param dir Direction (LTR or RTL).
 * @param out_left Pointer to receive physical left margin.
 * @param out_right Pointer to receive physical right margin.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_bidi_resolve_logical_margin(
    float start, float end, enum ui_bidi_direction dir, float *out_left,
    float *out_right);

/**
 * @brief Resolves logical CSS paddings (padding-inline-start /
 * padding-inline-end) to physical left/right paddings based on direction.
 *
 * @param start Start padding in points.
 * @param end End padding in points.
 * @param dir Direction (LTR or RTL).
 * @param out_left Pointer to receive physical left padding.
 * @param out_right Pointer to receive physical right padding.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_bidi_resolve_logical_padding(
    float start, float end, enum ui_bidi_direction dir, float *out_left,
    float *out_right);

/**
 * @brief Resolves logical insets (inset-inline-start / inset-inline-end)
 * to physical left/right offsets.
 *
 * @param start Start offset in points.
 * @param end End offset in points.
 * @param dir Direction (LTR or RTL).
 * @param out_left Pointer to receive physical left offset.
 * @param out_right Pointer to receive physical right offset.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_bidi_resolve_logical_inset(
    float start, float end, enum ui_bidi_direction dir, float *out_left,
    float *out_right);

/**
 * @brief Determines if an SF Symbol or accessory glyph requires horizontal
 * mirroring in RTL mode according to Apple HIG.
 *
 * @param symbol_name SF Symbol or icon identifier.
 * @param dir Current layout direction.
 * @param out_should_mirror Pointer to receive 1 if should mirror, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_bidi_should_mirror_symbol(
    const char *symbol_name, enum ui_bidi_direction dir,
    int *out_should_mirror);

/**
 * @brief Converts an integer to a localized numeral string in the target
 * system.
 *
 * @param number Input integer.
 * @param system Target numeral system.
 * @param out_buf Buffer to store UTF-8 string.
 * @param buf_size Size of out_buf in bytes.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT, or
 * UI_ERROR_OUT_OF_BOUNDS.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_i18n_format_numeral(int number, enum cupertino_numeral_system system,
                              char *out_buf, size_t buf_size);

/**
 * @brief Converts a Gregorian calendar date to Islamic, Buddhist, Japanese
 * Imperial, or Hebrew calendar.
 *
 * @param g_year Gregorian year (e.g. 2026).
 * @param g_month Gregorian month (1-12).
 * @param g_day Gregorian day (1-31).
 * @param target_type Target calendar system type.
 * @param out_date Pointer to receive the converted date.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_calendar_convert_from_gregorian(
    int g_year, int g_month, int g_day,
    enum cupertino_calendar_type target_type,
    struct cupertino_calendar_date *out_date);

/**
 * @brief Resolves first day of the week for a given locale (0 = Sunday, 1 =
 * Monday, 6 = Saturday).
 *
 * @param locale BCP 47 locale string (e.g. "en_US", "fr_FR", "ar_SA").
 * @param out_first_day Pointer to receive 0, 1, or 6.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_i18n_get_first_day_of_week(const char *locale, int *out_first_day);

/**
 * @brief Evaluates the CLDR plural category for Apple .stringsdict translation.
 *
 * @param count Count value.
 * @param locale BCP 47 locale string.
 * @param out_category Pointer to receive plural category.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_i18n_get_plural_category(
    int count, const char *locale,
    enum cupertino_plural_category *out_category);

/**
 * @brief Checks if a character is prohibited at the start of a line under
 * Japanese Kinsoku Shori rules.
 *
 * @param utf8_char UTF-8 character string (null terminated).
 * @param out_prohibited Pointer to receive 1 if prohibited at start, 0
 * otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_i18n_kinsoku_prohibited_start(const char *utf8_char,
                                        int *out_prohibited);

/**
 * @brief Checks if a character is prohibited at the end of a line under
 * Japanese Kinsoku Shori rules.
 *
 * @param utf8_char UTF-8 character string (null terminated).
 * @param out_prohibited Pointer to receive 1 if prohibited at end, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_i18n_kinsoku_prohibited_end(
    const char *utf8_char, int *out_prohibited);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_BIDI_H */
