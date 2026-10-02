/**
 * @file test_cupertino_content_unavailable.c
 * @brief Unit tests for Cupertino Content Unavailable View
 * (UIContentUnavailableView).
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_content_unavailable.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_content_unavailable_suite);

TEST test_content_unavailable_invalid_arguments(void) {
  struct cupertino_content_unavailable_descriptor desc;
  struct cupertino_content_unavailable_view *view = NULL;
  struct ui_empty_state_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *str = NULL;
  float w, h;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid */
  rc = cupertino_content_unavailable_create(NULL, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_create(dummy_engine, NULL, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range style */
  desc.style = (enum cupertino_content_unavailable_style)99;
  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_content_unavailable_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Setters / getters / trigger / bounds / base invalid */
  rc = cupertino_content_unavailable_set_title(NULL, "Title");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_set_title(
      (struct cupertino_content_unavailable_view *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_get_title(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_get_title(
      (const struct cupertino_content_unavailable_view *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_set_description(NULL, "Desc");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_set_description(
      (struct cupertino_content_unavailable_view *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_get_description(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_get_description(
      (const struct cupertino_content_unavailable_view *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_set_search_query(NULL, "query");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_trigger_action(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_get_dimensions(
      (const struct cupertino_content_unavailable_view *)0x123, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_get_dimensions(
      (const struct cupertino_content_unavailable_view *)0x123, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_content_unavailable_get_base(
      (struct cupertino_content_unavailable_view *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range style below 0 */
  desc.style = (enum cupertino_content_unavailable_style) - 1;
  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_content_unavailable_lifecycle_and_search(void) {
  struct cupertino_content_unavailable_descriptor desc;
  struct cupertino_content_unavailable_view *view = NULL;
  struct ui_empty_state_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *title = NULL;
  const char *desc_text = NULL;
  float w, h;
  ui_error_t rc;

  /* Standard empty state */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_CONTENT_UNAVAILABLE_STANDARD;
  desc.symbol_name = "tray.fill";
  desc.title = "No Documents";
  desc.description_text = "Files saved in iCloud will appear here.";
  desc.action_title = "Add Document";

  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);

  rc = cupertino_content_unavailable_get_base(view, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = cupertino_content_unavailable_get_title(view, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("No Documents", title);

  rc = cupertino_content_unavailable_get_description(view, &desc_text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Files saved in iCloud will appear here.", desc_text);

  rc = cupertino_content_unavailable_get_dimensions(view, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_CONTENT_UNAVAILABLE_DEFAULT_WIDTH, w);
  ASSERT_EQ(CUPERTINO_CONTENT_UNAVAILABLE_DEFAULT_HEIGHT, h);

  /* Trigger action */
  rc = cupertino_content_unavailable_trigger_action(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, view->action_triggered_count);

  /* Update title and description */
  rc = cupertino_content_unavailable_set_title(view, "Empty Inbox");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_content_unavailable_get_title(view, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Empty Inbox", title);

  rc = cupertino_content_unavailable_set_description(view,
                                                     "You are all caught up!");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_content_unavailable_get_description(view, &desc_text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("You are all caught up!", desc_text);

  rc = cupertino_content_unavailable_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Search empty state */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_CONTENT_UNAVAILABLE_SEARCH;
  desc.search_query = "Receipts";

  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);

  rc = cupertino_content_unavailable_get_title(view, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("No Results for 'Receipts'", title);

  /* Update search query dynamically */
  rc = cupertino_content_unavailable_set_search_query(view, "Invoices");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_content_unavailable_get_title(view, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("No Results for 'Invoices'", title);

  rc = cupertino_content_unavailable_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_content_unavailable_defaults(void) {
  struct cupertino_content_unavailable_descriptor desc;
  struct cupertino_content_unavailable_view *view = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *title = NULL;
  const char *desc_text = NULL;
  ui_error_t rc;

  /* Standard style with NULL strings -> defaults: symbol "tray", title "No
   * Content", desc "Check again later." */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_CONTENT_UNAVAILABLE_STANDARD;

  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);
  ASSERT_STR_EQ("tray", view->symbol_name);

  rc = cupertino_content_unavailable_get_title(view, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("No Content", title);

  rc = cupertino_content_unavailable_get_description(view, &desc_text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Check again later.", desc_text);

  /* Set search query on standard style -> should NOT change title to "No
   * Results" */
  rc = cupertino_content_unavailable_set_search_query(view, "test");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_content_unavailable_get_title(view, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("No Content", title);

  /* Setting search query with NULL clears query */
  rc = cupertino_content_unavailable_set_search_query(view, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", view->search_query);

  rc = cupertino_content_unavailable_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Search style with NULL search_query and NULL symbol/title/desc */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_CONTENT_UNAVAILABLE_SEARCH;

  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);
  ASSERT_STR_EQ("magnifyingglass", view->symbol_name);

  rc = cupertino_content_unavailable_get_title(view, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("No Results for ''", title);

  /* Set search query with NULL on SEARCH style */
  rc = cupertino_content_unavailable_set_search_query(view, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_content_unavailable_get_title(view, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("No Results for ''", title);

  /* Detach base to test view->base == NULL branches */
  view->base = NULL;
  rc = cupertino_content_unavailable_set_title(view, "Detached Title");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_content_unavailable_set_description(view, "Detached Desc");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_content_unavailable_set_search_query(view, "Detached Query");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_content_unavailable_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_content_unavailable_oom_mock(void) {
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_content_unavailable_mock_create_fail;
  extern int g_content_unavailable_mock_set_title_fail;
  extern int g_content_unavailable_mock_set_desc_fail;
  extern int g_content_unavailable_mock_destroy_fail;
#endif
  struct cupertino_content_unavailable_descriptor desc;
  struct cupertino_content_unavailable_view *view = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  /* Malloc fail */
  g_malloc_fail_countdown = 0;
  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, view);
  g_malloc_fail_countdown = -1;

  /* Fail empty_state_base_create */
  g_content_unavailable_mock_create_fail = 1;
  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, view);
  g_content_unavailable_mock_create_fail = 0;

  /* Fail empty_state_base_set_title in create (with mock destroy succeeding) */
  g_content_unavailable_mock_set_title_fail = 1;
  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, view);

  /* Fail empty_state_base_set_title in create (with mock destroy failing) */
  g_content_unavailable_mock_destroy_fail = 1;
  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, view);
  g_content_unavailable_mock_set_title_fail = 0;
  g_content_unavailable_mock_destroy_fail = 0;

  /* Fail empty_state_base_set_description in create (with mock destroy
   * succeeding) */
  g_content_unavailable_mock_set_desc_fail = 1;
  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, view);

  /* Fail empty_state_base_set_description in create (with mock destroy failing)
   */
  g_content_unavailable_mock_destroy_fail = 1;
  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, view);
  g_content_unavailable_mock_set_desc_fail = 0;
  g_content_unavailable_mock_destroy_fail = 0;

  /* Create successful view to test setter failure branches */
  rc = cupertino_content_unavailable_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);

  g_content_unavailable_mock_set_title_fail = 1;
  rc = cupertino_content_unavailable_set_title(view, "New Title");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_content_unavailable_mock_set_title_fail = 0;

  g_content_unavailable_mock_set_desc_fail = 1;
  rc = cupertino_content_unavailable_set_description(view, "New Desc");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_content_unavailable_mock_set_desc_fail = 0;

  /* Test destroy failure */
  g_content_unavailable_mock_destroy_fail = 1;
  rc = cupertino_content_unavailable_destroy(view);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_content_unavailable_mock_destroy_fail = 0;

  rc = cupertino_content_unavailable_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  rc = cupertino_content_unavailable_create(dummy_engine, NULL, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_content_unavailable_suite) {
  RUN_TEST(test_content_unavailable_invalid_arguments);
  RUN_TEST(test_content_unavailable_lifecycle_and_search);
  RUN_TEST(test_content_unavailable_defaults);
  RUN_TEST(test_content_unavailable_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_content_unavailable_suite);
  GREATEST_MAIN_END();
}
