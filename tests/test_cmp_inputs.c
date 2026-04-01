/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_wheel_picker(void) {
  cmp_wheel_picker_t *ctx = NULL;
  const char *items[] = {"A", "B", "C"};
  size_t idx;

  ASSERT_EQ(CMP_SUCCESS, cmp_wheel_picker_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_wheel_picker_set_items(ctx, items, 3));

  /* Initial state */
  ASSERT_EQ(CMP_SUCCESS, cmp_wheel_picker_get_selected(ctx, &idx));
  ASSERT_EQ(0, idx);

  /* Scroll down 1 item (44px) */
  ASSERT_EQ(CMP_SUCCESS, cmp_wheel_picker_scroll(ctx, 44.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_wheel_picker_get_selected(ctx, &idx));
  ASSERT_EQ(1, idx);

  /* Over-scroll, clamp */
  ASSERT_EQ(CMP_SUCCESS, cmp_wheel_picker_scroll(ctx, 999.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_wheel_picker_get_selected(ctx, &idx));
  ASSERT_EQ(2, idx);

  ASSERT_EQ(CMP_SUCCESS, cmp_wheel_picker_destroy(ctx));
  PASS();
}

TEST test_segmented_control(void) {
  cmp_segmented_control_t *ctx = NULL;
  const char *segs[] = {"Map", "Transit", "Satellite"};
  size_t idx;
  float offset;

  ASSERT_EQ(CMP_SUCCESS, cmp_segmented_control_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_segmented_control_set_segments(ctx, segs, 3));

  /* Initial state */
  ASSERT_EQ(CMP_SUCCESS, cmp_segmented_control_get_visuals(ctx, &idx, &offset));
  ASSERT_EQ(0, idx);
  ASSERT_EQ(0.0f, offset);

  /* Select */
  ASSERT_EQ(CMP_SUCCESS, cmp_segmented_control_select(ctx, 2));
  ASSERT_EQ(CMP_SUCCESS, cmp_segmented_control_get_visuals(ctx, &idx, &offset));
  ASSERT_EQ(2, idx);
  ASSERT_EQ(200.0f, offset);

  ASSERT_EQ(CMP_SUCCESS, cmp_segmented_control_destroy(ctx));
  PASS();
}

TEST test_stepper(void) {
  cmp_stepper_t *ctx = NULL;
  int val;

  ASSERT_EQ(CMP_SUCCESS, cmp_stepper_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_stepper_set_limits(ctx, 0, 10, 2));

  ASSERT_EQ(CMP_SUCCESS, cmp_stepper_increment(ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_stepper_get_value(ctx, &val));
  ASSERT_EQ(2, val);

  ASSERT_EQ(CMP_SUCCESS, cmp_stepper_decrement(ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_stepper_get_value(ctx, &val));
  ASSERT_EQ(0, val);

  ASSERT_EQ(CMP_SUCCESS, cmp_stepper_decrement(ctx)); /* clamp */
  ASSERT_EQ(CMP_SUCCESS, cmp_stepper_get_value(ctx, &val));
  ASSERT_EQ(0, val);

  ASSERT_EQ(CMP_SUCCESS, cmp_stepper_destroy(ctx));
  PASS();
}

TEST test_slider(void) {
  cmp_slider_t *ctx = NULL;
  float perc;

  ASSERT_EQ(CMP_SUCCESS, cmp_slider_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_slider_set_limits(ctx, 50.0f, 150.0f));

  ASSERT_EQ(CMP_SUCCESS, cmp_slider_set_value(ctx, 100.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_slider_get_visuals(ctx, &perc));
  ASSERT_EQ(0.5f, perc);

  ASSERT_EQ(CMP_SUCCESS, cmp_slider_destroy(ctx));
  PASS();
}

TEST test_system_pickers(void) {
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_color_picker_show(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_date_picker_show(NULL));
  PASS();
}

TEST test_null_args(void) {
  cmp_wheel_picker_t *wp = NULL;
  cmp_segmented_control_t *sc = NULL;
  cmp_stepper_t *st = NULL;
  cmp_slider_t *sl = NULL;
  size_t i;
  float f;
  int n;

  cmp_wheel_picker_create(&wp);
  cmp_segmented_control_create(&sc);
  cmp_stepper_create(&st);
  cmp_slider_create(&sl);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_wheel_picker_set_items(NULL, NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_wheel_picker_set_items(wp, NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_wheel_picker_scroll(NULL, 1.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_wheel_picker_get_selected(NULL, &i));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_wheel_picker_get_selected(wp, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_segmented_control_set_segments(NULL, NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_segmented_control_set_segments(sc, NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_segmented_control_select(NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_segmented_control_select(sc, 999));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_segmented_control_get_visuals(NULL, &i, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_segmented_control_get_visuals(sc, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_segmented_control_get_visuals(sc, &i, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_stepper_set_limits(NULL, 0, 10, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_stepper_set_limits(st, 10, 0, 1)); /* inverted bounds */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_stepper_get_value(NULL, &n));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_stepper_get_value(st, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_stepper_increment(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_stepper_decrement(NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_slider_set_limits(NULL, 0.0f, 1.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_slider_set_limits(sl, 1.0f, 0.0f)); /* inverted */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_slider_set_value(NULL, 0.5f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_slider_get_visuals(NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_slider_get_visuals(sl, NULL));

  cmp_wheel_picker_destroy(wp);
  cmp_segmented_control_destroy(sc);
  cmp_stepper_destroy(st);
  cmp_slider_destroy(sl);
  PASS();
}

SUITE(inputs_suite) {
  RUN_TEST(test_wheel_picker);
  RUN_TEST(test_segmented_control);
  RUN_TEST(test_stepper);
  RUN_TEST(test_slider);
  RUN_TEST(test_system_pickers);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(inputs_suite);
  GREATEST_MAIN_END();
}
