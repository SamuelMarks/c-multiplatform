#include "cmpc/cmp_list.h"

#include <string.h>

#ifdef CMP_TESTING
#define CMP_LIST_TEST_FAIL_NONE 0u
#define CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE 1u
#define CMP_LIST_TEST_FAIL_VISIBLE_STRIDE 2u
#define CMP_LIST_TEST_FAIL_VISIBLE_END 3u
#define CMP_LIST_TEST_FAIL_VISIBLE_LAST_BEFORE_FIRST 4u
#define CMP_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN 5u
#define CMP_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO 6u
#define CMP_LIST_TEST_FAIL_GRID_STRIDE 7u
#define CMP_LIST_TEST_FAIL_GRID_END 8u
#define CMP_LIST_TEST_FAIL_GRID_END_NEGATIVE 9u
#define CMP_LIST_TEST_FAIL_GRID_LAST_TOO_LARGE 10u
#define CMP_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST 11u
#define CMP_LIST_TEST_FAIL_RESERVE_NODE_BYTES 12u
#define CMP_LIST_TEST_FAIL_RESERVE_COPY_BYTES 13u
#define CMP_LIST_TEST_FAIL_LIST_ITEM_BOUNDS 14u
#define CMP_LIST_TEST_FAIL_LIST_RENDER_BOUNDS 15u
#define CMP_LIST_TEST_FAIL_GRID_ITEM_BOUNDS 16u
#define CMP_LIST_TEST_FAIL_GRID_RENDER_BOUNDS 17u /* GCOVR_EXCL_LINE */
#define CMP_LIST_TEST_FAIL_LIST_MEASURE_CONTENT_NEGATIVE 18u
#define CMP_LIST_TEST_FAIL_GRID_MEASURE_CONTENT_NEGATIVE /* GCOVR_EXCL_LINE */ \
  19u                                                    /* GCOVR_EXCL_LINE */

static cmp_u32 g_cmp_list_test_fail_point =
    CMP_LIST_TEST_FAIL_NONE; /* GCOVR_EXCL_LINE */

static CMPBool cmp_list_test_fail_point_match(cmp_u32 point) {
  if (g_cmp_list_test_fail_point != point) {
    return CMP_FALSE;
  }
  g_cmp_list_test_fail_point = CMP_LIST_TEST_FAIL_NONE;
  return CMP_TRUE;
}
#endif /* GCOVR_EXCL_LINE */

static cmp_usize cmp_list_usize_max_value(void) {
  return (cmp_usize) ~(cmp_usize)0;
}

static int cmp_list_mul_overflow(cmp_usize a, cmp_usize b,
                                 cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = cmp_list_usize_max_value();
  if (a != 0 && b > max_value / a) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return CMP_OK;
}

static int cmp_list_validate_color(const CMPColor *color) {
  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_list_validate_edges(const CMPLayoutEdges *edges) {
  if (edges == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_list_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_list_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED &&
      spec.mode != CMP_MEASURE_EXACTLY && spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_list_validate_style(const CMPListStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->orientation != CMP_LIST_ORIENTATION_VERTICAL &&
      style->orientation != CMP_LIST_ORIENTATION_HORIZONTAL) {
    return CMP_ERR_RANGE;
  }
  if (style->spacing < 0.0f || style->item_extent < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_list_validate_edges(&style->padding);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_list_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int cmp_grid_validate_style(const CMPGridStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->scroll_axis != CMP_GRID_SCROLL_VERTICAL &&
      style->scroll_axis != CMP_GRID_SCROLL_HORIZONTAL) {
    return CMP_ERR_RANGE;
  }
  if (style->span == 0) {
    return CMP_ERR_RANGE;
  }
  if (style->spacing_x < 0.0f || style->spacing_y < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->item_width < 0.0f || style->item_height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_list_validate_edges(&style->padding);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_list_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int
cmp_list_compute_content_extent(cmp_usize item_count, CMPScalar item_extent,
                                CMPScalar spacing, CMPScalar padding_start,
                                CMPScalar padding_end, CMPScalar *out_extent) {
  if (out_extent == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (item_extent < 0.0f || spacing < 0.0f || padding_start < 0.0f ||
      padding_end < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (item_count == 0) {
    *out_extent = padding_start + padding_end;
    return CMP_OK;
  }
  if (item_extent <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  *out_extent = padding_start + padding_end +
                item_extent * (CMPScalar)item_count +
                spacing * (CMPScalar)(item_count - 1);
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(
          CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE)) {
    *out_extent = -1.0f;
  }
#endif
  if (*out_extent < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int
cmp_list_compute_visible_range(cmp_usize item_count, CMPScalar item_extent,
                               CMPScalar spacing, CMPScalar padding_start,
                               CMPScalar scroll, CMPScalar viewport,
                               cmp_usize overscan, cmp_usize *out_first,
                               cmp_usize *out_last, cmp_usize *out_count) {
  CMPScalar stride;
  CMPScalar start;
  CMPScalar end;
  cmp_usize first;
  cmp_usize last;

  if (out_first == NULL || out_last == NULL || out_count == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_first = CMP_LIST_INVALID_INDEX;
  *out_last = CMP_LIST_INVALID_INDEX;
  *out_count = 0;

  if (item_count == 0 || viewport <= 0.0f) {
    return CMP_OK;
  }
  if (item_extent <= 0.0f || spacing < 0.0f || padding_start < 0.0f) {
    return CMP_ERR_RANGE;
  }

  stride = item_extent + spacing;
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_VISIBLE_STRIDE)) {
    stride = 0.0f;
  }
#endif
  if (stride <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (scroll + viewport <= padding_start) {
    return CMP_OK;
  }

  start = scroll - padding_start;
  if (start <= 0.0f) {
    first = 0;
  } else {
    first = (cmp_usize)(start / stride);
  }

  end = scroll + viewport - padding_start;
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_VISIBLE_END)) {
    end = 0.0f;
  }
#endif
  if (end <= 0.0f) {
    return CMP_OK;
  }
  end -= 0.0001f;
  if (end < 0.0f) {
    end = 0.0f;
  }
  last = (cmp_usize)(end / stride);

  if (first >= item_count) {
    return CMP_OK;
  }
  if (last >= item_count) {
    last = item_count - 1;
  }
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(
          CMP_LIST_TEST_FAIL_VISIBLE_LAST_BEFORE_FIRST)) {
    if (first > 0) {
      last = first - 1;
    } else {
      last = 0;
    }
  }
#endif
  if (last < first) {
    return CMP_OK;
  }

  if (overscan > first) {
    first = 0;
  } else {
    first -= overscan;
  }

  if (last < item_count - 1) {
    cmp_usize max_add = item_count - 1 - last;
    if (overscan > max_add) {
      last = item_count - 1;
    } else {
      last += overscan;
    }
  }

#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(
          CMP_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN)) {
    if (first > 0) {
      last = first - 1;
    } else {
      last = 0;
    }
  }
#endif
  if (last < first) {
    return CMP_OK;
  }

  *out_first = first;
  *out_last = last;
  *out_count = last - first + 1;
  return CMP_OK;
}

static int cmp_grid_compute_visible_range(const CMPGridView *view,
                                          cmp_usize *out_first,
                                          cmp_usize *out_last,
                                          cmp_usize *out_count) {
  CMPScalar item_main;
  CMPScalar spacing_main;
  CMPScalar padding_main;
  CMPScalar viewport;
  CMPScalar stride;
  CMPScalar start;
  CMPScalar end;
  cmp_usize line_count;
  cmp_usize first_line;
  cmp_usize last_line;
  cmp_usize first_index;
  cmp_usize last_index = 0u;

  if (view == NULL || out_first == NULL || out_last == NULL ||
      out_count == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_first = CMP_LIST_INVALID_INDEX;
  *out_last = CMP_LIST_INVALID_INDEX;
  *out_count = 0;

  if (view->item_count == 0) {
    return CMP_OK;
  }

  if (view->style.scroll_axis == CMP_GRID_SCROLL_VERTICAL) {
    item_main = view->style.item_height;
    spacing_main = view->style.spacing_y;
    padding_main = view->style.padding.top;
    viewport = view->bounds.height;
  } else {
    item_main = view->style.item_width;
    spacing_main = view->style.spacing_x;
    padding_main = view->style.padding.left;
    viewport = view->bounds.width;
  }

  if (item_main <= 0.0f || spacing_main < 0.0f || padding_main < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (viewport <= 0.0f) {
    return CMP_OK;
  }

  line_count = view->item_count / view->style.span;
  if (view->item_count % view->style.span != 0) {
    line_count += 1;
  }
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO)) {
    line_count = 0;
  }
#endif
  if (line_count == 0) {
    return CMP_OK;
  }

  stride = item_main + spacing_main;
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_GRID_STRIDE)) {
    stride = 0.0f;
  }
#endif
  if (stride <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (view->scroll_offset + viewport <= padding_main) {
    return CMP_OK;
  }

  start = view->scroll_offset - padding_main;
  if (start <= 0.0f) {
    first_line = 0;
  } else {
    first_line = (cmp_usize)(start / stride);
  }

  end = view->scroll_offset + viewport - padding_main;
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_GRID_END)) {
    end = 0.0f;
  }
#endif
  if (end <= 0.0f) {
    return CMP_OK;
  }
  end -= 0.0001f;
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_GRID_END_NEGATIVE)) {
    end = -1.0f;
  }
