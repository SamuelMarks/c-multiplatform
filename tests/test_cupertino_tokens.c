/**
 * @file test_cupertino_tokens.c
 * @brief Unit tests for Cupertino design tokens and dynamic colors.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_tokens.h"
/* clang-format on */

SUITE(cupertino_tokens_suite);

TEST test_cupertino_system_colors(void) {
  ui_color_t col = 0;
  ui_error_t rc;

  /* Null checks */
  rc = cupertino_get_system_color(CUPERTINO_COLOR_BLUE, 0, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range */
  rc =
      cupertino_get_system_color((enum cupertino_system_color) - 1, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_get_system_color(CUPERTINO_COLOR_COUNT, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_get_system_color(CUPERTINO_COLOR_COUNT, 1, 0, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_get_system_color(CUPERTINO_COLOR_COUNT, 0, 1, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Light colors */
  rc = cupertino_get_system_color(CUPERTINO_COLOR_BLUE, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x00, UI_COLOR_RED(col));
  ASSERT_EQ(0x7A, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xFF, UI_COLOR_BLUE(col));
  ASSERT_EQ(255, UI_COLOR_ALPHA(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_GREEN, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x34, UI_COLOR_RED(col));
  ASSERT_EQ(0xC7, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x59, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_INDIGO, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x58, UI_COLOR_RED(col));
  ASSERT_EQ(0x56, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xD6, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_ORANGE, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xFF, UI_COLOR_RED(col));
  ASSERT_EQ(0x95, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x00, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_PINK, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xFF, UI_COLOR_RED(col));
  ASSERT_EQ(0x2D, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x55, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_PURPLE, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xAF, UI_COLOR_RED(col));
  ASSERT_EQ(0x52, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xDE, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_RED, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xFF, UI_COLOR_RED(col));
  ASSERT_EQ(0x3B, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x30, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_TEAL, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x59, UI_COLOR_RED(col));
  ASSERT_EQ(0xAD, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xC4, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_YELLOW, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xFF, UI_COLOR_RED(col));
  ASSERT_EQ(0xCC, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x00, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_MINT, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x00, UI_COLOR_RED(col));
  ASSERT_EQ(0xC7, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xBE, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_CYAN, 0, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x32, UI_COLOR_RED(col));
  ASSERT_EQ(0xAD, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xE6, UI_COLOR_BLUE(col));

  /* Dark colors */
  rc = cupertino_get_system_color(CUPERTINO_COLOR_BLUE, 1, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x0A, UI_COLOR_RED(col));
  ASSERT_EQ(0x84, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xFF, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_GREEN, 1, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x30, UI_COLOR_RED(col));
  ASSERT_EQ(0xD1, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x58, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_INDIGO, 1, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x5E, UI_COLOR_RED(col));
  ASSERT_EQ(0x5C, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xE6, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_ORANGE, 1, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xFF, UI_COLOR_RED(col));
  ASSERT_EQ(0x9F, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x0A, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_PINK, 1, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xFF, UI_COLOR_RED(col));
  ASSERT_EQ(0x37, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x5F, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_PURPLE, 1, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xBF, UI_COLOR_RED(col));
  ASSERT_EQ(0x5A, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xF2, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_RED, 1, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xFF, UI_COLOR_RED(col));
  ASSERT_EQ(0x45, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x3A, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_TEAL, 1, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x64, UI_COLOR_RED(col));
  ASSERT_EQ(0xD2, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xFF, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_YELLOW, 1, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xFF, UI_COLOR_RED(col));
  ASSERT_EQ(0xD6, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x0A, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_MINT, 1, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x63, UI_COLOR_RED(col));
  ASSERT_EQ(0xE6, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xE2, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_CYAN, 1, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x64, UI_COLOR_RED(col));
  ASSERT_EQ(0xD2, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xFF, UI_COLOR_BLUE(col));

  /* High contrast colors */
  rc = cupertino_get_system_color(CUPERTINO_COLOR_BLUE, 0, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x00, UI_COLOR_RED(col));
  ASSERT_EQ(0x40, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xDD, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_GREEN, 0, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x24, UI_COLOR_RED(col));
  ASSERT_EQ(0x8A, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x3D, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_INDIGO, 0, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x36, UI_COLOR_RED(col));
  ASSERT_EQ(0x34, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xA3, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_ORANGE, 0, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xC9, UI_COLOR_RED(col));
  ASSERT_EQ(0x34, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x00, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_PINK, 0, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xD3, UI_COLOR_RED(col));
  ASSERT_EQ(0x0F, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x45, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_PURPLE, 0, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x89, UI_COLOR_RED(col));
  ASSERT_EQ(0x44, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xAB, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_RED, 0, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xD7, UI_COLOR_RED(col));
  ASSERT_EQ(0x00, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x15, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_TEAL, 0, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x00, UI_COLOR_RED(col));
  ASSERT_EQ(0x71, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xA4, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_YELLOW, 0, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xB2, UI_COLOR_RED(col));
  ASSERT_EQ(0x50, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x00, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_MINT, 0, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x0C, UI_COLOR_RED(col));
  ASSERT_EQ(0x81, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x7B, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_color(CUPERTINO_COLOR_CYAN, 0, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x00, UI_COLOR_RED(col));
  ASSERT_EQ(0x71, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xA4, UI_COLOR_BLUE(col));

  PASS();
}

TEST test_cupertino_system_grays(void) {
  ui_color_t col = 0;
  ui_error_t rc;

  /* Invalid args */
  rc = cupertino_get_system_gray(CUPERTINO_GRAY_1, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_get_system_gray((enum cupertino_gray_level) - 1, 0, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_get_system_gray(CUPERTINO_GRAY_COUNT, 0, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_get_system_gray(CUPERTINO_GRAY_COUNT, 1, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Light grays 1 through 6 */
  rc = cupertino_get_system_gray(CUPERTINO_GRAY_1, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x8E, UI_COLOR_RED(col));
  ASSERT_EQ(0x8E, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x93, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_gray(CUPERTINO_GRAY_2, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xAE, UI_COLOR_RED(col));
  ASSERT_EQ(0xAE, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xB2, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_gray(CUPERTINO_GRAY_3, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xC7, UI_COLOR_RED(col));
  ASSERT_EQ(0xC7, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xCC, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_gray(CUPERTINO_GRAY_4, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xD1, UI_COLOR_RED(col));
  ASSERT_EQ(0xD1, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xD6, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_gray(CUPERTINO_GRAY_5, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xE5, UI_COLOR_RED(col));
  ASSERT_EQ(0xE5, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xEA, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_gray(CUPERTINO_GRAY_6, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xF2, UI_COLOR_RED(col));
  ASSERT_EQ(0xF2, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xF7, UI_COLOR_BLUE(col));

  /* Dark grays 1 through 6 */
  rc = cupertino_get_system_gray(CUPERTINO_GRAY_1, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x8E, UI_COLOR_RED(col));
  ASSERT_EQ(0x8E, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x93, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_gray(CUPERTINO_GRAY_2, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x63, UI_COLOR_RED(col));
  ASSERT_EQ(0x63, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x66, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_gray(CUPERTINO_GRAY_3, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x48, UI_COLOR_RED(col));
  ASSERT_EQ(0x48, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x4A, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_gray(CUPERTINO_GRAY_4, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x3A, UI_COLOR_RED(col));
  ASSERT_EQ(0x3A, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x3C, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_gray(CUPERTINO_GRAY_5, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x2C, UI_COLOR_RED(col));
  ASSERT_EQ(0x2C, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x2E, UI_COLOR_BLUE(col));

  rc = cupertino_get_system_gray(CUPERTINO_GRAY_6, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x1C, UI_COLOR_RED(col));
  ASSERT_EQ(0x1C, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x1E, UI_COLOR_BLUE(col));

  PASS();
}

TEST test_cupertino_surfaces_labels_named(void) {
  ui_color_t col = 0;
  ui_error_t rc;

  /* Surfaces null checks */
  rc = cupertino_get_surface_color(CUPERTINO_SURFACE_BACKGROUND, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_get_surface_color((enum cupertino_surface_level) - 1, 0, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_get_surface_color(CUPERTINO_SURFACE_COUNT, 0, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_get_surface_color(CUPERTINO_SURFACE_COUNT, 1, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Light surfaces */
  rc = cupertino_get_surface_color(CUPERTINO_SURFACE_BACKGROUND, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xFF, UI_COLOR_RED(col));
  ASSERT_EQ(0xFF, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xFF, UI_COLOR_BLUE(col));

  rc = cupertino_get_surface_color(CUPERTINO_SURFACE_SECONDARY_BACKGROUND, 0,
                                   &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xF2, UI_COLOR_RED(col));
  ASSERT_EQ(0xF2, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xF7, UI_COLOR_BLUE(col));

  rc = cupertino_get_surface_color(CUPERTINO_SURFACE_TERTIARY_BACKGROUND, 0,
                                   &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xFF, UI_COLOR_RED(col));
  ASSERT_EQ(0xFF, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xFF, UI_COLOR_BLUE(col));

  rc = cupertino_get_surface_color(CUPERTINO_SURFACE_GROUPED_BACKGROUND, 0,
                                   &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xF2, UI_COLOR_RED(col));
  ASSERT_EQ(0xF2, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xF7, UI_COLOR_BLUE(col));

  rc = cupertino_get_surface_color(
      CUPERTINO_SURFACE_SECONDARY_GROUPED_BACKGROUND, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xFF, UI_COLOR_RED(col));
  ASSERT_EQ(0xFF, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xFF, UI_COLOR_BLUE(col));

  rc = cupertino_get_surface_color(
      CUPERTINO_SURFACE_TERTIARY_GROUPED_BACKGROUND, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0xF2, UI_COLOR_RED(col));
  ASSERT_EQ(0xF2, UI_COLOR_GREEN(col));
  ASSERT_EQ(0xF7, UI_COLOR_BLUE(col));

  /* Dark surfaces */
  rc = cupertino_get_surface_color(CUPERTINO_SURFACE_BACKGROUND, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x00, UI_COLOR_RED(col));
  ASSERT_EQ(0x00, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x00, UI_COLOR_BLUE(col));

  rc = cupertino_get_surface_color(CUPERTINO_SURFACE_SECONDARY_BACKGROUND, 1,
                                   &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x1C, UI_COLOR_RED(col));
  ASSERT_EQ(0x1C, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x1E, UI_COLOR_BLUE(col));

  rc = cupertino_get_surface_color(CUPERTINO_SURFACE_TERTIARY_BACKGROUND, 1,
                                   &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x2C, UI_COLOR_RED(col));
  ASSERT_EQ(0x2C, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x2E, UI_COLOR_BLUE(col));

  rc = cupertino_get_surface_color(CUPERTINO_SURFACE_GROUPED_BACKGROUND, 1,
                                   &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x00, UI_COLOR_RED(col));
  ASSERT_EQ(0x00, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x00, UI_COLOR_BLUE(col));

  rc = cupertino_get_surface_color(
      CUPERTINO_SURFACE_SECONDARY_GROUPED_BACKGROUND, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x1C, UI_COLOR_RED(col));
  ASSERT_EQ(0x1C, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x1E, UI_COLOR_BLUE(col));

  rc = cupertino_get_surface_color(
      CUPERTINO_SURFACE_TERTIARY_GROUPED_BACKGROUND, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0x2C, UI_COLOR_RED(col));
  ASSERT_EQ(0x2C, UI_COLOR_GREEN(col));
  ASSERT_EQ(0x2E, UI_COLOR_BLUE(col));

  /* Labels null checks */
  rc = cupertino_get_label_color(CUPERTINO_LABEL_PRIMARY, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_get_label_color((enum cupertino_label_level) - 1, 0, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_get_label_color(CUPERTINO_LABEL_COUNT, 0, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Light labels */
  rc = cupertino_get_label_color(CUPERTINO_LABEL_PRIMARY, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, UI_COLOR_RED(col));
  ASSERT_EQ(255, UI_COLOR_ALPHA(col));

  rc = cupertino_get_label_color(CUPERTINO_LABEL_SECONDARY, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(153, UI_COLOR_ALPHA(col));

  rc = cupertino_get_label_color(CUPERTINO_LABEL_TERTIARY, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(77, UI_COLOR_ALPHA(col));

  rc = cupertino_get_label_color(CUPERTINO_LABEL_QUATERNARY, 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(46, UI_COLOR_ALPHA(col));

  /* Dark labels */
  rc = cupertino_get_label_color(CUPERTINO_LABEL_PRIMARY, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(255, UI_COLOR_RED(col));
  ASSERT_EQ(255, UI_COLOR_ALPHA(col));

  rc = cupertino_get_label_color(CUPERTINO_LABEL_SECONDARY, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(153, UI_COLOR_ALPHA(col));

  rc = cupertino_get_label_color(CUPERTINO_LABEL_TERTIARY, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(77, UI_COLOR_ALPHA(col));

  rc = cupertino_get_label_color(CUPERTINO_LABEL_QUATERNARY, 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(46, UI_COLOR_ALPHA(col));

  /* Named colors null & invalid */
  rc = cupertino_resolve_named_color(NULL, 0, &col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_resolve_named_color("--apple-separator", 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_resolve_named_color("unknown-color", 0, &col);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  /* Named colors light & dark */
  rc = cupertino_resolve_named_color("--apple-separator", 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_resolve_named_color("--apple-separator", 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_resolve_named_color("--apple-opaque-separator", 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_resolve_named_color("--apple-opaque-separator", 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_resolve_named_color("--apple-link", 0, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_resolve_named_color("--apple-link", 1, &col);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(cupertino_tokens_suite) {
  RUN_TEST(test_cupertino_system_colors);
  RUN_TEST(test_cupertino_system_grays);
  RUN_TEST(test_cupertino_surfaces_labels_named);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_tokens_suite);
  GREATEST_MAIN_END();
}
