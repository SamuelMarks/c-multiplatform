/**
 * @file test_cupertino_dynamic_island.c
 * @brief Unit tests for Dynamic Island morphing container.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_dynamic_island.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_dynamic_island_suite);

TEST test_island_invalid_arguments(void) {
  struct cupertino_dynamic_island_descriptor desc;
  struct cupertino_dynamic_island *island = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_island_state st;
  enum cupertino_island_template tmpl;
  float x = 0.0f;
  float y = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  float r = 0.0f;
  int changed = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid */
  rc = cupertino_dynamic_island_create(NULL, &desc, &island);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_create(dummy_engine, NULL, &island);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range state / template */
  desc.initial_state = (enum cupertino_island_state)99;
  rc = cupertino_dynamic_island_create(dummy_engine, &desc, &island);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.initial_state = (enum cupertino_island_state) - 1;
  rc = cupertino_dynamic_island_create(dummy_engine, &desc, &island);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.initial_state = CUPERTINO_ISLAND_COMPACT;
  desc.initial_template = (enum cupertino_island_template)99;
  rc = cupertino_dynamic_island_create(dummy_engine, &desc, &island);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.initial_template = (enum cupertino_island_template) - 1;
  rc = cupertino_dynamic_island_create(dummy_engine, &desc, &island);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_dynamic_island_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* State / template / tick / bounds / touch invalid */
  rc = cupertino_dynamic_island_set_state(NULL, CUPERTINO_ISLAND_COMPACT, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_set_state(
      (struct cupertino_dynamic_island *)0x123,
      (enum cupertino_island_state) - 1, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_set_state(
      (struct cupertino_dynamic_island *)0x123, (enum cupertino_island_state)3,
      0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_dynamic_island_get_state(NULL, &st);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_get_state(
      (const struct cupertino_dynamic_island *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_dynamic_island_set_template(NULL,
                                             CUPERTINO_ISLAND_TEMPLATE_CALL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_set_template(
      (struct cupertino_dynamic_island *)0x123,
      (enum cupertino_island_template) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_set_template(
      (struct cupertino_dynamic_island *)0x123,
      (enum cupertino_island_template)5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_dynamic_island_get_template(NULL, &tmpl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_get_template(
      (const struct cupertino_dynamic_island *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_dynamic_island_tick(NULL, 16.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_tick((struct cupertino_dynamic_island *)0x123,
                                     -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_dynamic_island_get_bounds(NULL, &x, &y, &w, &h, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_get_bounds(
      (const struct cupertino_dynamic_island *)0x123, NULL, &y, &w, &h, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_get_bounds(
      (const struct cupertino_dynamic_island *)0x123, &x, NULL, &w, &h, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_get_bounds(
      (const struct cupertino_dynamic_island *)0x123, &x, &y, NULL, &h, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_get_bounds(
      (const struct cupertino_dynamic_island *)0x123, &x, &y, &w, NULL, &r);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_get_bounds(
      (const struct cupertino_dynamic_island *)0x123, &x, &y, &w, &h, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_dynamic_island_handle_touch(NULL, 0, &changed);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_handle_touch(
      (struct cupertino_dynamic_island *)0x123, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_island_morphing_and_templates(void) {
  struct cupertino_dynamic_island_descriptor desc;
  struct cupertino_dynamic_island *island = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_island_state st;
  enum cupertino_island_template tmpl;
  float x = 0.0f;
  float y = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  float r = 0.0f;
  int changed = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.initial_state = CUPERTINO_ISLAND_COMPACT;
  desc.initial_template = CUPERTINO_ISLAND_TEMPLATE_CALL;
  desc.screen_width = 393.0f;

  rc = cupertino_dynamic_island_create(dummy_engine, &desc, &island);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(island != NULL);

  rc = cupertino_dynamic_island_get_state(island, &st);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_ISLAND_COMPACT, st);

  rc = cupertino_dynamic_island_get_template(island, &tmpl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_ISLAND_TEMPLATE_CALL, tmpl);

  rc = cupertino_dynamic_island_get_bounds(island, &x, &y, &w, &h, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_ISLAND_COMPACT_WIDTH, w);
  ASSERT_EQ(CUPERTINO_ISLAND_COMPACT_HEIGHT, h);
  ASSERT_EQ(CUPERTINO_ISLAND_TOP_MARGIN, y);

  /* Transition to Minimal */
  rc = cupertino_dynamic_island_set_state(island, CUPERTINO_ISLAND_MINIMAL, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_dynamic_island_get_bounds(island, &x, &y, &w, &h, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_ISLAND_MINIMAL_SIZE, w);
  ASSERT_EQ(CUPERTINO_ISLAND_MINIMAL_SIZE, h);

  /* Transition to Expanded animated */
  rc = cupertino_dynamic_island_set_state(island, CUPERTINO_ISLAND_EXPANDED, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, island->is_animating);

  /* Tick 500ms -> finishes animation */
  rc = cupertino_dynamic_island_tick(island, 500.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_dynamic_island_get_bounds(island, &x, &y, &w, &h, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w > 300.0f);
  ASSERT_EQ(CUPERTINO_ISLAND_EXPANDED_HEIGHT, h);

  /* Change template to Media */
  rc = cupertino_dynamic_island_set_template(island,
                                             CUPERTINO_ISLAND_TEMPLATE_MEDIA);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_dynamic_island_get_template(island, &tmpl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_ISLAND_TEMPLATE_MEDIA, tmpl);

  /* Touch toggle (expanded -> compact) */
  rc = cupertino_dynamic_island_handle_touch(island, 0, &changed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, changed);
  rc = cupertino_dynamic_island_get_state(island, &st);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_ISLAND_COMPACT, st);

  /* Negative is_long_press should fail */
  rc = cupertino_dynamic_island_handle_touch(island, -1, &changed);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Touch toggle (compact -> expanded) */
  rc = cupertino_dynamic_island_handle_touch(island, 0, &changed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, changed);
  rc = cupertino_dynamic_island_get_state(island, &st);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_ISLAND_EXPANDED, st);

  /* Tick when NOT animating should be a no-op */
  island->is_animating = 0;
  rc = cupertino_dynamic_island_tick(island, 16.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_dynamic_island_destroy(island);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Create with wide screen > 400.0f and default screen_width <= 0 */
  memset(&desc, 0, sizeof(desc));
  desc.screen_width = 450.0f;
  desc.initial_state = CUPERTINO_ISLAND_EXPANDED;
  rc = cupertino_dynamic_island_create(dummy_engine, &desc, &island);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_ISLAND_EXPANDED_WIDTH, island->current_width);

  /* Test delta_ms < 400 with target diff < 1.0f */
  rc = cupertino_dynamic_island_set_state(island, CUPERTINO_ISLAND_COMPACT, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Step midway where width diff is large */
  rc = cupertino_dynamic_island_tick(island, 16.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, island->is_animating);

  /* Step with diff < 1.0f */
  island->current_width = island->target_width + 0.5f;
  island->current_height = island->target_height + 0.5f;
  rc = cupertino_dynamic_island_tick(island, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, island->is_animating);

  /* Test branch where width diff < 1.0f but height diff >= 1.0f */
  rc = cupertino_dynamic_island_set_state(island, CUPERTINO_ISLAND_EXPANDED, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  island->current_width = island->target_width + 0.2f;
  island->current_height = island->target_height + 5.0f;
  rc = cupertino_dynamic_island_tick(island, 1.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, island->is_animating);

  rc = cupertino_dynamic_island_destroy(island);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Create with screen_width <= 0 to hit default 393.0f */
  memset(&desc, 0, sizeof(desc));
  desc.screen_width = 0.0f;
  rc = cupertino_dynamic_island_create(dummy_engine, &desc, &island);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(393.0f, island->screen_width);

  rc = cupertino_dynamic_island_destroy(island);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_island_oom_mock(void) {
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_dynamic_island_mock_set_state_fail;
#endif
  struct cupertino_dynamic_island_descriptor desc;
  struct cupertino_dynamic_island *island = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  int changed = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.initial_state = CUPERTINO_ISLAND_COMPACT;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_dynamic_island_create(dummy_engine, &desc, &island);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, island);
  g_malloc_fail_countdown = -1;

  rc = cupertino_dynamic_island_create(dummy_engine, &desc, &island);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(island != NULL);

  /* Fail handle_touch in compact state */
  g_cupertino_dynamic_island_mock_set_state_fail = 1;
  rc = cupertino_dynamic_island_handle_touch(island, 0, &changed);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_dynamic_island_mock_set_state_fail = 0;

  /* Expand */
  rc = cupertino_dynamic_island_handle_touch(island, 0, &changed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_ISLAND_EXPANDED, island->state);

  /* Fail handle_touch in expanded state */
  g_cupertino_dynamic_island_mock_set_state_fail = 1;
  rc = cupertino_dynamic_island_handle_touch(island, 0, &changed);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_dynamic_island_mock_set_state_fail = 0;

  rc = cupertino_dynamic_island_destroy(island);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  rc = cupertino_dynamic_island_create(dummy_engine, NULL, &island);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dynamic_island_handle_touch(NULL, 0, &changed);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_dynamic_island_suite) {
  RUN_TEST(test_island_invalid_arguments);
  RUN_TEST(test_island_morphing_and_templates);
  RUN_TEST(test_island_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_dynamic_island_suite);
  GREATEST_MAIN_END();
}