#endif
  if (end < 0.0f) {
    end = 0.0f;
  }
  last_line = (cmp_usize)(end / stride);

  if (first_line >= line_count) {
    return CMP_OK;
  }
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_GRID_LAST_TOO_LARGE)) {
    last_line = line_count;
  }
#endif
  if (last_line >= line_count) {
    last_line = line_count - 1;
  }
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(
          CMP_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST)) {
    if (first_line > 0) {
      last_line = first_line - 1;
    } else {
      last_line = 0;
    }
  }
#endif
  if (last_line < first_line) {
    return CMP_OK;
  }

  if (view->style.overscan > first_line) {
    first_line = 0;
  } else {
    first_line -= view->style.overscan;
  }

  if (last_line < line_count - 1) {
    cmp_usize max_add = line_count - 1 - last_line;
    if (view->style.overscan > max_add) {
      last_line = line_count - 1;
    } else {
      last_line += view->style.overscan;
    }
  }

  first_index = first_line * view->style.span;
  last_index = (last_line + 1) * view->style.span - 1;
  if (last_index >= view->item_count) {
    last_index = view->item_count - 1;
  }

  if (last_index < first_index) {
    return CMP_OK;
  }

  *out_first = first_index;
  *out_last = last_index;
  *out_count = last_index - first_index + 1;
  return CMP_OK;
}

