#include "m3/m3_layout.h"

#ifdef M3_TESTING
static M3Bool g_m3_layout_force_style_init_fail = M3_FALSE;
static M3Bool g_m3_layout_force_direction_fail = M3_FALSE;
#endif

static M3Scalar m3_layout_clamp_non_negative(M3Scalar value) {
  if (value < 0.0f) {
    return 0.0f;
  }
  return value;
}

static int m3_layout_validate_direction(const M3LayoutDirection *direction) {
  if (direction == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (g_m3_layout_force_direction_fail) {
    return M3_ERR_UNKNOWN;
  }
#endif

  if (direction->flow != M3_DIRECTION_LTR &&
      direction->flow != M3_DIRECTION_RTL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  return M3_OK;
}

static int m3_layout_resolve_horizontal_padding(
    const M3LayoutDirection *direction, const M3LayoutEdges *padding,
    M3Scalar *out_left, M3Scalar *out_right) {
  M3Scalar start;
  M3Scalar end;
  M3Bool is_rtl;
  int rc;

  if (padding == NULL || out_left == NULL || out_right == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_layout_validate_direction(direction);
  if (rc != M3_OK) {
    return rc;
  }

  is_rtl = (direction->flow == M3_DIRECTION_RTL) ? M3_TRUE : M3_FALSE;

  start = padding->left;
  end = padding->right;

  if (is_rtl) {
    *out_left = end;
    *out_right = start;
  } else {
    *out_left = start;
    *out_right = end;
  }

  return M3_OK;
}

static int m3_layout_validate_measure_spec(const M3LayoutMeasureSpec *spec) {
  if (spec == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  switch (spec->mode) {
  case M3_LAYOUT_MEASURE_UNSPECIFIED:
    return M3_OK;
  case M3_LAYOUT_MEASURE_EXACTLY:
  case M3_LAYOUT_MEASURE_AT_MOST:
    if (spec->size < 0.0f) {
      return M3_ERR_RANGE;
    }
    return M3_OK;
  default:
    return M3_ERR_INVALID_ARGUMENT;
  }
}

static int m3_layout_validate_style(const M3LayoutStyle *style) {
  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (style->direction != M3_LAYOUT_DIRECTION_ROW &&
      style->direction != M3_LAYOUT_DIRECTION_COLUMN) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (style->wrap != M3_LAYOUT_WRAP_NO && style->wrap != M3_LAYOUT_WRAP_YES) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  switch (style->align_main) {
  case M3_LAYOUT_ALIGN_START:
  case M3_LAYOUT_ALIGN_CENTER:
  case M3_LAYOUT_ALIGN_END:
  case M3_LAYOUT_ALIGN_SPACE_BETWEEN:
  case M3_LAYOUT_ALIGN_SPACE_AROUND:
  case M3_LAYOUT_ALIGN_SPACE_EVENLY:
    break;
  default:
    return M3_ERR_INVALID_ARGUMENT;
  }

  switch (style->align_cross) {
  case M3_LAYOUT_ALIGN_START:
  case M3_LAYOUT_ALIGN_CENTER:
  case M3_LAYOUT_ALIGN_END:
  case M3_LAYOUT_ALIGN_STRETCH:
    break;
  default:
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (style->padding.left < 0.0f || style->padding.top < 0.0f ||
      style->padding.right < 0.0f || style->padding.bottom < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (style->width < 0.0f && style->width != M3_LAYOUT_AUTO) {
    return M3_ERR_RANGE;
  }

  if (style->height < 0.0f && style->height != M3_LAYOUT_AUTO) {
    return M3_ERR_RANGE;
  }

  return M3_OK;
}

static int m3_layout_apply_spec(M3Scalar size, M3LayoutMeasureSpec spec,
                                M3Scalar *out_size) {
  if (out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  switch (spec.mode) {
  case M3_LAYOUT_MEASURE_UNSPECIFIED:
    *out_size = size;
    return M3_OK;
  case M3_LAYOUT_MEASURE_EXACTLY:
    if (spec.size < 0.0f) {
      return M3_ERR_RANGE;
    }
    *out_size = spec.size;
    return M3_OK;
  case M3_LAYOUT_MEASURE_AT_MOST:
    if (spec.size < 0.0f) {
      return M3_ERR_RANGE;
    }
    *out_size = (size > spec.size) ? spec.size : size;
    return M3_OK;
  default:
    return M3_ERR_INVALID_ARGUMENT;
  }
}

static M3Scalar m3_layout_resolve_available(M3Scalar style_size,
                                            M3LayoutMeasureSpec spec) {
  M3Scalar available;

  available = -1.0f;
  if (style_size >= 0.0f) {
    available = style_size;
  }

  if (spec.mode != M3_LAYOUT_MEASURE_UNSPECIFIED) {
    if (available < 0.0f) {
      available = spec.size;
    } else if (spec.size < available) {
      available = spec.size;
    }
  }

  return available;
}

static int m3_layout_measure_node(M3LayoutNode *node,
                                  const M3LayoutDirection *direction,
                                  M3LayoutMeasureSpec width,
                                  M3LayoutMeasureSpec height);

static int m3_layout_measure_leaf(M3LayoutNode *node, M3LayoutMeasureSpec width,
                                  M3LayoutMeasureSpec height,
                                  M3Size *out_size) {
  M3LayoutMeasureSpec measure_width;
  M3LayoutMeasureSpec measure_height;
  M3Size measured;
  M3Scalar forced_width;
  M3Scalar forced_height;
  int rc;

  if (node == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  forced_width = node->style.width;
  forced_height = node->style.height;

  measure_width = width;
  measure_height = height;

  if (forced_width >= 0.0f) {
    measure_width.mode = M3_LAYOUT_MEASURE_EXACTLY;
    measure_width.size = forced_width;
  }
  if (forced_height >= 0.0f) {
    measure_height.mode = M3_LAYOUT_MEASURE_EXACTLY;
    measure_height.size = forced_height;
  }

  if (node->measure != NULL) {
    rc = node->measure(node->measure_ctx, measure_width, measure_height,
                       &measured);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    measured.width = (forced_width >= 0.0f) ? forced_width : 0.0f;
    measured.height = (forced_height >= 0.0f) ? forced_height : 0.0f;
  }

  if (measured.width < 0.0f || measured.height < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (forced_width >= 0.0f) {
    measured.width = forced_width;
  }
  if (forced_height >= 0.0f) {
    measured.height = forced_height;
  }

  rc = m3_layout_apply_spec(measured.width, width, &measured.width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_layout_apply_spec(measured.height, height, &measured.height);
  if (rc != M3_OK) {
    return rc;
  }

  *out_size = measured;
  return M3_OK;
}

static int m3_layout_measure_row(M3LayoutNode *node,
                                 const M3LayoutDirection *direction,
                                 M3LayoutMeasureSpec width,
                                 M3LayoutMeasureSpec height,
                                 M3Size *out_size) {
  M3LayoutMeasureSpec child_width_spec;
  M3LayoutMeasureSpec child_height_spec;
  M3Scalar padding_lr;
  M3Scalar padding_tb;
  M3Scalar padding_left;
  M3Scalar padding_right;
  M3Scalar available_width;
  M3Scalar available_height;
  M3Scalar inner_width_limit;
  M3Scalar inner_height_limit;
  M3Scalar line_main;
  M3Scalar line_cross;
  M3Scalar content_main;
  M3Scalar content_cross;
  M3Scalar content_main_max;
  M3Scalar content_cross_sum;
  M3Scalar total_width;
  M3Scalar total_height;
  M3Bool wrap_enabled;
  m3_usize i;
  int rc;

  if (node == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_layout_resolve_horizontal_padding(direction, &node->style.padding,
                                            &padding_left, &padding_right);
  if (rc != M3_OK) {
    return rc;
  }

  padding_lr = padding_left + padding_right;
  padding_tb = node->style.padding.top + node->style.padding.bottom;

  available_width = m3_layout_resolve_available(node->style.width, width);
  available_height = m3_layout_resolve_available(node->style.height, height);

  if (available_width >= 0.0f) {
    inner_width_limit =
        m3_layout_clamp_non_negative(available_width - padding_lr);
  } else {
    inner_width_limit = -1.0f;
  }

  if (available_height >= 0.0f) {
    inner_height_limit =
        m3_layout_clamp_non_negative(available_height - padding_tb);
  } else {
    inner_height_limit = -1.0f;
  }

  if (inner_width_limit >= 0.0f) {
    child_width_spec.mode = M3_LAYOUT_MEASURE_AT_MOST;
    child_width_spec.size = inner_width_limit;
  } else {
    child_width_spec.mode = M3_LAYOUT_MEASURE_UNSPECIFIED;
    child_width_spec.size = 0.0f;
  }

  if (inner_height_limit >= 0.0f) {
    child_height_spec.mode = M3_LAYOUT_MEASURE_AT_MOST;
    child_height_spec.size = inner_height_limit;
  } else {
    child_height_spec.mode = M3_LAYOUT_MEASURE_UNSPECIFIED;
    child_height_spec.size = 0.0f;
  }

  wrap_enabled =
      (node->style.wrap == M3_LAYOUT_WRAP_YES && inner_width_limit >= 0.0f)
          ? M3_TRUE
          : M3_FALSE;

  line_main = 0.0f;
  line_cross = 0.0f;
  content_main_max = 0.0f;
  content_cross_sum = 0.0f;

  for (i = 0; i < node->child_count; ++i) {
    M3LayoutNode *child;
    M3Scalar child_main;
    M3Scalar child_cross;

    child = node->children[i];
    rc = m3_layout_measure_node(child, direction, child_width_spec,
                                child_height_spec);
    if (rc != M3_OK) {
      return rc;
    }

    child_main = child->measured.width;
    child_cross = child->measured.height;

    if (wrap_enabled && line_main > 0.0f &&
        line_main + child_main > inner_width_limit) {
      if (line_main > content_main_max) {
        content_main_max = line_main;
      }
      content_cross_sum += line_cross;
      line_main = 0.0f;
      line_cross = 0.0f;
    }

    line_main += child_main;
    if (child_cross > line_cross) {
      line_cross = child_cross;
    }
  }

  if (wrap_enabled) {
    if (line_main > content_main_max) {
      content_main_max = line_main;
    }
    content_cross_sum += line_cross;
    content_main = content_main_max;
    content_cross = content_cross_sum;
  } else {
    content_main = line_main;
    content_cross = line_cross;
  }

  total_width = content_main + padding_lr;
  total_height = content_cross + padding_tb;

  if (node->style.width >= 0.0f) {
    total_width = node->style.width;
  }
  if (node->style.height >= 0.0f) {
    total_height = node->style.height;
  }

  rc = m3_layout_apply_spec(total_width, width, &total_width);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_layout_apply_spec(total_height, height, &total_height);
  if (rc != M3_OK) {
    return rc;
  }

  out_size->width = total_width;
  out_size->height = total_height;
  return M3_OK;
}

static int m3_layout_measure_column(M3LayoutNode *node,
                                    const M3LayoutDirection *direction,
                                    M3LayoutMeasureSpec width,
                                    M3LayoutMeasureSpec height,
                                    M3Size *out_size) {
  M3LayoutMeasureSpec child_width_spec;
  M3LayoutMeasureSpec child_height_spec;
  M3Scalar padding_lr;
  M3Scalar padding_tb;
  M3Scalar padding_left;
  M3Scalar padding_right;
  M3Scalar available_width;
  M3Scalar available_height;
  M3Scalar inner_width_limit;
  M3Scalar inner_height_limit;
  M3Scalar line_main;
  M3Scalar line_cross;
  M3Scalar content_main;
  M3Scalar content_cross;
  M3Scalar content_main_max;
  M3Scalar content_cross_sum;
  M3Scalar total_width;
  M3Scalar total_height;
  M3Bool wrap_enabled;
  m3_usize i;
  int rc;

  if (node == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_layout_resolve_horizontal_padding(direction, &node->style.padding,
                                            &padding_left, &padding_right);
  if (rc != M3_OK) {
    return rc;
  }

  padding_lr = padding_left + padding_right;
  padding_tb = node->style.padding.top + node->style.padding.bottom;

  available_width = m3_layout_resolve_available(node->style.width, width);
  available_height = m3_layout_resolve_available(node->style.height, height);

  if (available_width >= 0.0f) {
    inner_width_limit =
        m3_layout_clamp_non_negative(available_width - padding_lr);
  } else {
    inner_width_limit = -1.0f;
  }

  if (available_height >= 0.0f) {
    inner_height_limit =
        m3_layout_clamp_non_negative(available_height - padding_tb);
  } else {
    inner_height_limit = -1.0f;
  }

  if (inner_width_limit >= 0.0f) {
    child_width_spec.mode = M3_LAYOUT_MEASURE_AT_MOST;
    child_width_spec.size = inner_width_limit;
  } else {
    child_width_spec.mode = M3_LAYOUT_MEASURE_UNSPECIFIED;
    child_width_spec.size = 0.0f;
  }

  if (inner_height_limit >= 0.0f) {
    child_height_spec.mode = M3_LAYOUT_MEASURE_AT_MOST;
    child_height_spec.size = inner_height_limit;
  } else {
    child_height_spec.mode = M3_LAYOUT_MEASURE_UNSPECIFIED;
    child_height_spec.size = 0.0f;
  }

  wrap_enabled =
      (node->style.wrap == M3_LAYOUT_WRAP_YES && inner_height_limit >= 0.0f)
          ? M3_TRUE
          : M3_FALSE;

  line_main = 0.0f;
  line_cross = 0.0f;
  content_main_max = 0.0f;
  content_cross_sum = 0.0f;

  for (i = 0; i < node->child_count; ++i) {
    M3LayoutNode *child;
    M3Scalar child_main;
    M3Scalar child_cross;

    child = node->children[i];
    rc = m3_layout_measure_node(child, direction, child_width_spec,
                                child_height_spec);
    if (rc != M3_OK) {
      return rc;
    }

    child_main = child->measured.height;
    child_cross = child->measured.width;

    if (wrap_enabled && line_main > 0.0f &&
        line_main + child_main > inner_height_limit) {
      if (line_main > content_main_max) {
        content_main_max = line_main;
      }
      content_cross_sum += line_cross;
      line_main = 0.0f;
      line_cross = 0.0f;
    }

    line_main += child_main;
    if (child_cross > line_cross) {
      line_cross = child_cross;
    }
  }

  if (wrap_enabled) {
    if (line_main > content_main_max) {
      content_main_max = line_main;
    }
    content_cross_sum += line_cross;
    content_main = content_main_max;
    content_cross = content_cross_sum;
  } else {
    content_main = line_main;
    content_cross = line_cross;
  }

  total_width = content_cross + padding_lr;
  total_height = content_main + padding_tb;

  if (node->style.width >= 0.0f) {
    total_width = node->style.width;
  }
  if (node->style.height >= 0.0f) {
    total_height = node->style.height;
  }

  rc = m3_layout_apply_spec(total_width, width, &total_width);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_layout_apply_spec(total_height, height, &total_height);
  if (rc != M3_OK) {
    return rc;
  }

  out_size->width = total_width;
  out_size->height = total_height;
  return M3_OK;
}

static int m3_layout_measure_node(M3LayoutNode *node,
                                  const M3LayoutDirection *direction,
                                  M3LayoutMeasureSpec width,
                                  M3LayoutMeasureSpec height) {
  M3Size measured;
  int rc;

  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_layout_validate_direction(direction);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_layout_validate_measure_spec(&width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_layout_validate_measure_spec(&height);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_layout_validate_style(&node->style);
  if (rc != M3_OK) {
    return rc;
  }

  if (node->child_count > 0 && node->children == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (node->child_count == 0) {
    rc = m3_layout_measure_leaf(node, width, height, &measured);
  } else {
    if (node->style.direction == M3_LAYOUT_DIRECTION_ROW) {
      rc = m3_layout_measure_row(node, direction, width, height, &measured);
    } else {
      rc = m3_layout_measure_column(node, direction, width, height, &measured);
    }
  }

  if (rc != M3_OK) {
    return rc;
  }

  node->measured = measured;
  return M3_OK;
}

static int m3_layout_layout_children_row(M3LayoutNode *node,
                                         const M3LayoutDirection *direction,
                                         M3Scalar origin_x, M3Scalar origin_y,
                                         M3Scalar width, M3Scalar height);
static int m3_layout_layout_children_column(M3LayoutNode *node,
                                            const M3LayoutDirection *direction,
                                            M3Scalar origin_x,
                                            M3Scalar origin_y, M3Scalar width,
                                            M3Scalar height);

static int m3_layout_layout_node(M3LayoutNode *node,
                                 const M3LayoutDirection *direction,
                                 M3Scalar origin_x, M3Scalar origin_y,
                                 M3Scalar width, M3Scalar height) {
  int rc;

  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_layout_validate_direction(direction);
  if (rc != M3_OK) {
    return rc;
  }

  node->layout.x = origin_x;
  node->layout.y = origin_y;
  node->layout.width = width;
  node->layout.height = height;

  if (node->child_count == 0) {
    return M3_OK;
  }

  if (node->children == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (node->style.direction == M3_LAYOUT_DIRECTION_ROW) {
    rc = m3_layout_layout_children_row(node, direction, origin_x, origin_y,
                                       width, height);
  } else if (node->style.direction == M3_LAYOUT_DIRECTION_COLUMN) {
    rc = m3_layout_layout_children_column(node, direction, origin_x, origin_y,
                                          width, height);
  } else {
    rc = M3_ERR_INVALID_ARGUMENT;
  }

  return rc;
}

static int m3_layout_layout_children_row(M3LayoutNode *node,
                                         const M3LayoutDirection *direction,
                                         M3Scalar origin_x, M3Scalar origin_y,
                                         M3Scalar width, M3Scalar height) {
  M3Scalar padding_left;
  M3Scalar padding_top;
  M3Scalar padding_right;
  M3Scalar padding_bottom;
  M3Scalar inner_width;
  M3Scalar inner_height;
  M3Bool wrap_enabled;
  m3_usize index;
  M3Scalar cursor_cross;
  M3Scalar main_start;
  M3Scalar main_sign;
  M3Bool is_rtl;
  int rc;

  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_layout_resolve_horizontal_padding(direction, &node->style.padding,
                                            &padding_left, &padding_right);
  if (rc != M3_OK) {
    return rc;
  }

  is_rtl = (direction->flow == M3_DIRECTION_RTL) ? M3_TRUE : M3_FALSE;
  padding_top = node->style.padding.top;
  padding_bottom = node->style.padding.bottom;

  inner_width = width - (padding_left + padding_right);
  inner_height = height - (padding_top + padding_bottom);

  inner_width = m3_layout_clamp_non_negative(inner_width);
  inner_height = m3_layout_clamp_non_negative(inner_height);

  wrap_enabled = (node->style.wrap == M3_LAYOUT_WRAP_YES && inner_width >= 0.0f)
                     ? M3_TRUE
                     : M3_FALSE;

  if (is_rtl) {
    main_start = origin_x + width - padding_right;
    main_sign = -1.0f;
  } else {
    main_start = origin_x + padding_left;
    main_sign = 1.0f;
  }

  index = 0;
  cursor_cross = 0.0f;
  rc = M3_OK;

  while (index < node->child_count) {
    M3Scalar line_main;
    M3Scalar line_cross;
    m3_usize line_count;
    m3_usize i;
    m3_usize end;
    M3Scalar remaining;
    M3Scalar offset;
    M3Scalar spacing;
    M3Scalar cursor_main;

    line_main = 0.0f;
    line_cross = 0.0f;
    line_count = 0;
    end = index;

    while (end < node->child_count) {
      M3LayoutNode *child;
      M3Scalar child_main;
      M3Scalar child_cross;

      child = node->children[end];
      child_main = child->measured.width;
      child_cross = child->measured.height;

      if (wrap_enabled && line_count > 0 &&
          line_main + child_main > inner_width) {
        break;
      }

      line_main += child_main;
      if (child_cross > line_cross) {
        line_cross = child_cross;
      }
      line_count += 1;
      end += 1;
    }

    if (node->style.wrap == M3_LAYOUT_WRAP_NO && inner_height > line_cross) {
      line_cross = inner_height;
    }

    remaining = inner_width - line_main;
    if (remaining < 0.0f) {
      remaining = 0.0f;
    }

    offset = 0.0f;
    spacing = 0.0f;

    switch (node->style.align_main) {
    case M3_LAYOUT_ALIGN_START:
      break;
    case M3_LAYOUT_ALIGN_CENTER:
      offset = remaining * 0.5f;
      break;
    case M3_LAYOUT_ALIGN_END:
      offset = remaining;
      break;
    case M3_LAYOUT_ALIGN_SPACE_BETWEEN:
      if (line_count > 1) {
        spacing = remaining / (M3Scalar)(line_count - 1);
      }
      break;
    case M3_LAYOUT_ALIGN_SPACE_AROUND:
      if (line_count > 0) {
        spacing = remaining / (M3Scalar)line_count;
        offset = spacing * 0.5f;
      }
      break;
    case M3_LAYOUT_ALIGN_SPACE_EVENLY:
      if (line_count > 0) {
        spacing = remaining / (M3Scalar)(line_count + 1);
        offset = spacing;
      }
      break;
    default:
      return M3_ERR_INVALID_ARGUMENT;
    }

    cursor_main = offset;
    for (i = 0; i < line_count; ++i) {
      M3LayoutNode *child;
      M3Scalar child_main;
      M3Scalar child_cross;
      M3Scalar cross_offset;
      M3Scalar child_x;
      M3Scalar child_y;

      child = node->children[index + i];
      child_main = child->measured.width;
      child_cross = child->measured.height;

      cross_offset = 0.0f;
      switch (node->style.align_cross) {
      case M3_LAYOUT_ALIGN_START:
        break;
      case M3_LAYOUT_ALIGN_CENTER:
        cross_offset = (line_cross - child_cross) * 0.5f;
        break;
      case M3_LAYOUT_ALIGN_END:
        cross_offset = line_cross - child_cross;
        break;
      case M3_LAYOUT_ALIGN_STRETCH:
        child_cross = line_cross;
        break;
      default:
        return M3_ERR_INVALID_ARGUMENT;
      }

      if (main_sign > 0.0f) {
        child_x = main_start + cursor_main;
      } else {
        child_x = main_start - cursor_main - child_main;
      }
      child_y = origin_y + padding_top + cursor_cross + cross_offset;

      rc = m3_layout_layout_node(child, direction, child_x, child_y,
                                 child_main, child_cross);
      if (rc != M3_OK) {
        return rc;
      }

      cursor_main += child_main + spacing;
    }

    cursor_cross += line_cross;
    index = end;
  }

  return M3_OK;
}

static int m3_layout_layout_children_column(M3LayoutNode *node,
                                            const M3LayoutDirection *direction,
                                            M3Scalar origin_x,
                                            M3Scalar origin_y, M3Scalar width,
                                            M3Scalar height) {
  M3Scalar padding_left;
  M3Scalar padding_top;
  M3Scalar padding_right;
  M3Scalar padding_bottom;
  M3Scalar inner_width;
  M3Scalar inner_height;
  M3Bool wrap_enabled;
  m3_usize index;
  M3Scalar cursor_cross;
  M3Scalar cross_start;
  M3Scalar cross_sign;
  M3Bool is_rtl;
  int rc;

  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_layout_resolve_horizontal_padding(direction, &node->style.padding,
                                            &padding_left, &padding_right);
  if (rc != M3_OK) {
    return rc;
  }

  is_rtl = (direction->flow == M3_DIRECTION_RTL) ? M3_TRUE : M3_FALSE;
  padding_top = node->style.padding.top;
  padding_bottom = node->style.padding.bottom;

  inner_width = width - (padding_left + padding_right);
  inner_height = height - (padding_top + padding_bottom);

  inner_width = m3_layout_clamp_non_negative(inner_width);
  inner_height = m3_layout_clamp_non_negative(inner_height);

  wrap_enabled =
      (node->style.wrap == M3_LAYOUT_WRAP_YES && inner_height >= 0.0f)
          ? M3_TRUE
          : M3_FALSE;

  if (is_rtl) {
    cross_start = origin_x + width - padding_right;
    cross_sign = -1.0f;
  } else {
    cross_start = origin_x + padding_left;
    cross_sign = 1.0f;
  }

  index = 0;
  cursor_cross = 0.0f;
  rc = M3_OK;

  while (index < node->child_count) {
    M3Scalar line_main;
    M3Scalar line_cross;
    m3_usize line_count;
    m3_usize i;
    m3_usize end;
    M3Scalar remaining;
    M3Scalar offset;
    M3Scalar spacing;
    M3Scalar cursor_main;

    line_main = 0.0f;
    line_cross = 0.0f;
    line_count = 0;
    end = index;

    while (end < node->child_count) {
      M3LayoutNode *child;
      M3Scalar child_main;
      M3Scalar child_cross;

      child = node->children[end];
      child_main = child->measured.height;
      child_cross = child->measured.width;

      if (wrap_enabled && line_count > 0 &&
          line_main + child_main > inner_height) {
        break;
      }

      line_main += child_main;
      if (child_cross > line_cross) {
        line_cross = child_cross;
      }
      line_count += 1;
      end += 1;
    }

    if (node->style.wrap == M3_LAYOUT_WRAP_NO && inner_width > line_cross) {
      line_cross = inner_width;
    }

    remaining = inner_height - line_main;
    if (remaining < 0.0f) {
      remaining = 0.0f;
    }

    offset = 0.0f;
    spacing = 0.0f;

    switch (node->style.align_main) {
    case M3_LAYOUT_ALIGN_START:
      break;
    case M3_LAYOUT_ALIGN_CENTER:
      offset = remaining * 0.5f;
      break;
    case M3_LAYOUT_ALIGN_END:
      offset = remaining;
      break;
    case M3_LAYOUT_ALIGN_SPACE_BETWEEN:
      if (line_count > 1) {
        spacing = remaining / (M3Scalar)(line_count - 1);
      }
      break;
    case M3_LAYOUT_ALIGN_SPACE_AROUND:
      if (line_count > 0) {
        spacing = remaining / (M3Scalar)line_count;
        offset = spacing * 0.5f;
      }
      break;
    case M3_LAYOUT_ALIGN_SPACE_EVENLY:
      if (line_count > 0) {
        spacing = remaining / (M3Scalar)(line_count + 1);
        offset = spacing;
      }
      break;
    default:
      return M3_ERR_INVALID_ARGUMENT;
    }

    cursor_main = offset;
    for (i = 0; i < line_count; ++i) {
      M3LayoutNode *child;
      M3Scalar child_main;
      M3Scalar child_cross;
      M3Scalar cross_offset;
      M3Scalar child_x;
      M3Scalar child_y;

      child = node->children[index + i];
      child_main = child->measured.height;
      child_cross = child->measured.width;

      cross_offset = 0.0f;
      switch (node->style.align_cross) {
      case M3_LAYOUT_ALIGN_START:
        break;
      case M3_LAYOUT_ALIGN_CENTER:
        cross_offset = (line_cross - child_cross) * 0.5f;
        break;
      case M3_LAYOUT_ALIGN_END:
        cross_offset = line_cross - child_cross;
        break;
      case M3_LAYOUT_ALIGN_STRETCH:
        child_cross = line_cross;
        break;
      default:
        return M3_ERR_INVALID_ARGUMENT;
      }

      if (cross_sign > 0.0f) {
        child_x = cross_start + cursor_cross + cross_offset;
      } else {
        child_x = cross_start - cursor_cross - cross_offset - child_cross;
      }
      child_y = origin_y + padding_top + cursor_main;

      rc = m3_layout_layout_node(child, direction, child_x, child_y,
                                 child_cross, child_main);
      if (rc != M3_OK) {
        return rc;
      }

      cursor_main += child_main + spacing;
    }

    cursor_cross += line_cross;
    index = end;
  }

  return M3_OK;
}

int M3_CALL m3_layout_measure_spec_init(M3LayoutMeasureSpec *spec, m3_u32 mode,
                                        M3Scalar size) {
  if (spec == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  spec->mode = mode;
  spec->size = size;
  return m3_layout_validate_measure_spec(spec);
}

int M3_CALL m3_layout_edges_set(M3LayoutEdges *edges, M3Scalar left,
                                M3Scalar top, M3Scalar right, M3Scalar bottom) {
  if (edges == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (left < 0.0f || top < 0.0f || right < 0.0f || bottom < 0.0f) {
    return M3_ERR_RANGE;
  }

  edges->left = left;
  edges->top = top;
  edges->right = right;
  edges->bottom = bottom;
  return M3_OK;
}

int M3_CALL m3_layout_direction_init(M3LayoutDirection *direction,
                                     m3_u32 flow) {
  if (direction == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  direction->flow = flow;
  return m3_layout_validate_direction(direction);
}

int M3_CALL m3_layout_style_init(M3LayoutStyle *style) {
  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (g_m3_layout_force_style_init_fail) {
    return M3_ERR_UNKNOWN;
  }
#endif

  style->direction = M3_LAYOUT_DIRECTION_ROW;
  style->wrap = M3_LAYOUT_WRAP_NO;
  style->align_main = M3_LAYOUT_ALIGN_START;
  style->align_cross = M3_LAYOUT_ALIGN_START;
  style->padding.left = 0.0f;
  style->padding.top = 0.0f;
  style->padding.right = 0.0f;
  style->padding.bottom = 0.0f;
  style->width = M3_LAYOUT_AUTO;
  style->height = M3_LAYOUT_AUTO;
  return M3_OK;
}

int M3_CALL m3_layout_node_init(M3LayoutNode *node,
                                const M3LayoutStyle *style) {
  int rc;

  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (style != NULL) {
    rc = m3_layout_validate_style(style);
    if (rc != M3_OK) {
      return rc;
    }
    node->style = *style;
  } else {
    rc = m3_layout_style_init(&node->style);
    if (rc != M3_OK) {
      return rc;
    }
  }

  node->children = NULL;
  node->child_count = 0;
  node->measure = NULL;
  node->measure_ctx = NULL;
  node->measured.width = 0.0f;
  node->measured.height = 0.0f;
  node->layout.x = 0.0f;
  node->layout.y = 0.0f;
  node->layout.width = 0.0f;
  node->layout.height = 0.0f;
  return M3_OK;
}

int M3_CALL m3_layout_node_set_children(M3LayoutNode *node,
                                        M3LayoutNode **children,
                                        m3_usize count) {
  m3_usize i;

  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (count > 0 && children == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < count; ++i) {
    if (children[i] == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
  }

  node->children = children;
  node->child_count = count;
  return M3_OK;
}

int M3_CALL m3_layout_node_set_measure(M3LayoutNode *node,
                                       M3LayoutMeasureFn measure, void *ctx) {
  if (node == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  node->measure = measure;
  node->measure_ctx = ctx;
  return M3_OK;
}

int M3_CALL m3_layout_node_get_measured(const M3LayoutNode *node,
                                        M3Size *out_size) {
  if (node == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_size = node->measured;
  return M3_OK;
}

int M3_CALL m3_layout_node_get_layout(const M3LayoutNode *node,
                                      M3Rect *out_rect) {
  if (node == NULL || out_rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_rect = node->layout;
  return M3_OK;
}

int M3_CALL m3_layout_compute(M3LayoutNode *root,
                              const M3LayoutDirection *direction,
                              M3LayoutMeasureSpec width,
                              M3LayoutMeasureSpec height) {
  int rc;

  if (root == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_layout_validate_direction(direction);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_layout_measure_node(root, direction, width, height);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_layout_layout_node(root, direction, 0.0f, 0.0f,
                             root->measured.width, root->measured.height);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL
m3_layout_test_validate_measure_spec(const M3LayoutMeasureSpec *spec) {
  return m3_layout_validate_measure_spec(spec);
}

int M3_CALL
m3_layout_test_validate_direction(const M3LayoutDirection *direction) {
  return m3_layout_validate_direction(direction);
}

int M3_CALL m3_layout_test_clamp_non_negative(M3Scalar value,
                                              M3Scalar *out_value) {
  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_value = m3_layout_clamp_non_negative(value);
  return M3_OK;
}

int M3_CALL m3_layout_test_resolve_available(M3Scalar style_size,
                                             M3LayoutMeasureSpec spec,
                                             M3Scalar *out_available) {
  if (out_available == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_available = m3_layout_resolve_available(style_size, spec);
  return M3_OK;
}

int M3_CALL m3_layout_test_resolve_horizontal_padding(
    const M3LayoutDirection *direction, const M3LayoutEdges *padding,
    M3Scalar *out_left, M3Scalar *out_right) {
  return m3_layout_resolve_horizontal_padding(direction, padding, out_left,
                                              out_right);
}

int M3_CALL m3_layout_test_validate_style(const M3LayoutStyle *style) {
  return m3_layout_validate_style(style);
}

int M3_CALL m3_layout_test_apply_spec(M3Scalar size, M3LayoutMeasureSpec spec,
                                      M3Scalar *out_size) {
  return m3_layout_apply_spec(size, spec, out_size);
}

int M3_CALL m3_layout_test_measure_leaf(M3LayoutNode *node,
                                        M3LayoutMeasureSpec width,
                                        M3LayoutMeasureSpec height,
                                        M3Size *out_size) {
  return m3_layout_measure_leaf(node, width, height, out_size);
}

int M3_CALL m3_layout_test_measure_row(M3LayoutNode *node,
                                       M3LayoutMeasureSpec width,
                                       M3LayoutMeasureSpec height,
                                       M3Size *out_size) {
  M3LayoutDirection direction;
  int rc;

  rc = m3_layout_direction_init(&direction, M3_DIRECTION_LTR);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_layout_measure_row(node, &direction, width, height, out_size);
}

int M3_CALL m3_layout_test_measure_row_with_direction(
    M3LayoutNode *node, const M3LayoutDirection *direction,
    M3LayoutMeasureSpec width, M3LayoutMeasureSpec height, M3Size *out_size) {
  return m3_layout_measure_row(node, direction, width, height, out_size);
}

int M3_CALL m3_layout_test_measure_column(M3LayoutNode *node,
                                          M3LayoutMeasureSpec width,
                                          M3LayoutMeasureSpec height,
                                          M3Size *out_size) {
  M3LayoutDirection direction;
  int rc;

  rc = m3_layout_direction_init(&direction, M3_DIRECTION_LTR);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_layout_measure_column(node, &direction, width, height, out_size);
}

int M3_CALL m3_layout_test_measure_column_with_direction(
    M3LayoutNode *node, const M3LayoutDirection *direction,
    M3LayoutMeasureSpec width, M3LayoutMeasureSpec height, M3Size *out_size) {
  return m3_layout_measure_column(node, direction, width, height, out_size);
}

int M3_CALL m3_layout_test_set_style_init_fail(M3Bool enable) {
  g_m3_layout_force_style_init_fail = enable ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_layout_test_measure_node_with_direction(
    M3LayoutNode *node, const M3LayoutDirection *direction,
    M3LayoutMeasureSpec width, M3LayoutMeasureSpec height) {
  return m3_layout_measure_node(node, direction, width, height);
}

int M3_CALL m3_layout_test_layout_node_with_direction(
    M3LayoutNode *node, const M3LayoutDirection *direction, M3Scalar x,
    M3Scalar y, M3Scalar width, M3Scalar height) {
  return m3_layout_layout_node(node, direction, x, y, width, height);
}

int M3_CALL m3_layout_test_layout_children_row_with_direction(
    M3LayoutNode *node, const M3LayoutDirection *direction, M3Scalar x,
    M3Scalar y, M3Scalar width, M3Scalar height) {
  return m3_layout_layout_children_row(node, direction, x, y, width, height);
}

int M3_CALL m3_layout_test_layout_children_column_with_direction(
    M3LayoutNode *node, const M3LayoutDirection *direction, M3Scalar x,
    M3Scalar y, M3Scalar width, M3Scalar height) {
  return m3_layout_layout_children_column(node, direction, x, y, width, height);
}

int M3_CALL m3_layout_test_set_direction_fail(M3Bool enable) {
  g_m3_layout_force_direction_fail = enable ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_layout_test_measure_node(M3LayoutNode *node,
                                        M3LayoutMeasureSpec width,
                                        M3LayoutMeasureSpec height) {
  M3LayoutDirection direction;
  int rc;

  rc = m3_layout_direction_init(&direction, M3_DIRECTION_LTR);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_layout_measure_node(node, &direction, width, height);
}

int M3_CALL m3_layout_test_layout_node(M3LayoutNode *node, M3Scalar x,
                                       M3Scalar y, M3Scalar width,
                                       M3Scalar height) {
  M3LayoutDirection direction;
  int rc;

  rc = m3_layout_direction_init(&direction, M3_DIRECTION_LTR);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_layout_layout_node(node, &direction, x, y, width, height);
}

int M3_CALL m3_layout_test_layout_children_row(M3LayoutNode *node, M3Scalar x,
                                               M3Scalar y, M3Scalar width,
                                               M3Scalar height) {
  M3LayoutDirection direction;
  int rc;

  rc = m3_layout_direction_init(&direction, M3_DIRECTION_LTR);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_layout_layout_children_row(node, &direction, x, y, width, height);
}

int M3_CALL m3_layout_test_layout_children_column(M3LayoutNode *node,
                                                  M3Scalar x, M3Scalar y,
                                                  M3Scalar width,
                                                  M3Scalar height) {
  M3LayoutDirection direction;
  int rc;

  rc = m3_layout_direction_init(&direction, M3_DIRECTION_LTR);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_layout_layout_children_column(node, &direction, x, y, width,
                                          height);
}
#endif
