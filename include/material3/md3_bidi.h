/**
 * @file md3_bidi.h
 * @brief Material 3 Bidirectional (BiDi) layout, visual mirroring, font
 * cascades, HarfBuzz text shaping hooks, multi-calendar support, and localized
 * formatting.
 */

#ifndef MATERIAL3_MD3_BIDI_H
#define MATERIAL3_MD3_BIDI_H

/* clang-format off */
#include "ui_bidi_manager.h"
#include "ui_error.h"
#include "ui_font_manager.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum md3_calendar_type
 * @brief Multi-calendar engine system types.
 */
enum md3_calendar_type {
  MD3_CALENDAR_GREGORIAN = 0, /**< Standard Western Gregorian calendar */
  MD3_CALENDAR_ISLAMIC_HIJRI, /**< Islamic lunar Hijri calendar */
  MD3_CALENDAR_BUDDHIST,      /**< Solar Buddhist calendar (BE = CE + 543) */
  MD3_CALENDAR_HEBREW         /**< Hebrew lunisolar calendar */
};

/**
 * @struct md3_calendar_date
 * @brief Representation of a date in a specified calendar system.
 */
struct md3_calendar_date {
  int year;                    /**< Calendar year */
  int month;                   /**< 1-based month */
  int day;                     /**< 1-based day */
  enum md3_calendar_type type; /**< Calendar system type */
};

/**
 * @enum md3_plural_form
 * @brief CLDR category pluralization forms.
 */
enum md3_plural_form {
  MD3_PLURAL_ZERO = 0, /**< Exactly zero */
  MD3_PLURAL_ONE,      /**< Singular one */
  MD3_PLURAL_TWO,      /**< Dual two (Arabic/Hebrew) */
  MD3_PLURAL_FEW,      /**< Paucal few */
  MD3_PLURAL_MANY,     /**< Many (e.g. Russian, Arabic) */
  MD3_PLURAL_OTHER     /**< General plural form */
};

/**
 * @brief Sets the global BiDi direction and synchronizes with ui_bidi_manager.
 *
 * @param direction Global direction (UI_BIDI_DIR_LTR or UI_BIDI_DIR_RTL).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_bidi_set_direction(enum ui_bidi_direction direction);

/**
 * @brief Gets the current global BiDi direction.
 *
 * @param out_dir Pointer to receive the direction.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_bidi_get_direction(enum ui_bidi_direction *out_dir);

/**
 * @brief Resolves logical CSS margins (margin-inline-start / margin-inline-end)
 * to physical left/right margins based on direction.
 *
 * @param start Start margin in dp.
 * @param end End margin in dp.
 * @param dir Direction (LTR or RTL).
 * @param out_left Pointer to receive physical left margin.
 * @param out_right Pointer to receive physical right margin.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_bidi_resolve_logical_margin(
    float start, float end, enum ui_bidi_direction dir, float *out_left,
    float *out_right);

/**
 * @brief Resolves logical CSS paddings (padding-inline-start /
 * padding-inline-end) to physical left/right paddings based on direction.
 *
 * @param start Start padding in dp.
 * @param end End padding in dp.
 * @param dir Direction (LTR or RTL).
 * @param out_left Pointer to receive physical left padding.
 * @param out_right Pointer to receive physical right padding.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_bidi_resolve_logical_padding(
    float start, float end, enum ui_bidi_direction dir, float *out_left,
    float *out_right);

/**
 * @brief Resolves logical insets (inset-inline-start / inset-inline-end)
 * to physical left/right offsets.
 *
 * @param start Start offset in dp.
 * @param end End offset in dp.
 * @param dir Direction (LTR or RTL).
 * @param out_left Pointer to receive physical left offset.
 * @param out_right Pointer to receive physical right offset.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_bidi_resolve_logical_inset(
    float start, float end, enum ui_bidi_direction dir, float *out_left,
    float *out_right);

/**
 * @brief Determines if an icon requires directional horizontal mirroring.
 * Directional icons (arrows, chevrons, back, forward, drawer toggles) are
 * mirrored, while media controls (play, pause, fast forward, volume, clock) are
 * suppressed.
 *
 * @param icon_name Name of the icon.
 * @param dir Current layout direction.
 * @param out_should_mirror Pointer to receive 1 if should mirror, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_bidi_should_mirror_icon(
    const char *icon_name, enum ui_bidi_direction dir, int *out_should_mirror);

/**
 * @brief Retrieves recommended fallback font family name for a given script.
 * Provides Noto Sans cascades for multi-script coverage.
 *
 * @param script ISO 15924 script code (e.g. "Arab", "Hebr", "Deva", "Thai").
 * @param out_font_family Pointer to receive the font family name string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_bidi_get_fallback_font_family(
    const char *script, const char **out_font_family);

/**
 * @brief Retrieves recommended line-height multiplier for non-Latin scripts to
 * prevent vertical glyph clipping.
 *
 * @param script ISO 15924 script code.
 * @param out_multiplier Pointer to receive line height multiplier (e.g. 1.25
 * for Arabic).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_bidi_get_script_line_height_multiplier(const char *script,
                                           float *out_multiplier);

/**
 * @brief Converts an integer to Eastern Arabic (Hindi-Arabic) digits:
 * ٠١٢٣٤٥٦٧٨٩.
 *
 * @param number Input integer to format.
 * @param out_buf Buffer to store UTF-8 formatted Arabic numeral string.
 * @param buf_size Size of out_buf in bytes (minimum 32 recommended).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_i18n_format_eastern_arabic_numerals(int number, char *out_buf,
                                        size_t buf_size);

/**
 * @brief Formats a floating-point number with locale-specific decimal
 * separators and grouping symbols.
 *
 * @param number Value to format.
 * @param decimals Number of decimal digits.
 * @param locale BCP 47 locale string (e.g. "de-DE", "fr-FR", "en-US", "ar-EG").
 * @param out_buf Output string buffer.
 * @param buf_size Size of out_buf.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_i18n_format_localized_decimal(
    double number, int decimals, const char *locale, char *out_buf,
    size_t buf_size);

/**
 * @brief Converts a Gregorian calendar date to Islamic (Hijri), Buddhist, or
 * Hebrew calendar.
 *
 * @param g_year Gregorian year (e.g. 2026).
 * @param g_month Gregorian month (1-12).
 * @param g_day Gregorian day (1-31).
 * @param target_type Target calendar system type.
 * @param out_date Pointer to receive the converted date.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_calendar_convert_from_gregorian(
    int g_year, int g_month, int g_day, enum md3_calendar_type target_type,
    struct md3_calendar_date *out_date);

/**
 * @brief Determines the CLDR plural form for a given count and locale.
 *
 * @param count Count value.
 * @param locale BCP 47 locale string (e.g. "en", "ar", "ru", "pl").
 * @param out_form Pointer to receive the evaluated plural form.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_i18n_get_plural_form(
    int count, const char *locale, enum md3_plural_form *out_form);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_BIDI_H */
