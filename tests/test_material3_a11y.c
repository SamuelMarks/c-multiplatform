/* clang-format off */
#include "greatest.h"
#include "material3/md3_a11y.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "ui_error.h"
#include "ui_focus_manager.h"
#include "ui_focus_trap.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

TEST test_md3_a11y_touch_bounds(void) {
  float pad_x = 0.0f;
  float pad_y = 0.0f;
  int is_valid = 0;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_a11y_enforce_touch_target(40.0f, 40.0f, NULL, &pad_y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_a11y_enforce_touch_target(40.0f, 40.0f, &pad_x, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_a11y_enforce_touch_target(-1.0f, 40.0f, &pad_x, &pad_y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_a11y_enforce_touch_target(40.0f, -1.0f, &pad_x, &pad_y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_a11y_validate_touch_target(40.0f, 40.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_a11y_validate_touch_target(-1.0f, 40.0f, &is_valid);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_a11y_validate_touch_target(40.0f, -1.0f, &is_valid);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Element smaller than 48x48dp */
  rc = md3_a11y_enforce_touch_target(40.0f, 32.0f, &pad_x, &pad_y);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4.0f, pad_x); /* (48 - 40) / 2 */
  ASSERT_EQ(8.0f, pad_y); /* (48 - 32) / 2 */

  rc = md3_a11y_validate_touch_target(40.0f, 32.0f, &is_valid);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_valid);

  /* Partial dimensions: width >= 48 but height < 48 */
  rc = md3_a11y_validate_touch_target(48.0f, 32.0f, &is_valid);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_valid);

  /* Element satisfying 48x48dp */
  rc = md3_a11y_enforce_touch_target(48.0f, 56.0f, &pad_x, &pad_y);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, pad_x);
  ASSERT_EQ(0.0f, pad_y);

  rc = md3_a11y_validate_touch_target(48.0f, 56.0f, &is_valid);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_valid);

  PASS();
}

