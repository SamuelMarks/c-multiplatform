/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_grid_suite);

TEST test_grid_ctx_create_destroy(void) {
  cmp_grid_ctx_t *ctx = NULL;
  printf("Before create\n");
  ASSERT_EQ(CMP_SUCCESS, cmp_grid_ctx_create(&ctx));
  printf("After create\n");
  ASSERT_NEQ(NULL, ctx);
  ASSERT_EQ(CMP_SUCCESS, cmp_grid_ctx_destroy(ctx));
  printf("After destroy\n");
  PASS();
}

TEST test_grid_track_evaluate(void) {
  cmp_grid_track_size_t track;
  float out_size;
  track.type = CMP_GRID_TRACK_FIXED;
  track.value = 100.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_grid_track_evaluate(&track, 1000.0f, &out_size));
  ASSERT_EQ(100.0f, out_size);

  track.type = CMP_GRID_TRACK_PERCENTAGE;
  track.value = 50.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_grid_track_evaluate(&track, 1000.0f, &out_size));
  ASSERT_EQ(500.0f, out_size);
  PASS();
}

TEST test_grid_add_item(void) {
  cmp_grid_ctx_t *ctx = NULL;
  cmp_layout_node_t *node = NULL;
  cmp_grid_item_t *item = NULL;

  cmp_layout_node_create(&node);
  cmp_grid_ctx_create(&ctx);
  ASSERT_EQ(CMP_SUCCESS, cmp_grid_ctx_add_item(ctx, node, &item));
  ASSERT_EQ(1, ctx->item_count);
  ASSERT_NEQ(NULL, item);

  cmp_grid_ctx_destroy(ctx);
  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_grid_placement_resolve(void) {
  cmp_grid_placement_t start = {0, 0, 0, NULL};
  cmp_grid_placement_t end = {0, 0, 0, NULL};
  int out_start = 0, out_end = 0;

  start.is_auto = 1;
  end.is_auto = 1;
  ASSERT_EQ(CMP_SUCCESS,
            cmp_grid_placement_resolve(&start, &end, 3, &out_start, &out_end));
  ASSERT_EQ(1, out_start);
  ASSERT_EQ(2, out_end);

  start.is_auto = 0;
  start.line = 2;
  end.is_auto = 0;
  end.line = 4;
  ASSERT_EQ(CMP_SUCCESS,
            cmp_grid_placement_resolve(&start, &end, 3, &out_start, &out_end));
  ASSERT_EQ(2, out_start);
  ASSERT_EQ(4, out_end);

  start.is_auto = 0;
  start.line = -1; /* negative indexing */
  end.is_auto = 1;
  end.span = 2;
  ASSERT_EQ(CMP_SUCCESS,
            cmp_grid_placement_resolve(&start, &end, 3, &out_start, &out_end));
  ASSERT_EQ(3, out_start); /* track_count (3) + 1 + (-1) = 3 */
  ASSERT_EQ(5, out_end);   /* 3 + 2 = 5 */

  PASS();
}

TEST test_grid_area_resolve(void) {
  cmp_grid_ctx_t *ctx = NULL;
  int r_start, c_start, r_end, c_end;
  cmp_grid_area_t *areas = NULL;

  cmp_grid_ctx_create(&ctx);
  areas = (cmp_grid_area_t *)malloc(sizeof(cmp_grid_area_t));
  if (!areas)
    FAIL();
  areas[0].name = "header";
  areas[0].row_start = 1;
  areas[0].col_start = 1;
  areas[0].row_end = 2;
  areas[0].col_end = 4;

  ctx->template_areas = areas;
  ctx->template_areas_count = 1;

  ASSERT_EQ(CMP_SUCCESS, cmp_grid_area_resolve(ctx, "header", &r_start,
                                               &c_start, &r_end, &c_end));
  ASSERT_EQ(1, r_start);
  ASSERT_EQ(1, c_start);
  ASSERT_EQ(2, r_end);
  ASSERT_EQ(4, c_end);

  ASSERT_EQ(
      CMP_ERROR_NOT_FOUND,
      cmp_grid_area_resolve(ctx, "footer", &r_start, &c_start, &r_end, &c_end));
  ctx->template_areas = NULL;
  ctx->template_areas_count = 0;
  free(areas);
  cmp_grid_ctx_destroy(ctx);
  PASS();
}

TEST test_grid_auto_place(void) {
  cmp_grid_ctx_t *ctx = NULL;
  cmp_layout_node_t *node = NULL;
  cmp_grid_item_t *item = NULL;

  cmp_layout_node_create(&node);
  cmp_grid_ctx_create(&ctx);
  cmp_grid_ctx_add_item(ctx, node, &item);
  item->row_start.is_auto = 1;
  item->col_start.is_auto = 1;

  ASSERT_EQ(CMP_SUCCESS, cmp_grid_auto_dense_place(ctx));
  ASSERT_EQ(1, item->resolved_row_start);
  ASSERT_EQ(2, item->resolved_row_end);
  ASSERT_EQ(1, item->resolved_col_start);
  ASSERT_EQ(2, item->resolved_col_end);

  ASSERT_EQ(CMP_SUCCESS, cmp_grid_auto_sparse_place(ctx));
  ASSERT_EQ(1, item->resolved_row_start);

  cmp_grid_ctx_destroy(ctx);
  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_grid_implicit_tracks(void) {
  cmp_grid_ctx_t *ctx = NULL;
  cmp_layout_node_t *node = NULL;
  cmp_grid_item_t *item = NULL;

  cmp_layout_node_create(&node);
  cmp_grid_ctx_create(&ctx);
  cmp_grid_ctx_add_item(ctx, node, &item);
  item->resolved_row_start = 4;
  item->resolved_row_end = 5;
  item->resolved_col_start = 4;
  item->resolved_col_end = 5;

  ctx->template_rows_count = 2;
  ctx->template_columns_count = 2;

  ASSERT_EQ(CMP_SUCCESS, cmp_grid_implicit_tracks_generate(ctx));
  ASSERT_EQ(4, ctx->computed_row_count); /* span line 4 to 5 is track 4 */
  ASSERT_EQ(4, ctx->computed_col_count);

  cmp_grid_ctx_destroy(ctx);
  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_grid_subgrid_sync(void) {
  cmp_grid_ctx_t *parent = NULL;
  cmp_grid_ctx_t *child = NULL;

  cmp_grid_ctx_create(&parent);
  cmp_grid_ctx_create(&child);
  parent->computed_col_count = 4;
  ASSERT_EQ(CMP_SUCCESS, cmp_subgrid_sync(parent, child));
  ASSERT_EQ(4, child->computed_col_count);

  cmp_grid_ctx_destroy(child);
  cmp_grid_ctx_destroy(parent);
  PASS();
}

TEST test_grid_align(void) {
  float out_offset;
  ASSERT_EQ(CMP_SUCCESS, cmp_grid_align_evaluate(CMP_GRID_ALIGN_CENTER, 100.0f,
                                                 40.0f, &out_offset));
  ASSERT_EQ(30.0f, out_offset); /* (100 - 40) / 2 */

  ASSERT_EQ(CMP_SUCCESS, cmp_grid_align_evaluate(CMP_GRID_ALIGN_START, 100.0f,
                                                 40.0f, &out_offset));
  ASSERT_EQ(0.0f, out_offset);

  PASS();
}

TEST test_grid_masonry(void) {
  cmp_grid_ctx_t *ctx = NULL;
  cmp_layout_node_t *node1 = NULL;
  cmp_layout_node_t *node2 = NULL;
  cmp_layout_node_t *node3 = NULL;
  cmp_layout_node_t *node4 = NULL;
  cmp_grid_item_t *item1 = NULL;
  cmp_grid_item_t *item2 = NULL;
  cmp_grid_item_t *item3 = NULL;
  cmp_grid_item_t *item4 = NULL;

  cmp_grid_ctx_create(&ctx);
  ctx->computed_col_count = 3;
  ctx->row_gap = 10.0f;

  cmp_layout_node_create(&node1);
  node1->computed_rect.height = 200.0f;
  cmp_grid_ctx_add_item(ctx, node1, &item1);
  cmp_layout_node_create(&node2);
  node2->computed_rect.height = 100.0f;
  cmp_grid_ctx_add_item(ctx, node2, &item2);
  cmp_layout_node_create(&node3);
  node3->computed_rect.height = 150.0f;
  cmp_grid_ctx_add_item(ctx, node3, &item3);
  cmp_layout_node_create(&node4);
  node4->computed_rect.height = 80.0f;
  cmp_grid_ctx_add_item(ctx, node4, &item4);
  ASSERT_EQ(CMP_SUCCESS, cmp_masonry_layout(ctx));

  /* Assert Column Allocations */
  /* Item 1 goes to col 1 (idx 0), height becomes 200 + 10 */
  ASSERT_EQ(1, item1->resolved_col_start);
  ASSERT_EQ(0, item1->resolved_row_start);

  /* Item 2 goes to col 2 (idx 1), height becomes 100 + 10 */
  ASSERT_EQ(2, item2->resolved_col_start);
  ASSERT_EQ(0, item2->resolved_row_start);

  /* Item 3 goes to col 3 (idx 2), height becomes 150 + 10 */
  ASSERT_EQ(3, item3->resolved_col_start);
  ASSERT_EQ(0, item3->resolved_row_start);

  /* Item 4 evaluates mins: Col 1 is 210, Col 2 is 110, Col 3 is 160.
     Smallest is Col 2. Item 4 goes to Col 2. */
  ASSERT_EQ(2, item4->resolved_col_start);
  ASSERT_EQ(110, item4->resolved_row_start);

  cmp_grid_ctx_destroy(ctx);
  cmp_layout_node_destroy(node1);
  cmp_layout_node_destroy(node2);
  cmp_layout_node_destroy(node3);
  cmp_layout_node_destroy(node4);
  PASS();
}

TEST test_grid_gap(void) {
  cmp_grid_ctx_t *ctx = NULL;
  float row_gap = 0.0f, col_gap = 0.0f;

  cmp_grid_ctx_create(&ctx);
  ctx->row_gap = 10.0f;
  ctx->column_gap = 20.0f;
  ctx->computed_row_count = 3;
  ctx->computed_col_count = 3;

  ASSERT_EQ(CMP_SUCCESS, cmp_grid_gap_apply(ctx, &row_gap, &col_gap));
  ASSERT_EQ(10.0f, row_gap);
  ASSERT_EQ(20.0f, col_gap);

  cmp_grid_ctx_destroy(ctx);
  PASS();
}

SUITE(cmp_grid_suite) {
  RUN_TEST(test_grid_ctx_create_destroy);
  RUN_TEST(test_grid_track_evaluate);
  RUN_TEST(test_grid_add_item);
  RUN_TEST(test_grid_placement_resolve);
  RUN_TEST(test_grid_area_resolve);
  RUN_TEST(test_grid_auto_place);
  RUN_TEST(test_grid_implicit_tracks);
  RUN_TEST(test_grid_subgrid_sync);
  RUN_TEST(test_grid_align);
  RUN_TEST(test_grid_masonry);
  RUN_TEST(test_grid_gap);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_grid_suite);
  GREATEST_MAIN_END();
}
