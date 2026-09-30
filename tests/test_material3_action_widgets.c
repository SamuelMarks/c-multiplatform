/**
 * @file test_material3_action_widgets.c
 * @brief Comprehensive tests for Material 3 Action Widgets.
 */

/* clang-format off */
#include "greatest.h"
#include "material3/md3_action_widgets.h"
#include "material3/md3_button.h"
#include "material3/md3_fab.h"
#include "ui_error.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

TEST test_md3_fab_menu_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_fab *primary_fab;
  struct md3_fab *action_fab1;
  struct md3_fab *action_fab2;
  struct md3_fab_menu *menu;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid arguments */
  rc = md3_fab_menu_create(NULL, NULL, MD3_FAB_MENU_DIRECTION_UP, &menu);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      md3_fab_menu_create(dummy_engine, NULL, MD3_FAB_MENU_DIRECTION_UP, &menu);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_REGULAR, MD3_FAB_PRIMARY,
                      "add", "Add", &primary_fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(primary_fab != NULL);

  rc = md3_fab_menu_create(dummy_engine, primary_fab, MD3_FAB_MENU_DIRECTION_UP,
                           NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_menu_create(dummy_engine, primary_fab,
                           (enum md3_fab_menu_direction) - 1, &menu);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_menu_create(dummy_engine, primary_fab,
                           MD3_FAB_MENU_DIRECTION_COUNT, &menu);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_menu_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_menu_toggle(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_menu_set_expanded(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_menu_add_action(NULL, 1, primary_fab, "Fail");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_fab_menu_create(dummy_engine, primary_fab, MD3_FAB_MENU_DIRECTION_UP,
                           &menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(menu != NULL);
  ASSERT_EQ(0, menu->is_expanded);
  ASSERT_EQ_FMT(0.0f, menu->rotation_deg, "%f");
  ASSERT_EQ_FMT(0.0f, menu->scrim_opacity, "%f");

  /* Add secondary actions */
  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_SMALL, MD3_FAB_SECONDARY,
                      "photo", "Photo", &action_fab1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_fab_menu_add_action(menu, 1, action_fab1, "Add Photo");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)menu->item_count);

  rc = md3_fab_create(dummy_engine, MD3_FAB_SIZE_SMALL, MD3_FAB_TERTIARY,
                      "description", "Doc", &action_fab2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_fab_menu_add_action(menu, 2, action_fab2, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, (int)menu->item_count);

  /* Add with null action fab */
  rc = md3_fab_menu_add_action(menu, 3, NULL, "Fail");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Expansion toggle */
  rc = md3_fab_menu_toggle(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, menu->is_expanded);
  ASSERT_EQ_FMT(90.0f, menu->rotation_deg, "%f");
  ASSERT_EQ_FMT(0.32f, menu->scrim_opacity, "%f");

  rc = md3_fab_menu_toggle(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, menu->is_expanded);
  ASSERT_EQ_FMT(0.0f, menu->rotation_deg, "%f");

  /* Destroy */
  rc = md3_fab_menu_destroy(menu);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_fab_destroy(primary_fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_button_group_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_button_group *group;
  struct md3_button *btn1;
  struct md3_button *btn2;
  struct md3_button *btn3;
  struct md3_button *btn4;
  struct md3_button *btn5;
  int selected;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_button_group_create(NULL, MD3_BUTTON_GROUP_HORIZONTAL, &group);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_button_group_create(dummy_engine, MD3_BUTTON_GROUP_HORIZONTAL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_group_create(dummy_engine,
                               (enum md3_button_group_orientation) - 1, &group);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_button_group_create(dummy_engine, MD3_BUTTON_GROUP_ORIENTATION_COUNT,
                               &group);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_group_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_button_group_create(dummy_engine, MD3_BUTTON_GROUP_VERTICAL, &group);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(group != NULL);
  rc = md3_button_group_destroy(group);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_button_group_create(dummy_engine, MD3_BUTTON_GROUP_HORIZONTAL,
                               &group);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(group != NULL);
  ASSERT_EQ(0, (int)group->button_count);

  /* Navigate on empty group */
  rc = md3_button_group_navigate(group, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create and add buttons */
  rc = md3_button_create(dummy_engine, MD3_BUTTON_OUTLINED, &btn1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_create(dummy_engine, MD3_BUTTON_OUTLINED, &btn2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_create(dummy_engine, MD3_BUTTON_OUTLINED, &btn3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_create(dummy_engine, MD3_BUTTON_OUTLINED, &btn4);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_create(dummy_engine, MD3_BUTTON_OUTLINED, &btn5);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_button_group_add_button(group, btn1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_group_add_button(group, btn2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_group_add_button(group, btn3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_group_add_button(group, btn4);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* 5th button expands capacity from 4 to 8 */
  rc = md3_button_group_add_button(group, btn5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5, (int)group->button_count);

  /* Add null */
  rc = md3_button_group_add_button(group, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_button_group_add_button(NULL, btn1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Selection mode: Invalid */
  rc = md3_button_group_set_selection_mode(NULL,
                                           MD3_BUTTON_GROUP_SELECTION_SINGLE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_button_group_set_selection_mode(
      group, (enum md3_button_group_selection_mode) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_button_group_set_selection_mode(
      group, MD3_BUTTON_GROUP_SELECTION_MODE_COUNT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Selection mode: SINGLE */
  rc = md3_button_group_set_selection_mode(group,
                                           MD3_BUTTON_GROUP_SELECTION_SINGLE);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Deselect without selection mode change */
  rc = md3_button_group_set_selected(group, 0, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_button_group_set_selected(group, 0, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_group_is_selected(group, 0, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, selected);

  /* Selecting button 1 deselects button 0 in SINGLE mode */
  rc = md3_button_group_set_selected(group, 1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_group_is_selected(group, 0, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, selected);
  rc = md3_button_group_is_selected(group, 1, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, selected);

  /* Selection mode: MULTI */
  rc = md3_button_group_set_selection_mode(group,
                                           MD3_BUTTON_GROUP_SELECTION_MULTI);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_group_set_selected(group, 2, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_button_group_is_selected(group, 1, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, selected);
  rc = md3_button_group_is_selected(group, 2, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, selected);

  /* Navigation roving tabindex */
  ASSERT_EQ(0, group->focused_index);
  rc = md3_button_group_navigate(group, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, group->focused_index);
  rc = md3_button_group_navigate(group, 10);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, group->focused_index);
  rc = md3_button_group_navigate(group, -1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, group->focused_index);
  rc = md3_button_group_navigate(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of bounds checks */
  rc = md3_button_group_set_selected(NULL, 0, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_button_group_set_selected(group, 99, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_button_group_is_selected(NULL, 0, &selected);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_button_group_is_selected(group, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_button_group_is_selected(group, 99, &selected);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy */
  rc = md3_button_group_destroy(group);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_toggle_button_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_toggle_button *toggle;
  struct ui_control_value_accessor *cva;
  int is_selected;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;
  cva = NULL;

  /* Invalid args */
  rc = md3_toggle_button_create(NULL, &toggle, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toggle_button_create(dummy_engine, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create without CVA */
  rc = md3_toggle_button_create(dummy_engine, &toggle, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(toggle != NULL);
  rc = md3_toggle_button_destroy(toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_toggle_button_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_toggle_button_create(dummy_engine, &toggle, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(toggle != NULL);
  ASSERT(cva != NULL);

  /* Initial state */
  rc = md3_toggle_button_is_selected(NULL, &is_selected);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toggle_button_is_selected(toggle, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toggle_button_is_selected(toggle, &is_selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_selected);

  /* Set selected */
  rc = md3_toggle_button_set_selected(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toggle_button_set_selected(toggle, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_toggle_button_is_selected(toggle, &is_selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_selected);

  /* Set text and icon */
  rc = md3_toggle_button_set_text(NULL, "Mute");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toggle_button_set_text(toggle, "Mute");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Mute", toggle->text);

  rc = md3_toggle_button_set_icon(NULL, "volume_off");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toggle_button_set_icon(toggle, "volume_off");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("volume_off", toggle->icon_name);

  /* Clear text and icon */
  rc = md3_toggle_button_set_text(toggle, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('\0', toggle->text[0]);

  rc = md3_toggle_button_set_icon(toggle, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ('\0', toggle->icon_name[0]);

  /* Destroy */
  if (cva) {
    C_MULTIPLATFORM_FREE(cva);
  }
  rc = md3_toggle_button_destroy(toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_toolbar_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_toolbar *toolbar;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_toolbar_create(NULL, &toolbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toolbar_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_toolbar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_toolbar_create(dummy_engine, &toolbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(toolbar != NULL);
  ASSERT_EQ(MD3_TOOLBAR_STANDARD, toolbar->variant);

  /* Variant */
  rc = md3_toolbar_set_variant(toolbar, MD3_TOOLBAR_DENSE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_TOOLBAR_DENSE, toolbar->variant);

  rc = md3_toolbar_set_variant(toolbar, (enum md3_toolbar_variant) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toolbar_set_variant(toolbar, MD3_TOOLBAR_VARIANT_COUNT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toolbar_set_variant(NULL, MD3_TOOLBAR_STANDARD);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set title */
  rc = md3_toolbar_set_title(NULL, "Dashboard Actions");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toolbar_set_title(toolbar, "Dashboard Actions");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add action items */
  rc = md3_toolbar_add_action(NULL, 1, "Save", 48.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toolbar_add_action(toolbar, 1, "Save", 48.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_toolbar_add_action(toolbar, 2, NULL, 48.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_toolbar_add_action(toolbar, 3, "Export", 48.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_toolbar_add_action(toolbar, 4, "Settings", 48.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, (int)toolbar->item_count);

  /* Invalid action width */
  rc = md3_toolbar_add_action(toolbar, 5, "Bad", 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toolbar_add_action(toolbar, 5, "Bad", -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Overflow calculation:
   * Width 140dp with 4 items of 48dp each.
   * Overflow reserve = 48dp -> available for visible = 92dp.
   * Item 1 (48dp) fits (cum 48 <= 92).
   * Item 2 (48dp) exceeds (cum 96 > 92) -> overflows.
   * Visible: 1, Overflow: 3.
   */
  rc = md3_toolbar_calculate_overflow(NULL, 140.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toolbar_calculate_overflow(toolbar, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toolbar_calculate_overflow(toolbar, -10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_toolbar_calculate_overflow(toolbar, 140.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)toolbar->visible_count);
  ASSERT_EQ(3, (int)toolbar->overflow_count);

  /* Width 300dp: All items fit (4 * 48 = 192 <= 300 - 48 = 252).
   * Visible: 4, Overflow: 0.
   */
  rc = md3_toolbar_calculate_overflow(toolbar, 300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, (int)toolbar->visible_count);
  ASSERT_EQ(0, (int)toolbar->overflow_count);

  /* All items fit exactly without overflow button (e.g. available_width = 192)
   */
  rc = md3_toolbar_calculate_overflow(toolbar, 192.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Navigation */
  ASSERT_EQ(0, toolbar->focused_index);
  rc = md3_toolbar_navigate(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_toolbar_navigate(toolbar, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, toolbar->focused_index);
  rc = md3_toolbar_navigate(toolbar, -1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, toolbar->focused_index);
  rc = md3_toolbar_navigate(toolbar, -1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, toolbar->focused_index);
  rc = md3_toolbar_navigate(toolbar, 10);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, toolbar->focused_index);

  /* Empty toolbar navigate */
  {
    struct md3_toolbar *empty_tb = NULL;
    rc = md3_toolbar_create(dummy_engine, &empty_tb);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = md3_toolbar_navigate(empty_tb, 1);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = md3_toolbar_destroy(empty_tb);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Destroy */
  rc = md3_toolbar_destroy(toolbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(md3_action_widgets_suite) {
  RUN_TEST(test_md3_fab_menu_lifecycle);
  RUN_TEST(test_md3_button_group_lifecycle);
  RUN_TEST(test_md3_toggle_button_lifecycle);
  RUN_TEST(test_md3_toolbar_lifecycle);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_action_widgets_suite);
  GREATEST_MAIN_END();
}
