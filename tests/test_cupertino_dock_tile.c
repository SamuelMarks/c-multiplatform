/**
 * @file test_cupertino_dock_tile.c
 * @brief Unit tests for macOS Dock Tile Live Sync conforming to Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_dock_tile.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

TEST test_dock_tile_lifecycle_and_badge(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_dock_tile_descriptor desc;
  struct cupertino_dock_tile *tile = NULL;
  const char *lbl = NULL;
  int has_badge = 0;
  float bx = 0.0f;
  float by = 0.0f;
  float bw = 0.0f;
  float bh = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.initial_badge_label = "3";
  desc.initial_progress = 0.0f;
  desc.show_progress = 0;

  /* Null checks */
  rc = cupertino_dock_tile_create(NULL, &desc, &tile);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_create(dummy_engine, NULL, &tile);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation with progress > 1.0f (clamp check) and empty badge */
  desc.initial_progress = 1.5f;
  desc.initial_badge_label = "";
  rc = cupertino_dock_tile_create(dummy_engine, &desc, &tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, tile);
  ASSERT_IN_RANGE(0.0f, tile->progress, 0.001f);
  ASSERT_EQ(0, tile->has_badge);
  rc = cupertino_dock_tile_destroy(tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Valid creation with progress < 0.0f (clamp check) */
  desc.initial_progress = -0.5f;
  desc.initial_badge_label = NULL;
  rc = cupertino_dock_tile_create(dummy_engine, &desc, &tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, tile);
  ASSERT_IN_RANGE(0.0f, tile->progress, 0.001f);
  ASSERT_EQ(0, tile->has_badge);
  rc = cupertino_dock_tile_destroy(tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Valid creation with valid badge and valid progress */
  desc.initial_badge_label = "3";
  desc.initial_progress = 0.5f;
  desc.show_progress = 1;
  rc = cupertino_dock_tile_create(dummy_engine, &desc, &tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, tile);

  rc = cupertino_dock_tile_get_badge_label(tile, &lbl, &has_badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("3", lbl);
  ASSERT_EQ(1, has_badge);

  rc = cupertino_dock_tile_get_badge_bounds(tile, &bx, &by, &bw, &bh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(24.0f, bw, 0.01f);
  ASSERT_IN_RANGE(24.0f, bh, 0.01f);

  /* Multi-digit badge */
  rc = cupertino_dock_tile_set_badge_label(tile, "99+");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_dock_tile_get_badge_bounds(tile, &bx, &by, &bw, &bh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_GT(bw, 24.0f);

  /* Clear badge */
  rc = cupertino_dock_tile_set_badge_label(tile, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_dock_tile_get_badge_label(tile, &lbl, &has_badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, has_badge);

  rc = cupertino_dock_tile_get_badge_bounds(tile, &bx, &by, &bw, &bh);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.0f, bw, 0.01f);

  /* Empty badge string */
  rc = cupertino_dock_tile_set_badge_label(tile, "");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_dock_tile_get_badge_label(tile, &lbl, &has_badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, has_badge);

  /* Null checks */
  rc = cupertino_dock_tile_set_badge_label(NULL, "1");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_badge_label(NULL, &lbl, &has_badge);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_badge_label(tile, NULL, &has_badge);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_badge_label(tile, &lbl, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* get_badge_bounds null combinations */
  rc = cupertino_dock_tile_get_badge_bounds(NULL, &bx, &by, &bw, &bh);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_badge_bounds(tile, NULL, &by, &bw, &bh);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_badge_bounds(tile, &bx, NULL, &bw, &bh);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_badge_bounds(tile, &bx, &by, NULL, &bh);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_badge_bounds(tile, &bx, &by, &bw, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_dock_tile_destroy(tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_dock_tile_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_dock_tile_progress(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_dock_tile_descriptor desc;
  struct cupertino_dock_tile *tile = NULL;
  float prog = 0.0f;
  int is_vis = 0;
  float px = 0.0f;
  float py = 0.0f;
  float pw = 0.0f;
  float ph = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  rc = cupertino_dock_tile_create(dummy_engine, &desc, &tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set progress */
  rc = cupertino_dock_tile_set_progress(tile, 0.45f, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_dock_tile_get_progress(tile, &prog, &is_vis);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.45f, prog, 0.01f);
  ASSERT_EQ(1, is_vis);

  rc = cupertino_dock_tile_get_progress_bounds(tile, &px, &py, &pw, &ph);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(104.0f, pw, 0.01f);
  ASSERT_IN_RANGE(8.0f, ph, 0.01f);

  /* Invalid progress */
  rc = cupertino_dock_tile_set_progress(tile, 1.5f, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_set_progress(tile, -0.1f, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_set_progress(NULL, 0.5f, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* get_progress null checks */
  rc = cupertino_dock_tile_get_progress(NULL, &prog, &is_vis);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_progress(tile, NULL, &is_vis);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_progress(tile, &prog, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Hide progress */
  rc = cupertino_dock_tile_set_progress(tile, 0.0f, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_dock_tile_get_progress_bounds(tile, &px, &py, &pw, &ph);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.0f, pw, 0.01f);

  /* get_progress_bounds null checks */
  rc = cupertino_dock_tile_get_progress_bounds(NULL, &px, &py, &pw, &ph);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_progress_bounds(tile, NULL, &py, &pw, &ph);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_progress_bounds(tile, &px, NULL, &pw, &ph);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_progress_bounds(tile, &px, &py, NULL, &ph);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_progress_bounds(tile, &px, &py, &pw, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_dock_tile_destroy(tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_dock_tile_actions(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_dock_tile_descriptor desc;
  struct cupertino_dock_tile *tile = NULL;
  const char *title = NULL;
  size_t count = 0;
  int action_id = 0;
  int is_disabled = 0;
  int handled = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  rc = cupertino_dock_tile_create(dummy_engine, &desc, &tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add actions */
  rc = cupertino_dock_tile_add_action(NULL, "New Window", 101);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_add_action(tile, NULL, 101);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_dock_tile_add_action(tile, "New Window", 101);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_dock_tile_add_action(tile, "Open Project...", 102);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_dock_tile_get_action_count(tile, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, count);

  rc = cupertino_dock_tile_get_action_at(tile, 0, &title, &action_id,
                                         &is_disabled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("New Window", title);
  ASSERT_EQ(101, action_id);
  ASSERT_EQ(0, is_disabled);

  /* get_action_at with optional NULL pointers */
  rc = cupertino_dock_tile_get_action_at(tile, 0, NULL, &action_id,
                                         &is_disabled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_dock_tile_get_action_at(tile, 0, &title, NULL, &is_disabled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_dock_tile_get_action_at(tile, 0, &title, &action_id, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_dock_tile_get_action_at(tile, 0, NULL, NULL, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Trigger action */
  rc = cupertino_dock_tile_trigger_action(tile, 101, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, handled);

  /* Trigger disabled action */
  tile->actions[0].is_disabled = 1;
  rc = cupertino_dock_tile_trigger_action(tile, 101, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, handled);
  tile->actions[0].is_disabled = 0;

  /* Unknown action */
  rc = cupertino_dock_tile_trigger_action(tile, 999, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, handled);

  /* Null checks */
  rc = cupertino_dock_tile_get_action_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_action_count(tile, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_action_at(NULL, 0, &title, &action_id,
                                         &is_disabled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_get_action_at(tile, 99, &title, &action_id,
                                         &is_disabled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_trigger_action(NULL, 101, &handled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_dock_tile_trigger_action(tile, 101, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Fill up actions to capacity */
  while (tile->action_count < CUPERTINO_DOCK_TILE_MAX_ACTIONS) {
    rc = cupertino_dock_tile_add_action(tile, "Extra Action", 200);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* Exceed capacity */
  rc = cupertino_dock_tile_add_action(tile, "Overflow", 201);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  rc = cupertino_dock_tile_destroy(tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_dock_tile_oom(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_dock_tile_descriptor desc;
  struct cupertino_dock_tile *tile = NULL;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  g_malloc_fail_countdown = 0;
  rc = cupertino_dock_tile_create(dummy_engine, &desc, &tile);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, tile);

  PASS();
}

SUITE(cupertino_dock_tile_suite) {
  RUN_TEST(test_dock_tile_lifecycle_and_badge);
  RUN_TEST(test_dock_tile_progress);
  RUN_TEST(test_dock_tile_actions);
  RUN_TEST(test_dock_tile_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_dock_tile_suite);
  GREATEST_MAIN_END();
}
