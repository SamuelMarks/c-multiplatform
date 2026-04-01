/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

static void *dummy_route_handler(const char *route_params) {
  (void)route_params;
  return NULL;
}

TEST test_edge_swipe_pop(void) {
  cmp_edge_swipe_t *ctx = NULL;
  cmp_router_t *router = NULL;
  cmp_string_t uri = {0};

  ASSERT_EQ(CMP_SUCCESS, cmp_router_create(&router));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_register(router, "/home",
                                             dummy_route_handler, NULL, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_register(router, "/detail",
                                             dummy_route_handler, NULL, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_push(router, "/home"));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_push(router, "/detail"));

  ASSERT_EQ(CMP_SUCCESS, cmp_edge_swipe_create(&ctx, router));

  /* Initial active drag */
  ASSERT_EQ(CMP_SUCCESS, cmp_edge_swipe_process(ctx, 100.0f, 400.0f,
                                                CMP_GESTURE_STATE_CHANGED));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_get_current(router, &uri));
  ASSERT_STR_EQ("/detail", uri.data); /* Still on detail page */
  cmp_string_destroy(&uri);

  /* Release before threshold (cancel pop) */
  ASSERT_EQ(CMP_SUCCESS, cmp_edge_swipe_process(ctx, 120.0f, 400.0f,
                                                CMP_GESTURE_STATE_ENDED));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_get_current(router, &uri));
  ASSERT_STR_EQ("/detail", uri.data); /* Not popped */
  cmp_string_destroy(&uri);

  /* Full drag past threshold and release */
  ASSERT_EQ(CMP_SUCCESS, cmp_edge_swipe_process(ctx, 200.0f, 400.0f,
                                                CMP_GESTURE_STATE_CHANGED));
  ASSERT_EQ(CMP_SUCCESS, cmp_edge_swipe_process(ctx, 220.0f, 400.0f,
                                                CMP_GESTURE_STATE_ENDED));

  ASSERT_EQ(CMP_SUCCESS, cmp_router_get_current(router, &uri));
  ASSERT_STR_EQ("/home", uri.data); /* Successfully popped back to home */
  cmp_string_destroy(&uri);

  ASSERT_EQ(CMP_SUCCESS, cmp_edge_swipe_destroy(ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_destroy(router));
  PASS();
}

TEST test_gesture_overrides(void) {
  cmp_gesture_t *dummy1 =
      (cmp_gesture_t *)1; /* Using pointer addresses for stub tests */
  cmp_gesture_t *dummy2 = (cmp_gesture_t *)2;

  /* Dependency linking */
  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_require_failure(dummy1, dummy2));

  /* System OS touches override */
  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_cancel_on_system_override(dummy1, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_cancel_on_system_override(
                             dummy1, 4)); /* Would cancel internally */

  PASS();
}

TEST test_null_args(void) {
  cmp_edge_swipe_t *es = NULL;
  cmp_router_t *rt = NULL;
  cmp_gesture_t *g = NULL;

  float f;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_edge_swipe_create(NULL, NULL));
  cmp_router_create(&rt);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_edge_swipe_create(&es, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_edge_swipe_create(NULL, rt));
  cmp_edge_swipe_create(&es, rt);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_edge_swipe_process(NULL, 1.0f, 1.0f, CMP_GESTURE_STATE_BEGAN));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_edge_swipe_process(es, 1.0f, -1.0f, CMP_GESTURE_STATE_BEGAN));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_gesture_require_failure(NULL, g));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_gesture_require_failure((cmp_gesture_t *)1, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_complex_gesture_set_zoom_limits(NULL, 0.5f, 2.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_complex_gesture_set_zoom_limits(
                (cmp_complex_gesture_t *)1, 2.0f, 0.5f)); /* Inverted bounds */

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_complex_gesture_get_zoom(NULL, &f, &f, &f));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_complex_gesture_get_zoom((cmp_complex_gesture_t *)1, NULL, &f, &f));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_complex_gesture_get_zoom((cmp_complex_gesture_t *)1, &f, NULL, &f));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_complex_gesture_get_zoom((cmp_complex_gesture_t *)1, &f, &f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_complex_gesture_set_rotation_snapping(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_complex_gesture_get_rotation(NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_complex_gesture_get_rotation((cmp_complex_gesture_t *)1, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_gesture_cancel_on_system_override(NULL, 3));

  cmp_edge_swipe_destroy(es);
  cmp_router_destroy(rt);
  PASS();
}

SUITE(gestures_hig_suite) {
  RUN_TEST(test_edge_swipe_pop);
  RUN_TEST(test_gesture_overrides);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(gestures_hig_suite);
  GREATEST_MAIN_END();
}
