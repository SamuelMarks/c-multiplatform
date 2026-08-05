/* clang-format off */
#include "ui_ribbon_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stdio.h>
/* clang-format on */

static int test_ribbon_lifecycle(void) {
  struct ui_arena *arena;
  struct ui_ribbon_base *ribbon = NULL;
  ui_signal_t *signal = NULL;

  if (ui_arena_create(1024 * 16, &arena) != UI_ERROR_NONE) {
    return 1;
  }

  if (ui_ribbon_base_create(arena, &ribbon) != UI_ERROR_NONE)
    return 1;

  if (ui_ribbon_base_get_group_state_changed_signal(ribbon, &signal) !=
      UI_ERROR_NONE)
    return 1;

  if (ui_ribbon_base_destroy(ribbon) != UI_ERROR_NONE)
    return 1;

  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_ribbon_groups(void) {
  struct ui_arena *arena;
  struct ui_ribbon_base *ribbon = NULL;
  struct ui_ribbon_group_config group1;
  struct ui_ribbon_group_config group2;
  struct ui_ribbon_group_config group3;
  enum ui_ribbon_group_collapse_state state;

  ui_arena_create(1024 * 16, &arena);
  ui_ribbon_base_create(arena, &ribbon);

  group1.group_id = 1;
  group1.min_width_normal = 200;
  group1.min_width_compact = 100;
  group1.priority = 10;

  group2.group_id = 2;
  group2.min_width_normal = 200;
  group2.min_width_compact = 100;
  group2.priority = 5;

  group3.group_id = 3;
  group3.min_width_normal = 200;
  group3.min_width_compact = 100;
  group3.priority = 1;

  ui_ribbon_base_add_group_config(ribbon, &group1);
  ui_ribbon_base_add_group_config(ribbon, &group2);
  ui_ribbon_base_add_group_config(ribbon, &group3);

  /* Test 1: Wide window, all should be NORMAL */
  ui_ribbon_base_recalculate_overflow(ribbon, 800);
  ui_ribbon_base_get_group_state(ribbon, 3, &state);
  if (state != UI_RIBBON_GROUP_COLLAPSE_STATE_NORMAL)
    return 1;

  /* Test 2: Shrink to 500, lowest priority (Group 3) should go COMPACT */
  ui_ribbon_base_recalculate_overflow(ribbon, 500);
  ui_ribbon_base_get_group_state(ribbon, 3, &state);
  if (state != UI_RIBBON_GROUP_COLLAPSE_STATE_COMPACT)
    return 1;

  ui_ribbon_base_get_group_state(ribbon, 2, &state);
  if (state != UI_RIBBON_GROUP_COLLAPSE_STATE_NORMAL)
    return 1;

  /* Test 3: Shrink to 250, low priorities should COLLAPSE completely */
  ui_ribbon_base_recalculate_overflow(ribbon, 250);
  ui_ribbon_base_get_group_state(ribbon, 3, &state);
  if (state != UI_RIBBON_GROUP_COLLAPSE_STATE_COLLAPSED)
    return 1;

  ui_ribbon_base_get_group_state(ribbon, 1, &state);

  (void)ui_ribbon_base_destroy(ribbon);
  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_ribbon_contextual_tabs(void) {
  struct ui_arena *arena;
  struct ui_ribbon_base *ribbon = NULL;
  ui_bool_t is_active;

  ui_arena_create(1024 * 16, &arena);
  ui_ribbon_base_create(arena, &ribbon);

  ui_ribbon_base_set_contextual_tab_active(ribbon, 101, UI_TRUE);

  if (ui_ribbon_base_get_contextual_tab_active(ribbon, 101, &is_active) !=
          UI_ERROR_NONE ||
      !is_active)
    return 1;

  /* Existing tab */
  ui_ribbon_base_set_contextual_tab_active(ribbon, 101, UI_FALSE);

  if (ui_ribbon_base_get_contextual_tab_active(ribbon, 101, &is_active) !=
          UI_ERROR_NONE ||
      is_active)
    return 1;

  (void)ui_ribbon_base_destroy(ribbon);
  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_ribbon_nulls_and_errors(void) {
  struct ui_arena *arena;
  struct ui_ribbon_base *ribbon = NULL;
  struct ui_ribbon_group_config group;
  enum ui_ribbon_group_collapse_state state;
  ui_signal_t *signal;
  ui_bool_t is_active;

  ui_arena_create(1024 * 16, &arena);

  if (ui_ribbon_base_create(NULL, &ribbon) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ribbon_base_create(arena, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_ribbon_base_create(arena, &ribbon);

  if (ui_ribbon_base_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ribbon_base_add_group_config(NULL, &group) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ribbon_base_add_group_config(ribbon, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_ribbon_base_recalculate_overflow(NULL, 100) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_ribbon_base_get_group_state(NULL, 1, &state) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ribbon_base_get_group_state(ribbon, 1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Get non existent group */
  if (ui_ribbon_base_get_group_state(ribbon, 1, &state) != UI_ERROR_NOT_FOUND)
    return 1;

  if (ui_ribbon_base_get_group_state_changed_signal(NULL, &signal) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ribbon_base_get_group_state_changed_signal(ribbon, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_ribbon_base_set_contextual_tab_active(NULL, 1, UI_TRUE) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_ribbon_base_get_contextual_tab_active(NULL, 1, &is_active) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_ribbon_base_get_contextual_tab_active(ribbon, 1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Get non existent contextual tab */
  if (ui_ribbon_base_get_contextual_tab_active(ribbon, 1, &is_active) !=
      UI_ERROR_NOT_FOUND)
    return 1;

  /* Add another tab to test loop continuation */
  ui_ribbon_base_set_contextual_tab_active(ribbon, 102, UI_TRUE);

  if (ui_ribbon_base_get_contextual_tab_active(ribbon, 102, &is_active) !=
          UI_ERROR_NONE ||
      !is_active)
    return 1;
  /* Tab 101 was not set in this test function yet, it was set in
     test_ribbon_contextual_tabs! Since this is test_ribbon_nulls_and_errors, we
     need to set it first. */
  ui_ribbon_base_set_contextual_tab_active(ribbon, 101, UI_TRUE);
  if (ui_ribbon_base_get_contextual_tab_active(ribbon, 101, &is_active) !=
          UI_ERROR_NONE ||
      !is_active)
    return 1;

  /* Calculate empty groups */
  if (ui_ribbon_base_recalculate_overflow(ribbon, 100) != UI_ERROR_NONE)
    return 1;

  (void)ui_ribbon_base_destroy(ribbon);
  (void)ui_arena_destroy(arena);
  return 0;
}

static int test_ribbon_limits(void) {
  struct ui_arena *arena;
  struct ui_ribbon_base *ribbon = NULL;
  struct ui_ribbon_group_config group;
  int i;

  ui_arena_create(1024 * 16, &arena);
  ui_ribbon_base_create(arena, &ribbon);

  group.min_width_normal = 200;
  group.min_width_compact = 100;
  group.priority = 10;

  for (i = 0; i < 32; i++) {
    group.group_id = i;
    ui_ribbon_base_add_group_config(ribbon, &group);
  }

  /* Over limit */
  if (ui_ribbon_base_add_group_config(ribbon, &group) != UI_ERROR_OUT_OF_BOUNDS)
    return 1;

  for (i = 0; i < 16; i++) {
    ui_ribbon_base_set_contextual_tab_active(ribbon, i, UI_TRUE);
  }

  /* Over limit */
  if (ui_ribbon_base_set_contextual_tab_active(ribbon, 999, UI_TRUE) !=
      UI_ERROR_OUT_OF_BOUNDS)
    return 1;

  (void)ui_ribbon_base_destroy(ribbon);
  (void)ui_arena_destroy(arena);
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_ribbon_base tests...\n");

  failed |= test_ribbon_lifecycle();
  failed |= test_ribbon_groups();
  failed |= test_ribbon_contextual_tabs();
  failed |= test_ribbon_nulls_and_errors();
  failed |= test_ribbon_limits();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
