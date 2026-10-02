/**
 * @file cupertino_bidi.c
 * @brief Implementation of Cupertino & Apple HIG Bidirectional layout
 * mirroring, localized numerals, multi-calendar conversion, and text rules.
 */

/* clang-format off */
#include "cupertino/cupertino_bidi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

ui_error_t cupertino_bidi_set_direction(enum ui_bidi_direction direction) {
  if (direction != UI_BIDI_DIR_LTR && direction != UI_BIDI_DIR_RTL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bidi_set_direction(direction);
}

ui_error_t cupertino_bidi_get_direction(enum ui_bidi_direction *out_dir) {
  if (!out_dir) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bidi_get_direction(out_dir);
}

ui_error_t cupertino_bidi_resolve_logical_margin(float start, float end,
                                                 enum ui_bidi_direction dir,
                                                 float *out_left,
                                                 float *out_right) {
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

ui_error_t cupertino_bidi_resolve_logical_padding(float start, float end,
                                                  enum ui_bidi_direction dir,
                                                  float *out_left,
                                                  float *out_right) {
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

ui_error_t cupertino_bidi_resolve_logical_inset(float start, float end,
                                                enum ui_bidi_direction dir,
                                                float *out_left,
                                                float *out_right) {
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

ui_error_t cupertino_bidi_should_mirror_symbol(const char *symbol_name,
                                               enum ui_bidi_direction dir,
                                               int *out_should_mirror) {
  if (!symbol_name || !out_should_mirror) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (dir != UI_BIDI_DIR_RTL) {
    *out_should_mirror = 0;
    return UI_ERROR_NONE;
  }

  /* Media playback, sound, clock and timeline symbols are NOT mirrored in Apple
   * HIG */
  if (strcmp(symbol_name, "play.fill") == 0 ||
      strcmp(symbol_name, "pause.fill") == 0 ||
      strcmp(symbol_name, "stop.fill") == 0 ||
      strcmp(symbol_name, "forward.fill") == 0 ||
      strcmp(symbol_name, "backward.fill") == 0 ||
      strcmp(symbol_name, "speaker.wave.2.fill") == 0 ||
      strcmp(symbol_name, "speaker.slash.fill") == 0 ||
      strcmp(symbol_name, "clock") == 0 || strcmp(symbol_name, "timer") == 0 ||
      strcmp(symbol_name, "stopwatch") == 0) {
    *out_should_mirror = 0;
    return UI_ERROR_NONE;
  }

  /* Directional chevrons, arrows, list disclosure indicators ARE mirrored in
   * RTL */
  if (strcmp(symbol_name, "chevron.left") == 0 ||
      strcmp(symbol_name, "chevron.right") == 0 ||
      strcmp(symbol_name, "chevron.backward") == 0 ||
      strcmp(symbol_name, "chevron.forward") == 0 ||
      strcmp(symbol_name, "arrow.left") == 0 ||
      strcmp(symbol_name, "arrow.right") == 0 ||
      strcmp(symbol_name, "arrow.backward") == 0 ||
      strcmp(symbol_name, "arrow.forward") == 0 ||
      strcmp(symbol_name, "arrow.uturn.backward") == 0 ||
      strcmp(symbol_name, "arrow.uturn.forward") == 0 ||
      strcmp(symbol_name, "sidebar.left") == 0 ||
      strcmp(symbol_name, "sidebar.right") == 0 ||
      strcmp(symbol_name, "line.3.horizontal") == 0) {
    *out_should_mirror = 1;
    return UI_ERROR_NONE;
  }

  *out_should_mirror = 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_i18n_format_numeral(int number,
                                         enum cupertino_numeral_system system,
                                         char *out_buf, size_t buf_size) {
  char temp[32];
  size_t len, i, pos;
  int is_neg;
  unsigned int val;

  static const char *const eastern_digits[10] = {
      "\xD9\xA0", "\xD9\xA1", "\xD9\xA2", "\xD9\xA3", "\xD9\xA4",
      "\xD9\xA5", "\xD9\xA6", "\xD9\xA7", "\xD9\xA8", "\xD9\xA9"};

  static const char *const persian_digits[10] = {
      "\xDB\xB0", "\xDB\xB1", "\xDB\xB2", "\xDB\xB3", "\xDB\xB4",
      "\xDB\xB5", "\xDB\xB6", "\xDB\xB7", "\xDB\xB8", "\xDB\xB9"};

  static const char *const devanagari_digits[10] = {
      "\xE0\xA5\xA6", "\xE0\xA5\xA7", "\xE0\xA5\xA8", "\xE0\xA5\xA9",
      "\xE0\xA5\xAA", "\xE0\xA5\xAB", "\xE0\xA5\xAC", "\xE0\xA5\xAD",
      "\xE0\xA5\xAE", "\xE0\xA5\xAF"};

  if (!out_buf || buf_size < 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (system != CUPERTINO_NUMERAL_WESTERN &&
      system != CUPERTINO_NUMERAL_EASTERN_ARABIC &&
      system != CUPERTINO_NUMERAL_PERSIAN &&
      system != CUPERTINO_NUMERAL_DEVANAGARI) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  is_neg = (number < 0);
  val = is_neg ? (unsigned int)(-number) : (unsigned int)number;

#if defined(_MSC_VER)
  sprintf_s(temp, sizeof(temp), "%u", val);
#else
  snprintf(temp, sizeof(temp), "%u", val);
#endif

  if (system == CUPERTINO_NUMERAL_WESTERN) {
    pos = 0;
    if (is_neg) {
      if (buf_size < 3) {
        return UI_ERROR_OUT_OF_BOUNDS;
      }
      out_buf[pos++] = '-';
    }
    len = strlen(temp);
    if (pos + len >= buf_size) {
      return UI_ERROR_OUT_OF_BOUNDS;
    }
#if defined(_MSC_VER)
    strcpy_s(out_buf + pos, buf_size - pos, temp);
#else
    strcpy(out_buf + pos, temp);
#endif
    return UI_ERROR_NONE;
  }

  pos = 0;
  if (is_neg) {
    out_buf[pos++] = '-';
  }

  len = strlen(temp);
  for (i = 0; i < len; ++i) {
    int d = temp[i] - '0';
    if (system == CUPERTINO_NUMERAL_EASTERN_ARABIC) {
      const char *utf8_digit = eastern_digits[d];
      if (pos + 2 >= buf_size) {
        return UI_ERROR_OUT_OF_BOUNDS;
      }
      out_buf[pos++] = utf8_digit[0];
      out_buf[pos++] = utf8_digit[1];
    } else if (system == CUPERTINO_NUMERAL_PERSIAN) {
      const char *utf8_digit = persian_digits[d];
      if (pos + 2 >= buf_size) {
        return UI_ERROR_OUT_OF_BOUNDS;
      }
      out_buf[pos++] = utf8_digit[0];
      out_buf[pos++] = utf8_digit[1];
    } else {
      const char *utf8_digit = devanagari_digits[d];
      if (pos + 3 >= buf_size) {
        return UI_ERROR_OUT_OF_BOUNDS;
      }
      out_buf[pos++] = utf8_digit[0];
      out_buf[pos++] = utf8_digit[1];
      out_buf[pos++] = utf8_digit[2];
    }
  }

  out_buf[pos] = '\0';
  return UI_ERROR_NONE;
}

ui_error_t cupertino_calendar_convert_from_gregorian(
    int g_year, int g_month, int g_day,
    enum cupertino_calendar_type target_type,
    struct cupertino_calendar_date *out_date) {
  if (!out_date || g_month < 1 || g_month > 12 || g_day < 1 || g_day > 31) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(out_date, 0, sizeof(*out_date));
  out_date->type = target_type;

  switch (target_type) {
  case CUPERTINO_CALENDAR_BUDDHIST:
    out_date->year = g_year + 543;
    out_date->month = g_month;
    out_date->day = g_day;
#if defined(_MSC_VER)
    strcpy_s(out_date->era_name, sizeof(out_date->era_name), "BE");
#else
    strncpy(out_date->era_name, "BE", sizeof(out_date->era_name) - 1);
#endif
    break;

  case CUPERTINO_CALENDAR_JAPANESE: {
    /* Era definitions for modern Japan */
    if (g_year > 2019 || (g_year == 2019 && g_month >= 5)) {
      out_date->year = g_year - 2018; /* Reiwa began May 1, 2019 = Year 1 */
#if defined(_MSC_VER)
      strcpy_s(out_date->era_name, sizeof(out_date->era_name), "Reiwa");
#else
      strncpy(out_date->era_name, "Reiwa", sizeof(out_date->era_name) - 1);
#endif
    } else if (g_year > 1989 ||
               (g_year == 1989 && (g_month > 1 || g_day >= 8))) {
      out_date->year = g_year - 1988; /* Heisei began Jan 8, 1989 = Year 1 */
#if defined(_MSC_VER)
      strcpy_s(out_date->era_name, sizeof(out_date->era_name), "Heisei");
#else
      strncpy(out_date->era_name, "Heisei", sizeof(out_date->era_name) - 1);
#endif
    } else if (g_year >= 1926) {
      out_date->year = g_year - 1925; /* Showa */
#if defined(_MSC_VER)
      strcpy_s(out_date->era_name, sizeof(out_date->era_name), "Showa");
#else
      strncpy(out_date->era_name, "Showa", sizeof(out_date->era_name) - 1);
#endif
    } else {
      out_date->year = g_year;
#if defined(_MSC_VER)
      strcpy_s(out_date->era_name, sizeof(out_date->era_name), "AD");
#else
      strncpy(out_date->era_name, "AD", sizeof(out_date->era_name) - 1);
#endif
    }
    out_date->month = g_month;
    out_date->day = g_day;
    break;
  }

  case CUPERTINO_CALENDAR_ISLAMIC_HIJRI: {
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
#if defined(_MSC_VER)
    strcpy_s(out_date->era_name, sizeof(out_date->era_name), "AH");
#else
    strncpy(out_date->era_name, "AH", sizeof(out_date->era_name) - 1);
#endif
    break;
  }

  case CUPERTINO_CALENDAR_HEBREW:
    out_date->year = g_year + 3760;
    out_date->month = g_month;
    out_date->day = g_day;
#if defined(_MSC_VER)
    strcpy_s(out_date->era_name, sizeof(out_date->era_name), "AM");
#else
    strncpy(out_date->era_name, "AM", sizeof(out_date->era_name) - 1);
#endif
    break;

  case CUPERTINO_CALENDAR_GREGORIAN:
  default:
    out_date->year = g_year;
    out_date->month = g_month;
    out_date->day = g_day;
#if defined(_MSC_VER)
    strcpy_s(out_date->era_name, sizeof(out_date->era_name), "AD");
#else
    strncpy(out_date->era_name, "AD", sizeof(out_date->era_name) - 1);
#endif
    break;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_i18n_get_first_day_of_week(const char *locale,
                                                int *out_first_day) {
  if (!locale || !out_first_day) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Islamic countries start on Saturday (6) */
  if (strncmp(locale, "ar_SA", 5) == 0 || strncmp(locale, "ar-SA", 5) == 0 ||
      strncmp(locale, "fa_IR", 5) == 0 || strncmp(locale, "fa-IR", 5) == 0 ||
      strncmp(locale, "ar_EG", 5) == 0 || strncmp(locale, "ar-EG", 5) == 0) {
    *out_first_day = 6; /* Saturday */
    return UI_ERROR_NONE;
  }

  /* US, Canada, Japan, Australia start on Sunday (0) */
  if (strstr(locale, "US") || strstr(locale, "CA") || strstr(locale, "JP") ||
      strstr(locale, "AU") || strncmp(locale, "en", 2) == 0) {
    *out_first_day = 0; /* Sunday */
    return UI_ERROR_NONE;
  }

  /* European and international default starts on Monday (1) */
  *out_first_day = 1; /* Monday */
  return UI_ERROR_NONE;
}

ui_error_t cupertino_i18n_get_plural_category(
    int count, const char *locale,
    enum cupertino_plural_category *out_category) {
  int n;

  if (!locale || !out_category) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  n = (count < 0) ? -count : count;

  /* Arabic CLDR categories */
  if (strncmp(locale, "ar", 2) == 0) {
    if (n == 0) {
      *out_category = CUPERTINO_PLURAL_ZERO;
    } else if (n == 1) {
      *out_category = CUPERTINO_PLURAL_ONE;
    } else if (n == 2) {
      *out_category = CUPERTINO_PLURAL_TWO;
    } else if ((n % 100) >= 3 && (n % 100) <= 10) {
      *out_category = CUPERTINO_PLURAL_FEW;
    } else if ((n % 100) >= 11) {
      *out_category = CUPERTINO_PLURAL_MANY;
    } else {
      *out_category = CUPERTINO_PLURAL_OTHER;
    }
    return UI_ERROR_NONE;
  }

  /* Slavic (Russian, Ukrainian) CLDR categories */
  if (strncmp(locale, "ru", 2) == 0 || strncmp(locale, "uk", 2) == 0) {
    if ((n % 10) == 1 && (n % 100) != 11) {
      *out_category = CUPERTINO_PLURAL_ONE;
    } else if ((n % 10) >= 2 && (n % 10) <= 4 &&
               ((n % 100) < 10 || (n % 100) >= 20)) {
      *out_category = CUPERTINO_PLURAL_FEW;
    } else {
      *out_category = CUPERTINO_PLURAL_MANY;
    }
    return UI_ERROR_NONE;
  }

  /* English / Germanic / Romance baseline */
  if (n == 1) {
    *out_category = CUPERTINO_PLURAL_ONE;
  } else {
    *out_category = CUPERTINO_PLURAL_OTHER;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_i18n_kinsoku_prohibited_start(const char *utf8_char,
                                                   int *out_prohibited) {
  if (!utf8_char || !out_prohibited) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Characters prohibited at line start in Japanese Kinsoku Shori:
     Closing punctuation: ), ], }, 。, 、, 〕, 〉, 》, 」, 』, ﹂, ｣, ?, !, ：,
     ；
  */
  if (strcmp(utf8_char, ")") == 0 || strcmp(utf8_char, "]") == 0 ||
      strcmp(utf8_char, "}") == 0 ||
      strcmp(utf8_char, "\xE3\x80\x82") == 0 /* 。 */ ||
      strcmp(utf8_char, "\xE3\x80\x81") == 0 /* 、 */ ||
      strcmp(utf8_char, "\xE3\x80\x8D") == 0 /* 」 */ ||
      strcmp(utf8_char, "\xE3\x80\x8F") == 0 /* 』 */ ||
      strcmp(utf8_char, "\xEF\xBC\x81") == 0 /* ！ */ ||
      strcmp(utf8_char, "\xEF\xBC\x9F") == 0 /* ？ */ ||
      strcmp(utf8_char, ",") == 0 || strcmp(utf8_char, ".") == 0 ||
      strcmp(utf8_char, "!") == 0 || strcmp(utf8_char, "?") == 0) {
    *out_prohibited = 1;
    return UI_ERROR_NONE;
  }

  *out_prohibited = 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_i18n_kinsoku_prohibited_end(const char *utf8_char,
                                                 int *out_prohibited) {
  if (!utf8_char || !out_prohibited) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Characters prohibited at line end in Japanese Kinsoku Shori:
     Opening punctuation: (, [, {, （, 〔, 〈, 《, 「, 『, ﹁, ｢
  */
  if (strcmp(utf8_char, "(") == 0 || strcmp(utf8_char, "[") == 0 ||
      strcmp(utf8_char, "{") == 0 ||
      strcmp(utf8_char, "\xE3\x80\x8C") == 0 /* 「 */ ||
      strcmp(utf8_char, "\xE3\x80\x8E") == 0 /* 『 */ ||
      strcmp(utf8_char, "\xEF\xBC\x88") == 0 /* （ */) {
    *out_prohibited = 1;
    return UI_ERROR_NONE;
  }

  *out_prohibited = 0;
  return UI_ERROR_NONE;
}
