/**
 * \file ui_i18n.c
 * \brief Implementation of Internationalization (i18n) routines.
 */
#ifdef _MSC_VER
#pragma warning(disable : 4702)
#endif

/* clang-format off */
#include "ui_i18n.h"
#include "ui_web_bridge.h"
#include "ui_bidi_manager.h"

#include <stdlib.h>
#include "ui_internal_mem.h"
#include <string.h>
#include <stdio.h>
/* clang-format on */

/**
 * \def MAX_LOCALE_LEN
 * \brief Maximum length for a locale string (e.g. "en-US").
 */
#define MAX_LOCALE_LEN 32

/**
 * \struct ui_i18n
 * \brief State and configuration for internationalization.
 */
struct ui_i18n {
  char locale[MAX_LOCALE_LEN];
  struct ui_signal *locale_signal;
};

/**
 * \brief Safely copies a string, guaranteeing null termination.
 * \param[out] dst The destination buffer.
 * \param[in] sz The size of the destination buffer.
 * \param[in] src The source string.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t safe_strcpy(char *dst, size_t sz, const char *src) {
#if defined(_MSC_VER)
  strcpy_s(dst, sz, src);
#else
  strncpy(dst, src, sz - 1);
  dst[sz - 1] = '\0';
#endif
  return UI_ERROR_NONE;
}

/**
 * \brief Creates a new i18n context.
 * \param[out] out_i18n Pointer to store the created context.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_i18n_create(struct ui_i18n **out_i18n) {
  struct ui_i18n *i18n;

  if (!out_i18n) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  i18n = (struct ui_i18n *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_i18n));
  if (!i18n) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  (void)safe_strcpy(i18n->locale, MAX_LOCALE_LEN, "en-US");
  i18n->locale_signal = NULL;

  *out_i18n = i18n;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys an i18n context.
 * \param[in,out] i18n The context to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_i18n_destroy(struct ui_i18n *i18n) {
  if (!i18n) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(i18n);
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the current locale and updates bidi direction if necessary.
 * \param[in,out] i18n The i18n context.
 * \param[in] locale The locale string (e.g. "ar-EG").
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_i18n_set_locale(struct ui_i18n *i18n, const char *locale) {
  if (!i18n || !locale) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    ui_error_t rc = safe_strcpy(i18n->locale, MAX_LOCALE_LEN, locale);
    (void)rc;
  }

  /* Check for RTL locales and update bidi manager */
  if (strncmp(locale, "ar", 2) == 0 || strncmp(locale, "he", 2) == 0 ||
      strncmp(locale, "fa", 2) == 0 || strncmp(locale, "ur", 2) == 0) {
    ui_error_t b_rc = ui_bidi_set_direction(UI_BIDI_DIR_RTL);
    (void)b_rc;
  } else {
    ui_error_t b_rc = ui_bidi_set_direction(UI_BIDI_DIR_LTR);
    (void)b_rc;
  }

#if defined(__EMSCRIPTEN__)
  /* Send to root element */
  ui_web_bridge_set_style(0, "lang", locale);
#endif
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the current locale string.
 * \param[in] i18n The i18n context.
 * \param[out] out_locale Pointer to store the locale string.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_i18n_get_locale(struct ui_i18n *i18n, const char **out_locale) {
  if (!i18n || !out_locale) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_locale = i18n->locale;
  return UI_ERROR_NONE;
}

