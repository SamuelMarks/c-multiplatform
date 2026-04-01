/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_event_lifecycle(void) {
  int res;

  res = cmp_event_system_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_event_system_shutdown();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

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

SUITE(event_suite) {
  RUN_TEST(test_event_lifecycle);
  RUN_TEST(test_event_push_pop);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(event_suite);
  GREATEST_MAIN_END();
}
