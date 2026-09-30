/**
 * @file md3_bidi.c
 * @brief Implementation of Material 3 Bidirectional (BiDi) layout, mirroring,
 * font cascades, and internationalization utilities.
 */

/* clang-format off */
#include "material3/md3_bidi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

ui_error_t md3_bidi_set_direction(enum ui_bidi_direction direction) {
  if (direction != UI_BIDI_DIR_LTR && direction != UI_BIDI_DIR_RTL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_bidi_set_direction(direction);
}

ui_error_t md3_bidi_get_direction(enum ui_bidi_direction *out_dir) {
  if (!out_dir) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_bidi_get_direction(out_dir);
}

ui_error_t md3_bidi_resolve_logical_margin(float start, float end,
                                           enum ui_bidi_direction dir,
                                           float *out_left, float *out_right) {
  if (!out_left || !out_right) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (dir == UI_BIDI_DIR_RTL) {
    *out_left = end;
    *out_right = start;
  } else {
    *out_left = start;
    *out_right = end;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_bidi_resolve_logical_padding(float start, float end,
                                            enum ui_bidi_direction dir,
                                            float *out_left, float *out_right) {
  if (!out_left || !out_right) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (dir == UI_BIDI_DIR_RTL) {
    *out_left = end;
    *out_right = start;
  } else {
    *out_left = start;
    *out_right = end;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_bidi_resolve_logical_inset(float start, float end,
                                          enum ui_bidi_direction dir,
                                          float *out_left, float *out_right) {
  if (!out_left || !out_right) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (dir == UI_BIDI_DIR_RTL) {
    *out_left = end;
    *out_right = start;
  } else {
    *out_left = start;
    *out_right = end;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_bidi_should_mirror_icon(const char *icon_name,
                                       enum ui_bidi_direction dir,
                                       int *out_should_mirror) {
  if (!icon_name || !out_should_mirror) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (dir != UI_BIDI_DIR_RTL) {
    *out_should_mirror = 0;
    return UI_ERROR_NONE;
  }

  /* Media controls and clocks MUST NOT be mirrored in RTL */
  if (strcmp(icon_name, "play") == 0 || strcmp(icon_name, "pause") == 0 ||
      strcmp(icon_name, "stop") == 0 ||
      strcmp(icon_name, "fast_forward") == 0 ||
      strcmp(icon_name, "fast_rewind") == 0 ||
      strcmp(icon_name, "replay") == 0 || strcmp(icon_name, "skip_next") == 0 ||
      strcmp(icon_name, "skip_previous") == 0 ||
      strcmp(icon_name, "volume_up") == 0 ||
      strcmp(icon_name, "volume_down") == 0 ||
      strcmp(icon_name, "volume_mute") == 0 ||
      strcmp(icon_name, "volume_off") == 0 ||
      strcmp(icon_name, "music_note") == 0 ||
      strcmp(icon_name, "access_time") == 0 ||
      strcmp(icon_name, "schedule") == 0 || strcmp(icon_name, "timer") == 0 ||
      strcmp(icon_name, "watch") == 0) {
    *out_should_mirror = 0;
    return UI_ERROR_NONE;
  }

  /* Directional navigation chevrons and back/forward icons ARE mirrored */
  if (strcmp(icon_name, "arrow_back") == 0 ||
      strcmp(icon_name, "arrow_forward") == 0 ||
      strcmp(icon_name, "arrow_left") == 0 ||
      strcmp(icon_name, "arrow_right") == 0 ||
      strcmp(icon_name, "chevron_left") == 0 ||
      strcmp(icon_name, "chevron_right") == 0 ||
      strcmp(icon_name, "navigate_before") == 0 ||
      strcmp(icon_name, "navigate_next") == 0 ||
      strcmp(icon_name, "menu_open") == 0 || strcmp(icon_name, "drawer") == 0 ||
      strcmp(icon_name, "redo") == 0 || strcmp(icon_name, "undo") == 0 ||
      strcmp(icon_name, "first_page") == 0 ||
      strcmp(icon_name, "last_page") == 0) {
    *out_should_mirror = 1;
    return UI_ERROR_NONE;
  }

  *out_should_mirror = 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_bidi_get_fallback_font_family(const char *script,
                                             const char **out_font_family) {
  if (!script || !out_font_family) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(script, "Arab") == 0) {
    *out_font_family = "Noto Sans Arabic";
  } else if (strcmp(script, "Hebr") == 0) {
    *out_font_family = "Noto Sans Hebrew";
  } else if (strcmp(script, "Deva") == 0) {
    *out_font_family = "Noto Sans Devanagari";
  } else if (strcmp(script, "Thai") == 0) {
    *out_font_family = "Noto Sans Thai";
  } else {
    *out_font_family = "Noto Sans";
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_bidi_get_script_line_height_multiplier(const char *script,
                                                      float *out_multiplier) {
  if (!script || !out_multiplier) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(script, "Arab") == 0) {
    *out_multiplier = 1.25f;
  } else if (strcmp(script, "Thai") == 0) {
    *out_multiplier = 1.20f;
  } else if (strcmp(script, "Deva") == 0) {
    *out_multiplier = 1.15f;
  } else {
    *out_multiplier = 1.00f;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_i18n_format_eastern_arabic_numerals(int number, char *out_buf,
                                                   size_t buf_size) {
  char temp[32];
  size_t len, i, pos;
  int is_neg;
  unsigned int val;

  static const char *const eastern_digits[10] = {
      "\xD9\xA0", "\xD9\xA1", "\xD9\xA2", "\xD9\xA3", "\xD9\xA4",
      "\xD9\xA5", "\xD9\xA6", "\xD9\xA7", "\xD9\xA8", "\xD9\xA9"};

  if (!out_buf || buf_size < 4) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  is_neg = (number < 0);
  val = is_neg ? (unsigned int)(-number) : (unsigned int)number;

#if defined(_MSC_VER)
  sprintf_s(temp, sizeof(temp), "%u", val);
#else
  snprintf(temp, sizeof(temp), "%u", val);
#endif

  len = strlen(temp);
  pos = 0;

  if (is_neg) {
    out_buf[pos++] = '-';
  }

  for (i = 0; i < len; ++i) {
    int d = temp[i] - '0';
    const char *utf8_digit = eastern_digits[d];
    if (pos + 2 >= buf_size) {
      return UI_ERROR_OUT_OF_BOUNDS;
    }
    out_buf[pos++] = utf8_digit[0];
    out_buf[pos++] = utf8_digit[1];
  }

  out_buf[pos] = '\0';
  return UI_ERROR_NONE;
}

ui_error_t md3_i18n_format_localized_decimal(double number, int decimals,
                                             const char *locale, char *out_buf,
                                             size_t buf_size) {
  char temp[64];
  size_t i;
  char decimal_sep;

  if (!locale || !out_buf || buf_size < 8 || decimals < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* European locales use comma as decimal separator */
  if (strncmp(locale, "de", 2) == 0 || strncmp(locale, "fr", 2) == 0 ||
      strncmp(locale, "es", 2) == 0 || strncmp(locale, "it", 2) == 0 ||
      strncmp(locale, "ru", 2) == 0) {
    decimal_sep = ',';
  } else {
    decimal_sep = '.';
  }

#if defined(_MSC_VER)
  sprintf_s(temp, sizeof(temp), "%.*f", decimals, number);
#else
  snprintf(temp, sizeof(temp), "%.*f", decimals, number);
#endif

  for (i = 0; temp[i] != '\0'; ++i) {
    if (temp[i] == '.') {
      temp[i] = decimal_sep;
    }
  }

#if defined(_MSC_VER)
  strncpy_s(out_buf, buf_size, temp, _TRUNCATE);
#else
  strncpy(out_buf, temp, buf_size - 1);
  out_buf[buf_size - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t
md3_calendar_convert_from_gregorian(int g_year, int g_month, int g_day,
                                    enum md3_calendar_type target_type,
                                    struct md3_calendar_date *out_date) {
  if (!out_date || g_month < 1 || g_month > 12 || g_day < 1 || g_day > 31) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_date->type = target_type;

  switch (target_type) {
  case MD3_CALENDAR_BUDDHIST:
    out_date->year = g_year + 543;
    out_date->month = g_month;
    out_date->day = g_day;
    break;

  case MD3_CALENDAR_ISLAMIC_HIJRI: {
    long a, y, m, jdn, l, n, j, m_h, d_h, y_h;

    a = (14 - (long)g_month) / 12;
    y = (long)g_year + 4800 - a;
    m = (long)g_month + 12 * a - 3;
    jdn = (long)g_day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 +
          y / 400 - 32045;

    l = jdn - 1948440 + 10632;
    n = (l - 1) / 10631;
    l = l - 10631 * n + 354;
    j = ((10985 - l) / 5316) * ((50 * l) / 17719) +
        (l / 5670) * ((43 * l) / 15238);
    l = l - ((30 - j) / 15) * ((17719 * j) / 50) -
        (j / 16) * ((15238 * j) / 43) + 29;
    m_h = (24 * l) / 709;
    d_h = l - (709 * m_h) / 24;
    y_h = 30 * n + j - 30;

    out_date->year = (int)y_h;
    out_date->month = (int)m_h;
    out_date->day = (int)d_h;
    break;
  }

  case MD3_CALENDAR_HEBREW:
    /* Approximated Hebrew calendar year conversion */
    out_date->year = g_year + 3760;
    out_date->month = g_month;
    out_date->day = g_day;
    break;

  case MD3_CALENDAR_GREGORIAN:
  default:
    out_date->year = g_year;
    out_date->month = g_month;
    out_date->day = g_day;
    break;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_i18n_get_plural_form(int count, const char *locale,
                                    enum md3_plural_form *out_form) {
  int n;

  if (!locale || !out_form) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  n = (count < 0) ? -count : count;

  /* Arabic CLDR plural rules */
  if (strncmp(locale, "ar", 2) == 0) {
    if (n == 0) {
      *out_form = MD3_PLURAL_ZERO;
    } else if (n == 1) {
      *out_form = MD3_PLURAL_ONE;
    } else if (n == 2) {
      *out_form = MD3_PLURAL_TWO;
    } else if ((n % 100) >= 3 && (n % 100) <= 10) {
      *out_form = MD3_PLURAL_FEW;
    } else if ((n % 100) >= 11) {
      *out_form = MD3_PLURAL_MANY;
    } else {
      *out_form = MD3_PLURAL_OTHER;
    }
    return UI_ERROR_NONE;
  }

  /* Slavic (Russian, Ukrainian) CLDR plural rules */
  if (strncmp(locale, "ru", 2) == 0 || strncmp(locale, "uk", 2) == 0) {
    if ((n % 10) == 1 && (n % 100) != 11) {
      *out_form = MD3_PLURAL_ONE;
    } else if ((n % 10) >= 2 && (n % 10) <= 4 &&
               ((n % 100) < 10 || (n % 100) >= 20)) {
      *out_form = MD3_PLURAL_FEW;
    } else {
      *out_form = MD3_PLURAL_MANY;
    }
    return UI_ERROR_NONE;
  }

  /* English / Germanic / Romance baseline rules */
  if (n == 1) {
    *out_form = MD3_PLURAL_ONE;
  } else {
    *out_form = MD3_PLURAL_OTHER;
  }

  return UI_ERROR_NONE;
}
