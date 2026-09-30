/* clang-format off */
#include "greatest.h"
#include "material3/md3_color.h"
#include "material3/md3_elevation.h"
#include "material3/md3_shape.h"
#include "material3/md3_state_layer.h"
#include "material3/md3_typography.h"
#include "ui_arena.h"
#include "ui_design_tokens.h"
#include "ui_error.h"
#include "ui_font_manager.h"
/* clang-format on */

static const unsigned char tests_tiny_ttf[] = {
    0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x63, 0x6d, 0x61, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c,
    0x00, 0x00, 0x00, 0x14, 0x68, 0x65, 0x61, 0x64, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x36, 0x68, 0x68, 0x65, 0x61,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x00, 0x00, 0x00, 0x24,
    0x68, 0x6d, 0x74, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xea,
    0x00, 0x00, 0x00, 0x08, 0x67, 0x6c, 0x79, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xf2, 0x00, 0x00, 0x00, 0x01, 0x6c, 0x6f, 0x63, 0x61,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf3, 0x00, 0x00, 0x00, 0x04,
    0x6d, 0x61, 0x78, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf7,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00, 0x04,
    0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x41, 0xff, 0xff, 0x00, 0x00,
    0x00, 0x41, 0xff, 0xff, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5f, 0x0f, 0x3c, 0xf5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x03, 0xe8, 0xff, 0x9c, 0x00, 0x00, 0x03, 0xe8,
    0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0xe8, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00};

