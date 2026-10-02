/**
 * @file test_cupertino_page_control.c
 * @brief Unit tests for Cupertino Page Control component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_page_control.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_page_control_suite);

TEST test_page_control_invalid_arguments(void) {
  struct cupertino_page_control_descriptor desc;
  struct cupertino_page_control *ctrl = NULL;
  struct ui_page_control_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float w = 0.0f;
  float h = 0.0f;
  float scale = 0.0f;
  int count = 0;
  int page = 0;
  int changed = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Invalid creation */
  rc = cupertino_page_control_create(NULL, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_create(dummy_engine, NULL, &ctrl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Negative pages or invalid page index */
  desc.number_of_pages = -1;
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.number_of_pages = 3;
  desc.current_page = 5; /* Out of bounds */
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy invalid */
  rc = cupertino_page_control_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / get number of pages */
  rc = cupertino_page_control_set_number_of_pages(NULL, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_get_number_of_pages(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / get current page */
  rc = cupertino_page_control_set_current_page(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_get_current_page(NULL, &page);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Scrubbing */
  rc = cupertino_page_control_scrub(NULL, 20.0f, &changed);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_end_scrub(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Scale / dimensions / base */
  rc = cupertino_page_control_get_dot_scale(NULL, 0, &scale);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_page_control_lifecycle_and_navigation(void) {
  struct cupertino_page_control_descriptor desc;
  struct cupertino_page_control *ctrl = NULL;
  struct ui_page_control_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  int count = 0;
  int page = 0;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.number_of_pages = 5;
  desc.current_page = 0;

  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ctrl != NULL);

  rc = cupertino_page_control_get_base(ctrl, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = cupertino_page_control_get_number_of_pages(ctrl, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5, count);

  rc = cupertino_page_control_get_current_page(ctrl, &page);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, page);

  rc = cupertino_page_control_get_dimensions(ctrl, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w > 0.0f);
  ASSERT_EQ(CUPERTINO_PAGE_CONTROL_HEIGHT, h);

  /* Change page */
  rc = cupertino_page_control_set_current_page(ctrl, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_control_get_current_page(ctrl, &page);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, page);

  /* Reject negative or out of bounds */
  rc = cupertino_page_control_set_current_page(ctrl, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_set_current_page(ctrl, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Shrink page count below current page -> clamp */
  rc = cupertino_page_control_set_number_of_pages(ctrl, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_control_get_current_page(ctrl, &page);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, page);

  rc = cupertino_page_control_destroy(ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_page_control_scrubbing_and_scaling(void) {
  struct cupertino_page_control_descriptor desc;
  struct cupertino_page_control *ctrl = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  int changed = 0;
  int page = 0;
  float scale = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  /* Test with > 10 pages for compression behavior */
  memset(&desc, 0, sizeof(desc));
  desc.number_of_pages = 15;
  desc.current_page = 0;

  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Active dot scale at distance 0 is 1.0 */
  rc = cupertino_page_control_get_dot_scale(ctrl, 0, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.0f, scale);

  /* Dot at distance 4 should be 0.66 */
  rc = cupertino_page_control_get_dot_scale(ctrl, 4, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scale > 0.65f && scale < 0.67f);

  /* Dot at distance >= 5 should be 0.33 */
  rc = cupertino_page_control_get_dot_scale(ctrl, 7, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scale > 0.32f && scale < 0.34f);

  /* Invalid dot index */
  rc = cupertino_page_control_get_dot_scale(ctrl, -1, &scale);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_get_dot_scale(ctrl, 20, &scale);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Scrubbing */
  /* Touch at beginning -> still page 0 */
  rc = cupertino_page_control_scrub(ctrl, 10.0f, &changed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, changed);

  /* Touch at page 2 (pitch = 16, start ~16.0f, so 16 + 2*16 = 48) */
  rc = cupertino_page_control_scrub(ctrl, 50.0f, &changed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, changed);
  rc = cupertino_page_control_get_current_page(ctrl, &page);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, page);

  /* Scrub beyond right edge -> clamped to last page */
  rc = cupertino_page_control_scrub(ctrl, 1000.0f, &changed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, changed);
  rc = cupertino_page_control_get_current_page(ctrl, &page);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(14, page);

  /* End scrub */
  rc = cupertino_page_control_end_scrub(ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_page_control_destroy(ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test hides_for_single_page */
  memset(&desc, 0, sizeof(desc));
  desc.number_of_pages = 1;
  desc.hides_for_single_page = 1;

  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_control_get_dimensions(ctrl, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, w);
  ASSERT_EQ(0.0f, h);

  rc = cupertino_page_control_destroy(ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_page_control_oom_mock(void) {
  struct cupertino_page_control_descriptor desc;
  struct cupertino_page_control *ctrl = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.number_of_pages = 5;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, ctrl);

  g_malloc_fail_countdown = 1;
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, ctrl);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_page_control_create(NULL, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_cupertino_page_control_mock_base_create_fail;
extern int g_cupertino_page_control_mock_base_destroy_fail;
extern int g_cupertino_page_control_mock_base_set_pages_fail;
extern int g_cupertino_page_control_mock_base_set_page_fail;
#endif

TEST test_page_control_coverage_branches(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_page_control_descriptor desc;
  struct cupertino_page_control *ctrl = NULL;
  struct ui_page_control_base *base = NULL;
  float w = 0.0f, h = 0.0f, scale = 0.0f;
  int count = 0, page = 0, changed = 0;
  ui_error_t rc;

  /* desc with number_of_pages > 0 and current_page < 0 -> invalid arg */
  memset(&desc, 0, sizeof(desc));
  desc.number_of_pages = 5;
  desc.current_page = -1;
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* desc with number_of_pages == 0 and valid current_page 0 -> success */
  memset(&desc, 0, sizeof(desc));
  desc.number_of_pages = 0;
  desc.current_page = 0;
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* set_number_of_pages with count < 0 -> invalid arg */
  rc = cupertino_page_control_set_number_of_pages(ctrl, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* set_current_page with 0 pages -> page 0 is valid */
  rc = cupertino_page_control_set_current_page(ctrl, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* set_current_page with 0 pages -> page < 0 is invalid */
  rc = cupertino_page_control_set_current_page(ctrl, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* get_dot_scale with dot_index < 0 -> invalid arg */
  rc = cupertino_page_control_get_dot_scale(ctrl, -1, &scale);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* get_dot_scale with NULL out_scale -> invalid arg */
  rc = cupertino_page_control_get_dot_scale(ctrl, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* get_dimensions with hides_for_single_page enabled but number_of_pages > 1
   */
  ctrl->hides_for_single_page = 1;
  ctrl->number_of_pages = 3;
  rc = cupertino_page_control_get_dimensions(ctrl, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w > 0.0f);
  ASSERT(h > 0.0f);

  rc = cupertino_page_control_destroy(ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ctrl = NULL;

  /* Custom tint colors */
  memset(&desc, 0, sizeof(desc));
  desc.number_of_pages = 5;
  desc.current_page = 2;
  desc.page_indicator_tint_color = UI_COLOR_ARGB(100, 200, 200, 200);
  desc.current_page_indicator_tint_color = UI_COLOR_ARGB(200, 100, 100, 100);
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(desc.page_indicator_tint_color, ctrl->page_indicator_tint_color);
  ASSERT_EQ(desc.current_page_indicator_tint_color,
            ctrl->current_page_indicator_tint_color);

  /* Set number of pages to 0 */
  rc = cupertino_page_control_set_number_of_pages(ctrl, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, ctrl->current_page);

  /* Scrub on 0 pages */
  rc = cupertino_page_control_scrub(ctrl, 50.0f, &changed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, changed);

  /* Dimensions with 0 pages */
  rc = cupertino_page_control_get_dimensions(ctrl, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, w);
  ASSERT_EQ(0.0f, h);

  /* Set number of pages to 5, current page to 4, then reduce to 3 (covers
   * current_page >= count) */
  rc = cupertino_page_control_set_number_of_pages(ctrl, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_control_set_current_page(ctrl, 4);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_control_set_number_of_pages(ctrl, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, ctrl->current_page);

  /* Dot scale when pages <= MAX_UNCOMPRESSED_DOTS (3 <= 9) */
  rc = cupertino_page_control_get_dot_scale(ctrl, 0, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.0f, scale);

  /* Null checks */
  rc = cupertino_page_control_get_number_of_pages(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_get_number_of_pages(ctrl, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_page_control_get_current_page(NULL, &page);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_get_current_page(ctrl, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_page_control_scrub(NULL, 10.0f, &changed);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_scrub(ctrl, 10.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_page_control_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_get_dimensions(ctrl, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_get_dimensions(ctrl, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_page_control_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_control_get_base(ctrl, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Operations when ctrl->base is NULL */
  ui_page_control_base_destroy(ctrl->base);
  ctrl->base = NULL;

  rc = cupertino_page_control_set_number_of_pages(ctrl, 4);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_control_set_current_page(ctrl, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_control_scrub(ctrl, 100.0f, &changed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_control_destroy(ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ctrl = NULL;

  /* Mock error paths */
#ifdef UI_TEST_MOCK_ALLOC
  /* base destroy failure in cupertino_page_control_destroy */
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_page_control_mock_base_destroy_fail = 1;
  rc = cupertino_page_control_destroy(ctrl);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_page_control_mock_base_destroy_fail = 0;

  /* set_number_of_pages failure on base */
  g_cupertino_page_control_mock_base_set_pages_fail = 1;
  rc = cupertino_page_control_set_number_of_pages(ctrl, 6);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_page_control_mock_base_set_pages_fail = 0;

  /* set_current_page failure on base in set_number_of_pages */
  g_cupertino_page_control_mock_base_set_page_fail = 1;
  rc = cupertino_page_control_set_number_of_pages(ctrl, 6);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_page_control_mock_base_set_page_fail = 0;

  /* set_current_page failure on base */
  g_cupertino_page_control_mock_base_set_page_fail = 1;
  rc = cupertino_page_control_set_current_page(ctrl, 1);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_page_control_mock_base_set_page_fail = 0;

  /* scrub set_current_page failure on base */
  g_cupertino_page_control_mock_base_set_page_fail = 1;
  rc = cupertino_page_control_scrub(ctrl, 100.0f, &changed);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_page_control_mock_base_set_page_fail = 0;

  rc = cupertino_page_control_destroy(ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ctrl = NULL;

  /* base_create failure in create */
  g_cupertino_page_control_mock_base_create_fail = 1;
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_page_control_mock_base_create_fail = 0;

  /* base_set_number_of_pages failure in create */
  g_cupertino_page_control_mock_base_set_pages_fail = 1;
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_page_control_mock_base_set_pages_fail = 0;

  /* base_set_number_of_pages failure with destroy failure in create */
  g_cupertino_page_control_mock_base_set_pages_fail = 1;
  g_cupertino_page_control_mock_base_destroy_fail = 1;
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_page_control_mock_base_set_pages_fail = 0;
  g_cupertino_page_control_mock_base_destroy_fail = 0;

  /* base_set_current_page failure in create */
  g_cupertino_page_control_mock_base_set_page_fail = 1;
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_page_control_mock_base_set_page_fail = 0;

  /* base_set_current_page failure with destroy failure in create */
  g_cupertino_page_control_mock_base_set_page_fail = 1;
  g_cupertino_page_control_mock_base_destroy_fail = 1;
  rc = cupertino_page_control_create(dummy_engine, &desc, &ctrl);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_page_control_mock_base_set_page_fail = 0;
  g_cupertino_page_control_mock_base_destroy_fail = 0;
#endif

  PASS();
}

SUITE(cupertino_page_control_suite) {
  RUN_TEST(test_page_control_invalid_arguments);
  RUN_TEST(test_page_control_lifecycle_and_navigation);
  RUN_TEST(test_page_control_scrubbing_and_scaling);
  RUN_TEST(test_page_control_oom_mock);
  RUN_TEST(test_page_control_coverage_branches);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_page_control_suite);
  GREATEST_MAIN_END();
}
