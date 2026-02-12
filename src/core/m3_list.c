#include "m3/m3_list.h"

#include <string.h>

#ifdef M3_TESTING
#define M3_LIST_TEST_FAIL_NONE 0u
#define M3_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE 1u
#define M3_LIST_TEST_FAIL_VISIBLE_STRIDE 2u
#define M3_LIST_TEST_FAIL_VISIBLE_END 3u
#define M3_LIST_TEST_FAIL_VISIBLE_LAST_BEFORE_FIRST 4u
#define M3_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN 5u
#define M3_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO 6u
#define M3_LIST_TEST_FAIL_GRID_STRIDE 7u
#define M3_LIST_TEST_FAIL_GRID_END 8u
#define M3_LIST_TEST_FAIL_GRID_END_NEGATIVE 9u
#define M3_LIST_TEST_FAIL_GRID_LAST_TOO_LARGE 10u
#define M3_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST 11u
#define M3_LIST_TEST_FAIL_RESERVE_NODE_BYTES 12u
#define M3_LIST_TEST_FAIL_RESERVE_COPY_BYTES 13u
#define M3_LIST_TEST_FAIL_LIST_ITEM_BOUNDS 14u
#define M3_LIST_TEST_FAIL_LIST_RENDER_BOUNDS 15u
#define M3_LIST_TEST_FAIL_GRID_ITEM_BOUNDS 16u
#define M3_LIST_TEST_FAIL_GRID_RENDER_BOUNDS 17u /* GCOVR_EXCL_LINE */
#define M3_LIST_TEST_FAIL_LIST_MEASURE_CONTENT_NEGATIVE 18u
#define M3_LIST_TEST_FAIL_GRID_MEASURE_CONTENT_NEGATIVE                        \
  19u /* GCOVR_EXCL_LINE */

static m3_u32 g_m3_list_test_fail_point = M3_LIST_TEST_FAIL_NONE;

static M3Bool m3_list_test_fail_point_match(m3_u32 point) {
  if (g_m3_list_test_fail_point != point) {
    return M3_FALSE;
  }
  g_m3_list_test_fail_point = M3_LIST_TEST_FAIL_NONE;
  return M3_TRUE;
}
#endif /* GCOVR_EXCL_LINE */

static m3_usize m3_list_usize_max_value(void) {
  return (m3_usize) ~(m3_usize)0;
}

static int m3_list_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value) {
  m3_usize max_value;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_value = m3_list_usize_max_value();
  if (a != 0 && b > max_value / a) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return M3_OK;
}

