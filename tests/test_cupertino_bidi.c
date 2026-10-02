/**
 * @file test_cupertino_bidi.c
 * @brief Unit tests for Cupertino & Apple HIG Bidirectional layout mirroring,
 * localized numerals, multi-calendar conversion, and Kinsoku Shori rules.
 */

/* clang-format off */
#include "cupertino/cupertino_bidi.h"
#include "greatest.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_cupertino_bidi_direction_and_logical(void) {
  enum ui_bidi_direction dir = UI_BIDI_DIR_LTR;
  float l = 0.0f, r = 0.0f;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_bidi_set_direction((enum ui_bidi_direction)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_bidi_get_direction(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_bidi_resolve_logical_margin(10.0f, 20.0f, UI_BIDI_DIR_LTR,
                                             NULL, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_bidi_resolve_logical_margin(10.0f, 20.0f, UI_BIDI_DIR_LTR, &l,
                                             NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_bidi_resolve_logical_padding(10.0f, 20.0f, UI_BIDI_DIR_LTR,
                                              NULL, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_bidi_resolve_logical_padding(10.0f, 20.0f, UI_BIDI_DIR_LTR, &l,
                                              NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_bidi_resolve_logical_inset(10.0f, 20.0f, UI_BIDI_DIR_LTR, NULL,
                                            &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_bidi_resolve_logical_inset(10.0f, 20.0f, UI_BIDI_DIR_LTR, &l,
                                            NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set LTR */
  rc = cupertino_bidi_set_direction(UI_BIDI_DIR_LTR);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_bidi_get_direction(&dir);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_BIDI_DIR_LTR, dir);

  /* LTR resolution: left = start, right = end */
  rc = cupertino_bidi_resolve_logical_margin(16.0f, 32.0f, UI_BIDI_DIR_LTR, &l,
                                             &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(16.0f, l);
  ASSERT_EQ(32.0f, r);

  rc = cupertino_bidi_resolve_logical_padding(8.0f, 24.0f, UI_BIDI_DIR_LTR, &l,
                                              &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(8.0f, l);
  ASSERT_EQ(24.0f, r);

  rc = cupertino_bidi_resolve_logical_inset(4.0f, 12.0f, UI_BIDI_DIR_LTR, &l,
                                            &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4.0f, l);
  ASSERT_EQ(12.0f, r);

  /* Set RTL */
  rc = cupertino_bidi_set_direction(UI_BIDI_DIR_RTL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_bidi_get_direction(&dir);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_BIDI_DIR_RTL, dir);

  /* RTL resolution: left = end, right = start */
  rc = cupertino_bidi_resolve_logical_margin(16.0f, 32.0f, UI_BIDI_DIR_RTL, &l,
                                             &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(32.0f, l);
  ASSERT_EQ(16.0f, r);

  rc = cupertino_bidi_resolve_logical_padding(8.0f, 24.0f, UI_BIDI_DIR_RTL, &l,
                                              &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(24.0f, l);
  ASSERT_EQ(8.0f, r);

  rc = cupertino_bidi_resolve_logical_inset(4.0f, 12.0f, UI_BIDI_DIR_RTL, &l,
                                            &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(12.0f, l);
  ASSERT_EQ(4.0f, r);

  PASS();
}

TEST test_cupertino_bidi_symbol_mirroring(void) {
  int mirror = 0;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_bidi_should_mirror_symbol(NULL, UI_BIDI_DIR_RTL, &mirror);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_bidi_should_mirror_symbol("chevron.left", UI_BIDI_DIR_RTL,
                                           NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* LTR never mirrors */
  rc = cupertino_bidi_should_mirror_symbol("chevron.left", UI_BIDI_DIR_LTR,
                                           &mirror);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, mirror);

  /* Directional SF symbols in RTL mirror */
  {
    static const char *const mirrored[] = {
        "chevron.left",        "chevron.right", "chevron.backward",
        "chevron.forward",     "arrow.left",    "arrow.right",
        "arrow.backward",      "arrow.forward", "arrow.uturn.backward",
        "arrow.uturn.forward", "sidebar.left",  "sidebar.right",
        "line.3.horizontal"};
    size_t i;
    for (i = 0; i < sizeof(mirrored) / sizeof(mirrored[0]); ++i) {
      rc = cupertino_bidi_should_mirror_symbol(mirrored[i], UI_BIDI_DIR_RTL,
                                               &mirror);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      ASSERT_EQ(1, mirror);
    }
  }

  /* Media playback & clocks MUST NOT mirror */
  {
    static const char *const non_mirrored[] = {
        "play.fill",          "pause.fill",    "stop.fill",
        "forward.fill",       "backward.fill", "speaker.wave.2.fill",
        "speaker.slash.fill", "clock",         "timer",
        "stopwatch"};
    size_t i;
    for (i = 0; i < sizeof(non_mirrored) / sizeof(non_mirrored[0]); ++i) {
      rc = cupertino_bidi_should_mirror_symbol(non_mirrored[i], UI_BIDI_DIR_RTL,
                                               &mirror);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      ASSERT_EQ(0, mirror);
    }
  }

  /* Unrecognized symbols do not mirror */
  rc = cupertino_bidi_should_mirror_symbol("gear", UI_BIDI_DIR_RTL, &mirror);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, mirror);

  PASS();
}

TEST test_cupertino_i18n_numerals(void) {
  char buf[64];
  ui_error_t rc;

  /* Invalid args */
  rc = cupertino_i18n_format_numeral(123, CUPERTINO_NUMERAL_WESTERN, NULL, 64);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_i18n_format_numeral(123, CUPERTINO_NUMERAL_WESTERN, buf, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_i18n_format_numeral(123, (enum cupertino_numeral_system)99,
                                     buf, 64);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Western numerals */
  rc = cupertino_i18n_format_numeral(42, CUPERTINO_NUMERAL_WESTERN, buf,
                                     sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("42", buf);

  rc = cupertino_i18n_format_numeral(-105, CUPERTINO_NUMERAL_WESTERN, buf,
                                     sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("-105", buf);

  /* Bounds check for Western */
  rc = cupertino_i18n_format_numeral(12345, CUPERTINO_NUMERAL_WESTERN, buf, 4);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_i18n_format_numeral(-5, CUPERTINO_NUMERAL_WESTERN, buf, 2);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Eastern Arabic */
  rc = cupertino_i18n_format_numeral(123, CUPERTINO_NUMERAL_EASTERN_ARABIC, buf,
                                     sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(strlen(buf) > 0);

  rc = cupertino_i18n_format_numeral(-7, CUPERTINO_NUMERAL_EASTERN_ARABIC, buf,
                                     sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('-', buf[0]);

  /* Persian */
  rc = cupertino_i18n_format_numeral(890, CUPERTINO_NUMERAL_PERSIAN, buf,
                                     sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(strlen(buf) > 0);

  rc = cupertino_i18n_format_numeral(-890, CUPERTINO_NUMERAL_PERSIAN, buf,
                                     sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('-', buf[0]);

  /* Devanagari */
  rc = cupertino_i18n_format_numeral(567, CUPERTINO_NUMERAL_DEVANAGARI, buf,
                                     sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(strlen(buf) > 0);

  rc = cupertino_i18n_format_numeral(-567, CUPERTINO_NUMERAL_DEVANAGARI, buf,
                                     sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('-', buf[0]);

  /* Bounds checks for multi-byte numerals */
  rc = cupertino_i18n_format_numeral(123, CUPERTINO_NUMERAL_EASTERN_ARABIC, buf,
                                     3);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_i18n_format_numeral(123, CUPERTINO_NUMERAL_PERSIAN, buf, 3);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_i18n_format_numeral(123, CUPERTINO_NUMERAL_DEVANAGARI, buf, 4);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  PASS();
}

TEST test_cupertino_calendar_conversion(void) {
  struct cupertino_calendar_date date;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_calendar_convert_from_gregorian(
      2026, 0, 15, CUPERTINO_CALENDAR_BUDDHIST, &date);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_calendar_convert_from_gregorian(
      2026, 13, 15, CUPERTINO_CALENDAR_BUDDHIST, &date);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_calendar_convert_from_gregorian(
      2026, 5, 0, CUPERTINO_CALENDAR_BUDDHIST, &date);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_calendar_convert_from_gregorian(
      2026, 5, 32, CUPERTINO_CALENDAR_BUDDHIST, &date);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_calendar_convert_from_gregorian(
      2026, 5, 15, CUPERTINO_CALENDAR_BUDDHIST, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Buddhist calendar: 2026 + 543 = 2569 BE */
  rc = cupertino_calendar_convert_from_gregorian(
      2026, 10, 1, CUPERTINO_CALENDAR_BUDDHIST, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2569, date.year);
  ASSERT_EQ(10, date.month);
  ASSERT_EQ(1, date.day);
  ASSERT_STR_EQ("BE", date.era_name);

  /* Japanese Reiwa: year > 2019 */
  rc = cupertino_calendar_convert_from_gregorian(
      2026, 10, 1, CUPERTINO_CALENDAR_JAPANESE, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(8, date.year);
  ASSERT_STR_EQ("Reiwa", date.era_name);

  /* Japanese Reiwa: 2019-06-01 (year == 2019, month > 5) */
  rc = cupertino_calendar_convert_from_gregorian(
      2019, 6, 1, CUPERTINO_CALENDAR_JAPANESE, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, date.year);
  ASSERT_STR_EQ("Reiwa", date.era_name);

  /* Japanese Reiwa: 2019-05-01 (year == 2019, month == 5, day >= 1) */
  rc = cupertino_calendar_convert_from_gregorian(
      2019, 5, 1, CUPERTINO_CALENDAR_JAPANESE, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, date.year);
  ASSERT_STR_EQ("Reiwa", date.era_name);

  /* Japanese Heisei: 2019-04-30 (year == 2019, month < 5) */
  rc = cupertino_calendar_convert_from_gregorian(
      2019, 4, 30, CUPERTINO_CALENDAR_JAPANESE, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(31, date.year);
  ASSERT_STR_EQ("Heisei", date.era_name);

  /* Japanese Heisei: year > 1989 (2000-05-05) */
  rc = cupertino_calendar_convert_from_gregorian(
      2000, 5, 5, CUPERTINO_CALENDAR_JAPANESE, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(12, date.year);
  ASSERT_STR_EQ("Heisei", date.era_name);

  /* Japanese Heisei: 1989-02-01 (year == 1989, month > 1) */
  rc = cupertino_calendar_convert_from_gregorian(
      1989, 2, 1, CUPERTINO_CALENDAR_JAPANESE, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, date.year);
  ASSERT_STR_EQ("Heisei", date.era_name);

  /* Japanese Heisei: 1989-01-08 (year == 1989, month == 1, day >= 8) */
  rc = cupertino_calendar_convert_from_gregorian(
      1989, 1, 8, CUPERTINO_CALENDAR_JAPANESE, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, date.year);
  ASSERT_STR_EQ("Heisei", date.era_name);

  /* Japanese Showa: 1989-01-07 (year == 1989, month == 1, day < 8) */
  rc = cupertino_calendar_convert_from_gregorian(
      1989, 1, 7, CUPERTINO_CALENDAR_JAPANESE, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(64, date.year);
  ASSERT_STR_EQ("Showa", date.era_name);

  /* Japanese Showa: 1980 = Showa 55 */
  rc = cupertino_calendar_convert_from_gregorian(
      1980, 1, 1, CUPERTINO_CALENDAR_JAPANESE, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(55, date.year);
  ASSERT_STR_EQ("Showa", date.era_name);

  /* Japanese Pre-Showa: 1910 */
  rc = cupertino_calendar_convert_from_gregorian(
      1910, 1, 1, CUPERTINO_CALENDAR_JAPANESE, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1910, date.year);
  ASSERT_STR_EQ("AD", date.era_name);

  /* Islamic Hijri */
  rc = cupertino_calendar_convert_from_gregorian(
      2026, 10, 1, CUPERTINO_CALENDAR_ISLAMIC_HIJRI, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(date.year >= 1447);
  ASSERT_STR_EQ("AH", date.era_name);

  /* Hebrew calendar */
  rc = cupertino_calendar_convert_from_gregorian(
      2026, 10, 1, CUPERTINO_CALENDAR_HEBREW, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5786, date.year);
  ASSERT_STR_EQ("AM", date.era_name);

  /* Gregorian baseline */
  rc = cupertino_calendar_convert_from_gregorian(
      2026, 10, 1, CUPERTINO_CALENDAR_GREGORIAN, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2026, date.year);
  ASSERT_STR_EQ("AD", date.era_name);

  /* Default calendar */
  rc = cupertino_calendar_convert_from_gregorian(
      2026, 10, 1, (enum cupertino_calendar_type)999, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2026, date.year);
  ASSERT_STR_EQ("AD", date.era_name);

  PASS();
}

TEST test_cupertino_i18n_first_day_and_plurals(void) {
  int first_day = -1;
  enum cupertino_plural_category cat;
  ui_error_t rc;

  /* First day of week invalid args */
  rc = cupertino_i18n_get_first_day_of_week(NULL, &first_day);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_i18n_get_first_day_of_week("en_US", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Saturday start: Saudi Arabia, Iran, Egypt variants */
  rc = cupertino_i18n_get_first_day_of_week("ar_SA", &first_day);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(6, first_day);
  rc = cupertino_i18n_get_first_day_of_week("ar-SA", &first_day);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(6, first_day);
  rc = cupertino_i18n_get_first_day_of_week("fa_IR", &first_day);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(6, first_day);
  rc = cupertino_i18n_get_first_day_of_week("fa-IR", &first_day);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(6, first_day);
  rc = cupertino_i18n_get_first_day_of_week("ar_EG", &first_day);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(6, first_day);
  rc = cupertino_i18n_get_first_day_of_week("ar-EG", &first_day);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(6, first_day);

  /* Sunday start: US, Canada, Japan, Australia, English */
  rc = cupertino_i18n_get_first_day_of_week("en_US", &first_day);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, first_day);
  rc = cupertino_i18n_get_first_day_of_week("fr_CA", &first_day);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, first_day);
  rc = cupertino_i18n_get_first_day_of_week("ja_JP", &first_day);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, first_day);
  rc = cupertino_i18n_get_first_day_of_week("en_AU", &first_day);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, first_day);
  rc = cupertino_i18n_get_first_day_of_week("en_GB", &first_day);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, first_day);

  /* Monday start: France, Germany, default */
  rc = cupertino_i18n_get_first_day_of_week("fr_FR", &first_day);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, first_day);

  /* Plural categories invalid args */
  rc = cupertino_i18n_get_plural_category(1, NULL, &cat);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_i18n_get_plural_category(1, "en", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* English: 1 -> ONE, others -> OTHER, negative numbers */
  rc = cupertino_i18n_get_plural_category(1, "en_US", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_ONE, cat);
  rc = cupertino_i18n_get_plural_category(-1, "en_US", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_ONE, cat);
  rc = cupertino_i18n_get_plural_category(0, "en_US", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_OTHER, cat);
  rc = cupertino_i18n_get_plural_category(5, "en_US", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_OTHER, cat);

  /* Arabic CLDR plural rules */
  rc = cupertino_i18n_get_plural_category(0, "ar_EG", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_ZERO, cat);
  rc = cupertino_i18n_get_plural_category(1, "ar_EG", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_ONE, cat);
  rc = cupertino_i18n_get_plural_category(2, "ar_EG", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_TWO, cat);
  rc = cupertino_i18n_get_plural_category(5, "ar_EG", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_FEW, cat);
  rc = cupertino_i18n_get_plural_category(25, "ar_EG", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_MANY, cat);
  rc = cupertino_i18n_get_plural_category(100, "ar_EG", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_OTHER, cat);

  /* Slavic (Russian & Ukrainian) CLDR plural rules */
  rc = cupertino_i18n_get_plural_category(1, "ru_RU", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_ONE, cat);
  rc = cupertino_i18n_get_plural_category(11, "ru_RU", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_MANY, cat);
  rc = cupertino_i18n_get_plural_category(2, "ru_RU", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_FEW, cat);
  rc = cupertino_i18n_get_plural_category(22, "ru_RU", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_FEW, cat);
  rc = cupertino_i18n_get_plural_category(12, "ru_RU", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_MANY, cat);
  rc = cupertino_i18n_get_plural_category(5, "ru_RU", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_MANY, cat);
  rc = cupertino_i18n_get_plural_category(10, "ru_RU", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_MANY, cat);

  /* Ukrainian branch */
  rc = cupertino_i18n_get_plural_category(1, "uk_UA", &cat);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PLURAL_ONE, cat);

  PASS();
}

TEST test_cupertino_kinsoku_shori(void) {
  int prohibited = 0;
  ui_error_t rc;

  /* Invalid args */
  rc = cupertino_i18n_kinsoku_prohibited_start(NULL, &prohibited);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_i18n_kinsoku_prohibited_start(")", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_i18n_kinsoku_prohibited_end(NULL, &prohibited);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_i18n_kinsoku_prohibited_end("(", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Prohibited start characters */
  {
    static const char *const start_chars[] = {")",
                                              "]",
                                              "}",
                                              "\xE3\x80\x82",
                                              "\xE3\x80\x81",
                                              "\xE3\x80\x8D",
                                              "\xE3\x80\x8F",
                                              "\xEF\xBC\x81",
                                              "\xEF\xBC\x9F",
                                              ",",
                                              ".",
                                              "!",
                                              "?"};
    size_t i;
    for (i = 0; i < sizeof(start_chars) / sizeof(start_chars[0]); ++i) {
      rc = cupertino_i18n_kinsoku_prohibited_start(start_chars[i], &prohibited);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      ASSERT_EQ(1, prohibited);
    }
  }

  rc = cupertino_i18n_kinsoku_prohibited_start("A", &prohibited);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, prohibited);

  /* Prohibited end characters */
  {
    static const char *const end_chars[] = {
        "(", "[", "{", "\xE3\x80\x8C", "\xE3\x80\x8E", "\xEF\xBC\x88"};
    size_t i;
    for (i = 0; i < sizeof(end_chars) / sizeof(end_chars[0]); ++i) {
      rc = cupertino_i18n_kinsoku_prohibited_end(end_chars[i], &prohibited);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      ASSERT_EQ(1, prohibited);
    }
  }

  rc = cupertino_i18n_kinsoku_prohibited_end("Z", &prohibited);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, prohibited);

  PASS();
}

SUITE(cupertino_bidi_suite) {
  RUN_TEST(test_cupertino_bidi_direction_and_logical);
  RUN_TEST(test_cupertino_bidi_symbol_mirroring);
  RUN_TEST(test_cupertino_i18n_numerals);
  RUN_TEST(test_cupertino_calendar_conversion);
  RUN_TEST(test_cupertino_i18n_first_day_and_plurals);
  RUN_TEST(test_cupertino_kinsoku_shori);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_bidi_suite);
  GREATEST_MAIN_END();
}