TEST test_md3_color_harmonization(void) {
  ui_color_t design = UI_COLOR_ARGB(255, 255, 0, 0); /* Red */
  ui_color_t source = UI_COLOR_ARGB(255, 0, 0, 255); /* Blue */
  ui_color_t harmonized = 0;
  ui_error_t rc;

  /* Invalid argument */
  rc = md3_color_harmonize(design, source, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid harmonization */
  rc = md3_color_harmonize(design, source, &harmonized);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(harmonized != 0);

  PASS();
}

TEST test_md3_color_schemes_all_modes(void) {
  ui_color_t seed = UI_COLOR_ARGB(255, 103, 80, 164); /* M3 Purple */
  struct md3_color_scheme scheme;
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  int mode;
  ui_error_t rc;

  /* Invalid argument */
  rc = md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = ui_arena_create(65536, &arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_design_token_dict_init(arena, &dict);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test every palette mode in both Light and Dark */
  for (mode = 0; mode <= (int)MD3_PALETTE_MODE_RAINBOW; mode++) {
    /* Light mode */
    rc = md3_color_scheme_create(seed, 0, (enum md3_palette_mode)mode, &scheme);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(scheme.primary != 0);
    ASSERT(scheme.surface != 0);
    ASSERT(scheme.surface_container_lowest != 0);
    ASSERT(scheme.surface_container_highest != 0);

    /* Apply tokens */
    rc = md3_color_scheme_apply_tokens(&scheme, &dict);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* Dark mode */
    rc = md3_color_scheme_create(seed, 1, (enum md3_palette_mode)mode, &scheme);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(scheme.primary != 0);
    ASSERT(scheme.surface != 0);
    ASSERT(scheme.surface_container != 0);
  }

  /* Test invalid arguments on apply_tokens */
  rc = md3_color_scheme_apply_tokens(NULL, &dict);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_color_scheme_apply_tokens(&scheme, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = ui_arena_destroy(arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_elevation(void) {
  ui_color_t surface = UI_COLOR_ARGB(255, 255, 255, 255);
  ui_color_t tint = UI_COLOR_ARGB(255, 103, 80, 164);
  ui_color_t result = 0;
  float dp = 0.0f;
  float opacity = 0.0f;
  int level;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_elevation_get_dp(-1, &dp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_elevation_get_dp(6, &dp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_elevation_get_dp(0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_elevation_get_tint_opacity(-1, &opacity);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_elevation_get_tint_opacity(6, &opacity);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_elevation_get_tint_opacity(0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_elevation_get_surface_color(surface, tint, -1, &result);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_elevation_get_surface_color(surface, tint, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid levels 0 through 5 */
  for (level = 0; level <= 5; level++) {
    rc = md3_elevation_get_dp(level, &dp);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = md3_elevation_get_tint_opacity(level, &opacity);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(opacity >= 0.0f && opacity <= 1.0f);

    rc = md3_elevation_get_surface_color(surface, tint, level, &result);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(result != 0);

    if (level == 0) {
      ASSERT_EQ(surface, result);
    }
  }

  PASS();
}

TEST test_md3_state_layer(void) {
  ui_color_t base = UI_COLOR_ARGB(255, 200, 200, 200);
  ui_color_t content = UI_COLOR_ARGB(255, 0, 0, 0);
  ui_color_t blended = 0;
  float opacity = 0.0f;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_state_layer_get_opacity((enum md3_state_layer_type)99, &opacity);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_state_layer_get_opacity(MD3_STATE_LAYER_HOVER, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_state_layer_blend(base, content, MD3_STATE_LAYER_HOVER, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_state_layer_blend(base, content, (enum md3_state_layer_type)99,
                             &blended);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Hover (8%) */
  rc = md3_state_layer_get_opacity(MD3_STATE_LAYER_HOVER, &opacity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.08f, opacity, 0.001f);
  rc = md3_state_layer_blend(base, content, MD3_STATE_LAYER_HOVER, &blended);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(blended != 0);

  /* Focus (10%) */
  rc = md3_state_layer_get_opacity(MD3_STATE_LAYER_FOCUS, &opacity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.10f, opacity, 0.001f);

  /* Pressed (10%) */
  rc = md3_state_layer_get_opacity(MD3_STATE_LAYER_PRESSED, &opacity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.10f, opacity, 0.001f);

  /* Dragged (16%) */
  rc = md3_state_layer_get_opacity(MD3_STATE_LAYER_DRAGGED, &opacity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.16f, opacity, 0.001f);
  rc = md3_state_layer_blend(base, content, MD3_STATE_LAYER_DRAGGED, &blended);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(blended != 0);

  PASS();
}

TEST test_md3_typography(void) {
  struct md3_type_style style;
  struct ui_arena *arena = NULL;
  struct ui_design_token_dict dict;
  int r;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_typography_get_style((enum md3_typescale_role) - 1, 0, &style);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_typography_get_style(
      (enum md3_typescale_role)MD3_TYPESCALE_ROLE_COUNT, 0, &style);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_typography_get_style(MD3_TYPESCALE_DISPLAY_LARGE, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_typography_apply_tokens(0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Validate all 15 styles in baseline and emphasized */
  for (r = 0; r < (int)MD3_TYPESCALE_ROLE_COUNT; r++) {
    rc = md3_typography_get_style((enum md3_typescale_role)r, 0, &style);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(style.size_sp > 0.0f);
    ASSERT(style.line_height_sp > 0.0f);

    /* Emphasized */
    rc = md3_typography_get_style((enum md3_typescale_role)r, 1, &style);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(style.weight >= 600);
  }

  rc = ui_arena_create(65536, &arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_design_token_dict_init(arena, &dict);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Apply baseline tokens */
  rc = md3_typography_apply_tokens(0, &dict);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(dict.count > 0);

  /* Apply baseline + expressive tokens */
  rc = md3_typography_apply_tokens(1, &dict);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_arena_destroy(arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test error branch in baseline token loop: dict with capacity but NULL arena
   */
  {
    struct ui_design_token_dict dict_fail;
    struct ui_design_token tokens_storage[16];
    memset(&dict_fail, 0, sizeof(dict_fail));
    dict_fail.tokens = tokens_storage;
    dict_fail.capacity = 16;
    dict_fail.count = 0;
    dict_fail.arena = NULL;
    rc = md3_typography_apply_tokens(0, &dict_fail);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  }

  /* Test error branch in expressive token loop */
  {
    struct ui_arena *arena_exp = NULL;
    struct ui_design_token_dict dict_exp;
    rc = ui_arena_create(65536, &arena_exp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_design_token_dict_init(arena_exp, &dict_exp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_typography_apply_tokens(0, &dict_exp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    dict_exp.arena = NULL;
    rc = md3_typography_apply_tokens(1, &dict_exp);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = ui_arena_destroy(arena_exp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  PASS();
}

TEST test_md3_shapes(void) {
  float radius = 0.0f;
  struct ui_arena *arena = NULL;
  struct ui_arena *arena_exp = NULL;
  struct ui_design_token_dict dict;
  struct ui_design_token_dict dict_fail;
  struct ui_design_token_dict dict_exp;
  struct ui_design_token tokens_storage[16];
  int s;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_shape_get_corner_radius((enum md3_shape_scale) - 1, &radius);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_shape_get_corner_radius((enum md3_shape_scale)MD3_SHAPE_SCALE_COUNT,
                                   &radius);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_shape_get_corner_radius(MD3_SHAPE_SMALL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_shape_apply_tokens(0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Validate all shape scales */
  for (s = 0; s < (int)MD3_SHAPE_SCALE_COUNT; s++) {
    rc = md3_shape_get_corner_radius((enum md3_shape_scale)s, &radius);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    if (s == MD3_SHAPE_NONE) {
      ASSERT_EQ(0.0f, radius);
    } else {
      ASSERT(radius > 0.0f);
    }
  }

  rc = ui_arena_create(65536, &arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_design_token_dict_init(arena, &dict);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Apply baseline shape tokens */
  rc = md3_shape_apply_tokens(0, &dict);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(dict.count > 0);

  /* Apply baseline and expressive shape tokens */
  rc = md3_shape_apply_tokens(1, &dict);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(dict.count > 0);

  rc = ui_arena_destroy(arena);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test error branch in baseline token loop: dict with capacity but NULL arena
   */
  memset(&dict_fail, 0, sizeof(dict_fail));
  dict_fail.tokens = tokens_storage;
  dict_fail.capacity = 16;
  dict_fail.count = 0;
  dict_fail.arena = NULL;
  rc = md3_shape_apply_tokens(0, &dict_fail);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Test error branch in expressive token loop */
  rc = ui_arena_create(65536, &arena_exp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_design_token_dict_init(arena_exp, &dict_exp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Populate baseline tokens first */
  rc = md3_shape_apply_tokens(0, &dict_exp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Break arena so expressive additions fail duplicate_string */
  dict_exp.arena = NULL;
  rc = md3_shape_apply_tokens(1, &dict_exp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = ui_arena_destroy(arena_exp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_contrast_schemes(void) {
  ui_color_t seed = UI_COLOR_ARGB(255, 103, 80, 164);
  struct md3_color_scheme scheme_normal, scheme_med, scheme_high;
  ui_error_t rc;

  /* Invalid argument checks */
  rc = md3_color_scheme_create_contrast(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_NORMAL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_color_scheme_create_contrast(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                                        (enum md3_contrast_level)99,
                                        &scheme_normal);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Light mode contrast variants */
  rc = md3_color_scheme_create_contrast(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_NORMAL, &scheme_normal);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_color_scheme_create_contrast(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_MEDIUM, &scheme_med);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_color_scheme_create_contrast(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_HIGH, &scheme_high);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT(scheme_normal.primary != scheme_med.primary ||
         scheme_med.primary != scheme_high.primary);

  /* Dark mode contrast variants */
  rc = md3_color_scheme_create_contrast(seed, 1, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_NORMAL, &scheme_normal);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_color_scheme_create_contrast(seed, 1, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_MEDIUM, &scheme_med);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_color_scheme_create_contrast(seed, 1, MD3_PALETTE_MODE_TONAL_SPOT,
                                        MD3_CONTRAST_HIGH, &scheme_high);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT(scheme_normal.primary != scheme_med.primary ||
         scheme_med.primary != scheme_high.primary);

  PASS();
}

TEST test_md3_expressive_ripple(void) {
  struct md3_ripple_params params;
  struct md3_ripple_frame frame;
  ui_error_t rc;

  /* Invalid argument checks */
  rc = md3_ripple_evaluate(NULL, &frame);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_ripple_evaluate(&params, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  params.style = MD3_RIPPLE_STYLE_STANDARD;
  params.origin_x = 10.0f;
  params.origin_y = 10.0f;
  params.max_radius = 50.0f;
  params.progress = -0.1f;
  rc = md3_ripple_evaluate(&params, &frame);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  params.progress = 1.1f;
  rc = md3_ripple_evaluate(&params, &frame);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  params.progress = 0.5f;
  params.max_radius = -10.0f;
  rc = md3_ripple_evaluate(&params, &frame);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  params.max_radius = 50.0f;
  params.style = (enum md3_ripple_style)99;
  rc = md3_ripple_evaluate(&params, &frame);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Standard style evaluation */
  params.max_radius = 50.0f;
  params.progress = 0.5f;
  params.style = MD3_RIPPLE_STYLE_STANDARD;
  rc = md3_ripple_evaluate(&params, &frame);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(25.0f, frame.current_radius);
  ASSERT_EQ(0.5f, frame.current_opacity);
  ASSERT_EQ(0, frame.sparkle_count);

  /* Fluid wave style evaluation */
  params.style = MD3_RIPPLE_STYLE_FLUID_WAVE;
  rc = md3_ripple_evaluate(&params, &frame);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(frame.current_radius > 0.0f);
  ASSERT(frame.current_opacity > 0.0f);

  /* Sparkle style evaluation */
  params.style = MD3_RIPPLE_STYLE_SPARKLE;
  rc = md3_ripple_evaluate(&params, &frame);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(8, frame.sparkle_count);
  ASSERT(frame.sparkle_scales[0] >= 0.0f);

  /* Sparkle style evaluation with progress triggering negative clamp */
  params.progress = 1.0f;
  rc = md3_ripple_evaluate(&params, &frame);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(8, frame.sparkle_count);
  ASSERT_EQ(0.0f, frame.sparkle_scales[1]);

  PASS();
}

TEST test_md3_font_variations(void) {
  struct ui_font_manager *mgr = NULL;
  struct ui_font *font = NULL;
  ui_error_t rc;

  /* Invalid argument checks */
  rc = md3_typography_apply_font_variations(NULL, MD3_TYPESCALE_BODY_LARGE, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = ui_font_manager_create(&mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_font_manager_load_font_memory(mgr, tests_tiny_ttf,
                                        sizeof(tests_tiny_ttf), &font);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(font != NULL);

  rc = md3_typography_apply_font_variations(
      font, (enum md3_typescale_role)MD3_TYPESCALE_ROLE_COUNT, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_typography_apply_font_variations(font, (enum md3_typescale_role) - 1,
                                            0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid font variations: baseline and expressive */
  rc = md3_typography_apply_font_variations(font, MD3_TYPESCALE_BODY_LARGE, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_typography_apply_font_variations(font, MD3_TYPESCALE_BODY_LARGE, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_font_manager_destroy(mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(md3_tokens_suite) {
  RUN_TEST(test_md3_color_harmonization);
  RUN_TEST(test_md3_color_schemes_all_modes);
  RUN_TEST(test_md3_contrast_schemes);
  RUN_TEST(test_md3_elevation);
  RUN_TEST(test_md3_state_layer);
  RUN_TEST(test_md3_expressive_ripple);
  RUN_TEST(test_md3_typography);
  RUN_TEST(test_md3_font_variations);
  RUN_TEST(test_md3_shapes);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_tokens_suite);
  GREATEST_MAIN_END();
}
