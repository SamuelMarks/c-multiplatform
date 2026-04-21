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

  ASSERT_EQ_FMT((size_t)2, root->child_count, "%zd");
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

  res = cmp_layout_calculate(root, 100.0f, 100.0f);
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

  res = cmp_layout_calculate(root, 200.0f, 100.0f);
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

  res = cmp_layout_calculate(root, 100.0f, 100.0f);
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

  ASSERT_EQ_FMT(50.0f, child1->computed_rect.x, "%f"); /* Wait, apply_rtl_mirroring currently does LTR-LEFT padding mirrored to right. So right space = LTR-LEFT space = 20! So right edge = 80. child1 X = 80 - 30 = 50. */
  ASSERT_EQ_FMT(10.0f, child2->computed_rect.x, "%f"); /* child2 X = 50 - 40 = 10. */

  cmp_i18n_set_bidi_direction(CMP_TEXT_DIR_LTR);

  cmp_layout_node_destroy(root);
  PASS();
}

static void my_click_cb(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  (void)ctx;
  printf("CLICK CALLBACK FIRED! node=%p, action=%d\n", (void*)node, evt->action);
}

TEST test_layout_debug_print(void) {
  cmp_ui_node_t *root = NULL;
  cmp_ui_node_t *btn = NULL;
  cmp_ui_node_t *label = NULL;

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

  cmp_hit_test_t *ht = NULL;
  cmp_ui_node_t *target = NULL;
  cmp_hit_test_create(root, &ht);
  
  int res = cmp_hit_test_query(ht, 60.0f, 60.0f, &target);
  printf("Hit query result: %d. Target: %p (label=%p, btn=%p)\n", res, (void*)target, (void*)label, (void*)btn);
  
  cmp_event_t evt = {0};
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
  RUN_TEST(test_layout_debug_print);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(layout_suite);
  GREATEST_MAIN_END();
}
