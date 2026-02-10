#include "m3/m3_layout.h"
#include "test_utils.h"

typedef struct MeasureCtx {
  M3Scalar width;
  M3Scalar height;
  M3Bool fail;
  int called;
} MeasureCtx;

typedef struct MutateCtx {
  M3LayoutNode *target;
  int called;
} MutateCtx;

static int m3_layout_near(M3Scalar a, M3Scalar b, M3Scalar tol) {
  M3Scalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }

  return (diff <= tol) ? 1 : 0;
}

static int m3_layout_measure_cb(void *ctx, M3LayoutMeasureSpec width,
                                M3LayoutMeasureSpec height, M3Size *out_size) {
  MeasureCtx *measure;

  if (ctx == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  measure = (MeasureCtx *)ctx;
  measure->called += 1;

  if (measure->fail) {
    return M3_ERR_UNKNOWN;
  }

  if (width.mode == M3_LAYOUT_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else {
    out_size->width = measure->width;
  }

  if (height.mode == M3_LAYOUT_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else {
    out_size->height = measure->height;
  }

  return M3_OK;
}

static int m3_layout_measure_mutate(void *ctx, M3LayoutMeasureSpec width,
                                    M3LayoutMeasureSpec height,
                                    M3Size *out_size) {
  MutateCtx *state;

  M3_UNUSED(width);
  M3_UNUSED(height);

  if (ctx == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (MutateCtx *)ctx;
  state->called += 1;
  if (state->target != NULL) {
    state->target->style.align_main = 99;
  }

  out_size->width = 1.0f;
  out_size->height = 1.0f;
  return M3_OK;
}

int main(void) {
  M3LayoutMeasureSpec spec;
  M3LayoutMeasureSpec spec_unspec;
  M3LayoutMeasureSpec spec_exact_60;
  M3LayoutMeasureSpec spec_exact_40;
  M3LayoutMeasureSpec spec_exact_25;
  M3LayoutMeasureSpec spec_exact_30;
  M3LayoutMeasureSpec spec_exact_50;
  M3LayoutMeasureSpec spec_at_most_15;
  M3LayoutDirection direction;
  M3LayoutDirection direction_rtl;
  M3LayoutStyle style;
  M3LayoutNode node;
  M3LayoutNode child1;
  M3LayoutNode child2;
  M3LayoutNode child3;
  M3LayoutNode *children[3];
  M3LayoutNode container;
  M3LayoutEdges edges;
  M3Rect rect;
  M3Size size;
  M3Scalar resolved;
  M3Scalar clamped;
  M3Scalar padding_left;
  M3Scalar padding_right;
  MeasureCtx measure_ctx;
  MutateCtx mutate_ctx;
  M3Scalar out_size;
  int rc;

  M3_TEST_EXPECT(
      m3_layout_measure_spec_init(NULL, M3_LAYOUT_MEASURE_UNSPECIFIED, 0.0f),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_measure_spec_init(&spec, 99, 0.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_layout_measure_spec_init(&spec, M3_LAYOUT_MEASURE_EXACTLY, -1.0f),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_layout_measure_spec_init(&spec_unspec,
                                         M3_LAYOUT_MEASURE_UNSPECIFIED, 0.0f));
  M3_TEST_OK(m3_layout_measure_spec_init(&spec_exact_60,
                                         M3_LAYOUT_MEASURE_EXACTLY, 60.0f));
  M3_TEST_OK(m3_layout_measure_spec_init(&spec_exact_40,
                                         M3_LAYOUT_MEASURE_EXACTLY, 40.0f));
  M3_TEST_OK(m3_layout_measure_spec_init(&spec_exact_25,
                                         M3_LAYOUT_MEASURE_EXACTLY, 25.0f));
  M3_TEST_OK(m3_layout_measure_spec_init(&spec_exact_30,
                                         M3_LAYOUT_MEASURE_EXACTLY, 30.0f));
  M3_TEST_OK(m3_layout_measure_spec_init(&spec_exact_50,
                                         M3_LAYOUT_MEASURE_EXACTLY, 50.0f));
  M3_TEST_OK(m3_layout_measure_spec_init(&spec_at_most_15,
                                         M3_LAYOUT_MEASURE_AT_MOST, 15.0f));
  M3_TEST_OK(m3_layout_direction_init(&direction, M3_DIRECTION_LTR));
  M3_TEST_OK(m3_layout_direction_init(&direction_rtl, M3_DIRECTION_RTL));
  M3_TEST_EXPECT(m3_layout_test_validate_direction(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  direction.flow = 99u;
  M3_TEST_EXPECT(m3_layout_test_validate_direction(&direction),
                 M3_ERR_INVALID_ARGUMENT);
  direction.flow = M3_DIRECTION_LTR;
  M3_TEST_OK(m3_layout_test_validate_direction(&direction));
  M3_TEST_EXPECT(m3_layout_test_clamp_non_negative(-1.0f, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_layout_test_clamp_non_negative(-1.0f, &clamped));
  M3_TEST_ASSERT(clamped == 0.0f);
  M3_TEST_OK(m3_layout_test_clamp_non_negative(2.0f, &clamped));
  M3_TEST_ASSERT(clamped == 2.0f);
  M3_TEST_EXPECT(
      m3_layout_test_resolve_available(M3_LAYOUT_AUTO, spec_unspec, NULL),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(
      m3_layout_test_resolve_available(M3_LAYOUT_AUTO, spec_unspec, &resolved));
  M3_TEST_ASSERT(resolved == -1.0f);
  M3_TEST_OK(m3_layout_test_resolve_available(10.0f, spec_unspec, &resolved));
  M3_TEST_ASSERT(resolved == 10.0f);
  M3_TEST_OK(m3_layout_test_resolve_available(M3_LAYOUT_AUTO, spec_exact_50,
                                              &resolved));
  M3_TEST_ASSERT(resolved == 50.0f);
  M3_TEST_OK(
      m3_layout_test_resolve_available(10.0f, spec_at_most_15, &resolved));
  M3_TEST_ASSERT(resolved == 10.0f);
  M3_TEST_OK(
      m3_layout_test_resolve_available(20.0f, spec_at_most_15, &resolved));
  M3_TEST_ASSERT(resolved == 15.0f);

  M3_TEST_EXPECT(m3_layout_edges_set(NULL, 0.0f, 0.0f, 0.0f, 0.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_edges_set(&edges, -1.0f, 0.0f, 0.0f, 0.0f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_layout_edges_set(&edges, 1.0f, 2.0f, 3.0f, 4.0f));
  M3_TEST_EXPECT(m3_layout_test_resolve_horizontal_padding(
                     NULL, &edges, &padding_left, &padding_right),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_test_resolve_horizontal_padding(
                     &direction, NULL, &padding_left, &padding_right),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_test_resolve_horizontal_padding(
                     &direction, &edges, NULL, &padding_right),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_test_resolve_horizontal_padding(&direction, &edges,
                                                           &padding_left, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_layout_test_resolve_horizontal_padding(
      &direction, &edges, &padding_left, &padding_right));
  M3_TEST_ASSERT(padding_left == 1.0f);
  M3_TEST_ASSERT(padding_right == 3.0f);
  M3_TEST_OK(m3_layout_test_resolve_horizontal_padding(
      &direction_rtl, &edges, &padding_left, &padding_right));
  M3_TEST_ASSERT(padding_left == 3.0f);
  M3_TEST_ASSERT(padding_right == 1.0f);

  M3_TEST_EXPECT(m3_layout_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_layout_style_init(&style));
  M3_TEST_ASSERT(style.direction == M3_LAYOUT_DIRECTION_ROW);

  M3_TEST_EXPECT(m3_layout_node_init(NULL, &style), M3_ERR_INVALID_ARGUMENT);

  style.direction = 99;
  M3_TEST_EXPECT(m3_layout_node_init(&node, &style), M3_ERR_INVALID_ARGUMENT);
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.wrap = 99;
  M3_TEST_EXPECT(m3_layout_node_init(&node, &style), M3_ERR_INVALID_ARGUMENT);
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.align_main = M3_LAYOUT_ALIGN_STRETCH;
  M3_TEST_EXPECT(m3_layout_node_init(&node, &style), M3_ERR_INVALID_ARGUMENT);
  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_SPACE_BETWEEN;
  M3_TEST_EXPECT(m3_layout_node_init(&node, &style), M3_ERR_INVALID_ARGUMENT);
  style.align_cross = M3_LAYOUT_ALIGN_START;
  style.padding.left = -1.0f;
  M3_TEST_EXPECT(m3_layout_node_init(&node, &style), M3_ERR_RANGE);
  style.padding.left = 0.0f;
  style.width = -2.0f;
  M3_TEST_EXPECT(m3_layout_node_init(&node, &style), M3_ERR_RANGE);
  style.width = M3_LAYOUT_AUTO;
  style.height = -3.0f;
  M3_TEST_EXPECT(m3_layout_node_init(&node, &style), M3_ERR_RANGE);

  M3_TEST_OK(m3_layout_style_init(&style));
  M3_TEST_OK(m3_layout_node_init(&node, &style));
  M3_TEST_OK(m3_layout_node_init(&node, NULL));
#ifdef M3_TESTING
  M3_TEST_OK(m3_layout_test_set_style_init_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_layout_node_init(&node, NULL), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_layout_test_set_style_init_fail(M3_FALSE));
#endif
  M3_TEST_EXPECT(m3_layout_node_set_children(NULL, NULL, 0),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_node_set_children(&node, NULL, 1),
                 M3_ERR_INVALID_ARGUMENT);
  children[0] = NULL;
  M3_TEST_EXPECT(m3_layout_node_set_children(&node, children, 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_layout_node_set_children(&node, NULL, 0));
  M3_TEST_OK(m3_layout_node_set_measure(&node, NULL, NULL));
  M3_TEST_EXPECT(m3_layout_node_set_measure(NULL, NULL, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_node_get_measured(NULL, &size),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_node_get_measured(&node, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_node_get_layout(NULL, &rect),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_node_get_layout(&node, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_compute(NULL, &direction, spec_unspec, spec_unspec),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_compute(&node, NULL, spec_unspec, spec_unspec),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_layout_test_validate_measure_spec(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = 99;
  spec.size = 0.0f;
  M3_TEST_EXPECT(m3_layout_test_validate_measure_spec(&spec),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_LAYOUT_MEASURE_AT_MOST;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_layout_test_validate_measure_spec(&spec), M3_ERR_RANGE);
  spec.mode = M3_LAYOUT_MEASURE_UNSPECIFIED;
  spec.size = 0.0f;
  M3_TEST_OK(m3_layout_test_validate_measure_spec(&spec));

  M3_TEST_EXPECT(m3_layout_test_validate_style(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_layout_style_init(&style));
  M3_TEST_OK(m3_layout_test_validate_style(&style));

  M3_TEST_EXPECT(m3_layout_test_apply_spec(1.0f, spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_LAYOUT_MEASURE_EXACTLY;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_layout_test_apply_spec(1.0f, spec, &out_size),
                 M3_ERR_RANGE);
  spec.mode = M3_LAYOUT_MEASURE_AT_MOST;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_layout_test_apply_spec(1.0f, spec, &out_size),
                 M3_ERR_RANGE);
  spec.mode = 99;
  spec.size = 0.0f;
  M3_TEST_EXPECT(m3_layout_test_apply_spec(1.0f, spec, &out_size),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_LAYOUT_MEASURE_EXACTLY;
  spec.size = 5.0f;
  M3_TEST_OK(m3_layout_test_apply_spec(1.0f, spec, &out_size));
  M3_TEST_ASSERT(out_size == 5.0f);
  spec.mode = M3_LAYOUT_MEASURE_AT_MOST;
  spec.size = 0.5f;
  M3_TEST_OK(m3_layout_test_apply_spec(1.0f, spec, &out_size));
  M3_TEST_ASSERT(out_size == 0.5f);

  M3_TEST_OK(m3_layout_style_init(&style));
  M3_TEST_OK(m3_layout_node_init(&node, &style));
  M3_TEST_EXPECT(
      m3_layout_test_measure_leaf(NULL, spec_unspec, spec_unspec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_layout_test_measure_leaf(&node, spec_unspec, spec_unspec, NULL),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_layout_test_measure_row(NULL, spec_unspec, spec_unspec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_layout_test_measure_row(&node, spec_unspec, spec_unspec, NULL),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_layout_test_measure_column(NULL, spec_unspec, spec_unspec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_layout_test_measure_column(&node, spec_unspec, spec_unspec, NULL),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_test_measure_node(NULL, spec_unspec, spec_unspec),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_test_layout_node(NULL, 0.0f, 0.0f, 1.0f, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);

  spec.mode = 99;
  spec.size = 0.0f;
  M3_TEST_EXPECT(m3_layout_test_measure_node(&node, spec_unspec, spec),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_LAYOUT_MEASURE_UNSPECIFIED;
  spec.size = 0.0f;
  node.child_count = 1;
  node.children = NULL;
  M3_TEST_EXPECT(m3_layout_test_measure_node(&node, spec_unspec, spec_unspec),
                 M3_ERR_INVALID_ARGUMENT);
  node.child_count = 0;

  spec.mode = M3_LAYOUT_MEASURE_EXACTLY;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_layout_test_measure_leaf(&node, spec, spec_unspec, &size),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_layout_test_measure_leaf(&node, spec_unspec, spec, &size),
                 M3_ERR_RANGE);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.width = M3_LAYOUT_AUTO;
  style.height = M3_LAYOUT_AUTO;
  M3_TEST_OK(m3_layout_node_init(&node, &style));
  measure_ctx.width = 10.0f;
  measure_ctx.height = 20.0f;
  measure_ctx.fail = M3_FALSE;
  measure_ctx.called = 0;
  M3_TEST_OK(
      m3_layout_node_set_measure(&node, m3_layout_measure_cb, &measure_ctx));
  M3_TEST_OK(m3_layout_compute(&node, &direction, spec_unspec, spec_unspec));
  M3_TEST_OK(m3_layout_node_get_layout(&node, &rect));
  M3_TEST_OK(m3_layout_node_get_measured(&node, &size));
  M3_TEST_ASSERT(size.width == 10.0f);
  M3_TEST_ASSERT(size.height == 20.0f);
  M3_TEST_ASSERT(rect.width == 10.0f);
  M3_TEST_ASSERT(rect.height == 20.0f);
  M3_TEST_ASSERT(measure_ctx.called == 1);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.width = 30.0f;
  style.height = M3_LAYOUT_AUTO;
  M3_TEST_OK(m3_layout_node_init(&node, &style));
  measure_ctx.width = 10.0f;
  measure_ctx.height = 20.0f;
  measure_ctx.fail = M3_FALSE;
  measure_ctx.called = 0;
  M3_TEST_OK(
      m3_layout_node_set_measure(&node, m3_layout_measure_cb, &measure_ctx));
  M3_TEST_OK(m3_layout_compute(&node, &direction, spec_unspec, spec_unspec));
  M3_TEST_OK(m3_layout_node_get_layout(&node, &rect));
  M3_TEST_ASSERT(rect.width == 30.0f);
  M3_TEST_ASSERT(rect.height == 20.0f);

  M3_TEST_OK(m3_layout_compute(&node, &direction, spec_exact_50, spec_unspec));
  M3_TEST_OK(m3_layout_node_get_layout(&node, &rect));
  M3_TEST_ASSERT(rect.width == 50.0f);

  M3_TEST_OK(
      m3_layout_compute(&node, &direction, spec_at_most_15, spec_unspec));
  M3_TEST_OK(m3_layout_node_get_layout(&node, &rect));
  M3_TEST_ASSERT(rect.width == 15.0f);

  measure_ctx.fail = M3_TRUE;
  rc = m3_layout_compute(&node, &direction, spec_unspec, spec_unspec);
  M3_TEST_ASSERT(rc == M3_ERR_UNKNOWN);

  measure_ctx.fail = M3_FALSE;
  measure_ctx.width = -1.0f;
  measure_ctx.height = 5.0f;
  M3_TEST_OK(m3_layout_style_init(&style));
  style.width = M3_LAYOUT_AUTO;
  style.height = M3_LAYOUT_AUTO;
  M3_TEST_OK(m3_layout_node_init(&node, &style));
  M3_TEST_OK(
      m3_layout_node_set_measure(&node, m3_layout_measure_cb, &measure_ctx));
  rc = m3_layout_compute(&node, &direction, spec_unspec, spec_unspec);
  M3_TEST_ASSERT(rc == M3_ERR_RANGE);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_START;
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  child1.style.width = 10.0f;
  child1.style.height = 10.0f;
  M3_TEST_OK(m3_layout_node_init(&child2, &style));
  child2.style.width = 20.0f;
  child2.style.height = 10.0f;
  M3_TEST_OK(m3_layout_node_init(&child3, &style));
  child3.style.width = 10.0f;
  child3.style.height = 10.0f;
  children[0] = &child1;
  children[1] = &child2;
  children[2] = &child3;

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.align_cross = M3_LAYOUT_ALIGN_START;

  style.align_main = M3_LAYOUT_ALIGN_START;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 0.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(rect.x == 10.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child3, &rect));
  M3_TEST_ASSERT(rect.x == 30.0f);

  style.align_main = M3_LAYOUT_ALIGN_CENTER;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 10.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(rect.x == 20.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child3, &rect));
  M3_TEST_ASSERT(rect.x == 40.0f);

  style.align_main = M3_LAYOUT_ALIGN_END;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 20.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(rect.x == 30.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child3, &rect));
  M3_TEST_ASSERT(rect.x == 50.0f);

  style.align_main = M3_LAYOUT_ALIGN_SPACE_BETWEEN;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 0.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(rect.x == 20.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child3, &rect));
  M3_TEST_ASSERT(rect.x == 50.0f);

  style.align_main = M3_LAYOUT_ALIGN_SPACE_AROUND;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(m3_layout_near(rect.x, 3.3333333f, 0.01f));
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(m3_layout_near(rect.x, 20.0f, 0.01f));
  M3_TEST_OK(m3_layout_node_get_layout(&child3, &rect));
  M3_TEST_ASSERT(m3_layout_near(rect.x, 46.666667f, 0.02f));

  style.align_main = M3_LAYOUT_ALIGN_SPACE_EVENLY;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(m3_layout_near(rect.x, 5.0f, 0.01f));
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(m3_layout_near(rect.x, 20.0f, 0.01f));
  M3_TEST_OK(m3_layout_node_get_layout(&child3, &rect));
  M3_TEST_ASSERT(m3_layout_near(rect.x, 45.0f, 0.01f));

  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_CENTER;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.y == 15.0f);

  style.align_cross = M3_LAYOUT_ALIGN_END;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.y == 30.0f);

  style.align_cross = M3_LAYOUT_ALIGN_STRETCH;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.height == 40.0f);

  style.align_cross = M3_LAYOUT_ALIGN_START;
  style.padding.left = 5.0f;
  style.padding.top = 2.0f;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 5.0f);
  M3_TEST_ASSERT(rect.y == 2.0f);

  style.padding.left = 0.0f;
  style.padding.top = 0.0f;
  style.wrap = M3_LAYOUT_WRAP_YES;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_30, spec_unspec));
  M3_TEST_OK(m3_layout_node_get_layout(&container, &rect));
  M3_TEST_ASSERT(rect.height == 20.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.y == 0.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child3, &rect));
  M3_TEST_ASSERT(rect.y == 10.0f);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.align_main = M3_LAYOUT_ALIGN_CENTER;
  style.align_cross = M3_LAYOUT_ALIGN_END;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_40, spec_exact_60));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.y == 15.0f);
  M3_TEST_ASSERT(rect.x == 30.0f);

  style.align_cross = M3_LAYOUT_ALIGN_STRETCH;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_40, spec_exact_60));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.width == 40.0f);

  style.wrap = M3_LAYOUT_WRAP_YES;
  style.align_cross = M3_LAYOUT_ALIGN_START;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 3));
  M3_TEST_OK(
      m3_layout_compute(&container, &direction, spec_exact_40, spec_exact_25));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 0.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child3, &rect));
  M3_TEST_ASSERT(rect.x == 20.0f);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_START;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 1));
  children[0]->measured.width = 1.0f;
  children[0]->measured.height = 1.0f;
  container.style.align_main = 99;
  M3_TEST_EXPECT(
      m3_layout_test_layout_children_row(&container, 0.0f, 0.0f, 10.0f, 10.0f),
      M3_ERR_INVALID_ARGUMENT);

  container.style.align_main = M3_LAYOUT_ALIGN_START;
  container.style.align_cross = 99;
  M3_TEST_EXPECT(
      m3_layout_test_layout_children_row(&container, 0.0f, 0.0f, 10.0f, 10.0f),
      M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_START;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 1));
  container.style.align_main = 99;
  M3_TEST_EXPECT(m3_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                       10.0f, 10.0f),
                 M3_ERR_INVALID_ARGUMENT);

  container.style.align_main = M3_LAYOUT_ALIGN_START;
  container.style.align_cross = 99;
  M3_TEST_EXPECT(m3_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                       10.0f, 10.0f),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_START;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 1));
  container.style.direction = 99;
  M3_TEST_EXPECT(
      m3_layout_test_layout_node(&container, 0.0f, 0.0f, 10.0f, 10.0f),
      M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.width = M3_LAYOUT_AUTO;
  style.height = M3_LAYOUT_AUTO;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  M3_TEST_OK(
      m3_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size));

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.width = 20.0f;
  style.height = 20.0f;
  style.padding.left = 15.0f;
  style.padding.right = 15.0f;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  spec.mode = M3_LAYOUT_MEASURE_AT_MOST;
  spec.size = 10.0f;
  M3_TEST_OK(m3_layout_test_measure_row(&container, spec, spec, &size));

  measure_ctx.fail = M3_TRUE;
  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  M3_TEST_OK(
      m3_layout_node_set_measure(&child1, m3_layout_measure_cb, &measure_ctx));
  children[0] = &child1;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 1));
  M3_TEST_EXPECT(
      m3_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size),
      M3_ERR_UNKNOWN);
  measure_ctx.fail = M3_FALSE;

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.wrap = M3_LAYOUT_WRAP_YES;
  style.width = 12.0f;
  style.height = 10.0f;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  measure_ctx.width = 6.0f;
  measure_ctx.height = 4.0f;
  measure_ctx.fail = M3_FALSE;
  M3_TEST_OK(
      m3_layout_node_set_measure(&child1, m3_layout_measure_cb, &measure_ctx));
  children[0] = &child1;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 1));
  M3_TEST_OK(
      m3_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size));

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.width = 33.0f;
  style.height = 44.0f;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  M3_TEST_OK(
      m3_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size));

  spec.mode = M3_LAYOUT_MEASURE_EXACTLY;
  spec.size = -1.0f;
  M3_TEST_EXPECT(
      m3_layout_test_measure_row(&container, spec, spec_unspec, &size),
      M3_ERR_RANGE);
  M3_TEST_EXPECT(
      m3_layout_test_measure_row(&container, spec_unspec, spec, &size),
      M3_ERR_RANGE);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.width = M3_LAYOUT_AUTO;
  style.height = M3_LAYOUT_AUTO;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  M3_TEST_OK(m3_layout_test_measure_column(&container, spec_unspec, spec_unspec,
                                           &size));

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.width = 20.0f;
  style.height = 20.0f;
  style.padding.top = 15.0f;
  style.padding.bottom = 15.0f;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  spec.mode = M3_LAYOUT_MEASURE_AT_MOST;
  spec.size = 10.0f;
  M3_TEST_OK(m3_layout_test_measure_column(&container, spec, spec, &size));

  measure_ctx.fail = M3_TRUE;
  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  M3_TEST_OK(
      m3_layout_node_set_measure(&child1, m3_layout_measure_cb, &measure_ctx));
  children[0] = &child1;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 1));
  M3_TEST_EXPECT(m3_layout_test_measure_column(&container, spec_unspec,
                                               spec_unspec, &size),
                 M3_ERR_UNKNOWN);
  measure_ctx.fail = M3_FALSE;

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  style.wrap = M3_LAYOUT_WRAP_YES;
  style.width = 10.0f;
  style.height = 12.0f;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  measure_ctx.width = 4.0f;
  measure_ctx.height = 6.0f;
  measure_ctx.fail = M3_FALSE;
  M3_TEST_OK(
      m3_layout_node_set_measure(&child1, m3_layout_measure_cb, &measure_ctx));
  children[0] = &child1;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 1));
  M3_TEST_OK(m3_layout_test_measure_column(&container, spec_unspec, spec_unspec,
                                           &size));

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  style.width = 55.0f;
  style.height = 66.0f;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  M3_TEST_OK(m3_layout_test_measure_column(&container, spec_unspec, spec_unspec,
                                           &size));

  spec.mode = M3_LAYOUT_MEASURE_EXACTLY;
  spec.size = -1.0f;
  M3_TEST_EXPECT(
      m3_layout_test_measure_column(&container, spec, spec_unspec, &size),
      M3_ERR_RANGE);
  M3_TEST_EXPECT(
      m3_layout_test_measure_column(&container, spec_unspec, spec, &size),
      M3_ERR_RANGE);

  M3_TEST_OK(m3_layout_style_init(&style));
  M3_TEST_OK(m3_layout_node_init(&node, &style));
  node.style.direction = 99;
  M3_TEST_EXPECT(m3_layout_test_measure_node(&node, spec_unspec, spec_unspec),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_START;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  M3_TEST_OK(m3_layout_node_init(&child2, &style));
  child1.measured.width = 8.0f;
  child1.measured.height = 2.0f;
  child2.measured.width = 8.0f;
  child2.measured.height = 3.0f;
  children[0] = &child1;
  children[1] = &child2;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));
  M3_TEST_OK(
      m3_layout_test_layout_children_row(&container, 0.0f, 0.0f, 10.0f, 5.0f));

  M3_TEST_OK(m3_layout_node_init(&child3, &style));
  child3.measured.width = 4.0f;
  child3.measured.height = 4.0f;
  child3.child_count = 1;
  child3.children = NULL;
  children[0] = &child3;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 1));
  M3_TEST_EXPECT(
      m3_layout_test_layout_children_row(&container, 0.0f, 0.0f, 10.0f, 5.0f),
      M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_START;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  M3_TEST_OK(m3_layout_node_init(&child2, &style));
  child1.measured.width = 2.0f;
  child1.measured.height = 8.0f;
  child2.measured.width = 3.0f;
  child2.measured.height = 8.0f;
  children[0] = &child1;
  children[1] = &child2;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));
  M3_TEST_OK(m3_layout_test_layout_children_column(&container, 0.0f, 0.0f, 5.0f,
                                                   10.0f));

  style.align_main = M3_LAYOUT_ALIGN_END;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));
  M3_TEST_OK(m3_layout_test_layout_children_column(&container, 0.0f, 0.0f, 5.0f,
                                                   10.0f));

  style.align_main = M3_LAYOUT_ALIGN_SPACE_BETWEEN;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));
  M3_TEST_OK(m3_layout_test_layout_children_column(&container, 0.0f, 0.0f, 5.0f,
                                                   10.0f));

  style.align_main = M3_LAYOUT_ALIGN_SPACE_AROUND;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));
  M3_TEST_OK(m3_layout_test_layout_children_column(&container, 0.0f, 0.0f, 5.0f,
                                                   10.0f));

  style.align_main = M3_LAYOUT_ALIGN_SPACE_EVENLY;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));
  M3_TEST_OK(m3_layout_test_layout_children_column(&container, 0.0f, 0.0f, 5.0f,
                                                   10.0f));

  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_CENTER;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));
  M3_TEST_OK(m3_layout_test_layout_children_column(&container, 0.0f, 0.0f, 5.0f,
                                                   10.0f));

  M3_TEST_OK(m3_layout_node_init(&child3, &style));
  child3.measured.width = 2.0f;
  child3.measured.height = 2.0f;
  child3.child_count = 1;
  child3.children = NULL;
  children[0] = &child3;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 1));
  M3_TEST_EXPECT(m3_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                       5.0f, 10.0f),
                 M3_ERR_INVALID_ARGUMENT);

  mutate_ctx.target = &container;
  mutate_ctx.called = 0;
  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_START;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  M3_TEST_OK(m3_layout_node_set_measure(&child1, m3_layout_measure_mutate,
                                        &mutate_ctx));
  children[0] = &child1;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 1));
  M3_TEST_EXPECT(
      m3_layout_compute(&container, &direction, spec_unspec, spec_unspec),
      M3_ERR_INVALID_ARGUMENT);

  container.child_count = 1;
  container.children = NULL;
  M3_TEST_EXPECT(
      m3_layout_test_layout_node(&container, 0.0f, 0.0f, 10.0f, 10.0f),
      M3_ERR_INVALID_ARGUMENT);

  node.child_count = 1;
  node.children = NULL;
  M3_TEST_EXPECT(m3_layout_test_measure_node(&node, spec_unspec, spec_unspec),
                 M3_ERR_INVALID_ARGUMENT);

  spec.mode = M3_LAYOUT_MEASURE_EXACTLY;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_layout_test_measure_node(&node, spec, spec_unspec),
                 M3_ERR_RANGE);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.width = M3_LAYOUT_AUTO;
  style.height = M3_LAYOUT_AUTO;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  M3_TEST_OK(
      m3_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size));

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.wrap = M3_LAYOUT_WRAP_YES;
  style.width = 5.0f;
  style.height = M3_LAYOUT_AUTO;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  M3_TEST_OK(m3_layout_node_init(&child2, &style));
  child1.style.width = 2.0f;
  child1.style.height = 1.0f;
  child2.style.width = 6.0f;
  child2.style.height = 1.0f;
  children[0] = &child1;
  children[1] = &child2;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));
  M3_TEST_OK(
      m3_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size));

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.width = 100.0f;
  style.height = 80.0f;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  spec.mode = M3_LAYOUT_MEASURE_EXACTLY;
  spec.size = 50.0f;
  M3_TEST_OK(m3_layout_test_measure_column(&container, spec, spec, &size));

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  style.wrap = M3_LAYOUT_WRAP_YES;
  style.width = M3_LAYOUT_AUTO;
  style.height = 5.0f;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  M3_TEST_OK(m3_layout_node_init(&child2, &style));
  child1.style.width = 1.0f;
  child1.style.height = 2.0f;
  child2.style.width = 1.0f;
  child2.style.height = 6.0f;
  children[0] = &child1;
  children[1] = &child2;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));
  M3_TEST_OK(m3_layout_test_measure_column(&container, spec_unspec, spec_unspec,
                                           &size));

  direction.flow = 99u;
  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.wrap = M3_LAYOUT_WRAP_NO;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_EXPECT(m3_layout_test_measure_row_with_direction(
                     &container, &direction, spec_unspec, spec_unspec, &size),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_test_measure_node_with_direction(
                     &container, &direction, spec_unspec, spec_unspec),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_test_layout_node_with_direction(
                     &container, &direction, 0.0f, 0.0f, 1.0f, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_test_layout_children_row_with_direction(
                     NULL, &direction, 0.0f, 0.0f, 1.0f, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_test_layout_children_row_with_direction(
                     &container, &direction, 0.0f, 0.0f, 1.0f, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_EXPECT(m3_layout_test_measure_column_with_direction(
                     &container, &direction, spec_unspec, spec_unspec, &size),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_test_layout_children_column_with_direction(
                     NULL, &direction, 0.0f, 0.0f, 1.0f, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_test_layout_children_column_with_direction(
                     &container, &direction, 0.0f, 0.0f, 1.0f, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  direction.flow = M3_DIRECTION_LTR;

  M3_TEST_OK(m3_layout_test_set_direction_fail(M3_TRUE));
  M3_TEST_EXPECT(
      m3_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size),
      M3_ERR_UNKNOWN);
  M3_TEST_EXPECT(m3_layout_test_measure_column(&container, spec_unspec,
                                               spec_unspec, &size),
                 M3_ERR_UNKNOWN);
  M3_TEST_EXPECT(
      m3_layout_test_measure_node(&container, spec_unspec, spec_unspec),
      M3_ERR_UNKNOWN);
  M3_TEST_EXPECT(m3_layout_test_layout_node(&container, 0.0f, 0.0f, 1.0f, 1.0f),
                 M3_ERR_UNKNOWN);
  M3_TEST_EXPECT(
      m3_layout_test_layout_children_row(&container, 0.0f, 0.0f, 1.0f, 1.0f),
      M3_ERR_UNKNOWN);
  M3_TEST_EXPECT(
      m3_layout_test_layout_children_column(&container, 0.0f, 0.0f, 1.0f, 1.0f),
      M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_layout_test_set_direction_fail(M3_FALSE));

  return 0;
}
