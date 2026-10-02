/**
 * @file test_cupertino_photo_picker.c
 * @brief Unit tests for Apple Photo Picker (PHPickerViewController) sheet.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_photo_picker.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_photo_picker_suite);

TEST test_photo_picker_invalid_arguments(void) {
  struct cupertino_photo_picker_descriptor desc;
  struct cupertino_photo_picker *picker = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_photo_filter filter;
  int val = 0;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid */
  rc = cupertino_photo_picker_create(NULL, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_create(dummy_engine, NULL, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Negative limit or out-of-range filter (both < 0 and > 3) */
  desc.selection_limit = -1;
  rc = cupertino_photo_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.selection_limit = 5;
  desc.filter = (enum cupertino_photo_filter) - 1;
  rc = cupertino_photo_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.filter = (enum cupertino_photo_filter)99;
  rc = cupertino_photo_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_photo_picker_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Add / select / clear / filter / search invalid on NULL picker */
  rc = cupertino_photo_picker_add_asset(NULL, "id1", "title1", 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_toggle_selection(NULL, 0, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_get_selected_count(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_get_selection_limit(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_clear_selection(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_set_filter(NULL, CUPERTINO_PHOTO_FILTER_ALL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_get_filter(NULL, &filter);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_filter_search(NULL, "test", &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create a valid picker for secondary invalid argument combinations */
  desc.selection_limit = 0; /* Unlimited */
  desc.filter = CUPERTINO_PHOTO_FILTER_ALL;
  rc = cupertino_photo_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, picker);

  rc = cupertino_photo_picker_add_asset(picker, NULL, "title1", 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_add_asset(picker, "id1", NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_photo_picker_add_asset(picker, "id1", "title1", 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_photo_picker_toggle_selection(picker, -1, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_toggle_selection(picker, 99, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_toggle_selection(picker, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_photo_picker_get_selected_count(picker, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_get_selection_limit(picker, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_photo_picker_set_filter(picker,
                                         (enum cupertino_photo_filter) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_set_filter(picker,
                                         (enum cupertino_photo_filter)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_photo_picker_get_filter(picker, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_photo_picker_filter_search(picker, "query", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_photo_picker_get_dimensions(picker, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_photo_picker_get_dimensions(picker, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_photo_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_photo_picker_selection_and_filters(void) {
  struct cupertino_photo_picker_descriptor desc;
  struct cupertino_photo_picker *picker = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_photo_filter filter;
  int is_selected = 0;
  int count = 0;
  int limit = 0;
  float w = 0.0f;
  float h = 0.0f;
  int i;
  char id_buf[32];
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.selection_limit = 2; /* Maximum 2 photos */
  desc.filter = CUPERTINO_PHOTO_FILTER_ALL;

  rc = cupertino_photo_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(picker != NULL);

  rc = cupertino_photo_picker_get_selection_limit(picker, &limit);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, limit);

  rc = cupertino_photo_picker_get_dimensions(picker, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w > 0.0f && h > 0.0f);

  /* Add assets: images and video */
  rc = cupertino_photo_picker_add_asset(picker, "IMG_001", "Sunset Beach", 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_photo_picker_add_asset(picker, "MOV_002", "Surfing Clip", 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_photo_picker_add_asset(picker, "IMG_003", "Mountain Peak", 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Select 1st asset */
  rc = cupertino_photo_picker_toggle_selection(picker, 0, &is_selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_selected);
  ASSERT_EQ(1, picker->assets[0].selection_order);

  /* Select 2nd asset */
  rc = cupertino_photo_picker_toggle_selection(picker, 1, &is_selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_selected);
  ASSERT_EQ(2, picker->assets[1].selection_order);

  /* Attempting to select 3rd asset exceeds limit (2) */
  rc = cupertino_photo_picker_toggle_selection(picker, 2, &is_selected);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_photo_picker_get_selected_count(picker, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, count);

  /* Deselect 1st asset */
  rc = cupertino_photo_picker_toggle_selection(picker, 0, &is_selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_selected);
  /* 2nd asset order should re-index to 1 */
  ASSERT_EQ(1, picker->assets[1].selection_order);

  /* Filter: Images only -> MOV_002 becomes hidden, IMG_001 & IMG_003 visible */
  rc = cupertino_photo_picker_set_filter(picker, CUPERTINO_PHOTO_FILTER_IMAGES);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_photo_picker_get_filter(picker, &filter);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_PHOTO_FILTER_IMAGES, filter);
  ASSERT_EQ(0, picker->assets[1].is_visible);
  ASSERT_EQ(1, picker->assets[0].is_visible);

  /* Search filtering with IMAGES filter active */
  rc = cupertino_photo_picker_filter_search(picker, "Beach", &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, count);
  ASSERT_EQ(1, picker->assets[0].is_visible);
  ASSERT_EQ(0, picker->assets[1].is_visible);
  ASSERT_EQ(0, picker->assets[2].is_visible);

  /* Search filtering for query with empty string "" */
  rc = cupertino_photo_picker_filter_search(picker, "", &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, count); /* IMG_001 and IMG_003 match */

  /* Filter: Videos only -> IMG_001 & IMG_003 hidden, MOV_002 visible */
  rc = cupertino_photo_picker_set_filter(picker, CUPERTINO_PHOTO_FILTER_VIDEOS);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, picker->assets[1].is_visible);
  ASSERT_EQ(0, picker->assets[0].is_visible);
  ASSERT_EQ(0, picker->assets[2].is_visible);

  /* Search filtering with VIDEOS filter active */
  rc = cupertino_photo_picker_filter_search(picker, "Clip", &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, count);
  ASSERT_EQ(1, picker->assets[1].is_visible);

  /* Search with non-matching query under VIDEOS */
  rc = cupertino_photo_picker_filter_search(picker, "NonExistent", &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, count);
  ASSERT_EQ(0, picker->assets[1].is_visible);

  /* Filter: Live Photos (else branch for other filter types) */
  rc = cupertino_photo_picker_set_filter(picker,
                                         CUPERTINO_PHOTO_FILTER_LIVE_PHOTOS);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, picker->assets[0].is_visible);
  ASSERT_EQ(1, picker->assets[1].is_visible);
  ASSERT_EQ(1, picker->assets[2].is_visible);

  /* Search with LIVE_PHOTOS filter */
  rc = cupertino_photo_picker_filter_search(picker, "Sunset", &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, count);
  ASSERT_EQ(1, picker->assets[0].is_visible);
  ASSERT_EQ(0, picker->assets[1].is_visible);

  /* Search filtering with ALL */
  rc = cupertino_photo_picker_set_filter(picker, CUPERTINO_PHOTO_FILTER_ALL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_photo_picker_filter_search(picker, "Mountain", &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, count);
  ASSERT_EQ(1, picker->assets[2].is_visible);
  ASSERT_EQ(0, picker->assets[0].is_visible);

  /* Clear search with NULL */
  rc = cupertino_photo_picker_filter_search(picker, NULL, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, count);

  /* Clear selection */
  rc = cupertino_photo_picker_clear_selection(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_photo_picker_get_selected_count(picker, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, count);

  /* Test unlimited selection toggle (selection_limit = 0) */
  picker->selection_limit = 0;
  rc = cupertino_photo_picker_toggle_selection(picker, 0, &is_selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_selected);
  rc = cupertino_photo_picker_toggle_selection(picker, 1, &is_selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_selected);
  rc = cupertino_photo_picker_toggle_selection(picker, 2, &is_selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_selected);

  /* Exceed asset capacity */
  while (picker->asset_count < CUPERTINO_PHOTO_PICKER_MAX_ASSETS) {
    snprintf(id_buf, sizeof(id_buf), "ID_%d", picker->asset_count);
    rc = cupertino_photo_picker_add_asset(picker, id_buf, "Asset", 0);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = cupertino_photo_picker_add_asset(picker, "Overflow", "Overflow", 0);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  rc = cupertino_photo_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_photo_picker_oom_mock(void) {
  struct cupertino_photo_picker_descriptor desc;
  struct cupertino_photo_picker *picker = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_photo_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, picker);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_photo_picker_create(NULL, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_photo_picker_suite) {
  RUN_TEST(test_photo_picker_invalid_arguments);
  RUN_TEST(test_photo_picker_selection_and_filters);
  RUN_TEST(test_photo_picker_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_photo_picker_suite);
  GREATEST_MAIN_END();
}
