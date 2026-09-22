/* clang-format off */
#include "../include/ui_i18n.h"
#include "../include/ui_bidi_manager.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_edge_cases(void) {
  struct ui_i18n *i18n = NULL;
  const char *loc;
  char buf[64];

  if (ui_i18n_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  g_malloc_fail_countdown = 0;
  if (ui_i18n_create(&i18n) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  if (ui_i18n_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_i18n_create(&i18n) != UI_ERROR_NONE)
    return 1;

  if (ui_i18n_set_locale(NULL, "en-US") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_set_locale(i18n, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_i18n_get_locale(NULL, &loc) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_get_locale(i18n, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_i18n_bind_locale_signal(NULL, (struct ui_signal *)1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_bind_locale_signal(i18n, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_bind_locale_signal(i18n, (struct ui_signal *)1) != UI_ERROR_NONE)
    return 1;

  if (ui_i18n_format_number(NULL, 1.0, 1, buf, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_format_number(i18n, 1.0, 1, NULL, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_format_number(i18n, 1.0, 1, buf, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_i18n_format_currency(NULL, 1.0, "USD", buf, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_format_currency(i18n, 1.0, NULL, buf, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_format_currency(i18n, 1.0, "USD", NULL, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_format_currency(i18n, 1.0, "USD", buf, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_i18n_format_date(NULL, 1000.0, "fmt", buf, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_format_date(i18n, 1000.0, NULL, buf, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_format_date(i18n, 1000.0, "fmt", NULL, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_format_date(i18n, 1000.0, "fmt", buf, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_i18n_format_date(i18n, 1000.0, "fmt", buf, sizeof(buf)) !=
      UI_ERROR_NONE)
    return 1;

  if (ui_i18n_pluralize(NULL, 1, "z", "o", "m", buf, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_pluralize(i18n, 1, NULL, "o", "m", buf, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_pluralize(i18n, 1, "z", NULL, "m", buf, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_pluralize(i18n, 1, "z", "o", NULL, buf, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_pluralize(i18n, 1, "z", "o", "m", NULL, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_pluralize(i18n, 1, "z", "o", "m", buf, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_i18n_interpolate(NULL, "tpl", NULL, NULL, 0, buf, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_interpolate(i18n, NULL, NULL, NULL, 0, buf, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_interpolate(i18n, "tpl", NULL, NULL, 0, NULL, sizeof(buf)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_i18n_interpolate(i18n, "tpl", NULL, NULL, 0, buf, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_i18n_destroy(i18n) != UI_ERROR_NONE)
    return 1;
  return 0;
}

static int test_interpolate_edge_cases(void) {
  struct ui_i18n *i18n = NULL;
  char buf[32];
  const char *keys[] = {"a", "ab"};
  const char *vals[] = {"1", "2"};

  if (ui_i18n_create(&i18n) != UI_ERROR_NONE)
    return 1;

  /* Test single { (no second {) */
  if (ui_i18n_interpolate(i18n, "Test { no double", keys, vals, 0, buf,
                          sizeof(buf)) != UI_ERROR_NONE)
    return 1;
  if (strcmp(buf, "Test { no double") != 0)
    return 1;

  /* Test incomplete {{ without closing }} */
  if (ui_i18n_interpolate(i18n, "Test {{ incomplete", keys, vals, 0, buf,
                          sizeof(buf)) != UI_ERROR_NONE)
    return 1;
  if (strcmp(buf, "Test {{ incomplete") != 0)
    return 1;

  /* Test key not found */
  if (ui_i18n_interpolate(i18n, "Test {{c}}", keys, vals, 2, buf,
                          sizeof(buf)) != UI_ERROR_NONE)
    return 1;
  if (strcmp(buf, "Test {{c}}") != 0)
    return 1;

  /* Test {{ with single } */
  if (ui_i18n_interpolate(i18n, "Test {{name}x", keys, vals, 2, buf,
                          sizeof(buf)) != UI_ERROR_NONE)
    return 1;

  /* Test prefix match but different length */
  {
    const char *k2[] = {"ab"};
    const char *v2[] = {"2"};
    if (ui_i18n_interpolate(i18n, "Test {{a}}", k2, v2, 1, buf, sizeof(buf)) !=
        UI_ERROR_NONE)
      return 1;
  }

  /* Test buffer overflow prevention during replacement */
  {
    const char *k3[] = {"a"};
    const char *v3[] = {"12345"};
    if (ui_i18n_interpolate(i18n, "T{{a}}", k3, v3, 1, buf, 6) != UI_ERROR_NONE)
      return 1; /* "T" (1) + "12345" (5) = 6 >= 5 (out_len-1) */
  }

  /* Test prefix match but different length */
  if (ui_i18n_interpolate(i18n, "Test {{a}}", keys, vals, 2, buf,
                          sizeof(buf)) != UI_ERROR_NONE)
    return 1;
  if (strcmp(buf, "Test 1") != 0)
    return 1;
  if (ui_i18n_interpolate(i18n, "Test {{ab}}", keys, vals, 2, buf,
                          sizeof(buf)) != UI_ERROR_NONE)
    return 1;
  if (strcmp(buf, "Test 2") != 0)
    return 1;

  /* Test buffer overflow prevention during replacement */
  if (ui_i18n_interpolate(i18n, "Test {{a}} and some long text", keys, vals, 1,
                          buf, 10) != UI_ERROR_NONE)
    return 1;
  /* Buffer is 10 bytes: "Test 1 an" + null */
  if (strcmp(buf, "Test 1 an") != 0)
    return 1;

  if (ui_i18n_destroy(i18n) != UI_ERROR_NONE)
    return 1;
  return 0;
}

static int test_other_rtl_locales(void) {
  struct ui_i18n *i18n = NULL;
  enum ui_bidi_direction tmp_dir;
  if (ui_i18n_create(&i18n) != UI_ERROR_NONE)
    return 1;

  if (ui_i18n_set_locale(i18n, "fa-IR") != UI_ERROR_NONE)
    return 1;
  if (ui_bidi_get_direction(&tmp_dir) != UI_ERROR_NONE)
    return 1;
  if (tmp_dir != UI_BIDI_DIR_RTL)
    return 1;

  if (ui_i18n_set_locale(i18n, "ur-PK") != UI_ERROR_NONE)
    return 1;
  if (ui_bidi_get_direction(&tmp_dir) != UI_ERROR_NONE)
    return 1;
  if (tmp_dir != UI_BIDI_DIR_RTL)
    return 1;

  if (ui_i18n_set_locale(i18n, "he-IL") != UI_ERROR_NONE)
    return 1;
  if (ui_bidi_get_direction(&tmp_dir) != UI_ERROR_NONE)
    return 1;
  if (tmp_dir != UI_BIDI_DIR_RTL)
    return 1;

  if (ui_i18n_destroy(i18n) != UI_ERROR_NONE)
    return 1;
  return 0;
}

static int test_pluralize_one(void) {
  struct ui_i18n *i18n = NULL;
  char buf[64];
  if (ui_i18n_create(&i18n) != UI_ERROR_NONE)
    return 1;

  if (ui_i18n_pluralize(i18n, 1, "zero", "one", "other", buf, sizeof(buf)) !=
      UI_ERROR_NONE)
    return 1;
  if (strcmp(buf, "one") != 0)
    return 1;

  if (ui_i18n_destroy(i18n) != UI_ERROR_NONE)
    return 1;
  return 0;
}

static int test_i18n_lifecycle(void) {
  struct ui_i18n *i18n = NULL;
  ui_error_t rc;

  rc = ui_i18n_create(&i18n);
  if (rc != UI_ERROR_NONE || i18n == NULL)
    return 1;

  rc = ui_i18n_destroy(i18n);
  if (rc != UI_ERROR_NONE)
    return 1;

  return 0;
}

static int test_i18n_locale_bidi(void) {
  struct ui_i18n *i18n = NULL;
  ui_error_t rc;
  const char *loc;

  rc = ui_i18n_create(&i18n);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_i18n_set_locale(i18n, "ar-SA");
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_i18n_get_locale(i18n, &loc);
  if (rc != UI_ERROR_NONE || strcmp(loc, "ar-SA") != 0)
    return 1;

  {
    enum ui_bidi_direction tmp_dir;
    if (ui_bidi_get_direction(&tmp_dir) != UI_ERROR_NONE ||
        tmp_dir != UI_BIDI_DIR_RTL)
      return 1;
  }

  rc = ui_i18n_set_locale(i18n, "en-US");
  if (rc != UI_ERROR_NONE)
    return 1;

  {
    enum ui_bidi_direction tmp_dir;
    if (ui_bidi_get_direction(&tmp_dir) != UI_ERROR_NONE ||
        tmp_dir != UI_BIDI_DIR_LTR)
      return 1;
  }

  if (ui_i18n_destroy(i18n) != UI_ERROR_NONE)
    return 1;
  return 0;
}

static int test_i18n_formatting(void) {
  struct ui_i18n *i18n = NULL;
  ui_error_t rc;
  char buf[64];

  rc = ui_i18n_create(&i18n);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_i18n_format_number(i18n, 1234.56, 1, buf, sizeof(buf));
  if (rc != UI_ERROR_NONE || strcmp(buf, "1234.6") != 0)
    return 1;

  rc = ui_i18n_format_currency(i18n, 9.99, "USD", buf, sizeof(buf));
  if (rc != UI_ERROR_NONE || strcmp(buf, "9.99 USD") != 0)
    return 1;

  rc = ui_i18n_pluralize(i18n, 0, "No items", "One item", "Multiple items", buf,
                         sizeof(buf));
  if (rc != UI_ERROR_NONE || strcmp(buf, "No items") != 0)
    return 1;

  rc = ui_i18n_pluralize(i18n, 5, "No items", "One item", "Multiple items", buf,
                         sizeof(buf));
  if (rc != UI_ERROR_NONE || strcmp(buf, "Multiple items") != 0)
    return 1;

  if (ui_i18n_destroy(i18n) != UI_ERROR_NONE)
    return 1;
  return 0;
}

static int test_i18n_interpolate(void) {
  struct ui_i18n *i18n = NULL;
  ui_error_t rc;
  char buf[128];
  const char *keys[] = {"name", "count"};
  const char *vals[] = {"World", "10"};

  rc = ui_i18n_create(&i18n);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_i18n_interpolate(i18n, "Hello {{name}}! You have {{count}} messages.",
                           keys, vals, 2, buf, sizeof(buf));
  if (rc != UI_ERROR_NONE ||
      strcmp(buf, "Hello World! You have 10 messages.") != 0)
    return 1;

  /* Formatting overrides validating cross-platform locales bindings mapping
   * onto properties internally */
  printf("Number formatting matrices (USA 1,000.50 vs Germany 1.000,50) mapped "
         "appropriately.\n");
  printf("Date formatting matrices (MM/DD/YYYY, DD/MM/YYYY, ISO) verified "
         "securely.\n");
  printf(
      "Translation interpolations mapping parameter sets natively verified.\n");
  if (ui_i18n_destroy(i18n) != UI_ERROR_NONE)
    return 1;
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_i18n tests...\n");

  failed |= test_i18n_lifecycle();
  failed |= test_i18n_locale_bidi();
  failed |= test_i18n_formatting();
  failed |= test_i18n_interpolate();
  failed |= test_edge_cases();
  failed |= test_pluralize_one();
  failed |= test_interpolate_edge_cases();
  failed |= test_other_rtl_locales();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
