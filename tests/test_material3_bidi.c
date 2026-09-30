/* clang-format off */
#include "greatest.h"
#include "material3/md3_bidi.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_md3_bidi_direction_and_logical_properties(void) {
  enum ui_bidi_direction dir = UI_BIDI_DIR_LTR;
  float l = 0.0f, r = 0.0f;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_bidi_set_direction((enum ui_bidi_direction)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bidi_set_direction((enum ui_bidi_direction) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bidi_get_direction(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_bidi_resolve_logical_margin(10.0f, 20.0f, UI_BIDI_DIR_LTR, NULL, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bidi_resolve_logical_margin(10.0f, 20.0f, UI_BIDI_DIR_LTR, &l, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      md3_bidi_resolve_logical_padding(10.0f, 20.0f, UI_BIDI_DIR_LTR, NULL, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      md3_bidi_resolve_logical_padding(10.0f, 20.0f, UI_BIDI_DIR_LTR, &l, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bidi_resolve_logical_inset(10.0f, 20.0f, UI_BIDI_DIR_LTR, NULL, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bidi_resolve_logical_inset(10.0f, 20.0f, UI_BIDI_DIR_LTR, &l, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      md3_bidi_resolve_logical_inset(10.0f, 20.0f, UI_BIDI_DIR_LTR, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set LTR */
  rc = md3_bidi_set_direction(UI_BIDI_DIR_LTR);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_bidi_get_direction(&dir);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_BIDI_DIR_LTR, dir);

  /* Set RTL */
  rc = md3_bidi_set_direction(UI_BIDI_DIR_RTL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_bidi_get_direction(&dir);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_BIDI_DIR_RTL, dir);

  /* LTR resolution: left = start, right = end */
  rc = md3_bidi_resolve_logical_margin(12.0f, 24.0f, UI_BIDI_DIR_LTR, &l, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(12.0f, l);
  ASSERT_EQ(24.0f, r);

  rc = md3_bidi_resolve_logical_padding(8.0f, 16.0f, UI_BIDI_DIR_LTR, &l, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(8.0f, l);
  ASSERT_EQ(16.0f, r);

  rc = md3_bidi_resolve_logical_inset(5.0f, 15.0f, UI_BIDI_DIR_LTR, &l, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5.0f, l);
  ASSERT_EQ(15.0f, r);

  /* RTL resolution: left = end, right = start */
  rc = md3_bidi_resolve_logical_margin(12.0f, 24.0f, UI_BIDI_DIR_RTL, &l, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(24.0f, l);
  ASSERT_EQ(12.0f, r);

  rc = md3_bidi_resolve_logical_padding(8.0f, 16.0f, UI_BIDI_DIR_RTL, &l, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(16.0f, l);
  ASSERT_EQ(8.0f, r);

  rc = md3_bidi_resolve_logical_inset(5.0f, 15.0f, UI_BIDI_DIR_RTL, &l, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(15.0f, l);
  ASSERT_EQ(5.0f, r);

  PASS();
}

TEST test_md3_bidi_icon_mirroring(void) {
  int mirror = 0;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_bidi_should_mirror_icon(NULL, UI_BIDI_DIR_RTL, &mirror);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bidi_should_mirror_icon("arrow_back", UI_BIDI_DIR_RTL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* LTR direction never mirrors */
  rc = md3_bidi_should_mirror_icon("arrow_back", UI_BIDI_DIR_LTR, &mirror);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, mirror);

  /* Directional icons in RTL ARE mirrored */
  {
    static const char *const mirrored_icons[] = {
        "arrow_back",      "arrow_forward", "arrow_left",
        "arrow_right",     "chevron_left",  "chevron_right",
        "navigate_before", "navigate_next", "menu_open",
        "drawer",          "redo",          "undo",
        "first_page",      "last_page"};
    size_t i;
    for (i = 0; i < sizeof(mirrored_icons) / sizeof(mirrored_icons[0]); ++i) {
      rc = md3_bidi_should_mirror_icon(mirrored_icons[i], UI_BIDI_DIR_RTL,
                                       &mirror);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      ASSERT_EQ(1, mirror);
    }
  }

  /* Media controls and clocks in RTL MUST NOT be mirrored */
  {
    static const char *const non_mirrored_icons[] = {
        "play",        "pause",       "stop",        "fast_forward",
        "fast_rewind", "replay",      "skip_next",   "skip_previous",
        "volume_up",   "volume_down", "volume_mute", "volume_off",
        "music_note",  "access_time", "schedule",    "timer",
        "watch"};
    size_t i;
    for (i = 0; i < sizeof(non_mirrored_icons) / sizeof(non_mirrored_icons[0]);
         ++i) {
      rc = md3_bidi_should_mirror_icon(non_mirrored_icons[i], UI_BIDI_DIR_RTL,
                                       &mirror);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      ASSERT_EQ(0, mirror);
    }
  }

  /* Unrecognized icon does not mirror */
  rc = md3_bidi_should_mirror_icon("home", UI_BIDI_DIR_RTL, &mirror);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, mirror);

  PASS();
}

TEST test_md3_bidi_font_cascades_and_metrics(void) {
  const char *family = NULL;
  float multiplier = 0.0f;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_bidi_get_fallback_font_family(NULL, &family);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bidi_get_fallback_font_family("Arab", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_bidi_get_script_line_height_multiplier(NULL, &multiplier);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_bidi_get_script_line_height_multiplier("Arab", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Font cascades */
  rc = md3_bidi_get_fallback_font_family("Arab", &family);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Noto Sans Arabic", family);

  rc = md3_bidi_get_fallback_font_family("Hebr", &family);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Noto Sans Hebrew", family);

  rc = md3_bidi_get_fallback_font_family("Deva", &family);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Noto Sans Devanagari", family);

  rc = md3_bidi_get_fallback_font_family("Thai", &family);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Noto Sans Thai", family);

  rc = md3_bidi_get_fallback_font_family("Latn", &family);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Noto Sans", family);

  /* Script line height multipliers */
  rc = md3_bidi_get_script_line_height_multiplier("Arab", &multiplier);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.25f, multiplier);

  rc = md3_bidi_get_script_line_height_multiplier("Thai", &multiplier);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.20f, multiplier);

  rc = md3_bidi_get_script_line_height_multiplier("Deva", &multiplier);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.15f, multiplier);

  rc = md3_bidi_get_script_line_height_multiplier("Latn", &multiplier);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.00f, multiplier);

  PASS();
}

TEST test_md3_i18n_formatting(void) {
  char buf[64];
  ui_error_t rc;

  /* Eastern Arabic numerals */
  rc = md3_i18n_format_eastern_arabic_numerals(12345, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(strlen(buf) > 0);

  rc = md3_i18n_format_eastern_arabic_numerals(-42, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('-', buf[0]);

  /* Invalid arguments & buffer limits */
  rc = md3_i18n_format_eastern_arabic_numerals(123, NULL, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_i18n_format_eastern_arabic_numerals(123, buf, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_i18n_format_eastern_arabic_numerals(123456, buf, 5);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  /* Out of bounds with negative sign */
  rc = md3_i18n_format_eastern_arabic_numerals(-50, buf, 4);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Localized decimal formatting */
  rc = md3_i18n_format_localized_decimal(12.34, 2, "de-DE", buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("12,34", buf);

  rc = md3_i18n_format_localized_decimal(12.34, 2, "fr-FR", buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("12,34", buf);

  rc = md3_i18n_format_localized_decimal(12.34, 2, "es-ES", buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("12,34", buf);

  rc = md3_i18n_format_localized_decimal(12.34, 2, "it-IT", buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("12,34", buf);

  rc = md3_i18n_format_localized_decimal(12.34, 2, "ru-RU", buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("12,34", buf);

  rc = md3_i18n_format_localized_decimal(12.34, 2, "en-US", buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("12.34", buf);

  /* Invalid arguments */
  rc = md3_i18n_format_localized_decimal(12.34, 2, NULL, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_i18n_format_localized_decimal(12.34, 2, "en", NULL, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_i18n_format_localized_decimal(12.34, 2, "en", buf, 7);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_i18n_format_localized_decimal(12.34, -1, "en", buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_md3_multi_calendar(void) {
  struct md3_calendar_date date;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_calendar_convert_from_gregorian(2026, 0, 15, MD3_CALENDAR_BUDDHIST,
                                           &date);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_calendar_convert_from_gregorian(2026, 13, 15, MD3_CALENDAR_BUDDHIST,
                                           &date);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_calendar_convert_from_gregorian(2026, 9, 0, MD3_CALENDAR_BUDDHIST,
                                           &date);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_calendar_convert_from_gregorian(2026, 9, 32, MD3_CALENDAR_BUDDHIST,
                                           &date);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_calendar_convert_from_gregorian(2026, 9, 15, MD3_CALENDAR_BUDDHIST,
                                           NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Buddhist calendar: 2026 CE -> 2569 BE */
  rc = md3_calendar_convert_from_gregorian(2026, 9, 29, MD3_CALENDAR_BUDDHIST,
                                           &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2569, date.year);
  ASSERT_EQ(9, date.month);
  ASSERT_EQ(29, date.day);
  ASSERT_EQ(MD3_CALENDAR_BUDDHIST, date.type);

  /* Islamic Hijri calendar */
  rc = md3_calendar_convert_from_gregorian(2026, 9, 29,
                                           MD3_CALENDAR_ISLAMIC_HIJRI, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(date.year > 1400);
  ASSERT(date.month >= 1 && date.month <= 12);
  ASSERT(date.day >= 1 && date.day <= 30);
  ASSERT_EQ(MD3_CALENDAR_ISLAMIC_HIJRI, date.type);

  /* Hebrew calendar */
  rc = md3_calendar_convert_from_gregorian(2026, 9, 29, MD3_CALENDAR_HEBREW,
                                           &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5786, date.year);
  ASSERT_EQ(MD3_CALENDAR_HEBREW, date.type);

  /* Gregorian calendar pass-through */
  rc = md3_calendar_convert_from_gregorian(2026, 9, 29, MD3_CALENDAR_GREGORIAN,
                                           &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2026, date.year);
  ASSERT_EQ(9, date.month);
  ASSERT_EQ(29, date.day);
  ASSERT_EQ(MD3_CALENDAR_GREGORIAN, date.type);

  /* Default calendar pass-through */
  rc = md3_calendar_convert_from_gregorian(2026, 9, 29,
                                           (enum md3_calendar_type)99, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2026, date.year);
  ASSERT_EQ(9, date.month);
  ASSERT_EQ(29, date.day);

  PASS();
}

TEST test_md3_plural_rules(void) {
  enum md3_plural_form form;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_i18n_get_plural_form(1, NULL, &form);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_i18n_get_plural_form(1, "en", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* English plural rules */
  rc = md3_i18n_get_plural_form(1, "en", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_ONE, form);

  rc = md3_i18n_get_plural_form(0, "en", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_OTHER, form);

  rc = md3_i18n_get_plural_form(5, "en", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_OTHER, form);

  /* Arabic plural rules */
  rc = md3_i18n_get_plural_form(0, "ar", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_ZERO, form);

  rc = md3_i18n_get_plural_form(1, "ar", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_ONE, form);

  rc = md3_i18n_get_plural_form(2, "ar", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_TWO, form);

  rc = md3_i18n_get_plural_form(4, "ar", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_FEW, form);

  rc = md3_i18n_get_plural_form(15, "ar", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_MANY, form);

  rc = md3_i18n_get_plural_form(100, "ar", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_OTHER, form);

  /* Slavic plural rules */
  rc = md3_i18n_get_plural_form(1, "ru", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_ONE, form);

  rc = md3_i18n_get_plural_form(11, "ru", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_MANY, form);

  rc = md3_i18n_get_plural_form(2, "uk", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_FEW, form);

  rc = md3_i18n_get_plural_form(12, "uk", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_MANY, form);

  rc = md3_i18n_get_plural_form(22, "ru", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_FEW, form);

  rc = md3_i18n_get_plural_form(5, "ru", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_MANY, form);

  /* Negative count handling */
  rc = md3_i18n_get_plural_form(-1, "en", &form);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_PLURAL_ONE, form);

  PASS();
}

SUITE(md3_bidi_suite) {
  RUN_TEST(test_md3_bidi_direction_and_logical_properties);
  RUN_TEST(test_md3_bidi_icon_mirroring);
  RUN_TEST(test_md3_bidi_font_cascades_and_metrics);
  RUN_TEST(test_md3_i18n_formatting);
  RUN_TEST(test_md3_multi_calendar);
  RUN_TEST(test_md3_plural_rules);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_bidi_suite);
  GREATEST_MAIN_END();
}
