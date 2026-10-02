/**
 * @file test_cupertino_magnifier.c
 * @brief Unit tests for Cupertino Text Magnifier (optical loupe) component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_magnifier.h"
#include "ui_test_mock_mem.h"
#include <math.h>
#include <string.h>
/* clang-format on */

SUITE(cupertino_magnifier_suite);

TEST test_magnifier_invalid_arguments(void) {
  struct cupertino_magnifier_descriptor desc;
  struct cupertino_magnifier *mag = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float x = 0.0f;
  float y = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  float scale = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid */
  rc = cupertino_magnifier_create(NULL, &desc, &mag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_create(dummy_engine, NULL, &mag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Negative width / height */
  desc.width = -1.0f;
  rc = cupertino_magnifier_create(dummy_engine, &desc, &mag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.width = 0.0f;
  desc.height = -1.0f;
  rc = cupertino_magnifier_create(dummy_engine, &desc, &mag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.height = 0.0f;

  /* Destruction invalid */
  rc = cupertino_magnifier_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Show / update / clamp / hide */
  rc = cupertino_magnifier_show(NULL, 100.0f, 100.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_update_position(NULL, 100.0f, 100.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_clamp_bounds(NULL, 320.0f, 480.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_clamp_bounds((struct cupertino_magnifier *)0x123,
                                        0.0f, 480.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_clamp_bounds((struct cupertino_magnifier *)0x123,
                                        320.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_hide(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Tick */
  rc = cupertino_magnifier_tick(NULL, 16.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_tick((struct cupertino_magnifier *)0x123, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Getters */
  rc = cupertino_magnifier_get_lens_rect(NULL, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_get_lens_rect(
      (const struct cupertino_magnifier *)0x123, NULL, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_get_lens_rect(
      (const struct cupertino_magnifier *)0x123, &x, NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_get_lens_rect(
      (const struct cupertino_magnifier *)0x123, &x, &y, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_get_lens_rect(
      (const struct cupertino_magnifier *)0x123, &x, &y, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_magnifier_get_sample_center(NULL, &x, &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_get_sample_center(
      (const struct cupertino_magnifier *)0x123, NULL, &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_get_sample_center(
      (const struct cupertino_magnifier *)0x123, &x, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_magnifier_get_scale(NULL, &scale);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_magnifier_get_scale((const struct cupertino_magnifier *)0x123,
                                     NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_magnifier_lifecycle_and_movement(void) {
  struct cupertino_magnifier_descriptor desc;
  struct cupertino_magnifier *mag = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float x = 0.0f;
  float y = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  float sx = 0.0f;
  float sy = 0.0f;
  float scale = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.width = 78.0f;
  desc.height = 38.0f;

  rc = cupertino_magnifier_create(dummy_engine, &desc, &mag);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(mag != NULL);

  /* Initial scale is 0 */
  rc = cupertino_magnifier_get_scale(mag, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, scale);

  /* Show at (150, 200) */
  rc = cupertino_magnifier_show(mag, 150.0f, 200.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_magnifier_get_sample_center(mag, &sx, &sy);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(150.0f, sx);
  ASSERT_EQ(200.0f, sy);

  /* Advance tick 200ms -> scale should reach 1.0 */
  rc = cupertino_magnifier_tick(mag, 200.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_magnifier_get_scale(mag, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.0f, scale);

  /* Lens rect at scale 1.0 */
  rc = cupertino_magnifier_get_lens_rect(mag, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(78.0f, w);
  ASSERT_EQ(38.0f, h);
  /* Lens center should be above touch by 48pt */
  ASSERT_EQ(150.0f - 39.0f, x);
  ASSERT_EQ(200.0f - 48.0f - 19.0f, y);

  /* Update touch position */
  rc = cupertino_magnifier_update_position(mag, 160.0f, 210.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_magnifier_get_sample_center(mag, &sx, &sy);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(160.0f, sx);
  ASSERT_EQ(210.0f, sy);

  /* Test boundary clamping */
  /* Near left edge */
  rc = cupertino_magnifier_update_position(mag, 10.0f, 200.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_magnifier_clamp_bounds(mag, 320.0f, 480.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_magnifier_get_lens_rect(mag, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(x >= 8.0f);

  /* Hide and advance tick -> scale down to 0 */
  rc = cupertino_magnifier_hide(mag);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_magnifier_tick(mag, 200.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_magnifier_get_scale(mag, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, scale);

  rc = cupertino_magnifier_destroy(mag);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_magnifier_coverage_edges(void) {
  struct cupertino_magnifier_descriptor desc;
  struct cupertino_magnifier *mag = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float x, y, w, h;
  float scale;
  ui_error_t rc;

  /* Custom descriptor with positive offset, custom zoom and radius */
  memset(&desc, 0, sizeof(desc));
  desc.width = 100.0f;
  desc.height = 50.0f;
  desc.vertical_offset = 20.0f;
  desc.magnification = 2.0f;
  desc.corner_radius = 12.0f;

  rc = cupertino_magnifier_create(dummy_engine, &desc, &mag);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(mag != NULL);
  ASSERT_EQ(100.0f, mag->width);
  ASSERT_EQ(50.0f, mag->height);
  ASSERT_EQ(20.0f, mag->vertical_offset);
  ASSERT_EQ(2.0f, mag->magnification);
  ASSERT_EQ(12.0f, mag->corner_radius);

  /* Show and tick halfway (e.g. 75ms / 150ms -> 0.5) */
  rc = cupertino_magnifier_show(mag, 500.0f, 500.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_magnifier_tick(mag, 75.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_magnifier_get_scale(mag, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scale > 0.49f && scale < 0.51f);

  /* Clamp right edge */
  mag->lens_x = 400.0f;
  rc = cupertino_magnifier_clamp_bounds(mag, 320.0f, 480.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(320.0f - 8.0f - 100.0f, mag->lens_x);

  /* Clamp top edge */
  mag->lens_y = 2.0f;
  rc = cupertino_magnifier_clamp_bounds(mag, 320.0f, 480.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(8.0f, mag->lens_y);

  /* Clamp bottom edge */
  mag->lens_y = 500.0f;
  rc = cupertino_magnifier_clamp_bounds(mag, 320.0f, 480.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(480.0f - 8.0f - 50.0f, mag->lens_y);

  /* Tick up to full scale */
  rc = cupertino_magnifier_tick(mag, 150.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_magnifier_get_scale(mag, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.0f, scale);

  /* Hide and tick halfway down */
  rc = cupertino_magnifier_hide(mag);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_magnifier_tick(mag, 75.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_magnifier_get_scale(mag, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scale > 0.49f && scale < 0.51f);

  /* Tick past zero -> should clamp to 0.0 */
  rc = cupertino_magnifier_tick(mag, 200.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_magnifier_get_scale(mag, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, scale);

  rc = cupertino_magnifier_get_lens_rect(mag, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, w);
  ASSERT_EQ(0.0f, h);

  rc = cupertino_magnifier_destroy(mag);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test creation with width == 0.0f and height == 0.0f to trigger defaults */
  memset(&desc, 0, sizeof(desc));
  rc = cupertino_magnifier_create(dummy_engine, &desc, &mag);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_MAGNIFIER_DEFAULT_WIDTH, mag->width);
  ASSERT_EQ(CUPERTINO_MAGNIFIER_DEFAULT_HEIGHT, mag->height);

  /* Test tick when scale is already equal to target (current_scale == target ==
   * 0.0f) */
  rc = cupertino_magnifier_tick(mag, 16.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, mag->current_scale);

  rc = cupertino_magnifier_destroy(mag);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_magnifier_oom_mock(void) {
  struct cupertino_magnifier_descriptor desc;
  struct cupertino_magnifier *mag = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_magnifier_create(dummy_engine, &desc, &mag);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, mag);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_magnifier_create(dummy_engine, NULL, &mag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_magnifier_suite) {
  RUN_TEST(test_magnifier_invalid_arguments);
  RUN_TEST(test_magnifier_lifecycle_and_movement);
  RUN_TEST(test_magnifier_coverage_edges);
  RUN_TEST(test_magnifier_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_magnifier_suite);
  GREATEST_MAIN_END();
}
