/**
 * @file test_cupertino_search_bar.c
 * @brief Unit tests for Cupertino Search Bar component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_search_bar.h"
#include "ui_component.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_search_bar_suite);

TEST test_search_bar_invalid_args(void) {
  struct cupertino_search_bar_descriptor desc;
  struct cupertino_search_bar *bar = NULL;
  struct ui_search_bar_base *base = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *str = NULL;
  size_t idx = 0;
  size_t count = 0;
  float progress = 0.0f;
  int focused = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Create invalid */
  rc = cupertino_search_bar_create(NULL, &desc, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_create(dummy_engine, NULL, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Scope count > max */
  desc.scope_count = CUPERTINO_SEARCH_BAR_MAX_SCOPES + 1;
  rc = cupertino_search_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.scope_count = 0;

  /* Destroy invalid */
  rc = cupertino_search_bar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Query / clear */
  rc = cupertino_search_bar_set_query(NULL, "Apple");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_get_query(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_get_query(
      (const struct cupertino_search_bar *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_clear(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Focus / cancel */
  rc = cupertino_search_bar_set_focused(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_is_focused(NULL, &focused);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_is_focused(
      (const struct cupertino_search_bar *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_get_cancel_progress(NULL, &progress);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_get_cancel_progress(
      (const struct cupertino_search_bar *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Scopes */
  rc = cupertino_search_bar_set_scope_selected(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_get_scope_selected(NULL, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_get_scope_selected(
      (const struct cupertino_search_bar *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Tokens */
  rc = cupertino_search_bar_add_token(NULL, "Tag", &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_add_token((struct cupertino_search_bar *)0x123,
                                      NULL, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_get_token_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_get_token_count(
      (const struct cupertino_search_bar *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* CVA / base */
  rc = cupertino_search_bar_get_cva(NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_get_cva((struct cupertino_search_bar *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_search_bar_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      cupertino_search_bar_get_base((struct cupertino_search_bar *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_search_bar_lifecycle_and_features(void) {
  struct cupertino_search_bar_descriptor desc;
  const char *scopes[3];
  struct cupertino_search_bar *bar = NULL;
  struct ui_search_bar_base *base = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *str = NULL;
  size_t idx = 0;
  size_t count = 0;
  float progress = 0.0f;
  int focused = 0;
  union ui_signal_payload payload;
  ui_error_t rc;

  scopes[0] = "All";
  scopes[1] = "Photos";
  scopes[2] = "Videos";

  memset(&desc, 0, sizeof(desc));
  desc.placeholder = "Search";
  desc.show_cancel_button = 1;
  desc.scope_titles = scopes;
  desc.scope_count = 3;
  desc.initial_scope_index = 0;

  rc = cupertino_search_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bar != NULL);

  /* Check base & CVA */
  rc = cupertino_search_bar_get_base(bar, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = cupertino_search_bar_get_cva(bar, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(cva != NULL);

  /* Initial query is empty */
  rc = cupertino_search_bar_get_query(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", str);

  /* Set query */
  rc = cupertino_search_bar_set_query(bar, "Cupertino HIG");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_search_bar_get_query(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Cupertino HIG", str);

  /* Clear query */
  rc = cupertino_search_bar_clear(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_search_bar_get_query(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", str);

  /* Test focus and cancel button slide */
  rc = cupertino_search_bar_is_focused(bar, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, focused);

  rc = cupertino_search_bar_get_cancel_progress(bar, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(progress < 0.001f);

  rc = cupertino_search_bar_set_focused(bar, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_search_bar_is_focused(bar, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, focused);

  rc = cupertino_search_bar_get_cancel_progress(bar, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(progress > 0.999f);

  rc = cupertino_search_bar_set_focused(bar, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_search_bar_is_focused(bar, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, focused);

  /* Scopes selection */
  rc = cupertino_search_bar_get_scope_selected(bar, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);

  rc = cupertino_search_bar_set_scope_selected(bar, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_search_bar_get_scope_selected(bar, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, idx);

  rc = cupertino_search_bar_set_scope_selected(bar, 99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Tokens */
  rc = cupertino_search_bar_get_token_count(bar, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, count);

  rc = cupertino_search_bar_add_token(bar, "swift", &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);

  rc = cupertino_search_bar_add_token(bar, "apple", &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);

  rc = cupertino_search_bar_get_token_count(bar, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, count);

  /* Max tokens limit */
  rc = cupertino_search_bar_add_token(bar, "token3", &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_search_bar_add_token(bar, "token4", &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_search_bar_add_token(bar, "overflow", &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* Test CVA integration */
  payload.ptr_val = (void *)"Search via CVA";
  rc = cva->write_value(bar, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_search_bar_get_query(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Search via CVA", str);

  payload.ptr_val = NULL;
  rc = cva->write_value(bar, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_search_bar_get_query(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", str);

  rc = cva->set_disabled_state(bar, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, bar->base.is_disabled);

  /* Test null query in set_query */
  rc = cupertino_search_bar_set_query(bar, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_search_bar_get_query(bar, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", str);

  /* Test CVA callbacks with NULL component */
  payload.ptr_val = (void *)"hello";
  rc = cva->write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cva->set_disabled_state(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_search_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test add_token with NULL out_index on a fresh search_bar */
  memset(&desc, 0, sizeof(desc));
  rc = cupertino_search_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bar != NULL);

  rc = cupertino_search_bar_add_token(bar, "no_out_idx", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_search_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test scope_count > 0 with scope_titles == NULL and with scope_titles[i] ==
   * NULL */
  memset(&desc, 0, sizeof(desc));
  desc.scope_count = 2;
  desc.scope_titles = NULL;
  rc = cupertino_search_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bar != NULL);
  rc = cupertino_search_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  scopes[0] = NULL;
  scopes[1] = "Defined";
  memset(&desc, 0, sizeof(desc));
  desc.scope_count = 2;
  desc.scope_titles = scopes;
  rc = cupertino_search_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bar != NULL);

  /* Test destroy when search_bar->component is NULL */
  ui_component_destroy(bar->component);
  bar->component = NULL;
  rc = cupertino_search_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_search_bar_oom_mock(void) {
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_search_bar_mock_init_fail;
  extern int g_cupertino_search_bar_mock_cleanup_fail;
  extern int g_cupertino_search_bar_mock_set_query_fail;
#endif
  struct cupertino_search_bar_descriptor desc;
  struct cupertino_search_bar *bar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  /* Fail malloc for struct cupertino_search_bar */
  g_malloc_fail_countdown = 0;
  rc = cupertino_search_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(bar == NULL);

  /* Fail malloc for component */
  g_malloc_fail_countdown = 1;
  rc = cupertino_search_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(bar == NULL);
  g_malloc_fail_countdown = -1;

  /* Fail ui_search_bar_base_init */
  g_cupertino_search_bar_mock_init_fail = 1;
  rc = cupertino_search_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT(bar == NULL);
  g_cupertino_search_bar_mock_init_fail = 0;

  /* Create successful search bar to test cleanup and set_query failures */
  rc = cupertino_search_bar_create(dummy_engine, &desc, &bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bar != NULL);

  g_cupertino_search_bar_mock_set_query_fail = 1;
  rc = cupertino_search_bar_set_query(bar, "test");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_search_bar_mock_set_query_fail = 0;

  g_cupertino_search_bar_mock_cleanup_fail = 1;
  rc = cupertino_search_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_search_bar_mock_cleanup_fail = 0;

  rc = cupertino_search_bar_destroy(bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  rc = cupertino_search_bar_create(dummy_engine, NULL, &bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_search_bar_suite) {
  RUN_TEST(test_search_bar_invalid_args);
  RUN_TEST(test_search_bar_lifecycle_and_features);
  RUN_TEST(test_search_bar_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_search_bar_suite);
  GREATEST_MAIN_END();
}
