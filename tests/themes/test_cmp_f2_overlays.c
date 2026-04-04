#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_overlays.h"
/* clang-format on */

TEST test_f2_dialog(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_dialog_t *data = NULL;
  int res;

  res = cmp_f2_dialog_create(&node, CMP_F2_DIALOG_VARIANT_STANDARD);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_dialog_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_DIALOG_VARIANT_STANDARD, data->variant);
  ASSERT_EQ(0, data->is_open);

  res = cmp_f2_dialog_set_open(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_open);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_flyout(void) {
  cmp_ui_node_t *node = NULL;
  cmp_ui_node_t *anchor = NULL;
  cmp_f2_flyout_t *data = NULL;
  int res;

  res = cmp_ui_box_create(&anchor);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_flyout_create(&node, anchor);
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_flyout_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_FLYOUT_PLACEMENT_BOTTOM, data->placement);
  ASSERT_EQ(0, data->is_open);

  res = cmp_f2_flyout_set_placement(node, CMP_F2_FLYOUT_PLACEMENT_TOP);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_FLYOUT_PLACEMENT_TOP, data->placement);

  res = cmp_f2_flyout_set_open(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_open);

  cmp_ui_node_destroy(node);
  cmp_ui_node_destroy(anchor);
  PASS();
}

TEST test_f2_tooltip(void) {
  cmp_ui_node_t *node = NULL;
  cmp_ui_node_t *anchor = NULL;
  cmp_f2_tooltip_t *data = NULL;
  int res;

  res = cmp_ui_box_create(&anchor);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_tooltip_create(&node, anchor, "Helper text");
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_tooltip_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(0, data->is_visible);
  ASSERT(data->label_node != NULL);

  res = cmp_f2_tooltip_set_visible(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_visible);

  cmp_ui_node_destroy(node);
  cmp_ui_node_destroy(anchor);
  PASS();
}

TEST test_f2_toast(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_toast_t *data = NULL;
  int res;

  res = cmp_f2_toast_create(&node, CMP_F2_TOAST_INTENT_SUCCESS);
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_toast_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_TOAST_INTENT_SUCCESS, data->intent);
  ASSERT_EQ(1, data->is_visible); /* Default behavior */

  res = cmp_f2_toast_set_visible(node, 0);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, data->is_visible);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_messagebar(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_messagebar_t *data = NULL;
  int res;

  res = cmp_f2_messagebar_create(&node, CMP_F2_TOAST_INTENT_INFO);
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_messagebar_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_MESSAGEBAR_VARIANT_SINGLELINE, data->variant);
  ASSERT_EQ(CMP_F2_TOAST_INTENT_INFO, data->intent);

  res =
      cmp_f2_messagebar_set_variant(node, CMP_F2_MESSAGEBAR_VARIANT_MULTILINE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_MESSAGEBAR_VARIANT_MULTILINE, data->variant);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_progress(void) {
  cmp_ui_node_t *node = NULL;
  cmp_ui_node_t *ring = NULL;
  cmp_f2_progress_t *data = NULL;
  int res;

  res = cmp_f2_progress_bar_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_progress_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_PROGRESS_VARIANT_INDETERMINATE, data->variant);
  ASSERT_EQ(CMP_F2_PROGRESS_STATE_ACTIVE, data->state);
  ASSERT_EQ(2.0f, data->thickness);

  res = cmp_f2_progress_set_variant(node, CMP_F2_PROGRESS_VARIANT_DETERMINATE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_PROGRESS_VARIANT_DETERMINATE, data->variant);

  res = cmp_f2_progress_set_state(node, CMP_F2_PROGRESS_STATE_PAUSED);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_PROGRESS_STATE_PAUSED, data->state);

  res = cmp_f2_progress_set_value(node, 0.75f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0.75f, data->value);

  res = cmp_f2_progress_ring_create(&ring);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(ring != NULL);

  cmp_ui_node_destroy(node);
  cmp_ui_node_destroy(ring);
  PASS();
}

TEST test_f2_skeleton(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_skeleton_t *data = NULL;
  int res;

  res = cmp_f2_skeleton_create(&node, CMP_F2_SKELETON_SHAPE_RECTANGLE);
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_skeleton_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_SKELETON_SHAPE_RECTANGLE, data->shape);
  ASSERT_EQ(1, data->is_animated);

  res = cmp_f2_skeleton_set_animated(node, 0);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, data->is_animated);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_overlays_invalid_args(void) {
  int res;
  cmp_ui_node_t *anchor = NULL;

  res = cmp_f2_dialog_create(NULL, CMP_F2_DIALOG_VARIANT_STANDARD);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_box_create(&anchor);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_flyout_create(NULL, anchor);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_tooltip_create(NULL, anchor, "Tooltip");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_toast_create(NULL, CMP_F2_TOAST_INTENT_INFO);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_messagebar_create(NULL, CMP_F2_TOAST_INTENT_INFO);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_progress_bar_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_skeleton_create(NULL, CMP_F2_SKELETON_SHAPE_RECTANGLE);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_ui_node_destroy(anchor);
  PASS();
}

SUITE(cmp_f2_overlays_suite) {
  RUN_TEST(test_f2_dialog);
  RUN_TEST(test_f2_flyout);
  RUN_TEST(test_f2_tooltip);
  RUN_TEST(test_f2_toast);
  RUN_TEST(test_f2_messagebar);
  RUN_TEST(test_f2_progress);
  RUN_TEST(test_f2_skeleton);
  RUN_TEST(test_f2_overlays_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_overlays_suite);
  GREATEST_MAIN_END();
}
