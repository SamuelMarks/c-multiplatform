/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_button_dimming_and_styles(void) {
  cmp_button_t *ctx = NULL;
  float opacity = 0.0f;
  float r, g, b, a;

  ASSERT_EQ(CMP_SUCCESS, cmp_button_create(&ctx));

  /* HIG Plain Button - instant dim to 30% */
  ASSERT_EQ(CMP_SUCCESS, cmp_button_set_style(ctx, CMP_BUTTON_STYLE_PLAIN));
  ASSERT_EQ(CMP_SUCCESS, cmp_button_handle_press(ctx, 1, &opacity));
  ASSERT_EQ(0.3f, opacity);

  /* Reset press */
  ASSERT_EQ(CMP_SUCCESS, cmp_button_handle_press(ctx, 0, &opacity));
  ASSERT_EQ(1.0f, opacity);

  /* Filled button gets subtle highlight (approx 80%) instead of full drop */
  ASSERT_EQ(CMP_SUCCESS, cmp_button_set_style(ctx, CMP_BUTTON_STYLE_FILLED));
  ASSERT_EQ(CMP_SUCCESS, cmp_button_handle_press(ctx, 1, &opacity));
  ASSERT_EQ(0.8f, opacity);

  /* Test Tinted Background */
  ASSERT_EQ(CMP_SUCCESS, cmp_button_set_style(ctx, CMP_BUTTON_STYLE_TINTED));
  ASSERT_EQ(CMP_SUCCESS, cmp_button_resolve_background(ctx, &r, &g, &b, &a));
  ASSERT_EQ(0.15f, a); /* Tinted background is 15-20% */

  /* Gray Button */
  ASSERT_EQ(CMP_SUCCESS, cmp_button_set_style(ctx, CMP_BUTTON_STYLE_GRAY));
  ASSERT_EQ(CMP_SUCCESS, cmp_button_resolve_background(ctx, &r, &g, &b, &a));
  ASSERT_EQ(0.2f, a);

  /* Close Button */
  ASSERT_EQ(CMP_SUCCESS, cmp_button_set_style(ctx, CMP_BUTTON_STYLE_CLOSE));
  ASSERT_EQ(CMP_SUCCESS, cmp_button_resolve_background(ctx, &r, &g, &b, &a));
  ASSERT_EQ(0.3f, a);

  ASSERT_EQ(CMP_SUCCESS, cmp_button_destroy(ctx));
  PASS();
}

TEST test_toggle_switch(void) {
  cmp_toggle_t *ctx = NULL;
  float r, g, b, a, x;

  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_create(&ctx));

  /* Initial OFF State */
  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_resolve_visuals(ctx, &r, &g, &b, &a, &x));
  ASSERT_EQ(0.3f, a); /* Translucent gray */
  ASSERT_EQ(2.0f, x); /* Left alignment */

  /* Toggle ON State */
  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_set_state(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_resolve_visuals(ctx, &r, &g, &b, &a, &x));
  ASSERT_EQ(1.0f, a); /* Solid green */
  ASSERT_GT(x, 2.0f); /* Right alignment offset */

  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_destroy(ctx));
  PASS();
}

TEST test_null_args(void) {
  cmp_button_t *btn = NULL;
  cmp_toggle_t *tgl = NULL;
  float f;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_button_create(NULL));
  cmp_button_create(&btn);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_button_set_style(NULL, CMP_BUTTON_STYLE_PLAIN));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_button_handle_press(NULL, 1, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_button_handle_press(btn, 1, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_button_resolve_background(NULL, &f, &f, &f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_button_resolve_background(btn, NULL, &f, &f, &f));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_toggle_create(NULL));
  cmp_toggle_create(&tgl);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_toggle_set_state(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_toggle_resolve_visuals(NULL, &f, &f, &f, &f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_toggle_resolve_visuals(tgl, NULL, &f, &f, &f, &f));

  cmp_button_destroy(btn);
  cmp_toggle_destroy(tgl);
  PASS();
}

SUITE(system_button_suite) {
  RUN_TEST(test_button_dimming_and_styles);
  RUN_TEST(test_toggle_switch);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(system_button_suite);
  GREATEST_MAIN_END();
}
