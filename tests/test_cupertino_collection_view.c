/**
 * @file test_cupertino_collection_view.c
 * @brief Unit tests for Cupertino Collection View & Compositional Layout.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_collection_view.h"
#include "ui_engine.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

TEST test_collection_view_lifecycle_and_breakpoints(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct cupertino_collection_view *view = NULL;
  struct cupertino_collection_view_descriptor desc;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  config.num_threads = 1;
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&desc, 0, sizeof(desc));
  desc.initial_width = 375.0f;
  desc.initial_height = 667.0f;

  /* Invalid args */
  rc = cupertino_collection_view_create(NULL, &desc, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_create(engine, NULL, &view);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_create(engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = cupertino_collection_view_create(engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, view);
  ASSERT_EQ(1, view->layout.active_column_count);

  /* Breakpoints */
  rc = cupertino_collection_view_set_breakpoints(NULL, 1, 2, 4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_set_breakpoints(view, 0, 2, 4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_set_breakpoints(view, 1, 0, 4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_set_breakpoints(view, 1, 2, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_collection_view_set_breakpoints(view, 2, 3, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, view->layout.active_column_count); /* width 375 < 600 -> 2 */

  /* Viewport update - regular size (800pt) */
  rc = cupertino_collection_view_update_viewport(NULL, 800.0f, 600.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_update_viewport(view, 0.0f, 600.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_update_viewport(view, 800.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_collection_view_update_viewport(view, 800.0f, 600.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, view->layout.active_column_count);

  /* Viewport update - wide size (1200pt) */
  rc = cupertino_collection_view_update_viewport(view, 1200.0f, 800.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5, view->layout.active_column_count);

  rc = cupertino_collection_view_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_collection_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_compositional_layout_and_geometry(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct cupertino_collection_view *view = NULL;
  struct cupertino_collection_view_descriptor desc;
  struct cupertino_collection_section section;
  struct cupertino_collection_group group;
  size_t s_idx = 0;
  float x, y, w, h;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  config.num_threads = 1;
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&desc, 0, sizeof(desc));
  desc.initial_width = 400.0f;
  desc.initial_height = 800.0f;

  rc = cupertino_collection_view_create(engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Build Section 0 */
  memset(&section, 0, sizeof(section));
  section.content_insets.top = 10.0f;
  section.content_insets.leading = 16.0f;
  section.content_insets.trailing = 16.0f;
  section.content_insets.bottom = 10.0f;
  section.inter_group_spacing = 8.0f;
  section.orthogonal_behavior = CUPERTINO_COLLECTION_ORTHOGONAL_NONE;

  rc = cupertino_collection_view_add_section(NULL, &section, &s_idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_add_section(view, NULL, &s_idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_add_section(view, &section, &s_idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ((size_t)0, s_idx);

  /* Build Group 0 in Section 0: 2 items horizontal, each fractional width 0.5
   */
  memset(&group, 0, sizeof(group));
  group.layout_size.width.type =
      CUPERTINO_COLLECTION_DIMENSION_FRACTIONAL_WIDTH;
  group.layout_size.width.value = 1.0f; /* 400pt wide */
  group.layout_size.height.type = CUPERTINO_COLLECTION_DIMENSION_ABSOLUTE;
  group.layout_size.height.value = 100.0f;
  group.direction = CUPERTINO_COLLECTION_GROUP_HORIZONTAL;
  group.inter_item_spacing = 10.0f;
  group.item_count = 2;

  group.items[0].layout_size.width.type =
      CUPERTINO_COLLECTION_DIMENSION_FRACTIONAL_WIDTH;
  group.items[0].layout_size.width.value = 0.5f;
  group.items[0].layout_size.height.type =
      CUPERTINO_COLLECTION_DIMENSION_FRACTIONAL_HEIGHT;
  group.items[0].layout_size.height.value = 1.0f;
  group.items[0].content_insets.leading = 4.0f;
  group.items[0].content_insets.trailing = 4.0f;
  group.items[0].content_insets.top = 2.0f;
  group.items[0].content_insets.bottom = 2.0f;

  group.items[1].layout_size.width.type =
      CUPERTINO_COLLECTION_DIMENSION_FRACTIONAL_WIDTH;
  group.items[1].layout_size.width.value = 0.5f;
  group.items[1].layout_size.height.type =
      CUPERTINO_COLLECTION_DIMENSION_FRACTIONAL_HEIGHT;
  group.items[1].layout_size.height.value = 1.0f;

  rc = cupertino_collection_view_add_group(NULL, 0, &group);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_add_group(view, 5, &group);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_collection_view_add_group(view, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_collection_view_add_group(view, 0, &group);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Calculate item layouts */
  rc = cupertino_collection_view_get_item_layout(NULL, 0, 0, 0, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_get_item_layout(view, 5, 0, 0, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_collection_view_get_item_layout(view, 0, 5, 0, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);
  rc = cupertino_collection_view_get_item_layout(view, 0, 0, 5, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Item 0: Section inset leading 16 + item inset leading 4 = 20. Y = section
   * top 10 + item top 2 = 12 */
  rc = cupertino_collection_view_get_item_layout(view, 0, 0, 0, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(20.0f, x);
  ASSERT_EQ(12.0f, y);
  ASSERT_EQ(192.0f, w); /* 0.5 * 400 = 200 - 8 (4+4) = 192 */
  ASSERT_EQ(96.0f, h);  /* 100 - 4 (2+2) = 96 */

  /* Item 1 */
  rc = cupertino_collection_view_get_item_layout(view, 0, 0, 1, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(226.0f, x); /* 16 + 200 + 10 inter-item = 226 */
  ASSERT_EQ(10.0f, y);
  ASSERT_EQ(200.0f, w);
  ASSERT_EQ(100.0f, h);

  rc = cupertino_collection_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_orthogonal_scrolling(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct cupertino_collection_view *view = NULL;
  struct cupertino_collection_view_descriptor desc;
  struct cupertino_collection_section section;
  struct cupertino_collection_group group;
  float x, y, w, h;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  config.num_threads = 1;
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&desc, 0, sizeof(desc));
  desc.initial_width = 375.0f;
  desc.initial_height = 667.0f;

  rc = cupertino_collection_view_create(engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&section, 0, sizeof(section));
  section.content_insets.leading = 20.0f;
  section.orthogonal_behavior = CUPERTINO_COLLECTION_ORTHOGONAL_CONTINUOUS;

  rc = cupertino_collection_view_add_section(view, &section, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&group, 0, sizeof(group));
  group.layout_size.width.type = CUPERTINO_COLLECTION_DIMENSION_ABSOLUTE;
  group.layout_size.width.value = 300.0f;
  group.layout_size.height.type = CUPERTINO_COLLECTION_DIMENSION_ABSOLUTE;
  group.layout_size.height.value = 150.0f;
  group.direction = CUPERTINO_COLLECTION_GROUP_HORIZONTAL;
  group.item_count = 1;
  group.items[0].layout_size.width.type =
      CUPERTINO_COLLECTION_DIMENSION_FRACTIONAL_WIDTH;
  group.items[0].layout_size.width.value = 1.0f;
  group.items[0].layout_size.height.type =
      CUPERTINO_COLLECTION_DIMENSION_FRACTIONAL_HEIGHT;
  group.items[0].layout_size.height.value = 1.0f;

  rc = cupertino_collection_view_add_group(view, 0, &group);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Initial X */
  rc = cupertino_collection_view_get_item_layout(view, 0, 0, 0, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(20.0f, x);

  /* Scroll orthogonal by 50pt */
  rc = cupertino_collection_view_scroll_orthogonal(NULL, 0, 50.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_scroll_orthogonal(view, 5, 50.0f);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  rc = cupertino_collection_view_scroll_orthogonal(view, 0, 50.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_collection_view_get_item_layout(view, 0, 0, 0, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(-30.0f, x); /* 20 - 50 = -30 */

  rc = cupertino_collection_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_collection_view_oom_mock(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct cupertino_collection_view *view = NULL;
  struct cupertino_collection_view_descriptor desc;
  struct cupertino_collection_section sec;
  struct cupertino_collection_group group;
  size_t i;
  size_t sec_idx;
  float x, y, w, h;
  ui_error_t rc;
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_collection_view_mock_grid_set_columns_fail;
  extern int g_cupertino_collection_view_mock_grid_destroy_fail;
  extern int g_cupertino_collection_view_mock_grid_add_item_fail;
#endif

  memset(&config, 0, sizeof(config));
  config.num_threads = 1;
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_collection_view_create(engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, view);

  g_malloc_fail_countdown = 1;
  rc = cupertino_collection_view_create(engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, view);
  g_malloc_fail_countdown = -1;
#endif

  /* Create valid view */
  desc.initial_width = 300.0f;
  desc.initial_height = 500.0f;
  rc = cupertino_collection_view_create(engine, &desc, &view);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Max out sections to test CUPERTINO_COLLECTION_VIEW_MAX_SECTIONS */
  memset(&sec, 0, sizeof(sec));
  for (i = 0; i < CUPERTINO_COLLECTION_VIEW_MAX_SECTIONS; i++) {
    rc = cupertino_collection_view_add_section(view, &sec, &sec_idx);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = cupertino_collection_view_add_section(view, &sec, &sec_idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Max out groups in section 0 to test
   * CUPERTINO_COLLECTION_VIEW_MAX_GROUPS_PER_SECTION */
  memset(&group, 0, sizeof(group));
  for (i = 0; i < CUPERTINO_COLLECTION_VIEW_MAX_GROUPS_PER_SECTION; i++) {
    rc = cupertino_collection_view_add_group(view, 0, &group);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = cupertino_collection_view_add_group(view, 0, &group);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  /* Test orthogonal scroll negative clamp branch and NONE behavior */
  rc = cupertino_collection_view_scroll_orthogonal(view, 0, -100.0f);
  ASSERT_EQ(UI_ERROR_UNSUPPORTED, rc);

  view->layout.sections[0].orthogonal_behavior =
      CUPERTINO_COLLECTION_ORTHOGONAL_CONTINUOUS;
  view->layout.sections[0].orthogonal_scroll_offset = 10.0f;
  rc = cupertino_collection_view_scroll_orthogonal(view, 0, -50.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, view->layout.sections[0].orthogonal_scroll_offset);

  /* Exercise with view->base == NULL */
  {
    struct ui_grid_list_base *saved_base = view->base;
    view->base = NULL;

    /* set_breakpoints when active_column_count changes but base is NULL */
    rc = cupertino_collection_view_set_breakpoints(view, 4, 6, 8);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    /* set_breakpoints when active_column_count remains unchanged */
    rc = cupertino_collection_view_set_breakpoints(view, 4, 6, 8);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* update_viewport when active_column_count changes but base is NULL */
    rc = cupertino_collection_view_update_viewport(view, 800.0f, 600.0f);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    /* update_viewport when active_column_count remains unchanged */
    rc = cupertino_collection_view_update_viewport(view, 800.0f, 600.0f);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* add_group when base is NULL */
    memset(&group, 0, sizeof(group));
    group.item_count = 1;
    view->layout.sections[0].group_count = 0;
    rc = cupertino_collection_view_add_group(view, 0, &group);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    view->base = saved_base;
  }

  /* Exercise dimension types in resolve_dimension (FRACTIONAL_HEIGHT,
   * ESTIMATED, DEFAULT) */
  {
    struct cupertino_collection_dimension dim_est;
    dim_est.type = CUPERTINO_COLLECTION_DIMENSION_ESTIMATED;
    dim_est.value = 42.0f;
    view->layout.sections[0].groups[0].layout_size.width = dim_est;
    dim_est.type = (enum cupertino_collection_size_dimension)99;
    dim_est.value = 52.0f;
    view->layout.sections[0].groups[0].layout_size.height = dim_est;
    rc = cupertino_collection_view_get_item_layout(view, 0, 0, 0, &x, &y, &w,
                                                   &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Test cupertino_collection_view_destroy with base == NULL */
  {
    struct cupertino_collection_view *view_null_base = NULL;
    rc = cupertino_collection_view_create(engine, &desc, &view_null_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_grid_list_base_destroy(view_null_base->base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    view_null_base->base = NULL;
    rc = cupertino_collection_view_destroy(view_null_base);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Test cupertino_collection_view_get_item_layout NULL checks for out
   * parameters */
  rc = cupertino_collection_view_get_item_layout(view, 0, 0, 0, NULL, &y, &w,
                                                 &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_get_item_layout(view, 0, 0, 0, &x, NULL, &w,
                                                 &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_get_item_layout(view, 0, 0, 0, &x, &y, NULL,
                                                 &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_get_item_layout(view, 0, 0, 0, &x, &y, &w,
                                                 NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Multi-section and multi-group layout traversal including vertical group */
  view->layout.section_count = 2;
  view->layout.sections[0].content_insets.top = 10.0f;
  view->layout.sections[0].content_insets.bottom = 15.0f;
  view->layout.sections[0].inter_group_spacing = 5.0f;
  view->layout.sections[0].group_count = 2;
  view->layout.sections[0].groups[0].layout_size.height.type =
      CUPERTINO_COLLECTION_DIMENSION_ABSOLUTE;
  view->layout.sections[0].groups[0].layout_size.height.value = 50.0f;
  view->layout.sections[0].groups[1].layout_size.height.type =
      CUPERTINO_COLLECTION_DIMENSION_ABSOLUTE;
  view->layout.sections[0].groups[1].layout_size.height.value = 60.0f;

  view->layout.sections[1].content_insets.top = 20.0f;
  view->layout.sections[1].inter_group_spacing = 4.0f;
  view->layout.sections[1].group_count = 2;
  view->layout.sections[1].groups[0].layout_size.height.type =
      CUPERTINO_COLLECTION_DIMENSION_ABSOLUTE;
  view->layout.sections[1].groups[0].layout_size.height.value = 40.0f;
  view->layout.sections[1].groups[1].layout_size.height.type =
      CUPERTINO_COLLECTION_DIMENSION_ABSOLUTE;
  view->layout.sections[1].groups[1].layout_size.height.value = 80.0f;
  view->layout.sections[1].groups[1].direction =
      CUPERTINO_COLLECTION_GROUP_VERTICAL;
  view->layout.sections[1].groups[1].inter_item_spacing = 8.0f;
  view->layout.sections[1].groups[1].item_count = 2;
  view->layout.sections[1].groups[1].items[0].layout_size.height.type =
      CUPERTINO_COLLECTION_DIMENSION_ABSOLUTE;
  view->layout.sections[1].groups[1].items[0].layout_size.height.value = 30.0f;
  view->layout.sections[1].groups[1].items[1].layout_size.height.type =
      CUPERTINO_COLLECTION_DIMENSION_ABSOLUTE;
  view->layout.sections[1].groups[1].items[1].layout_size.height.value = 30.0f;

  rc = cupertino_collection_view_get_item_layout(view, 1, 1, 1, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  /* Test mock failures */
  g_cupertino_collection_view_mock_grid_set_columns_fail = 1;
  view->layout.active_column_count = 1;
  rc = cupertino_collection_view_set_breakpoints(view, 7, 5, 8);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_collection_view_update_viewport(view, 1200.0f, 600.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_collection_view_mock_grid_set_columns_fail = 0;

  g_cupertino_collection_view_mock_grid_add_item_fail = 1;
  group.item_count = 1;
  view->layout.sections[0].group_count = 0;
  rc = cupertino_collection_view_add_group(view, 0, &group);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_cupertino_collection_view_mock_grid_add_item_fail = 0;

  g_cupertino_collection_view_mock_grid_destroy_fail = 1;
  rc = cupertino_collection_view_destroy(view);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_collection_view_mock_grid_destroy_fail = 0;
  view = NULL;
#else
  rc = cupertino_collection_view_destroy(view);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  view = NULL;
#endif

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(cupertino_collection_view_suite) {
  RUN_TEST(test_collection_view_lifecycle_and_breakpoints);
  RUN_TEST(test_compositional_layout_and_geometry);
  RUN_TEST(test_orthogonal_scrolling);
  RUN_TEST(test_collection_view_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_collection_view_suite);
  GREATEST_MAIN_END();
}
