/* clang-format off */
#include "greatest.h"
#include "material3/md3_a11y.h"
#include "material3/md3_color.h"
#include "material3/md3_shape.h"
#include "material3/md3_shape_morph.h"
#include "material3/md3_state_layer.h"
#include "ui_error.h"
#include "ui_test_visual.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_md3_visual_catalog_light_and_dark(void) {
  ui_color_t seed = UI_COLOR_ARGB(255, 103, 80, 164);
  struct md3_color_scheme light_scheme, dark_scheme;
  unsigned char light_buf[16 * 4];
  unsigned char dark_buf[16 * 4];
  struct ui_visual_test_config cfg;
  int matched = 0;
  int i;
  ui_error_t rc;

  rc = md3_color_scheme_create(seed, 0, MD3_PALETTE_MODE_TONAL_SPOT,
                               &light_scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_color_scheme_create(seed, 1, MD3_PALETTE_MODE_TONAL_SPOT,
                               &dark_scheme);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Synthesize small 4x4 swatches */
  for (i = 0; i < 16; ++i) {
    light_buf[i * 4 + 0] = (unsigned char)UI_COLOR_RED(light_scheme.primary);
    light_buf[i * 4 + 1] = (unsigned char)UI_COLOR_GREEN(light_scheme.primary);
    light_buf[i * 4 + 2] = (unsigned char)UI_COLOR_BLUE(light_scheme.primary);
    light_buf[i * 4 + 3] = (unsigned char)UI_COLOR_ALPHA(light_scheme.primary);

    dark_buf[i * 4 + 0] = (unsigned char)UI_COLOR_RED(dark_scheme.primary);
    dark_buf[i * 4 + 1] = (unsigned char)UI_COLOR_GREEN(dark_scheme.primary);
    dark_buf[i * 4 + 2] = (unsigned char)UI_COLOR_BLUE(dark_scheme.primary);
    dark_buf[i * 4 + 3] = (unsigned char)UI_COLOR_ALPHA(dark_scheme.primary);
  }

  cfg.rms_threshold = 0.01;
  cfg.delta_e_threshold = 0.01;
  cfg.max_drift_percentage = 0.0;

  /* Self-match on light swatch */
  rc = ui_visual_fuzzy_match(light_buf, light_buf, 4, 4, &cfg, &matched);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, matched);

  /* Self-match on dark swatch */
  rc = ui_visual_fuzzy_match(dark_buf, dark_buf, 4, 4, &cfg, &matched);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, matched);

  /* Light and dark must differ */
  rc = ui_visual_fuzzy_match(light_buf, dark_buf, 4, 4, &cfg, &matched);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, matched);

  PASS();
}

TEST test_md3_visual_catalog_forced_colors(void) {
  float border_w = 0.0f;
  ui_color_t border_col = 0;
  unsigned char normal_buf[16 * 4];
  unsigned char forced_buf[16 * 4];
  struct ui_visual_test_config cfg;
  int matched = 0;
  int i;
  ui_error_t rc;

  rc = md3_a11y_get_forced_colors_border(0, &border_w, &border_col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  memset(normal_buf, 0, sizeof(normal_buf));

  rc = md3_a11y_get_forced_colors_border(1, &border_w, &border_col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  for (i = 0; i < 16; ++i) {
    forced_buf[i * 4 + 0] = (unsigned char)UI_COLOR_RED(border_col);
    forced_buf[i * 4 + 1] = (unsigned char)UI_COLOR_GREEN(border_col);
    forced_buf[i * 4 + 2] = (unsigned char)UI_COLOR_BLUE(border_col);
    forced_buf[i * 4 + 3] = (unsigned char)UI_COLOR_ALPHA(border_col);
  }

  cfg.rms_threshold = 0.01;
  cfg.delta_e_threshold = 0.01;
  cfg.max_drift_percentage = 0.0;

  rc = ui_visual_fuzzy_match(forced_buf, forced_buf, 4, 4, &cfg, &matched);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, matched);

  rc = ui_visual_fuzzy_match(normal_buf, forced_buf, 4, 4, &cfg, &matched);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, matched);

  PASS();
}

TEST test_md3_visual_catalog_shape_morphing(void) {
  struct md3_shape shape_from, shape_to, shape_mid;
  ui_error_t rc;

  /* Initialize shapes using expressive shape library */
  rc = md3_shape_create_expressive(MD3_EXPRESSIVE_SHAPE_RECTANGLE, 8,
                                   &shape_from);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_shape_create_expressive(MD3_EXPRESSIVE_SHAPE_CIRCLE, 8, &shape_to);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Progress at 50% */
  rc = md3_shape_morph_interpolate(&shape_from, &shape_to, 0.5f, &shape_mid);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(shape_from.vertex_count, shape_mid.vertex_count);
  ASSERT(shape_mid.vertices[0].x != shape_from.vertices[0].x ||
         shape_mid.vertices[0].y != shape_from.vertices[0].y ||
         shape_mid.vertices[1].x != shape_from.vertices[1].x);

  PASS();
}

TEST test_md3_visual_catalog_ripple_rasterization(void) {
  struct md3_ripple_params params;
  struct md3_ripple_frame frame_0, frame_50, frame_100;
  ui_error_t rc;

  params.style = MD3_RIPPLE_STYLE_FLUID_WAVE;
  params.origin_x = 24.0f;
  params.origin_y = 24.0f;
  params.max_radius = 48.0f;

  /* Frame 0 */
  params.progress = 0.0f;
  rc = md3_ripple_evaluate(&params, &frame_0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, frame_0.current_radius);
  ASSERT_EQ(1.0f, frame_0.current_opacity);

  /* Frame 0.5 */
  params.progress = 0.5f;
  rc = md3_ripple_evaluate(&params, &frame_50);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(frame_50.current_radius > 0.0f);
  ASSERT(frame_50.current_opacity < 1.0f);
  ASSERT(frame_50.wave_distortion != 0.0f);

  /* Frame 1.0 */
  params.progress = 1.0f;
  rc = md3_ripple_evaluate(&params, &frame_100);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, frame_100.current_opacity);

  PASS();
}

SUITE(md3_visual_catalog_suite) {
  RUN_TEST(test_md3_visual_catalog_light_and_dark);
  RUN_TEST(test_md3_visual_catalog_forced_colors);
  RUN_TEST(test_md3_visual_catalog_shape_morphing);
  RUN_TEST(test_md3_visual_catalog_ripple_rasterization);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_visual_catalog_suite);
  GREATEST_MAIN_END();
}
