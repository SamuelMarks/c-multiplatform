/**
 * @file test_cupertino_activity_rings.c
 * @brief Unit tests for Cupertino Activity & Health Rings Gauge component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_activity_rings.h"
#include "ui_test_mock_mem.h"
#include <math.h>
#include <string.h>
/* clang-format on */

SUITE(cupertino_activity_rings_suite);

TEST test_rings_invalid_arguments(void) {
  struct cupertino_activity_rings_descriptor desc;
  struct cupertino_activity_rings *rings = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_color_t c1 = 0;
  ui_color_t c2 = 0;
  ui_color_t c3 = 0;
  float val = 0.0f;
  float x = 0.0f;
  float y = 0.0f;
  int has_shadow = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.outer_radius = 80.0f;
  desc.ring_thickness = 18.0f;

  /* Creation invalid */
  rc = cupertino_activity_rings_create(NULL, &desc, &rings);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_create(dummy_engine, NULL, &rings);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Radius / thickness <= 0 */
  desc.outer_radius = 0.0f;
  rc = cupertino_activity_rings_create(dummy_engine, &desc, &rings);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.outer_radius = 80.0f;
  desc.ring_thickness = -5.0f;
  rc = cupertino_activity_rings_create(dummy_engine, &desc, &rings);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_activity_rings_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / get progress invalid */
  rc = cupertino_activity_rings_set_progress(NULL, CUPERTINO_RING_MOVE, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_progress(NULL, CUPERTINO_RING_MOVE, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Radius / shadow / cap center / colors / dimensions invalid */
  rc =
      cupertino_activity_rings_get_ring_radius(NULL, CUPERTINO_RING_MOVE, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_calculate_shadow(NULL, CUPERTINO_RING_MOVE,
                                                 &has_shadow, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_cap_center(NULL, CUPERTINO_RING_MOVE, &x,
                                               &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_colors(NULL, CUPERTINO_RING_MOVE, &c1, &c2,
                                           &c3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_dimensions(NULL, &x, &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid rings with invalid ring IDs or NULL out pointers */
  desc.ring_thickness = 18.0f;
  rc = cupertino_activity_rings_create(dummy_engine, &desc, &rings);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(rings != NULL);

  rc = cupertino_activity_rings_set_progress(
      rings, (enum cupertino_ring_id) - 1, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_set_progress(
      rings, (enum cupertino_ring_id)CUPERTINO_RING_COUNT, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_activity_rings_get_progress(
      rings, (enum cupertino_ring_id) - 1, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_progress(
      rings, (enum cupertino_ring_id)CUPERTINO_RING_COUNT, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_progress(rings, CUPERTINO_RING_MOVE, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_activity_rings_get_ring_radius(
      rings, (enum cupertino_ring_id) - 1, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_ring_radius(
      rings, (enum cupertino_ring_id)CUPERTINO_RING_COUNT, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_ring_radius(rings, CUPERTINO_RING_MOVE,
                                                NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_activity_rings_calculate_shadow(
      rings, (enum cupertino_ring_id) - 1, &has_shadow, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_calculate_shadow(
      rings, (enum cupertino_ring_id)CUPERTINO_RING_COUNT, &has_shadow, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_calculate_shadow(rings, CUPERTINO_RING_MOVE,
                                                 NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_calculate_shadow(rings, CUPERTINO_RING_MOVE,
                                                 &has_shadow, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_activity_rings_get_cap_center(rings, CUPERTINO_RING_MOVE, NULL,
                                               &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_cap_center(rings, CUPERTINO_RING_MOVE, &x,
                                               NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_cap_center(
      rings, (enum cupertino_ring_id)CUPERTINO_RING_COUNT, &x, &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_activity_rings_get_colors(rings, (enum cupertino_ring_id) - 1,
                                           &c1, &c2, &c3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_colors(
      rings, (enum cupertino_ring_id)CUPERTINO_RING_COUNT, &c1, &c2, &c3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_colors(rings, CUPERTINO_RING_MOVE, NULL,
                                           &c2, &c3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_colors(rings, CUPERTINO_RING_MOVE, &c1,
                                           NULL, &c3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_colors(rings, CUPERTINO_RING_MOVE, &c1, &c2,
                                           NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_activity_rings_get_dimensions(rings, NULL, &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_activity_rings_get_dimensions(rings, &x, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_activity_rings_destroy(rings);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_rings_geometry_and_progress(void) {
  struct cupertino_activity_rings_descriptor desc;
  struct cupertino_activity_rings *rings = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float p_move = 0.0f;
  float p_exercise = 0.0f;
  float p_stand = 0.0f;
  float r0 = 0.0f;
  float r1 = 0.0f;
  float r2 = 0.0f;
  float x = 0.0f;
  float y = 0.0f;
  float shadow_angle = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  int has_shadow = 0;
  ui_color_t col_start = 0;
  ui_color_t col_end = 0;
  ui_color_t col_track = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.outer_radius = 80.0f;
  desc.ring_thickness = 18.0f;
  desc.ring_gap = 4.0f;
  desc.initial_progress[0] = 0.75f;
  desc.initial_progress[1] = 1.25f; /* Over 100% loop */
  desc.initial_progress[2] = 0.50f;

  rc = cupertino_activity_rings_create(dummy_engine, &desc, &rings);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(rings != NULL);

  /* Verify progress values */
  rc = cupertino_activity_rings_get_progress(rings, CUPERTINO_RING_MOVE,
                                             &p_move);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(p_move - 0.75f) < 0.001f);

  rc = cupertino_activity_rings_get_progress(rings, CUPERTINO_RING_EXERCISE,
                                             &p_exercise);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(p_exercise - 1.25f) < 0.001f);

  rc = cupertino_activity_rings_get_progress(rings, CUPERTINO_RING_STAND,
                                             &p_stand);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(p_stand - 0.50f) < 0.001f);

  /* Radii */
  rc =
      cupertino_activity_rings_get_ring_radius(rings, CUPERTINO_RING_MOVE, &r0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(80.0f, r0);

  rc = cupertino_activity_rings_get_ring_radius(rings, CUPERTINO_RING_EXERCISE,
                                                &r1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(80.0f - (18.0f + 4.0f), r1);

  rc = cupertino_activity_rings_get_ring_radius(rings, CUPERTINO_RING_STAND,
                                                &r2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(80.0f - 2.0f * (18.0f + 4.0f), r2);

  /* Shadow test: Move ring at 0.75 has no shadow */
  rc = cupertino_activity_rings_calculate_shadow(rings, CUPERTINO_RING_MOVE,
                                                 &has_shadow, &shadow_angle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, has_shadow);

  /* Exercise ring at 1.25 has overlapping shadow */
  rc = cupertino_activity_rings_calculate_shadow(rings, CUPERTINO_RING_EXERCISE,
                                                 &has_shadow, &shadow_angle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, has_shadow);

  /* Cap center coordinate test */
  rc = cupertino_activity_rings_get_cap_center(rings, CUPERTINO_RING_MOVE, &x,
                                               &y);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(x) > 0.0f || fabs(y) > 0.0f);

  /* Colors */
  rc = cupertino_activity_rings_get_colors(rings, CUPERTINO_RING_MOVE,
                                           &col_start, &col_end, &col_track);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(col_start != 0);
  ASSERT(col_end != 0);
  ASSERT(col_track != 0);

  /* Dimensions */
  rc = cupertino_activity_rings_get_dimensions(rings, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w > 170.0f);
  ASSERT_EQ(w, h);

  /* Update progress */
  rc = cupertino_activity_rings_set_progress(rings, CUPERTINO_RING_MOVE, 2.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_activity_rings_calculate_shadow(rings, CUPERTINO_RING_MOVE,
                                                 &has_shadow, &shadow_angle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, has_shadow);

  /* Reject negative progress */
  rc = cupertino_activity_rings_set_progress(rings, CUPERTINO_RING_MOVE, -0.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Reject invalid ring id */
  rc = cupertino_activity_rings_set_progress(
      rings, (enum cupertino_ring_id)CUPERTINO_RING_COUNT, 1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_activity_rings_destroy(rings);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Fallback for negative ring_gap and negative initial_progress */
  memset(&desc, 0, sizeof(desc));
  desc.outer_radius = 80.0f;
  desc.ring_thickness = 18.0f;
  desc.ring_gap = -1.0f;
  desc.initial_progress[0] = -0.1f;
  desc.initial_progress[1] = -0.2f;
  desc.initial_progress[2] = -0.3f;
  rc = cupertino_activity_rings_create(dummy_engine, &desc, &rings);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_activity_rings_get_progress(rings, CUPERTINO_RING_MOVE,
                                             &p_move);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, p_move);
  rc = cupertino_activity_rings_destroy(rings);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_rings_oom_mock(void) {
  struct cupertino_activity_rings_descriptor desc;
  struct cupertino_activity_rings *rings = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.outer_radius = 80.0f;
  desc.ring_thickness = 18.0f;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_activity_rings_create(dummy_engine, &desc, &rings);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, rings);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_activity_rings_create(NULL, &desc, &rings);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_activity_rings_suite) {
  RUN_TEST(test_rings_invalid_arguments);
  RUN_TEST(test_rings_geometry_and_progress);
  RUN_TEST(test_rings_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_activity_rings_suite);
  GREATEST_MAIN_END();
}
