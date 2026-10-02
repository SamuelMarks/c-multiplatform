/**
 * @file test_cupertino_sheet.c
 * @brief Unit tests for Cupertino Sheet & Action Sheet component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_sheet.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_sheet_suite);

TEST test_sheet_invalid_args(void) {
  struct cupertino_sheet_descriptor desc;
  struct cupertino_sheet *sheet = NULL;
  struct ui_bottom_sheet_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_sheet_detent detent;
  float val = 0.0f;
  float scale = 0.0f;
  float radius = 0.0f;
  size_t count = 0;
  size_t idx = 0;
  int is_open = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Create invalid */
  rc = cupertino_sheet_create(NULL, &desc, &sheet);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_create(dummy_engine, NULL, &sheet);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy invalid */
  rc = cupertino_sheet_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / is open */
  rc = cupertino_sheet_set_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_is_open(NULL, &is_open);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Detent */
  rc = cupertino_sheet_set_detent(NULL, CUPERTINO_SHEET_DETENT_MEDIUM);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_get_detent(NULL, &detent);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Custom detent height */
  rc = cupertino_sheet_set_custom_detent_height(NULL, 200.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Computed height */
  rc = cupertino_sheet_get_computed_height(NULL, 800.0f, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Parent card scale */
  rc = cupertino_sheet_get_parent_card_scale(NULL, 0.5f, &scale, &radius);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Add action / cancel */
  rc = cupertino_sheet_add_action(NULL, "Action", 0, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_set_cancel_action(NULL, "Cancel");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Action count / base */
  rc = cupertino_sheet_get_action_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_sheet_bottom_sheet_mode(void) {
  struct cupertino_sheet_descriptor desc;
  struct cupertino_sheet *sheet = NULL;
  struct ui_bottom_sheet_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_sheet_detent detent;
  float height = 0.0f;
  float scale = 0.0f;
  float radius = 0.0f;
  int is_open = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.mode = CUPERTINO_SHEET_MODE_BOTTOM_SHEET;
  desc.detent = CUPERTINO_SHEET_DETENT_MEDIUM;
  desc.show_drag_grabber = 1;

  rc = cupertino_sheet_create(dummy_engine, &desc, &sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sheet != NULL);

  /* Check base */
  rc = cupertino_sheet_get_base(sheet, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Base NULL check */
  rc = cupertino_sheet_get_base(sheet, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Check initial detent = Medium */
  rc = cupertino_sheet_get_detent(sheet, &detent);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SHEET_DETENT_MEDIUM, detent);

  /* get_detent NULL check */
  rc = cupertino_sheet_get_detent(sheet, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Invalid detents on set_detent */
  rc = cupertino_sheet_set_detent(sheet, (enum cupertino_sheet_detent) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_set_detent(sheet, (enum cupertino_sheet_detent)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Medium detent on 800pt viewport = 400pt */
  rc = cupertino_sheet_get_computed_height(sheet, 800.0f, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(height > 399.9f && height < 400.1f);

  /* Switch to Large detent = 92% of 800 = 736pt */
  rc = cupertino_sheet_set_detent(sheet, CUPERTINO_SHEET_DETENT_LARGE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_sheet_get_computed_height(sheet, 800.0f, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(height > 735.9f && height < 736.1f);

  /* Switch to Custom detent without custom_detent_height set (fallback 50%) */
  sheet->custom_detent_height = 0.0f;
  rc = cupertino_sheet_set_detent(sheet, CUPERTINO_SHEET_DETENT_CUSTOM);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_sheet_get_computed_height(sheet, 800.0f, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(height > 399.9f && height < 400.1f);

  /* Switch to Custom detent with height set */
  rc = cupertino_sheet_set_custom_detent_height(sheet, 320.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_sheet_get_computed_height(sheet, 800.0f, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(height > 319.9f && height < 320.1f);

  /* get_computed_height NULL and invalid viewport checks */
  rc = cupertino_sheet_get_computed_height(sheet, 800.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_get_computed_height(sheet, 0.0f, &height);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_get_computed_height(sheet, -10.0f, &height);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Invalid custom detent height */
  rc = cupertino_sheet_set_custom_detent_height(sheet, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_set_custom_detent_height(sheet, -10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Parent card scale */
  /* Progress 0.0 -> scale 1.0, radius 0 */
  rc = cupertino_sheet_get_parent_card_scale(sheet, 0.0f, &scale, &radius);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scale > 0.999f);
  ASSERT(radius < 0.001f);

  /* Progress 1.0 -> scale 0.92, radius 12.0 */
  rc = cupertino_sheet_get_parent_card_scale(sheet, 1.0f, &scale, &radius);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scale > 0.919f && scale < 0.921f);
  ASSERT(radius > 11.9f && radius < 12.1f);

  /* Progress out of range and null checks */
  rc = cupertino_sheet_get_parent_card_scale(sheet, -0.1f, &scale, &radius);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_get_parent_card_scale(sheet, 1.1f, &scale, &radius);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_get_parent_card_scale(sheet, 0.5f, NULL, &radius);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_get_parent_card_scale(sheet, 0.5f, &scale, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Open / close state */
  rc = cupertino_sheet_is_open(sheet, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  rc = cupertino_sheet_is_open(sheet, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_sheet_set_open(sheet, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_sheet_is_open(sheet, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_open);

  rc = cupertino_sheet_set_open(sheet, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_sheet_is_open(sheet, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_cupertino_sheet_mock_base_set_open_fail;
    extern int g_cupertino_sheet_mock_base_destroy_fail;

    g_cupertino_sheet_mock_base_set_open_fail = 1;
    rc = cupertino_sheet_set_open(sheet, 1);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_cupertino_sheet_mock_base_set_open_fail = 0;

    g_cupertino_sheet_mock_base_destroy_fail = 1;
    rc = cupertino_sheet_destroy(sheet);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_cupertino_sheet_mock_base_destroy_fail = 0;
  }
#else
  rc = cupertino_sheet_destroy(sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#endif

  /* Test when sheet->base is NULL in set_open and destroy */
  {
    struct cupertino_sheet *sheet_no_base = NULL;
    struct ui_bottom_sheet_base *saved_base = NULL;
    rc = cupertino_sheet_create(dummy_engine, &desc, &sheet_no_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(sheet_no_base != NULL);

    saved_base = sheet_no_base->base;
    sheet_no_base->base = NULL;

    rc = cupertino_sheet_set_open(sheet_no_base, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = cupertino_sheet_destroy(sheet_no_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    ui_bottom_sheet_base_destroy(saved_base);
  }

  PASS();
}

TEST test_sheet_action_sheet_mode(void) {
  struct cupertino_sheet_descriptor desc;
  struct cupertino_sheet *sheet = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  size_t idx = 0;
  size_t count = 0;
  float height = 0.0f;
  ui_error_t rc;

  /* Test action sheet without title or message */
  memset(&desc, 0, sizeof(desc));
  desc.mode = CUPERTINO_SHEET_MODE_ACTION_SHEET;
  rc = cupertino_sheet_create(dummy_engine, &desc, &sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sheet != NULL);
  rc = cupertino_sheet_get_computed_height(sheet, 800.0f, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(16.0f, height, 0.01f);
  rc = cupertino_sheet_destroy(sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test action sheet with title only */
  memset(&desc, 0, sizeof(desc));
  desc.mode = CUPERTINO_SHEET_MODE_ACTION_SHEET;
  desc.title = "Action Only Title";
  rc = cupertino_sheet_create(dummy_engine, &desc, &sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_sheet_get_computed_height(sheet, 800.0f, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(76.0f, height, 0.01f);
  rc = cupertino_sheet_destroy(sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test action sheet with message only */
  memset(&desc, 0, sizeof(desc));
  desc.mode = CUPERTINO_SHEET_MODE_ACTION_SHEET;
  desc.message = "Message Only";
  rc = cupertino_sheet_create(dummy_engine, &desc, &sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_sheet_get_computed_height(sheet, 800.0f, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(76.0f, height, 0.01f);
  rc = cupertino_sheet_destroy(sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Full action sheet with both title and message */
  memset(&desc, 0, sizeof(desc));
  desc.mode = CUPERTINO_SHEET_MODE_ACTION_SHEET;
  desc.title = "Select Option";
  desc.message = "Choose an action from the list below.";

  rc = cupertino_sheet_create(dummy_engine, &desc, &sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sheet != NULL);

  /* Add actions with out_index == NULL */
  rc = cupertino_sheet_add_action(sheet, "Info", 0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add actions with out_index */
  rc = cupertino_sheet_add_action(sheet, "Edit", 0, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);

  rc = cupertino_sheet_add_action(sheet, "Delete", 1, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, idx);

  /* Add action null checks */
  rc = cupertino_sheet_add_action(NULL, "Delete", 1, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_add_action(sheet, NULL, 1, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_sheet_set_cancel_action(sheet, "Cancel");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_sheet_get_action_count(sheet, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, count);

  /* get_action_count null check */
  rc = cupertino_sheet_get_action_count(sheet, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Computed height: 16 (margin) + 60 (header) + 3*56 (actions) + 64 (cancel) =
   * 308 */
  rc = cupertino_sheet_get_computed_height(sheet, 800.0f, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(height > 307.9f && height < 308.1f);

  /* Clear cancel */
  rc = cupertino_sheet_set_cancel_action(sheet, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_sheet_get_computed_height(sheet, 800.0f, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(height > 243.9f && height < 244.1f);

  /* Fill actions to max */
  while (sheet->action_count < CUPERTINO_SHEET_MAX_ACTIONS) {
    rc = cupertino_sheet_add_action(sheet, "Item", 0, &idx);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* Exceeding max actions */
  rc = cupertino_sheet_add_action(sheet, "Overflow", 0, &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* Destroy */
  rc = cupertino_sheet_destroy(sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_sheet_oom_mock(void) {
  struct cupertino_sheet_descriptor desc;
  struct cupertino_sheet *sheet = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  /* Fail malloc for struct cupertino_sheet */
  g_malloc_fail_countdown = 0;
  rc = cupertino_sheet_create(dummy_engine, &desc, &sheet);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(sheet == NULL);

  /* Fail malloc for ui_bottom_sheet_base */
  g_malloc_fail_countdown = 1;
  rc = cupertino_sheet_create(dummy_engine, &desc, &sheet);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(sheet == NULL);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_sheet_create(NULL, &desc, &sheet);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_sheet_passthrough_detents(void) {
  struct cupertino_sheet_descriptor desc;
  struct cupertino_sheet *sheet = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  enum cupertino_sheet_detent detent = CUPERTINO_SHEET_DETENT_LARGE;
  int enabled = 0;
  int active = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.mode = CUPERTINO_SHEET_MODE_BOTTOM_SHEET;
  desc.detent = CUPERTINO_SHEET_DETENT_MEDIUM;

  rc = cupertino_sheet_create(dummy_engine, &desc, &sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Initially disabled */
  rc = cupertino_sheet_get_largest_undimmed_detent(sheet, &detent, &enabled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SHEET_DETENT_MEDIUM, detent);
  ASSERT_EQ(0, enabled);

  rc = cupertino_sheet_is_passthrough_active(sheet, &active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, active); /* Sheet is not open yet */

  /* Open sheet with passthrough enabled at Medium detent */
  rc = cupertino_sheet_set_largest_undimmed_detent(
      sheet, CUPERTINO_SHEET_DETENT_MEDIUM, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_sheet_set_open(sheet, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_sheet_is_passthrough_active(sheet, &active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, active);

  /* Passthrough when open but passthrough_enabled is 0 */
  sheet->passthrough_enabled = 0;
  rc = cupertino_sheet_is_passthrough_active(sheet, &active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, active);
  sheet->passthrough_enabled = 1;

  /* Expand to Large detent (exceeds Medium) -> passthrough becomes inactive */
  rc = cupertino_sheet_set_detent(sheet, CUPERTINO_SHEET_DETENT_LARGE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_sheet_is_passthrough_active(sheet, &active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, active);

  /* Null and range checks for largest_undimmed_detent */
  rc = cupertino_sheet_set_largest_undimmed_detent(
      NULL, CUPERTINO_SHEET_DETENT_MEDIUM, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_set_largest_undimmed_detent(
      sheet, (enum cupertino_sheet_detent) - 1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_set_largest_undimmed_detent(
      sheet, (enum cupertino_sheet_detent)99, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_sheet_get_largest_undimmed_detent(NULL, &detent, &enabled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_get_largest_undimmed_detent(sheet, NULL, &enabled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_get_largest_undimmed_detent(sheet, &detent, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_is_passthrough_active(NULL, &active);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_sheet_is_passthrough_active(sheet, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_sheet_destroy(sheet);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(cupertino_sheet_suite) {
  RUN_TEST(test_sheet_invalid_args);
  RUN_TEST(test_sheet_bottom_sheet_mode);
  RUN_TEST(test_sheet_action_sheet_mode);
  RUN_TEST(test_sheet_passthrough_detents);
  RUN_TEST(test_sheet_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_sheet_suite);
  GREATEST_MAIN_END();
}
