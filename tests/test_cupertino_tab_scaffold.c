/**
 * @file test_cupertino_tab_scaffold.c
 * @brief Unit tests for Cupertino Tab Scaffold and Controller.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_tab_scaffold.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_tab_scaffold_suite);

TEST test_tab_controller_lifecycle(void) {
  struct cupertino_tab_controller *controller = NULL;
  size_t idx, count;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_tab_controller_create(0, 0, &controller);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_controller_create(5, 3, &controller);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_controller_create(0, 10, &controller);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_controller_create(0, 4, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_tab_controller_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_controller_set_index(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_controller_get_index(NULL, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_controller_get_tab_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Normal creation */
  rc = cupertino_tab_controller_create(1, 4, &controller);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(controller != NULL);

  rc = cupertino_tab_controller_get_index(controller, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_controller_get_tab_count(controller, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_tab_controller_get_index(controller, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)1, idx);

  rc = cupertino_tab_controller_get_tab_count(controller, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)4, count);

  /* Set index within bounds */
  rc = cupertino_tab_controller_set_index(controller, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_controller_get_index(controller, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)3, idx);

  /* Set index out of bounds */
  rc = cupertino_tab_controller_set_index(controller, 4);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  rc = cupertino_tab_controller_destroy(controller);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_tab_scaffold_lifecycle_and_views(void) {
  struct cupertino_tab_scaffold_descriptor desc;
  struct cupertino_tab_scaffold *scaffold = NULL;
  struct cupertino_tab_bar_descriptor bar_desc;
  struct cupertino_tab_item_descriptor items[2];
  struct cupertino_tab_bar *tab_bar = NULL;
  struct cupertino_tab_bar *ret_bar = NULL;
  struct cupertino_tab_controller *custom_ctrl = NULL;
  struct cupertino_tab_controller *ret_ctrl = NULL;
  struct ui_component *view1 = NULL;
  struct ui_component *view2 = NULL;
  struct ui_component *ret_view = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  int hidden;
  size_t idx;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.initial_index = 0;
  desc.bar_hidden = 0;
  desc.is_dark = 1;

  /* Invalid scaffold creation */
  rc = cupertino_tab_scaffold_create(NULL, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_create(dummy_engine, NULL, &scaffold);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.initial_index = 99;
  rc = cupertino_tab_scaffold_create(dummy_engine, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.initial_index = 0;

  /* Create valid scaffold with bar_hidden=1, is_dark=0 */
  {
    struct cupertino_tab_scaffold *dark_scaffold = NULL;
    struct cupertino_tab_scaffold_descriptor d2;
    memset(&d2, 0, sizeof(d2));
    d2.bar_hidden = 1;
    d2.is_dark = 0;
    rc = cupertino_tab_scaffold_create(dummy_engine, &d2, &dark_scaffold);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(dark_scaffold != NULL);
    rc = cupertino_tab_scaffold_destroy(dark_scaffold);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Create valid scaffold */
  rc = cupertino_tab_scaffold_create(dummy_engine, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scaffold != NULL);

  /* Tab bar operations invalid args */
  rc = cupertino_tab_scaffold_get_tab_bar(NULL, &ret_bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_get_tab_bar(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_set_tab_bar(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Tab bar operations */
  rc = cupertino_tab_scaffold_get_tab_bar(scaffold, &ret_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(NULL, ret_bar);

  memset(&bar_desc, 0, sizeof(bar_desc));
  memset(items, 0, sizeof(items));
  items[0].label = "Home";
  items[0].icon_name = "house";
  items[1].label = "Settings";
  items[1].icon_name = "gear";
  bar_desc.items = items;
  bar_desc.item_count = 2;
  bar_desc.initial_index = 0;

  rc = cupertino_tab_bar_create(dummy_engine, &bar_desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* set_tab_bar with tab_bar == NULL */
  rc = cupertino_tab_scaffold_set_tab_bar(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* set_tab_bar with tab_bar != NULL and scaffold->controller == NULL */
  {
    struct cupertino_tab_controller *orig_ctrl = scaffold->controller;
    scaffold->controller = NULL;
    rc = cupertino_tab_scaffold_set_tab_bar(scaffold, tab_bar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    scaffold->controller = orig_ctrl;
  }

  rc = cupertino_tab_scaffold_set_tab_bar(scaffold, tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_scaffold_get_tab_bar(scaffold, &ret_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(tab_bar, ret_bar);

  /* Controller operations invalid args */
  rc = cupertino_tab_scaffold_set_controller(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_set_controller(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_get_controller(NULL, &ret_ctrl);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_get_controller(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Controller operations */
  rc = cupertino_tab_scaffold_get_controller(scaffold, &ret_ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ret_ctrl != NULL);

  rc = cupertino_tab_controller_create(1, 2, &custom_ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* set_controller when owns_controller == 1 but controller == NULL */
  {
    struct cupertino_tab_controller *temp_ctrl = scaffold->controller;
    scaffold->owns_controller = 1;
    scaffold->controller = NULL;
    rc = cupertino_tab_scaffold_set_controller(scaffold, custom_ctrl);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    scaffold->controller = temp_ctrl;
    scaffold->owns_controller = 1;
  }

  rc = cupertino_tab_scaffold_set_controller(scaffold, custom_ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_scaffold_get_controller(scaffold, &ret_ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(custom_ctrl, ret_ctrl);

  /* set_controller when scaffold has no tab_bar and does not own controller */
  {
    struct cupertino_tab_controller *ctrl3 = NULL;
    rc = cupertino_tab_controller_create(0, 2, &ctrl3);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_tab_scaffold_set_tab_bar(scaffold, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_tab_scaffold_set_controller(scaffold, ctrl3);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_tab_scaffold_set_tab_bar(scaffold, tab_bar);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_tab_scaffold_set_controller(scaffold, custom_ctrl);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_tab_controller_destroy(ctrl3);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = cupertino_tab_controller_get_index(custom_ctrl, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)1, idx);

  /* View slotting invalid args */
  rc = cupertino_tab_scaffold_set_tab_view(NULL, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_get_tab_view(NULL, 0, &ret_view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_get_tab_view(scaffold, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_get_active_view(NULL, &ret_view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_get_active_view(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* View slotting */
  rc = ui_component_create(&view1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&view2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_tab_scaffold_set_tab_view(scaffold, 0, view1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_scaffold_set_tab_view(scaffold, 1, view2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Clearing a tab view (view == NULL) */
  rc = cupertino_tab_scaffold_set_tab_view(scaffold, 2, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Out of bounds slotting */
  rc = cupertino_tab_scaffold_set_tab_view(scaffold, 99, view1);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_tab_scaffold_get_tab_view(scaffold, 99, &ret_view);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  rc = cupertino_tab_scaffold_get_tab_view(scaffold, 0, &ret_view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(view1, ret_view);

  /* Active view (index 1 is active) */
  rc = cupertino_tab_scaffold_get_active_view(scaffold, &ret_view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(view2, ret_view);

  /* Active view when selected_index is >= MAX_ITEMS */
  custom_ctrl->selected_index = 99;
  rc = cupertino_tab_scaffold_get_active_view(scaffold, &ret_view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(NULL, ret_view);
  custom_ctrl->selected_index = 0;

  /* Active view when controller is NULL */
  scaffold->controller = NULL;
  rc = cupertino_tab_scaffold_get_active_view(scaffold, &ret_view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(NULL, ret_view);
  scaffold->controller = custom_ctrl;

  /* Switch tab to 0 */
  rc = cupertino_tab_controller_set_index(custom_ctrl, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_scaffold_get_active_view(scaffold, &ret_view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(view1, ret_view);

  /* Bar hidden invalid args */
  rc = cupertino_tab_scaffold_set_bar_hidden(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_get_bar_hidden(NULL, &hidden);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_scaffold_get_bar_hidden(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Bar hidden toggle (0 and 1) */
  rc = cupertino_tab_scaffold_get_bar_hidden(scaffold, &hidden);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, hidden);

  rc = cupertino_tab_scaffold_set_bar_hidden(scaffold, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_scaffold_get_bar_hidden(scaffold, &hidden);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, hidden);

  rc = cupertino_tab_scaffold_set_bar_hidden(scaffold, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_scaffold_get_bar_hidden(scaffold, &hidden);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, hidden);

  /* Clean up */
  rc = ui_component_destroy(view1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(view2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_tab_bar_destroy(tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_tab_controller_destroy(custom_ctrl);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy with owns_controller == 0 */
  rc = cupertino_tab_scaffold_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy NULL scaffold */
  rc = cupertino_tab_scaffold_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_tab_scaffold_oom_simulation(void) {
  struct cupertino_tab_controller *controller = NULL;
  struct cupertino_tab_scaffold_descriptor desc;
  struct cupertino_tab_scaffold *scaffold = NULL;
  struct cupertino_tab_bar_descriptor bar_desc;
  struct cupertino_tab_item_descriptor items[2];
  struct cupertino_tab_bar *tab_bar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_tab_scaffold_mock_controller_destroy_fail;
  extern int g_cupertino_tab_scaffold_mock_set_selected_fail;

  g_malloc_fail_countdown = 0;
  rc = cupertino_tab_controller_create(0, 4, &controller);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, controller);

  rc = cupertino_tab_scaffold_create(dummy_engine, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, scaffold);

  /* OOM during controller creation inside scaffold_create */
  g_malloc_fail_countdown = 1;
  rc = cupertino_tab_scaffold_create(dummy_engine, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, scaffold);
  g_malloc_fail_countdown = -1;

  /* Normal creation for mock failure tests */
  rc = cupertino_tab_scaffold_create(dummy_engine, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scaffold != NULL);

  memset(&bar_desc, 0, sizeof(bar_desc));
  memset(items, 0, sizeof(items));
  items[0].label = "Home";
  items[0].icon_name = "house";
  items[1].label = "Settings";
  items[1].icon_name = "gear";
  bar_desc.items = items;
  bar_desc.item_count = 2;
  bar_desc.initial_index = 0;
  rc = cupertino_tab_bar_create(dummy_engine, &bar_desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* tab_bar_set_selected failure during set_tab_bar */
  g_cupertino_tab_scaffold_mock_set_selected_fail = 1;
  rc = cupertino_tab_scaffold_set_tab_bar(scaffold, tab_bar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_tab_scaffold_mock_set_selected_fail = 0;

  rc = cupertino_tab_scaffold_set_tab_bar(scaffold, tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* controller_destroy failure during set_controller */
  rc = cupertino_tab_controller_create(0, 2, &controller);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_tab_scaffold_mock_controller_destroy_fail = 1;
  rc = cupertino_tab_scaffold_set_controller(scaffold, controller);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_tab_scaffold_mock_controller_destroy_fail = 0;

  /* tab_bar_set_selected failure during set_controller */
  g_cupertino_tab_scaffold_mock_set_selected_fail = 1;
  rc = cupertino_tab_scaffold_set_controller(scaffold, controller);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_tab_scaffold_mock_set_selected_fail = 0;

  /* Destroy with owns_controller == 1 and controller == NULL */
  {
    struct cupertino_tab_scaffold *sc2 = NULL;
    rc = cupertino_tab_scaffold_create(dummy_engine, &desc, &sc2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_tab_controller_destroy(sc2->controller);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    sc2->controller = NULL;
    rc = cupertino_tab_scaffold_destroy(sc2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* controller_destroy failure during scaffold_destroy */
  {
    struct cupertino_tab_scaffold *sc3 = NULL;
    rc = cupertino_tab_scaffold_create(dummy_engine, &desc, &sc3);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_cupertino_tab_scaffold_mock_controller_destroy_fail = 1;
    rc = cupertino_tab_scaffold_destroy(sc3);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_cupertino_tab_scaffold_mock_controller_destroy_fail = 0;
    rc = cupertino_tab_scaffold_destroy(sc3);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = cupertino_tab_bar_destroy(tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_controller_destroy(controller);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_scaffold_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  rc = cupertino_tab_controller_create(0, 0, &controller);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_tab_scaffold_suite) {
  RUN_TEST(test_tab_controller_lifecycle);
  RUN_TEST(test_tab_scaffold_lifecycle_and_views);
  RUN_TEST(test_tab_scaffold_oom_simulation);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_tab_scaffold_suite);
  GREATEST_MAIN_END();
}
