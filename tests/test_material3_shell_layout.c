/**
 * @file test_material3_shell_layout.c
 * @brief Comprehensive tests for Material 3 Shell, Layout, and Workflow
 * Components.
 */

/* clang-format off */
#include "greatest.h"
#include "material3/md3_shell_layout.h"
#include "ui_component.h"
#include "ui_error.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_md3_shell_layout_mock_fail;
extern int g_malloc_fail_countdown;
#endif

static int sample_stepper_validator(struct ui_stepper_base *base,
                                    int step_index, void *user_data) {
  int *allowed_step = (int *)user_data;
  if (base != NULL) {
  }
  if (allowed_step && step_index >= *allowed_step) {
    return 0;
  }
  return 1;
}

TEST test_md3_loading_indicator_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_loading_indicator *ind;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc =
      md3_loading_indicator_create(NULL, MD3_LOADING_INDICATOR_CONTAINED, &ind);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_loading_indicator_create(
      dummy_engine, (enum md3_loading_indicator_style) - 1, &ind);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_loading_indicator_create(dummy_engine,
                                    MD3_LOADING_INDICATOR_CONTAINED, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation: Contained */
  rc = md3_loading_indicator_create(dummy_engine,
                                    MD3_LOADING_INDICATOR_CONTAINED, &ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ind != NULL);
  ASSERT_EQ(MD3_LOADING_INDICATOR_CONTAINED, ind->style);
  ASSERT_EQ(MD3_LOADING_INDICATOR_SIZE_STANDARD, ind->size);
  ASSERT_EQ(0, ind->is_reduced_motion);

  /* Size configuration */
  rc = md3_loading_indicator_set_size(ind,
                                      (enum md3_loading_indicator_size) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_loading_indicator_set_size(ind, MD3_LOADING_INDICATOR_SIZE_COMPACT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_LOADING_INDICATOR_SIZE_COMPACT, ind->size);

  rc = md3_loading_indicator_set_size(ind, MD3_LOADING_INDICATOR_SIZE_LARGE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(MD3_LOADING_INDICATOR_SIZE_LARGE, ind->size);

  /* Animation update normal */
  rc = md3_loading_indicator_update(ind, -0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_loading_indicator_update(ind, 0.5f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_loading_indicator_update(ind, 1.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Reduced motion mode */
  rc = md3_loading_indicator_set_reduced_motion(ind, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, ind->is_reduced_motion);

  rc = md3_loading_indicator_update(ind, 0.4f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_loading_indicator_update(ind, 1.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy */
  rc = md3_loading_indicator_destroy(ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Valid creation: Uncontained */
  rc = md3_loading_indicator_create(dummy_engine,
                                    MD3_LOADING_INDICATOR_UNCONTAINED, &ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_loading_indicator_destroy(ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_stepper_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_stepper *stp;
  enum ui_stepper_step_state state;
  int active;
  int allowed;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_stepper_create(NULL, UI_STEPPER_MODE_LINEAR, MD3_STEPPER_HORIZONTAL,
                          &stp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_stepper_create(dummy_engine, (enum ui_stepper_mode) - 1,
                          MD3_STEPPER_HORIZONTAL, &stp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_LINEAR,
                          (enum md3_stepper_orientation) - 1, &stp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_LINEAR,
                          MD3_STEPPER_HORIZONTAL, &stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(stp != NULL);

  /* Add steps */
  rc = md3_stepper_add_step(stp, "step1", "Account Info", "Enter credentials");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc =
      md3_stepper_add_step(stp, "step2", "Personal Profile", "Contact details");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_add_step(stp, "step3", "Confirmation", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, (int)stp->step_count);

  /* Check initial state of step 0 and 1 */
  rc = md3_stepper_get_step_state(stp, 0, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_STEPPER_STEP_STATE_ACTIVE, state);

  rc = md3_stepper_get_step_state(stp, 1, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_STEPPER_STEP_STATE_DEFAULT, state);

  /* Set state */
  rc = md3_stepper_set_step_state(stp, 1, UI_STEPPER_STEP_STATE_COMPLETED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_get_step_state(stp, 1, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_STEPPER_STEP_STATE_COMPLETED, state);

  /* Set editable */
  rc = md3_stepper_set_editable(stp, 0, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Active index navigation */
  rc = md3_stepper_get_active_index(stp, &active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, active);

  rc = md3_stepper_next_step(stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_get_active_index(stp, &active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, active);

  rc = md3_stepper_prev_step(stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_get_active_index(stp, &active);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, active);

  /* Validation hook test */
  allowed = 1; /* Step 1 will fail validation */
  rc = md3_stepper_set_validate_hook(stp, sample_stepper_validator, &allowed);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Advancing from 0 to 2 requires validating 0 and 1; 1 fails */
  rc = md3_stepper_set_active_index(stp, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy */
  rc = md3_stepper_destroy(stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Non-linear vertical stepper */
  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_NON_LINEAR,
                          MD3_STEPPER_VERTICAL, &stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_add_step(stp, "s1", "First", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_add_step(stp, "s2", "Second", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_set_active_index(stp, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_destroy(stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_scaffold_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_scaffold *scaffold;
  struct ui_component *top_bar;
  struct ui_component *content;
  struct ui_component *side_nav;
  struct ui_component *bottom_bar;
  struct ui_component *fab;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_scaffold_create(NULL, &scaffold);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create scaffold */
  rc = md3_scaffold_create(dummy_engine, &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scaffold != NULL);

  /* Create child components */
  rc = ui_component_create(&top_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&content);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&side_nav);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&bottom_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Assign slots */
  rc = md3_scaffold_set_top_bar(scaffold, top_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_scaffold_set_main_content(scaffold, content);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_scaffold_set_side_nav(scaffold, side_nav);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_scaffold_set_bottom_bar(scaffold, bottom_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_scaffold_set_fab(scaffold, fab, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, scaffold->fab_cradle_docked);

  /* Safe area */
  rc = md3_scaffold_set_safe_area(scaffold, 24.0f, 16.0f, 0.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(24.0f, scaffold->safe_area_top, "%f");
  ASSERT_EQ_FMT(16.0f, scaffold->safe_area_bottom, "%f");

  /* Scroll elevation coordination */
  rc = md3_scaffold_on_scroll(scaffold, 0.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(0.0f, scaffold->top_bar_elevation, "%f");

  rc = md3_scaffold_on_scroll(scaffold, 12.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(2.0f, scaffold->top_bar_elevation, "%f");
  ASSERT_EQ_FMT(2.0f, scaffold->bottom_bar_elevation, "%f");

  /* Cleanup */
  rc = md3_scaffold_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  top_bar->shadow_root = NULL;
  rc = ui_component_destroy(top_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  content->shadow_root = NULL;
  rc = ui_component_destroy(content);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(side_nav);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(bottom_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(fab);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_canonical_layout_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_canonical_layout *layout;
  struct ui_component *body;
  struct ui_component *leading;
  struct ui_component *trailing;
  enum ui_window_size_class sc;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_canonical_layout_create(NULL, MD3_CANONICAL_LAYOUT_LIST_DETAIL,
                                   &layout);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_canonical_layout_create(
      dummy_engine, (enum md3_canonical_layout_type) - 1, &layout);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* List-Detail creation */
  rc = md3_canonical_layout_create(dummy_engine,
                                   MD3_CANONICAL_LAYOUT_LIST_DETAIL, &layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(layout != NULL);
  ASSERT_EQ(MD3_CANONICAL_LAYOUT_LIST_DETAIL, layout->type);

  rc = ui_component_create(&body);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&leading);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&trailing);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_canonical_layout_set_body(layout, body);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_canonical_layout_set_leading_pane(layout, leading);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_canonical_layout_set_trailing_pane(layout, trailing);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Split ratio */
  rc = md3_canonical_layout_set_split_ratio(layout, 0.05f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_canonical_layout_set_split_ratio(layout, 0.40f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Size classes */
  rc =
      md3_canonical_layout_set_size_class(layout, UI_WINDOW_SIZE_CLASS_COMPACT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_canonical_layout_get_size_class(layout, &sc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_WINDOW_SIZE_CLASS_COMPACT, sc);

  rc = md3_canonical_layout_destroy(layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Feed Layout columns adaptation */
  rc = md3_canonical_layout_create(dummy_engine, MD3_CANONICAL_LAYOUT_FEED,
                                   &layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc =
      md3_canonical_layout_set_size_class(layout, UI_WINDOW_SIZE_CLASS_COMPACT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, layout->feed_columns);

  rc = md3_canonical_layout_set_size_class(layout, UI_WINDOW_SIZE_CLASS_MEDIUM);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, layout->feed_columns);

  rc = md3_canonical_layout_set_size_class(layout,
                                           UI_WINDOW_SIZE_CLASS_EXPANDED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, layout->feed_columns);

  rc = md3_canonical_layout_destroy(layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_component_destroy(body);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(leading);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(trailing);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_breadcrumbs_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_breadcrumbs *bc;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_breadcrumbs_create(NULL, &bc);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_breadcrumbs_create(dummy_engine, &bc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(bc != NULL);
  ASSERT_EQ(0, (int)bc->item_count);
  ASSERT_EQ(8, (int)bc->max_items);

  /* Add items */
  rc = md3_breadcrumbs_add_item(bc, "Home", "/home");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_breadcrumbs_add_item(bc, "Settings", "/settings");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_breadcrumbs_add_item(bc, "Security", "/settings/security");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, (int)bc->item_count);
  ASSERT_EQ(2, (int)bc->active_index);

  /* Separator */
  rc = md3_breadcrumbs_set_separator(bc, "•");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("•", bc->separator);

  /* Max items limit */
  rc = md3_breadcrumbs_set_max_items(bc, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_breadcrumbs_set_max_items(bc, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5, (int)bc->max_items);

  /* Simulate click */
  rc = md3_breadcrumbs_simulate_click(bc, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_breadcrumbs_simulate_click(bc, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, (int)bc->active_index);

  /* Destroy */
  rc = md3_breadcrumbs_destroy(bc);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_page_indicator_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_page_indicator *ind;
  int current;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;

  /* Invalid args */
  rc = md3_page_indicator_create(NULL, 5, &ind);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_page_indicator_create(dummy_engine, 0, &ind);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_page_indicator_create(dummy_engine, 5, &ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ind != NULL);
  ASSERT_EQ(5, ind->page_count);

  rc = md3_page_indicator_get_current_page(ind, &current);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, current);

  /* Set current page */
  rc = md3_page_indicator_set_current_page(ind, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_page_indicator_set_current_page(ind, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_page_indicator_set_current_page(ind, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_page_indicator_get_current_page(ind, &current);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, current);

  /* Set page count */
  rc = md3_page_indicator_set_page_count(ind, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_page_indicator_set_page_count(ind, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, ind->page_count);
  /* Clamped current page because 2 >= 2 */
  ASSERT_EQ(1, ind->current_page);

  /* Destroy */
  rc = md3_page_indicator_destroy(ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_shell_layout_branches(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_loading_indicator *ind = NULL;
  struct md3_stepper *stp = NULL;
  struct md3_scaffold *scaffold = NULL;
  struct md3_canonical_layout *layout = NULL;
  struct md3_breadcrumbs *bc = NULL;
  struct md3_page_indicator *pi = NULL;
  struct ui_component comp;
  struct ui_component comp_tb1;
  struct ui_component comp_tb2;
  struct ui_component comp_mc1;
  struct ui_component comp_mc2;
  struct ui_component comp_fab;
  enum ui_stepper_step_state step_state;
  enum ui_window_size_class sc;
  int cur_page = 0;
  int act_idx = 0;
  int i;
  ui_error_t rc;

  memset(&comp_tb1, 0, sizeof(comp_tb1));
  memset(&comp_tb2, 0, sizeof(comp_tb2));
  memset(&comp_mc1, 0, sizeof(comp_mc1));
  memset(&comp_mc2, 0, sizeof(comp_mc2));
  memset(&comp_fab, 0, sizeof(comp_fab));

  /* 1. Loading indicator */
  rc = md3_loading_indicator_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_loading_indicator_set_reduced_motion(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      md3_loading_indicator_set_size(NULL, MD3_LOADING_INDICATOR_SIZE_STANDARD);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_loading_indicator_update(NULL, 1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_loading_indicator_create(dummy_engine,
                                    MD3_LOADING_INDICATOR_CONTAINED, &ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_loading_indicator_set_size(ind, MD3_LOADING_INDICATOR_SIZE_STANDARD);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_loading_indicator_set_reduced_motion(ind, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_loading_indicator_update(ind, 0.05f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_loading_indicator_update(ind, 0.05f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_loading_indicator_set_reduced_motion(ind, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_loading_indicator_update(ind, 0.05f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ind->base = NULL;
  rc = md3_loading_indicator_destroy(ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 2. Stepper */
  rc = md3_stepper_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_LINEAR,
                          MD3_STEPPER_VERTICAL, &stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_destroy(stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_NON_LINEAR,
                          MD3_STEPPER_HORIZONTAL, &stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_destroy(stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_LINEAR,
                          MD3_STEPPER_HORIZONTAL, &stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  stp->base = NULL;
  rc = md3_stepper_destroy(stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_LINEAR,
                          MD3_STEPPER_HORIZONTAL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_LINEAR,
                          MD3_STEPPER_HORIZONTAL, &stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_add_step(NULL, "id", "t", "s");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_add_step(stp, NULL, "t", "s");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_add_step(stp, "id", NULL, "s");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_add_step(stp, "id0", "t0", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 1; i < 32; i++) {
    rc = md3_stepper_add_step(stp, "step", "title", "sub");
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_stepper_add_step(stp, "overflow", "title", "sub");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_stepper_set_step_state(NULL, 0, UI_STEPPER_STEP_STATE_DEFAULT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_set_step_state(stp, -1, UI_STEPPER_STEP_STATE_DEFAULT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_set_step_state(stp, 32, UI_STEPPER_STEP_STATE_DEFAULT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_set_step_state(stp, 0, (enum ui_stepper_step_state)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_set_step_state(stp, 0, UI_STEPPER_STEP_STATE_DEFAULT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_set_step_state(stp, 0, UI_STEPPER_STEP_STATE_ACTIVE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_set_step_state(stp, 0, UI_STEPPER_STEP_STATE_ERROR);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_stepper_get_step_state(NULL, 0, &step_state);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_get_step_state(stp, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_get_step_state(stp, -1, &step_state);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_get_step_state(stp, 32, &step_state);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_get_step_state(stp, 0, &step_state);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_stepper_get_active_index(NULL, &act_idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_get_active_index(stp, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_get_active_index(stp, &act_idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_stepper_set_active_index(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_set_active_index(stp, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_set_active_index(stp, 32);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Next & prev bounds */
  rc = md3_stepper_next_step(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_prev_step(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  stp->active_index = 0;
  rc = md3_stepper_prev_step(stp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  stp->active_index = 31;
  rc = md3_stepper_next_step(stp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_stepper_set_validate_hook(NULL, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_stepper_set_editable(NULL, 0, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_set_editable(stp, -1, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_set_editable(stp, 32, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_stepper_set_editable(stp, 0, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_stepper_set_editable(stp, 0, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* set_active_index when index == active_index */
  stp->active_index = 0;
  rc = md3_stepper_set_active_index(stp, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* set_active_index when old active step is not ACTIVE */
  stp->steps[0].state = UI_STEPPER_STEP_STATE_DEFAULT;
  rc = md3_stepper_set_active_index(stp, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_stepper_destroy(stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 3. Scaffold */
  rc = md3_scaffold_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_create(dummy_engine, &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  scaffold->base = NULL;
  rc = md3_scaffold_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_scaffold_create(dummy_engine, &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_scaffold_set_top_bar(NULL, &comp_tb1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_top_bar(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_bottom_bar(NULL, &comp_tb1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_bottom_bar(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_side_nav(NULL, &comp_tb1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_side_nav(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_main_content(NULL, &comp_mc1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_main_content(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_scaffold_set_top_bar(scaffold, &comp_tb1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &comp_tb2.shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_scaffold_set_top_bar(scaffold, &comp_tb2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_scaffold_set_main_content(scaffold, &comp_mc1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &comp_mc2.shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_scaffold_set_main_content(scaffold, &comp_mc2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_scaffold_set_fab(NULL, &comp_fab, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_fab(scaffold, NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_fab(scaffold, &comp_fab, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_scaffold_set_safe_area(NULL, 0.0f, 0.0f, 0.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_safe_area(scaffold, -1.0f, 0.0f, 0.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_safe_area(scaffold, 0.0f, -1.0f, 0.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_safe_area(scaffold, 0.0f, 0.0f, -1.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_set_safe_area(scaffold, 0.0f, 0.0f, 0.0f, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_scaffold_on_scroll(NULL, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_scaffold_on_scroll(scaffold, 1.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_scaffold_on_scroll(scaffold, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_scaffold_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 4. Canonical Layout */
  rc = md3_canonical_layout_create(
      dummy_engine, MD3_CANONICAL_LAYOUT_SUPPORTING_PANE, &layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_canonical_layout_destroy(layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_canonical_layout_create(dummy_engine,
                                   MD3_CANONICAL_LAYOUT_LIST_DETAIL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_canonical_layout_create(dummy_engine, MD3_CANONICAL_LAYOUT_FEED,
                                   &layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_canonical_layout_set_size_class(NULL, UI_WINDOW_SIZE_CLASS_MEDIUM);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_canonical_layout_set_size_class(layout,
                                           (enum ui_window_size_class)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      md3_canonical_layout_set_size_class(layout, UI_WINDOW_SIZE_CLASS_COMPACT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_canonical_layout_set_size_class(layout, UI_WINDOW_SIZE_CLASS_MEDIUM);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_canonical_layout_set_size_class(layout,
                                           UI_WINDOW_SIZE_CLASS_EXPANDED);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_canonical_layout_get_size_class(NULL, &sc);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_canonical_layout_get_size_class(layout, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_canonical_layout_set_body(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_canonical_layout_set_body(layout, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_canonical_layout_set_leading_pane(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_canonical_layout_set_leading_pane(layout, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_canonical_layout_set_trailing_pane(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_canonical_layout_set_trailing_pane(layout, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_canonical_layout_set_split_ratio(NULL, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_canonical_layout_set_split_ratio(layout, 0.05f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_canonical_layout_set_split_ratio(layout, 0.95f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_canonical_layout_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  layout->base = NULL;
  layout->arena = NULL;
  rc = md3_canonical_layout_destroy(layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 5. Breadcrumbs */
  rc = md3_breadcrumbs_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_breadcrumbs_create(dummy_engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_breadcrumbs_create(dummy_engine, &bc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  bc->base = NULL;
  rc = md3_breadcrumbs_destroy(bc);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_breadcrumbs_create(dummy_engine, &bc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_breadcrumbs_add_item(NULL, "l", "h");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_breadcrumbs_add_item(bc, NULL, "h");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_breadcrumbs_add_item(bc, "home", NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 1; i < 32; i++) {
    rc = md3_breadcrumbs_add_item(bc, "sub", "url");
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = md3_breadcrumbs_add_item(bc, "overflow", "url");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_breadcrumbs_set_separator(NULL, "/");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_breadcrumbs_set_separator(bc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_breadcrumbs_set_max_items(NULL, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_breadcrumbs_set_max_items(bc, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_breadcrumbs_set_max_items(bc, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_breadcrumbs_simulate_click(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_breadcrumbs_simulate_click(bc, 50);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_breadcrumbs_destroy(bc);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 6. Page indicator */
  rc = md3_page_indicator_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_page_indicator_create(NULL, 5, &pi);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_page_indicator_create(dummy_engine, 5, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_page_indicator_create(dummy_engine, 5, &pi);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  pi->base->base.shadow_root = NULL;
  rc = md3_page_indicator_destroy(pi);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_page_indicator_create(dummy_engine, 5, &pi);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  pi->base = NULL;
  rc = md3_page_indicator_destroy(pi);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_page_indicator_create(dummy_engine, 5, &pi);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_page_indicator_set_page_count(NULL, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_page_indicator_set_page_count(pi, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_page_indicator_set_page_count(pi, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_page_indicator_set_current_page(pi, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_page_indicator_set_page_count(pi, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_page_indicator_set_current_page(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_page_indicator_set_current_page(pi, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_page_indicator_set_current_page(pi, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_page_indicator_get_current_page(NULL, &cur_page);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_page_indicator_get_current_page(pi, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_page_indicator_destroy(pi);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_shell_layout_mock_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_loading_indicator *ind = NULL;
  struct md3_stepper *stp = NULL;
  struct md3_scaffold *scaffold = NULL;
  struct md3_canonical_layout *layout = NULL;
  struct md3_breadcrumbs *bc = NULL;
  struct md3_page_indicator *pi = NULL;
  struct ui_component comp;
  ui_error_t rc;

  memset(&comp, 0, sizeof(comp));

  /* Mock 1: loading indicator progress create fails */
  g_md3_shell_layout_mock_fail = 1;
  rc = md3_loading_indicator_create(dummy_engine,
                                    MD3_LOADING_INDICATOR_CONTAINED, &ind);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 2: loading indicator progress set_indeterminate fails */
  g_md3_shell_layout_mock_fail = 2;
  rc = md3_loading_indicator_create(dummy_engine,
                                    MD3_LOADING_INDICATOR_CONTAINED, &ind);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 3: loading indicator progress destroy fails */
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_loading_indicator_create(dummy_engine,
                                    MD3_LOADING_INDICATOR_CONTAINED, &ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_shell_layout_mock_fail = 3;
  rc = md3_loading_indicator_destroy(ind);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_loading_indicator_destroy(ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 4: stepper create fails */
  g_md3_shell_layout_mock_fail = 4;
  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_LINEAR,
                          MD3_STEPPER_HORIZONTAL, &stp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 5: stepper set_mode fails */
  g_md3_shell_layout_mock_fail = 5;
  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_LINEAR,
                          MD3_STEPPER_HORIZONTAL, &stp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 6: stepper destroy fails */
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_LINEAR,
                          MD3_STEPPER_HORIZONTAL, &stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_shell_layout_mock_fail = 6;
  rc = md3_stepper_destroy(stp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_stepper_destroy(stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 7: stepper dom_node_create fails */
  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_LINEAR,
                          MD3_STEPPER_HORIZONTAL, &stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_shell_layout_mock_fail = 7;
  rc = md3_stepper_add_step(stp, "s1", "t1", "d1");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 32: stepper 2nd dom_node_create fails */
  g_md3_shell_layout_mock_fail = 32;
  rc = md3_stepper_add_step(stp, "s2", "t2", "d2");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;

  /* Mock 8: stepper add_step fails */
  g_md3_shell_layout_mock_fail = 8;
  rc = md3_stepper_add_step(stp, "s1", "t1", "d1");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_stepper_add_step(stp, "s1", "t1", "d1");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 9: stepper set_step_state fails */
  g_md3_shell_layout_mock_fail = 9;
  rc = md3_stepper_set_step_state(stp, 0, UI_STEPPER_STEP_STATE_COMPLETED);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 10: stepper set_active_index fails */
  g_md3_shell_layout_mock_fail = 10;
  rc = md3_stepper_set_active_index(stp, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 11: stepper set_validate_hook fails */
  g_md3_shell_layout_mock_fail = 11;
  rc = md3_stepper_set_validate_hook(stp, NULL, NULL);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_stepper_destroy(stp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 12: scaffold create fails */
  g_md3_shell_layout_mock_fail = 12;
  rc = md3_scaffold_create(dummy_engine, &scaffold);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 13: scaffold destroy component fails */
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_scaffold_create(dummy_engine, &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_shell_layout_mock_fail = 13;
  rc = md3_scaffold_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_scaffold_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 14: scaffold set_top_bar fails */
  rc = md3_scaffold_create(dummy_engine, &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_shell_layout_mock_fail = 14;
  rc = md3_scaffold_set_top_bar(scaffold, &comp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Scaffold set_top_bar dom_node_create fails */
  {
    struct ui_component comp_no_shadow;
    memset(&comp_no_shadow, 0, sizeof(comp_no_shadow));
    g_md3_shell_layout_mock_fail = 7;
    rc = md3_scaffold_set_top_bar(scaffold, &comp_no_shadow);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_md3_shell_layout_mock_fail = 0;
  }

  /* Mock 15: scaffold set_main_content fails */
  g_md3_shell_layout_mock_fail = 15;
  rc = md3_scaffold_set_main_content(scaffold, &comp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Scaffold set_main_content dom_node_create fails */
  {
    struct ui_component comp_no_shadow;
    memset(&comp_no_shadow, 0, sizeof(comp_no_shadow));
    g_md3_shell_layout_mock_fail = 7;
    rc = md3_scaffold_set_main_content(scaffold, &comp_no_shadow);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_md3_shell_layout_mock_fail = 0;
  }

  g_md3_shell_layout_mock_fail = 0;
  rc = md3_scaffold_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 16: canonical layout arena create fails */
  g_md3_shell_layout_mock_fail = 16;
  rc = md3_canonical_layout_create(dummy_engine,
                                   MD3_CANONICAL_LAYOUT_LIST_DETAIL, &layout);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 17: canonical layout base create fails */
  g_md3_shell_layout_mock_fail = 17;
  rc = md3_canonical_layout_create(dummy_engine,
                                   MD3_CANONICAL_LAYOUT_LIST_DETAIL, &layout);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 18: canonical layout base destroy fails */
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_canonical_layout_create(dummy_engine,
                                   MD3_CANONICAL_LAYOUT_LIST_DETAIL, &layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_shell_layout_mock_fail = 18;
  rc = md3_canonical_layout_destroy(layout);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 19: canonical layout arena destroy fails */
  g_md3_shell_layout_mock_fail = 19;
  rc = md3_canonical_layout_destroy(layout);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_canonical_layout_destroy(layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 20: canonical layout set_size_class fails */
  rc = md3_canonical_layout_create(dummy_engine,
                                   MD3_CANONICAL_LAYOUT_LIST_DETAIL, &layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_shell_layout_mock_fail = 20;
  rc = md3_canonical_layout_set_size_class(layout, UI_WINDOW_SIZE_CLASS_MEDIUM);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 21: canonical layout set_body fails */
  g_md3_shell_layout_mock_fail = 21;
  rc = md3_canonical_layout_set_body(layout, &comp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 22: canonical layout set_leading_pane fails */
  g_md3_shell_layout_mock_fail = 22;
  rc = md3_canonical_layout_set_leading_pane(layout, &comp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 23: canonical layout set_trailing_pane fails */
  g_md3_shell_layout_mock_fail = 23;
  rc = md3_canonical_layout_set_trailing_pane(layout, &comp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_canonical_layout_destroy(layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 24: breadcrumbs base create fails */
  g_md3_shell_layout_mock_fail = 24;
  rc = md3_breadcrumbs_create(dummy_engine, &bc);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 25: breadcrumbs base destroy fails */
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_breadcrumbs_create(dummy_engine, &bc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_shell_layout_mock_fail = 25;
  rc = md3_breadcrumbs_destroy(bc);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;

  /* Mock 26: breadcrumbs set_path fails */
  g_md3_shell_layout_mock_fail = 26;
  rc = md3_breadcrumbs_add_item(bc, "home", "url");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_breadcrumbs_add_item(bc, "home", "url");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 27: breadcrumbs simulate_click fails */
  g_md3_shell_layout_mock_fail = 27;
  rc = md3_breadcrumbs_simulate_click(bc, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_breadcrumbs_destroy(bc);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 28: page indicator create fails */
  g_md3_shell_layout_mock_fail = 28;
  rc = md3_page_indicator_create(dummy_engine, 3, &pi);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 29: page indicator set_number_of_pages fails */
  g_md3_shell_layout_mock_fail = 29;
  rc = md3_page_indicator_create(dummy_engine, 3, &pi);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 30: page indicator set_current_page fails */
  g_md3_shell_layout_mock_fail = 30;
  rc = md3_page_indicator_create(dummy_engine, 3, &pi);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 29 on set_page_count */
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_page_indicator_create(dummy_engine, 3, &pi);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_shell_layout_mock_fail = 29;
  rc = md3_page_indicator_set_page_count(pi, 5);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 30 on set_current_page */
  g_md3_shell_layout_mock_fail = 30;
  rc = md3_page_indicator_set_current_page(pi, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_page_indicator_destroy(pi);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock 33: page indicator set_number_of_pages fails when shadow_root is NULL
   */
  g_md3_shell_layout_mock_fail = 33;
  rc = md3_page_indicator_create(dummy_engine, 3, &pi);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Mock 34: page indicator set_current_page fails when shadow_root is NULL */
  g_md3_shell_layout_mock_fail = 34;
  rc = md3_page_indicator_create(dummy_engine, 3, &pi);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;

  /* Mock 31: page indicator destroy dom_node_destroy fails */
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_page_indicator_create(dummy_engine, 3, &pi);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_shell_layout_mock_fail = 31;
  rc = md3_page_indicator_destroy(pi);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_md3_shell_layout_mock_fail = 0;
  rc = md3_page_indicator_destroy(pi);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#endif
  PASS();
}

TEST test_md3_shell_layout_oom_alloc(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_loading_indicator *ind = NULL;
  struct md3_stepper *stp = NULL;
  struct md3_scaffold *scaffold = NULL;
  struct md3_canonical_layout *layout = NULL;
  struct md3_breadcrumbs *bc = NULL;
  struct md3_page_indicator *pi = NULL;
  ui_error_t rc;

  /* Loading indicator OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_loading_indicator_create(dummy_engine,
                                    MD3_LOADING_INDICATOR_CONTAINED, &ind);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Stepper OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_stepper_create(dummy_engine, UI_STEPPER_MODE_LINEAR,
                          MD3_STEPPER_HORIZONTAL, &stp);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Scaffold OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_scaffold_create(dummy_engine, &scaffold);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Canonical layout OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_canonical_layout_create(dummy_engine,
                                   MD3_CANONICAL_LAYOUT_LIST_DETAIL, &layout);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Breadcrumbs OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_breadcrumbs_create(dummy_engine, &bc);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Page indicator OOM */
  g_malloc_fail_countdown = 0;
  rc = md3_page_indicator_create(dummy_engine, 3, &pi);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
#endif
  PASS();
}

SUITE(md3_shell_layout_suite) {
  RUN_TEST(test_md3_loading_indicator_lifecycle);
  RUN_TEST(test_md3_stepper_lifecycle);
  RUN_TEST(test_md3_scaffold_lifecycle);
  RUN_TEST(test_md3_canonical_layout_lifecycle);
  RUN_TEST(test_md3_breadcrumbs_lifecycle);
  RUN_TEST(test_md3_page_indicator_lifecycle);
  RUN_TEST(test_md3_shell_layout_branches);
  RUN_TEST(test_md3_shell_layout_mock_failures);
  RUN_TEST(test_md3_shell_layout_oom_alloc);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_shell_layout_suite);
  GREATEST_MAIN_END();
}
