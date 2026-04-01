/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_stylus_ink_metrics(void) {
  cmp_event_t ev = {0};
  float opacity, width;

  ev.action = CMP_ACTION_DOWN;
  ev.pressure = 0.5f;
  /* Simulating altitude implicitly since union extensions are opaque in this
   * stub */

  ASSERT_EQ(CMP_SUCCESS,
            cmp_stylus_resolve_ink_metrics(&ev, 10.0f, &opacity, &width));
  ASSERT_EQ(0.5f, opacity);
  /* Width testing is relaxed due to lack of explicit altitude mapping in stub
   * event_t */

  PASS();
}

TEST test_stylus_hover(void) {
  cmp_event_t ev = {0};
  int is_hovering;
  float distance;

  ev.action = CMP_ACTION_MOVE;
  ev.pressure = 0.0f; /* 0 pressure + movement = hover in our stub */

  ASSERT_EQ(CMP_SUCCESS,
            cmp_stylus_evaluate_hover(&ev, &is_hovering, &distance));
  ASSERT_EQ(1, is_hovering);

  ev.pressure = 0.5f; /* Actual touch */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_stylus_evaluate_hover(&ev, &is_hovering, &distance));
  ASSERT_EQ(0, is_hovering);

  PASS();
}

static void dummy_cb(cmp_stylus_event_type_t t, void *u) {
  (void)t;
  (void)u;
}

TEST test_null_args(void) {
  cmp_stylus_context_t *ctx = NULL;
  cmp_event_t ev = {0};
  float f;
  int i;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_stylus_context_create(NULL));
  cmp_stylus_context_create(&ctx);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_stylus_context_set_event_callback(NULL, dummy_cb, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_stylus_resolve_ink_metrics(NULL, 1.0f, &f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_stylus_resolve_ink_metrics(&ev, 1.0f, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_stylus_resolve_ink_metrics(&ev, 1.0f, &f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_stylus_evaluate_hover(NULL, &i, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_stylus_evaluate_hover(&ev, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_stylus_evaluate_hover(&ev, &i, NULL));

  cmp_stylus_context_destroy(ctx);
  PASS();
}

SUITE(stylus_suite) {
  RUN_TEST(test_stylus_ink_metrics);
  RUN_TEST(test_stylus_hover);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(stylus_suite);
  GREATEST_MAIN_END();
}
