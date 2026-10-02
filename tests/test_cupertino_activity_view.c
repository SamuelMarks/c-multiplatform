/**
 * @file test_cupertino_activity_view.c
 * @brief Unit tests for Cupertino Activity View (Share Sheet /
 * UIActivityViewController).
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_activity_view.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_activity_view_suite);

TEST test_activity_view_invalid_arguments(void) {
  struct cupertino_activity_view_descriptor desc;
  struct cupertino_activity_item item;
  struct cupertino_activity_view *view = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *txt = NULL;
  const char *url = NULL;
  size_t count;
  int flag;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  memset(&item, 0, sizeof(item));

  /* Creation invalid */
  rc = cupertino_activity_view_create(NULL, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_create(dummy_engine, NULL, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_activity_view_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Item operations invalid */
  rc = cupertino_activity_view_add_item(NULL, &item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_activity_view_get_item_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_get_item(NULL, 0, &item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Presentation invalid */
  rc = cupertino_activity_view_present(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_dismiss(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_is_open(NULL, &flag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Performance invalid */
  rc = cupertino_activity_view_perform_activity(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_get_last_activated_index(NULL, &flag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_get_share_content(NULL, &txt, &url);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid view with NULL out pointers */
  rc = cupertino_activity_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);

  rc = cupertino_activity_view_add_item(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_get_item_count(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_get_item(view, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_is_open(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_get_last_activated_index(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_get_share_content(view, NULL, &url);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_view_get_share_content(view, &txt, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_activity_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_activity_view_items_and_presentation(void) {
  struct cupertino_activity_view_descriptor desc;
  struct cupertino_activity_item item;
  struct cupertino_activity_item ret_item;
  struct cupertino_activity_view *view = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *txt = NULL;
  const char *url = NULL;
  size_t count;
  int is_open, last_act;
  size_t i;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.share_text = "Check out this design system!";
  desc.share_url = "https://example.com/c-multiplatform";
  desc.is_dark = 1;

  rc = cupertino_activity_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);

  rc = cupertino_activity_view_get_share_content(view, &txt, &url);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Check out this design system!", txt);
  ASSERT_STR_EQ("https://example.com/c-multiplatform", url);

  rc = cupertino_activity_view_get_item_count(view, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)0, count);

  /* Item 0: AirDrop contact */
  memset(&item, 0, sizeof(item));
  strcpy(item.title, "Alice's iPhone");
  strcpy(item.symbol_name, "person.crop.circle");
  item.category = CUPERTINO_ACTIVITY_AIRDROP;
  item.is_disabled = 0;
  rc = cupertino_activity_view_add_item(view, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Item 1: App extension (Messages) */
  memset(&item, 0, sizeof(item));
  strcpy(item.title, "Messages");
  strcpy(item.symbol_name, "message.fill");
  item.category = CUPERTINO_ACTIVITY_APP_EXTENSION;
  item.is_disabled = 0;
  rc = cupertino_activity_view_add_item(view, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Item 2: Disabled App extension */
  memset(&item, 0, sizeof(item));
  strcpy(item.title, "Mail");
  strcpy(item.symbol_name, "envelope.fill");
  item.category = CUPERTINO_ACTIVITY_APP_EXTENSION;
  item.is_disabled = 1;
  rc = cupertino_activity_view_add_item(view, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Item 3: System action (Copy) */
  memset(&item, 0, sizeof(item));
  strcpy(item.title, "Copy");
  strcpy(item.symbol_name, "doc.on.doc");
  item.category = CUPERTINO_ACTIVITY_SYSTEM_ACTION;
  item.is_disabled = 0;
  rc = cupertino_activity_view_add_item(view, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_activity_view_get_item_count(view, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)4, count);

  /* Inspect item 3 */
  rc = cupertino_activity_view_get_item(view, 3, &ret_item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Copy", ret_item.title);
  ASSERT_STR_EQ("doc.on.doc", ret_item.symbol_name);
  ASSERT_EQ(CUPERTINO_ACTIVITY_SYSTEM_ACTION, ret_item.category);
  ASSERT_EQ(0, ret_item.is_disabled);

  /* Out of bounds item retrieval */
  rc = cupertino_activity_view_get_item(view, 99, &ret_item);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Presentation testing */
  rc = cupertino_activity_view_is_open(view, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  rc = cupertino_activity_view_present(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_activity_view_is_open(view, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_open);

  /* Try performing disabled activity (index 2: Mail), should fail */
  rc = cupertino_activity_view_perform_activity(view, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Perform valid activity (index 3: Copy), should succeed and dismiss */
  rc = cupertino_activity_view_perform_activity(view, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_activity_view_is_open(view, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  rc = cupertino_activity_view_get_last_activated_index(view, &last_act);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, last_act);

  /* Out of bounds perform */
  rc = cupertino_activity_view_perform_activity(view, 99);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Re-present and dismiss */
  rc = cupertino_activity_view_present(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_activity_view_dismiss(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_activity_view_is_open(view, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  /* Fill remaining slots to reach CUPERTINO_ACTIVITY_VIEW_MAX_ITEMS */
  for (i = 4; i < CUPERTINO_ACTIVITY_VIEW_MAX_ITEMS; i++) {
    memset(&item, 0, sizeof(item));
    strcpy(item.title, "Action");
    rc = cupertino_activity_view_add_item(view, &item);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* One more should overflow */
  rc = cupertino_activity_view_add_item(view, &item);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  rc = cupertino_activity_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_activity_view_oom_simulation(void) {
  struct cupertino_activity_view_descriptor desc;
  struct cupertino_activity_view *view = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.share_text = "Text";

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_activity_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, view);
  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_activity_view_create(NULL, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_activity_view_suite) {
  RUN_TEST(test_activity_view_invalid_arguments);
  RUN_TEST(test_activity_view_items_and_presentation);
  RUN_TEST(test_activity_view_oom_simulation);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_activity_view_suite);
  GREATEST_MAIN_END();
}
