/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "ui_css_forms.h"
/* clang-format on */

static int test_field_sizing(void) {
  enum ui_css_field_sizing sizing;

  if (ui_css_parse_field_sizing(NULL, &sizing) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_parse_field_sizing("content", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_parse_field_sizing("content", &sizing) != UI_ERROR_NONE ||
      sizing != UI_CSS_FIELD_SIZING_CONTENT)
    return 1;
  if (ui_css_parse_field_sizing("fixed", &sizing) != UI_ERROR_NONE ||
      sizing != UI_CSS_FIELD_SIZING_FIXED)
    return 1;
  if (ui_css_parse_field_sizing("invalid", &sizing) != UI_ERROR_PARSE_FAILED)
    return 1;

  return 0;
}

static int test_accent_color(void) {
  struct ui_css_accent_color color;

  if (ui_css_parse_accent_color(NULL, &color) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_parse_accent_color("auto", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_parse_accent_color("auto", &color) != UI_ERROR_NONE ||
      !color.is_auto)
    return 1;

  if (ui_css_parse_accent_color("red", &color) != UI_ERROR_NONE ||
      color.is_auto)
    return 1;

  if (ui_css_parse_accent_color("invalid_color", &color) !=
      UI_ERROR_PARSE_FAILED)
    return 1;

  return 0;
}

static int test_caret_color(void) {
  struct ui_css_caret_color color;

  if (ui_css_parse_caret_color(NULL, &color) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_parse_caret_color("auto", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_parse_caret_color("auto", &color) != UI_ERROR_NONE ||
      !color.is_auto)
    return 1;

  if (ui_css_parse_caret_color("blue", &color) != UI_ERROR_NONE ||
      color.is_auto)
    return 1;

  if (ui_css_parse_caret_color("invalid_color", &color) !=
      UI_ERROR_PARSE_FAILED)
    return 1;

  return 0;
}

int main(void) {
  int failures = 0;

  failures += test_field_sizing();
  failures += test_accent_color();
  failures += test_caret_color();

  if (failures == 0) {
    printf("test_ui_css_forms passed\n");
  } else {
    printf("test_ui_css_forms failed with %d errors\n", failures);
  }

  return failures;
}