static int m3_list_validate_color(const M3Color *color) {
  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_list_validate_edges(const M3LayoutEdges *edges) {
  if (edges == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_list_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_list_validate_measure_spec(M3MeasureSpec spec) {
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY &&
      spec.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_list_validate_style(const M3ListStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->orientation != M3_LIST_ORIENTATION_VERTICAL &&
      style->orientation != M3_LIST_ORIENTATION_HORIZONTAL) {
    return M3_ERR_RANGE;
  }
  if (style->spacing < 0.0f || style->item_extent < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_list_validate_edges(&style->padding);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_list_validate_color(&style->background_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_grid_validate_style(const M3GridStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->scroll_axis != M3_GRID_SCROLL_VERTICAL &&
      style->scroll_axis != M3_GRID_SCROLL_HORIZONTAL) {
    return M3_ERR_RANGE;
  }
  if (style->span == 0) {
    return M3_ERR_RANGE;
  }
  if (style->spacing_x < 0.0f || style->spacing_y < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->item_width < 0.0f || style->item_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_list_validate_edges(&style->padding);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_list_validate_color(&style->background_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int
m3_list_compute_content_extent(m3_usize item_count, M3Scalar item_extent,
                               M3Scalar spacing, M3Scalar padding_start,
                               M3Scalar padding_end, M3Scalar *out_extent) {
  if (out_extent == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (item_extent < 0.0f || spacing < 0.0f || padding_start < 0.0f ||
      padding_end < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (item_count == 0) {
    *out_extent = padding_start + padding_end;
    return M3_OK;
  }
  if (item_extent <= 0.0f) {
    return M3_ERR_RANGE;
  }

  *out_extent = padding_start + padding_end +
                item_extent * (M3Scalar)item_count +
                spacing * (M3Scalar)(item_count - 1);
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(
          M3_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE)) {
    *out_extent = -1.0f;
  }
#endif
  if (*out_extent < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_list_compute_visible_range(m3_usize item_count,
                                         M3Scalar item_extent, M3Scalar spacing,
                                         M3Scalar padding_start,
                                         M3Scalar scroll, M3Scalar viewport,
                                         m3_usize overscan, m3_usize *out_first,
                                         m3_usize *out_last,
                                         m3_usize *out_count) {
  M3Scalar stride;
  M3Scalar start;
  M3Scalar end;
  m3_usize first;
  m3_usize last;

  if (out_first == NULL || out_last == NULL || out_count == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_first = M3_LIST_INVALID_INDEX;
  *out_last = M3_LIST_INVALID_INDEX;
  *out_count = 0;

  if (item_count == 0 || viewport <= 0.0f) {
    return M3_OK;
  }
  if (item_extent <= 0.0f || spacing < 0.0f || padding_start < 0.0f) {
    return M3_ERR_RANGE;
  }

  stride = item_extent + spacing;
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_VISIBLE_STRIDE)) {
    stride = 0.0f;
  }
#endif
  if (stride <= 0.0f) {
    return M3_ERR_RANGE;
  }

  if (scroll + viewport <= padding_start) {
    return M3_OK;
  }

  start = scroll - padding_start;
  if (start <= 0.0f) {
    first = 0;
  } else {
    first = (m3_usize)(start / stride);
  }

  end = scroll + viewport - padding_start;
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_VISIBLE_END)) {
    end = 0.0f;
  }
#endif
  if (end <= 0.0f) {
    return M3_OK;
  }
  end -= 0.0001f;
  if (end < 0.0f) {
    end = 0.0f;
  }
  last = (m3_usize)(end / stride);

  if (first >= item_count) {
    return M3_OK;
  }
  if (last >= item_count) {
    last = item_count - 1;
  }
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(
          M3_LIST_TEST_FAIL_VISIBLE_LAST_BEFORE_FIRST)) {
    if (first > 0) {
      last = first - 1;
    } else {
      last = 0;
    }
  }
#endif
  if (last < first) {
    return M3_OK;
  }

  if (overscan > first) {
    first = 0;
  } else {
    first -= overscan;
  }

  if (last < item_count - 1) {
    m3_usize max_add = item_count - 1 - last;
    if (overscan > max_add) {
      last = item_count - 1;
    } else {
      last += overscan;
    }
  }

#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(
          M3_LIST_TEST_FAIL_VISIBLE_LAST_AFTER_OVERSCAN)) {
    if (first > 0) {
      last = first - 1;
    } else {
      last = 0;
    }
  }
#endif
  if (last < first) {
    return M3_OK;
  }

  *out_first = first;
  *out_last = last;
  *out_count = last - first + 1;
  return M3_OK;
}

static int m3_grid_compute_visible_range(const M3GridView *view,
                                         m3_usize *out_first,
                                         m3_usize *out_last,
                                         m3_usize *out_count) {
  M3Scalar item_main;
  M3Scalar spacing_main;
  M3Scalar padding_main;
  M3Scalar viewport;
  M3Scalar stride;
  M3Scalar start;
  M3Scalar end;
  m3_usize line_count;
  m3_usize first_line;
  m3_usize last_line;
  m3_usize first_index;
  m3_usize last_index = 0u;

  if (view == NULL || out_first == NULL || out_last == NULL ||
      out_count == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_first = M3_LIST_INVALID_INDEX;
  *out_last = M3_LIST_INVALID_INDEX;
  *out_count = 0;

  if (view->item_count == 0) {
    return M3_OK;
  }

  if (view->style.scroll_axis == M3_GRID_SCROLL_VERTICAL) {
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
    return M3_ERR_RANGE;
  }
  if (viewport <= 0.0f) {
    return M3_OK;
  }

  line_count = view->item_count / view->style.span;
  if (view->item_count % view->style.span != 0) {
    line_count += 1;
  }
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_GRID_LINE_COUNT_ZERO)) {
    line_count = 0;
  }
#endif
  if (line_count == 0) {
    return M3_OK;
  }

  stride = item_main + spacing_main;
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_GRID_STRIDE)) {
    stride = 0.0f;
  }
#endif
  if (stride <= 0.0f) {
    return M3_ERR_RANGE;
  }

  if (view->scroll_offset + viewport <= padding_main) {
    return M3_OK;
  }

  start = view->scroll_offset - padding_main;
  if (start <= 0.0f) {
    first_line = 0;
  } else {
    first_line = (m3_usize)(start / stride);
  }

  end = view->scroll_offset + viewport - padding_main;
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_GRID_END)) {
    end = 0.0f;
  }
#endif
  if (end <= 0.0f) {
    return M3_OK;
  }
  end -= 0.0001f;
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_GRID_END_NEGATIVE)) {
    end = -1.0f;
  }
#endif
  if (end < 0.0f) {
    end = 0.0f;
  }
  last_line = (m3_usize)(end / stride);

  if (first_line >= line_count) {
    return M3_OK;
  }
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_GRID_LAST_TOO_LARGE)) {
    last_line = line_count;
  }
#endif
  if (last_line >= line_count) {
    last_line = line_count - 1;
  }
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_GRID_LAST_BEFORE_FIRST)) {
    if (first_line > 0) {
      last_line = first_line - 1;
    } else {
      last_line = 0;
    }
  }
#endif
  if (last_line < first_line) {
    return M3_OK;
  }

  if (view->style.overscan > first_line) {
    first_line = 0;
  } else {
    first_line -= view->style.overscan;
  }

  if (last_line < line_count - 1) {
    m3_usize max_add = line_count - 1 - last_line;
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
    return M3_OK;
  }

  *out_first = first_index;
  *out_last = last_index;
  *out_count = last_index - first_index + 1;
  return M3_OK;
}

