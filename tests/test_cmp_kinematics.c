/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_rubber_band_create_destroy(void) {
  cmp_rubber_band_t *band = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_rubber_band_create(&band));
  ASSERT_NEQ(NULL, band);
  ASSERT_EQ(CMP_SUCCESS, cmp_rubber_band_destroy(band));

  PASS();
}

TEST test_rubber_band_physics(void) {
  cmp_rubber_band_t *band = NULL;
  float target = 0.0f;
  float pos = 0.0f;
  int resting = 0;
  int iters = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_rubber_band_create(&band));

  /* Fling past boundary with positive velocity */
  ASSERT_EQ(CMP_SUCCESS, cmp_rubber_band_start(band, 2000.0f, 100.0f));

  /* Step 1: Should stretch further out, then velocity reverses towards target
   * (0.0) */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_rubber_band_step(band, 16.0f, target, &pos, &resting));
  ASSERT_EQ(0, resting);

  /* Fast-forward simulation */
  while (!resting && iters < 1000) {
    cmp_rubber_band_step(band, 16.0f, target, &pos, &resting);
    iters++;
  }

  ASSERT_EQ(1, resting);
  ASSERT_EQ(0.0f, pos); /* Snapped cleanly back to target */

  ASSERT_EQ(CMP_SUCCESS, cmp_rubber_band_destroy(band));
  PASS();
}

TEST test_scroll_smooth_create_destroy(void) {
  cmp_scroll_smooth_t *smooth = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_smooth_create(&smooth));
  ASSERT_NEQ(NULL, smooth);
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_smooth_destroy(smooth));

  PASS();
}

TEST test_scroll_smooth_interpolation(void) {
  cmp_scroll_smooth_t *smooth = NULL;
  float pos = 0.0f;
  int complete = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_smooth_create(&smooth));

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_smooth_start(smooth, 0.0f, 500.0f));

  /* Step midway */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_scroll_smooth_step(smooth, 150.0f, &pos, &complete));
  ASSERT_EQ(0, complete);
  ASSERT(pos > 0.0f && pos < 500.0f); /* Should be roughly 250 on ease-in-out */

  /* Step to end */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_scroll_smooth_step(smooth, 160.0f, &pos, &complete));
  ASSERT_EQ(1, complete);
  ASSERT_EQ(500.0f, pos);

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_smooth_destroy(smooth));
  PASS();
}

TEST test_kinematics_edge_cases(void) {
  cmp_rubber_band_t *band = NULL;
  cmp_scroll_smooth_t *smooth = NULL;
  float pos;
  int state;

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_rubber_band_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_rubber_band_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_rubber_band_start(NULL, 0, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_rubber_band_step(NULL, 16, 0, &pos, &state));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_smooth_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_smooth_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_smooth_start(NULL, 0, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_smooth_step(NULL, 16, &pos, &state));

  /* Valid pointers, invalid output arg pointers */
  ASSERT_EQ(CMP_SUCCESS, cmp_rubber_band_create(&band));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_rubber_band_step(band, 16, 0, NULL, &state));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_rubber_band_step(band, 16, 0, &pos, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_rubber_band_destroy(band));

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_smooth_create(&smooth));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_smooth_step(smooth, 16, NULL, &state));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_smooth_step(smooth, 16, &pos, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_smooth_destroy(smooth));

  PASS();
}

TEST test_scroll_snap_stop(void) {
  /* ALWAYS */
  ASSERT_EQ(1,
            cmp_scroll_snap_stop_evaluate(CMP_SCROLL_SNAP_STOP_ALWAYS, 50.0f));
  ASSERT_EQ(
      1, cmp_scroll_snap_stop_evaluate(CMP_SCROLL_SNAP_STOP_ALWAYS, 5000.0f));

  /* NORMAL */
  ASSERT_EQ(1, cmp_scroll_snap_stop_evaluate(CMP_SCROLL_SNAP_STOP_NORMAL,
                                             50.0f)); /* Slow */
  ASSERT_EQ(0, cmp_scroll_snap_stop_evaluate(CMP_SCROLL_SNAP_STOP_NORMAL,
                                             2000.0f)); /* Fast */

  PASS();
}

TEST test_scroll_padding(void) {
  cmp_rect_t el = {0, 500.0f, 100.0f, 100.0f};
  cmp_scroll_padding_t pad = {50.0f, 0, 0, 0};
  cmp_scroll_padding_t mar = {25.0f, 0, 0, 0};
  float out_y = 0.0f;

  /* Base Y */
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_padding_apply(&el, NULL, NULL, &out_y));
  ASSERT_EQ(500.0f, out_y);

  /* With Padding */
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_padding_apply(&el, &pad, NULL, &out_y));
  ASSERT_EQ(450.0f, out_y);

  /* With Margin */
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_padding_apply(&el, NULL, &mar, &out_y));
  ASSERT_EQ(475.0f, out_y);

  /* With Both */
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_padding_apply(&el, &pad, &mar, &out_y));
  ASSERT_EQ(425.0f, out_y);

  /* Edge cases */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_padding_apply(NULL, &pad, &mar, &out_y));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_padding_apply(&el, &pad, &mar, NULL));

  PASS();
}

TEST test_overscroll(void) {
  ASSERT_EQ(1, cmp_overscroll_evaluate(CMP_OVERSCROLL_AUTO, 0));
  ASSERT_EQ(1, cmp_overscroll_evaluate(CMP_OVERSCROLL_AUTO, 1));
  ASSERT_EQ(1, cmp_overscroll_evaluate(CMP_OVERSCROLL_CONTAIN, 0));
  ASSERT_EQ(0, cmp_overscroll_evaluate(CMP_OVERSCROLL_CONTAIN, 1));
  ASSERT_EQ(1, cmp_overscroll_evaluate(CMP_OVERSCROLL_NONE, 0));
  ASSERT_EQ(0, cmp_overscroll_evaluate(CMP_OVERSCROLL_NONE, 1));
  PASS();
}

TEST test_scroll_momentum(void) {
  float offset = 0.0f;
  float vel = 0.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_momentum_calculate(1000.0f, 16.666f, 0.9f,
                                                       &offset, &vel));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_momentum_calculate(1000.0f, 16.666f, 0.9f, NULL, &vel));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_momentum_calculate(
                                       1000.0f, 16.666f, 0.9f, &offset, NULL));
  PASS();
}

SUITE(cmp_kinematics_suite) {
  RUN_TEST(test_rubber_band_create_destroy);
  RUN_TEST(test_rubber_band_physics);
  RUN_TEST(test_scroll_smooth_create_destroy);
  RUN_TEST(test_scroll_smooth_interpolation);
  RUN_TEST(test_kinematics_edge_cases);
  RUN_TEST(test_scroll_snap_stop);
  RUN_TEST(test_scroll_padding);
  RUN_TEST(test_overscroll);
  RUN_TEST(test_scroll_momentum);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_kinematics_suite);
  GREATEST_MAIN_END();
}
