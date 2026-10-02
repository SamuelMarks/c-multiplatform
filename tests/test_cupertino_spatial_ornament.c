/**
 * @file test_cupertino_spatial_ornament.c
 * @brief Unit tests for visionOS & macOS Floating Spatial Glass Ornaments
 * conforming to Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_spatial_ornament.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

TEST test_spatial_ornament_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_spatial_ornament_descriptor desc;
  struct cupertino_spatial_ornament *orn = NULL;
  enum cupertino_ornament_placement plc;
  float d = 0.0f;
  float ao = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  int vis = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.placement = CUPERTINO_ORNAMENT_PLACEMENT_BOTTOM;
  desc.width = 300.0f;
  desc.height = 54.0f;
  desc.corner_radius = 27.0f;
  desc.depth_offset_pt = 32.0f;
  desc.ambient_occlusion = 0.85f;
  desc.edge_gap = 14.0f;

  /* Null checks */
  rc = cupertino_spatial_ornament_create(NULL, &desc, &orn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_create(dummy_engine, NULL, &orn);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = cupertino_spatial_ornament_create(dummy_engine, &desc, &orn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, orn);

  rc = cupertino_spatial_ornament_get_placement(orn, &plc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_ORNAMENT_PLACEMENT_BOTTOM, plc);

  rc = cupertino_spatial_ornament_get_depth(orn, &d, &ao);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(32.0f, d, 0.01f);
  ASSERT_IN_RANGE(0.85f, ao, 0.01f);

  rc = cupertino_spatial_ornament_get_size(orn, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(300.0f, w, 0.01f);
  ASSERT_IN_RANGE(54.0f, h, 0.01f);

  rc = cupertino_spatial_ornament_is_visible(orn, &vis);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, vis);

  rc = cupertino_spatial_ornament_set_visible(orn, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_spatial_ornament_is_visible(orn, &vis);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, vis);

  /* Null checks */
  rc = cupertino_spatial_ornament_set_placement(
      NULL, CUPERTINO_ORNAMENT_PLACEMENT_TOP);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_get_placement(NULL, &plc);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_get_placement(orn, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_get_depth(NULL, &d, &ao);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_get_depth(orn, NULL, &ao);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_get_depth(orn, &d, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_set_size(NULL, 100.0f, 50.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_get_size(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_get_size(orn, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_get_size(orn, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_set_visible(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_is_visible(NULL, &vis);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_is_visible(orn, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_compute_bounds(NULL, 0.0f, 0.0f, 100.0f,
                                                 100.0f, &w, &h, &d, &ao);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_compute_shadow(NULL, &d, &ao, &w);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_spatial_ornament_destroy(orn);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_spatial_ornament_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_spatial_ornament_bounds_and_shadow(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_spatial_ornament_descriptor desc;
  struct cupertino_spatial_ornament *orn = NULL;
  float ox = 0.0f;
  float oy = 0.0f;
  float ow = 0.0f;
  float oh = 0.0f;
  float sy = 0.0f;
  float sr = 0.0f;
  float sa = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.placement = CUPERTINO_ORNAMENT_PLACEMENT_TOP;
  desc.width = 200.0f;
  desc.height = 40.0f;
  desc.depth_offset_pt = 20.0f;
  desc.ambient_occlusion = 0.6f;
  desc.edge_gap = 10.0f;

  rc = cupertino_spatial_ornament_create(dummy_engine, &desc, &orn);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Placement: TOP. Window at (100, 100), 800x600 */
  rc = cupertino_spatial_ornament_compute_bounds(orn, 100.0f, 100.0f, 800.0f,
                                                 600.0f, &ox, &oy, &ow, &oh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(100.0f + (800.0f - 200.0f) * 0.5f, ox, 0.01f);
  ASSERT_IN_RANGE(100.0f - 40.0f - 10.0f, oy, 0.01f);
  ASSERT_IN_RANGE(200.0f, ow, 0.01f);
  ASSERT_IN_RANGE(40.0f, oh, 0.01f);

  /* Placement: BOTTOM */
  rc = cupertino_spatial_ornament_set_placement(
      orn, CUPERTINO_ORNAMENT_PLACEMENT_BOTTOM);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_spatial_ornament_compute_bounds(orn, 100.0f, 100.0f, 800.0f,
                                                 600.0f, &ox, &oy, &ow, &oh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(100.0f + 600.0f + 10.0f, oy, 0.01f);

  /* Placement: LEADING */
  rc = cupertino_spatial_ornament_set_placement(
      orn, CUPERTINO_ORNAMENT_PLACEMENT_LEADING);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_spatial_ornament_compute_bounds(orn, 100.0f, 100.0f, 800.0f,
                                                 600.0f, &ox, &oy, &ow, &oh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(100.0f - 200.0f - 10.0f, ox, 0.01f);

  /* Placement: TRAILING */
  rc = cupertino_spatial_ornament_set_placement(
      orn, CUPERTINO_ORNAMENT_PLACEMENT_TRAILING);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_spatial_ornament_compute_bounds(orn, 100.0f, 100.0f, 800.0f,
                                                 600.0f, &ox, &oy, &ow, &oh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(100.0f + 800.0f + 10.0f, ox, 0.01f);

  /* Shadow computation */
  rc = cupertino_spatial_ornament_compute_shadow(orn, &sy, &sr, &sa);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(20.0f * 0.4f, sy, 0.01f);
  ASSERT_IN_RANGE(20.0f * 0.8f, sr, 0.01f);
  ASSERT_IN_RANGE(0.6f * 0.35f, sa, 0.01f);

  /* NULL checks for compute_shadow */
  rc = cupertino_spatial_ornament_compute_shadow(orn, NULL, &sr, &sa);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_compute_shadow(orn, &sy, NULL, &sa);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_compute_shadow(orn, &sy, &sr, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Unknown placement for compute_bounds default branch */
  rc = cupertino_spatial_ornament_set_placement(
      orn, (enum cupertino_ornament_placement)999);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_spatial_ornament_compute_bounds(orn, 100.0f, 100.0f, 800.0f,
                                                 600.0f, &ox, &oy, &ow, &oh);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* NULL checks for compute_bounds */
  rc = cupertino_spatial_ornament_compute_bounds(orn, 100.0f, 100.0f, 800.0f,
                                                 600.0f, NULL, &oy, &ow, &oh);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_compute_bounds(orn, 100.0f, 100.0f, 800.0f,
                                                 600.0f, &ox, NULL, &ow, &oh);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_compute_bounds(orn, 100.0f, 100.0f, 800.0f,
                                                 600.0f, &ox, &oy, NULL, &oh);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_compute_bounds(orn, 100.0f, 100.0f, 800.0f,
                                                 600.0f, &ox, &oy, &ow, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Size mutation */
  rc = cupertino_spatial_ornament_set_size(orn, 350.0f, 60.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_spatial_ornament_get_size(orn, &ow, &oh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(350.0f, ow, 0.01f);
  ASSERT_IN_RANGE(60.0f, oh, 0.01f);

  /* Invalid sizes */
  rc = cupertino_spatial_ornament_set_size(orn, -10.0f, 50.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_set_size(orn, 50.0f, -10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Depth mutation */
  rc = cupertino_spatial_ornament_set_depth(orn, 40.0f, 0.9f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Invalid depths */
  rc = cupertino_spatial_ornament_set_depth(NULL, 40.0f, 0.9f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_set_depth(orn, -1.0f, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_set_depth(orn, 20.0f, -0.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spatial_ornament_set_depth(orn, 20.0f, 1.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_spatial_ornament_destroy(orn);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Descriptor fallback testing */
  memset(&desc, 0, sizeof(desc));
  desc.width = 0.0f;
  desc.height = 0.0f;
  desc.corner_radius = -1.0f;
  desc.depth_offset_pt = -1.0f;
  desc.ambient_occlusion = -0.5f;
  desc.edge_gap = -1.0f;
  rc = cupertino_spatial_ornament_create(dummy_engine, &desc, &orn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_spatial_ornament_destroy(orn);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&desc, 0, sizeof(desc));
  desc.ambient_occlusion = 2.0f;
  rc = cupertino_spatial_ornament_create(dummy_engine, &desc, &orn);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_spatial_ornament_destroy(orn);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_spatial_ornament_oom(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_spatial_ornament_descriptor desc;
  struct cupertino_spatial_ornament *orn = NULL;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  g_malloc_fail_countdown = 0;
  rc = cupertino_spatial_ornament_create(dummy_engine, &desc, &orn);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, orn);

  PASS();
}

SUITE(cupertino_spatial_ornament_suite) {
  RUN_TEST(test_spatial_ornament_lifecycle);
  RUN_TEST(test_spatial_ornament_bounds_and_shadow);
  RUN_TEST(test_spatial_ornament_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_spatial_ornament_suite);
  GREATEST_MAIN_END();
}
