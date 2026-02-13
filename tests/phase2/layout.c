#include "cmpc/cmp_layout.h"
#include "test_utils.h"

typedef struct MeasureCtx {
  CMPScalar width;
  CMPScalar height;
  CMPBool fail;
  int called;
} MeasureCtx;

typedef struct MutateCtx {
  CMPLayoutNode *target;
  int called;
} MutateCtx;

static int cmp_layout_near(CMPScalar a, CMPScalar b, CMPScalar tol) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }

  return (diff <= tol) ? 1 : 0;
}

static int cmp_layout_measure_cb(void *ctx, CMPLayoutMeasureSpec width,
                                 CMPLayoutMeasureSpec height,
                                 CMPSize *out_size) {
  MeasureCtx *measure;

  if (ctx == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  measure = (MeasureCtx *)ctx;
  measure->called += 1;

  if (measure->fail) {
    return CMP_ERR_UNKNOWN;
  }

  if (width.mode == CMP_LAYOUT_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else {
    out_size->width = measure->width;
  }

  if (height.mode == CMP_LAYOUT_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else {
    out_size->height = measure->height;
  }

  return CMP_OK;
}

static int cmp_layout_measure_mutate(void *ctx, CMPLayoutMeasureSpec width,
                                     CMPLayoutMeasureSpec height,
                                     CMPSize *out_size) {
  MutateCtx *state;

  CMP_UNUSED(width);
  CMP_UNUSED(height);

  if (ctx == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (MutateCtx *)ctx;
  state->called += 1;
  if (state->target != NULL) {
    state->target->style.align_main = 99;
  }

  out_size->width = 1.0f;
  out_size->height = 1.0f;
  return CMP_OK;
}

int main(void) {
  CMPLayoutMeasureSpec spec;
  CMPLayoutMeasureSpec spec_unspec;
  CMPLayoutMeasureSpec spec_exact_60;
  CMPLayoutMeasureSpec spec_exact_40;
  CMPLayoutMeasureSpec spec_exact_25;
  CMPLayoutMeasureSpec spec_exact_30;
  CMPLayoutMeasureSpec spec_exact_50;
  CMPLayoutMeasureSpec spec_at_most_15;
  CMPLayoutDirection direction;
  CMPLayoutDirection direction_rtl;
  CMPLayoutStyle style;
  CMPLayoutNode node;
  CMPLayoutNode child1;
  CMPLayoutNode child2;
  CMPLayoutNode child3;
  CMPLayoutNode *children[3];
  CMPLayoutNode container;
  CMPLayoutEdges edges;
  CMPRect rect;
  CMPSize size;
  CMPScalar resolved;
  CMPScalar clamped;
  CMPScalar padding_left;
  CMPScalar padding_right;
  MeasureCtx measure_ctx;
  MutateCtx mutate_ctx;
  CMPScalar out_size;
  int rc;

  CMP_TEST_EXPECT(
      cmp_layout_measure_spec_init(NULL, CMP_LAYOUT_MEASURE_UNSPECIFIED, 0.0f),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_measure_spec_init(&spec, 99, 0.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_layout_measure_spec_init(&spec, CMP_LAYOUT_MEASURE_EXACTLY, -1.0f),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_layout_measure_spec_init(
      &spec_unspec, CMP_LAYOUT_MEASURE_UNSPECIFIED, 0.0f));
  CMP_TEST_OK(cmp_layout_measure_spec_init(&spec_exact_60,
                                           CMP_LAYOUT_MEASURE_EXACTLY, 60.0f));
  CMP_TEST_OK(cmp_layout_measure_spec_init(&spec_exact_40,
                                           CMP_LAYOUT_MEASURE_EXACTLY, 40.0f));
  CMP_TEST_OK(cmp_layout_measure_spec_init(&spec_exact_25,
                                           CMP_LAYOUT_MEASURE_EXACTLY, 25.0f));
  CMP_TEST_OK(cmp_layout_measure_spec_init(&spec_exact_30,
                                           CMP_LAYOUT_MEASURE_EXACTLY, 30.0f));
  CMP_TEST_OK(cmp_layout_measure_spec_init(&spec_exact_50,
                                           CMP_LAYOUT_MEASURE_EXACTLY, 50.0f));
  CMP_TEST_OK(cmp_layout_measure_spec_init(&spec_at_most_15,
                                           CMP_LAYOUT_MEASURE_AT_MOST, 15.0f));
  CMP_TEST_OK(cmp_layout_direction_init(&direction, CMP_DIRECTION_LTR));
  CMP_TEST_OK(cmp_layout_direction_init(&direction_rtl, CMP_DIRECTION_RTL));
  CMP_TEST_EXPECT(cmp_layout_test_validate_direction(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  direction.flow = 99u;
  CMP_TEST_EXPECT(cmp_layout_test_validate_direction(&direction),
                  CMP_ERR_INVALID_ARGUMENT);
  direction.flow = CMP_DIRECTION_LTR;
  CMP_TEST_OK(cmp_layout_test_validate_direction(&direction));
  CMP_TEST_EXPECT(cmp_layout_test_clamp_non_negative(-1.0f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_layout_test_clamp_non_negative(-1.0f, &clamped));
  CMP_TEST_ASSERT(clamped == 0.0f);
  CMP_TEST_OK(cmp_layout_test_clamp_non_negative(2.0f, &clamped));
  CMP_TEST_ASSERT(clamped == 2.0f);
  CMP_TEST_EXPECT(
      cmp_layout_test_resolve_available(CMP_LAYOUT_AUTO, spec_unspec, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_layout_test_resolve_available(CMP_LAYOUT_AUTO, spec_unspec,
                                                &resolved));
  CMP_TEST_ASSERT(resolved == -1.0f);
  CMP_TEST_OK(cmp_layout_test_resolve_available(10.0f, spec_unspec, &resolved));
  CMP_TEST_ASSERT(resolved == 10.0f);
  CMP_TEST_OK(cmp_layout_test_resolve_available(CMP_LAYOUT_AUTO, spec_exact_50,
                                                &resolved));
  CMP_TEST_ASSERT(resolved == 50.0f);
  CMP_TEST_OK(
      cmp_layout_test_resolve_available(10.0f, spec_at_most_15, &resolved));
  CMP_TEST_ASSERT(resolved == 10.0f);
  CMP_TEST_OK(
      cmp_layout_test_resolve_available(20.0f, spec_at_most_15, &resolved));
  CMP_TEST_ASSERT(resolved == 15.0f);

  CMP_TEST_EXPECT(cmp_layout_edges_set(NULL, 0.0f, 0.0f, 0.0f, 0.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_edges_set(&edges, -1.0f, 0.0f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_layout_edges_set(&edges, 1.0f, 2.0f, 3.0f, 4.0f));
  CMP_TEST_EXPECT(cmp_layout_test_resolve_horizontal_padding(
                      NULL, &edges, &padding_left, &padding_right),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_test_resolve_horizontal_padding(
                      &direction, NULL, &padding_left, &padding_right),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_test_resolve_horizontal_padding(
                      &direction, &edges, NULL, &padding_right),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_test_resolve_horizontal_padding(
                      &direction, &edges, &padding_left, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_layout_test_resolve_horizontal_padding(
      &direction, &edges, &padding_left, &padding_right));
  CMP_TEST_ASSERT(padding_left == 1.0f);
  CMP_TEST_ASSERT(padding_right == 3.0f);
  CMP_TEST_OK(cmp_layout_test_resolve_horizontal_padding(
      &direction_rtl, &edges, &padding_left, &padding_right));
  CMP_TEST_ASSERT(padding_left == 3.0f);
  CMP_TEST_ASSERT(padding_right == 1.0f);

  CMP_TEST_EXPECT(cmp_layout_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_layout_style_init(&style));
  CMP_TEST_ASSERT(style.direction == CMP_LAYOUT_DIRECTION_ROW);

  CMP_TEST_EXPECT(cmp_layout_node_init(NULL, &style), CMP_ERR_INVALID_ARGUMENT);

  style.direction = 99;
  CMP_TEST_EXPECT(cmp_layout_node_init(&node, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.wrap = 99;
  CMP_TEST_EXPECT(cmp_layout_node_init(&node, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.align_main = CMP_LAYOUT_ALIGN_STRETCH;
  CMP_TEST_EXPECT(cmp_layout_node_init(&node, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_SPACE_BETWEEN;
  CMP_TEST_EXPECT(cmp_layout_node_init(&node, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  style.align_cross = CMP_LAYOUT_ALIGN_START;
  style.padding.left = -1.0f;
  CMP_TEST_EXPECT(cmp_layout_node_init(&node, &style), CMP_ERR_RANGE);
  style.padding.left = 0.0f;
  style.width = -2.0f;
  CMP_TEST_EXPECT(cmp_layout_node_init(&node, &style), CMP_ERR_RANGE);
  style.width = CMP_LAYOUT_AUTO;
  style.height = -3.0f;
  CMP_TEST_EXPECT(cmp_layout_node_init(&node, &style), CMP_ERR_RANGE);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  CMP_TEST_OK(cmp_layout_node_init(&node, &style));
  CMP_TEST_OK(cmp_layout_node_init(&node, NULL));
#ifdef CMP_TESTING
  CMP_TEST_OK(cmp_layout_test_set_style_init_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_layout_node_init(&node, NULL), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_layout_test_set_style_init_fail(CMP_FALSE));
#endif
  CMP_TEST_EXPECT(cmp_layout_node_set_children(NULL, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_node_set_children(&node, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  children[0] = NULL;
  CMP_TEST_EXPECT(cmp_layout_node_set_children(&node, children, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_layout_node_set_children(&node, NULL, 0));
  CMP_TEST_OK(cmp_layout_node_set_measure(&node, NULL, NULL));
  CMP_TEST_EXPECT(cmp_layout_node_set_measure(NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_node_get_measured(NULL, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_node_get_measured(&node, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_node_get_layout(NULL, &rect),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_node_get_layout(&node, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_layout_compute(NULL, &direction, spec_unspec, spec_unspec),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_compute(&node, NULL, spec_unspec, spec_unspec),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_layout_test_validate_measure_spec(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = 99;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(cmp_layout_test_validate_measure_spec(&spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_LAYOUT_MEASURE_AT_MOST;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(cmp_layout_test_validate_measure_spec(&spec), CMP_ERR_RANGE);
  spec.mode = CMP_LAYOUT_MEASURE_UNSPECIFIED;
  spec.size = 0.0f;
  CMP_TEST_OK(cmp_layout_test_validate_measure_spec(&spec));

  CMP_TEST_EXPECT(cmp_layout_test_validate_style(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_layout_style_init(&style));
  CMP_TEST_OK(cmp_layout_test_validate_style(&style));

  CMP_TEST_EXPECT(cmp_layout_test_apply_spec(1.0f, spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_LAYOUT_MEASURE_EXACTLY;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(cmp_layout_test_apply_spec(1.0f, spec, &out_size),
                  CMP_ERR_RANGE);
  spec.mode = CMP_LAYOUT_MEASURE_AT_MOST;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(cmp_layout_test_apply_spec(1.0f, spec, &out_size),
                  CMP_ERR_RANGE);
  spec.mode = 99;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(cmp_layout_test_apply_spec(1.0f, spec, &out_size),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_LAYOUT_MEASURE_EXACTLY;
  spec.size = 5.0f;
  CMP_TEST_OK(cmp_layout_test_apply_spec(1.0f, spec, &out_size));
  CMP_TEST_ASSERT(out_size == 5.0f);
  spec.mode = CMP_LAYOUT_MEASURE_AT_MOST;
  spec.size = 0.5f;
  CMP_TEST_OK(cmp_layout_test_apply_spec(1.0f, spec, &out_size));
  CMP_TEST_ASSERT(out_size == 0.5f);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  CMP_TEST_OK(cmp_layout_node_init(&node, &style));
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_leaf(NULL, spec_unspec, spec_unspec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_leaf(&node, spec_unspec, spec_unspec, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_row(NULL, spec_unspec, spec_unspec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_row(&node, spec_unspec, spec_unspec, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_column(NULL, spec_unspec, spec_unspec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_column(&node, spec_unspec, spec_unspec, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_test_measure_node(NULL, spec_unspec, spec_unspec),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_test_layout_node(NULL, 0.0f, 0.0f, 1.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);

  spec.mode = 99;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(cmp_layout_test_measure_node(&node, spec_unspec, spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_LAYOUT_MEASURE_UNSPECIFIED;
  spec.size = 0.0f;
  node.child_count = 1;
  node.children = NULL;
  CMP_TEST_EXPECT(cmp_layout_test_measure_node(&node, spec_unspec, spec_unspec),
                  CMP_ERR_INVALID_ARGUMENT);
  node.child_count = 0;

  spec.mode = CMP_LAYOUT_MEASURE_EXACTLY;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(cmp_layout_test_measure_leaf(&node, spec, spec_unspec, &size),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_layout_test_measure_leaf(&node, spec_unspec, spec, &size),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.width = CMP_LAYOUT_AUTO;
  style.height = CMP_LAYOUT_AUTO;
  CMP_TEST_OK(cmp_layout_node_init(&node, &style));
  measure_ctx.width = 10.0f;
  measure_ctx.height = 20.0f;
  measure_ctx.fail = CMP_FALSE;
  measure_ctx.called = 0;
  CMP_TEST_OK(
      cmp_layout_node_set_measure(&node, cmp_layout_measure_cb, &measure_ctx));
  CMP_TEST_OK(cmp_layout_compute(&node, &direction, spec_unspec, spec_unspec));
  CMP_TEST_OK(cmp_layout_node_get_layout(&node, &rect));
  CMP_TEST_OK(cmp_layout_node_get_measured(&node, &size));
  CMP_TEST_ASSERT(size.width == 10.0f);
  CMP_TEST_ASSERT(size.height == 20.0f);
  CMP_TEST_ASSERT(rect.width == 10.0f);
  CMP_TEST_ASSERT(rect.height == 20.0f);
  CMP_TEST_ASSERT(measure_ctx.called == 1);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.width = 30.0f;
  style.height = CMP_LAYOUT_AUTO;
  CMP_TEST_OK(cmp_layout_node_init(&node, &style));
  measure_ctx.width = 10.0f;
  measure_ctx.height = 20.0f;
  measure_ctx.fail = CMP_FALSE;
  measure_ctx.called = 0;
  CMP_TEST_OK(
      cmp_layout_node_set_measure(&node, cmp_layout_measure_cb, &measure_ctx));
  CMP_TEST_OK(cmp_layout_compute(&node, &direction, spec_unspec, spec_unspec));
  CMP_TEST_OK(cmp_layout_node_get_layout(&node, &rect));
  CMP_TEST_ASSERT(rect.width == 30.0f);
  CMP_TEST_ASSERT(rect.height == 20.0f);

  CMP_TEST_OK(
      cmp_layout_compute(&node, &direction, spec_exact_50, spec_unspec));
  CMP_TEST_OK(cmp_layout_node_get_layout(&node, &rect));
  CMP_TEST_ASSERT(rect.width == 50.0f);

  CMP_TEST_OK(
      cmp_layout_compute(&node, &direction, spec_at_most_15, spec_unspec));
  CMP_TEST_OK(cmp_layout_node_get_layout(&node, &rect));
  CMP_TEST_ASSERT(rect.width == 15.0f);

  measure_ctx.fail = CMP_TRUE;
  rc = cmp_layout_compute(&node, &direction, spec_unspec, spec_unspec);
  CMP_TEST_ASSERT(rc == CMP_ERR_UNKNOWN);

  measure_ctx.fail = CMP_FALSE;
  measure_ctx.width = -1.0f;
  measure_ctx.height = 5.0f;
  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.width = CMP_LAYOUT_AUTO;
  style.height = CMP_LAYOUT_AUTO;
  CMP_TEST_OK(cmp_layout_node_init(&node, &style));
  CMP_TEST_OK(
      cmp_layout_node_set_measure(&node, cmp_layout_measure_cb, &measure_ctx));
  rc = cmp_layout_compute(&node, &direction, spec_unspec, spec_unspec);
  CMP_TEST_ASSERT(rc == CMP_ERR_RANGE);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_START;
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  child1.style.width = 10.0f;
  child1.style.height = 10.0f;
  CMP_TEST_OK(cmp_layout_node_init(&child2, &style));
  child2.style.width = 20.0f;
  child2.style.height = 10.0f;
  CMP_TEST_OK(cmp_layout_node_init(&child3, &style));
  child3.style.width = 10.0f;
  child3.style.height = 10.0f;
  children[0] = &child1;
  children[1] = &child2;
  children[2] = &child3;

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.align_cross = CMP_LAYOUT_ALIGN_START;

  style.align_main = CMP_LAYOUT_ALIGN_START;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 0.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(rect.x == 10.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child3, &rect));
  CMP_TEST_ASSERT(rect.x == 30.0f);

  style.align_main = CMP_LAYOUT_ALIGN_CENTER;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 10.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(rect.x == 20.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child3, &rect));
  CMP_TEST_ASSERT(rect.x == 40.0f);

  style.align_main = CMP_LAYOUT_ALIGN_END;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 20.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(rect.x == 30.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child3, &rect));
  CMP_TEST_ASSERT(rect.x == 50.0f);

  style.align_main = CMP_LAYOUT_ALIGN_SPACE_BETWEEN;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 0.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(rect.x == 20.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child3, &rect));
  CMP_TEST_ASSERT(rect.x == 50.0f);

  style.align_main = CMP_LAYOUT_ALIGN_SPACE_AROUND;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(cmp_layout_near(rect.x, 3.3333333f, 0.01f));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(cmp_layout_near(rect.x, 20.0f, 0.01f));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child3, &rect));
  CMP_TEST_ASSERT(cmp_layout_near(rect.x, 46.666667f, 0.02f));

  style.align_main = CMP_LAYOUT_ALIGN_SPACE_EVENLY;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(cmp_layout_near(rect.x, 5.0f, 0.01f));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(cmp_layout_near(rect.x, 20.0f, 0.01f));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child3, &rect));
  CMP_TEST_ASSERT(cmp_layout_near(rect.x, 45.0f, 0.01f));

  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_CENTER;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.y == 15.0f);

  style.align_cross = CMP_LAYOUT_ALIGN_END;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.y == 30.0f);

  style.align_cross = CMP_LAYOUT_ALIGN_STRETCH;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.height == 40.0f);

  style.align_cross = CMP_LAYOUT_ALIGN_START;
  style.padding.left = 5.0f;
  style.padding.top = 2.0f;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_60, spec_exact_40));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 5.0f);
  CMP_TEST_ASSERT(rect.y == 2.0f);

  style.padding.left = 0.0f;
  style.padding.top = 0.0f;
  style.wrap = CMP_LAYOUT_WRAP_YES;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_30, spec_unspec));
  CMP_TEST_OK(cmp_layout_node_get_layout(&container, &rect));
  CMP_TEST_ASSERT(rect.height == 20.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.y == 0.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child3, &rect));
  CMP_TEST_ASSERT(rect.y == 10.0f);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.align_main = CMP_LAYOUT_ALIGN_CENTER;
  style.align_cross = CMP_LAYOUT_ALIGN_END;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_40, spec_exact_60));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.y == 15.0f);
  CMP_TEST_ASSERT(rect.x == 30.0f);

  style.align_cross = CMP_LAYOUT_ALIGN_STRETCH;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_40, spec_exact_60));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.width == 40.0f);

  style.wrap = CMP_LAYOUT_WRAP_YES;
  style.align_cross = CMP_LAYOUT_ALIGN_START;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 3));
  CMP_TEST_OK(
      cmp_layout_compute(&container, &direction, spec_exact_40, spec_exact_25));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 0.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child3, &rect));
  CMP_TEST_ASSERT(rect.x == 20.0f);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_START;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 1));
  children[0]->measured.width = 1.0f;
  children[0]->measured.height = 1.0f;
  container.style.align_main = 99;
  CMP_TEST_EXPECT(
      cmp_layout_test_layout_children_row(&container, 0.0f, 0.0f, 10.0f, 10.0f),
      CMP_ERR_INVALID_ARGUMENT);

  container.style.align_main = CMP_LAYOUT_ALIGN_START;
  container.style.align_cross = 99;
  CMP_TEST_EXPECT(
      cmp_layout_test_layout_children_row(&container, 0.0f, 0.0f, 10.0f, 10.0f),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_START;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 1));
  container.style.align_main = 99;
  CMP_TEST_EXPECT(cmp_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                         10.0f, 10.0f),
                  CMP_ERR_INVALID_ARGUMENT);

  container.style.align_main = CMP_LAYOUT_ALIGN_START;
  container.style.align_cross = 99;
  CMP_TEST_EXPECT(cmp_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                         10.0f, 10.0f),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_START;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 1));
  container.style.direction = 99;
  CMP_TEST_EXPECT(
      cmp_layout_test_layout_node(&container, 0.0f, 0.0f, 10.0f, 10.0f),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.width = CMP_LAYOUT_AUTO;
  style.height = CMP_LAYOUT_AUTO;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  CMP_TEST_OK(
      cmp_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size));

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.width = 20.0f;
  style.height = 20.0f;
  style.padding.left = 15.0f;
  style.padding.right = 15.0f;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  spec.mode = CMP_LAYOUT_MEASURE_AT_MOST;
  spec.size = 10.0f;
  CMP_TEST_OK(cmp_layout_test_measure_row(&container, spec, spec, &size));

  measure_ctx.fail = CMP_TRUE;
  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  CMP_TEST_OK(cmp_layout_node_set_measure(&child1, cmp_layout_measure_cb,
                                          &measure_ctx));
  children[0] = &child1;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 1));
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size),
      CMP_ERR_UNKNOWN);
  measure_ctx.fail = CMP_FALSE;

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.wrap = CMP_LAYOUT_WRAP_YES;
  style.width = 12.0f;
  style.height = 10.0f;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  measure_ctx.width = 6.0f;
  measure_ctx.height = 4.0f;
  measure_ctx.fail = CMP_FALSE;
  CMP_TEST_OK(cmp_layout_node_set_measure(&child1, cmp_layout_measure_cb,
                                          &measure_ctx));
  children[0] = &child1;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 1));
  CMP_TEST_OK(
      cmp_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size));

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.width = 33.0f;
  style.height = 44.0f;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  CMP_TEST_OK(
      cmp_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size));

  spec.mode = CMP_LAYOUT_MEASURE_EXACTLY;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_row(&container, spec, spec_unspec, &size),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_row(&container, spec_unspec, spec, &size),
      CMP_ERR_RANGE);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.width = CMP_LAYOUT_AUTO;
  style.height = CMP_LAYOUT_AUTO;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  CMP_TEST_OK(cmp_layout_test_measure_column(&container, spec_unspec,
                                             spec_unspec, &size));

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.width = 20.0f;
  style.height = 20.0f;
  style.padding.top = 15.0f;
  style.padding.bottom = 15.0f;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  spec.mode = CMP_LAYOUT_MEASURE_AT_MOST;
  spec.size = 10.0f;
  CMP_TEST_OK(cmp_layout_test_measure_column(&container, spec, spec, &size));

  measure_ctx.fail = CMP_TRUE;
  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  CMP_TEST_OK(cmp_layout_node_set_measure(&child1, cmp_layout_measure_cb,
                                          &measure_ctx));
  children[0] = &child1;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 1));
  CMP_TEST_EXPECT(cmp_layout_test_measure_column(&container, spec_unspec,
                                                 spec_unspec, &size),
                  CMP_ERR_UNKNOWN);
  measure_ctx.fail = CMP_FALSE;

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  style.wrap = CMP_LAYOUT_WRAP_YES;
  style.width = 10.0f;
  style.height = 12.0f;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  measure_ctx.width = 4.0f;
  measure_ctx.height = 6.0f;
  measure_ctx.fail = CMP_FALSE;
  CMP_TEST_OK(cmp_layout_node_set_measure(&child1, cmp_layout_measure_cb,
                                          &measure_ctx));
  children[0] = &child1;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 1));
  CMP_TEST_OK(cmp_layout_test_measure_column(&container, spec_unspec,
                                             spec_unspec, &size));

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  style.width = 55.0f;
  style.height = 66.0f;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  CMP_TEST_OK(cmp_layout_test_measure_column(&container, spec_unspec,
                                             spec_unspec, &size));

  spec.mode = CMP_LAYOUT_MEASURE_EXACTLY;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_column(&container, spec, spec_unspec, &size),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_column(&container, spec_unspec, spec, &size),
      CMP_ERR_RANGE);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  CMP_TEST_OK(cmp_layout_node_init(&node, &style));
  node.style.direction = 99;
  CMP_TEST_EXPECT(cmp_layout_test_measure_node(&node, spec_unspec, spec_unspec),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_START;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child2, &style));
  child1.measured.width = 8.0f;
  child1.measured.height = 2.0f;
  child2.measured.width = 8.0f;
  child2.measured.height = 3.0f;
  children[0] = &child1;
  children[1] = &child2;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));
  CMP_TEST_OK(
      cmp_layout_test_layout_children_row(&container, 0.0f, 0.0f, 10.0f, 5.0f));

  CMP_TEST_OK(cmp_layout_node_init(&child3, &style));
  child3.measured.width = 4.0f;
  child3.measured.height = 4.0f;
  child3.child_count = 1;
  child3.children = NULL;
  children[0] = &child3;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 1));
  CMP_TEST_EXPECT(
      cmp_layout_test_layout_children_row(&container, 0.0f, 0.0f, 10.0f, 5.0f),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_START;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child2, &style));
  child1.measured.width = 2.0f;
  child1.measured.height = 8.0f;
  child2.measured.width = 3.0f;
  child2.measured.height = 8.0f;
  children[0] = &child1;
  children[1] = &child2;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));
  CMP_TEST_OK(cmp_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                     5.0f, 10.0f));

  style.align_main = CMP_LAYOUT_ALIGN_END;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));
  CMP_TEST_OK(cmp_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                     5.0f, 10.0f));

  style.align_main = CMP_LAYOUT_ALIGN_SPACE_BETWEEN;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));
  CMP_TEST_OK(cmp_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                     5.0f, 10.0f));

  style.align_main = CMP_LAYOUT_ALIGN_SPACE_AROUND;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));
  CMP_TEST_OK(cmp_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                     5.0f, 10.0f));

  style.align_main = CMP_LAYOUT_ALIGN_SPACE_EVENLY;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));
  CMP_TEST_OK(cmp_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                     5.0f, 10.0f));

  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_CENTER;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));
  CMP_TEST_OK(cmp_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                     5.0f, 10.0f));

  CMP_TEST_OK(cmp_layout_node_init(&child3, &style));
  child3.measured.width = 2.0f;
  child3.measured.height = 2.0f;
  child3.child_count = 1;
  child3.children = NULL;
  children[0] = &child3;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 1));
  CMP_TEST_EXPECT(cmp_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                         5.0f, 10.0f),
                  CMP_ERR_INVALID_ARGUMENT);

  mutate_ctx.target = &container;
  mutate_ctx.called = 0;
  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_START;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  CMP_TEST_OK(cmp_layout_node_set_measure(&child1, cmp_layout_measure_mutate,
                                          &mutate_ctx));
  children[0] = &child1;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 1));
  CMP_TEST_EXPECT(
      cmp_layout_compute(&container, &direction, spec_unspec, spec_unspec),
      CMP_ERR_INVALID_ARGUMENT);

  container.child_count = 1;
  container.children = NULL;
  CMP_TEST_EXPECT(
      cmp_layout_test_layout_node(&container, 0.0f, 0.0f, 10.0f, 10.0f),
      CMP_ERR_INVALID_ARGUMENT);

  node.child_count = 1;
  node.children = NULL;
  CMP_TEST_EXPECT(cmp_layout_test_measure_node(&node, spec_unspec, spec_unspec),
                  CMP_ERR_INVALID_ARGUMENT);

  spec.mode = CMP_LAYOUT_MEASURE_EXACTLY;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(cmp_layout_test_measure_node(&node, spec, spec_unspec),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.width = CMP_LAYOUT_AUTO;
  style.height = CMP_LAYOUT_AUTO;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  container.child_count = 0;
  container.children = NULL;
  CMP_TEST_OK(
      cmp_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size));

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.wrap = CMP_LAYOUT_WRAP_YES;
  style.width = 5.0f;
  style.height = CMP_LAYOUT_AUTO;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child2, &style));
  child1.style.width = 2.0f;
  child1.style.height = 1.0f;
  child2.style.width = 6.0f;
  child2.style.height = 1.0f;
  children[0] = &child1;
  children[1] = &child2;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));
  CMP_TEST_OK(
      cmp_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size));

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.width = 100.0f;
  style.height = 80.0f;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  spec.mode = CMP_LAYOUT_MEASURE_EXACTLY;
  spec.size = 50.0f;
  CMP_TEST_OK(cmp_layout_test_measure_column(&container, spec, spec, &size));

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  style.wrap = CMP_LAYOUT_WRAP_YES;
  style.width = CMP_LAYOUT_AUTO;
  style.height = 5.0f;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child2, &style));
  child1.style.width = 1.0f;
  child1.style.height = 2.0f;
  child2.style.width = 1.0f;
  child2.style.height = 6.0f;
  children[0] = &child1;
  children[1] = &child2;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));
  CMP_TEST_OK(cmp_layout_test_measure_column(&container, spec_unspec,
                                             spec_unspec, &size));

  direction.flow = 99u;
  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_EXPECT(cmp_layout_test_measure_row_with_direction(
                      &container, &direction, spec_unspec, spec_unspec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_test_measure_node_with_direction(
                      &container, &direction, spec_unspec, spec_unspec),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_test_layout_node_with_direction(
                      &container, &direction, 0.0f, 0.0f, 1.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_test_layout_children_row_with_direction(
                      NULL, &direction, 0.0f, 0.0f, 1.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_test_layout_children_row_with_direction(
                      &container, &direction, 0.0f, 0.0f, 1.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_EXPECT(cmp_layout_test_measure_column_with_direction(
                      &container, &direction, spec_unspec, spec_unspec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_test_layout_children_column_with_direction(
                      NULL, &direction, 0.0f, 0.0f, 1.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_test_layout_children_column_with_direction(
                      &container, &direction, 0.0f, 0.0f, 1.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  direction.flow = CMP_DIRECTION_LTR;

  CMP_TEST_OK(cmp_layout_test_set_direction_fail(CMP_TRUE));
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_row(&container, spec_unspec, spec_unspec, &size),
      CMP_ERR_UNKNOWN);
  CMP_TEST_EXPECT(cmp_layout_test_measure_column(&container, spec_unspec,
                                                 spec_unspec, &size),
                  CMP_ERR_UNKNOWN);
  CMP_TEST_EXPECT(
      cmp_layout_test_measure_node(&container, spec_unspec, spec_unspec),
      CMP_ERR_UNKNOWN);
  CMP_TEST_EXPECT(
      cmp_layout_test_layout_node(&container, 0.0f, 0.0f, 1.0f, 1.0f),
      CMP_ERR_UNKNOWN);
  CMP_TEST_EXPECT(
      cmp_layout_test_layout_children_row(&container, 0.0f, 0.0f, 1.0f, 1.0f),
      CMP_ERR_UNKNOWN);
  CMP_TEST_EXPECT(cmp_layout_test_layout_children_column(&container, 0.0f, 0.0f,
                                                         1.0f, 1.0f),
                  CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_layout_test_set_direction_fail(CMP_FALSE));

  return 0;
}