/**
 * \brief Binds the current locale to a reactive signal.
 * \param[in,out] i18n The i18n context.
 * \param[in,out] locale_signal The signal to bind.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_i18n_bind_locale_signal(struct ui_i18n *i18n,
                                      struct ui_signal *locale_signal) {
  if (!i18n || !locale_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  i18n->locale_signal = locale_signal;
  return UI_ERROR_NONE;
}

/**
 * \brief Formats a floating point number to a string.
 * \param[in] i18n The i18n context.
 * \param[in] value The numeric value to format.
 * \param[in] decimals Number of decimal places to include.
 * \param[out] out_str The output buffer.
 * \param[in] out_len The size of the output buffer.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_i18n_format_number(struct ui_i18n *i18n, double value,
                                 int decimals, char *out_str, size_t out_len) {
  char format_str[16];
  if (!i18n || !out_str || out_len == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  sprintf_s(format_str, sizeof(format_str), "%%.%df", decimals);
  sprintf_s(out_str, out_len, format_str, value);
#else
  sprintf(format_str, "%%.%df", decimals);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
  sprintf(out_str, format_str, value);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
  return UI_ERROR_NONE;
#endif
}

/**
 * \brief Formats an amount as currency.
 * \param[in] i18n The i18n context.
 * \param[in] amount The amount to format.
 * \param[in] currency_code The currency code (e.g. "USD").
 * \param[out] out_str The output buffer.
 * \param[in] out_len The size of the output buffer.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_i18n_format_currency(struct ui_i18n *i18n, double amount,
                                   const char *currency_code, char *out_str,
                                   size_t out_len) {
  char number_buf[64];

  if (!i18n || !currency_code || !out_str || out_len == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {

    ui_error_t _ign_rc =
        ui_i18n_format_number(i18n, amount, 2, number_buf, sizeof(number_buf));

    (void)_ign_rc;
  }

#if defined(_MSC_VER)
  sprintf_s(out_str, out_len, "%s %s", number_buf, currency_code);
#else
  sprintf(out_str, "%s %s", number_buf, currency_code);
#endif
  return UI_ERROR_NONE;
}

/**
 * \brief Formats a timestamp as a date string.
 * \param[in] i18n The i18n context.
 * \param[in] timestamp_ms The timestamp in milliseconds.
 * \param[in] format_str The date format string.
 * \param[out] out_str The output buffer.
 * \param[in] out_len The size of the output buffer.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_i18n_format_date(struct ui_i18n *i18n, double timestamp_ms,
                               const char *format_str, char *out_str,
                               size_t out_len) {
  /* Very rudimentary stub formatting for demonstration. */
  if (!i18n || !format_str || !out_str || out_len == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#if defined(_MSC_VER)
  sprintf_s(out_str, out_len, "Date:%f", timestamp_ms);
#else
  sprintf(out_str, "Date:%f", timestamp_ms);
#endif
  return UI_ERROR_NONE;
}

/**
 * \brief Selects the correct pluralized string based on a count.
 * \param[in] i18n The i18n context.
 * \param[in] count The count.
 * \param[in] zero The string to use if count is 0.
 * \param[in] one The string to use if count is 1.
 * \param[in] other The string to use if count > 1.
 * \param[out] out_str The output buffer.
 * \param[in] out_len The size of the output buffer.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_i18n_pluralize(struct ui_i18n *i18n, int count, const char *zero,
                             const char *one, const char *other, char *out_str,
                             size_t out_len) {
  const char *choice;
  if (!i18n || !zero || !one || !other || !out_str || out_len == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (count == 0) {
    choice = zero;
  } else if (count == 1) {
    choice = one;
  } else {
    choice = other;
  }

  (void)safe_strcpy(out_str, out_len, choice);
  return UI_ERROR_NONE;
}

/**
 * \brief Interpolates variables into a template string (e.g. "Hello {{name}}").
 * \param[in] i18n The i18n context.
 * \param[in] template_str The template string containing placeholders.
 * \param[in] keys The array of placeholder keys.
 * \param[in] values The array of replacement values.
 * \param[in] count The number of key-value pairs.
 * \param[out] out_str The output buffer.
 * \param[in] out_len The size of the output buffer.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_i18n_interpolate(struct ui_i18n *i18n, const char *template_str,
                               const char **keys, const char **values,
                               size_t count, char *out_str, size_t out_len) {
  size_t t_idx = 0;
  size_t o_idx = 0;

  if (!i18n || !template_str || !out_str || out_len == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  while (template_str[t_idx] != '\0' && o_idx < out_len - 1) {
    if (template_str[t_idx] == '{' && template_str[t_idx + 1] == '{') {
      size_t end_idx = t_idx + 2;
      while (
          template_str[end_idx] != '\0' &&
          !(template_str[end_idx] == '}' && template_str[end_idx + 1] == '}')) {
        end_idx++;
      }

      if (template_str[end_idx] == '}') {
        size_t key_len = end_idx - (t_idx + 2);
        size_t k;
        int found = 0;

        for (k = 0; k < count; k++) {
          if (strncmp(&template_str[t_idx + 2], keys[k], key_len) == 0 &&
              strlen(keys[k]) == key_len) {
            size_t v_len = strlen(values[k]);
            if (o_idx + v_len < out_len - 1) {
              (void)safe_strcpy(&out_str[o_idx], out_len - o_idx, values[k]);
              o_idx += v_len;
              found = 1;
              break;
            }
          }
        }

        if (found) {
          t_idx = end_idx + 2;
          continue;
        }
      }
    }

    out_str[o_idx++] = template_str[t_idx++];
  }
  out_str[o_idx] = '\0';

  return UI_ERROR_NONE;
}
