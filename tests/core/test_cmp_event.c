/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_event_lifecycle(void) {
  int res;

  res = cmp_event_system_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  
  /* Double init */
  res = cmp_event_system_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_event_system_shutdown();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Double shutdown */
  res = cmp_event_system_shutdown();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_event_uninitialized(void) {
  cmp_event_t evt;
  int res;

  /* Assuming it's shutdown from previous test */
  res = cmp_event_push(&evt);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  res = cmp_event_pop(&evt);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  PASS();
}

TEST test_event_null_args(void) {
  int res;

  cmp_event_system_init();

  res = cmp_event_push(NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  res = cmp_event_pop(NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  cmp_event_system_shutdown();
  PASS();
}

TEST test_event_push_pop(void) {
  int res;
  cmp_event_t evt;
  cmp_event_t out;

  cmp_event_system_init();

  evt.type = 1;
  evt.action = CMP_ACTION_DOWN;
  evt.x = 100;
  evt.y = 200;
  evt.source_id = 0;
  evt.pressure = 1.0f;
  evt.modifiers = 0;

  res = cmp_event_push(&evt);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_event_pop(&out);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT((int)1, (int)out.type, "%d");
  ASSERT_EQ_FMT((int)CMP_ACTION_DOWN, (int)out.action, "%d");
  ASSERT_EQ_FMT(100, out.x, "%d");
  ASSERT_EQ_FMT(200, out.y, "%d");

  /* Second pop should fail */
  res = cmp_event_pop(&out);
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");

  cmp_event_system_shutdown();
  PASS();
}

TEST test_event_focus(void) {
  int res;

  res = cmp_event_clear_focus();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT(-1, cmp_event_get_focus(), "%d");
  ASSERT_EQ_FMT(0, cmp_event_is_focus_ring_visible(), "%d");

  res = cmp_event_set_focus(42);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT(42, cmp_event_get_focus(), "%d");
  ASSERT_EQ_FMT(1, cmp_event_is_focus_ring_visible(), "%d");

  cmp_event_clear_focus();
  PASS();
}

TEST test_event_tab_targeting(void) {
  cmp_event_clear_focus();

  /* Initially no focus, tab targeting forward sets to 1 */
  cmp_event_handle_tab_targeting(0);
  ASSERT_EQ_FMT(1, cmp_event_get_focus(), "%d");
  ASSERT_EQ_FMT(1, cmp_event_is_focus_ring_visible(), "%d");

  /* Tab forward */
  cmp_event_handle_tab_targeting(0);
  ASSERT_EQ_FMT(2, cmp_event_get_focus(), "%d");

  /* Tab backward */
  cmp_event_handle_tab_targeting(1);
  ASSERT_EQ_FMT(1, cmp_event_get_focus(), "%d");

  /* Back below 1 should wrap or stay at bounds (current code goes to 100) */
  cmp_event_handle_tab_targeting(1);
  ASSERT_EQ_FMT(100, cmp_event_get_focus(), "%d");

  /* Forward above 100 goes to 1 */
  cmp_event_handle_tab_targeting(0);
  ASSERT_EQ_FMT(1, cmp_event_get_focus(), "%d");

  cmp_event_clear_focus();
  PASS();
}

TEST test_event_hit_test(void) {
  /* Simple stub test */
  ASSERT_EQ_FMT(1, cmp_event_hit_test(50, 50), "%d");
  PASS();
}

SUITE(event_suite) {
  RUN_TEST(test_event_lifecycle);
  RUN_TEST(test_event_uninitialized);
  RUN_TEST(test_event_null_args);
  RUN_TEST(test_event_push_pop);
  RUN_TEST(test_event_focus);
  RUN_TEST(test_event_tab_targeting);
  RUN_TEST(test_event_hit_test);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(event_suite);
  GREATEST_MAIN_END();
}
