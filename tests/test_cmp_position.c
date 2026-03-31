/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_position_suite);

TEST test_absolute_relative(void) {
  cmp_layout_node_t *node = NULL;
  cmp_rect_t parent = {10.0f, 20.0f, 100.0f, 100.0f};
  cmp_layout_node_create(&node);

  node->position_type = CMP_POSITION_ABSOLUTE;
  node->position[0] = 5.0f;  /* Top */
  node->position[3] = 15.0f; /* Left */

  ASSERT_EQ(CMP_SUCCESS, cmp_pos_absolute_relative(node, &parent));
  ASSERT_EQ(25.0f, node->computed_rect.x); /* parent x (10) + left (15) */
  ASSERT_EQ(25.0f, node->computed_rect.y); /* parent y (20) + top (5) */

  node->position_type = CMP_POSITION_RELATIVE;
  node->computed_rect.x = 50.0f;
  node->computed_rect.y = 50.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_pos_absolute_relative(node, &parent));
  ASSERT_EQ(65.0f, node->computed_rect.x);
  ASSERT_EQ(55.0f, node->computed_rect.y);

  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_fixed_sticky(void) {
  cmp_layout_node_t *node = NULL;
  cmp_rect_t viewport = {0.0f, 0.0f, 1920.0f, 1080.0f};
  cmp_rect_t container = {0.0f, 0.0f, 500.0f, 500.0f};
  cmp_layout_node_create(&node);

  node->position_type = CMP_POSITION_FIXED;
  node->position[0] = 100.0f; /* Top */
  node->position[3] = 200.0f; /* Left */

  ASSERT_EQ(CMP_SUCCESS, cmp_pos_fixed(node, &viewport));
  ASSERT_EQ(200.0f, node->computed_rect.x);
  ASSERT_EQ(100.0f, node->computed_rect.y);

  node->position_type = CMP_POSITION_STICKY;
  node->position[0] = 50.0f; /* Sticky top limit */
  node->computed_rect.y = 100.0f;

  /* Scroll offset < top limit, no change */
  ASSERT_EQ(CMP_SUCCESS, cmp_pos_sticky(node, 20.0f, &container));
  ASSERT_EQ(100.0f, node->computed_rect.y);

  /* Scroll offset > top limit, pushes it down */
  ASSERT_EQ(CMP_SUCCESS, cmp_pos_sticky(node, 80.0f, &container));
  ASSERT_EQ(130.0f, node->computed_rect.y); /* 80 + 50 */

  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_anchor_positioning(void) {
  cmp_layout_node_t *node = NULL;
  cmp_rect_t anchor = {100.0f, 100.0f, 200.0f, 50.0f};
  cmp_rect_t viewport = {0.0f, 0.0f, 1920.0f, 1080.0f};
  cmp_layout_node_create(&node);

  /* Default drop below */
  ASSERT_EQ(CMP_SUCCESS, cmp_anchor_position(node, &anchor));
  ASSERT_EQ(100.0f, node->computed_rect.x);
  ASSERT_EQ(150.0f, node->computed_rect.y);

  /* Anchor size projection */
  ASSERT_EQ(CMP_SUCCESS, cmp_anchor_size(node, &anchor));
  ASSERT_EQ(200.0f, node->computed_rect.width);

  /* Fallback collision testing */
  node->computed_rect.height =
      1000.0f; /* Collides with bottom (150 + 1000 > 1080) */
  ASSERT_EQ(CMP_SUCCESS, cmp_anchor_fallback(node, &anchor, &viewport));
  ASSERT_EQ(-900.0f,
            node->computed_rect.y); /* Flips above anchor: 100 - 1000 = -900 */

  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_stack_context(void) {
  cmp_stack_ctx_t *parent_ctx = NULL;
  cmp_stack_ctx_t *child_ctx1 = NULL;
  cmp_stack_ctx_t *child_ctx2 = NULL;
  cmp_layout_node_t *node = NULL;

  cmp_layout_node_create(&node);
  node->z_index = 10;

  ASSERT_EQ(CMP_SUCCESS, cmp_stack_ctx_create(&parent_ctx, node));
  ASSERT_EQ(10, parent_ctx->z_index);

  node->z_index = 5;
  cmp_stack_ctx_create(&child_ctx1, node);
  node->z_index = 2;
  cmp_stack_ctx_create(&child_ctx2, node);

  ASSERT_EQ(CMP_SUCCESS, cmp_stack_ctx_add_child(parent_ctx, child_ctx1));
  ASSERT_EQ(CMP_SUCCESS, cmp_stack_ctx_add_child(parent_ctx, child_ctx2));

  ASSERT_EQ(2, parent_ctx->child_count);
  ASSERT_EQ(5, parent_ctx->children[0]->z_index);

  ASSERT_EQ(CMP_SUCCESS, cmp_z_index_sort(parent_ctx));
  ASSERT_EQ(2,
            parent_ctx->children[0]->z_index); /* child_ctx2 comes first now */
  ASSERT_EQ(5, parent_ctx->children[1]->z_index);

  cmp_stack_ctx_destroy(child_ctx1);
  cmp_stack_ctx_destroy(child_ctx2);
  cmp_stack_ctx_destroy(parent_ctx);
  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_3d_transform_and_backface(void) {
  cmp_layout_node_t *node = NULL;
  int is_visible;
  cmp_layout_node_create(&node);

  ASSERT_EQ(CMP_SUCCESS, cmp_transform_3d_evaluate(node, 1));

  /* Visible because rotation < 90 */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_backface_visibility_evaluate(node, 1, 45.0f, &is_visible));
  ASSERT_EQ(1, is_visible);

  /* Hidden because backface is turned towards camera */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_backface_visibility_evaluate(node, 1, 180.0f, &is_visible));
  ASSERT_EQ(0, is_visible);

  /* Visible despite backface turning, because hide rule is 0 */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_backface_visibility_evaluate(node, 0, 180.0f, &is_visible));
  ASSERT_EQ(1, is_visible);

  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_popover_and_layers(void) {
  cmp_layout_node_t *node = NULL;
  cmp_layer_t *layer_root = NULL;
  cmp_layout_node_create(&node);

  ASSERT_EQ(CMP_SUCCESS, cmp_top_layer_promote(node));
  ASSERT_EQ(CMP_SUCCESS, cmp_popover_toggle(node, CMP_POPOVER_SHOWING));

  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tree_build(node, &layer_root));
  ASSERT_NEQ(NULL, layer_root);
  ASSERT_EQ(node, layer_root->node);

  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tree_destroy(layer_root));
  cmp_layout_node_destroy(node);
  PASS();
}

SUITE(cmp_position_suite) {
  RUN_TEST(test_absolute_relative);
  RUN_TEST(test_fixed_sticky);
  RUN_TEST(test_anchor_positioning);
  RUN_TEST(test_stack_context);
  RUN_TEST(test_3d_transform_and_backface);
  RUN_TEST(test_popover_and_layers);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_position_suite);
  GREATEST_MAIN_END();
}