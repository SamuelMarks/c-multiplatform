/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "ui_css_lists.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_parse_list_style_type(void) {
  struct ui_css_list_style_type_ext type;

  if (ui_css_parse_list_style_type(NULL, &type) != UI_ERROR_INVALID_ARGUMENT) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }
  if (ui_css_parse_list_style_type("disc", NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }

  if (ui_css_parse_list_style_type("disc", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_DISC)
    return __LINE__;
  if (ui_css_parse_list_style_type("circle", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_CIRCLE)
    return __LINE__;
  if (ui_css_parse_list_style_type("square", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_SQUARE)
    return __LINE__;
  if (ui_css_parse_list_style_type("decimal", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_DECIMAL)
    return __LINE__;
  if (ui_css_parse_list_style_type("decimal-leading-zero", &type) !=
          UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO)
    return __LINE__;
  if (ui_css_parse_list_style_type("lower-roman", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_LOWER_ROMAN)
    return __LINE__;
  if (ui_css_parse_list_style_type("upper-roman", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_UPPER_ROMAN)
    return __LINE__;
  if (ui_css_parse_list_style_type("lower-greek", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_LOWER_GREEK)
    return __LINE__;
  if (ui_css_parse_list_style_type("lower-alpha", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_LOWER_ALPHA)
    return __LINE__;
  if (ui_css_parse_list_style_type("lower-latin", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_LOWER_LATIN)
    return __LINE__;
  if (ui_css_parse_list_style_type("upper-alpha", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_UPPER_ALPHA)
    return __LINE__;
  if (ui_css_parse_list_style_type("upper-latin", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_UPPER_LATIN)
    return __LINE__;
  if (ui_css_parse_list_style_type("armenian", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_ARMENIAN)
    return __LINE__;
  if (ui_css_parse_list_style_type("georgian", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_GEORGIAN)
    return __LINE__;
  if (ui_css_parse_list_style_type("hebrew", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_HEBREW)
    return __LINE__;
  if (ui_css_parse_list_style_type("none", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_NONE)
    return __LINE__;
  if (ui_css_parse_list_style_type("\"->\"", &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_STRING ||
      strcmp(type.string_value, "->") != 0)
    return __LINE__;

  char long_quote_str[600];
  long_quote_str[0] = '"';
  memset(long_quote_str + 1, 'a', 597);
  long_quote_str[598] = '"';
  long_quote_str[599] = '\0';
  if (ui_css_parse_list_style_type(long_quote_str, &type) != UI_ERROR_NONE ||
      type.type != UI_CSS_LIST_STYLE_TYPE_STRING)
    return __LINE__;

  if (ui_css_parse_list_style_type("\"unclosed", &type) !=
      UI_ERROR_PARSE_FAILED)
    return __LINE__;

  /* Strings */
  if (ui_css_parse_list_style_type("\"unclosed", &type) !=
      UI_ERROR_PARSE_FAILED) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }

  /* empty string */
  if (ui_css_parse_list_style_type("", &type) != UI_ERROR_PARSE_FAILED) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }

  /* huge string */
  {
    char long_str[600];
    char css_str[1000];
    int i;
    for (i = 0; i < 550; i++)
      long_str[i] = 'a';
    long_str[550] = '\0';
    sprintf(css_str, "'%s'", long_str);
    if (ui_css_parse_list_style_type(css_str, &type) != UI_ERROR_NONE) {
      printf("fail line %d\n", __LINE__);
      return 1;
    }
  }

  return 0;
}

static int test_parse_list_style_position(void) {
  enum ui_css_list_style_position pos;

  if (ui_css_parse_list_style_position(NULL, &pos) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }
  if (ui_css_parse_list_style_position("inside", NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }

  if (ui_css_parse_list_style_position("inside", &pos) != UI_ERROR_NONE ||
      pos != UI_CSS_LIST_STYLE_POSITION_INSIDE)
    return __LINE__;
  if (ui_css_parse_list_style_position("outside", &pos) != UI_ERROR_NONE ||
      pos != UI_CSS_LIST_STYLE_POSITION_OUTSIDE)
    return __LINE__;
  if (ui_css_parse_list_style_position("invalid", &pos) == UI_ERROR_NONE)
    return __LINE__;

  return 0;
}

static int test_parse_list_style_image(void) {
  struct ui_css_image img;
  int is_none;

  if (ui_css_parse_list_style_image(NULL, &img, &is_none) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }
  if (ui_css_parse_list_style_image("none", NULL, &is_none) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }
  if (ui_css_parse_list_style_image("none", &img, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }

  if (ui_css_parse_list_style_image("none", &img, &is_none) != UI_ERROR_NONE ||
      !is_none)
    return __LINE__;

  if (ui_css_parse_list_style_image("url(\"test.png\")", &img, &is_none) !=
          UI_ERROR_NONE ||
      is_none || img.type != UI_CSS_IMAGE_URL ||
      strcmp(img.data.url, "test.png") != 0)
    return __LINE__;

  return 0;
}

static int test_parse_list_style(void) {
  struct ui_css_list_style style;

  if (ui_css_parse_list_style(NULL, &style) != UI_ERROR_INVALID_ARGUMENT) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }
  if (ui_css_parse_list_style("none", NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }

  if (ui_css_parse_list_style("none", &style) != UI_ERROR_NONE ||
      style.type.type != UI_CSS_LIST_STYLE_TYPE_NONE ||
      style.position != UI_CSS_LIST_STYLE_POSITION_OUTSIDE || style.has_image)
    return __LINE__;

  if (ui_css_parse_list_style("inside square url(\"icon.png\")", &style) !=
      UI_ERROR_NONE)
    return __LINE__;
  if (style.position != UI_CSS_LIST_STYLE_POSITION_INSIDE ||
      style.type.type != UI_CSS_LIST_STYLE_TYPE_SQUARE || !style.has_image ||
      strcmp(style.image.data.url, "icon.png") != 0)
    return __LINE__;

  if (ui_css_parse_list_style("url(\"icon.png\") circle outside", &style) !=
      UI_ERROR_NONE)
    return __LINE__;
  if (style.position != UI_CSS_LIST_STYLE_POSITION_OUTSIDE ||
      style.type.type != UI_CSS_LIST_STYLE_TYPE_CIRCLE || !style.has_image)
    return __LINE__;

  if (ui_css_parse_list_style("circle", &style) != UI_ERROR_NONE)
    return __LINE__;
  if (style.type.type != UI_CSS_LIST_STYLE_TYPE_CIRCLE ||
      style.position != UI_CSS_LIST_STYLE_POSITION_OUTSIDE || style.has_image)
    return __LINE__;

  /* Duplicates/errors */
  if (ui_css_parse_list_style("circle square", &style) !=
      UI_ERROR_PARSE_FAILED) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }
  if (ui_css_parse_list_style("inside outside", &style) !=
      UI_ERROR_PARSE_FAILED) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }
  if (ui_css_parse_list_style("url(\"icon.png\") url(\"icon.png\")", &style) !=
      UI_ERROR_PARSE_FAILED) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }

  {
    ui_error_t e = ui_css_parse_list_style("invalid-token", &style);
    if (e != UI_ERROR_PARSE_FAILED) {
      printf("fail line %d: returned %d\n", __LINE__, e);
      return 1;
    }
  }

  return 0;
}

static int test_parse_counter_action(void) {
  struct ui_css_counter_action *head = NULL;

  if (ui_css_parse_counter_action(NULL, &head) != UI_ERROR_INVALID_ARGUMENT) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }
  if (ui_css_parse_counter_action("none", NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }

  if (ui_css_parse_counter_action("none", &head) != UI_ERROR_NONE ||
      head != NULL)
    return __LINE__;

  if (ui_css_parse_counter_action("chapter section 2 page", &head) !=
          UI_ERROR_NONE ||
      !head)
    return __LINE__;
  if (strcmp(head->name, "chapter") != 0 || head->value != 1 || !head->next)
    return __LINE__;
  if (strcmp(head->next->name, "section") != 0 || head->next->value != 2 ||
      !head->next->next)
    return __LINE__;
  if (strcmp(head->next->next->name, "page") != 0 ||
      head->next->next->value != 1 || head->next->next->next)
    return __LINE__;
  ui_css_counter_action_destroy(head);

  if (ui_css_parse_counter_action("junk 2x", &head) != UI_ERROR_NONE || !head)
    return __LINE__;
  if (strcmp(head->name, "junk") != 0 || head->value != 1 || !head->next)
    return __LINE__;
  if (strcmp(head->next->name, "2x") != 0 || head->next->value != 1 ||
      head->next->next)
    return __LINE__;
  ui_css_counter_action_destroy(head);

  char long_str[600];
  memset(long_str, 'a', 599);
  long_str[599] = '\0';
  if (ui_css_parse_counter_action(long_str, &head) != UI_ERROR_NONE || !head)
    return __LINE__;
  ui_css_counter_action_destroy(head);

  /* Test empty effectively */
  if (ui_css_parse_counter_action("   ", &head) != UI_ERROR_PARSE_FAILED) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }

  /* OOM */
  g_malloc_fail_countdown = 0;
  if (ui_css_parse_counter_action("chapter section 2 page", &head) !=
      UI_ERROR_OUT_OF_MEMORY) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 1;
  if (ui_css_parse_counter_action("chapter section 2 page", &head) !=
      UI_ERROR_OUT_OF_MEMORY) {
    printf("fail line %d\n", __LINE__);
    return 1;
  }
  g_malloc_fail_countdown = -1;

  return 0;
}

int main(void) {
  int failures = 0;

  failures += test_parse_list_style_type();
  failures += test_parse_list_style_position();
  failures += test_parse_list_style_image();
  failures += test_parse_list_style();
  failures += test_parse_counter_action();

  if (failures == 0) {
    printf("test_ui_css_lists passed\n");
  } else {
    printf("test_ui_css_lists failed with %d errors\n", failures);
  }

  return failures;
}

static int test_skip_whitespace_coverage(void) {
  /* Internal function so we need a front API to hit the p_str == NULL or *p_str
   * == NULL branch */
  /* ui_css_parse_list_style_type passes &str which is never NULL but *p_str
   * could be NULL if we pass NULL for str, but we have a guard at the top! */
  /* So it is impossible. We should just remove the guard in skip_whitespace */
  return 0;
}
