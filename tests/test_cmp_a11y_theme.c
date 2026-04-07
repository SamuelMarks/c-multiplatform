/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_cmp_a11y_theme_init_cleanup(void) {
  ASSERT_EQ(0, cmp_a11y_theme_init());
  ASSERT_EQ(0, cmp_a11y_theme_cleanup());
  PASS();
}

TEST test_cmp_a11y_detect_high_contrast(void) {
  int is_hc = cmp_a11y_detect_high_contrast();
  /* It can be 0 or 1 depending on the host machine. We just ensure it runs. */
  ASSERT(is_hc == 0 || is_hc == 1);
  PASS();
}

TEST test_cmp_a11y_build_theme_null(void) {
  ASSERT_EQ(-1, cmp_a11y_build_theme(CMP_COLOR_BLIND_NONE, NULL));
  PASS();
}

TEST test_cmp_a11y_build_theme_regular(void) {
  cmp_a11y_theme_t theme;
  memset(&theme, 0, sizeof(theme));
  ASSERT_EQ(0, cmp_a11y_build_theme(CMP_COLOR_BLIND_NONE, &theme));

  if (!theme.is_high_contrast) {
    /* Check default non-hc colors */
    ASSERT_EQ_FMT(0.1f, theme.background.r, "%f");
    ASSERT_EQ_FMT(0.1f, theme.background.g, "%f");
    ASSERT_EQ_FMT(0.12f, theme.background.b, "%f");

    ASSERT_EQ_FMT(0.0f, theme.primary_accent.r, "%f");
    ASSERT_EQ_FMT(0.47f, theme.primary_accent.g, "%f");
    ASSERT_EQ_FMT(1.0f, theme.primary_accent.b, "%f");

    ASSERT_EQ_FMT(1.0f, theme.error_text.r, "%f");
    ASSERT_EQ_FMT(0.23f, theme.error_text.g, "%f");
    ASSERT_EQ_FMT(0.18f, theme.error_text.b, "%f");
  } else {
    /* High contrast mode check */
    ASSERT_EQ_FMT(0.0f, theme.background.r, "%f");
    ASSERT_EQ_FMT(1.0f, theme.primary_accent.r, "%f");
    ASSERT_EQ_FMT(1.0f, theme.error_text.r, "%f");
  }
  PASS();
}

TEST test_cmp_a11y_build_theme_protanopia(void) {
  cmp_a11y_theme_t theme;
  memset(&theme, 0, sizeof(theme));
  ASSERT_EQ(0, cmp_a11y_build_theme(CMP_COLOR_BLIND_PROTANOPIA, &theme));

  if (!theme.is_high_contrast) {
    ASSERT_EQ_FMT(0.0f, theme.primary_accent.r, "%f");
    ASSERT_EQ_FMT(0.33f, theme.primary_accent.g, "%f");
    ASSERT_EQ_FMT(0.88f, theme.primary_accent.b, "%f");
  }
  PASS();
}

TEST test_cmp_a11y_build_theme_tritanopia(void) {
  cmp_a11y_theme_t theme;
  memset(&theme, 0, sizeof(theme));
  ASSERT_EQ(0, cmp_a11y_build_theme(CMP_COLOR_BLIND_TRITANOPIA, &theme));

  if (!theme.is_high_contrast) {
    ASSERT_EQ_FMT(0.85f, theme.primary_accent.r, "%f");
    ASSERT_EQ_FMT(0.11f, theme.primary_accent.g, "%f");
    ASSERT_EQ_FMT(0.38f, theme.primary_accent.b, "%f");
  }
  PASS();
}

/* Add definitions that need to be in the main runner */
SUITE(cmp_a11y_theme_suite) {
  RUN_TEST(test_cmp_a11y_theme_init_cleanup);
  RUN_TEST(test_cmp_a11y_detect_high_contrast);
  RUN_TEST(test_cmp_a11y_build_theme_null);
  RUN_TEST(test_cmp_a11y_build_theme_regular);
  RUN_TEST(test_cmp_a11y_build_theme_protanopia);
  RUN_TEST(test_cmp_a11y_build_theme_tritanopia);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_a11y_theme_suite);
  GREATEST_MAIN_END();
}
