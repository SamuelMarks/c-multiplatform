/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_dynamic_type(void) {
  cmp_dynamic_type_t *dt = NULL;
  float scaled;
  int reflow;

  ASSERT_EQ(CMP_SUCCESS, cmp_dynamic_type_create(&dt));

  ASSERT_EQ(CMP_SUCCESS, cmp_dynamic_type_apply_scale(dt, 16.0f, &scaled));
  ASSERT_EQ(16.0f, scaled);

  ASSERT_EQ(CMP_SUCCESS, cmp_dynamic_type_should_reflow(dt, &reflow));
  ASSERT_EQ(0, reflow);

  ASSERT_EQ(CMP_SUCCESS, cmp_dynamic_type_set_category(
                             dt, CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_LARGE));

  ASSERT_EQ(CMP_SUCCESS, cmp_dynamic_type_apply_scale(dt, 16.0f, &scaled));
  ASSERT_EQ(40.0f, scaled);

  ASSERT_EQ(CMP_SUCCESS, cmp_dynamic_type_should_reflow(dt, &reflow));
  ASSERT_EQ(1, reflow);

  ASSERT_EQ(CMP_SUCCESS, cmp_dynamic_type_destroy(dt));
  PASS();
}

TEST test_bold_text(void) {
  cmp_a11y_bold_text_t *bt = NULL;
  int weight;

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_bold_text_create(&bt));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_bold_text_apply(bt, 400, &weight));
  ASSERT_EQ(400, weight);

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_bold_text_set(bt, 1));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_bold_text_apply(bt, 400, &weight));
  ASSERT_EQ(700, weight);

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_bold_text_apply(bt, 800, &weight));
  ASSERT_EQ(900, weight); /* clamped */

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_bold_text_destroy(bt));
  PASS();
}

TEST test_button_shapes(void) {
  cmp_a11y_button_shapes_t *bs = NULL;
  int should_draw;

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_button_shapes_create(&bs));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_button_shapes_should_draw(bs, &should_draw));
  ASSERT_EQ(0, should_draw);

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_button_shapes_set(bs, 1));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_button_shapes_should_draw(bs, &should_draw));
  ASSERT_EQ(1, should_draw);

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_button_shapes_destroy(bs));
  PASS();
}

TEST test_increase_contrast(void) {
  cmp_a11y_increase_contrast_t *ic = NULL;
  float opacity;

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_increase_contrast_create(&ic));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_increase_contrast_apply(ic, &opacity));
  ASSERT_EQ(0.85f, opacity);

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_increase_contrast_set(ic, 1));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_increase_contrast_apply(ic, &opacity));
  ASSERT_EQ(1.0f, opacity);

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_increase_contrast_destroy(ic));
  PASS();
}

TEST test_hover_text(void) {
  cmp_a11y_hover_text_t *ht = NULL;
  char buf[32];

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_hover_text_create(&ht));
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_hover_text_get_bubble(ht, 1, buf, 32));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_hover_text_destroy(ht));
  PASS();
}

TEST test_contrast_ratio(void) {
  int passes;

  /* Black on White */
  ASSERT_EQ(CMP_SUCCESS, cmp_color_verify_contrast_ratio(0x000000FF, 0xFFFFFFFF,
                                                         0, &passes));
  ASSERT_EQ(1, passes);

  /* Dark Gray on Mid Gray (fails) */
  ASSERT_EQ(CMP_SUCCESS, cmp_color_verify_contrast_ratio(0x505050FF, 0x909090FF,
                                                         0, &passes));
  ASSERT_EQ(0, passes);

  /* Light-Gray on White (fails small text 4.5, passes large text 3.0) */
  /* #767676 on #FFFFFF gives 4.54 which is exactly a pass for 4.5.
     Let's use #808080 which is ~3.9:1 */
  ASSERT_EQ(CMP_SUCCESS, cmp_color_verify_contrast_ratio(0x808080FF, 0xFFFFFFFF,
                                                         0, &passes));
  ASSERT_EQ(0, passes);

  ASSERT_EQ(CMP_SUCCESS, cmp_color_verify_contrast_ratio(0x808080FF, 0xFFFFFFFF,
                                                         1, &passes));
  ASSERT_EQ(1, passes);

  PASS();
}

TEST test_null_args(void) {
  cmp_dynamic_type_t *dt = NULL;
  cmp_a11y_bold_text_t *bt = NULL;
  cmp_a11y_button_shapes_t *bs = NULL;
  cmp_a11y_increase_contrast_t *ic = NULL;
  cmp_a11y_hover_text_t *ht = NULL;
  cmp_a11y_autoplay_avoidance_t *ap = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dynamic_type_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_a11y_bold_text_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_a11y_button_shapes_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_a11y_increase_contrast_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_a11y_hover_text_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_a11y_autoplay_avoidance_create(NULL));

  cmp_dynamic_type_create(&dt);
  cmp_a11y_bold_text_create(&bt);
  cmp_a11y_button_shapes_create(&bs);
  cmp_a11y_increase_contrast_create(&ic);
  cmp_a11y_hover_text_create(&ht);
  cmp_a11y_autoplay_avoidance_create(&ap);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_dynamic_type_apply_scale(NULL, 10, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dynamic_type_should_reflow(NULL, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_dynamic_type_set_category(NULL, 0));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_a11y_bold_text_apply(NULL, 0, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_a11y_bold_text_set(NULL, 0));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_a11y_button_shapes_should_draw(NULL, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_a11y_button_shapes_set(NULL, 0));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_a11y_increase_contrast_apply(NULL, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_a11y_increase_contrast_set(NULL, 0));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_a11y_hover_text_get_bubble(NULL, 0, NULL, 0));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_color_verify_contrast_ratio(0, 0, 0, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_a11y_autoplay_avoidance_set(NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_a11y_autoplay_should_play(NULL, NULL));

  cmp_dynamic_type_destroy(dt);
  cmp_a11y_bold_text_destroy(bt);
  cmp_a11y_button_shapes_destroy(bs);
  cmp_a11y_increase_contrast_destroy(ic);
  cmp_a11y_hover_text_destroy(ht);
  cmp_a11y_autoplay_avoidance_destroy(ap);

  PASS();
}

TEST test_autoplay_avoidance(void) {
  cmp_a11y_autoplay_avoidance_t *ctx = NULL;
  int should_play;

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_autoplay_avoidance_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_autoplay_should_play(ctx, &should_play));
  ASSERT_EQ(1, should_play);

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_autoplay_avoidance_set(ctx, 1));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_autoplay_should_play(ctx, &should_play));
  ASSERT_EQ(0, should_play);

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_autoplay_avoidance_destroy(ctx));
  PASS();
}

SUITE(dynamic_type_suite) {
  RUN_TEST(test_dynamic_type);
  RUN_TEST(test_bold_text);
  RUN_TEST(test_button_shapes);
  RUN_TEST(test_increase_contrast);
  RUN_TEST(test_hover_text);
  RUN_TEST(test_contrast_ratio);
  RUN_TEST(test_autoplay_avoidance);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(dynamic_type_suite);
  GREATEST_MAIN_END();
}
