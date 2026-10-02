/**
 * @file test_cupertino_tab_view.c
 * @brief Unit tests for Cupertino Tab View and parallel navigation stacks.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_tab_view.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_tab_view_suite);

TEST test_tab_view_invalid_arguments(void) {
  struct cupertino_tab_view_descriptor desc;
  struct cupertino_tab_view *view = NULL;
  struct ui_component *root_comp = (struct ui_component *)0x1234;
  struct ui_component *comp = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x5678;
  const char *title = NULL;
  size_t depth;
  float offset;
  int did_scroll;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.root_view = root_comp;

  /* Creation invalid */
  rc = cupertino_tab_view_create(NULL, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_create(dummy_engine, NULL, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.root_view = NULL;
  rc = cupertino_tab_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_tab_view_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Operations invalid */
  rc = cupertino_tab_view_push(NULL, root_comp, "Title");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_pop(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_pop_to_root(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_get_stack_depth(NULL, &depth);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_get_current_view(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_get_current_title(NULL, &title);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_set_scroll_offset(NULL, 100.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_get_scroll_offset(NULL, &offset);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_handle_tab_tap(NULL, &did_scroll);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Operations with valid view but NULL second argument */
  desc.root_view = root_comp;
  desc.default_title = NULL;
  rc = cupertino_tab_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);

  rc = cupertino_tab_view_push(view, NULL, "Title");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_get_stack_depth(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_get_current_view(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_get_current_title(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_get_scroll_offset(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_view_handle_tab_tap(view, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Pop with NULL out_popped */
  rc = cupertino_tab_view_push(view, root_comp, "SubView");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_view_pop(view, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_tab_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_tab_view_stack_and_scroll_preservation(void) {
  struct cupertino_tab_view_descriptor desc;
  struct cupertino_tab_view *view = NULL;
  struct ui_component *root_comp = NULL;
  struct ui_component *sub_comp1 = NULL;
  struct ui_component *sub_comp2 = NULL;
  struct ui_component *ret_comp = NULL;
  struct ui_component *popped = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *title = NULL;
  size_t depth;
  float offset;
  int did_scroll;
  size_t i;
  ui_error_t rc;

  rc = ui_component_create(&root_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&sub_comp1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&sub_comp2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&desc, 0, sizeof(desc));
  desc.root_view = root_comp;
  desc.default_title = "Root Feed";

  rc = cupertino_tab_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(view != NULL);

  rc = cupertino_tab_view_get_stack_depth(view, &depth);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)1, depth);

  rc = cupertino_tab_view_get_current_view(view, &ret_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(root_comp, ret_comp);

  rc = cupertino_tab_view_get_current_title(view, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Root Feed", title);

  /* Setting root scroll offset */
  rc = cupertino_tab_view_set_scroll_offset(view, 320.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_view_get_scroll_offset(view, &offset);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(320.0f, offset, "%f");

  /* Push sub_comp1 */
  rc = cupertino_tab_view_push(view, sub_comp1, "Post Details");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_view_get_stack_depth(view, &depth);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)2, depth);

  rc = cupertino_tab_view_get_current_view(view, &ret_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(sub_comp1, ret_comp);

  rc = cupertino_tab_view_get_current_title(view, &title);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Post Details", title);

  /* Set sub_comp1 scroll offset */
  rc = cupertino_tab_view_set_scroll_offset(view, 150.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Push sub_comp2 */
  rc = cupertino_tab_view_push(view, sub_comp2, "Author Profile");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_view_get_stack_depth(view, &depth);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)3, depth);

  /* Pop sub_comp2 */
  rc = cupertino_tab_view_pop(view, &popped);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(sub_comp2, popped);

  rc = cupertino_tab_view_get_stack_depth(view, &depth);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)2, depth);

  /* Verify sub_comp1 offset was preserved */
  rc = cupertino_tab_view_get_scroll_offset(view, &offset);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(150.0f, offset, "%f");

  /* Test handle_tab_tap when depth == 2: should pop to root without scrolling
   * to top */
  rc = cupertino_tab_view_handle_tab_tap(view, &did_scroll);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, did_scroll);
  rc = cupertino_tab_view_get_stack_depth(view, &depth);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)1, depth);

  /* Root scroll offset was preserved at 320.0f */
  rc = cupertino_tab_view_get_scroll_offset(view, &offset);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(320.0f, offset, "%f");

  /* Second tap on active tab when at root: resets scroll offset to 0.0f
   * (scrolls to top) */
  rc = cupertino_tab_view_handle_tab_tap(view, &did_scroll);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, did_scroll);
  rc = cupertino_tab_view_get_scroll_offset(view, &offset);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(0.0f, offset, "%f");

  /* Try popping when at root -> should fail with UI_ERROR_OUT_OF_BOUNDS */
  rc = cupertino_tab_view_pop(view, &popped);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Fill up to max stack depth */
  for (i = 1; i < CUPERTINO_TAB_VIEW_MAX_STACK_DEPTH; i++) {
    rc = cupertino_tab_view_push(view, sub_comp1, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Exceeding max depth should return UI_ERROR_OUT_OF_BOUNDS */
  rc = cupertino_tab_view_push(view, sub_comp2, "Too Deep");
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Pop to root */
  rc = cupertino_tab_view_pop_to_root(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_view_get_stack_depth(view, &depth);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)1, depth);

  /* Cleanup */
  rc = ui_component_destroy(root_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(sub_comp1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(sub_comp2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_tab_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_tab_view_oom_simulation(void) {
  struct cupertino_tab_view_descriptor desc;
  struct cupertino_tab_view *view = NULL;
  struct ui_component dummy_comp;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&dummy_comp, 0, sizeof(dummy_comp));
  memset(&desc, 0, sizeof(desc));
  desc.root_view = &dummy_comp;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_tab_view_create(dummy_engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, view);
  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_tab_view_create(NULL, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_tab_view_suite) {
  RUN_TEST(test_tab_view_invalid_arguments);
  RUN_TEST(test_tab_view_stack_and_scroll_preservation);
  RUN_TEST(test_tab_view_oom_simulation);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_tab_view_suite);
  GREATEST_MAIN_END();
}
