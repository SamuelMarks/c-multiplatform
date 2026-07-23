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

#define MAX_LOCALE_LEN 32

struct ui_i18n {
  char locale[MAX_LOCALE_LEN];
  struct ui_signal *locale_signal;
};

static enum ui_error safe_strcpy(char *dst, size_t sz, const char *src) {
#if defined(_MSC_VER)
  strcpy_s(dst, sz, src);
#else
  strncpy(dst, src, sz - 1);
  dst[sz - 1] = '\0';
#endif
  return UI_ERROR_NONE;
}

enum ui_error ui_i18n_create(struct ui_i18n **out_i18n) {
  struct ui_i18n *i18n;

  if (!out_i18n) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  i18n = (struct ui_i18n *)UI_MALLOC(sizeof(struct ui_i18n));
  if (!i18n) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  (void)safe_strcpy(i18n->locale, MAX_LOCALE_LEN, "en-US");
  i18n->locale_signal = NULL;

  *out_i18n = i18n;
  return UI_ERROR_NONE;
}

enum ui_error ui_i18n_destroy(struct ui_i18n *i18n) {
  if (!i18n) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  UI_FREE(i18n);
  return UI_ERROR_NONE;
}

enum ui_error ui_i18n_set_locale(struct ui_i18n *i18n, const char *locale) {
  if (!i18n || !locale) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)safe_strcpy(i18n->locale, MAX_LOCALE_LEN, locale);

  /* Check for RTL locales and update bidi manager */
  if (strncmp(locale, "ar", 2) == 0 || strncmp(locale, "he", 2) == 0 ||
      strncmp(locale, "fa", 2) == 0 || strncmp(locale, "ur", 2) == 0) {
    ui_bidi_set_direction(UI_BIDI_DIR_RTL);
  } else {
    ui_bidi_set_direction(UI_BIDI_DIR_LTR);
  }

#if defined(__EMSCRIPTEN__)
  /* Send to root element */
  ui_web_bridge_set_style(0, "lang", locale);
#endif
  return UI_ERROR_NONE;

  return UI_ERROR_NONE;
}

enum ui_error ui_i18n_get_locale(struct ui_i18n *i18n,
                                 const char **out_locale) {
  if (!i18n || !out_locale) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_locale = i18n->locale;
  return UI_ERROR_NONE;
}

enum ui_error ui_i18n_bind_locale_signal(struct ui_i18n *i18n,
                                         struct ui_signal *locale_signal) {
  if (!i18n || !locale_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  i18n->locale_signal = locale_signal;
  return UI_ERROR_NONE;
}

enum ui_error ui_i18n_format_number(struct ui_i18n *i18n, double value,
                                    int decimals, char *out_str,
                                    size_t out_len) {
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
  return UI_ERROR_NONE;
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
  return UI_ERROR_NONE;
#endif
  return UI_ERROR_NONE;

  return UI_ERROR_NONE;
}

enum ui_error ui_i18n_format_currency(struct ui_i18n *i18n, double amount,
                                      const char *currency_code, char *out_str,
                                      size_t out_len) {
  char number_buf[64];

  if (!i18n || !currency_code || !out_str || out_len == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)ui_i18n_format_number(i18n, amount, 2, number_buf, sizeof(number_buf));

#if defined(_MSC_VER)
  sprintf_s(out_str, out_len, "%s %s", number_buf, currency_code);
#else
  sprintf(out_str, "%s %s", number_buf, currency_code);
#endif
  return UI_ERROR_NONE;

  return UI_ERROR_NONE;
}

enum ui_error ui_i18n_format_date(struct ui_i18n *i18n, double timestamp_ms,
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
  return UI_ERROR_NONE;
}

enum ui_error ui_i18n_pluralize(struct ui_i18n *i18n, int count,
                                const char *zero, const char *one,
                                const char *other, char *out_str,
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

enum ui_error ui_i18n_interpolate(struct ui_i18n *i18n,
                                  const char *template_str, const char **keys,
                                  const char **values, size_t count,
                                  char *out_str, size_t out_len) {
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

      if (template_str[end_idx] == '}' && template_str[end_idx + 1] == '}') {
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
