/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_docking_lifecycle(void) {
  cmp_docking_framework_t *docking = NULL;
  int res;

  res = cmp_docking_framework_create(&docking);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, docking);

  res = cmp_docking_framework_destroy(docking);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_docking_null_args(void) {
  cmp_docking_framework_t *docking = NULL;
  cmp_tool_panel_t *panel = NULL;
  int res;

  res = cmp_docking_framework_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_docking_framework_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_docking_framework_create(&docking);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_docking_framework_register_panel(NULL, "id", "title",
                                             CMP_PANE_SIDEBAR);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_docking_framework_register_panel(docking, NULL, "title",
                                             CMP_PANE_SIDEBAR);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_docking_framework_register_panel(docking, "id", NULL,
                                             CMP_PANE_SIDEBAR);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_docking_framework_float_panel(NULL, "id", 0.0f, 0.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_docking_framework_float_panel(docking, NULL, 0.0f, 0.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_docking_framework_dock_panel(NULL, "id", CMP_PANE_SIDEBAR);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_docking_framework_dock_panel(docking, NULL, CMP_PANE_SIDEBAR);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_docking_framework_get_panel(NULL, "id", &panel);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_docking_framework_get_panel(docking, NULL, &panel);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_docking_framework_get_panel(docking, "id", NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_docking_framework_destroy(docking);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_docking_operations(void) {
  cmp_docking_framework_t *docking = NULL;
  cmp_tool_panel_t *panel = NULL;
  int res;

  res = cmp_docking_framework_create(&docking);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Register a panel */
  res = cmp_docking_framework_register_panel(docking, "panel1", "My Panel",
                                             CMP_PANE_SIDEBAR);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Register same panel should not fail, should probably just do nothing or
   * return SUCCESS */
  res = cmp_docking_framework_register_panel(docking, "panel1", "My Panel",
                                             CMP_PANE_SIDEBAR);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Get panel */
  res = cmp_docking_framework_get_panel(docking, "panel1", &panel);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, panel);
  ASSERT_EQ(CMP_PANEL_STATE_DOCKED, panel->state);
  ASSERT_EQ(CMP_PANE_SIDEBAR, panel->docked_pane);

  /* Float panel */
  res = cmp_docking_framework_float_panel(docking, "panel1", 100.0f, 200.0f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_PANEL_STATE_FLOATING, panel->state);
  ASSERT_EQ(100.0f, panel->floating_x);
  ASSERT_EQ(200.0f, panel->floating_y);

  /* Dock panel */
  res = cmp_docking_framework_dock_panel(docking, "panel1", CMP_PANE_CHAT);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_PANEL_STATE_DOCKED, panel->state);
  ASSERT_EQ(CMP_PANE_CHAT, panel->docked_pane);

  /* Operations on missing panel */
  res = cmp_docking_framework_get_panel(docking, "missing", &panel);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_docking_framework_float_panel(docking, "missing", 0.0f, 0.0f);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_docking_framework_dock_panel(docking, "missing", CMP_PANE_SIDEBAR);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_docking_framework_destroy(docking);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(docking_framework_suite) {
  RUN_TEST(test_docking_lifecycle);
  RUN_TEST(test_docking_null_args);
  RUN_TEST(test_docking_operations);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(docking_framework_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
