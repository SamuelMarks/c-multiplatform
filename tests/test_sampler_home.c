/**
 * @file test_sampler_home.c
 * @brief Unit tests for sampler home screen.
 */

#include "greatest.h"
#include "sampler/sampler_home.h"
#include "ui_internal_mem.h"

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

enum greatest_test_res test_sampler_home_create(void) {
  struct sampler_home *screen = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_engine *dummy_engine = NULL;
  struct ui_engine_config engine_cfg;
  struct sampler_nav *nav = NULL;
  sampler_error_t rc;
  ui_error_t u_rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  u_rc = ui_engine_create(&engine_cfg, &dummy_engine);
  ASSERT_EQ(UI_ERROR_NONE, u_rc);
  rc = sampler_nav_create(&nav);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);

  /* Null checks */
  rc = sampler_home_create(NULL, nav, &screen);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_home_create(dummy_engine, NULL, &screen);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_home_create(dummy_engine, nav, NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);

  rc = sampler_home_get_root(NULL, &root);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_home_destroy(NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);

  /* Valid creation */
  rc = sampler_home_create(dummy_engine, nav, &screen);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(screen != NULL);

  rc = sampler_home_get_root(screen, &root);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(root != NULL);

  rc = sampler_home_destroy(&screen);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(screen == NULL);

  sampler_nav_destroy(&nav);
  ui_engine_destroy(dummy_engine);

  PASS();
}

SUITE(sampler_home_suite) { RUN_TEST(test_sampler_home_create); }
