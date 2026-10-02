/**
 * @file test_cupertino_init.c
 * @brief Unit tests for Cupertino design system lifecycle and registration.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_init.h"
#include "cupertino/cupertino_tokens.h"
#include "ui_engine.h"
/* clang-format on */

SUITE(cupertino_init_suite);

TEST test_cupertino_lifecycle(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  const struct ui_design_system_vtable *vtable = NULL;
  struct ui_design_token token;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  config.num_threads = 1;

  /* Invalid arguments check */
  rc = cupertino_initialize(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_get_vtable(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Get vtable */
  rc = cupertino_get_vtable(&vtable);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(vtable != NULL);
  ASSERT(vtable->init != NULL);
  ASSERT(vtable->shutdown != NULL);
  ASSERT(vtable->apply_theme != NULL);
  ASSERT(vtable->resolve_token != NULL);

  /* Create engine to test initialization */
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(engine != NULL);

  /* Initialize cupertino and register with engine */
  rc = cupertino_initialize(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Vtable methods checks */
  rc = vtable->init(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = vtable->init(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Apply themes */
  rc = vtable->apply_theme(NULL, "ios_light");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = vtable->apply_theme(engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = vtable->apply_theme(engine, "invalid_theme");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = vtable->apply_theme(engine, "ios_light");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = vtable->apply_theme(engine, "ios_dark");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = vtable->resolve_token("--apple-system-blue", &token);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = vtable->apply_theme(engine, "macos_light");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = vtable->apply_theme(engine, "macos_dark");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = vtable->resolve_token("--apple-system-blue", &token);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = vtable->apply_theme(engine, "visionos");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = vtable->resolve_token("--apple-system-blue", &token);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Token resolution via vtable */
  rc = vtable->resolve_token(NULL, &token);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = vtable->resolve_token("--apple-system-blue", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = vtable->resolve_token("nonexistent_token", &token);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  rc = vtable->resolve_token("--apple-system-blue", &token);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_TOKEN_TYPE_COLOR, token.type);

  rc = vtable->resolve_token("--apple-system-green", &token);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = vtable->resolve_token("--apple-system-red", &token);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = vtable->resolve_token("--apple-separator", &token);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Vtable shutdown */
  rc = vtable->shutdown(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = vtable->shutdown(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Shutdown cupertino */
  rc = cupertino_shutdown();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Secondary shutdown is idempotent */
  rc = cupertino_shutdown();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_cupertino_init_mock_errors(void) {
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_init_mock_register_fail;
  extern int g_cupertino_init_mock_unregister_fail;
  extern int g_cupertino_init_mock_get_color_fail;
#endif
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  const struct ui_design_system_vtable *vtable = NULL;
  struct ui_design_token token;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  config.num_threads = 1;

  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(engine != NULL);

  rc = cupertino_get_vtable(&vtable);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(vtable != NULL);

#ifdef UI_TEST_MOCK_ALLOC
  /* Fail ui_design_system_register during cupertino_initialize */
  g_cupertino_init_mock_register_fail = 1;
  rc = cupertino_initialize(engine);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_init_mock_register_fail = 0;

  /* Initialize successfully */
  rc = cupertino_initialize(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test failure of get_system_color for blue, green, and red tokens */
  g_cupertino_init_mock_get_color_fail = 1;
  rc = vtable->resolve_token("--apple-system-blue", &token);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = vtable->resolve_token("--apple-system-green", &token);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = vtable->resolve_token("--apple-system-red", &token);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_init_mock_get_color_fail = 0;

  /* Fail ui_design_system_unregister during cupertino_shutdown */
  g_cupertino_init_mock_unregister_fail = 1;
  rc = cupertino_shutdown();
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_init_mock_unregister_fail = 0;

  rc = cupertino_shutdown();
  ASSERT_EQ(UI_ERROR_NONE, rc);
#endif

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(cupertino_init_suite) {
  RUN_TEST(test_cupertino_lifecycle);
  RUN_TEST(test_cupertino_init_mock_errors);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_init_suite);
  GREATEST_MAIN_END();
}
