/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_state_layer_update(void) {
  cmp_state_layer_t layer;
  memset(&layer, 0, sizeof(cmp_state_layer_t));

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_state_layer_update(&layer, 16.0f), "%d");

  layer.is_active = 1;
  layer.max_radius = 100.0f;
  layer.current_radius = 0.0f;

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_state_layer_update(&layer, 16.0f), "%d");
  ASSERT(layer.current_radius > 0.0f);

  layer.is_active = 0;
  layer.current_opacity = 1.0f;
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_state_layer_update(&layer, 16.0f), "%d");
  ASSERT(layer.current_opacity < 1.0f);

  PASS();
}

TEST test_state_layer_trigger_ripple(void) {
  cmp_state_layer_t layer;
  memset(&layer, 0, sizeof(cmp_state_layer_t));

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_state_layer_trigger_ripple(&layer, 50.0f, 50.0f, 200.0f),
                "%d");
  ASSERT_EQ(1, layer.is_active);
  ASSERT_EQ(50.0f, layer.origin_x);
  ASSERT_EQ(50.0f, layer.origin_y);
  ASSERT_EQ(200.0f, layer.max_radius);
  ASSERT_EQ(0.0f, layer.current_radius);
  ASSERT(layer.current_opacity > 0.0f);

  PASS();
}

TEST test_state_layer_trigger_fluent_reveal(void) {
  cmp_state_layer_t layer;
  memset(&layer, 0, sizeof(cmp_state_layer_t));

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_state_layer_trigger_fluent_reveal(&layer, 10.0f, 20.0f),
                "%d");
  ASSERT_EQ(10.0f, layer.origin_x);
  ASSERT_EQ(20.0f, layer.origin_y);
  ASSERT_EQ(1.0f, layer.current_opacity);

  PASS();
}

TEST test_state_layer_apply_vibrancy_mask(void) {
  cmp_state_layer_t layer;
  memset(&layer, 0, sizeof(cmp_state_layer_t));

  ASSERT_EQ_FMT(
      CMP_SUCCESS,
      cmp_state_layer_apply_vibrancy_mask(&layer, CMP_VIBRANCY_STYLE_FILL),
      "%d");
  ASSERT_EQ(0.2f, layer.current_opacity);

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_state_layer_apply_vibrancy_mask(
                    &layer, CMP_VIBRANCY_STYLE_SECONDARY_FILL),
                "%d");
  ASSERT_EQ(0.1f, layer.current_opacity);

  PASS();
}

SUITE(state_layers_suite) {
  RUN_TEST(test_state_layer_update);
  RUN_TEST(test_state_layer_trigger_ripple);
  RUN_TEST(test_state_layer_trigger_fluent_reveal);
  RUN_TEST(test_state_layer_apply_vibrancy_mask);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(state_layers_suite);
  GREATEST_MAIN_END();
}