static int cmp_list_compute_item_bounds(const CMPListView *view,
                                        cmp_usize index, CMPRect *out_bounds) {
  CMPScalar x;
  CMPScalar y;
  CMPScalar width;
  CMPScalar height;
  CMPScalar stride;

  if (view == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (index >= view->item_count) {
    return CMP_ERR_NOT_FOUND;
  }

  stride = view->style.item_extent + view->style.spacing;
  if (stride <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (view->style.orientation == CMP_LIST_ORIENTATION_VERTICAL) {
    width = view->bounds.width - view->style.padding.left -
            view->style.padding.right;
    height = view->style.item_extent;
    x = view->bounds.x + view->style.padding.left;
    y = view->bounds.y + view->style.padding.top - view->scroll_offset +
        (CMPScalar)index * stride;
  } else {
    width = view->style.item_extent;
    height = view->bounds.height - view->style.padding.top -
             view->style.padding.bottom;
    x = view->bounds.x + view->style.padding.left - view->scroll_offset +
        (CMPScalar)index * stride;
    y = view->bounds.y + view->style.padding.top;
  }

  if (width < 0.0f || height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  out_bounds->x = x;
  out_bounds->y = y;
  out_bounds->width = width;
  out_bounds->height = height;
  return CMP_OK;
}

static int cmp_grid_compute_item_bounds(const CMPGridView *view,
                                        cmp_usize index, CMPRect *out_bounds) {
  cmp_usize row;
  cmp_usize col;
  CMPScalar x;
  CMPScalar y;

  if (view == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (index >= view->item_count) {
    return CMP_ERR_NOT_FOUND;
  }

  if (view->style.item_width <= 0.0f || view->style.item_height <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (view->style.scroll_axis == CMP_GRID_SCROLL_VERTICAL) {
    row = index / view->style.span;
    col = index % view->style.span;
    x = view->bounds.x + view->style.padding.left +
        (CMPScalar)col * (view->style.item_width + view->style.spacing_x);
    y = view->bounds.y + view->style.padding.top +
        (CMPScalar)row * (view->style.item_height + view->style.spacing_y) -
        view->scroll_offset;
  } else {
    row = index % view->style.span;
    col = index / view->style.span;
    x = view->bounds.x + view->style.padding.left +
        (CMPScalar)col * (view->style.item_width + view->style.spacing_x) -
        view->scroll_offset;
    y = view->bounds.y + view->style.padding.top +
        (CMPScalar)row * (view->style.item_height + view->style.spacing_y);
  }

  out_bounds->x = x;
  out_bounds->y = y;
  out_bounds->width = view->style.item_width;
  out_bounds->height = view->style.item_height;
  return CMP_OK;
}

static int cmp_list_view_update_metrics(CMPListView *view) {
  CMPScalar padding_start;
  CMPScalar padding_end;
  CMPScalar viewport;
  CMPScalar max_scroll;
  int rc;

  if (view == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (view->style.orientation == CMP_LIST_ORIENTATION_VERTICAL) {
    padding_start = view->style.padding.top;
    padding_end = view->style.padding.bottom;
    viewport = view->bounds.height;
  } else {
    padding_start = view->style.padding.left;
    padding_end = view->style.padding.right;
    viewport = view->bounds.width;
  }

  rc = cmp_list_compute_content_extent(
      view->item_count, view->style.item_extent, view->style.spacing,
      padding_start, padding_end, &view->content_extent);
  if (rc != CMP_OK) {
    return rc;
  }

  if (viewport < 0.0f) {
    return CMP_ERR_RANGE;
  }

  max_scroll = view->content_extent - viewport;
  if (max_scroll < 0.0f) {
    max_scroll = 0.0f;
  }

  if (view->scroll_offset < 0.0f) {
    view->scroll_offset = 0.0f;
  }
  if (view->scroll_offset > max_scroll) {
    view->scroll_offset = max_scroll;
  }
  return CMP_OK;
}

static int cmp_grid_view_update_metrics(CMPGridView *view) {
  CMPScalar padding_start;
  CMPScalar padding_end;
  CMPScalar spacing_main;
  CMPScalar item_main;
  CMPScalar viewport;
  CMPScalar max_scroll;
  cmp_usize line_count;
  int rc;

  if (view == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (view->style.scroll_axis == CMP_GRID_SCROLL_VERTICAL) {
    padding_start = view->style.padding.top;
    padding_end = view->style.padding.bottom;
    spacing_main = view->style.spacing_y;
    item_main = view->style.item_height;
    viewport = view->bounds.height;
  } else {
    padding_start = view->style.padding.left;
    padding_end = view->style.padding.right;
    spacing_main = view->style.spacing_x;
    item_main = view->style.item_width;
    viewport = view->bounds.width;
  }

  if (spacing_main < 0.0f || item_main < 0.0f || padding_start < 0.0f ||
      padding_end < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (view->style.span == 0) {
    return CMP_ERR_RANGE;
  }

  line_count = 0;
  if (view->item_count > 0) {
    line_count = view->item_count / view->style.span;
    if (view->item_count % view->style.span != 0) {
      line_count += 1;
    }
  }

  if (line_count == 0) {
    view->content_extent = padding_start + padding_end;
  } else {
    if (item_main <= 0.0f) {
      return CMP_ERR_RANGE;
    }
    view->content_extent = padding_start + padding_end +
                           item_main * (CMPScalar)line_count +
                           spacing_main * (CMPScalar)(line_count - 1);
  }
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(
          CMP_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE)) {
    view->content_extent = -1.0f;
  }
#endif

  if (viewport < 0.0f) {
    return CMP_ERR_RANGE;
  }

  max_scroll = view->content_extent - viewport;
  if (max_scroll < 0.0f) {
    max_scroll = 0.0f;
  }

  if (view->scroll_offset < 0.0f) {
    view->scroll_offset = 0.0f;
  }
  if (view->scroll_offset > max_scroll) {
    view->scroll_offset = max_scroll;
  }
  rc = CMP_OK;
  return rc;
}

static int cmp_list_reserve_slots(CMPListSlot **slots,
                                  CMPRenderNode ***visible_nodes,
                                  cmp_usize *slot_capacity,
                                  const CMPAllocator *allocator,
                                  cmp_usize capacity) { /* GCOVR_EXCL_LINE */
  CMPListSlot *new_slots;
  CMPRenderNode **new_nodes;
  cmp_usize slot_bytes;
  cmp_usize node_bytes;
  cmp_usize i;
  int rc;

  if (slots == NULL || visible_nodes == NULL || slot_capacity == NULL ||
      allocator == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (capacity <= *slot_capacity) {
    return CMP_OK;
  }

  rc = cmp_list_mul_overflow(capacity, (cmp_usize)sizeof(CMPListSlot),
                             &slot_bytes);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_list_mul_overflow(capacity, (cmp_usize)sizeof(CMPRenderNode *),
                             &node_bytes);
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_RESERVE_NODE_BYTES)) {
    rc = CMP_ERR_OVERFLOW;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  rc = allocator->alloc(allocator->ctx, slot_bytes, (void **)&new_slots);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = allocator->alloc(allocator->ctx, node_bytes, (void **)&new_nodes);
  if (rc != CMP_OK) {
    allocator->free(allocator->ctx, new_slots);
    return rc;
  }

  if (*slots != NULL && *slot_capacity > 0) {
    cmp_usize copy_bytes;
    rc = cmp_list_mul_overflow(*slot_capacity, (cmp_usize)sizeof(CMPListSlot),
                               &copy_bytes);
#ifdef CMP_TESTING
    if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_RESERVE_COPY_BYTES)) {
      rc = CMP_ERR_OVERFLOW;
    }
#endif
    if (rc != CMP_OK) {
      allocator->free(allocator->ctx, new_nodes);
      allocator->free(allocator->ctx, new_slots);
      return rc;
    }
    memcpy(new_slots, *slots, (size_t)copy_bytes);
  }

  for (i = *slot_capacity; i < capacity; ++i) {
    memset(&new_slots[i], 0, sizeof(new_slots[i]));
    new_slots[i].index = CMP_LIST_INVALID_INDEX;
  }

  memset(new_nodes, 0, (size_t)node_bytes);

  if (*slots != NULL) {
    allocator->free(allocator->ctx, *slots);
  }
  if (*visible_nodes != NULL) {
    allocator->free(allocator->ctx, *visible_nodes);
  }

  *slots = new_slots;
  *visible_nodes = new_nodes;
  *slot_capacity = capacity;
  return CMP_OK;
}

static int cmp_list_widget_measure(void *widget, CMPMeasureSpec width,
                                   CMPMeasureSpec height,
                                   CMPSize *out_size) { /* GCOVR_EXCL_LINE */
  CMPListView *view = NULL;
  CMPScalar content_width = 0.0f;
  CMPScalar content_height = 0.0f;
  int rc = CMP_OK;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_list_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_list_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc;
  }

  view = (CMPListView *)widget;
  rc = cmp_list_validate_style(&view->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_list_view_update_metrics(view);
  if (rc != CMP_OK) {
    return rc;
  }

  if (view->style.orientation == CMP_LIST_ORIENTATION_VERTICAL) {
    content_width = view->style.padding.left + view->style.padding.right;
    content_height = view->content_extent;
  } else {
    content_width = view->content_extent;
    content_height = view->style.padding.top + view->style.padding.bottom;
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_list_test_fail_point_match(
          CMP_LIST_TEST_FAIL_LIST_MEASURE_CONTENT_NEGATIVE)) {
    content_width = -1.0f;
  }
#endif /* GCOVR_EXCL_LINE */

  if (content_width < 0.0f || content_height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    out_size->width = (content_width > width.size) ? width.size : content_width;
  } else {
    out_size->width = content_width;
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == CMP_MEASURE_AT_MOST) {
    out_size->height =
        (content_height > height.size) ? height.size : content_height;
  } else {
    out_size->height = content_height;
  }

  return CMP_OK;
}

static int cmp_list_widget_layout(void *widget, CMPRect bounds) {
  CMPListView *view = NULL;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_list_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  view = (CMPListView *)widget;
  view->bounds = bounds;
  return cmp_list_view_update_metrics(view);
}

static int cmp_list_widget_paint(void *widget, CMPPaintContext *ctx) {
  CMPListView *view = NULL;
  int rc = CMP_OK;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  view = (CMPListView *)widget;
  rc = cmp_list_validate_style(&view->style);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_list_validate_rect(&view->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  if (view->style.background_color.a <= 0.0f) {
    return CMP_OK;
  }
  if (ctx->gfx->vtable == NULL || ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  return ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &view->bounds,
                                     view->style.background_color, 0.0f);
}

static int cmp_list_widget_event(void *widget, const CMPInputEvent *event,
                                 CMPBool *out_handled) { /* GCOVR_EXCL_LINE */
  CMPListView *view = NULL;
  CMPScalar delta; /* GCOVR_EXCL_LINE */
  int rc;          /* GCOVR_EXCL_LINE */

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  view = (CMPListView *)widget;
  if (view->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  if (event->type != CMP_INPUT_POINTER_SCROLL) {
    return CMP_OK;
  }

  if (view->style.orientation == CMP_LIST_ORIENTATION_VERTICAL) {
    delta = (CMPScalar)event->data.pointer.scroll_y;
  } else {
    delta = (CMPScalar)event->data.pointer.scroll_x;
  }

  if (delta == 0.0f) {
    return CMP_OK;
  }

  rc = cmp_list_view_set_scroll(view, view->scroll_offset + delta);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_handled = CMP_TRUE;
  return CMP_OK;
}

static int cmp_list_widget_get_semantics(
    void *widget, CMPSemantics *out_semantics) { /* GCOVR_EXCL_LINE */
  CMPListView *view = NULL;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  view = (CMPListView *)widget;
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (view->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int cmp_list_widget_destroy(void *widget) {
  CMPListView *view = NULL;
  int rc = CMP_OK;
  int rc2 = CMP_OK;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  view = (CMPListView *)widget;
  rc = CMP_OK;

  if (view->slots != NULL && view->allocator.free != NULL) {
    rc2 = view->allocator.free(view->allocator.ctx, view->slots);
    if (rc2 != CMP_OK && rc == CMP_OK) {
      rc = rc2;
    }
  }
  if (view->visible_nodes != NULL && view->allocator.free != NULL) {
    rc2 = view->allocator.free(view->allocator.ctx, view->visible_nodes);
    if (rc2 != CMP_OK && rc == CMP_OK) {
      rc = rc2;
    }
  }

  view->slots = NULL;
  view->visible_nodes = NULL;
  view->slot_capacity = 0u;
  view->visible_count = 0u;
  view->visible_first = CMP_LIST_INVALID_INDEX;
  view->visible_last = CMP_LIST_INVALID_INDEX;
  view->item_count = 0u;
  view->scroll_offset = 0.0f;
  view->content_extent = 0.0f;
  view->bind = NULL;
  view->bind_ctx = NULL;
  view->allocator.ctx = NULL;
  view->allocator.alloc = NULL;
  view->allocator.realloc = NULL;
  view->allocator.free = NULL;
  view->widget.ctx = NULL;
  view->widget.vtable = NULL;
  return rc;
}

static const CMPWidgetVTable g_cmp_list_widget_vtable = /* GCOVR_EXCL_LINE */
    {cmp_list_widget_measure,
     cmp_list_widget_layout, /* GCOVR_EXCL_LINE */
     cmp_list_widget_paint,
     cmp_list_widget_event, /* GCOVR_EXCL_LINE */
     cmp_list_widget_get_semantics,
     cmp_list_widget_destroy}; /* GCOVR_EXCL_LINE */

static int cmp_grid_widget_measure(void *widget, CMPMeasureSpec width,
                                   CMPMeasureSpec height, CMPSize *out_size) {
  CMPGridView *view = NULL;
  CMPScalar content_width = 0.0f;
  CMPScalar content_height = 0.0f;
  int rc = CMP_OK;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_list_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_list_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc;
  }

  view = (CMPGridView *)widget;
  rc = cmp_grid_validate_style(&view->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_grid_view_update_metrics(view);
  if (rc != CMP_OK) {
    return rc;
  }

  if (view->style.scroll_axis == CMP_GRID_SCROLL_VERTICAL) {
    content_width = view->style.padding.left + view->style.padding.right +
                    view->style.item_width * (CMPScalar)view->style.span +
                    view->style.spacing_x * (CMPScalar)(view->style.span - 1);
    content_height = view->content_extent;
  } else {
    content_width = view->content_extent;
    content_height = view->style.padding.top + view->style.padding.bottom +
                     view->style.item_height * (CMPScalar)view->style.span +
                     view->style.spacing_y * (CMPScalar)(view->style.span - 1);
  }
#ifdef CMP_TESTING
  if (cmp_list_test_fail_point_match(
          CMP_LIST_TEST_FAIL_GRID_MEASURE_CONTENT_NEGATIVE)) {
    content_width = -1.0f;
  }
#endif

  if (content_width < 0.0f || content_height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    out_size->width = (content_width > width.size) ? width.size : content_width;
  } else {
    out_size->width = content_width;
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == CMP_MEASURE_AT_MOST) {
    out_size->height =
        (content_height > height.size) ? height.size : content_height;
  } else {
    out_size->height = content_height;
  }

  return CMP_OK;
}

static int cmp_grid_widget_layout(void *widget, CMPRect bounds) {
  CMPGridView *view; /* GCOVR_EXCL_LINE */
  int rc;            /* GCOVR_EXCL_LINE */

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_list_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  view = (CMPGridView *)widget;
  view->bounds = bounds;
  return cmp_grid_view_update_metrics(view);
}

static int cmp_grid_widget_paint(void *widget, CMPPaintContext *ctx) {
  CMPGridView *view = NULL;
  int rc = CMP_OK;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  view = (CMPGridView *)widget;
  rc = cmp_grid_validate_style(&view->style);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_list_validate_rect(&view->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  if (view->style.background_color.a <= 0.0f) {
    return CMP_OK;
  }
  if (ctx->gfx->vtable == NULL || ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  return ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &view->bounds,
                                     view->style.background_color, 0.0f);
}

static int cmp_grid_widget_event(void *widget, const CMPInputEvent *event,
                                 CMPBool *out_handled) { /* GCOVR_EXCL_LINE */
  CMPGridView *view = NULL;
  CMPScalar delta = 0.0f;
  int rc = CMP_OK;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  view = (CMPGridView *)widget;
  if (view->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  if (event->type != CMP_INPUT_POINTER_SCROLL) {
    return CMP_OK;
  }

  if (view->style.scroll_axis == CMP_GRID_SCROLL_VERTICAL) {
    delta = (CMPScalar)event->data.pointer.scroll_y;
  } else {
    delta = (CMPScalar)event->data.pointer.scroll_x;
  }

  if (delta == 0.0f) {
    return CMP_OK;
  }

  rc = cmp_grid_view_set_scroll(view, view->scroll_offset + delta);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_handled = CMP_TRUE;
  return CMP_OK;
}

static int /* GCOVR_EXCL_LINE */
cmp_grid_widget_get_semantics(void *widget, CMPSemantics *out_semantics) {
  CMPGridView *view;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  view = (CMPGridView *)widget;
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (view->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int cmp_grid_widget_destroy(void *widget) {
  CMPGridView *view = NULL;
  int rc = CMP_OK;
  int rc2 = CMP_OK;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  view = (CMPGridView *)widget;
  rc = CMP_OK;

  if (view->slots != NULL && view->allocator.free != NULL) {
    rc2 = view->allocator.free(view->allocator.ctx, view->slots);
    if (rc2 != CMP_OK && rc == CMP_OK) {
      rc = rc2;
    }
  }
  if (view->visible_nodes != NULL && view->allocator.free != NULL) {
    rc2 = view->allocator.free(view->allocator.ctx, view->visible_nodes);
    if (rc2 != CMP_OK && rc == CMP_OK) {
      rc = rc2;
    }
  }

  view->slots = NULL;
  view->visible_nodes = NULL;
  view->slot_capacity = 0u;
  view->visible_count = 0u;
  view->visible_first = CMP_LIST_INVALID_INDEX;
  view->visible_last = CMP_LIST_INVALID_INDEX;
  view->item_count = 0u;
  view->scroll_offset = 0.0f;
  view->content_extent = 0.0f;
  view->bind = NULL;
  view->bind_ctx = NULL;
  view->allocator.ctx = NULL;
  view->allocator.alloc = NULL;
  view->allocator.realloc = NULL;
  view->allocator.free = NULL;
  view->widget.ctx = NULL;
  view->widget.vtable = NULL;
  return rc;
}

static const CMPWidgetVTable g_cmp_grid_widget_vtable = {
    cmp_grid_widget_measure,       cmp_grid_widget_layout,
    cmp_grid_widget_paint,         cmp_grid_widget_event,
    cmp_grid_widget_get_semantics, cmp_grid_widget_destroy};

int CMP_CALL cmp_list_style_init(CMPListStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  style->orientation = CMP_LIST_ORIENTATION_VERTICAL;
  style->padding.left = 0.0f;
  style->padding.top = 0.0f;
  style->padding.right = 0.0f;
  style->padding.bottom = 0.0f;
  style->spacing = CMP_LIST_DEFAULT_SPACING;
  style->item_extent = CMP_LIST_DEFAULT_ITEM_EXTENT;
  style->background_color.r = 0.0f;
  style->background_color.g = 0.0f;
  style->background_color.b = 0.0f;
  style->background_color.a = 0.0f;
  style->overscan = CMP_LIST_DEFAULT_OVERSCAN;
  return CMP_OK;
}

int CMP_CALL cmp_list_view_init(CMPListView *view, const CMPListStyle *style,
                                const CMPAllocator *allocator,
                                cmp_usize item_count, cmp_usize slot_capacity) {
  CMPAllocator alloc;
  int rc;

  if (view == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_list_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  if (allocator == NULL) {
    rc = cmp_get_default_allocator(&alloc);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    alloc = *allocator;
  }

  if (alloc.alloc == NULL || alloc.realloc == NULL || alloc.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(view, 0, sizeof(*view));
  view->style = *style;
  view->allocator = alloc;
  view->item_count = item_count;
  view->visible_first = CMP_LIST_INVALID_INDEX;
  view->visible_last = CMP_LIST_INVALID_INDEX;
  view->widget.ctx = view;
  view->widget.vtable = &g_cmp_list_widget_vtable;
  view->widget.handle.id = 0u;
  view->widget.handle.generation = 0u;
  view->widget.flags = 0u;

  if (slot_capacity > 0) {
    rc = cmp_list_view_reserve(view, slot_capacity);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

int CMP_CALL cmp_list_view_set_bind(CMPListView *view, CMPListBindFn bind,
                                    void *ctx) {
  if (view == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  view->bind = bind;
  view->bind_ctx = ctx;
  return CMP_OK;
}

int CMP_CALL cmp_list_view_set_item_count(CMPListView *view,
                                          cmp_usize item_count) {
  int rc;

  if (view == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  view->item_count = item_count;
  rc = cmp_list_view_update_metrics(view);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

int CMP_CALL cmp_list_view_set_style(CMPListView *view,
                                     const CMPListStyle *style) {
  int rc = CMP_OK;

  if (view == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_list_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  view->style = *style;
  rc = cmp_list_view_update_metrics(view);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

int CMP_CALL cmp_list_view_set_scroll(CMPListView *view, CMPScalar offset) {
  int rc;

  if (view == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (offset < 0.0f) {
    offset = 0.0f;
  }

  view->scroll_offset = offset;
  rc = cmp_list_view_update_metrics(view);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

int CMP_CALL cmp_list_view_get_scroll(const CMPListView *view,
                                      CMPScalar *out_offset) {
  if (view == NULL || out_offset == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_offset = view->scroll_offset;
  return CMP_OK;
}

int CMP_CALL cmp_list_view_get_content_extent(const CMPListView *view,
                                              CMPScalar *out_extent) {
  CMPListView temp = {0};
  int rc = CMP_OK;

  if (view == NULL || out_extent == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  temp = *view;
  rc = cmp_list_view_update_metrics(&temp);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_extent = temp.content_extent;
  return CMP_OK;
}

int CMP_CALL cmp_list_view_get_required_slots(const CMPListView *view,
                                              cmp_usize *out_required) {
  CMPListView temp; /* GCOVR_EXCL_LINE */
  CMPScalar padding_start;
  CMPScalar viewport;
  cmp_usize first;
  cmp_usize last;
  cmp_usize count;
  int rc;

  if (view == NULL || out_required == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_list_validate_style(&view->style);
  if (rc != CMP_OK) {
    return rc;
  }

  temp = *view;
  rc = cmp_list_view_update_metrics(&temp);
  if (rc != CMP_OK) {
    return rc;
  }

  if (temp.style.orientation == CMP_LIST_ORIENTATION_VERTICAL) {
    padding_start = temp.style.padding.top;
    viewport = temp.bounds.height;
  } else {
    padding_start = temp.style.padding.left;
    viewport = temp.bounds.width;
  }

  rc = cmp_list_compute_visible_range(
      temp.item_count, temp.style.item_extent, temp.style.spacing,
      padding_start, temp.scroll_offset, viewport, temp.style.overscan, &first,
      &last, &count);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_required = count;
  return CMP_OK;
}

int CMP_CALL cmp_list_view_reserve(CMPListView *view, cmp_usize capacity) {
  int rc;

  if (view == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_list_reserve_slots(&view->slots, &view->visible_nodes,
                              &view->slot_capacity, &view->allocator, capacity);
  if (rc != CMP_OK) {
    return rc;
  }

  if (capacity > 0 && view->visible_nodes != NULL) {
    memset(view->visible_nodes, 0,
           (size_t)(capacity * sizeof(CMPRenderNode *)));
  }
  view->visible_count = 0u;
  view->visible_first = CMP_LIST_INVALID_INDEX;
  view->visible_last = CMP_LIST_INVALID_INDEX;
  return CMP_OK;
}

int CMP_CALL cmp_list_view_update(CMPListView *view) {
  CMPScalar padding_start;
  CMPScalar viewport;
  cmp_usize first;
  cmp_usize last;
  cmp_usize count = 0u;
  cmp_usize i = 0u;
  cmp_usize index;
  CMPRect bounds;
  int rc;

  if (view == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_list_validate_style(&view->style);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_list_validate_rect(&view->bounds);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_list_view_update_metrics(view);
  if (rc != CMP_OK) {
    return rc;
  }

  if (view->style.orientation == CMP_LIST_ORIENTATION_VERTICAL) {
    padding_start = view->style.padding.top;
    viewport = view->bounds.height;
  } else {
    padding_start = view->style.padding.left;
    viewport = view->bounds.width;
  }

  rc = cmp_list_compute_visible_range(
      view->item_count, view->style.item_extent, view->style.spacing,
      padding_start, view->scroll_offset, viewport, view->style.overscan,
      &first, &last, &count); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) {
    return rc;
  }

  view->visible_count = 0u;
  view->visible_first = CMP_LIST_INVALID_INDEX;
  view->visible_last = CMP_LIST_INVALID_INDEX;

  if (count == 0) {
    if (view->visible_nodes != NULL && view->slot_capacity > 0) {
      memset(view->visible_nodes, 0,
             (size_t)(view->slot_capacity * sizeof(CMPRenderNode *)));
    }
    return CMP_OK;
  }

  if (view->slots == NULL || view->visible_nodes == NULL) {
    return CMP_ERR_STATE;
  }
  if (view->slot_capacity < count) {
    return CMP_ERR_RANGE;
  }
  if (view->bind == NULL) {
    return CMP_ERR_STATE;
  }

  view->visible_first = first;
  view->visible_last = last;

  index = first;
  for (i = 0; i < count; ++i) {
    CMPListSlot *slot = &view->slots[i];

    slot->index = index;
    rc = view->bind(view->bind_ctx, slot, index);
    if (rc != CMP_OK) {
      return rc;
    }
    if (slot->node.widget == NULL || slot->node.widget->vtable == NULL) {
      return CMP_ERR_STATE;
    }

    rc = cmp_list_compute_item_bounds(view, index, &bounds);
#ifdef CMP_TESTING
    if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_LIST_ITEM_BOUNDS)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_render_node_set_bounds(&slot->node, &bounds);
#ifdef CMP_TESTING
    if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_LIST_RENDER_BOUNDS)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }

    view->visible_nodes[i] = &slot->node;
    view->visible_count += 1;
    index += 1;
  }

  for (i = count; i < view->slot_capacity; ++i) {
    view->slots[i].index = CMP_LIST_INVALID_INDEX;
    view->visible_nodes[i] = NULL;
  }

  return CMP_OK;
}

int CMP_CALL cmp_list_view_get_visible(const CMPListView *view,
                                       CMPRenderNode ***out_nodes,
                                       cmp_usize *out_count) {
  if (view == NULL || out_nodes == NULL || out_count == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_nodes = view->visible_nodes;
  *out_count = view->visible_count;
  return CMP_OK;
}

int CMP_CALL cmp_grid_style_init(CMPGridStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  style->scroll_axis = CMP_GRID_SCROLL_VERTICAL;
  style->span = CMP_GRID_DEFAULT_SPAN;
  style->padding.left = 0.0f;
  style->padding.top = 0.0f;
  style->padding.right = 0.0f;
  style->padding.bottom = 0.0f;
  style->spacing_x = CMP_GRID_DEFAULT_SPACING;
  style->spacing_y = CMP_GRID_DEFAULT_SPACING;
  style->item_width = CMP_GRID_DEFAULT_ITEM_WIDTH;
  style->item_height = CMP_GRID_DEFAULT_ITEM_HEIGHT;
  style->background_color.r = 0.0f;
  style->background_color.g = 0.0f;
  style->background_color.b = 0.0f;
  style->background_color.a = 0.0f;
  style->overscan = CMP_GRID_DEFAULT_OVERSCAN;
  return CMP_OK;
}

int CMP_CALL cmp_grid_view_init(CMPGridView *view, const CMPGridStyle *style,
                                const CMPAllocator *allocator,
                                cmp_usize item_count, cmp_usize slot_capacity) {
  CMPAllocator alloc;
  int rc;

  if (view == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_grid_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  if (allocator == NULL) {
    rc = cmp_get_default_allocator(&alloc);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    alloc = *allocator;
  }

  if (alloc.alloc == NULL || alloc.realloc == NULL || alloc.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(view, 0, sizeof(*view));
  view->style = *style;
  view->allocator = alloc;
  view->item_count = item_count;
  view->visible_first = CMP_LIST_INVALID_INDEX;
  view->visible_last = CMP_LIST_INVALID_INDEX;
  view->widget.ctx = view;
  view->widget.vtable = &g_cmp_grid_widget_vtable;
  view->widget.handle.id = 0u;
  view->widget.handle.generation = 0u;
  view->widget.flags = 0u;

  if (slot_capacity > 0) {
    rc = cmp_grid_view_reserve(view, slot_capacity);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

int CMP_CALL cmp_grid_view_set_bind(CMPGridView *view, CMPListBindFn bind,
                                    void *ctx) {
  if (view == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  view->bind = bind;
  view->bind_ctx = ctx;
  return CMP_OK;
}

int CMP_CALL cmp_grid_view_set_item_count(CMPGridView *view,
                                          cmp_usize item_count) {
  int rc;

  if (view == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  view->item_count = item_count;
  rc = cmp_grid_view_update_metrics(view);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

int CMP_CALL cmp_grid_view_set_style(CMPGridView *view,
                                     const CMPGridStyle *style) {
  int rc;

  if (view == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_grid_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  view->style = *style;
  rc = cmp_grid_view_update_metrics(view);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

int CMP_CALL cmp_grid_view_set_scroll(CMPGridView *view, CMPScalar offset) {
  int rc = CMP_OK;

  if (view == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (offset < 0.0f) {
    offset = 0.0f;
  }

  view->scroll_offset = offset;
  rc = cmp_grid_view_update_metrics(view);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

int CMP_CALL cmp_grid_view_get_scroll(const CMPGridView *view,
                                      CMPScalar *out_offset) {
  if (view == NULL || out_offset == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_offset = view->scroll_offset;
  return CMP_OK;
}

int CMP_CALL cmp_grid_view_get_content_extent(const CMPGridView *view,
                                              CMPScalar *out_extent) {
  CMPGridView temp;
  int rc;

  if (view == NULL || out_extent == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  temp = *view;
  rc = cmp_grid_view_update_metrics(&temp);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_extent = temp.content_extent;
  return CMP_OK;
}

int CMP_CALL cmp_grid_view_get_required_slots(const CMPGridView *view,
                                              cmp_usize *out_required) {
  CMPGridView temp;
  cmp_usize first;
  cmp_usize last;
  cmp_usize count;
  int rc;

  if (view == NULL || out_required == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_grid_validate_style(&view->style);
  if (rc != CMP_OK) {
    return rc;
  }

  temp = *view;
  rc = cmp_grid_view_update_metrics(&temp);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_grid_compute_visible_range(&temp, &first, &last, &count);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_required = count;
  return CMP_OK;
}

int CMP_CALL cmp_grid_view_reserve(CMPGridView *view, cmp_usize capacity) {
  int rc; /* GCOVR_EXCL_LINE */

  if (view == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_list_reserve_slots(&view->slots, &view->visible_nodes,
                              &view->slot_capacity, &view->allocator, capacity);
  if (rc != CMP_OK) {
    return rc;
  }

  if (capacity > 0 && view->visible_nodes != NULL) {
    memset(view->visible_nodes, 0,
           (size_t)(capacity * sizeof(CMPRenderNode *)));
  }
  view->visible_count = 0u;
  view->visible_first = CMP_LIST_INVALID_INDEX;
  view->visible_last = CMP_LIST_INVALID_INDEX;
  return CMP_OK;
}

int CMP_CALL cmp_grid_view_update(CMPGridView *view) {
  cmp_usize first;
  cmp_usize last;
  cmp_usize count = 0u;
  cmp_usize i;
  cmp_usize index;
  CMPRect bounds;
  int rc;

  if (view == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_grid_validate_style(&view->style);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_list_validate_rect(&view->bounds);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_grid_view_update_metrics(view);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_grid_compute_visible_range(view, &first, &last, &count);
  if (rc != CMP_OK) {
    return rc;
  }

  view->visible_count = 0u;
  view->visible_first = CMP_LIST_INVALID_INDEX;
  view->visible_last = CMP_LIST_INVALID_INDEX;

  if (count == 0) {
    if (view->visible_nodes != NULL && view->slot_capacity > 0) {
      memset(view->visible_nodes, 0,
             (size_t)(view->slot_capacity * sizeof(CMPRenderNode *)));
    }
    return CMP_OK;
  }

  if (view->slots == NULL || view->visible_nodes == NULL) {
    return CMP_ERR_STATE;
  }
  if (view->slot_capacity < count) {
    return CMP_ERR_RANGE;
  }
  if (view->bind == NULL) {
    return CMP_ERR_STATE;
  }

  view->visible_first = first;
  view->visible_last = last;

  index = first;
  for (i = 0; i < count; ++i) {
    CMPListSlot *slot = &view->slots[i];

    slot->index = index;
    rc = view->bind(view->bind_ctx, slot, index);
    if (rc != CMP_OK) {
      return rc;
    }
    if (slot->node.widget == NULL || slot->node.widget->vtable == NULL) {
      return CMP_ERR_STATE;
    }

    rc = cmp_grid_compute_item_bounds(view, index, &bounds);
#ifdef CMP_TESTING
    if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_GRID_ITEM_BOUNDS)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_render_node_set_bounds(&slot->node, &bounds);
#ifdef CMP_TESTING
    if (cmp_list_test_fail_point_match(CMP_LIST_TEST_FAIL_GRID_RENDER_BOUNDS)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }

    view->visible_nodes[i] = &slot->node;
    view->visible_count += 1;
    index += 1;
  }

  for (i = count; i < view->slot_capacity; ++i) {
    view->slots[i].index = CMP_LIST_INVALID_INDEX;
    view->visible_nodes[i] = NULL;
  }

  return CMP_OK;
}

int CMP_CALL cmp_grid_view_get_visible(const CMPGridView *view,
                                       CMPRenderNode ***out_nodes,
                                       cmp_usize *out_count) {
  if (view == NULL || out_nodes == NULL || out_count == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_nodes = view->visible_nodes;
  *out_count = view->visible_count;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_list_test_set_fail_point(cmp_u32 fail_point) {
  g_cmp_list_test_fail_point = fail_point;
  return CMP_OK;
}

int CMP_CALL cmp_list_test_clear_fail_points(void) {
  g_cmp_list_test_fail_point = CMP_LIST_TEST_FAIL_NONE;
  return CMP_OK;
}

int CMP_CALL cmp_list_test_mul_overflow(cmp_usize a, cmp_usize b,
                                        cmp_usize *out_value) {
  return cmp_list_mul_overflow(a, b, out_value);
}

int CMP_CALL cmp_list_test_validate_color(const CMPColor *color) {
  return cmp_list_validate_color(color);
}

int CMP_CALL cmp_list_test_validate_edges(const CMPLayoutEdges *edges) {
  return cmp_list_validate_edges(edges);
}

int CMP_CALL cmp_list_test_validate_rect(const CMPRect *rect) {
  return cmp_list_validate_rect(rect);
}

int CMP_CALL cmp_list_test_validate_measure_spec(CMPMeasureSpec spec) {
  return cmp_list_validate_measure_spec(spec);
}

int CMP_CALL cmp_list_test_validate_style(const CMPListStyle *style) {
  return cmp_list_validate_style(style);
}

int CMP_CALL cmp_list_test_validate_grid_style(const CMPGridStyle *style) {
  return cmp_grid_validate_style(style);
}

int CMP_CALL cmp_list_test_compute_content_extent(
    cmp_usize item_count, CMPScalar item_extent, CMPScalar spacing,
    CMPScalar padding_start, CMPScalar padding_end, CMPScalar *out_extent) {
  return cmp_list_compute_content_extent(
      item_count, item_extent, spacing, padding_start, padding_end, out_extent);
}

int CMP_CALL cmp_list_test_compute_visible_range(
    cmp_usize item_count, CMPScalar item_extent, CMPScalar spacing,
    CMPScalar padding_start, CMPScalar scroll, CMPScalar viewport,
    cmp_usize overscan, cmp_usize *out_first, cmp_usize *out_last,
    cmp_usize *out_count) {
  return cmp_list_compute_visible_range(
      item_count, item_extent, spacing, padding_start, scroll, viewport,
      overscan, out_first, out_last, out_count);
}

int CMP_CALL cmp_list_test_grid_compute_visible_range(const CMPGridView *view,
                                                      cmp_usize *out_first,
                                                      cmp_usize *out_last,
                                                      cmp_usize *out_count) {
  return cmp_grid_compute_visible_range(view, out_first, out_last, out_count);
}

int CMP_CALL cmp_list_test_compute_item_bounds(const CMPListView *view,
                                               cmp_usize index,
                                               CMPRect *out_bounds) {
  return cmp_list_compute_item_bounds(view, index, out_bounds);
}

int CMP_CALL cmp_list_test_grid_compute_item_bounds(const CMPGridView *view,
                                                    cmp_usize index,
                                                    CMPRect *out_bounds) {
  return cmp_grid_compute_item_bounds(view, index, out_bounds);
}

int CMP_CALL cmp_list_test_update_metrics(CMPListView *view) {
  return cmp_list_view_update_metrics(view);
}

int CMP_CALL cmp_list_test_grid_update_metrics(CMPGridView *view) {
  return cmp_grid_view_update_metrics(view);
}

int CMP_CALL cmp_list_test_reserve_slots(CMPListSlot **slots,
                                         CMPRenderNode ***visible_nodes,
                                         cmp_usize *slot_capacity,
                                         const CMPAllocator *allocator,
                                         cmp_usize capacity) {
  return cmp_list_reserve_slots(slots, visible_nodes, slot_capacity, allocator,
                                capacity);
}
#endif
