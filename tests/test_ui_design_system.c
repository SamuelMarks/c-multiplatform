/* clang-format off */
#include "greatest.h"
#include "ui_design_system.h"
#include "ui_error.h"
#include <string.h>
#ifdef UI_TEST_MOCK_ALLOC
#include "ui_test_mock_mem.h"
#endif
/* clang-format on */

static int g_mock_init_called = 0;
static int g_mock_init_fail = 0;
static int g_mock_shutdown_called = 0;

static ui_error_t mock_ds_init(struct ui_engine *engine) {
  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  g_mock_init_called++;
  if (g_mock_init_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return UI_ERROR_NONE;
}

static ui_error_t mock_ds_shutdown(struct ui_engine *engine) {
  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  g_mock_shutdown_called++;
  return UI_ERROR_NONE;
}

static ui_error_t mock_ds_apply_theme(struct ui_engine *engine,
                                      const char *theme_name) {
  if (!engine || !theme_name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

static ui_error_t mock_ds_resolve_token(const char *token_name,
                                        struct ui_design_token *out_token) {
  if (!token_name || !out_token) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NOT_FOUND;
}

static const struct ui_design_system_vtable g_test_vtable_1 = {
    mock_ds_init, mock_ds_shutdown, mock_ds_apply_theme, mock_ds_resolve_token};

static const struct ui_design_system_vtable g_test_vtable_2 = {
    mock_ds_init, mock_ds_shutdown, mock_ds_apply_theme, mock_ds_resolve_token};

static const struct ui_design_system_vtable g_test_vtable_no_init = {
    NULL, mock_ds_shutdown, mock_ds_apply_theme, mock_ds_resolve_token};

TEST test_design_system_invalid_args(void) {
  const struct ui_design_system_vtable *out_vtable = NULL;
  ui_error_t rc;

  /* Register NULL checks */
  rc = ui_design_system_register(NULL, &g_test_vtable_1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_design_system_register("test", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Unregister NULL checks */
  rc = ui_design_system_unregister(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Get NULL checks */
  rc = ui_design_system_get(NULL, &out_vtable);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_design_system_get("test", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set active NULL checks */
  rc = ui_design_system_set_active(NULL, "test");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_design_system_set_active((struct ui_engine *)1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Get active NULL checks */
  rc = ui_design_system_get_active(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_design_system_lifecycle_and_lookup(void) {
  const struct ui_design_system_vtable *retrieved = NULL;
  const char *active = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  rc = ui_design_system_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Get active when none set */
  rc = ui_design_system_get_active(&active);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  /* Register two design systems */
  rc = ui_design_system_register("system1", &g_test_vtable_1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_design_system_register("system2", &g_test_vtable_2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Duplicate registration check */
  rc = ui_design_system_register("system1", &g_test_vtable_1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Query system1 */
  rc = ui_design_system_get("system1", &retrieved);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(&g_test_vtable_1, retrieved);

  /* Query non-existent */
  rc = ui_design_system_get("nonexistent", &retrieved);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  /* Set active non-existent */
  rc = ui_design_system_set_active(dummy_engine, "nonexistent");
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  /* Set active system1 */
  g_mock_init_called = 0;
  g_mock_init_fail = 0;
  rc = ui_design_system_set_active(dummy_engine, "system1");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, g_mock_init_called);

  /* Get active */
  rc = ui_design_system_get_active(&active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("system1", active);

  /* Set active fail branch */
  g_mock_init_fail = 1;
  rc = ui_design_system_set_active(dummy_engine, "system2");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_mock_init_fail = 0;

  /* Unregister middle/last node */
  rc = ui_design_system_unregister("system1");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Active system was system1, so active should now be reset */
  rc = ui_design_system_get_active(&active);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  /* Register a design system with NULL init */
  rc = ui_design_system_register("system_no_init", &g_test_vtable_no_init);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set active on system without init function (vtable->init == NULL branch) */
  rc = ui_design_system_set_active(dummy_engine, "system_no_init");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Active is system_no_init; unregistering system2 tests g_active != name
   * branch */
  rc = ui_design_system_unregister("system2");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_design_system_get_active(&active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("system_no_init", active);

  /* Unregister already removed */
  rc = ui_design_system_unregister("system2");
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  /* Unregister head node (system_no_init has prev == NULL) */
  rc = ui_design_system_unregister("system_no_init");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Register system1 and system2 so multiple entries are in registry */
  rc = ui_design_system_register("system1", &g_test_vtable_1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_design_system_register("system2", &g_test_vtable_2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Unregister when no active system is set (g_active_design_system_name ==
   * NULL branch) */
  rc = ui_design_system_unregister("system1");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Reset registry while items are still present to test while(cur) freeing */
  rc = ui_design_system_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_design_system_oom_branches(void) {
  ui_error_t rc;

  rc = ui_design_system_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  /* Entry allocation failure */
  g_malloc_fail_countdown = 0;
  rc = ui_design_system_register("oom_sys", &g_test_vtable_1);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Name strdup allocation failure */
  g_malloc_fail_countdown = 1;
  rc = ui_design_system_register("oom_sys", &g_test_vtable_1);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_design_system_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(ui_design_system_suite) {
  RUN_TEST(test_design_system_invalid_args);
  RUN_TEST(test_design_system_lifecycle_and_lookup);
  RUN_TEST(test_design_system_oom_branches);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_design_system_suite);
  GREATEST_MAIN_END();
}
