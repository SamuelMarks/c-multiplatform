/**
 * @file test_cupertino_popover.c
 * @brief Unit tests for Cupertino Popover.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_popover.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_popover_suite);

TEST test_popover_invalid_arguments(void) {
  struct cupertino_popover_descriptor desc;
  struct cupertino_popover *popover = NULL;
  struct ui_popover_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_arrow_direction dir;
  int is_pres = 0;
  float x, y, w, h, offset;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid */
  rc = cupertino_popover_create(NULL, &desc, &popover);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_create(dummy_engine, NULL, &popover);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range permitted arrows */
  desc.permitted_arrows = (enum cupertino_arrow_direction) - 1;
  rc = cupertino_popover_create(dummy_engine, &desc, &popover);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.permitted_arrows = (enum cupertino_arrow_direction)5;
  rc = cupertino_popover_create(dummy_engine, &desc, &popover);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_popover_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Present / dismiss / bounds / arrow / base invalid with NULL popover */
  rc = cupertino_popover_present(NULL, 10.0f, 10.0f, 40.0f, 40.0f, 320.0f,
                                 480.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_dismiss(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_is_presented(NULL, &is_pres);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_get_bounds(NULL, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_get_arrow_position(NULL, &dir, &offset);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid popover with NULL out pointers and invalid presentation args */
  desc.permitted_arrows = CUPERTINO_ARROW_DIRECTION_ANY;
  rc = cupertino_popover_create(dummy_engine, &desc, &popover);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(popover != NULL);

  rc = cupertino_popover_is_presented(popover, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_get_bounds(popover, NULL, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_get_bounds(popover, &x, NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_get_bounds(popover, &x, &y, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_get_bounds(popover, &x, &y, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_get_arrow_position(popover, NULL, &offset);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_get_arrow_position(popover, &dir, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_get_base(popover, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_popover_present(popover, 10.0f, 10.0f, -1.0f, 40.0f, 320.0f,
                                 480.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_present(popover, 10.0f, 10.0f, 40.0f, -1.0f, 320.0f,
                                 480.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_present(popover, 10.0f, 10.0f, 40.0f, 40.0f, 0.0f,
                                 480.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popover_present(popover, 10.0f, 10.0f, 40.0f, 40.0f, 320.0f,
                                 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_popover_destroy(popover);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_popover_presentation_and_positioning(void) {
  struct cupertino_popover_descriptor desc;
  struct cupertino_popover *popover = NULL;
  struct cupertino_popover *popover2 = NULL;
  struct ui_popover_base *base = NULL;
  struct ui_popover_base *saved_base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_arrow_direction dir;
  int is_pres = 0;
  float x, y, w, h, offset;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.content_width = 200.0f;
  desc.content_height = 100.0f;
  desc.permitted_arrows = CUPERTINO_ARROW_DIRECTION_ANY;

  rc = cupertino_popover_create(dummy_engine, &desc, &popover);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(popover != NULL);

  rc = cupertino_popover_get_base(popover, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = cupertino_popover_is_presented(popover, &is_pres);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_pres);

  /* Present below anchor (y = 50, screen_h = 500) -> fits below, arrow points
   * UP */
  rc = cupertino_popover_present(popover, 100.0f, 50.0f, 40.0f, 30.0f, 320.0f,
                                 500.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popover_is_presented(popover, &is_pres);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_pres);

  rc = cupertino_popover_get_arrow_position(popover, &dir, &offset);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_ARROW_DIRECTION_UP, dir);
  ASSERT(offset > 0.0f);

  rc = cupertino_popover_get_bounds(popover, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(200.0f, w);
  ASSERT_EQ(100.0f + CUPERTINO_POPOVER_ARROW_HEIGHT, h);
  ASSERT_EQ(80.0f, y); /* anchor_y (50) + anchor_h (30) */

  /* Present near bottom (y = 450, screen_h = 500) -> overflows below, flips
   * above, arrow points DOWN */
  rc = cupertino_popover_present(popover, 100.0f, 450.0f, 40.0f, 30.0f, 320.0f,
                                 500.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popover_get_arrow_position(popover, &dir, &offset);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_ARROW_DIRECTION_DOWN, dir);

  /* Fallback: neither above nor below fits in screen_h = 120 */
  rc = cupertino_popover_present(popover, 100.0f, 50.0f, 40.0f, 30.0f, 320.0f,
                                 120.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popover_get_bounds(popover, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10.0f, y);

  /* Clamping left and arrow_offset < min_offset */
  rc = cupertino_popover_present(popover, -150.0f, 50.0f, 40.0f, 30.0f, 500.0f,
                                 500.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popover_get_bounds(popover, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10.0f, x);

  /* Clamping right and arrow_offset > max_offset */
  rc = cupertino_popover_present(popover, 600.0f, 50.0f, 40.0f, 30.0f, 500.0f,
                                 500.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popover_get_bounds(popover, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(500.0f - 10.0f - 200.0f, x);

  /* Dismiss */
  rc = cupertino_popover_dismiss(popover);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popover_is_presented(popover, &is_pres);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_pres);

  rc = cupertino_popover_destroy(popover);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Default dimension fallback and base == NULL destroy */
  memset(&desc, 0, sizeof(desc));
  desc.content_width = -1.0f;
  desc.content_height = 0.0f;
  desc.permitted_arrows = CUPERTINO_ARROW_DIRECTION_ANY;
  rc = cupertino_popover_create(dummy_engine, &desc, &popover2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(popover2 != NULL);
  ASSERT_EQ(CUPERTINO_POPOVER_DEFAULT_WIDTH, popover2->content_width);
  ASSERT_EQ(CUPERTINO_POPOVER_DEFAULT_HEIGHT, popover2->content_height);

  saved_base = popover2->base;
  popover2->base = NULL;
  rc = cupertino_popover_destroy(popover2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_popover_base_destroy(saved_base);

  PASS();
}

TEST test_popover_oom_mock(void) {
  struct cupertino_popover_descriptor desc;
  struct cupertino_popover *popover = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_popover_create(dummy_engine, &desc, &popover);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, popover);

  g_malloc_fail_countdown = 1;
  rc = cupertino_popover_create(dummy_engine, &desc, &popover);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, popover);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_popover_create(NULL, &desc, &popover);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_popover_suite) {
  RUN_TEST(test_popover_invalid_arguments);
  RUN_TEST(test_popover_presentation_and_positioning);
  RUN_TEST(test_popover_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_popover_suite);
  GREATEST_MAIN_END();
}
