/**
 * @file test_cupertino_color_well.c
 * @brief Unit tests for Cupertino Color Well Swatch Control.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_color_well.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_color_well_suite);

TEST test_color_well_invalid_arguments(void) {
  struct cupertino_color_well_descriptor desc;
  struct cupertino_color_well *well = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_color_t color;
  int is_open = 0;
  float w, h;
  union ui_signal_payload payload;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  memset(&payload, 0, sizeof(payload));

  /* Creation invalid */
  rc = cupertino_color_well_create(NULL, &desc, &well);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_color_well_create(dummy_engine, NULL, &well);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_color_well_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_color_well_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Get / set color */
  rc = cupertino_color_well_set_color(NULL, 0xFF00FF00);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_color_well_get_color(NULL, &color);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Panel open / close / check */
  rc = cupertino_color_well_open_panel(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_color_well_close_panel(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_color_well_is_panel_open(NULL, &is_open);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Drag operations */
  rc = cupertino_color_well_start_drag(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_color_well_end_drag(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Dimensions & CVA */
  rc = cupertino_color_well_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_color_well_get_cva(NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid well with NULL output pointers */
  rc = cupertino_color_well_create(dummy_engine, &desc, &well);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(well != NULL);

  rc = cupertino_color_well_get_color(well, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_color_well_is_panel_open(well, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_color_well_get_dimensions(well, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_color_well_get_dimensions(well, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_color_well_get_cva(well, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_color_well_get_cva(well, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva->write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cva->set_disabled_state(NULL, UI_TRUE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cva->set_disabled_state(cva->component, (ui_bool_t)42);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_color_well_destroy(well);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_color_well_lifecycle_and_cva(void) {
  struct cupertino_color_well_descriptor desc;
  struct cupertino_color_well *well = NULL;
  struct cupertino_color_well *well2 = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_color_picker_base *saved_base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_color_t color;
  ui_color_t new_color = UI_COLOR_ARGB(255, 0, 122, 255);
  int is_open = 0;
  float w, h;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.initial_color = UI_COLOR_ARGB(255, 255, 59, 48); /* Red */

  rc = cupertino_color_well_create(dummy_engine, &desc, &well);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(well != NULL);

  rc = cupertino_color_well_get_color(well, &color);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(desc.initial_color, color);

  /* Set color directly */
  rc = cupertino_color_well_set_color(well, new_color);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_color_well_get_color(well, &color);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(new_color, color);

  rc = cupertino_color_well_get_dimensions(well, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_COLOR_WELL_DEFAULT_WIDTH, w);
  ASSERT_EQ(CUPERTINO_COLOR_WELL_DEFAULT_HEIGHT, h);

  /* Panel open and close */
  rc = cupertino_color_well_is_panel_open(well, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  rc = cupertino_color_well_open_panel(well);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_color_well_is_panel_open(well, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_open);

  rc = cupertino_color_well_close_panel(well);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_color_well_is_panel_open(well, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  /* Drag operations */
  rc = cupertino_color_well_start_drag(well);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, well->is_dragging);

  rc = cupertino_color_well_end_drag(well);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, well->is_dragging);

  /* CVA integration */
  rc = cupertino_color_well_get_cva(well, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(cva != NULL);

  {
    union ui_signal_payload payload;
    memset(&payload, 0, sizeof(payload));
    payload.int_val = (ui_int32)new_color;
    rc = cva->write_value(cva->component, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = cupertino_color_well_get_color(well, &color);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(new_color, color);

  rc = cva->set_disabled_state(cva->component, UI_FALSE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva->set_disabled_state(cva->component, UI_TRUE);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Custom dimensions */
  desc.width = 50.0f;
  desc.height = 35.0f;
  rc = cupertino_color_well_create(dummy_engine, &desc, &well2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_color_well_get_dimensions(well2, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(50.0f, w);
  ASSERT_EQ(35.0f, h);

  /* Destroy well2 with base == NULL */
  saved_base = well2->base;
  well2->base = NULL;
  rc = cupertino_color_well_destroy(well2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_color_picker_base_destroy(saved_base);

  rc = cupertino_color_well_destroy(well);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_color_well_oom_mock(void) {
  struct cupertino_color_well_descriptor desc;
  struct cupertino_color_well *well = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_color_well_create(dummy_engine, &desc, &well);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, well);

  g_malloc_fail_countdown = 1;
  rc = cupertino_color_well_create(dummy_engine, &desc, &well);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, well);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_color_well_create(NULL, &desc, &well);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_color_well_suite) {
  RUN_TEST(test_color_well_invalid_arguments);
  RUN_TEST(test_color_well_lifecycle_and_cva);
  RUN_TEST(test_color_well_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_color_well_suite);
  GREATEST_MAIN_END();
}
