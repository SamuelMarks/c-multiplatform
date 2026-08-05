/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "ui_css_content.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_parse_content_none_normal(void) {
  struct ui_css_content c;

  if (ui_css_parse_content(NULL, &c) != UI_ERROR_INVALID_ARGUMENT)
    return __LINE__;
  if (ui_css_parse_content("none", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return __LINE__;

  if (ui_css_parse_content("none", &c) != UI_ERROR_NONE || !c.is_none ||
      c.is_normal || c.items)
    return __LINE__;
  if (ui_css_parse_content("normal", &c) != UI_ERROR_NONE || c.is_none ||
      !c.is_normal || c.items)
    return __LINE__;

  if (ui_css_parse_content("none_", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("none1", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("none-", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;

  (void)ui_css_content_destroy(NULL);
  if (0)
    return __LINE__;

  return 0;
}

static int test_parse_content_strings(void) {
  struct ui_css_content c;

  if (ui_css_parse_content("\"hello world\"", &c) != UI_ERROR_NONE ||
      c.is_none || c.is_normal || !c.items)
    return __LINE__;
  if (c.items->type != UI_CSS_CONTENT_ITEM_STRING ||
      strcmp(c.items->value.string_val, "hello world") != 0)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  if (ui_css_parse_content("'single quote'", &c) != UI_ERROR_NONE)
    return __LINE__;
  if (c.items->type != UI_CSS_CONTENT_ITEM_STRING ||
      strcmp(c.items->value.string_val, "single quote") != 0)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  /* Escapes */
  if (ui_css_parse_content("\"escape \\\" \\\\ \"", &c) != UI_ERROR_NONE)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  /* Unclosed quote with escape at end */
  if (ui_css_parse_content("\"foo\\", &c) != UI_ERROR_PARSE_FAILED)
    return 1;
  /* Unclosed quote */
  if (ui_css_parse_content("\"unclosed", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;

  return 0;
}

static int test_parse_content_functions(void) {
  struct ui_css_content c;

  if (ui_css_parse_content("url(\"icon.png\")", &c) != UI_ERROR_NONE ||
      !c.items)
    return __LINE__;
  if (c.items->type != UI_CSS_CONTENT_ITEM_IMAGE ||
      c.items->value.image.type != UI_CSS_IMAGE_URL ||
      strcmp(c.items->value.image.data.url, "icon.png") != 0)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  /* url without parens */
  if (ui_css_parse_content("url \"icon.png\")", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  /* Missing close paren for url handled? */
  if (ui_css_parse_content("url(\"icon\"", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;

  /* Very long url */
  {
    char long_url[600];
    char css_str[620];
    int i;
    for (i = 0; i < 550; i++) {
      long_url[i] = 'a';
    }
    long_url[550] = '\0';
    sprintf(css_str, "url(%s)", long_url);
    if (ui_css_parse_content(css_str, &c) != UI_ERROR_PARSE_FAILED)
      return __LINE__;
    (void)ui_css_content_destroy(&c);
  }

  if (ui_css_parse_content("attr(data-title)", &c) != UI_ERROR_NONE || !c.items)
    return __LINE__;
  if (c.items->type != UI_CSS_CONTENT_ITEM_ATTR ||
      strcmp(c.items->value.attr_name, "data-title") != 0)
    return __LINE__;
  (void)ui_css_content_destroy(&c);
  /* attr syntax error */
  if (ui_css_parse_content("attr(data-title", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("attr(\")", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;

  if (ui_css_parse_content("counter(chapter)", &c) != UI_ERROR_NONE || !c.items)
    return __LINE__;
  if (c.items->type != UI_CSS_CONTENT_ITEM_COUNTER ||
      strcmp(c.items->value.counter.name, "chapter") != 0 ||
      c.items->value.counter.style[0] != '\0')
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  if (ui_css_parse_content("counter(section, upper-roman)", &c) !=
          UI_ERROR_NONE ||
      !c.items)
    return __LINE__;
  if (c.items->type != UI_CSS_CONTENT_ITEM_COUNTER ||
      strcmp(c.items->value.counter.name, "section") != 0 ||
      strcmp(c.items->value.counter.style, "upper-roman") != 0)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  if (ui_css_parse_content("counter(sec upper)", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("counter(sec, )", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("counter(sec", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("counter(123)", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__; /* invalid ident */

  if (ui_css_parse_content("counters(page, \".\")", &c) != UI_ERROR_NONE ||
      !c.items)
    return __LINE__;
  if (c.items->type != UI_CSS_CONTENT_ITEM_COUNTERS ||
      strcmp(c.items->value.counters.name, "page") != 0 ||
      strcmp(c.items->value.counters.separator, ".") != 0)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  if (ui_css_parse_content("counters(page, \".\", upper-roman)", &c) !=
          UI_ERROR_NONE ||
      !c.items)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  if (ui_css_parse_content("counters(page)", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("counters(page, unquoted)", &c) !=
      UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("counters(page, \".\"", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("counters(page, \".\", )", &c) !=
      UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("counters(123, \".\")", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__; /* invalid ident */

  if (ui_css_parse_content("attr(_name)", &c) != UI_ERROR_NONE ||
      strcmp(c.items->value.attr_name, "_name") != 0)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  if (ui_css_parse_content("attr(-name)", &c) != UI_ERROR_NONE ||
      strcmp(c.items->value.attr_name, "-name") != 0)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  return 0;
}

static int test_parse_content_quotes(void) {
  struct ui_css_content c;

  if (ui_css_parse_content(
          "open-quote close-quote no-open-quote no-close-quote", &c) !=
          UI_ERROR_NONE ||
      !c.items)
    return __LINE__;
  if (c.items->type != UI_CSS_CONTENT_ITEM_OPEN_QUOTE ||
      c.items->next->type != UI_CSS_CONTENT_ITEM_CLOSE_QUOTE ||
      c.items->next->next->type != UI_CSS_CONTENT_ITEM_NO_OPEN_QUOTE ||
      c.items->next->next->next->type != UI_CSS_CONTENT_ITEM_NO_CLOSE_QUOTE)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  /* Unknown ident */
  if (ui_css_parse_content("invalid-quote", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;

  /* Keyword word boundary check */
  if (ui_css_parse_content("open-quoted", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("open-quote_", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("open-quote-", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("none-none", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;
  if (ui_css_parse_content("normal_abc", &c) != UI_ERROR_PARSE_FAILED)
    return __LINE__;

  return 0;
}

static int test_parse_content_multiple(void) {
  struct ui_css_content c;

  if (ui_css_parse_content("\"Section \" counter(chapter) \": \"", &c) !=
          UI_ERROR_NONE ||
      !c.items)
    return __LINE__;

  if (c.items->type != UI_CSS_CONTENT_ITEM_STRING ||
      strcmp(c.items->value.string_val, "Section ") != 0)
    return __LINE__;
  if (!c.items->next || c.items->next->type != UI_CSS_CONTENT_ITEM_COUNTER ||
      strcmp(c.items->next->value.counter.name, "chapter") != 0)
    return __LINE__;
  if (!c.items->next->next ||
      c.items->next->next->type != UI_CSS_CONTENT_ITEM_STRING ||
      strcmp(c.items->next->next->value.string_val, ": ") != 0)
    return __LINE__;

  (void)ui_css_content_destroy(&c);
  return 0;
}

static int test_parse_content_oom(void) {
  struct ui_css_content c;
  int i;
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    if (ui_css_parse_content("\"test\" url(\"img.png\") attr(data-test)", &c) ==
        UI_ERROR_NONE) {
      (void)ui_css_content_destroy(&c);
    }
  }
  g_malloc_fail_countdown = -1;
  return 0;
}

static int test_parse_content_long(void) {
  struct ui_css_content c;
  char long_str[600];
  char css_str[620];
  int i;
  for (i = 0; i < 550; i++) {
    long_str[i] = 'a';
  }
  long_str[550] = '\0';

  /* string */
  sprintf(css_str, "\"%s\"", long_str);
  if (ui_css_parse_content(css_str, &c) != UI_ERROR_NONE)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  /* ident in attr */
  sprintf(css_str, "attr(%s)", long_str);
  if (ui_css_parse_content(css_str, &c) != UI_ERROR_NONE)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  /* escape at end of max len */
  sprintf(css_str, "\"%s\\\"\"", long_str);
  if (ui_css_parse_content(css_str, &c) != UI_ERROR_NONE)
    return __LINE__;
  (void)ui_css_content_destroy(&c);

  return 0;
}

int main(void) {
  int failures = 0;

  failures += test_parse_content_none_normal();
  failures += test_parse_content_strings();
  failures += test_parse_content_functions();
  failures += test_parse_content_quotes();
  failures += test_parse_content_multiple();
  failures += test_parse_content_oom();
  failures += test_parse_content_long();

  if (failures == 0) {
    printf("test_ui_css_content passed\n");
  } else {
    printf("test_ui_css_content failed with %d errors\n", failures);
  }

  return failures;
}
