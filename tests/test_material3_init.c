/* clang-format off */
#include "greatest.h"
#include "material3/md3_init.h"
#include "ui_design_system.h"
#include "ui_error.h"
/* clang-format on */

TEST test_md3_init_invalid_args(void) {
  const struct ui_design_system_vtable *vtable = NULL;
  ui_error_t rc;

  /* NULL engine to initialize */
  rc = md3_initialize(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* NULL out_vtable to md3_get_vtable */
  rc = md3_get_vtable(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* NULL out_vtable to md3_expressive_get_vtable */
  rc = md3_expressive_get_vtable(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_get_vtable(&vtable);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(vtable != NULL);

  /* Test vtable methods with invalid arguments */
  rc = vtable->init(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = vtable->shutdown(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = vtable->apply_theme(NULL, "light");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = vtable->apply_theme((struct ui_engine *)1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = vtable->resolve_token(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = vtable->resolve_token(NULL, (struct ui_design_token *)1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = vtable->resolve_token("token", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_md3_init_error_branches(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x5678;
  const struct ui_design_system_vtable *vtable = NULL;
  ui_error_t rc;

  rc = ui_design_system_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_get_vtable(&vtable);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Case 1: "material3" is already registered, so md3_initialize fails on first
   * register */
  rc = ui_design_system_register("material3", vtable);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_initialize(dummy_engine);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_design_system_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Case 2: "material3-expressive" is already registered, so md3_initialize
   * fails on second register and rolls back */
  rc = ui_design_system_register("material3-expressive", vtable);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_initialize(dummy_engine);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  /* Verify rollback unregistered material3 */
  rc = ui_design_system_get("material3", &vtable);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);
  rc = ui_design_system_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Case 3: md3_shutdown failure on unregistering material3-expressive */
  rc = md3_initialize(dummy_engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_design_system_unregister("material3-expressive");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_shutdown();
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);
  rc = ui_design_system_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Case 4: md3_shutdown failure on unregistering material3 */
  rc = md3_initialize(dummy_engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_design_system_unregister("material3");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_shutdown();
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);
  rc = ui_design_system_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Case 5: both unregistered before md3_shutdown */
  rc = md3_initialize(dummy_engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_design_system_unregister("material3-expressive");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_design_system_unregister("material3");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_shutdown();
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);
  rc = ui_design_system_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_lifecycle(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x5678;
  const struct ui_design_system_vtable *vtable_m3 = NULL;
  const struct ui_design_system_vtable *vtable_exp = NULL;
  const char *active = NULL;
  struct ui_design_token tok;
  ui_error_t rc;

  rc = ui_design_system_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Shutdown before initialize is a safe no-op */
  rc = md3_shutdown();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Retrieve both vtables */
  rc = md3_get_vtable(&vtable_m3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(vtable_m3 != NULL);

  rc = md3_expressive_get_vtable(&vtable_exp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(vtable_exp != NULL);

  /* Exercise vtable methods with valid arguments */
  rc = vtable_m3->init(dummy_engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = vtable_m3->shutdown(dummy_engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = vtable_m3->apply_theme(dummy_engine, "dark");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = vtable_m3->resolve_token("color-primary", &tok);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  /* Initialize md3 registers "material3" and "material3-expressive" */
  rc = md3_initialize(dummy_engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set active to material3 */
  rc = ui_design_system_set_active(dummy_engine, "material3");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_design_system_get_active(&active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("material3", active);

  /* Switch active to material3-expressive */
  rc = ui_design_system_set_active(dummy_engine, "material3-expressive");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_design_system_get_active(&active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("material3-expressive", active);

  /* Shutdown md3 unregisters both */
  rc = md3_shutdown();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_design_system_get("material3", &vtable_m3);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  rc = ui_design_system_get("material3-expressive", &vtable_exp);
  ASSERT_EQ(UI_ERROR_NOT_FOUND, rc);

  rc = ui_design_system_registry_reset();
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(md3_init_suite) {
  RUN_TEST(test_md3_init_invalid_args);
  RUN_TEST(test_md3_init_error_branches);
  RUN_TEST(test_md3_lifecycle);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_init_suite);
  GREATEST_MAIN_END();
}
