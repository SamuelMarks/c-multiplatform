/* clang-format off */
#include "../include/ui_cssom_api.h"
#include "../include/ui_css_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_malloc_called;

#define TEST_ASSERT(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "Assertion failed: %s at %s:%d\n", #cond, __FILE__,      \
              __LINE__);                                                       \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

int main(void) {
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule = NULL;
  const char *val = NULL;
  int is_imp = 0;
  enum ui_error rc;

  rc = ui_css_stylesheet_create(&sheet);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  /* Error paths for insert */
  rc = ui_cssom_insert_rule(NULL, "div { color: red; }", 0);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_cssom_insert_rule(sheet, NULL, 0);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  /* Parse failure (empty css_text fails to parse any rules typically, or
   * invalid syntax) */
  rc = ui_cssom_insert_rule(
      sheet, "", 0); /* This returns INVALID_ARGUMENT since no rule parsed */
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_cssom_insert_rule(sheet, "div {",
                            0); /* parse incomplete but accepted */
  TEST_ASSERT(rc == UI_ERROR_NONE);

  g_malloc_fail_countdown = 0;
  rc = ui_cssom_insert_rule(sheet, "div { color: red; }", 0);
  g_malloc_fail_countdown = -1;
  TEST_ASSERT(rc == UI_ERROR_OUT_OF_MEMORY);

  rc = ui_cssom_insert_rule(sheet, "div { color: red; }", 100);
  TEST_ASSERT(rc == UI_ERROR_OUT_OF_BOUNDS);

  /* Insert Rules */
  rc = ui_cssom_insert_rule(sheet, "div { color: red; }", 0);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_cssom_insert_rule(sheet, "span { display: block; }", 1);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc =
      ui_cssom_insert_rule(sheet, "p { margin: 0; }", 1); /* insert in middle */
  TEST_ASSERT(rc == UI_ERROR_NONE);

  rule = sheet->rules; /* The div rule */

  /* Set Property errors */
  rc = ui_cssom_set_property(NULL, "color", "blue", 1);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_cssom_set_property(rule, NULL, "blue", 1);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_cssom_set_property(rule, "color", NULL, 1);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);

  /* OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_cssom_set_property(rule, "color", "blue", 1);
  g_malloc_fail_countdown = -1;
  TEST_ASSERT(rc == UI_ERROR_OUT_OF_MEMORY);

  /* Set Property (Overwrite) */
  rc = ui_cssom_set_property(rule, "color", "blue", 1);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  /* New property */
  rc = ui_cssom_set_property(rule, "margin", "10px", 0);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_cssom_set_property(rule, "padding", "5px", 0);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_cssom_set_property(rule, "border", "1px", 0);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  /* Get Property Value errors */
  rc = ui_cssom_get_property_value(NULL, "color", &val);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_cssom_get_property_value(rule, NULL, &val);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_cssom_get_property_value(rule, "color", NULL);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_cssom_get_property_value(rule, "nonexistent", &val);
  TEST_ASSERT(rc == UI_ERROR_NOT_FOUND);
  rc = ui_cssom_get_property_value(rule, "color", &val);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(strcmp(val, "blue") == 0);

  /* Get Property Priority errors */
  rc = ui_cssom_get_property_priority(NULL, "color", &is_imp);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_cssom_get_property_priority(rule, NULL, &is_imp);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_cssom_get_property_priority(rule, "color", NULL);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_cssom_get_property_priority(rule, "nonexistent", &is_imp);
  TEST_ASSERT(rc == UI_ERROR_NOT_FOUND);
  rc = ui_cssom_get_property_priority(rule, "color", &is_imp);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  TEST_ASSERT(is_imp == 1);

  /* Remove Property errors */
  rc = ui_cssom_remove_property(NULL, "color");
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_cssom_remove_property(rule, NULL);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_cssom_remove_property(rule, "nonexistent");
  TEST_ASSERT(rc == UI_ERROR_NOT_FOUND);

  /* Remove Property from start, middle, end */
  rc = ui_cssom_remove_property(rule, "color"); /* start */
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_cssom_remove_property(rule, "padding"); /* middle */
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_cssom_remove_property(rule, "border"); /* end */
  TEST_ASSERT(rc == UI_ERROR_NONE);

  /* Delete Rule errors */
  rc = ui_cssom_delete_rule(NULL, 0);
  TEST_ASSERT(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_cssom_delete_rule(sheet, 100);
  TEST_ASSERT(rc == UI_ERROR_OUT_OF_BOUNDS);

  /* Delete Rule (middle) */
  rc = ui_cssom_delete_rule(sheet, 1);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  /* Delete Rule (start) */
  rc = ui_cssom_delete_rule(sheet, 0);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  ui_css_stylesheet_destroy(sheet);

  printf("test_ui_cssom_api passed.\n");
  return 0;
}
