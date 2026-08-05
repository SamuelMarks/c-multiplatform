/* clang-format off */
#ifndef UI_I18N_H
#define UI_I18N_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_error.h"
#include "ui_signal.h"
#include "ui_types.h"

/* clang-format on */

struct ui_i18n;

/**
 * @brief Creates a new I18n / L10n format engine.
 *
 * @param out_i18n Pointer to store the created i18n engine.
 * @return ui_error_t
 */
ui_error_t ui_i18n_create(struct ui_i18n **out_i18n);

/**
 * @brief Destroys the i18n engine.
 *
 * @param i18n The i18n engine.
 * @return ui_error_t
 */
ui_error_t ui_i18n_destroy(struct ui_i18n *i18n);

/**
 * @brief Sets the current locale (e.g., "en-US", "ar-SA", "he-IL").
 * Updates internal bidi direction automatically for known RTL locales.
 *
 * @param i18n The i18n engine.
 * @param locale The locale string.
 * @return ui_error_t
 */
ui_error_t ui_i18n_set_locale(struct ui_i18n *i18n, const char *locale);

/**
 * @brief Gets the current locale.
 *
 * @param i18n The i18n engine.
 * @param out_locale Pointer to store the locale string pointer.
 * @return ui_error_t
 */
ui_error_t ui_i18n_get_locale(struct ui_i18n *i18n, const char **out_locale);

/**
 * @brief Binds the locale state to a signal for global reactive updates.
 * Payload should be treated as a pointer to a string.
 *
 * @param i18n The i18n engine.
 * @param locale_signal The signal to bind to.
 * @return ui_error_t
 */
ui_error_t ui_i18n_bind_locale_signal(struct ui_i18n *i18n,
                                      struct ui_signal *locale_signal);

/**
 * @brief Formats a number according to the current locale.
 *
 * @param i18n The i18n engine.
 * @param value The numeric value.
 * @param decimals Number of decimal places.
 * @param out_str Buffer to write formatted string to.
 * @param out_len Size of the buffer.
 * @return ui_error_t
 */
ui_error_t ui_i18n_format_number(struct ui_i18n *i18n, double value,
                                 int decimals, char *out_str, size_t out_len);

/**
 * @brief Formats currency according to the current locale.
 *
 * @param i18n The i18n engine.
 * @param amount The currency amount.
 * @param currency_code The ISO 4217 currency code (e.g. "USD").
 * @param out_str Buffer to write formatted string to.
 * @param out_len Size of the buffer.
 * @return ui_error_t
 */
ui_error_t ui_i18n_format_currency(struct ui_i18n *i18n, double amount,
                                   const char *currency_code, char *out_str,
                                   size_t out_len);

/**
 * @brief Formats a date/time (Unix timestamp in ms) according to the current
 * locale.
 *
 * @param i18n The i18n engine.
 * @param timestamp_ms Timestamp in milliseconds since epoch.
 * @param format_str Format string compatible with strftime (or internal
 * parser).
 * @param out_str Buffer to write formatted string to.
 * @param out_len Size of the buffer.
 * @return ui_error_t
 */
ui_error_t ui_i18n_format_date(struct ui_i18n *i18n, double timestamp_ms,
                               const char *format_str, char *out_str,
                               size_t out_len);

/**
 * @brief Selects the correct pluralized string based on count.
 *
 * @param i18n The i18n engine.
 * @param count The item count.
 * @param zero String for 0 items.
 * @param one String for 1 item.
 * @param other String for other counts.
 * @param out_str Buffer to write the chosen string to.
 * @param out_len Size of the buffer.
 * @return ui_error_t
 */
ui_error_t ui_i18n_pluralize(struct ui_i18n *i18n, int count, const char *zero,
                             const char *one, const char *other, char *out_str,
                             size_t out_len);

/**
 * @brief Interpolates values into a template string (e.g. "Hello {{name}}!").
 *
 * @param i18n The i18n engine.
 * @param template_str The template string.
 * @param keys Array of key strings.
 * @param values Array of value strings corresponding to the keys.
 * @param count Number of key/value pairs.
 * @param out_str Buffer to write interpolated string to.
 * @param out_len Size of the buffer.
 * @return ui_error_t
 */
ui_error_t ui_i18n_interpolate(struct ui_i18n *i18n, const char *template_str,
                               const char **keys, const char **values,
                               size_t count, char *out_str, size_t out_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_I18N_H */
