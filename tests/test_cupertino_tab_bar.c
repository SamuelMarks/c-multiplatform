/**
 * @file test_cupertino_tab_bar.c
 * @brief Unit tests for Cupertino Tab Bar component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_tab_bar.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_cupertino_tab_bar_mock_base_create_fail;
extern int g_cupertino_tab_bar_mock_base_destroy_fail;
extern int g_cupertino_tab_bar_mock_append_fail;
extern int g_cupertino_tab_bar_mock_item_create_fail;
extern int g_cupertino_tab_bar_mock_item_destroy_fail;
extern int g_cupertino_tab_bar_mock_set_active_fail;
#endif

SUITE(cupertino_tab_bar_suite);

TEST test_tab_bar_invalid_args(void) {
  struct cupertino_tab_bar_descriptor desc;
  struct cupertino_tab_item_descriptor item_desc;
  struct cupertino_tab_bar *tab_bar = NULL;
  struct ui_bottom_nav_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *badge = NULL;
  size_t idx = 0;
  size_t count = 0;
  float height = 0.0f;
  int present = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  memset(&item_desc, 0, sizeof(item_desc));

  /* Create invalid */
  rc = cupertino_tab_bar_create(NULL, &desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_create(dummy_engine, NULL, &tab_bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Item count > MAX */
  desc.item_count = CUPERTINO_TAB_BAR_MAX_ITEMS + 1;
  rc = cupertino_tab_bar_create(dummy_engine, &desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.item_count = 0;

  /* Destroy invalid */
  rc = cupertino_tab_bar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Add item invalid */
  rc = cupertino_tab_bar_add_item(NULL, &item_desc, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_create(dummy_engine, &desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_bar_add_item(tab_bar, NULL, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  /* NULL out_index on add_item */
  item_desc.label = "Tab";
  rc = cupertino_tab_bar_add_item(tab_bar, &item_desc, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Set selected invalid index */
  rc = cupertino_tab_bar_set_selected(tab_bar, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  /* Get selected NULL */
  rc = cupertino_tab_bar_get_selected(tab_bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  /* Set badge invalid index */
  rc = cupertino_tab_bar_set_badge(tab_bar, 5, "badge");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  /* Set badge empty string */
  rc = cupertino_tab_bar_set_badge(tab_bar, 0, "");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Get badge invalid index and NULL out */
  rc = cupertino_tab_bar_get_badge(tab_bar, 5, &badge);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_badge(tab_bar, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  /* Home indicator NULL */
  rc = cupertino_tab_bar_is_home_indicator_present(tab_bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  /* Height / count / base NULL */
  rc = cupertino_tab_bar_get_height(tab_bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_item_count(tab_bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_base(tab_bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_tab_bar_destroy(tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set / get selected invalid */
  rc = cupertino_tab_bar_set_selected(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_selected(NULL, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / get badge invalid */
  rc = cupertino_tab_bar_set_badge(NULL, 0, "1");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_badge(NULL, 0, &badge);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Home indicator invalid */
  rc = cupertino_tab_bar_set_home_indicator_present(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_is_home_indicator_present(NULL, &present);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Height / count / base invalid */
  rc = cupertino_tab_bar_get_height(NULL, &height);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_item_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_tab_bar_lifecycle_and_items(void) {
  struct cupertino_tab_bar_descriptor desc;
  struct cupertino_tab_item_descriptor items[3];
  struct cupertino_tab_item_descriptor extra_item;
  struct cupertino_tab_bar *tab_bar = NULL;
  struct ui_bottom_nav_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *badge = NULL;
  size_t idx = 0;
  size_t count = 0;
  float height = 0.0f;
  int present = 0;
  ui_error_t rc;

  memset(items, 0, sizeof(items));
  items[0].label = "Home";
  items[0].icon_name = "house.fill";

  items[1].label = "Search";
  items[1].icon_name = "magnifyingglass";
  items[1].badge_text = "5";

  items[2].label = "Profile";
  items[2].icon_name = "person.crop.circle";

  memset(&desc, 0, sizeof(desc));
  desc.is_home_indicator_present = 0;
  desc.items = items;
  desc.item_count = 3;
  desc.initial_index = 0;

  rc = cupertino_tab_bar_create(dummy_engine, &desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tab_bar != NULL);

  /* Check base retrieval */
  rc = cupertino_tab_bar_get_base(tab_bar, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Check item count */
  rc = cupertino_tab_bar_get_item_count(tab_bar, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, count);

  /* Check initial selected */
  rc = cupertino_tab_bar_get_selected(tab_bar, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);

  /* Change selection */
  rc = cupertino_tab_bar_set_selected(tab_bar, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_bar_get_selected(tab_bar, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);

  /* Selection out of bounds */
  rc = cupertino_tab_bar_set_selected(tab_bar, 99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Check badge on item 1 */
  rc = cupertino_tab_bar_get_badge(tab_bar, 1, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(badge != NULL);
  ASSERT_STR_EQ("5", badge);

  /* Check no badge on item 0 */
  rc = cupertino_tab_bar_get_badge(tab_bar, 0, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(badge == NULL);

  /* Update badge */
  rc = cupertino_tab_bar_set_badge(tab_bar, 1, "99+");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_bar_get_badge(tab_bar, 1, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("99+", badge);

  /* Clear badge */
  rc = cupertino_tab_bar_set_badge(tab_bar, 1, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_bar_get_badge(tab_bar, 1, &badge);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(badge == NULL);

  /* Standard height without home indicator = 49pt */
  rc = cupertino_tab_bar_get_height(tab_bar, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(height > 48.9f && height < 49.1f);

  /* Toggle home indicator = 83pt */
  rc = cupertino_tab_bar_set_home_indicator_present(tab_bar, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_bar_is_home_indicator_present(tab_bar, &present);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, present);
  rc = cupertino_tab_bar_get_height(tab_bar, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(height > 82.9f && height < 83.1f);

  /* Add extra items up to CUPERTINO_TAB_BAR_MAX_ITEMS */
  memset(&extra_item, 0, sizeof(extra_item));
  extra_item.label = "Extra";
  extra_item.icon_name = "star";
  for (idx = count; idx < CUPERTINO_TAB_BAR_MAX_ITEMS; idx++) {
    size_t new_idx;
    rc = cupertino_tab_bar_add_item(tab_bar, &extra_item, &new_idx);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(idx, new_idx);
  }

  /* Adding one more should fail with UI_ERROR_OUT_OF_MEMORY */
  rc = cupertino_tab_bar_add_item(tab_bar, &extra_item, &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  rc = cupertino_tab_bar_get_item_count(tab_bar, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_TAB_BAR_MAX_ITEMS, count);

  /* Destroy tab bar */
  rc = cupertino_tab_bar_destroy(tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_tab_bar_oom_mock(void) {
  struct cupertino_tab_bar_descriptor desc;
  struct cupertino_tab_bar *tab_bar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  /* Fail malloc for struct cupertino_tab_bar */
  g_malloc_fail_countdown = 0;
  rc = cupertino_tab_bar_create(dummy_engine, &desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(tab_bar == NULL);

  /* Fail malloc for ui_bottom_nav_base */
  g_malloc_fail_countdown = 1;
  rc = cupertino_tab_bar_create(dummy_engine, &desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(tab_bar == NULL);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_tab_bar_create(NULL, &desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_tab_bar_adaptive_sidebar(void) {
  struct cupertino_tab_bar_descriptor desc;
  struct cupertino_tab_bar *tab_bar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  int enabled = 0;
  float bp = 0.0f;
  float sb_w = 0.0f;
  int is_sidebar = 0;
  float x = 0.0f;
  float y = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  rc = cupertino_tab_bar_create(dummy_engine, &desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Configure adaptive sidebar */
  rc = cupertino_tab_bar_set_adaptive_sidebar(tab_bar, 1, 768.0f, 260.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_tab_bar_get_adaptive_sidebar(tab_bar, &enabled, &bp, &sb_w);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, enabled);
  ASSERT_IN_RANGE(768.0f, bp, 0.01f);
  ASSERT_IN_RANGE(260.0f, sb_w, 0.01f);

  /* Narrow screen (393pt) with adaptive_sidebar_enabled = 0 -> bottom bar */
  tab_bar->adaptive_sidebar_enabled = 0;
  rc = cupertino_tab_bar_update_layout_for_width(tab_bar, 393.0f, &is_sidebar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_sidebar);

  /* Test when adaptive_sidebar_enabled is 1 but viewport_width <
   * sidebar_breakpoint */
  rc = cupertino_tab_bar_set_adaptive_sidebar(tab_bar, 1, 768.0f, 260.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_bar_update_layout_for_width(tab_bar, 500.0f, &is_sidebar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_sidebar);

  rc = cupertino_tab_bar_get_layout_bounds(tab_bar, 393.0f, 852.0f, &x, &y, &w,
                                           &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.0f, x, 0.01f);
  ASSERT_IN_RANGE(393.0f, w, 0.01f);
  ASSERT_IN_RANGE(CUPERTINO_TAB_BAR_HEIGHT_STANDARD, h, 0.01f);

  /* Narrow screen with home indicator */
  rc = cupertino_tab_bar_set_home_indicator_present(tab_bar, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_bar_get_layout_bounds(tab_bar, 393.0f, 852.0f, &x, &y, &w,
                                           &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(CUPERTINO_TAB_BAR_HEIGHT_HOME_INDICATOR, h, 0.01f);

  /* Wide screen (1024pt) -> sidebar */
  rc = cupertino_tab_bar_update_layout_for_width(tab_bar, 1024.0f, &is_sidebar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_sidebar);

  rc = cupertino_tab_bar_is_in_sidebar_mode(tab_bar, &is_sidebar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_sidebar);

  rc = cupertino_tab_bar_get_layout_bounds(tab_bar, 1024.0f, 768.0f, &x, &y, &w,
                                           &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.0f, x, 0.01f);
  ASSERT_IN_RANGE(0.0f, y, 0.01f);
  ASSERT_IN_RANGE(260.0f, w, 0.01f);
  ASSERT_IN_RANGE(768.0f, h, 0.01f);

  /* Test setting adaptive sidebar with negative / zero values to hit fallback
   */
  rc = cupertino_tab_bar_set_adaptive_sidebar(tab_bar, 0, -1.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_tab_bar_get_adaptive_sidebar(tab_bar, &enabled, &bp, &sb_w);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, enabled);
  ASSERT_IN_RANGE(768.0f, bp, 0.01f);
  ASSERT_IN_RANGE(260.0f, sb_w, 0.01f);

  /* Null checks */
  rc = cupertino_tab_bar_set_adaptive_sidebar(NULL, 1, 768.0f, 260.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_adaptive_sidebar(NULL, &enabled, &bp, &sb_w);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_adaptive_sidebar(tab_bar, NULL, &bp, &sb_w);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_adaptive_sidebar(tab_bar, &enabled, NULL, &sb_w);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_adaptive_sidebar(tab_bar, &enabled, &bp, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_update_layout_for_width(NULL, 1024.0f, &is_sidebar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_update_layout_for_width(tab_bar, 0.0f, &is_sidebar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_update_layout_for_width(tab_bar, 1024.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_is_in_sidebar_mode(NULL, &is_sidebar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_is_in_sidebar_mode(tab_bar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_layout_bounds(NULL, 1024.0f, 768.0f, &x, &y, &w,
                                           &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_layout_bounds(tab_bar, 0.0f, 768.0f, &x, &y, &w,
                                           &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_layout_bounds(tab_bar, 1024.0f, 0.0f, &x, &y, &w,
                                           &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_layout_bounds(tab_bar, 1024.0f, 768.0f, NULL, &y,
                                           &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_layout_bounds(tab_bar, 1024.0f, 768.0f, &x, NULL,
                                           &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_layout_bounds(tab_bar, 1024.0f, 768.0f, &x, &y,
                                           NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_tab_bar_get_layout_bounds(tab_bar, 1024.0f, 768.0f, &x, &y, &w,
                                           NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_tab_bar_destroy(tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_tab_bar_mock_failures(void) {
  struct cupertino_tab_bar_descriptor desc;
  struct cupertino_tab_item_descriptor item_desc;
  struct cupertino_tab_bar *tab_bar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  size_t idx;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  memset(&item_desc, 0, sizeof(item_desc));
  item_desc.label = "Tab1";

#ifdef UI_TEST_MOCK_ALLOC
  /* 1. Base create fail */
  g_cupertino_tab_bar_mock_base_create_fail = 1;
  rc = cupertino_tab_bar_create(dummy_engine, &desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(tab_bar == NULL);
  g_cupertino_tab_bar_mock_base_create_fail = 0;

  /* 2. Item create fail during add_item */
  rc = cupertino_tab_bar_create(dummy_engine, &desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_tab_bar_mock_item_create_fail = 1;
  rc = cupertino_tab_bar_add_item(tab_bar, &item_desc, &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_cupertino_tab_bar_mock_item_create_fail = 0;

  /* 3. Base append fail during add_item (with normal item destroy) */
  g_cupertino_tab_bar_mock_append_fail = 1;
  rc = cupertino_tab_bar_add_item(tab_bar, &item_desc, &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_cupertino_tab_bar_mock_append_fail = 0;

  /* 4. Base append fail AND item destroy fail during cleanup */
  g_cupertino_tab_bar_mock_append_fail = 1;
  g_cupertino_tab_bar_mock_item_destroy_fail = 1;
  rc = cupertino_tab_bar_add_item(tab_bar, &item_desc, &idx);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_tab_bar_mock_append_fail = 0;
  g_cupertino_tab_bar_mock_item_destroy_fail = 0;

  /* 5. Set active fail during add_item (with normal item destroy) */
  g_cupertino_tab_bar_mock_set_active_fail = 1;
  rc = cupertino_tab_bar_add_item(tab_bar, &item_desc, &idx);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_tab_bar_mock_set_active_fail = 0;

  /* 5b. Set active fail during add_item AND item destroy fail during cleanup */
  g_cupertino_tab_bar_mock_set_active_fail = 1;
  g_cupertino_tab_bar_mock_item_destroy_fail = 1;
  rc = cupertino_tab_bar_add_item(tab_bar, &item_desc, &idx);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_tab_bar_mock_set_active_fail = 0;
  g_cupertino_tab_bar_mock_item_destroy_fail = 0;

  /* 6. Successfully add an item with NULL label to test item_desc->label ==
   * NULL branch */
  item_desc.label = NULL;
  rc = cupertino_tab_bar_add_item(tab_bar, &item_desc, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  item_desc.label = "Tab1";

  /* 7. Set selected failure */
  g_cupertino_tab_bar_mock_set_active_fail = 1;
  rc = cupertino_tab_bar_set_selected(tab_bar, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_tab_bar_mock_set_active_fail = 0;

  /* 7b. Set selected with items[i].base_item == NULL */
  ui_bottom_nav_item_base_destroy(tab_bar->items[0].base_item);
  tab_bar->items[0].base_item = NULL;
  rc = cupertino_tab_bar_set_selected(tab_bar, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 8. Destroy with item destroy failure */
  /* Add an item with valid base_item first */
  rc = cupertino_tab_bar_add_item(tab_bar, &item_desc, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_tab_bar_mock_item_destroy_fail = 1;
  rc = cupertino_tab_bar_destroy(tab_bar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_tab_bar_mock_item_destroy_fail = 0;

  /* 9. Destroy with base destroy failure */
  /* Remove items first so item destroy succeeds */
  for (idx = 0; idx < tab_bar->item_count; idx++) {
    if (tab_bar->items[idx].base_item) {
      ui_bottom_nav_item_base_destroy(tab_bar->items[idx].base_item);
      tab_bar->items[idx].base_item = NULL;
    }
  }
  g_cupertino_tab_bar_mock_base_destroy_fail = 1;
  rc = cupertino_tab_bar_destroy(tab_bar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_tab_bar_mock_base_destroy_fail = 0;

  /* 9b. Destroy with tab_bar->base == NULL */
  ui_bottom_nav_base_destroy(tab_bar->base);
  tab_bar->base = NULL;
  rc = cupertino_tab_bar_destroy(tab_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 10. Creation with initial items failing add_item */
  desc.items = &item_desc;
  desc.item_count = 1;
  g_cupertino_tab_bar_mock_append_fail = 1;
  rc = cupertino_tab_bar_create(dummy_engine, &desc, &tab_bar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(tab_bar == NULL);
  g_cupertino_tab_bar_mock_append_fail = 0;
#else
  (void)dummy_engine;
  (void)tab_bar;
  (void)idx;
  (void)rc;
#endif

  PASS();
}

SUITE(cupertino_tab_bar_suite) {
  RUN_TEST(test_tab_bar_invalid_args);
  RUN_TEST(test_tab_bar_lifecycle_and_items);
  RUN_TEST(test_tab_bar_adaptive_sidebar);
  RUN_TEST(test_tab_bar_mock_failures);
  RUN_TEST(test_tab_bar_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_tab_bar_suite);
  GREATEST_MAIN_END();
}
