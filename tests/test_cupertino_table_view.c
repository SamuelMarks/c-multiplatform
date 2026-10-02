/**
 * @file test_cupertino_table_view.c
 * @brief Unit tests for Cupertino Table View component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_table_view.h"
#include "ui_engine.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

static int g_action_called = 0;

static int dummy_swipe_action(void *user_data) {
  int *flag = (int *)user_data;
  if (flag != NULL) {
    *flag = 1;
  }
  g_action_called = 1;
  return 0;
}

TEST test_table_view_lifecycle_and_sections(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct cupertino_table_view *table = NULL;
  struct cupertino_table_view_descriptor desc;
  struct cupertino_list_section *sec1 = NULL;
  struct cupertino_list_section *sec2 = NULL;
  struct cupertino_list_section_descriptor sec_desc;
  struct cupertino_list_section *ret_sec = NULL;
  size_t count = 0;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  config.num_threads = 1;
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_TABLE_VIEW_INSET_GROUPED;
  desc.edit_mode = 0;
  desc.is_rtl = 0;

  /* Invalid arguments */
  rc = cupertino_table_view_create(NULL, &desc, &table);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_create(engine, NULL, &table);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_create(engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = cupertino_table_view_create(engine, &desc, &table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, table);

  rc = cupertino_table_view_get_section_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_get_section_count(table, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_get_section_count(table, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)0, count);

  /* Create sections */
  memset(&sec_desc, 0, sizeof(sec_desc));
  sec_desc.style = CUPERTINO_LIST_SECTION_INSET_GROUPED;
  sec_desc.header = "GENERAL";
  rc = cupertino_list_section_create(engine, &sec_desc, &sec1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  sec_desc.header = "PRIVACY";
  rc = cupertino_list_section_create(engine, &sec_desc, &sec2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add sections */
  rc = cupertino_table_view_add_section(NULL, sec1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_add_section(table, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_table_view_add_section(table, sec1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_table_view_add_section(table, sec2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add sections until MAX_SECTIONS */
  while (table->section_count < CUPERTINO_TABLE_VIEW_MAX_SECTIONS) {
    rc = cupertino_table_view_add_section(table, sec1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = cupertino_table_view_add_section(table, sec1);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  table->section_count = 2;

  rc = cupertino_table_view_get_section_count(table, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)2, count);

  rc = cupertino_table_view_get_section(NULL, 0, &ret_sec);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_get_section(table, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_get_section(table, 0, &ret_sec);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(sec1, ret_sec);

  rc = cupertino_table_view_get_section(table, 5, &ret_sec);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Edit mode resetting reorder_active */
  table->reorder_active = 1;
  rc = cupertino_table_view_set_edit_mode(table, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, table->reorder_active);
  ASSERT_EQ(-1, table->reorder_from_section);

  /* Edit mode */
  rc = cupertino_table_view_set_edit_mode(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_set_edit_mode(table, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  {
    int edit = 0;
    rc = cupertino_table_view_get_edit_mode(NULL, &edit);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = cupertino_table_view_get_edit_mode(table, NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = cupertino_table_view_get_edit_mode(table, &edit);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(1, edit);
  }

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_cupertino_table_view_mock_list_base_destroy_fail;
    g_cupertino_table_view_mock_list_base_destroy_fail = 1;
    rc = cupertino_table_view_destroy(table);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_cupertino_table_view_mock_list_base_destroy_fail = 0;
  }
#endif

  /* Destroy with base == NULL */
  {
    struct cupertino_table_view *table_no_base = NULL;
    rc = cupertino_table_view_create(engine, &desc, &table_no_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ui_list_base_destroy(table_no_base->base);
    table_no_base->base = NULL;
    rc = cupertino_table_view_destroy(table_no_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = cupertino_table_view_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_list_section_destroy(sec1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_section_destroy(sec2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_table_view_swipe_actions_and_full_swipe(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct cupertino_table_view *table = NULL;
  struct cupertino_table_view_descriptor desc;
  struct cupertino_swipe_action action;
  int user_flag = 0;
  int triggered = 0;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  config.num_threads = 1;
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_TABLE_VIEW_PLAIN;

  rc = cupertino_table_view_create(engine, &desc, &table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&action, 0, sizeof(action));
  strncpy(action.title, "Delete", sizeof(action.title) - 1);
  action.style = CUPERTINO_SWIPE_ACTION_DESTRUCTIVE;
  action.handler = dummy_swipe_action;
  action.user_data = &user_flag;

  /* Invalid args for swipe actions */
  rc = cupertino_table_view_add_trailing_swipe_action(NULL, &action);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_add_trailing_swipe_action(table, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_add_leading_swipe_action(NULL, &action);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_add_leading_swipe_action(table, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_clear_swipe_actions(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_table_view_add_trailing_swipe_action(table, &action);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Trailing swipe limit */
  while (table->trailing_action_count <
         CUPERTINO_TABLE_VIEW_MAX_SWIPE_ACTIONS) {
    rc = cupertino_table_view_add_trailing_swipe_action(table, &action);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = cupertino_table_view_add_trailing_swipe_action(table, &action);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  table->trailing_action_count = 1;

  /* Pan partial swipe (below 60%) negative offset */
  rc = cupertino_table_view_swipe_pan(NULL, 0, 0, -100.0f, 300.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_swipe_pan(table, 0, 0, -100.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_table_view_swipe_pan(table, 0, 0, -100.0f, 300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, table->full_swipe_triggered);

  /* Pan partial swipe (below 60%) positive offset */
  rc = cupertino_table_view_swipe_pan(table, 0, 0, 100.0f, 300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, table->full_swipe_triggered);

  rc = cupertino_table_view_swipe_release(NULL, 300.0f, &triggered);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_swipe_release(table, 300.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_swipe_release(table, 0.0f, &triggered);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_table_view_swipe_release(table, 300.0f, &triggered);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, triggered);
  ASSERT_EQ(0, user_flag);

  /* Full swipe release when trailing_action_count is 0 */
  table->trailing_action_count = 0;
  rc = cupertino_table_view_swipe_pan(table, 0, 0, -200.0f, 300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, table->full_swipe_triggered);
  rc = cupertino_table_view_swipe_release(table, 300.0f, &triggered);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, triggered);
  table->trailing_action_count = 1;

  /* Pan full swipe (> 60% of row width) */
  rc = cupertino_table_view_swipe_pan(table, 0, 0, -200.0f, 300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, table->full_swipe_triggered);

  rc = cupertino_table_view_swipe_release(table, 300.0f, &triggered);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, triggered);
  ASSERT_EQ(1, user_flag);

  /* Trailing action with NULL handler */
  table->trailing_actions[0].handler = NULL;
  rc = cupertino_table_view_swipe_pan(table, 0, 0, -200.0f, 300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_table_view_swipe_release(table, 300.0f, &triggered);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, triggered);

  /* Leading swipe */
  user_flag = 0;
  action.style = CUPERTINO_SWIPE_ACTION_NORMAL;
  strncpy(action.title, "Unread", sizeof(action.title) - 1);
  rc = cupertino_table_view_add_leading_swipe_action(table, &action);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Leading swipe limit */
  while (table->leading_action_count < CUPERTINO_TABLE_VIEW_MAX_SWIPE_ACTIONS) {
    rc = cupertino_table_view_add_leading_swipe_action(table, &action);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = cupertino_table_view_add_leading_swipe_action(table, &action);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  table->leading_action_count = 1;

  /* Full swipe release when leading_action_count is 0 */
  table->leading_action_count = 0;
  rc = cupertino_table_view_swipe_pan(table, 0, 0, 250.0f, 300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, table->full_swipe_triggered);
  rc = cupertino_table_view_swipe_release(table, 300.0f, &triggered);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, triggered);
  table->leading_action_count = 1;

  rc = cupertino_table_view_swipe_pan(table, 0, 0, 250.0f, 300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, table->full_swipe_triggered);

  rc = cupertino_table_view_swipe_release(table, 300.0f, &triggered);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, triggered);
  ASSERT_EQ(1, user_flag);

  /* Leading action with NULL handler */
  table->leading_actions[0].handler = NULL;
  rc = cupertino_table_view_swipe_pan(table, 0, 0, 250.0f, 300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_table_view_swipe_release(table, 300.0f, &triggered);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, triggered);

  rc = cupertino_table_view_clear_swipe_actions(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)0, table->leading_action_count);
  ASSERT_EQ((size_t)0, table->trailing_action_count);

  rc = cupertino_table_view_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_table_view_reorder_and_separators(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct cupertino_table_view *table = NULL;
  struct cupertino_table_view_descriptor desc;
  struct cupertino_list_section *sec = NULL;
  struct cupertino_list_section_descriptor sec_desc;
  struct cupertino_list_tile *tile1 = NULL;
  struct cupertino_list_tile *tile2 = NULL;
  struct cupertino_list_tile_descriptor tile_desc;
  size_t idx1 = 0;
  size_t idx2 = 0;
  float inset = 0.0f;
  int hidden = 0;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  config.num_threads = 1;
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_TABLE_VIEW_INSET_GROUPED;
  desc.edit_mode = 1;

  rc = cupertino_table_view_create(engine, &desc, &table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&sec_desc, 0, sizeof(sec_desc));
  sec_desc.style = CUPERTINO_LIST_SECTION_INSET_GROUPED;
  rc = cupertino_list_section_create(engine, &sec_desc, &sec);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&tile_desc, 0, sizeof(tile_desc));
  tile_desc.title = "Airplane Mode";
  tile_desc.leading_icon = "airplane";
  rc = cupertino_list_tile_create(engine, &tile_desc, &tile1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  tile_desc.title = "Wi-Fi";
  tile_desc.leading_icon = NULL;
  rc = cupertino_list_tile_create(engine, &tile_desc, &tile2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_list_section_add_tile(sec, tile1, &idx1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_section_add_tile(sec, tile2, &idx2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_table_view_add_section(table, sec);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Separator layout tests */
  rc = cupertino_table_view_get_separator_layout(NULL, 0, 0, &inset, &hidden);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_get_separator_layout(table, 0, 0, NULL, &hidden);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_get_separator_layout(table, 0, 0, &inset, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_get_separator_layout(table, 5, 0, &inset, &hidden);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_table_view_get_separator_layout(table, 0, 9, &inset, &hidden);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Row 0 has icon -> inset 56, row 1 is last in INSET_GROUPED -> hidden */
  rc = cupertino_table_view_get_separator_layout(table, 0, 0, &inset, &hidden);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, hidden);
  ASSERT_EQ(CUPERTINO_LIST_SECTION_ICON_INSET, inset);

  rc = cupertino_table_view_get_separator_layout(table, 0, 1, &inset, &hidden);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, hidden);

  /* Plain style separator layout: last cell does NOT hide separator */
  table->style = CUPERTINO_TABLE_VIEW_PLAIN;
  rc = cupertino_table_view_get_separator_layout(table, 0, 1, &inset, &hidden);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, hidden);
  ASSERT_EQ(CUPERTINO_LIST_SECTION_DEFAULT_INSET, inset);

  /* Tile without leading icon */
  tile1->leading_icon[0] = '\0';
  rc = cupertino_table_view_get_separator_layout(table, 0, 0, &inset, &hidden);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, hidden);
  ASSERT_EQ(CUPERTINO_LIST_SECTION_DEFAULT_INSET, inset);
  tile1->leading_icon[0] = 'a';

  /* Null tile in section */
  sec->tiles[0] = NULL;
  rc = cupertino_table_view_get_separator_layout(table, 0, 0, &inset, &hidden);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, hidden);
  ASSERT_EQ(CUPERTINO_LIST_SECTION_DEFAULT_INSET, inset);
  sec->tiles[0] = tile1;
  table->style = CUPERTINO_TABLE_VIEW_INSET_GROUPED;

  /* Reorder unsupported when inactive */
  rc = cupertino_table_view_reorder_update(table, 0, 0);
  ASSERT_EQ(UI_ERROR_UNSUPPORTED, rc);
  rc = cupertino_table_view_reorder_commit(table);
  ASSERT_EQ(UI_ERROR_UNSUPPORTED, rc);

  /* Reorder error branches */
  rc = cupertino_table_view_reorder_begin(NULL, 0, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_reorder_begin(table, -1, 0);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_table_view_reorder_begin(table, 5, 0);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_table_view_reorder_begin(table, 0, -1);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_table_view_reorder_begin(table, 0, 9);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Begin reorder */
  rc = cupertino_table_view_reorder_begin(table, 0, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, table->reorder_active);

  rc = cupertino_table_view_reorder_update(NULL, 0, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_reorder_update(table, -1, 1);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_table_view_reorder_update(table, 5, 1);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_table_view_reorder_update(table, 0, -1);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_table_view_reorder_update(table, 0, 9);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  rc = cupertino_table_view_reorder_update(table, 0, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Commit with reorder_from_row out of bounds */
  table->reorder_from_row = 99;
  rc = cupertino_table_view_reorder_commit(table);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  ASSERT_EQ(0, table->reorder_active);

  rc = cupertino_table_view_reorder_commit(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Reorder commit with target > tile_count */
  rc = cupertino_table_view_reorder_begin(table, 0, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  table->reorder_to_row = 99;
  rc = cupertino_table_view_reorder_commit(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, table->reorder_active);
  ASSERT_EQ(tile2, sec->tiles[0]);
  ASSERT_EQ(tile1, sec->tiles[1]);

  /* Reorder commit moving to row 0 (triggers loop shifting elements) */
  rc = cupertino_table_view_reorder_begin(table, 0, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_table_view_reorder_update(table, 0, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_table_view_reorder_commit(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(tile1, sec->tiles[0]);
  ASSERT_EQ(tile2, sec->tiles[1]);

  /* Reorder commit when dst_sec->tile_count == MAX_TILES */
  {
    struct cupertino_list_section *sec2 = NULL;
    rc = cupertino_list_section_create(engine, &sec_desc, &sec2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    sec2->tile_count = CUPERTINO_LIST_SECTION_MAX_TILES;
    rc = cupertino_table_view_add_section(table, sec2);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = cupertino_table_view_reorder_begin(table, 0, 0);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_table_view_reorder_update(table, 1, 0);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_table_view_reorder_commit(table);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    sec2->tile_count = 0;
    rc = cupertino_list_section_destroy(sec2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    table->section_count = 1;
  }

  /* Reorder cancel */
  rc = cupertino_table_view_reorder_begin(table, 0, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_table_view_reorder_cancel(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_reorder_cancel(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, table->reorder_active);

  /* Edit mode disabled blocks reorder begin */
  rc = cupertino_table_view_set_edit_mode(table, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_table_view_reorder_begin(table, 0, 0);
  ASSERT_EQ(UI_ERROR_UNSUPPORTED, rc);

  rc = cupertino_table_view_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_section_destroy(sec);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_destroy(tile1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_destroy(tile2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_table_view_index_scrub(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct cupertino_table_view *table = NULL;
  struct cupertino_table_view_descriptor desc;
  const char *titles[] = {"A", "B", "C", "D", "E"};
  int selected = -1;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  config.num_threads = 1;
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&desc, 0, sizeof(desc));

  rc = cupertino_table_view_create(engine, &desc, &table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Invalid index titles */
  rc = cupertino_table_view_set_index_titles(NULL, titles, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_set_index_titles(
      table, titles, CUPERTINO_TABLE_VIEW_MAX_INDEX_TITLES + 5);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Scrub when count == 0 */
  rc = cupertino_table_view_scrub_index(table, 50.0f, 100.0f, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(-1, selected);

  /* Titles with NULL pointer and NULL elements */
  rc = cupertino_table_view_set_index_titles(table, NULL, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  {
    const char *null_titles[2];
    null_titles[0] = NULL;
    null_titles[1] = "Z";
    rc = cupertino_table_view_set_index_titles(table, null_titles, 2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = cupertino_table_view_set_index_titles(table, titles, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)5, table->index_scrub.count);

  /* Scrub invalid args */
  rc = cupertino_table_view_scrub_index(NULL, 50.0f, 100.0f, &selected);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_scrub_index(table, 50.0f, 100.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_scrub_index(table, 50.0f, 0.0f, &selected);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Scrub at 50% down 100pt bar -> index 2 ("C") */
  rc = cupertino_table_view_scrub_index(table, 50.0f, 100.0f, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, selected);
  ASSERT_EQ(1, table->index_scrub.is_active);
  ASSERT_EQ(1, table->index_scrub.show_magnifying_bubble);

  /* Scrub negative Y clamp */
  rc = cupertino_table_view_scrub_index(table, -10.0f, 100.0f, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, selected);

  /* Scrub past height clamp */
  rc = cupertino_table_view_scrub_index(table, 120.0f, 100.0f, &selected);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, selected);

  rc = cupertino_table_view_scrub_release(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_table_view_scrub_release(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, table->index_scrub.is_active);
  ASSERT_EQ(0, table->index_scrub.show_magnifying_bubble);

  rc = cupertino_table_view_destroy(table);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_table_view_oom_mock(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct cupertino_table_view *table = NULL;
  struct cupertino_table_view_descriptor desc;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  config.num_threads = 1;
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_table_view_create(engine, &desc, &table);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, table);

  g_malloc_fail_countdown = 1;
  rc = cupertino_table_view_create(engine, &desc, &table);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, table);
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(cupertino_table_view_suite) {
  RUN_TEST(test_table_view_lifecycle_and_sections);
  RUN_TEST(test_table_view_swipe_actions_and_full_swipe);
  RUN_TEST(test_table_view_reorder_and_separators);
  RUN_TEST(test_table_view_index_scrub);
  RUN_TEST(test_table_view_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_table_view_suite);
  GREATEST_MAIN_END();
}
