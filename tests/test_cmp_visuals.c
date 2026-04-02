/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_semantic_colors(void) {
  cmp_semantic_colors_t *ctx = NULL;
  uint32_t color;

  ASSERT_EQ(CMP_SUCCESS, cmp_semantic_colors_create(&ctx));

  /* Test True Black background for Dark Mode OLED */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_semantic_colors_resolve(ctx, "systemBackground", 1, &color));
  ASSERT_EQ(0x000000FF, color);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_semantic_colors_resolve(ctx, "systemBackground", 0, &color));
  ASSERT_EQ(0xFFFFFFFF, color);

  /* Test Tint colors */
  ASSERT_EQ(CMP_SUCCESS, cmp_semantic_colors_get_tint_color(ctx, &color));
  ASSERT_EQ(0x007AFFFF, color); /* Default Blue */

  ASSERT_EQ(CMP_SUCCESS, cmp_semantic_colors_set_tint_color(ctx, 0xFF0000FF));
  ASSERT_EQ(CMP_SUCCESS, cmp_semantic_colors_get_tint_color(ctx, &color));
  ASSERT_EQ(0xFF0000FF, color);

  /* Test Elevation Shadows (Dark mode uses lighter grays) */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_semantic_colors_resolve_elevation(ctx, 0, 1, &color));
  ASSERT_EQ(0x000000FF, color);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_semantic_colors_resolve_elevation(ctx, 1, 1, &color));
  ASSERT_EQ(0x1C1C1EFF, color);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_semantic_colors_resolve_elevation(ctx, 1, 0, &color));
  ASSERT_EQ(0xFFFFFFFF, color);

  /* Not Found */
  ASSERT_EQ(CMP_ERROR_NOT_FOUND,
            cmp_semantic_colors_resolve(ctx, "doesNotExist", 1, &color));

  ASSERT_EQ(CMP_SUCCESS, cmp_semantic_colors_destroy(ctx));
  PASS();
}

TEST test_color_pipeline(void) {
  cmp_color_pipeline_t *pipeline = NULL;
  int supports_p3, supports_edr;
  float p3r, p3g, p3b;

  ASSERT_EQ(CMP_SUCCESS, cmp_color_pipeline_create(&pipeline));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_color_pipeline_supports_p3(pipeline, &supports_p3));
  ASSERT_EQ(1, supports_p3);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_color_pipeline_supports_edr(pipeline, &supports_edr));
  ASSERT_EQ(1, supports_edr);

  ASSERT_EQ(CMP_SUCCESS, cmp_color_pipeline_srgb_to_p3(pipeline, 1.0f, 0.5f,
                                                       0.0f, &p3r, &p3g, &p3b));

  ASSERT_EQ(CMP_SUCCESS, cmp_color_pipeline_destroy(pipeline));
  PASS();
}

TEST test_null_args(void) {
  cmp_semantic_colors_t *sc = NULL;
  cmp_color_pipeline_t *pl = NULL;
  uint32_t color;
  int bool_res;
  float fr, fg, fb;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_semantic_colors_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_color_pipeline_create(NULL));

  cmp_semantic_colors_create(&sc);
  cmp_color_pipeline_create(&pl);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_semantic_colors_resolve(NULL, "label", 0, &color));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_semantic_colors_resolve(sc, NULL, 0, &color));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_semantic_colors_resolve(sc, "label", 0, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_semantic_colors_set_tint_color(NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_semantic_colors_get_tint_color(NULL, &color));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_semantic_colors_get_tint_color(sc, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_semantic_colors_resolve_elevation(NULL, 0, 0, &color));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_semantic_colors_resolve_elevation(sc, 0, 0, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_color_pipeline_supports_p3(NULL, &bool_res));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_color_pipeline_supports_p3(pl, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_color_pipeline_supports_edr(NULL, &bool_res));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_color_pipeline_supports_edr(pl, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_color_pipeline_srgb_to_p3(
                                       NULL, 1.0f, 1.0f, 1.0f, &fr, &fg, &fb));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_color_pipeline_srgb_to_p3(
                                       pl, 1.0f, 1.0f, 1.0f, NULL, &fg, &fb));

  cmp_semantic_colors_destroy(sc);
  cmp_color_pipeline_destroy(pl);

  PASS();
}

TEST test_golden_image_visual_regression(void) {
  cmp_window_t *window = NULL;
  cmp_window_config_t config;
  void *pixels = NULL;
  int w, h;

  cmp_window_system_init();

  memset(&config, 0, sizeof(cmp_window_config_t));
  config.width = 200;
  config.height = 200;
  config.title = "Golden Image Validation";

  ASSERT_EQ(CMP_SUCCESS, cmp_window_create(&config, &window));

  /* Mock integration: In CI/CD, this would assert exact bit-matched RGBA arrays
     between the reference snapshot and the runtime capture at 1.0x, 1.5x
     and 2.0x display scales */
  ASSERT_EQ(CMP_SUCCESS, cmp_test_capture_snapshot(window, &pixels, &w, &h));
  ASSERT_NEQ(NULL, pixels);
  ASSERT_EQ(200, w);
  ASSERT_EQ(200, h);

  CMP_FREE(pixels);
  cmp_window_destroy(window);
  cmp_window_system_shutdown();
  PASS();
}
SUITE(visuals_suite) {
  RUN_TEST(test_semantic_colors);
  RUN_TEST(test_color_pipeline);
  RUN_TEST(test_null_args);
  RUN_TEST(test_golden_image_visual_regression);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(visuals_suite);
  GREATEST_MAIN_END();
}
