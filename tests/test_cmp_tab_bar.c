/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_tab_bar_lifecycle(void) {
  cmp_tab_bar_t *ctx = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_tab_bar_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_tab_bar_add_tab(ctx, "Home", "house.fill", "/home"));
  ASSERT_EQ(CMP_SUCCESS, cmp_tab_bar_add_tab(ctx, "Settings", "gearshape.fill",
                                             "/settings"));

  /* Set badges */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_tab_bar_set_badge(ctx, 1, 3)); /* 3 notifications on Settings */

  /* Bad index */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tab_bar_set_badge(ctx, 99, 1));

  ASSERT_EQ(CMP_SUCCESS, cmp_tab_bar_destroy(ctx));
  PASS();
}

TEST test_tab_layout_resolution(void) {
  cmp_tab_bar_t *ctx = NULL;
  cmp_tab_bar_placement_t placement;
  cmp_macos_material_t material;
  int is_bottom_placed;

  ASSERT_EQ(CMP_SUCCESS, cmp_tab_bar_create(&ctx));

  /* iPhone / Compact Width */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_tab_bar_resolve_layout(ctx, 375.0f, &placement, &material));
  ASSERT_EQ(CMP_TAB_BAR_PLACEMENT_BOTTOM, placement);
  ASSERT_EQ(CMP_MACOS_MATERIAL_WINDOW_BACKGROUND, material);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_toolbar_resolve_placement(375.0f, &is_bottom_placed));
  ASSERT_EQ(1, is_bottom_placed);

  /* iPad / Mac Width */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_tab_bar_resolve_layout(ctx, 1024.0f, &placement, &material));
  ASSERT_EQ(CMP_TAB_BAR_PLACEMENT_LEADING, placement);   /* Becomes Sidebar */
  ASSERT_EQ(CMP_MACOS_MATERIAL_BEHIND_WINDOW, material); /* Special deep blur */

  ASSERT_EQ(CMP_SUCCESS,
            cmp_toolbar_resolve_placement(1024.0f, &is_bottom_placed));
  ASSERT_EQ(0, is_bottom_placed); /* Moves up into navigation bar */

  ASSERT_EQ(CMP_SUCCESS, cmp_tab_bar_destroy(ctx));
  PASS();
}

TEST test_null_args(void) {
  cmp_tab_bar_t *ctx = NULL;
  cmp_tab_bar_placement_t p;
  cmp_macos_material_t m;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tab_bar_create(NULL));
  cmp_tab_bar_create(&ctx);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tab_bar_add_tab(NULL, "T", "I", "R"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tab_bar_add_tab(ctx, NULL, "I", "R"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tab_bar_add_tab(ctx, "T", NULL, "R"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tab_bar_add_tab(ctx, "T", "I", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tab_bar_set_badge(NULL, 0, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_tab_bar_resolve_layout(NULL, 300.0f, &p, &m));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_tab_bar_resolve_layout(ctx, 300.0f, NULL, &m));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_tab_bar_resolve_layout(ctx, 300.0f, &p, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_toolbar_resolve_placement(300.0f, NULL));

  cmp_tab_bar_destroy(ctx);
  PASS();
}

SUITE(tab_bar_suite) {
  RUN_TEST(test_tab_bar_lifecycle);
  RUN_TEST(test_tab_layout_resolution);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(tab_bar_suite);
  GREATEST_MAIN_END();
}
