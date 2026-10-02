/**
 * @file test_cupertino_wheel.c
 * @brief Unit tests for Cupertino 3D Cylinder Wheel Picker component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_wheel.h"
#include "ui_test_mock_mem.h"
#include <math.h>
#include <string.h>
/* clang-format on */

SUITE(cupertino_wheel_suite);

TEST test_wheel_invalid_arguments(void) {
  struct cupertino_wheel_descriptor desc;
  struct cupertino_wheel *wheel = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float y = 0.0f;
  float scale = 0.0f;
  float op = 0.0f;
  float angle = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  int count = 0;
  int idx = 0;
  int tick = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid */
  rc = cupertino_wheel_create(NULL, &desc, &wheel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_create(dummy_engine, NULL, &wheel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Negative items or invalid selected index */
  desc.item_count = -1;
  rc = cupertino_wheel_create(dummy_engine, &desc, &wheel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Creation with item_count == 0 */
  desc.item_count = 0;
  desc.selected_index = 0;
  rc = cupertino_wheel_create(dummy_engine, &desc, &wheel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, wheel);

  /* Set selected index on wheel with item_count == 0 */
  rc = cupertino_wheel_set_selected_index(wheel, 0, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_wheel_set_selected_index(wheel, 1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Drag end on wheel with item_count == 0 */
  rc = cupertino_wheel_drag_end(wheel, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tick on wheel with item_count == 0 */
  rc = cupertino_wheel_tick(wheel, 16.0f, &tick);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_wheel_destroy(wheel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  wheel = NULL;

  desc.item_count = 5;
  desc.selected_index = -1;
  rc = cupertino_wheel_create(dummy_engine, &desc, &wheel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.selected_index = 10;
  rc = cupertino_wheel_create(dummy_engine, &desc, &wheel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation with default viewport and item dimensions (0.0f) */
  desc.selected_index = 2;
  desc.viewport_height = 0.0f;
  desc.item_height = 0.0f;
  rc = cupertino_wheel_create(dummy_engine, &desc, &wheel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, wheel);

  /* Destruction invalid */
  rc = cupertino_wheel_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / get item count */
  rc = cupertino_wheel_set_item_count(NULL, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_set_item_count(wheel, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_get_item_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_get_item_count(wheel, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set item count to 0 (count == 0 branch) */
  rc = cupertino_wheel_set_item_count(wheel, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, wheel->item_count);
  ASSERT_EQ(0, wheel->selected_index);

  /* Set / get selected index */
  rc = cupertino_wheel_set_selected_index(NULL, 0, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_set_selected_index(wheel, -1, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Restore count to test out-of-bounds index */
  rc = cupertino_wheel_set_item_count(wheel, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_wheel_set_selected_index(wheel, 10, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_wheel_get_selected_index(NULL, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_get_selected_index(wheel, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Drag operations */
  rc = cupertino_wheel_drag_start(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_drag_update(NULL, 10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_drag_end(NULL, 50.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Tick invalid */
  rc = cupertino_wheel_tick(NULL, 16.0f, &tick);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_tick(wheel, -1.0f, &tick);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_tick(wheel, 16.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Calculate item transform invalid */
  rc = cupertino_wheel_calculate_item_transform(NULL, 0, &y, &scale, &op,
                                                &angle);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_calculate_item_transform(wheel, -1, &y, &scale, &op,
                                                &angle);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_calculate_item_transform(wheel, 10, &y, &scale, &op,
                                                &angle);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_calculate_item_transform(wheel, 0, NULL, &scale, &op,
                                                &angle);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      cupertino_wheel_calculate_item_transform(wheel, 0, &y, NULL, &op, &angle);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_calculate_item_transform(wheel, 0, &y, &scale, NULL,
                                                &angle);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      cupertino_wheel_calculate_item_transform(wheel, 0, &y, &scale, &op, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Dimensions invalid */
  rc = cupertino_wheel_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_get_dimensions(wheel, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_wheel_get_dimensions(wheel, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_wheel_destroy(wheel);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_wheel_lifecycle_and_selection(void) {
  struct cupertino_wheel_descriptor desc;
  struct cupertino_wheel *wheel = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  int count = 0;
  int idx = 0;
  int tick = 0;
  float w = 0.0f;
  float h = 0.0f;
  float y = 0.0f;
  float scale = 0.0f;
  float op = 0.0f;
  float angle = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.item_count = 10;
  desc.selected_index = 2;
  desc.viewport_height = 216.0f;
  desc.item_height = 32.0f;

  rc = cupertino_wheel_create(dummy_engine, &desc, &wheel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(wheel != NULL);

  rc = cupertino_wheel_get_item_count(wheel, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10, count);

  rc = cupertino_wheel_get_selected_index(wheel, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, idx);

  rc = cupertino_wheel_get_dimensions(wheel, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(216.0f, h);

  /* Immediate selection change */
  rc = cupertino_wheel_set_selected_index(wheel, 5, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_wheel_get_selected_index(wheel, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5, idx);

  /* Animated selection change (animated = 1 branch) */
  rc = cupertino_wheel_set_selected_index(wheel, 8, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, wheel->is_settling);

  /* Tick until settled (< 0.2f branch) */
  while (wheel->is_settling) {
    rc = cupertino_wheel_tick(wheel, 16.0f, &tick);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  ASSERT_EQ(0, wheel->is_settling);
  ASSERT_EQ(8, wheel->selected_index);

  /* Calculate transform of selected item (at center) */
  rc = cupertino_wheel_calculate_item_transform(wheel, 8, &y, &scale, &op,
                                                &angle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(y - 108.0f) < 0.01f); /* Center of 216 viewport */
  ASSERT(fabs(scale - 1.0f) < 0.01f);
  ASSERT(fabs(op - 1.0f) < 0.01f);
  ASSERT(fabs(angle - 0.0f) < 0.01f);

  /* Item far off the cylinder (fabs(theta) >= CUPERTINO_HALF_PI branch) */
  rc = cupertino_wheel_calculate_item_transform(wheel, 0, &y, &scale, &op,
                                                &angle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(-9999.0f, y);
  ASSERT_EQ(0.0f, scale);
  ASSERT_EQ(0.0f, op);

  /* Calculate transform of adjacent item */
  rc = cupertino_wheel_calculate_item_transform(wheel, 7, &y, &scale, &op,
                                                &angle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(y < 108.0f);
  ASSERT(scale < 1.0f);
  ASSERT(op < 1.0f);
  ASSERT(angle < 0.0f);

  /* Dragging within normal bounds (neither rubberband branch) */
  wheel->scroll_offset_y = 50.0f;
  rc = cupertino_wheel_drag_update(wheel, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(40.0f, wheel->scroll_offset_y);

  /* Dragging with rubber-band damping below 0 */
  wheel->scroll_offset_y = -50.0f;
  rc = cupertino_wheel_drag_update(wheel, -10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_LT(wheel->scroll_offset_y, 0.0f);

  /* Dragging with rubber-band damping above max */
  wheel->scroll_offset_y = 1000.0f;
  rc = cupertino_wheel_drag_update(wheel, -10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_GT(wheel->scroll_offset_y, 9.0f * 32.0f);

  /* Drag end with negative target_idx and > max_idx */
  wheel->scroll_offset_y = -100.0f;
  rc = cupertino_wheel_drag_end(
      wheel, 500.0f); /* projected_y = -200, target_idx = -6 < 0 */
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, wheel->target_offset_y);

  wheel->scroll_offset_y = 1500.0f;
  rc = cupertino_wheel_drag_end(wheel, -500.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(9.0f * 32.0f, wheel->target_offset_y);

  /* Tick when is_settling == 0 */
  wheel->is_settling = 0;
  rc = cupertino_wheel_tick(wheel, 16.0f, &tick);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tick with negative scroll offset and > max offset to test current_idx
   * clamping */
  wheel->scroll_offset_y = -50.0f;
  wheel->last_ticked_index = -1;
  rc = cupertino_wheel_tick(wheel, 16.0f, &tick);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, tick);
  ASSERT_EQ(0, wheel->selected_index);

  wheel->scroll_offset_y = 1500.0f;
  wheel->last_ticked_index = 0;
  rc = cupertino_wheel_tick(wheel, 16.0f, &tick);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, tick);
  ASSERT_EQ(9, wheel->selected_index);

  /* Single item wheel to test max_offset 0.0f branch */
  {
    struct cupertino_wheel *wheel1 = NULL;
    memset(&desc, 0, sizeof(desc));
    desc.item_count = 1;
    rc = cupertino_wheel_create(dummy_engine, &desc, &wheel1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_wheel_drag_start(wheel1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_wheel_drag_update(wheel1, 10.0f);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_wheel_drag_end(wheel1, 0.0f);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_wheel_destroy(wheel1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Item count shrink */
  rc = cupertino_wheel_set_item_count(wheel, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_wheel_get_item_count(wheel, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, count);
  rc = cupertino_wheel_get_selected_index(wheel, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, idx); /* Clamped */

  rc = cupertino_wheel_destroy(wheel);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_wheel_oom_mock(void) {
  struct cupertino_wheel_descriptor desc;
  struct cupertino_wheel *wheel = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.item_count = 5;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_wheel_create(dummy_engine, &desc, &wheel);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, wheel);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_wheel_create(NULL, &desc, &wheel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_wheel_suite) {
  RUN_TEST(test_wheel_invalid_arguments);
  RUN_TEST(test_wheel_lifecycle_and_selection);
  RUN_TEST(test_wheel_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_wheel_suite);
  GREATEST_MAIN_END();
}
