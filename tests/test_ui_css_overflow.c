/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "ui_css_overflow.h"
/* clang-format on */

static int test_parse_overflow(void) {
  enum ui_css_overflow overflow;

  if (ui_css_parse_overflow(NULL, &overflow) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_parse_overflow("visible", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_parse_overflow(" visible", &overflow) != UI_ERROR_NONE ||
      overflow != UI_CSS_OVERFLOW_VISIBLE)
    return 1;
  if (ui_css_parse_overflow("hidden", &overflow) != UI_ERROR_NONE ||
      overflow != UI_CSS_OVERFLOW_HIDDEN)
    return 1;
  if (ui_css_parse_overflow("clip", &overflow) != UI_ERROR_NONE ||
      overflow != UI_CSS_OVERFLOW_CLIP)
    return 1;
  if (ui_css_parse_overflow("scroll", &overflow) != UI_ERROR_NONE ||
      overflow != UI_CSS_OVERFLOW_SCROLL)
    return 1;
  if (ui_css_parse_overflow("auto", &overflow) != UI_ERROR_NONE ||
      overflow != UI_CSS_OVERFLOW_AUTO)
    return 1;
  if (ui_css_parse_overflow("invalid", &overflow) == UI_ERROR_NONE)
    return 1;

  return 0;
}

static int test_parse_text_overflow(void) {
  struct ui_css_text_overflow to;

  if (ui_css_parse_text_overflow(NULL, &to) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_parse_text_overflow("clip", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_parse_text_overflow("clip", &to) != UI_ERROR_NONE ||
      to.type != UI_CSS_TEXT_OVERFLOW_CLIP)
    return 1;
  if (ui_css_parse_text_overflow("ellipsis", &to) != UI_ERROR_NONE ||
      to.type != UI_CSS_TEXT_OVERFLOW_ELLIPSIS)
    return 1;
  if (ui_css_parse_text_overflow("\"unclosed", &to) != UI_ERROR_PARSE_FAILED)
    return 1;
  if (ui_css_parse_text_overflow("\"...\"", &to) != UI_ERROR_NONE ||
      to.type != UI_CSS_TEXT_OVERFLOW_STRING || strcmp(to.string, "...") != 0)
    return 1;
  if (ui_css_parse_text_overflow("'read more'", &to) != UI_ERROR_NONE ||
      to.type != UI_CSS_TEXT_OVERFLOW_STRING ||
      strcmp(to.string, "read more") != 0)
    return 1;

  /* unmatched quote */
  if (ui_css_parse_text_overflow("\"abc", &to) == UI_ERROR_NONE)
    return 1;

  /* very long string */
  {
    char long_str[300] = "\"";
    memset(long_str + 1, 'x', 260);
    long_str[261] = '"';
    long_str[262] = '\0';
    if (ui_css_parse_text_overflow(long_str, &to) != UI_ERROR_NONE)
      return 1;
    if (to.type != UI_CSS_TEXT_OVERFLOW_STRING)
      return 1;
  }

  if (ui_css_parse_text_overflow("invalid", &to) == UI_ERROR_NONE)
    return 1;

  return 0;
}

static int test_parse_block_ellipsis(void) {
  struct ui_css_block_ellipsis be;

  if (ui_css_parse_block_ellipsis(NULL, &be) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_parse_block_ellipsis("none", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_parse_block_ellipsis("none", &be) != UI_ERROR_NONE ||
      be.type != UI_CSS_TEXT_OVERFLOW_CLIP)
    return 1;
  if (ui_css_parse_block_ellipsis("auto", &be) != UI_ERROR_NONE ||
      be.type != UI_CSS_TEXT_OVERFLOW_ELLIPSIS)
    return 1;
  if (ui_css_parse_block_ellipsis("\"unclosed", &be) != UI_ERROR_PARSE_FAILED)
    return 1;
  if (ui_css_parse_block_ellipsis("\"more\"", &be) != UI_ERROR_NONE ||
      be.type != UI_CSS_TEXT_OVERFLOW_STRING || strcmp(be.string, "more") != 0)
    return 1;

  /* unmatched quote */
  if (ui_css_parse_block_ellipsis("'more", &be) == UI_ERROR_NONE)
    return 1;

  /* very long string */
  {
    char long_str[300] = "'";
    memset(long_str + 1, 'y', 260);
    long_str[261] = '\'';
    long_str[262] = '\0';
    if (ui_css_parse_block_ellipsis(long_str, &be) != UI_ERROR_NONE)
      return 1;
    if (be.type != UI_CSS_TEXT_OVERFLOW_STRING)
      return 1;
  }

  if (ui_css_parse_block_ellipsis("invalid", &be) == UI_ERROR_NONE)
    return 1;

  return 0;
}

static int test_parse_line_clamp(void) {
  struct ui_css_line_clamp lc;

  if (ui_css_parse_line_clamp(NULL, &lc) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_parse_line_clamp("none", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_parse_line_clamp("none", &lc) != UI_ERROR_NONE || !lc.is_none)
    return 1;
  if (ui_css_parse_line_clamp("3", &lc) != UI_ERROR_NONE || lc.is_none ||
      lc.lines != 3)
    return 1;
  if (ui_css_parse_line_clamp("0", &lc) == UI_ERROR_NONE)
    return 1; /* must be > 0 */
  if (ui_css_parse_line_clamp("-1", &lc) == UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_line_clamp("invalid", &lc) == UI_ERROR_NONE)
    return 1;

  return 0;
}

static int test_parse_max_lines(void) {
  struct ui_css_max_lines ml;

  if (ui_css_parse_max_lines(NULL, &ml) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_parse_max_lines("none", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_parse_max_lines("none", &ml) != UI_ERROR_NONE || !ml.is_none)
    return 1;
  if (ui_css_parse_max_lines("5", &ml) != UI_ERROR_NONE || ml.is_none ||
      ml.lines != 5)
    return 1;
  if (ui_css_parse_max_lines("0", &ml) != UI_ERROR_NONE || ml.is_none ||
      ml.lines != 0)
    return 1; /* 0 is valid for max-lines */
  if (ui_css_parse_max_lines("-2", &ml) == UI_ERROR_NONE)
    return 1;
  if (ui_css_parse_max_lines("invalid", &ml) == UI_ERROR_NONE)
    return 1;

  return 0;
}

static int test_parse_overflow_clip_margin(void) {
  struct ui_css_value val;

  if (ui_css_parse_overflow_clip_margin(NULL, &val) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_css_parse_overflow_clip_margin("10px", NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_css_parse_overflow_clip_margin("10px", &val) != UI_ERROR_NONE ||
      val.unit != UI_CSS_UNIT_PX || val.value != 10.0f)
    return 1;

  return 0;
}

int main(void) {
  int failures = 0;

  failures += test_parse_overflow();
  failures += test_parse_text_overflow();
  failures += test_parse_block_ellipsis();
  failures += test_parse_line_clamp();
  failures += test_parse_max_lines();
  failures += test_parse_overflow_clip_margin();

  if (failures == 0) {
    printf("test_ui_css_overflow passed\n");
  } else {
    printf("test_ui_css_overflow failed with %d errors\n", failures);
  }

  return failures;
}
