/**
 * @file test_cupertino_quick_look.c
 * @brief Unit tests for Cupertino Quick Look document preview sheet.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_quick_look.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

TEST test_cupertino_quick_look_lifecycle(void) {
  struct cupertino_quick_look *ql = NULL;
  struct cupertino_quick_look_descriptor desc;
  ui_error_t rc;
  int is_open = 0;

  memset(&desc, 0, sizeof(desc));
  desc.is_markup_enabled = 1;
  desc.is_share_enabled = 1;
  desc.initial_zoom = 1.0f;

  /* Null validations */
  rc = cupertino_quick_look_create(NULL, &desc, &ql);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_create((struct ui_engine *)1, NULL, &ql);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_create((struct ui_engine *)1, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Successful creation */
  rc = cupertino_quick_look_create((struct ui_engine *)1, &desc, &ql);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, ql);

  rc = cupertino_quick_look_is_open(ql, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_open);

  /* is_open null checks */
  rc = cupertino_quick_look_is_open(NULL, &is_open);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_is_open(ql, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* set_open toggle tests */
  rc = cupertino_quick_look_set_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_set_open(ql, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_quick_look_set_markup_open(ql, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_quick_look_set_open(ql, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, ql->is_open);
  ASSERT_EQ(0, ql->is_markup_open);
  ASSERT_EQ(CUPERTINO_QL_MARKUP_NONE, ql->active_tool);

  rc = cupertino_quick_look_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_quick_look_destroy(ql);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_cupertino_quick_look_items_and_navigation(void) {
  struct cupertino_quick_look *ql = NULL;
  struct cupertino_quick_look_descriptor desc;
  const struct cupertino_ql_preview_item *item = NULL;
  ui_error_t rc;
  size_t i;

  memset(&desc, 0, sizeof(desc));
  desc.is_markup_enabled = 1;
  desc.is_share_enabled = 1;
  desc.initial_zoom = 1.0f;

  rc = cupertino_quick_look_create((struct ui_engine *)1, &desc, &ql);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks on get_current_item */
  rc = cupertino_quick_look_get_current_item(NULL, &item);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_get_current_item(ql, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* No items initially */
  rc = cupertino_quick_look_get_current_item(ql, &item);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  /* Set current index when item_count == 0 */
  rc = cupertino_quick_look_set_current_index(ql, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Add invalid args */
  rc = cupertino_quick_look_add_item(NULL, "file://a.pdf", "Doc",
                                     CUPERTINO_QL_TYPE_DOCUMENT, 10, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_add_item(ql, NULL, "Doc",
                                     CUPERTINO_QL_TYPE_DOCUMENT, 10, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_add_item(ql, "file://a.pdf", NULL,
                                     CUPERTINO_QL_TYPE_DOCUMENT, 10, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Add item 1 */
  rc = cupertino_quick_look_add_item(ql, "file:///path/to/spec.pdf", "Spec PDF",
                                     CUPERTINO_QL_TYPE_DOCUMENT, 42, 0.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_quick_look_get_current_item(ql, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Spec PDF", item->title);
  ASSERT_EQ(CUPERTINO_QL_TYPE_DOCUMENT, item->item_type);
  ASSERT_EQ(42, (int)item->page_count);

  /* Add item 2 */
  rc = cupertino_quick_look_add_item(ql, "file:///path/to/photo.jpg", "Photo",
                                     CUPERTINO_QL_TYPE_IMAGE, 1, 0.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Switch index */
  rc = cupertino_quick_look_set_current_index(ql, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_quick_look_get_current_item(ql, &item);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Photo", item->title);
  ASSERT_EQ(CUPERTINO_QL_TYPE_IMAGE, item->item_type);

  /* Test current_index >= item_count in get_current_item */
  ql->current_index = 999;
  rc = cupertino_quick_look_get_current_item(ql, &item);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);
  ql->current_index = 1;

  /* Out of range index */
  rc = cupertino_quick_look_set_current_index(ql, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_set_current_index(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Fill up table to capacity test */
  for (i = 2; i < CUPERTINO_QL_MAX_ITEMS; i++) {
    rc = cupertino_quick_look_add_item(ql, "file://item.mp4", "Video",
                                       CUPERTINO_QL_TYPE_VIDEO, 0, 120.5f);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* Exceed max */
  rc = cupertino_quick_look_add_item(ql, "file://extra.mp4", "Extra",
                                     CUPERTINO_QL_TYPE_VIDEO, 0, 10.0f);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  rc = cupertino_quick_look_destroy(ql);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_cupertino_quick_look_markup_and_actions(void) {
  struct cupertino_quick_look *ql = NULL;
  struct cupertino_quick_look_descriptor desc;
  enum cupertino_ql_markup_tool tool = CUPERTINO_QL_MARKUP_NONE;
  float clamped_zoom = 0.0f;
  int is_open = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.is_markup_enabled = 1;
  desc.is_share_enabled = 1;
  desc.initial_zoom = 1.0f;

  rc = cupertino_quick_look_create((struct ui_engine *)1, &desc, &ql);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Markup toggle when active_tool is NONE */
  rc = cupertino_quick_look_set_markup_open(ql, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_quick_look_get_markup_tool(ql, &tool);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_QL_MARKUP_PEN, tool);

  /* Markup toggle when active_tool is already set (non-NONE) */
  rc = cupertino_quick_look_set_markup_open(ql, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_QL_MARKUP_PEN, ql->active_tool);

  /* Change tool to non-NONE */
  rc =
      cupertino_quick_look_set_markup_tool(ql, CUPERTINO_QL_MARKUP_HIGHLIGHTER);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_quick_look_get_markup_tool(ql, &tool);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_QL_MARKUP_HIGHLIGHTER, tool);

  /* Set tool to NONE */
  rc = cupertino_quick_look_set_markup_tool(ql, CUPERTINO_QL_MARKUP_NONE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_quick_look_get_markup_tool(ql, &tool);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_QL_MARKUP_NONE, tool);

  /* Close markup */
  rc = cupertino_quick_look_set_markup_open(ql, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_quick_look_get_markup_tool(ql, &tool);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_QL_MARKUP_NONE, tool);

  /* Zoom pinch */
  rc = cupertino_quick_look_apply_pinch_zoom(ql, 1.5f, &clamped_zoom);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.5f, clamped_zoom);

  /* Pinch zoom clamp max */
  rc = cupertino_quick_look_apply_pinch_zoom(ql, 10.0f, &clamped_zoom);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5.0f, clamped_zoom);

  /* Pinch zoom clamp min */
  rc = cupertino_quick_look_apply_pinch_zoom(ql, 0.01f, &clamped_zoom);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.5f, clamped_zoom);

  /* Share & Done actions */
  rc = cupertino_quick_look_trigger_share(ql);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, ql->share_invoked_count);

  rc = cupertino_quick_look_trigger_done(ql);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, ql->done_invoked_count);

  rc = cupertino_quick_look_is_open(ql, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  rc = cupertino_quick_look_destroy(ql);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_cupertino_quick_look_disabled_features_and_nulls(void) {
  struct cupertino_quick_look *ql = NULL;
  struct cupertino_quick_look_descriptor desc;
  enum cupertino_ql_markup_tool tool;
  float clamped_zoom = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.is_markup_enabled = 0; /* disabled markup */
  desc.is_share_enabled = 0;  /* disabled share */
  desc.initial_zoom = 0.0f;   /* fallback to 1.0f */

  rc = cupertino_quick_look_create((struct ui_engine *)1, &desc, &ql);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Markup disabled allows tool = NONE */
  rc = cupertino_quick_look_set_markup_tool(ql, CUPERTINO_QL_MARKUP_NONE);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_quick_look_set_markup_open(ql, 1);
  ASSERT_EQ(UI_ERROR_UNSUPPORTED, rc);

  rc = cupertino_quick_look_set_markup_tool(ql, CUPERTINO_QL_MARKUP_SIGNATURE);
  ASSERT_EQ(UI_ERROR_UNSUPPORTED, rc);

  rc = cupertino_quick_look_trigger_share(ql);
  ASSERT_EQ(UI_ERROR_UNSUPPORTED, rc);

  /* Invalid args */
  rc = cupertino_quick_look_apply_pinch_zoom(NULL, 1.0f, &clamped_zoom);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_apply_pinch_zoom(ql, -1.0f, &clamped_zoom);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_apply_pinch_zoom(ql, 1.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_quick_look_trigger_share(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_trigger_done(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_quick_look_set_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_is_open(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_quick_look_set_markup_open(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_set_markup_tool(NULL, CUPERTINO_QL_MARKUP_NONE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_get_markup_tool(NULL, &tool);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_quick_look_get_markup_tool(ql, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_quick_look_destroy(ql);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_cupertino_quick_look_oom(void) {
  struct cupertino_quick_look *ql = NULL;
  struct cupertino_quick_look_descriptor desc;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.is_markup_enabled = 1;
  desc.is_share_enabled = 1;
  desc.initial_zoom = 1.0f;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_quick_look_create((struct ui_engine *)1, &desc, &ql);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif
  PASS();
}

SUITE(cupertino_quick_look_suite) {
  RUN_TEST(test_cupertino_quick_look_lifecycle);
  RUN_TEST(test_cupertino_quick_look_items_and_navigation);
  RUN_TEST(test_cupertino_quick_look_markup_and_actions);
  RUN_TEST(test_cupertino_quick_look_disabled_features_and_nulls);
  RUN_TEST(test_cupertino_quick_look_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_quick_look_suite);
  GREATEST_MAIN_END();
}
