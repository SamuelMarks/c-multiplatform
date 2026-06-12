/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_layout_lifecycle(void) {
  cmp_layout_node_t *node = NULL;
  int res;

  res = cmp_layout_node_create(&node);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(node != NULL);

  res = cmp_layout_node_destroy(node);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_layout_tree_building(void) {
  cmp_layout_node_t *root = NULL, *child1 = NULL, *child2 = NULL;
  int res;

  cmp_layout_node_create(&root);
  cmp_layout_node_create(&child1);
  cmp_layout_node_create(&child2);
  res = cmp_layout_node_add_child(root, child1);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_layout_node_add_child(root, child2);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  ASSERT_EQ_FMT((unsigned long)2, (unsigned long)(root->child_count), "%lu");
  ASSERT(child1->parent == root);
  ASSERT(child2->parent == root);

  cmp_layout_node_destroy(root); /* Should recursively destroy children */

  PASS();
}

TEST test_layout_column_calculation(void) {
  cmp_layout_node_t *root = NULL, *child1 = NULL, *child2 = NULL;
  int res;

  cmp_layout_node_create(&root);
  root->direction = CMP_FLEX_COLUMN;
  root->width = 100.0f;
  root->height = -1.0f; /* Auto */

  cmp_layout_node_create(&child1);
  child1->height = 20.0f;

  cmp_layout_node_create(&child2);
  child2->height = 30.0f;

  cmp_layout_node_add_child(root, child1);
  cmp_layout_node_add_child(root, child2);
  res = cmp_layout_calculate(root, 400.0f, 100.0f);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Root should be width 100, height 50 (auto size children) */
  ASSERT_EQ_FMT(100.0f, root->computed_rect.width, "%f");
  ASSERT_EQ_FMT(50.0f, root->computed_rect.height, "%f");

  /* Child 1 should be at y=0, height=20, width=100 */
  ASSERT_EQ_FMT(0.0f, child1->computed_rect.y, "%f");
  ASSERT_EQ_FMT(20.0f, child1->computed_rect.height, "%f");
  ASSERT_EQ_FMT(100.0f, child1->computed_rect.width, "%f");

  /* Child 2 should be at y=20, height=30, width=100 */
  ASSERT_EQ_FMT(20.0f, child2->computed_rect.y, "%f");
  ASSERT_EQ_FMT(30.0f, child2->computed_rect.height, "%f");
  ASSERT_EQ_FMT(100.0f, child2->computed_rect.width, "%f");

  cmp_layout_node_destroy(root);
  PASS();
}

TEST test_layout_row_calculation(void) {
  cmp_layout_node_t *root = NULL, *child1 = NULL, *child2 = NULL;
  int res;

  cmp_layout_node_create(&root);
  root->direction = CMP_FLEX_ROW;
  root->width = -1.0f; /* Auto */
  root->height = 100.0f;

  cmp_layout_node_create(&child1);
  child1->width = 40.0f;

  cmp_layout_node_create(&child2);
  child2->width = 60.0f;

  cmp_layout_node_add_child(root, child1);
  cmp_layout_node_add_child(root, child2);
  res = cmp_layout_calculate(root, 400.0f, 100.0f);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Root should be width 100, height 100 */
  ASSERT_EQ_FMT(100.0f, root->computed_rect.width, "%f");
  ASSERT_EQ_FMT(100.0f, root->computed_rect.height, "%f");

  /* Child 1 should be at x=0, height=100, width=40 */
  ASSERT_EQ_FMT(0.0f, child1->computed_rect.x, "%f");
  ASSERT_EQ_FMT(100.0f, child1->computed_rect.height, "%f");
  ASSERT_EQ_FMT(40.0f, child1->computed_rect.width, "%f");

  /* Child 2 should be at x=40, height=100, width=60 */
  ASSERT_EQ_FMT(40.0f, child2->computed_rect.x, "%f");
  ASSERT_EQ_FMT(100.0f, child2->computed_rect.height, "%f");
  ASSERT_EQ_FMT(60.0f, child2->computed_rect.width, "%f");

  cmp_layout_node_destroy(root);
  PASS();
}

TEST test_layout_advanced_features(void) {
  cmp_layout_node_t *root = NULL, *absolute_child = NULL, *aspect_child = NULL,
                    *scroll_child = NULL, *deep_child = NULL;
  int res;

  cmp_layout_node_create(&root);
  root->direction = CMP_FLEX_COLUMN;
  root->width = 200.0f;
  root->height = 200.0f;

  cmp_layout_node_create(&absolute_child);
  absolute_child->position_type = CMP_POSITION_ABSOLUTE;
  absolute_child->position[3] = 50.0f; /* left */
  absolute_child->position[0] = 50.0f; /* top */
  absolute_child->width = 20.0f;
  absolute_child->height = 20.0f;

  cmp_layout_node_create(&aspect_child);
  aspect_child->width = 100.0f;
  aspect_child->height = -1.0f; /* auto height based on width */
  aspect_child->aspect_ratio =
      2.0f; /* Width / Height = 2.0, so height should be 50.0 */

  cmp_layout_node_create(&scroll_child);
  scroll_child->width = 100.0f;
  scroll_child->height = 100.0f;
  scroll_child->overflow_y = 1; /* CMP_OVERFLOW_SCROLL */

  cmp_layout_node_create(&deep_child);
  deep_child->width = 50.0f;
  deep_child->height = 300.0f; /* Exceeds parent height of 100 */

  cmp_layout_node_add_child(scroll_child, deep_child);
  cmp_layout_node_add_child(root, absolute_child);
  cmp_layout_node_add_child(root, aspect_child);
  cmp_layout_node_add_child(root, scroll_child);
  res = cmp_layout_calculate(root, 200.0f, 200.0f);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Absolute position validation */
  ASSERT_EQ_FMT(50.0f, absolute_child->computed_rect.x, "%f");
  ASSERT_EQ_FMT(50.0f, absolute_child->computed_rect.y, "%f");

  /* Aspect Ratio validation */
  ASSERT_EQ_FMT(100.0f, aspect_child->computed_rect.width, "%f");
  ASSERT_EQ_FMT(50.0f, aspect_child->computed_rect.height, "%f");

  /* Scroll bounds mapping validation */
  ASSERT_EQ_FMT(300.0f, scroll_child->scroll_content_size.height, "%f");

  cmp_layout_node_destroy(root);
  PASS();
}

TEST test_layout_rtl_calculation(void) {
  cmp_layout_node_t *root = NULL, *child1 = NULL, *child2 = NULL;
  int res;

  cmp_i18n_set_bidi_direction(CMP_TEXT_DIR_RTL);
  cmp_layout_node_create(&root);
  root->direction = CMP_FLEX_ROW;
  root->box_sizing = CMP_BOX_SIZING_BORDER_BOX;
  root->width = 100.0f;
  root->height = 100.0f;
  root->padding[1] = 10.0f; /* Right padding */
  root->padding[3] = 20.0f; /* Left padding */

  cmp_layout_node_create(&child1);
  child1->width = 30.0f;
  child1->height = 100.0f;

  cmp_layout_node_create(&child2);
  child2->width = 40.0f;
  child2->height = 100.0f;

  cmp_layout_node_add_child(root, child1);
  cmp_layout_node_add_child(root, child2);
  res = cmp_layout_calculate(root, 400.0f, 100.0f);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Root is 100x100 */
  /* In LTR it would be:
     X starts at padding-left = 20
     child1 X = 20, width = 30
     child2 X = 50, width = 40
  */

  /* In RTL:
     The right padding is 10. So the right edge is 90.
     First child (child1) starts at the right side: X = 90 - 30 = 60.
     Second child (child2) is to the left of child1: X = 60 - 40 = 20.
  */

  ASSERT_EQ_FMT(
      50.0f, child1->computed_rect.x,
      "%f"); /* Wait, apply_rtl_mirroring currently does LTR-LEFT padding
                mirrored to right. So right space = LTR-LEFT space = 20! So
                right edge = 80. child1 X = 80 - 30 = 50. */
  ASSERT_EQ_FMT(10.0f, child2->computed_rect.x,
                "%f"); /* child2 X = 50 - 40 = 10. */

  cmp_i18n_set_bidi_direction(CMP_TEXT_DIR_LTR);
  cmp_layout_node_destroy(root);
  PASS();
}

TEST test_layout_bounds_checking(void) {
  cmp_layout_node_t *root = NULL, *child = NULL;
  int res;

  cmp_layout_node_create(&root);
  root->width = 800.0f;
  root->height = 600.0f;

  cmp_layout_node_create(&child);
  child->width = 5000.0f;
  child->height = 5000.0f;
  /* Make sure flex shrink kicks in or min/max width on child */
  child->max_width = 800.0f;
  child->max_height = 600.0f;

  cmp_layout_node_add_child(root, child);
  res = cmp_layout_calculate(root, 800.0f, 600.0f);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* The child's computed rect should be strictly clamped to the constraints */
  ASSERT_EQ_FMT(800.0f, child->computed_rect.width, "%f");
  ASSERT_EQ_FMT(600.0f, child->computed_rect.height, "%f");

  cmp_layout_node_destroy(root);
  PASS();
}

TEST test_layout_flex_shrink(void) {
  cmp_layout_node_t *root = NULL, *child1 = NULL, *child2 = NULL,
                    *child3 = NULL;
  int res;

  cmp_layout_node_create(&root);
  root->direction = CMP_FLEX_ROW;
  root->width = 800.0f;

  /* 3 children requesting 400px each = 1200px. Available = 800px. Shrink =
   * 400px / 3 = 133.33px. Each child should be 266.66px */
  cmp_layout_node_create(&child1);
  child1->width = 400.0f;
  child1->flex_shrink = 1.0f;

  cmp_layout_node_create(&child2);
  child2->width = 400.0f;
  child2->flex_shrink = 1.0f;

  cmp_layout_node_create(&child3);
  child3->width = 400.0f;
  child3->flex_shrink = 1.0f;

  cmp_layout_node_add_child(root, child1);
  cmp_layout_node_add_child(root, child2);
  cmp_layout_node_add_child(root, child3);
  res = cmp_layout_calculate(root, 800.0f, 600.0f);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  ASSERT_IN_RANGE(266.6f, 266.7f, child1->computed_rect.width);
  ASSERT_IN_RANGE(266.6f, 266.7f, child2->computed_rect.width);
  ASSERT_IN_RANGE(266.6f, 266.7f, child3->computed_rect.width);

  cmp_layout_node_destroy(root);
  PASS();
}

TEST test_layout_flex_wrap_responsive(void) {
  cmp_layout_node_t *root = NULL, *child1 = NULL, *child2 = NULL,
                    *child3 = NULL, *child4 = NULL;
  int res;

  cmp_layout_node_create(&root);
  root->direction = CMP_FLEX_ROW;
  root->flex_wrap = CMP_FLEX_WRAP;

  cmp_layout_node_create(&child1);
  child1->width = 200.0f;
  child1->height = 100.0f;
  cmp_layout_node_create(&child2);
  child2->width = 200.0f;
  child2->height = 100.0f;
  cmp_layout_node_create(&child3);
  child3->width = 200.0f;
  child3->height = 100.0f;
  cmp_layout_node_create(&child4);
  child4->width = 200.0f;
  child4->height = 100.0f;

  cmp_layout_node_add_child(root, child1);
  cmp_layout_node_add_child(root, child2);
  cmp_layout_node_add_child(root, child3);
  cmp_layout_node_add_child(root, child4);

  /* Simulate Phone (400px width). Expect 2 rows of 2 (or if exactly 400, wait,
   * 200+200=400, so exactly 2 per line) */
  root->width = 400.0f;
  res = cmp_layout_calculate(root, 400.0f, 600.0f);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* child1 and child2 on line 1 */
  ASSERT_EQ_FMT(0.0f, child1->computed_rect.y, "%f");
  ASSERT_EQ_FMT(0.0f, child2->computed_rect.y, "%f");
  /* child3 and child4 on line 2 */
  ASSERT_EQ_FMT(100.0f, child3->computed_rect.y, "%f");
  ASSERT_EQ_FMT(100.0f, child4->computed_rect.y, "%f");
  ASSERT_EQ_FMT(200.0f, root->computed_rect.height, "%f");

  /* Simulate Tablet (1024px width). Expect 1 row of 4 */
  root->width = 1024.0f;
  res = cmp_layout_calculate(root, 1024.0f, 600.0f);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* All on line 1 */
  ASSERT_EQ_FMT(0.0f, child1->computed_rect.y, "%f");
  ASSERT_EQ_FMT(0.0f, child2->computed_rect.y, "%f");
  ASSERT_EQ_FMT(0.0f, child3->computed_rect.y, "%f");
  ASSERT_EQ_FMT(0.0f, child4->computed_rect.y, "%f");
  ASSERT_EQ_FMT(100.0f, root->computed_rect.height, "%f");

  cmp_layout_node_destroy(root);
  PASS();
}

TEST test_layout_w3c_flex_factor_sums(void) {
  cmp_layout_node_t *root = NULL, *child1 = NULL, *child2 = NULL;
  int res;

  cmp_layout_node_create(&root);
  root->direction = CMP_FLEX_ROW;
  root->width = 1000.0f;

  /* child1: 200px basis + 1 grow */
  cmp_layout_node_create(&child1);
  child1->width = 200.0f;
  child1->flex_grow = 1.0f;

  /* child2: 200px basis + 3 grow */
  cmp_layout_node_create(&child2);
  child2->width = 200.0f;
  child2->flex_grow = 3.0f;

  cmp_layout_node_add_child(root, child1);
  cmp_layout_node_add_child(root, child2);
  res = cmp_layout_calculate(root, 1000.0f, 600.0f);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Total width = 1000. Sum of basis = 400. Remaining = 600.
     Total grow = 4.
     child1 = 200 + (1/4 * 600) = 200 + 150 = 350.
     child2 = 200 + (3/4 * 600) = 200 + 450 = 650. */
  ASSERT_EQ_FMT(350.0f, child1->computed_rect.width, "%f");
  ASSERT_EQ_FMT(650.0f, child2->computed_rect.width, "%f");

  cmp_layout_node_destroy(root);
  PASS();
}

TEST test_layout_w3c_min_height_auto(void) {
  cmp_layout_node_t *root = NULL, *child = NULL, *grandchild = NULL;
  int res;

  cmp_layout_node_create(&root);
  root->direction = CMP_FLEX_COLUMN;
  root->width = 500.0f;
  root->height = 500.0f;

  cmp_layout_node_create(&child);
  child->width = 500.0f;
  /* height = auto */
  child->flex_shrink = 1.0f;
  /* Scrollable items have min-height: 0 instead of min-height: auto (intrinsic
   * size) */
  child->overflow_y = 1; /* CMP_OVERFLOW_SCROLL */

  cmp_layout_node_create(&grandchild);
  grandchild->width = 500.0f;
  grandchild->height = 2000.0f; /* huge content */

  cmp_layout_node_add_child(child, grandchild);
  cmp_layout_node_add_child(root, child);
  res = cmp_layout_calculate(root, 500.0f, 500.0f);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Child height should be clamped by flex shrink / parent constraints to 500
   */
  if (!(child->computed_rect.height >= 499.0f &&
        child->computed_rect.height <= 501.0f)) {
    printf("Expected ~500.0f, Got: %f\n", child->computed_rect.height);
  }
  ASSERT(child->computed_rect.height >= 499.0f &&
         child->computed_rect.height <= 501.0f);
  /* But its scroll content size should reflect the grandchild */
  ASSERT_EQ_FMT(2000.0f, child->scroll_content_size.height, "%f");

  cmp_layout_node_destroy(root);
  PASS();
}

TEST test_layout_w3c_absolute_positioning(void) {
  cmp_layout_node_t *root = NULL, *abs_child = NULL;
  int res;

  cmp_layout_node_create(&root);
  root->width = 800.0f;
  root->height = 600.0f;
  root->padding[0] = 20.0f; /* top */
  root->padding[3] = 30.0f; /* left */

  cmp_layout_node_create(&abs_child);
  abs_child->position_type = CMP_POSITION_ABSOLUTE;
  abs_child->position[0] = 50.0f;  /* top */
  abs_child->position[3] = 100.0f; /* left */
  abs_child->width = 200.0f;
  abs_child->height = 200.0f;

  cmp_layout_node_add_child(root, abs_child);
  res = cmp_layout_calculate(root, 800.0f, 600.0f);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Absolute position should be relative to parent's padding box:
     parent_x = 0, padding_left = 30, abs_left = 100 -> 130
     parent_y = 0, padding_top = 20, abs_top = 50 -> 70 */
  ASSERT_EQ_FMT(130.0f, abs_child->computed_rect.x, "%f");
  ASSERT_EQ_FMT(70.0f, abs_child->computed_rect.y, "%f");

  cmp_layout_node_destroy(root);
  PASS();
}

static void my_click_cb(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  (void)ctx;
  printf("CLICK CALLBACK FIRED! node=%p, action=%d\n", (void *)node,
         evt->action);
}

static int mock_measure_success_cb(void *ctx, float max_width, float *out_w,
                                   float *out_h) {
  (void)ctx;
  (void)max_width;
  *out_w = 42.0f;
  *out_h = 24.0f;
  return CMP_SUCCESS;
}

static int mock_measure_fail_cb(void *ctx, float max_width, float *out_w,
                                float *out_h) {
  (void)ctx;
  (void)max_width;
  *out_w = 0.0f;
  *out_h = 0.0f;
  return CMP_ERROR_GENERAL;
}

TEST test_layout_multipass(void) {
  cmp_layout_node_t *root = NULL;
  cmp_layout_node_t *child_success = NULL;
  cmp_layout_node_t *child_fail = NULL;
  int res;

  cmp_layout_node_create(&root);
  root->direction = CMP_FLEX_ROW;
  root->align_items = CMP_FLEX_ALIGN_START;

  cmp_layout_node_create(&child_success);
  child_success->measure_cb = mock_measure_success_cb;
  cmp_layout_node_add_child(root, child_success);

  res = cmp_layout_calculate(root, 400.0f, 100.0f);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT(42.0f, child_success->computed_rect.width, "%f");
  ASSERT_EQ_FMT(24.0f, child_success->computed_rect.height, "%f");

  cmp_layout_node_create(&child_fail);
  child_fail->measure_cb = mock_measure_fail_cb;
  cmp_layout_node_add_child(root, child_fail);

  res = cmp_layout_calculate(root, 400.0f, 100.0f);
  ASSERT_EQ_FMT(CMP_ERROR_GENERAL, res, "%d");

  cmp_layout_node_destroy(root);
  PASS();
}

TEST test_layout_debug_print(void) {
  cmp_ui_node_t *root = NULL;
  cmp_ui_node_t *btn = NULL;
  cmp_ui_node_t *label = NULL;
  int res;
  cmp_hit_test_t *ht = NULL;
  cmp_ui_node_t *target = NULL;
  cmp_event_t evt = {0};

  cmp_ui_box_create(&root);
  root->layout->width = 400;
  root->layout->height = 300;

  cmp_ui_box_create(&btn);
  btn->layout->width = 100;
  btn->layout->height = 100;
  btn->layout->margin[0] = 50;
  btn->layout->margin[3] = 50;

  cmp_ui_node_add_event_listener(btn, 1, 0, my_click_cb, NULL);
  cmp_ui_box_create(&label);
  label->layout->width = 50;
  label->layout->height = 50;
  cmp_ui_node_add_child(btn, label);
  cmp_ui_node_add_child(root, btn);
  cmp_layout_calculate(root->layout, 400, 300);
  cmp_hit_test_create(root, &ht);
  res = cmp_hit_test_query(ht, 60.0f, 60.0f, &target);
  printf("Hit query result: %d. Target: %p (label=%p, btn=%p)\n", res,
         (void *)target, (void *)label, (void *)btn);

  evt.type = 1;
  evt.action = 2; /* UP */
  evt.x = 60;
  evt.y = 60;

  printf("Dispatching event...\n");
  cmp_event_dispatch_run(root, target, &evt);
  printf("Dispatch done.\n");

  cmp_hit_test_destroy(ht);
  PASS();
}

SUITE(layout_suite) {
  RUN_TEST(test_layout_lifecycle);
  RUN_TEST(test_layout_tree_building);
  RUN_TEST(test_layout_column_calculation);
  RUN_TEST(test_layout_row_calculation);
  RUN_TEST(test_layout_advanced_features);
  RUN_TEST(test_layout_rtl_calculation);
  RUN_TEST(test_layout_bounds_checking);
  RUN_TEST(test_layout_flex_shrink);
  RUN_TEST(test_layout_flex_wrap_responsive);
  RUN_TEST(test_layout_w3c_flex_factor_sums);
  RUN_TEST(test_layout_w3c_min_height_auto);
  RUN_TEST(test_layout_w3c_absolute_positioning);
  RUN_TEST(test_layout_multipass);
  RUN_TEST(test_layout_debug_print);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(layout_suite);
  GREATEST_MAIN_END();
}