TEST test_md3_a11y_high_contrast_and_forced_colors(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float border_w = 0.0f;
  ui_color_t border_col = 0;
  int enabled = 0;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_a11y_is_high_contrast_enabled(NULL, &enabled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_a11y_is_high_contrast_enabled(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_a11y_set_high_contrast_enabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_a11y_get_forced_colors_border(1, NULL, &border_col);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_a11y_get_forced_colors_border(1, &border_w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Toggle high contrast */
  rc = md3_a11y_set_high_contrast_enabled(dummy_engine, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_a11y_is_high_contrast_enabled(dummy_engine, &enabled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, enabled);

  /* Forced colors 2dp border in high contrast */
  rc = md3_a11y_get_forced_colors_border(1, &border_w, &border_col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2.0f, border_w);
  ASSERT(border_col != 0);

  /* Standard mode: 0dp border */
  rc = md3_a11y_set_high_contrast_enabled(dummy_engine, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_a11y_get_forced_colors_border(0, &border_w, &border_col);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, border_w);

  PASS();
}

TEST test_md3_a11y_focus_ring(void) {
  struct ui_focus_ring ring;
  ui_color_t light_bg = UI_COLOR_ARGB(255, 255, 255, 255);
  ui_color_t dark_bg = UI_COLOR_ARGB(255, 10, 10, 10);
  ui_error_t rc;

  /* Invalid argument */
  rc = md3_a11y_get_focus_ring(light_bg, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Focus ring against light background */
  rc = md3_a11y_get_focus_ring(light_bg, &ring);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2.0f, ring.offset);
  ASSERT_EQ(3.0f, ring.width);
  ASSERT_EQ(1.5f, ring.inner_width);
  ASSERT_EQ(UI_COLOR_ARGB(255, 0, 0, 0), ring.color);
  ASSERT_EQ(UI_COLOR_ARGB(255, 255, 255, 255), ring.inner_color);

  /* Focus ring against dark background */
  rc = md3_a11y_get_focus_ring(dark_bg, &ring);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_COLOR_ARGB(255, 255, 255, 255), ring.color);
  ASSERT_EQ(UI_COLOR_ARGB(255, 0, 0, 0), ring.inner_color);

  PASS();
}

TEST test_md3_a11y_roving_tabindex(void) {
  struct md3_roving_tabindex *roving = NULL;
  size_t focused = 0;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_roving_tabindex_create(0, &roving);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_roving_tabindex_create(5, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_roving_tabindex_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_roving_tabindex_create(5, &roving);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(roving != NULL);

  rc = md3_roving_tabindex_get_focused(roving, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)focused);

  /* Invalid arguments on handle_key and set/get */
  rc = md3_roving_tabindex_handle_key(NULL, UI_KEY_DOWN, &focused);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_roving_tabindex_handle_key(roving, UI_KEY_DOWN, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_roving_tabindex_get_focused(NULL, &focused);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_roving_tabindex_get_focused(roving, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_roving_tabindex_set_focused(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Unhandled default key */
  rc = md3_roving_tabindex_handle_key(roving, (enum ui_key_code)999, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)focused);

  /* Down arrow */
  rc = md3_roving_tabindex_handle_key(roving, UI_KEY_DOWN, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)focused);

  /* Right arrow */
  rc = md3_roving_tabindex_handle_key(roving, UI_KEY_RIGHT, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, (int)focused);

  /* Up arrow */
  rc = md3_roving_tabindex_handle_key(roving, UI_KEY_UP, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)focused);

  /* Left arrow */
  rc = md3_roving_tabindex_handle_key(roving, UI_KEY_LEFT, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)focused);

  /* Left arrow wraps to end */
  rc = md3_roving_tabindex_handle_key(roving, UI_KEY_LEFT, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, (int)focused);

  /* Home key */
  rc = md3_roving_tabindex_handle_key(roving, UI_KEY_HOME, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)focused);

  /* End key */
  rc = md3_roving_tabindex_handle_key(roving, UI_KEY_END, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, (int)focused);

  /* Down arrow wraps to start */
  rc = md3_roving_tabindex_handle_key(roving, UI_KEY_DOWN, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, (int)focused);

  /* Explicit set focused */
  rc = md3_roving_tabindex_set_focused(roving, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_roving_tabindex_get_focused(roving, &focused);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, (int)focused);

  /* Out of range set */
  rc = md3_roving_tabindex_set_focused(roving, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_roving_tabindex_destroy(roving);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_a11y_announcer(void) {
  struct md3_announcer *announcer = NULL;
  ui_error_t rc;

  /* Invalid arguments */
  rc = md3_announcer_create(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_announcer_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_announcer_announce(NULL, "Test", UI_LIVE_POLITE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_announcer_clear(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_announcer_create(&announcer);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(announcer != NULL);

  rc = md3_announcer_announce(announcer, NULL, UI_LIVE_POLITE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_announcer_announce(announcer, "Item updated", UI_LIVE_POLITE);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_announcer_announce(announcer, "Alert error", UI_LIVE_ASSERTIVE);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_announcer_clear(announcer);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_announcer_destroy(announcer);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_a11y_sync_aria(void) {
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_component_create(&comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Invalid arguments */
  rc = md3_a11y_sync_aria(NULL, "button", 1, 0, 0, 0, 50.0, 0.0, 100.0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Sync full ARIA attributes */
  rc =
      md3_a11y_sync_aria(comp, "slider", -1, 1, 0, 0, 75.0, 0.0, 100.0, "menu");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Sync with indeterminate checked */
  rc =
      md3_a11y_sync_aria(comp, "checkbox", 2, -1, 1, 1, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Sync with role=NULL, is_checked=0 (false), and disabled */
  rc = md3_a11y_sync_aria(comp, NULL, 0, -1, -1, 1, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Sync with is_checked=1 (true) */
  rc = md3_a11y_sync_aria(comp, "checkbox", 1, -1, -1, -1, -1.0, -1.0, -1.0,
                          NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  /* Test error percolation from ui_component_set_property in sync_aria */
  /* 1. role */
  g_malloc_fail_countdown = 0;
  rc = md3_a11y_sync_aria(comp, "button", -1, -1, -1, -1, -1.0, -1.0, -1.0,
                          NULL);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  /* 2. aria-checked */
  g_malloc_fail_countdown = 0;
  rc = md3_a11y_sync_aria(comp, NULL, 1, -1, -1, -1, -1.0, -1.0, -1.0, NULL);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  /* 3. aria-expanded */
  g_malloc_fail_countdown = 0;
  rc = md3_a11y_sync_aria(comp, NULL, -1, 1, -1, -1, -1.0, -1.0, -1.0, NULL);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  /* 4. aria-selected */
  g_malloc_fail_countdown = 0;
  rc = md3_a11y_sync_aria(comp, NULL, -1, -1, 1, -1, -1.0, -1.0, -1.0, NULL);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  /* 5. aria-disabled */
  g_malloc_fail_countdown = 0;
  rc = md3_a11y_sync_aria(comp, NULL, -1, -1, -1, 1, -1.0, -1.0, -1.0, NULL);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  /* 6. aria-valuenow */
  g_malloc_fail_countdown = 0;
  rc = md3_a11y_sync_aria(comp, NULL, -1, -1, -1, -1, 50.0, -1.0, -1.0, NULL);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  /* 7. aria-valuemin */
  g_malloc_fail_countdown = 0;
  rc = md3_a11y_sync_aria(comp, NULL, -1, -1, -1, -1, -1.0, 0.0, -1.0, NULL);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  /* 8. aria-valuemax */
  g_malloc_fail_countdown = 0;
  rc = md3_a11y_sync_aria(comp, NULL, -1, -1, -1, -1, -1.0, -1.0, 100.0, NULL);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  /* 9. aria-haspopup */
  g_malloc_fail_countdown = 0;
  rc = md3_a11y_sync_aria(comp, NULL, -1, -1, -1, -1, -1.0, -1.0, -1.0,
                          "dialog");
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_component_destroy(comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_a11y_focus_trap(void) {
  struct ui_focus_manager *manager = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_focus_trap *trap = NULL;
  ui_error_t rc;

  rc = ui_focus_manager_create(&manager);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Invalid argument checks */
  rc = md3_a11y_trap_focus(NULL, manager, root);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_a11y_trap_focus(&trap, NULL, root);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_a11y_trap_focus(&trap, manager, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_a11y_release_focus(NULL, manager);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_a11y_release_focus((struct ui_focus_trap *)0x1234, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid trap & release */
  rc = md3_a11y_trap_focus(&trap, manager, root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(trap != NULL);

  rc = md3_a11y_release_focus(trap, manager);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_dom_node_destroy(root);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_focus_manager_destroy(manager);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_a11y_shell_layout(void) {
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_component_create(&comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Loading indicator ARIA sync: role="progressbar", aria-busy="true" */
  rc = md3_a11y_sync_aria(comp, "progressbar", -1, -1, -1, 0, -1.0, -1.0, -1.0,
                          NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Stepper ARIA sync: role="tablist", items role="tab" */
  rc = md3_a11y_sync_aria(comp, "tablist", -1, -1, -1, 0, -1.0, -1.0, -1.0,
                          NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_a11y_sync_aria(comp, "tab", -1, -1, 1, 0, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Breadcrumbs ARIA sync: role="navigation" */
  rc = md3_a11y_sync_aria(comp, "navigation", -1, -1, -1, 0, -1.0, -1.0, -1.0,
                          NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Page indicator ARIA sync: role="tablist", dots role="tab", active dot
   * aria-selected="true" */
  rc = md3_a11y_sync_aria(comp, "tablist", -1, -1, -1, 0, -1.0, -1.0, -1.0,
                          NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_a11y_sync_aria(comp, "tab", -1, -1, 1, 0, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Scaffold ARIA sync: role="region" */
  rc =
      md3_a11y_sync_aria(comp, "region", -1, -1, -1, 0, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Canonical layout: main and complementary panes */
  rc = md3_a11y_sync_aria(comp, "main", -1, -1, -1, 0, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_a11y_sync_aria(comp, "complementary", -1, -1, -1, 0, -1.0, -1.0,
                          -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_component_destroy(comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_a11y_overlay_menus(void) {
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_component_create(&comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Date range picker dialog and grid */
  rc =
      md3_a11y_sync_aria(comp, "dialog", -1, -1, -1, 0, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_a11y_sync_aria(comp, "grid", -1, -1, -1, 0, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Hover card tooltip */
  rc = md3_a11y_sync_aria(comp, "tooltip", -1, -1, -1, 0, -1.0, -1.0, -1.0,
                          NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Popover dialog */
  rc =
      md3_a11y_sync_aria(comp, "dialog", -1, -1, -1, 0, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Banner status */
  rc =
      md3_a11y_sync_aria(comp, "status", -1, -1, -1, 0, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Inline alert alert */
  rc = md3_a11y_sync_aria(comp, "alert", -1, -1, -1, 0, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Menubar and menu hierarchy */
  rc = md3_a11y_sync_aria(comp, "menubar", -1, -1, -1, 0, -1.0, -1.0, -1.0,
                          NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc =
      md3_a11y_sync_aria(comp, "menu", -1, -1, -1, 0, -1.0, -1.0, -1.0, "menu");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_a11y_sync_aria(comp, "menuitem", -1, -1, -1, 0, -1.0, -1.0, -1.0,
                          NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Context menu */
  rc = md3_a11y_sync_aria(comp, "menu", -1, -1, -1, 0, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Action sheet */
  rc = md3_a11y_sync_aria(comp, "dialog", -1, 1, -1, 0, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Coachmark */
  rc =
      md3_a11y_sync_aria(comp, "dialog", -1, -1, -1, 0, -1.0, -1.0, -1.0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Color picker */
  rc = md3_a11y_sync_aria(comp, "slider", -1, -1, -1, 0, 120.0, 0.0, 360.0,
                          NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Command palette */
  rc = md3_a11y_sync_aria(comp, "combobox", -1, 1, -1, 0, -1.0, -1.0, -1.0,
                          "listbox");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_component_destroy(comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(md3_a11y_suite) {
  RUN_TEST(test_md3_a11y_touch_bounds);
  RUN_TEST(test_md3_a11y_high_contrast_and_forced_colors);
  RUN_TEST(test_md3_a11y_focus_ring);
  RUN_TEST(test_md3_a11y_roving_tabindex);
  RUN_TEST(test_md3_a11y_announcer);
  RUN_TEST(test_md3_a11y_sync_aria);
  RUN_TEST(test_md3_a11y_focus_trap);
  RUN_TEST(test_md3_a11y_shell_layout);
  RUN_TEST(test_md3_a11y_overlay_menus);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_a11y_suite);
  GREATEST_MAIN_END();
}
