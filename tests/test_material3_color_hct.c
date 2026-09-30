/**
 * @file test_material3_color_hct.c
 * @brief Accuracy and round-trip conversion tests for Material 3 HCT / CAM16
 * color engine.
 */

/* clang-format off */
#include "greatest.h"
#include "material3/md3_color.h"
#include "ui_arena.h"
#include "ui_design_tokens.h"
#include "ui_error.h"
#include <math.h>
#include <string.h>
/* clang-format on */

TEST test_md3_color_scheme_generation(void) {
  ui_color_t seed = 0xFF6750A4; /* M3 Baseline Primary seed */
  struct md3_color_scheme scheme_light;
  struct md3_color_scheme scheme_dark;
  ui_color_t harmonized = 0;
  ui_error_t rc;

  memset(&scheme_light, 0, sizeof(scheme_light));
  memset(&scheme_dark, 0, sizeof(scheme_dark));

  rc = md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                               &scheme_light);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scheme_light.primary != 0);
  ASSERT(scheme_light.surface_container != 0);

  rc = md3_color_scheme_create(seed, 1, MD3_PALETTE_MODE_EXPRESSIVE,
                               &scheme_dark);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scheme_dark.primary != 0);
  ASSERT(scheme_dark.surface_container != 0);

  /* Test harmonization */
  rc = md3_color_harmonize(0xFFFF0000, seed, &harmonized);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(harmonized != 0);

  /* Null checks */
  rc = md3_color_harmonize(0xFFFF0000, seed, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Harmonization angle branches */
  /* diff > 180 degrees -> diff -= 360.0f */
  /* rotation > 15 -> clamp to 15 */
  rc = md3_color_harmonize(0xFFFF0000, 0xFF00FF00, &harmonized);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(harmonized != 0);

  /* rotation < -15 -> clamp to -15 */
  rc = md3_color_harmonize(0xFF00FF00, 0xFFFF0000, &harmonized);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(harmonized != 0);

  /* small rotation within [-15, 15] */
  rc = md3_color_harmonize(0xFF6750A4, 0xFF6850A0, &harmonized);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(harmonized != 0);

  PASS();
}

TEST test_md3_color_palette_modes(void) {
  ui_color_t seed = 0xFF00FF00;
  ui_color_t high_chroma_seed = 0xFFFF0000;
  ui_color_t low_chroma_seed = 0xFF808080;
  struct md3_color_scheme scheme;
  ui_error_t rc;

  /* Vibrant with high and low chroma seeds */
  rc = md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_VIBRANT, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_color_scheme_create(high_chroma_seed, 0, MD3_PALETTE_MODE_VIBRANT,
                               &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_color_scheme_create(low_chroma_seed, 0, MD3_PALETTE_MODE_VIBRANT,
                               &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Monochrome */
  rc = md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_MONOCHROME, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Fruit Salad */
  rc = md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_FRUIT_SALAD, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Rainbow */
  rc = md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_RAINBOW, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Content based */
  rc =
      md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_CONTENT_BASED, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Expressive */
  rc = md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_EXPRESSIVE, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tonal spot with chroma > 48 and chroma <= 48 */
  rc = md3_color_scheme_create(high_chroma_seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                               &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_color_scheme_create(low_chroma_seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                               &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Default / fallback mode */
  rc = md3_color_scheme_create(seed, 0, (enum md3_palette_mode)999, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_color_contrast_levels(void) {
  ui_color_t seed = 0xFF6750A4;
  struct md3_color_scheme scheme;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_color_scheme_create_contrast(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_NORMAL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_scheme_create_contrast(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                                        (enum md3_contrast_level) - 1, &scheme);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_scheme_create_contrast(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                                        (enum md3_contrast_level)3, &scheme);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Light theme: NORMAL, MEDIUM, HIGH */
  rc = md3_color_scheme_create_contrast(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_NORMAL, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_color_scheme_create_contrast(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_MEDIUM, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_color_scheme_create_contrast(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_HIGH, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Dark theme: NORMAL, MEDIUM, HIGH */
  rc = md3_color_scheme_create_contrast(seed, 1, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_NORMAL, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_color_scheme_create_contrast(seed, 1, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_MEDIUM, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_color_scheme_create_contrast(seed, 1, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_HIGH, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_color_scheme_tokens(void) {
  ui_color_t seed = 0xFF6750A4;
  struct md3_color_scheme scheme;
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  ui_error_t rc;

  rc = md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md3_color_scheme_apply_tokens(NULL, &dict);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_scheme_apply_tokens(&scheme, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Apply tokens successfully */
  rc = ui_arena_create(64 * 1024, &arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_design_token_dict_init(arena, &dict);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_color_scheme_apply_tokens(&scheme, &dict);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(dict.count >= 35);

  rc = ui_arena_destroy(arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_color_scheme_tokens_errors(void) {
  ui_color_t seed = 0xFF6750A4;
  struct md3_color_scheme scheme;
  struct ui_design_token_dict dict_fail;
  struct ui_design_token tokens_storage[16];
  ui_error_t rc;

  rc = md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT, &scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test error branch: dict with tokens storage but arena == NULL causes
   * ui_design_token_set_color to fail */
  memset(&dict_fail, 0, sizeof(dict_fail));
  dict_fail.tokens = tokens_storage;
  dict_fail.capacity = 16;
  dict_fail.count = 0;
  dict_fail.arena = NULL;
  rc = md3_color_scheme_apply_tokens(&scheme, &dict_fail);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_md3_color_mock_fail;
#endif

TEST test_md3_color_mock_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
  ui_color_t seed = 0xFF6750A4;
  struct md3_color_scheme scheme;
  ui_color_t harmonized = 0;
  ui_error_t rc;

  /* Mock failure 1: first ui_color_argb_to_hct fails */
  g_md3_color_mock_fail = 1;
  rc = md3_color_harmonize(0xFFFF0000, seed, &harmonized);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  rc = md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT, &scheme);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock failure 2: second ui_color_argb_to_hct fails */
  g_md3_color_mock_fail = 2;
  rc = md3_color_harmonize(0xFFFF0000, seed, &harmonized);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_color_mock_fail = 0;

  /* Mock failure 3: palette_tone / ui_tonal_palette_get_tone fails */
  g_md3_color_mock_fail = 3;
  rc = md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT, &scheme);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  g_md3_color_mock_fail = 0;
#endif
  PASS();
}

SUITE(md3_color_hct_suite) {
  RUN_TEST(test_md3_color_scheme_generation);
  RUN_TEST(test_md3_color_palette_modes);
  RUN_TEST(test_md3_color_contrast_levels);
  RUN_TEST(test_md3_color_scheme_tokens);
  RUN_TEST(test_md3_color_scheme_tokens_errors);
  RUN_TEST(test_md3_color_mock_failures);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_color_hct_suite);
  GREATEST_MAIN_END();
}