static int m3_list_compute_item_bounds(const M3ListView *view, m3_usize index,
                                       M3Rect *out_bounds) {
  M3Scalar x;
  M3Scalar y;
  M3Scalar width;
  M3Scalar height;
  M3Scalar stride;

  if (view == NULL || out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (index >= view->item_count) {
    return M3_ERR_NOT_FOUND;
  }

  stride = view->style.item_extent + view->style.spacing;
  if (stride <= 0.0f) {
    return M3_ERR_RANGE;
  }

  if (view->style.orientation == M3_LIST_ORIENTATION_VERTICAL) {
    width = view->bounds.width - view->style.padding.left -
            view->style.padding.right;
    height = view->style.item_extent;
    x = view->bounds.x + view->style.padding.left;
    y = view->bounds.y + view->style.padding.top - view->scroll_offset +
        (M3Scalar)index * stride;
  } else {
    width = view->style.item_extent;
    height = view->bounds.height - view->style.padding.top -
             view->style.padding.bottom;
    x = view->bounds.x + view->style.padding.left - view->scroll_offset +
        (M3Scalar)index * stride;
    y = view->bounds.y + view->style.padding.top;
  }

  if (width < 0.0f || height < 0.0f) {
    return M3_ERR_RANGE;
  }

  out_bounds->x = x;
  out_bounds->y = y;
  out_bounds->width = width;
  out_bounds->height = height;
  return M3_OK;
}

static int m3_grid_compute_item_bounds(const M3GridView *view, m3_usize index,
                                       M3Rect *out_bounds) {
  m3_usize row;
  m3_usize col;
  M3Scalar x;
  M3Scalar y;

  if (view == NULL || out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (index >= view->item_count) {
    return M3_ERR_NOT_FOUND;
  }

  if (view->style.item_width <= 0.0f || view->style.item_height <= 0.0f) {
    return M3_ERR_RANGE;
  }

  if (view->style.scroll_axis == M3_GRID_SCROLL_VERTICAL) {
    row = index / view->style.span;
    col = index % view->style.span;
    x = view->bounds.x + view->style.padding.left +
        (M3Scalar)col * (view->style.item_width + view->style.spacing_x);
    y = view->bounds.y + view->style.padding.top +
        (M3Scalar)row * (view->style.item_height + view->style.spacing_y) -
        view->scroll_offset;
  } else {
    row = index % view->style.span;
    col = index / view->style.span;
    x = view->bounds.x + view->style.padding.left +
        (M3Scalar)col * (view->style.item_width + view->style.spacing_x) -
        view->scroll_offset;
    y = view->bounds.y + view->style.padding.top +
        (M3Scalar)row * (view->style.item_height + view->style.spacing_y);
  }

  out_bounds->x = x;
  out_bounds->y = y;
  out_bounds->width = view->style.item_width;
  out_bounds->height = view->style.item_height;
  return M3_OK;
}

static int m3_list_view_update_metrics(M3ListView *view) {
  M3Scalar padding_start;
  M3Scalar padding_end;
  M3Scalar viewport;
  M3Scalar max_scroll;
  int rc;

  if (view == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (view->style.orientation == M3_LIST_ORIENTATION_VERTICAL) {
    padding_start = view->style.padding.top;
    padding_end = view->style.padding.bottom;
    viewport = view->bounds.height;
  } else {
    padding_start = view->style.padding.left;
    padding_end = view->style.padding.right;
    viewport = view->bounds.width;
  }

  rc = m3_list_compute_content_extent(view->item_count, view->style.item_extent,
                                      view->style.spacing, padding_start,
                                      padding_end, &view->content_extent);
  if (rc != M3_OK) {
    return rc;
  }

  if (viewport < 0.0f) {
    return M3_ERR_RANGE;
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
  return M3_OK;
}

static int m3_grid_view_update_metrics(M3GridView *view) {
  M3Scalar padding_start;
  M3Scalar padding_end;
  M3Scalar spacing_main;
  M3Scalar item_main;
  M3Scalar viewport;
  M3Scalar max_scroll;
  m3_usize line_count;
  int rc;

  if (view == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (view->style.scroll_axis == M3_GRID_SCROLL_VERTICAL) {
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
    return M3_ERR_RANGE;
  }
  if (view->style.span == 0) {
    return M3_ERR_RANGE;
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
      return M3_ERR_RANGE;
    }
    view->content_extent = padding_start + padding_end +
                           item_main * (M3Scalar)line_count +
                           spacing_main * (M3Scalar)(line_count - 1);
  }
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(
          M3_LIST_TEST_FAIL_CONTENT_EXTENT_NEGATIVE)) {
    view->content_extent = -1.0f;
  }
#endif

  if (viewport < 0.0f) {
    return M3_ERR_RANGE;
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
  rc = M3_OK;
  return rc;
}

static int m3_list_reserve_slots(M3ListSlot **slots,
                                 M3RenderNode ***visible_nodes,
                                 m3_usize *slot_capacity,
                                 const M3Allocator *allocator,
                                 m3_usize capacity) { /* GCOVR_EXCL_LINE */
  M3ListSlot *new_slots;
  M3RenderNode **new_nodes;
  m3_usize slot_bytes;
  m3_usize node_bytes;
  m3_usize i;
  int rc;

  if (slots == NULL || visible_nodes == NULL || slot_capacity == NULL ||
      allocator == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (capacity <= *slot_capacity) {
    return M3_OK;
  }

  rc =
      m3_list_mul_overflow(capacity, (m3_usize)sizeof(M3ListSlot), &slot_bytes);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_list_mul_overflow(capacity, (m3_usize)sizeof(M3RenderNode *),
                            &node_bytes);
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_RESERVE_NODE_BYTES)) {
    rc = M3_ERR_OVERFLOW;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  rc = allocator->alloc(allocator->ctx, slot_bytes, (void **)&new_slots);
  if (rc != M3_OK) {
    return rc;
  }

  rc = allocator->alloc(allocator->ctx, node_bytes, (void **)&new_nodes);
  if (rc != M3_OK) {
    allocator->free(allocator->ctx, new_slots);
    return rc;
  }

  if (*slots != NULL && *slot_capacity > 0) {
    m3_usize copy_bytes;
    rc = m3_list_mul_overflow(*slot_capacity, (m3_usize)sizeof(M3ListSlot),
                              &copy_bytes);
#ifdef M3_TESTING
    if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_RESERVE_COPY_BYTES)) {
      rc = M3_ERR_OVERFLOW;
    }
#endif
    if (rc != M3_OK) {
      allocator->free(allocator->ctx, new_nodes);
      allocator->free(allocator->ctx, new_slots);
      return rc;
    }
    memcpy(new_slots, *slots, (size_t)copy_bytes);
  }

  for (i = *slot_capacity; i < capacity; ++i) {
    memset(&new_slots[i], 0, sizeof(new_slots[i]));
    new_slots[i].index = M3_LIST_INVALID_INDEX;
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
  return M3_OK;
}

static int m3_list_widget_measure(void *widget, M3MeasureSpec width,
                                  M3MeasureSpec height,
                                  M3Size *out_size) { /* GCOVR_EXCL_LINE */
  M3ListView *view = NULL;
  M3Scalar content_width = 0.0f;
  M3Scalar content_height = 0.0f;
  int rc = M3_OK;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_list_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_list_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc;
  }

  view = (M3ListView *)widget;
  rc = m3_list_validate_style(&view->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_list_view_update_metrics(view);
  if (rc != M3_OK) {
    return rc;
  }

  if (view->style.orientation == M3_LIST_ORIENTATION_VERTICAL) {
    content_width = view->style.padding.left + view->style.padding.right;
    content_height = view->content_extent;
  } else {
    content_width = view->content_extent;
    content_height = view->style.padding.top + view->style.padding.bottom;
  }
#ifdef M3_TESTING /* GCOVR_EXCL_LINE */
  if (m3_list_test_fail_point_match(
          M3_LIST_TEST_FAIL_LIST_MEASURE_CONTENT_NEGATIVE)) {
    content_width = -1.0f;
  }
#endif /* GCOVR_EXCL_LINE */

  if (content_width < 0.0f || content_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (width.mode == M3_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == M3_MEASURE_AT_MOST) {
    out_size->width = (content_width > width.size) ? width.size : content_width;
  } else {
    out_size->width = content_width;
  }

  if (height.mode == M3_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == M3_MEASURE_AT_MOST) {
    out_size->height =
        (content_height > height.size) ? height.size : content_height;
  } else {
    out_size->height = content_height;
  }

  return M3_OK;
}

static int m3_list_widget_layout(void *widget, M3Rect bounds) {
  M3ListView *view;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_list_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  view = (M3ListView *)widget;
  view->bounds = bounds;
  return m3_list_view_update_metrics(view);
}

static int m3_list_widget_paint(void *widget, M3PaintContext *ctx) {
  M3ListView *view = NULL;
  int rc = M3_OK;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  view = (M3ListView *)widget;
  rc = m3_list_validate_style(&view->style);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_list_validate_rect(&view->bounds);
  if (rc != M3_OK) {
    return rc;
  }

  if (view->style.background_color.a <= 0.0f) {
    return M3_OK;
  }
  if (ctx->gfx->vtable == NULL || ctx->gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  return ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &view->bounds,
                                     view->style.background_color, 0.0f);
}

static int m3_list_widget_event(void *widget, const M3InputEvent *event,
                                M3Bool *out_handled) {
  M3ListView *view;
  M3Scalar delta; /* GCOVR_EXCL_LINE */
  int rc;         /* GCOVR_EXCL_LINE */

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;

  view = (M3ListView *)widget;
  if (view->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_OK;
  }

  if (event->type != M3_INPUT_POINTER_SCROLL) {
    return M3_OK;
  }

  if (view->style.orientation == M3_LIST_ORIENTATION_VERTICAL) {
    delta = (M3Scalar)event->data.pointer.scroll_y;
  } else {
    delta = (M3Scalar)event->data.pointer.scroll_x;
  }

  if (delta == 0.0f) {
    return M3_OK;
  }

  rc = m3_list_view_set_scroll(view, view->scroll_offset + delta);
  if (rc != M3_OK) {
    return rc;
  }

  *out_handled = M3_TRUE;
  return M3_OK;
}

static int
m3_list_widget_get_semantics(void *widget,
                             M3Semantics *out_semantics) { /* GCOVR_EXCL_LINE */
  M3ListView *view = NULL;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  view = (M3ListView *)widget;
  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (view->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_list_widget_destroy(void *widget) {
  M3ListView *view = NULL;
  int rc = M3_OK;
  int rc2 = M3_OK;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  view = (M3ListView *)widget;
  rc = M3_OK;

  if (view->slots != NULL && view->allocator.free != NULL) {
    rc2 = view->allocator.free(view->allocator.ctx, view->slots);
    if (rc2 != M3_OK && rc == M3_OK) {
      rc = rc2;
    }
  }
  if (view->visible_nodes != NULL && view->allocator.free != NULL) {
    rc2 = view->allocator.free(view->allocator.ctx, view->visible_nodes);
    if (rc2 != M3_OK && rc == M3_OK) {
      rc = rc2;
    }
  }

  view->slots = NULL;
  view->visible_nodes = NULL;
  view->slot_capacity = 0u;
  view->visible_count = 0u;
  view->visible_first = M3_LIST_INVALID_INDEX;
  view->visible_last = M3_LIST_INVALID_INDEX;
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

static const M3WidgetVTable g_m3_list_widget_vtable =
    {/* GCOVR_EXCL_LINE */
     m3_list_widget_measure,
     m3_list_widget_layout, /* GCOVR_EXCL_LINE */
     m3_list_widget_paint,
     m3_list_widget_event, /* GCOVR_EXCL_LINE */
     m3_list_widget_get_semantics,
     m3_list_widget_destroy}; /* GCOVR_EXCL_LINE */

static int m3_grid_widget_measure(void *widget, M3MeasureSpec width,
                                  M3MeasureSpec height, M3Size *out_size) {
  M3GridView *view = NULL;
  M3Scalar content_width = 0.0f;
  M3Scalar content_height = 0.0f;
  int rc = M3_OK;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_list_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_list_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc;
  }

  view = (M3GridView *)widget;
  rc = m3_grid_validate_style(&view->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_grid_view_update_metrics(view);
  if (rc != M3_OK) {
    return rc;
  }

  if (view->style.scroll_axis == M3_GRID_SCROLL_VERTICAL) {
    content_width = view->style.padding.left + view->style.padding.right +
                    view->style.item_width * (M3Scalar)view->style.span +
                    view->style.spacing_x * (M3Scalar)(view->style.span - 1);
    content_height = view->content_extent;
  } else {
    content_width = view->content_extent;
    content_height = view->style.padding.top + view->style.padding.bottom +
                     view->style.item_height * (M3Scalar)view->style.span +
                     view->style.spacing_y * (M3Scalar)(view->style.span - 1);
  }
#ifdef M3_TESTING
  if (m3_list_test_fail_point_match(
          M3_LIST_TEST_FAIL_GRID_MEASURE_CONTENT_NEGATIVE)) {
    content_width = -1.0f;
  }
#endif

  if (content_width < 0.0f || content_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (width.mode == M3_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == M3_MEASURE_AT_MOST) {
    out_size->width = (content_width > width.size) ? width.size : content_width;
  } else {
    out_size->width = content_width;
  }

  if (height.mode == M3_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == M3_MEASURE_AT_MOST) {
    out_size->height =
        (content_height > height.size) ? height.size : content_height;
  } else {
    out_size->height = content_height;
  }

  return M3_OK;
}

static int m3_grid_widget_layout(void *widget, M3Rect bounds) {
  M3GridView *view; /* GCOVR_EXCL_LINE */
  int rc;           /* GCOVR_EXCL_LINE */

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_list_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  view = (M3GridView *)widget;
  view->bounds = bounds;
  return m3_grid_view_update_metrics(view);
}

static int m3_grid_widget_paint(void *widget, M3PaintContext *ctx) {
  M3GridView *view = NULL;
  int rc = M3_OK;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  view = (M3GridView *)widget;
  rc = m3_grid_validate_style(&view->style);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_list_validate_rect(&view->bounds);
  if (rc != M3_OK) {
    return rc;
  }

  if (view->style.background_color.a <= 0.0f) {
    return M3_OK;
  }
  if (ctx->gfx->vtable == NULL || ctx->gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  return ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &view->bounds,
                                     view->style.background_color, 0.0f);
}

static int m3_grid_widget_event(void *widget, const M3InputEvent *event,
                                M3Bool *out_handled) {
  M3GridView *view = NULL;
  M3Scalar delta = 0.0f;
  int rc = M3_OK;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;

  view = (M3GridView *)widget;
  if (view->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_OK;
  }

  if (event->type != M3_INPUT_POINTER_SCROLL) {
    return M3_OK;
  }

  if (view->style.scroll_axis == M3_GRID_SCROLL_VERTICAL) {
    delta = (M3Scalar)event->data.pointer.scroll_y;
  } else {
    delta = (M3Scalar)event->data.pointer.scroll_x;
  }

  if (delta == 0.0f) {
    return M3_OK;
  }

  rc = m3_grid_view_set_scroll(view, view->scroll_offset + delta);
  if (rc != M3_OK) {
    return rc;
  }

  *out_handled = M3_TRUE;
  return M3_OK;
}

static int
m3_grid_widget_get_semantics(void *widget,
                             M3Semantics *out_semantics) { /* GCOVR_EXCL_LINE */
  M3GridView *view;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  view = (M3GridView *)widget;
  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (view->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_grid_widget_destroy(void *widget) {
  M3GridView *view = NULL;
  int rc = M3_OK;
  int rc2 = M3_OK;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  view = (M3GridView *)widget;
  rc = M3_OK;

  if (view->slots != NULL && view->allocator.free != NULL) {
    rc2 = view->allocator.free(view->allocator.ctx, view->slots);
    if (rc2 != M3_OK && rc == M3_OK) {
      rc = rc2;
    }
  }
  if (view->visible_nodes != NULL && view->allocator.free != NULL) {
    rc2 = view->allocator.free(view->allocator.ctx, view->visible_nodes);
    if (rc2 != M3_OK && rc == M3_OK) {
      rc = rc2;
    }
  }

  view->slots = NULL;
  view->visible_nodes = NULL;
  view->slot_capacity = 0u;
  view->visible_count = 0u;
  view->visible_first = M3_LIST_INVALID_INDEX;
  view->visible_last = M3_LIST_INVALID_INDEX;
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

static const M3WidgetVTable g_m3_grid_widget_vtable = {
    m3_grid_widget_measure,       m3_grid_widget_layout,
    m3_grid_widget_paint,         m3_grid_widget_event,
    m3_grid_widget_get_semantics, m3_grid_widget_destroy};

int M3_CALL m3_list_style_init(M3ListStyle *style) {
  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  style->orientation = M3_LIST_ORIENTATION_VERTICAL;
  style->padding.left = 0.0f;
  style->padding.top = 0.0f;
  style->padding.right = 0.0f;
  style->padding.bottom = 0.0f;
  style->spacing = M3_LIST_DEFAULT_SPACING;
  style->item_extent = M3_LIST_DEFAULT_ITEM_EXTENT;
  style->background_color.r = 0.0f;
  style->background_color.g = 0.0f;
  style->background_color.b = 0.0f;
  style->background_color.a = 0.0f;
  style->overscan = M3_LIST_DEFAULT_OVERSCAN;
  return M3_OK;
}

int M3_CALL m3_list_view_init(M3ListView *view, const M3ListStyle *style,
                              const M3Allocator *allocator, m3_usize item_count,
                              m3_usize slot_capacity) {
  M3Allocator alloc;
  int rc;

  if (view == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_list_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  if (allocator == NULL) {
    rc = m3_get_default_allocator(&alloc);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    alloc = *allocator;
  }

  if (alloc.alloc == NULL || alloc.realloc == NULL || alloc.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(view, 0, sizeof(*view));
  view->style = *style;
  view->allocator = alloc;
  view->item_count = item_count;
  view->visible_first = M3_LIST_INVALID_INDEX;
  view->visible_last = M3_LIST_INVALID_INDEX;
  view->widget.ctx = view;
  view->widget.vtable = &g_m3_list_widget_vtable;
  view->widget.handle.id = 0u;
  view->widget.handle.generation = 0u;
  view->widget.flags = 0u;

  if (slot_capacity > 0) {
    rc = m3_list_view_reserve(view, slot_capacity);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

int M3_CALL m3_list_view_set_bind(M3ListView *view, M3ListBindFn bind,
                                  void *ctx) {
  if (view == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  view->bind = bind;
  view->bind_ctx = ctx;
  return M3_OK;
}

int M3_CALL m3_list_view_set_item_count(M3ListView *view, m3_usize item_count) {
  int rc;

  if (view == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  view->item_count = item_count;
  rc = m3_list_view_update_metrics(view);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

int M3_CALL m3_list_view_set_style(M3ListView *view, const M3ListStyle *style) {
  int rc = M3_OK;

  if (view == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_list_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  view->style = *style;
  rc = m3_list_view_update_metrics(view);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

int M3_CALL m3_list_view_set_scroll(M3ListView *view, M3Scalar offset) {
  int rc;

  if (view == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (offset < 0.0f) {
    offset = 0.0f;
  }

  view->scroll_offset = offset;
  rc = m3_list_view_update_metrics(view);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

int M3_CALL m3_list_view_get_scroll(const M3ListView *view,
                                    M3Scalar *out_offset) {
  if (view == NULL || out_offset == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_offset = view->scroll_offset;
  return M3_OK;
}

int M3_CALL m3_list_view_get_content_extent(const M3ListView *view,
                                            M3Scalar *out_extent) {
  M3ListView temp = {0};
  int rc = M3_OK;

  if (view == NULL || out_extent == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  temp = *view;
  rc = m3_list_view_update_metrics(&temp);
  if (rc != M3_OK) {
    return rc;
  }

  *out_extent = temp.content_extent;
  return M3_OK;
}

int M3_CALL m3_list_view_get_required_slots(const M3ListView *view,
                                            m3_usize *out_required) {
  M3ListView temp; /* GCOVR_EXCL_LINE */
  M3Scalar padding_start;
  M3Scalar viewport;
  m3_usize first;
  m3_usize last;
  m3_usize count;
  int rc;

  if (view == NULL || out_required == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_list_validate_style(&view->style);
  if (rc != M3_OK) {
    return rc;
  }

  temp = *view;
  rc = m3_list_view_update_metrics(&temp);
  if (rc != M3_OK) {
    return rc;
  }

  if (temp.style.orientation == M3_LIST_ORIENTATION_VERTICAL) {
    padding_start = temp.style.padding.top;
    viewport = temp.bounds.height;
  } else {
    padding_start = temp.style.padding.left;
    viewport = temp.bounds.width;
  }

  rc = m3_list_compute_visible_range(
      temp.item_count, temp.style.item_extent, temp.style.spacing,
      padding_start, temp.scroll_offset, viewport, temp.style.overscan, &first,
      &last, &count);
  if (rc != M3_OK) {
    return rc;
  }

  *out_required = count;
  return M3_OK;
}

int M3_CALL m3_list_view_reserve(M3ListView *view, m3_usize capacity) {
  int rc;

  if (view == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_list_reserve_slots(&view->slots, &view->visible_nodes,
                             &view->slot_capacity, &view->allocator, capacity);
  if (rc != M3_OK) {
    return rc;
  }

  if (capacity > 0 && view->visible_nodes != NULL) {
    memset(view->visible_nodes, 0, (size_t)(capacity * sizeof(M3RenderNode *)));
  }
  view->visible_count = 0u;
  view->visible_first = M3_LIST_INVALID_INDEX;
  view->visible_last = M3_LIST_INVALID_INDEX;
  return M3_OK;
}

int M3_CALL m3_list_view_update(M3ListView *view) {
  M3Scalar padding_start;
  M3Scalar viewport;
  m3_usize first;
  m3_usize last;
  m3_usize count = 0u;
  m3_usize i;
  m3_usize index;
  M3Rect bounds;
  int rc;

  if (view == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_list_validate_style(&view->style);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_list_validate_rect(&view->bounds);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_list_view_update_metrics(view);
  if (rc != M3_OK) {
    return rc;
  }

  if (view->style.orientation == M3_LIST_ORIENTATION_VERTICAL) {
    padding_start = view->style.padding.top;
    viewport = view->bounds.height;
  } else {
    padding_start = view->style.padding.left;
    viewport = view->bounds.width;
  }

  rc = m3_list_compute_visible_range(
      view->item_count, view->style.item_extent, view->style.spacing,
      padding_start, view->scroll_offset, viewport, view->style.overscan,
      &first, &last, &count);
  if (rc != M3_OK) {
    return rc;
  }

  view->visible_count = 0u;
  view->visible_first = M3_LIST_INVALID_INDEX;
  view->visible_last = M3_LIST_INVALID_INDEX;

  if (count == 0) {
    if (view->visible_nodes != NULL && view->slot_capacity > 0) {
      memset(view->visible_nodes, 0,
             (size_t)(view->slot_capacity * sizeof(M3RenderNode *)));
    }
    return M3_OK;
  }

  if (view->slots == NULL || view->visible_nodes == NULL) {
    return M3_ERR_STATE;
  }
  if (view->slot_capacity < count) {
    return M3_ERR_RANGE;
  }
  if (view->bind == NULL) {
    return M3_ERR_STATE;
  }

  view->visible_first = first;
  view->visible_last = last;

  index = first;
  for (i = 0; i < count; ++i) {
    M3ListSlot *slot = &view->slots[i];

    slot->index = index;
    rc = view->bind(view->bind_ctx, slot, index);
    if (rc != M3_OK) {
      return rc;
    }
    if (slot->node.widget == NULL || slot->node.widget->vtable == NULL) {
      return M3_ERR_STATE;
    }

    rc = m3_list_compute_item_bounds(view, index, &bounds);
#ifdef M3_TESTING
    if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_LIST_ITEM_BOUNDS)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    rc = m3_render_node_set_bounds(&slot->node, &bounds);
#ifdef M3_TESTING
    if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_LIST_RENDER_BOUNDS)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }

    view->visible_nodes[i] = &slot->node;
    view->visible_count += 1;
    index += 1;
  }

  for (i = count; i < view->slot_capacity; ++i) {
    view->slots[i].index = M3_LIST_INVALID_INDEX;
    view->visible_nodes[i] = NULL;
  }

  return M3_OK;
}

int M3_CALL m3_list_view_get_visible(const M3ListView *view,
                                     M3RenderNode ***out_nodes,
                                     m3_usize *out_count) {
  if (view == NULL || out_nodes == NULL || out_count == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_nodes = view->visible_nodes;
  *out_count = view->visible_count;
  return M3_OK;
}

int M3_CALL m3_grid_style_init(M3GridStyle *style) {
  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  style->scroll_axis = M3_GRID_SCROLL_VERTICAL;
  style->span = M3_GRID_DEFAULT_SPAN;
  style->padding.left = 0.0f;
  style->padding.top = 0.0f;
  style->padding.right = 0.0f;
  style->padding.bottom = 0.0f;
  style->spacing_x = M3_GRID_DEFAULT_SPACING;
  style->spacing_y = M3_GRID_DEFAULT_SPACING;
  style->item_width = M3_GRID_DEFAULT_ITEM_WIDTH;
  style->item_height = M3_GRID_DEFAULT_ITEM_HEIGHT;
  style->background_color.r = 0.0f;
  style->background_color.g = 0.0f;
  style->background_color.b = 0.0f;
  style->background_color.a = 0.0f;
  style->overscan = M3_GRID_DEFAULT_OVERSCAN;
  return M3_OK;
}

int M3_CALL m3_grid_view_init(M3GridView *view, const M3GridStyle *style,
                              const M3Allocator *allocator, m3_usize item_count,
                              m3_usize slot_capacity) {
  M3Allocator alloc;
  int rc;

  if (view == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_grid_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  if (allocator == NULL) {
    rc = m3_get_default_allocator(&alloc);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    alloc = *allocator;
  }

  if (alloc.alloc == NULL || alloc.realloc == NULL || alloc.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(view, 0, sizeof(*view));
  view->style = *style;
  view->allocator = alloc;
  view->item_count = item_count;
  view->visible_first = M3_LIST_INVALID_INDEX;
  view->visible_last = M3_LIST_INVALID_INDEX;
  view->widget.ctx = view;
  view->widget.vtable = &g_m3_grid_widget_vtable;
  view->widget.handle.id = 0u;
  view->widget.handle.generation = 0u;
  view->widget.flags = 0u;

  if (slot_capacity > 0) {
    rc = m3_grid_view_reserve(view, slot_capacity);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

int M3_CALL m3_grid_view_set_bind(M3GridView *view, M3ListBindFn bind,
                                  void *ctx) {
  if (view == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  view->bind = bind;
  view->bind_ctx = ctx;
  return M3_OK;
}

int M3_CALL m3_grid_view_set_item_count(M3GridView *view, m3_usize item_count) {
  int rc;

  if (view == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  view->item_count = item_count;
  rc = m3_grid_view_update_metrics(view);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

int M3_CALL m3_grid_view_set_style(M3GridView *view, const M3GridStyle *style) {
  int rc;

  if (view == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_grid_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  view->style = *style;
  rc = m3_grid_view_update_metrics(view);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

int M3_CALL m3_grid_view_set_scroll(M3GridView *view, M3Scalar offset) {
  int rc = M3_OK;

  if (view == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (offset < 0.0f) {
    offset = 0.0f;
  }

  view->scroll_offset = offset;
  rc = m3_grid_view_update_metrics(view);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

int M3_CALL m3_grid_view_get_scroll(const M3GridView *view,
                                    M3Scalar *out_offset) {
  if (view == NULL || out_offset == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_offset = view->scroll_offset;
  return M3_OK;
}

int M3_CALL m3_grid_view_get_content_extent(const M3GridView *view,
                                            M3Scalar *out_extent) {
  M3GridView temp;
  int rc;

  if (view == NULL || out_extent == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  temp = *view;
  rc = m3_grid_view_update_metrics(&temp);
  if (rc != M3_OK) {
    return rc;
  }

  *out_extent = temp.content_extent;
  return M3_OK;
}

int M3_CALL m3_grid_view_get_required_slots(const M3GridView *view,
                                            m3_usize *out_required) {
  M3GridView temp;
  m3_usize first;
  m3_usize last;
  m3_usize count;
  int rc;

  if (view == NULL || out_required == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_grid_validate_style(&view->style);
  if (rc != M3_OK) {
    return rc;
  }

  temp = *view;
  rc = m3_grid_view_update_metrics(&temp);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_grid_compute_visible_range(&temp, &first, &last, &count);
  if (rc != M3_OK) {
    return rc;
  }

  *out_required = count;
  return M3_OK;
}

int M3_CALL m3_grid_view_reserve(M3GridView *view, m3_usize capacity) {
  int rc; /* GCOVR_EXCL_LINE */

  if (view == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_list_reserve_slots(&view->slots, &view->visible_nodes,
                             &view->slot_capacity, &view->allocator, capacity);
  if (rc != M3_OK) {
    return rc;
  }

  if (capacity > 0 && view->visible_nodes != NULL) {
    memset(view->visible_nodes, 0, (size_t)(capacity * sizeof(M3RenderNode *)));
  }
  view->visible_count = 0u;
  view->visible_first = M3_LIST_INVALID_INDEX;
  view->visible_last = M3_LIST_INVALID_INDEX;
  return M3_OK;
}

int M3_CALL m3_grid_view_update(M3GridView *view) {
  m3_usize first;
  m3_usize last;
  m3_usize count = 0u;
  m3_usize i;
  m3_usize index;
  M3Rect bounds;
  int rc;

  if (view == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_grid_validate_style(&view->style);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_list_validate_rect(&view->bounds);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_grid_view_update_metrics(view);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_grid_compute_visible_range(view, &first, &last, &count);
  if (rc != M3_OK) {
    return rc;
  }

  view->visible_count = 0u;
  view->visible_first = M3_LIST_INVALID_INDEX;
  view->visible_last = M3_LIST_INVALID_INDEX;

  if (count == 0) {
    if (view->visible_nodes != NULL && view->slot_capacity > 0) {
      memset(view->visible_nodes, 0,
             (size_t)(view->slot_capacity * sizeof(M3RenderNode *)));
    }
    return M3_OK;
  }

  if (view->slots == NULL || view->visible_nodes == NULL) {
    return M3_ERR_STATE;
  }
  if (view->slot_capacity < count) {
    return M3_ERR_RANGE;
  }
  if (view->bind == NULL) {
    return M3_ERR_STATE;
  }

  view->visible_first = first;
  view->visible_last = last;

  index = first;
  for (i = 0; i < count; ++i) {
    M3ListSlot *slot = &view->slots[i];

    slot->index = index;
    rc = view->bind(view->bind_ctx, slot, index);
    if (rc != M3_OK) {
      return rc;
    }
    if (slot->node.widget == NULL || slot->node.widget->vtable == NULL) {
      return M3_ERR_STATE;
    }

    rc = m3_grid_compute_item_bounds(view, index, &bounds);
#ifdef M3_TESTING
    if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_GRID_ITEM_BOUNDS)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    rc = m3_render_node_set_bounds(&slot->node, &bounds);
#ifdef M3_TESTING
    if (m3_list_test_fail_point_match(M3_LIST_TEST_FAIL_GRID_RENDER_BOUNDS)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }

    view->visible_nodes[i] = &slot->node;
    view->visible_count += 1;
    index += 1;
  }

  for (i = count; i < view->slot_capacity; ++i) {
    view->slots[i].index = M3_LIST_INVALID_INDEX;
    view->visible_nodes[i] = NULL;
  }

  return M3_OK;
}

int M3_CALL m3_grid_view_get_visible(const M3GridView *view,
                                     M3RenderNode ***out_nodes,
                                     m3_usize *out_count) {
  if (view == NULL || out_nodes == NULL || out_count == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_nodes = view->visible_nodes;
  *out_count = view->visible_count;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_list_test_set_fail_point(m3_u32 fail_point) {
  g_m3_list_test_fail_point = fail_point;
  return M3_OK;
}

int M3_CALL m3_list_test_clear_fail_points(void) {
  g_m3_list_test_fail_point = M3_LIST_TEST_FAIL_NONE;
  return M3_OK;
}

int M3_CALL m3_list_test_mul_overflow(m3_usize a, m3_usize b,
                                      m3_usize *out_value) {
  return m3_list_mul_overflow(a, b, out_value);
}

int M3_CALL m3_list_test_validate_color(const M3Color *color) {
  return m3_list_validate_color(color);
}

int M3_CALL m3_list_test_validate_edges(const M3LayoutEdges *edges) {
  return m3_list_validate_edges(edges);
}

int M3_CALL m3_list_test_validate_rect(const M3Rect *rect) {
  return m3_list_validate_rect(rect);
}

int M3_CALL m3_list_test_validate_measure_spec(M3MeasureSpec spec) {
  return m3_list_validate_measure_spec(spec);
}

int M3_CALL m3_list_test_validate_style(const M3ListStyle *style) {
  return m3_list_validate_style(style);
}

int M3_CALL m3_list_test_validate_grid_style(const M3GridStyle *style) {
  return m3_grid_validate_style(style);
}

int M3_CALL m3_list_test_compute_content_extent(
    m3_usize item_count, M3Scalar item_extent, M3Scalar spacing,
    M3Scalar padding_start, M3Scalar padding_end, M3Scalar *out_extent) {
  return m3_list_compute_content_extent(item_count, item_extent, spacing,
                                        padding_start, padding_end, out_extent);
}

int M3_CALL m3_list_test_compute_visible_range(
    m3_usize item_count, M3Scalar item_extent, M3Scalar spacing,
    M3Scalar padding_start, M3Scalar scroll, M3Scalar viewport,
    m3_usize overscan, m3_usize *out_first, m3_usize *out_last,
    m3_usize *out_count) {
  return m3_list_compute_visible_range(
      item_count, item_extent, spacing, padding_start, scroll, viewport,
      overscan, out_first, out_last, out_count);
}

int M3_CALL m3_list_test_grid_compute_visible_range(const M3GridView *view,
                                                    m3_usize *out_first,
                                                    m3_usize *out_last,
                                                    m3_usize *out_count) {
  return m3_grid_compute_visible_range(view, out_first, out_last, out_count);
}

int M3_CALL m3_list_test_compute_item_bounds(const M3ListView *view,
                                             m3_usize index,
                                             M3Rect *out_bounds) {
  return m3_list_compute_item_bounds(view, index, out_bounds);
}

int M3_CALL m3_list_test_grid_compute_item_bounds(const M3GridView *view,
                                                  m3_usize index,
                                                  M3Rect *out_bounds) {
  return m3_grid_compute_item_bounds(view, index, out_bounds);
}

int M3_CALL m3_list_test_update_metrics(M3ListView *view) {
  return m3_list_view_update_metrics(view);
}

int M3_CALL m3_list_test_grid_update_metrics(M3GridView *view) {
  return m3_grid_view_update_metrics(view);
}

int M3_CALL m3_list_test_reserve_slots(M3ListSlot **slots,
                                       M3RenderNode ***visible_nodes,
                                       m3_usize *slot_capacity,
                                       const M3Allocator *allocator,
                                       m3_usize capacity) {
  return m3_list_reserve_slots(slots, visible_nodes, slot_capacity, allocator,
                               capacity);
}
#endif
