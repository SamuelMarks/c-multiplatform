/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_nav_bar_scroll_collapse(void) {
  cmp_nav_bar_t *ctx = NULL;
  float scale, y_off;

  ASSERT_EQ(CMP_SUCCESS, cmp_nav_bar_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_nav_bar_set_prefers_large_titles(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_nav_bar_set_appearance(
                             ctx, CMP_NAV_BAR_APPEARANCE_SCROLL_EDGE));

  /* Rest */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_nav_bar_calculate_scroll_collapse(ctx, 0.0f, &scale, &y_off));
  ASSERT_EQ(1.0f, scale);
  ASSERT_EQ(0.0f, y_off);

  /* Mid */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_nav_bar_calculate_scroll_collapse(ctx, 25.0f, &scale, &y_off));
  ASSERT_EQ(0.8f, scale);
  ASSERT_EQ(-20.0f, y_off);

  /* Fully Collapsed */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_nav_bar_calculate_scroll_collapse(ctx, 100.0f, &scale, &y_off));
  ASSERT_EQ(0.6f, scale);
  ASSERT_EQ(-40.0f, y_off);

  /* Disabled Large Title */
  ASSERT_EQ(CMP_SUCCESS, cmp_nav_bar_set_prefers_large_titles(ctx, 0));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_nav_bar_calculate_scroll_collapse(ctx, 50.0f, &scale, &y_off));
  ASSERT_EQ(1.0f, scale);
  ASSERT_EQ(0.0f, y_off);

  ASSERT_EQ(CMP_SUCCESS, cmp_nav_bar_destroy(ctx));
  PASS();
}

TEST test_nav_bar_back_button(void) {
  cmp_nav_bar_t *ctx = NULL;
  char label[128];
  float wt, pad;

  ASSERT_EQ(CMP_SUCCESS, cmp_nav_bar_create(&ctx));

  /* Exact metrics */
  ASSERT_EQ(CMP_SUCCESS, cmp_nav_bar_get_chevron_metrics(ctx, &wt, &pad));
  ASSERT_EQ(600.0f, wt);
  ASSERT_EQ(8.0f, pad);

  /* Truncation */
  ASSERT_EQ(CMP_SUCCESS, cmp_nav_bar_resolve_back_button_label(
                             ctx, "Settings", 200.0f, label, 128));
  ASSERT_STR_EQ("Settings", label);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_nav_bar_resolve_back_button_label(
                ctx,
                "A Very Long Super Ridiculous Deep View Controller Name That "
                "Defeats The Flow Of Information And Causes Issues",
                100.0f, label, 128));
  ASSERT_STR_EQ("Back", label); /* Automatically degrades */

  /* Null prev title */
  ASSERT_EQ(CMP_SUCCESS, cmp_nav_bar_resolve_back_button_label(
                             ctx, NULL, 100.0f, label, 128));
  ASSERT_STR_EQ("Back", label);

  ASSERT_EQ(CMP_SUCCESS, cmp_nav_bar_destroy(ctx));
  PASS();
}

TEST test_null_args(void) {
  cmp_nav_bar_t *ctx = NULL;
  float f;
  char buf[32];

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_nav_bar_create(NULL));
  cmp_nav_bar_create(&ctx);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_nav_bar_set_prefers_large_titles(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_nav_bar_set_appearance(NULL, CMP_NAV_BAR_APPEARANCE_STANDARD));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_nav_bar_calculate_scroll_collapse(NULL, 0.0f, &f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_nav_bar_calculate_scroll_collapse(ctx, 0.0f, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_nav_bar_calculate_scroll_collapse(ctx, 0.0f, &f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_nav_bar_resolve_back_button_label(
                                       ctx, "Title", 100.0f, NULL, 32));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_nav_bar_resolve_back_button_label(
                                       ctx, "Title", 100.0f, buf, 0));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_nav_bar_get_chevron_metrics(NULL, &f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_nav_bar_get_chevron_metrics(ctx, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_nav_bar_get_chevron_metrics(ctx, &f, NULL));

  cmp_nav_bar_destroy(ctx);
  PASS();
}

SUITE(nav_bar_suite) {
  RUN_TEST(test_nav_bar_scroll_collapse);
  RUN_TEST(test_nav_bar_back_button);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(nav_bar_suite);
  GREATEST_MAIN_END();
}
