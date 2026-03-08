#include "cmpc/cmp_layout.h"

#ifdef CMP_TESTING
static CMPBool g_cmp_layout_force_style_init_fail = CMP_FALSE;
static CMPBool g_cmp_layout_force_direction_fail = CMP_FALSE;
#endif

static CMPScalar cmp_layout_clamp_non_negative(CMPScalar value) {
  if (value < 0.0f) {
    return 0.0f;
  }
  return value;
}

static int cmp_layout_validate_direction(const CMPLayoutDirection *direction) {
  if (direction == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_layout_force_direction_fail) {
    return CMP_ERR_UNKNOWN;
  }
#endif

  if (direction->flow != CMP_DIRECTION_LTR &&
      direction->flow != CMP_DIRECTION_RTL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return CMP_OK;
}

static int cmp_layout_resolve_horizontal_padding(
    const CMPLayoutDirection *direction, const CMPLayoutEdges *padding,
    CMPScalar *out_left, CMPScalar *out_right) {
  CMPScalar start;
  CMPScalar end;
  CMPBool is_rtl;
  int rc;

  if (padding == NULL || out_left == NULL || out_right == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_layout_validate_direction(direction);
  if (rc != CMP_OK) {
    return rc;
  }

  is_rtl = (direction->flow == CMP_DIRECTION_RTL) ? CMP_TRUE : CMP_FALSE;

  start = padding->left;
  end = padding->right;

  if (is_rtl) {
    *out_left = end;
    *out_right = start;
  } else {
    *out_left = start;
    *out_right = end;
  }

  return CMP_OK;
}

static int cmp_layout_validate_measure_spec(const CMPLayoutMeasureSpec *spec) {
  if (spec == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  switch (spec->mode) {
  case CMP_LAYOUT_MEASURE_UNSPECIFIED:
    return CMP_OK;
  case CMP_LAYOUT_MEASURE_EXACTLY:
  case CMP_LAYOUT_MEASURE_AT_MOST:
    if (spec->size < 0.0f) {
      return CMP_ERR_RANGE;
    }
    return CMP_OK;
  default:
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

static int cmp_layout_validate_style(const CMPLayoutStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (style->direction != CMP_LAYOUT_DIRECTION_ROW &&
      style->direction != CMP_LAYOUT_DIRECTION_COLUMN) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (style->wrap != CMP_LAYOUT_WRAP_NO && style->wrap != CMP_LAYOUT_WRAP_YES) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  switch (style->align_main) {
  case CMP_LAYOUT_ALIGN_START:
  case CMP_LAYOUT_ALIGN_CENTER:
  case CMP_LAYOUT_ALIGN_END:
  case CMP_LAYOUT_ALIGN_SPACE_BETWEEN:
  case CMP_LAYOUT_ALIGN_SPACE_AROUND:
  case CMP_LAYOUT_ALIGN_SPACE_EVENLY:
    break;
  default:
    return CMP_ERR_INVALID_ARGUMENT;
  }

  switch (style->align_cross) {
  case CMP_LAYOUT_ALIGN_START:
  case CMP_LAYOUT_ALIGN_CENTER:
  case CMP_LAYOUT_ALIGN_END:
  case CMP_LAYOUT_ALIGN_STRETCH:
    break;
  default:
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (style->padding.left < 0.0f || style->padding.top < 0.0f ||
      style->padding.right < 0.0f || style->padding.bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (style->width < 0.0f && style->width != CMP_LAYOUT_AUTO) {
    return CMP_ERR_RANGE;
  }

  if (style->height < 0.0f && style->height != CMP_LAYOUT_AUTO) {
    return CMP_ERR_RANGE;
  }

  return CMP_OK;
}

static int cmp_layout_apply_spec(CMPScalar size, CMPLayoutMeasureSpec spec,
                                 CMPScalar *out_size) {
  if (out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  switch (spec.mode) {
  case CMP_LAYOUT_MEASURE_UNSPECIFIED:
    *out_size = size;
    return CMP_OK;
  case CMP_LAYOUT_MEASURE_EXACTLY:
    if (spec.size < 0.0f) {
      return CMP_ERR_RANGE;
    }
    *out_size = spec.size;
    return CMP_OK;
  case CMP_LAYOUT_MEASURE_AT_MOST:
    if (spec.size < 0.0f) {
      return CMP_ERR_RANGE;
    }
    *out_size = (size > spec.size) ? spec.size : size;
    return CMP_OK;
  default:
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

static CMPScalar cmp_layout_resolve_available(CMPScalar style_size,
                                              CMPLayoutMeasureSpec spec) {
  CMPScalar available;

  available = -1.0f;
  if (style_size >= 0.0f) {
    available = style_size;
  }

  if (spec.mode != CMP_LAYOUT_MEASURE_UNSPECIFIED) {
    if (available < 0.0f) {
      available = spec.size;
    } else if (spec.size < available) {
      available = spec.size;
    }
  }

  return available;
}

static int cmp_layout_measure_node(CMPLayoutNode *node,
                                   const CMPLayoutDirection *direction,
                                   CMPLayoutMeasureSpec width,
                                   CMPLayoutMeasureSpec height);

static int cmp_layout_measure_leaf(CMPLayoutNode *node,
                                   CMPLayoutMeasureSpec width,
                                   CMPLayoutMeasureSpec height,
                                   CMPSize *out_size) {
  CMPLayoutMeasureSpec measure_width;
  CMPLayoutMeasureSpec measure_height;
  CMPSize measured;
  CMPScalar forced_width;
  CMPScalar forced_height;
  int rc;

  if (node == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  forced_width = node->style.width;
  forced_height = node->style.height;

  measure_width = width;
  measure_height = height;

  if (forced_width >= 0.0f) {
    measure_width.mode = CMP_LAYOUT_MEASURE_EXACTLY;
    measure_width.size = forced_width;
  }
  if (forced_height >= 0.0f) {
    measure_height.mode = CMP_LAYOUT_MEASURE_EXACTLY;
    measure_height.size = forced_height;
  }

  if (node->measure != NULL) {
    rc = node->measure(node->measure_ctx, measure_width, measure_height,
                       &measured);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    measured.width = (forced_width >= 0.0f) ? forced_width : 0.0f;
    measured.height = (forced_height >= 0.0f) ? forced_height : 0.0f;
  }

  if (measured.width < 0.0f || measured.height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (forced_width >= 0.0f) {
    measured.width = forced_width;
  }
  if (forced_height >= 0.0f) {
    measured.height = forced_height;
  }

  rc = cmp_layout_apply_spec(measured.width, width, &measured.width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_layout_apply_spec(measured.height, height, &measured.height);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_size = measured;
  return CMP_OK;
}

static int cmp_layout_measure_row(CMPLayoutNode *node,
                                  const CMPLayoutDirection *direction,
                                  CMPLayoutMeasureSpec width,
                                  CMPLayoutMeasureSpec height,
                                  CMPSize *out_size) {
  CMPLayoutMeasureSpec child_width_spec;
  CMPLayoutMeasureSpec child_height_spec;
  CMPScalar padding_lr;
  CMPScalar padding_tb;
  CMPScalar padding_left;
  CMPScalar padding_right;
  CMPScalar available_width;
  CMPScalar available_height;
  CMPScalar inner_width_limit;
  CMPScalar inner_height_limit;
  CMPScalar line_main;
  CMPScalar line_cross;
  CMPScalar content_main;
  CMPScalar content_cross;
  CMPScalar content_main_max;
  CMPScalar content_cross_sum;
  CMPScalar total_width;
  CMPScalar total_height;
  CMPBool wrap_enabled;
  cmp_usize i;
  int rc;

  if (node == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_layout_resolve_horizontal_padding(direction, &node->style.padding,
                                             &padding_left, &padding_right);
  if (rc != CMP_OK) {
    return rc;
  }

  padding_lr = padding_left + padding_right;
  padding_tb = node->style.padding.top + node->style.padding.bottom;

  available_width = cmp_layout_resolve_available(node->style.width, width);
  available_height = cmp_layout_resolve_available(node->style.height, height);

  if (available_width >= 0.0f) {
    inner_width_limit =
        cmp_layout_clamp_non_negative(available_width - padding_lr);
  } else {
    inner_width_limit = -1.0f;
  }

  if (available_height >= 0.0f) {
    inner_height_limit =
        cmp_layout_clamp_non_negative(available_height - padding_tb);
  } else {
    inner_height_limit = -1.0f;
  }

  if (inner_width_limit >= 0.0f) {
    child_width_spec.mode = CMP_LAYOUT_MEASURE_AT_MOST;
    child_width_spec.size = inner_width_limit;
  } else {
    child_width_spec.mode = CMP_LAYOUT_MEASURE_UNSPECIFIED;
    child_width_spec.size = 0.0f;
  }

  if (inner_height_limit >= 0.0f) {
    child_height_spec.mode = CMP_LAYOUT_MEASURE_AT_MOST;
    child_height_spec.size = inner_height_limit;
  } else {
    child_height_spec.mode = CMP_LAYOUT_MEASURE_UNSPECIFIED;
    child_height_spec.size = 0.0f;
  }

  wrap_enabled =
      (node->style.wrap == CMP_LAYOUT_WRAP_YES && inner_width_limit >= 0.0f)
          ? CMP_TRUE
          : CMP_FALSE;

  line_main = 0.0f;
  line_cross = 0.0f;
  content_main_max = 0.0f;
  content_cross_sum = 0.0f;

  for (i = 0; i < node->child_count; ++i) {
    CMPLayoutNode *child;
    CMPScalar child_main;
    CMPScalar child_cross;

    child = node->children[i];
    rc = cmp_layout_measure_node(child, direction, child_width_spec,
                                 child_height_spec);
    if (rc != CMP_OK) {
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

  rc = cmp_layout_apply_spec(total_width, width, &total_width);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_layout_apply_spec(total_height, height, &total_height);
  if (rc != CMP_OK) {
    return rc;
  }

  out_size->width = total_width;
  out_size->height = total_height;
  return CMP_OK;
}

static int cmp_layout_measure_column(CMPLayoutNode *node,
                                     const CMPLayoutDirection *direction,
                                     CMPLayoutMeasureSpec width,
                                     CMPLayoutMeasureSpec height,
                                     CMPSize *out_size) {
  CMPLayoutMeasureSpec child_width_spec;
  CMPLayoutMeasureSpec child_height_spec;
  CMPScalar padding_lr;
  CMPScalar padding_tb;
  CMPScalar padding_left;
  CMPScalar padding_right;
  CMPScalar available_width;
  CMPScalar available_height;
  CMPScalar inner_width_limit;
  CMPScalar inner_height_limit;
  CMPScalar line_main;
  CMPScalar line_cross;
  CMPScalar content_main;
  CMPScalar content_cross;
  CMPScalar content_main_max;
  CMPScalar content_cross_sum;
  CMPScalar total_width;
  CMPScalar total_height;
  CMPBool wrap_enabled;
  cmp_usize i;
  int rc;

  if (node == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_layout_resolve_horizontal_padding(direction, &node->style.padding,
                                             &padding_left, &padding_right);
  if (rc != CMP_OK) {
    return rc;
  }

  padding_lr = padding_left + padding_right;
  padding_tb = node->style.padding.top + node->style.padding.bottom;

  available_width = cmp_layout_resolve_available(node->style.width, width);
  available_height = cmp_layout_resolve_available(node->style.height, height);

  if (available_width >= 0.0f) {
    inner_width_limit =
        cmp_layout_clamp_non_negative(available_width - padding_lr);
  } else {
    inner_width_limit = -1.0f;
  }

  if (available_height >= 0.0f) {
    inner_height_limit =
        cmp_layout_clamp_non_negative(available_height - padding_tb);
  } else {
    inner_height_limit = -1.0f;
  }

  if (inner_width_limit >= 0.0f) {
    child_width_spec.mode = CMP_LAYOUT_MEASURE_AT_MOST;
    child_width_spec.size = inner_width_limit;
  } else {
    child_width_spec.mode = CMP_LAYOUT_MEASURE_UNSPECIFIED;
    child_width_spec.size = 0.0f;
  }

  if (inner_height_limit >= 0.0f) {
    child_height_spec.mode = CMP_LAYOUT_MEASURE_AT_MOST;
    child_height_spec.size = inner_height_limit;
  } else {
    child_height_spec.mode = CMP_LAYOUT_MEASURE_UNSPECIFIED;
    child_height_spec.size = 0.0f;
  }

  wrap_enabled =
      (node->style.wrap == CMP_LAYOUT_WRAP_YES && inner_height_limit >= 0.0f)
          ? CMP_TRUE
          : CMP_FALSE;

  line_main = 0.0f;
  line_cross = 0.0f;
  content_main_max = 0.0f;
  content_cross_sum = 0.0f;

  for (i = 0; i < node->child_count; ++i) {
    CMPLayoutNode *child;
    CMPScalar child_main;
    CMPScalar child_cross;

    child = node->children[i];
    rc = cmp_layout_measure_node(child, direction, child_width_spec,
                                 child_height_spec);
    if (rc != CMP_OK) {
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

  rc = cmp_layout_apply_spec(total_width, width, &total_width);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_layout_apply_spec(total_height, height, &total_height);
  if (rc != CMP_OK) {
    return rc;
  }

  out_size->width = total_width;
  out_size->height = total_height;
  return CMP_OK;
}

static int cmp_layout_measure_node(CMPLayoutNode *node,
                                   const CMPLayoutDirection *direction,
                                   CMPLayoutMeasureSpec width,
                                   CMPLayoutMeasureSpec height) {
  CMPSize measured;
  int rc;

  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_layout_validate_direction(direction);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_layout_validate_measure_spec(&width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_layout_validate_measure_spec(&height);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_layout_validate_style(&node->style);
  if (rc != CMP_OK) {
    return rc;
  }

  if (node->child_count > 0 && node->children == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (node->child_count == 0) {
    rc = cmp_layout_measure_leaf(node, width, height, &measured);
  } else {
    if (node->style.direction == CMP_LAYOUT_DIRECTION_ROW) {
      rc = cmp_layout_measure_row(node, direction, width, height, &measured);
    } else {
      rc = cmp_layout_measure_column(node, direction, width, height, &measured);
    }
  }

  if (rc != CMP_OK) {
    return rc;
  }

  node->measured = measured;
  return CMP_OK;
}

static int cmp_layout_layout_children_row(CMPLayoutNode *node,
                                          const CMPLayoutDirection *direction,
                                          CMPScalar origin_x,
                                          CMPScalar origin_y, CMPScalar width,
                                          CMPScalar height);
static int cmp_layout_layout_children_column(
    CMPLayoutNode *node, const CMPLayoutDirection *direction,
    CMPScalar origin_x, CMPScalar origin_y, CMPScalar width, CMPScalar height);

static int cmp_layout_layout_node(CMPLayoutNode *node,
                                  const CMPLayoutDirection *direction,
                                  CMPScalar origin_x, CMPScalar origin_y,
                                  CMPScalar width, CMPScalar height) {
  int rc;

  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_layout_validate_direction(direction);
  if (rc != CMP_OK) {
    return rc;
  }

  node->layout.x = origin_x;
  node->layout.y = origin_y;
  node->layout.width = width;
  node->layout.height = height;

  if (node->child_count == 0) {
    return CMP_OK;
  }

  if (node->children == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (node->style.direction == CMP_LAYOUT_DIRECTION_ROW) {
    rc = cmp_layout_layout_children_row(node, direction, origin_x, origin_y,
                                        width, height);
  } else if (node->style.direction == CMP_LAYOUT_DIRECTION_COLUMN) {
    rc = cmp_layout_layout_children_column(node, direction, origin_x, origin_y,
                                           width, height);
  } else {
    rc = CMP_ERR_INVALID_ARGUMENT;
  }

  return rc;
}

static int cmp_layout_layout_children_row(CMPLayoutNode *node,
                                          const CMPLayoutDirection *direction,
                                          CMPScalar origin_x,
                                          CMPScalar origin_y, CMPScalar width,
                                          CMPScalar height) {
  CMPScalar padding_left;
  CMPScalar padding_top;
  CMPScalar padding_right;
  CMPScalar padding_bottom;
  CMPScalar inner_width;
  CMPScalar inner_height;
  CMPBool wrap_enabled;
  cmp_usize index;
  CMPScalar cursor_cross;
  CMPScalar main_start;
  CMPScalar main_sign;
  CMPBool is_rtl;
  int rc;

  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_layout_resolve_horizontal_padding(direction, &node->style.padding,
                                             &padding_left, &padding_right);
  if (rc != CMP_OK) {
    return rc;
  }

  is_rtl = (direction->flow == CMP_DIRECTION_RTL) ? CMP_TRUE : CMP_FALSE;
  padding_top = node->style.padding.top;
  padding_bottom = node->style.padding.bottom;

  inner_width = width - (padding_left + padding_right);
  inner_height = height - (padding_top + padding_bottom);

  inner_width = cmp_layout_clamp_non_negative(inner_width);
  inner_height = cmp_layout_clamp_non_negative(inner_height);

  wrap_enabled =
      (node->style.wrap == CMP_LAYOUT_WRAP_YES && inner_width >= 0.0f)
          ? CMP_TRUE
          : CMP_FALSE;

  if (is_rtl) {
    main_start = origin_x + width - padding_right;
    main_sign = -1.0f;
  } else {
    main_start = origin_x + padding_left;
    main_sign = 1.0f;
  }

  index = 0;
  cursor_cross = 0.0f;
  rc = CMP_OK;

  while (index < node->child_count) {
    CMPScalar line_main;
    CMPScalar line_cross;
    cmp_usize line_count;
    cmp_usize i;
    cmp_usize end;
    CMPScalar remaining;
    CMPScalar offset;
    CMPScalar spacing;
    CMPScalar cursor_main;

    line_main = 0.0f;
    line_cross = 0.0f;
    line_count = 0;
    end = index;

    while (end < node->child_count) {
      CMPLayoutNode *child;
      CMPScalar child_main;
      CMPScalar child_cross;

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

    if (node->style.wrap == CMP_LAYOUT_WRAP_NO && inner_height > line_cross) {
      line_cross = inner_height;
    }

    remaining = inner_width - line_main;
    if (remaining < 0.0f) {
      remaining = 0.0f;
    }

    offset = 0.0f;
    spacing = 0.0f;

    switch (node->style.align_main) {
    case CMP_LAYOUT_ALIGN_START:
      break;
    case CMP_LAYOUT_ALIGN_CENTER:
      offset = remaining * 0.5f;
      break;
    case CMP_LAYOUT_ALIGN_END:
      offset = remaining;
      break;
    case CMP_LAYOUT_ALIGN_SPACE_BETWEEN:
      if (line_count > 1) {
        spacing = remaining / (CMPScalar)(line_count - 1);
      }
      break;
    case CMP_LAYOUT_ALIGN_SPACE_AROUND:
      if (line_count > 0) {
        spacing = remaining / (CMPScalar)line_count;
        offset = spacing * 0.5f;
      }
      break;
    case CMP_LAYOUT_ALIGN_SPACE_EVENLY:
      if (line_count > 0) {
        spacing = remaining / (CMPScalar)(line_count + 1);
        offset = spacing;
      }
      break;
    default:
      return CMP_ERR_INVALID_ARGUMENT;
    }

    cursor_main = offset;
    for (i = 0; i < line_count; ++i) {
      CMPLayoutNode *child;
      CMPScalar child_main;
      CMPScalar child_cross;
      CMPScalar cross_offset;
      CMPScalar child_x;
      CMPScalar child_y;

      child = node->children[index + i];
      child_main = child->measured.width;
      child_cross = child->measured.height;

      cross_offset = 0.0f;
      switch (node->style.align_cross) {
      case CMP_LAYOUT_ALIGN_START:
        break;
      case CMP_LAYOUT_ALIGN_CENTER:
        cross_offset = (line_cross - child_cross) * 0.5f;
        break;
      case CMP_LAYOUT_ALIGN_END:
        cross_offset = line_cross - child_cross;
        break;
      case CMP_LAYOUT_ALIGN_STRETCH:
        child_cross = line_cross;
        break;
      default:
        return CMP_ERR_INVALID_ARGUMENT;
      }

      if (main_sign > 0.0f) {
        child_x = main_start + cursor_main;
      } else {
        child_x = main_start - cursor_main - child_main;
      }
      child_y = origin_y + padding_top + cursor_cross + cross_offset;

      rc = cmp_layout_layout_node(child, direction, child_x, child_y,
                                  child_main, child_cross);
      if (rc != CMP_OK) {
        return rc;
      }

      cursor_main += child_main + spacing;
    }

    cursor_cross += line_cross;
    index = end;
  }

  return CMP_OK;
}

static int cmp_layout_layout_children_column(
    CMPLayoutNode *node, const CMPLayoutDirection *direction,
    CMPScalar origin_x, CMPScalar origin_y, CMPScalar width, CMPScalar height) {
  CMPScalar padding_left;
  CMPScalar padding_top;
  CMPScalar padding_right;
  CMPScalar padding_bottom;
  CMPScalar inner_width;
  CMPScalar inner_height;
  CMPBool wrap_enabled;
  cmp_usize index;
  CMPScalar cursor_cross;
  CMPScalar cross_start;
  CMPScalar cross_sign;
  CMPBool is_rtl;
  int rc;

  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_layout_resolve_horizontal_padding(direction, &node->style.padding,
                                             &padding_left, &padding_right);
  if (rc != CMP_OK) {
    return rc;
  }

  is_rtl = (direction->flow == CMP_DIRECTION_RTL) ? CMP_TRUE : CMP_FALSE;
  padding_top = node->style.padding.top;
  padding_bottom = node->style.padding.bottom;

  inner_width = width - (padding_left + padding_right);
  inner_height = height - (padding_top + padding_bottom);

  inner_width = cmp_layout_clamp_non_negative(inner_width);
  inner_height = cmp_layout_clamp_non_negative(inner_height);

  wrap_enabled =
      (node->style.wrap == CMP_LAYOUT_WRAP_YES && inner_height >= 0.0f)
          ? CMP_TRUE
          : CMP_FALSE;

  if (is_rtl) {
    cross_start = origin_x + width - padding_right;
    cross_sign = -1.0f;
  } else {
    cross_start = origin_x + padding_left;
    cross_sign = 1.0f;
  }

  index = 0;
  cursor_cross = 0.0f;
  rc = CMP_OK;

  while (index < node->child_count) {
    CMPScalar line_main;
    CMPScalar line_cross;
    cmp_usize line_count;
    cmp_usize i;
    cmp_usize end;
    CMPScalar remaining;
    CMPScalar offset;
    CMPScalar spacing;
    CMPScalar cursor_main;

    line_main = 0.0f;
    line_cross = 0.0f;
    line_count = 0;
    end = index;

    while (end < node->child_count) {
      CMPLayoutNode *child;
      CMPScalar child_main;
      CMPScalar child_cross;

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

    if (node->style.wrap == CMP_LAYOUT_WRAP_NO && inner_width > line_cross) {
      line_cross = inner_width;
    }

    remaining = inner_height - line_main;
    if (remaining < 0.0f) {
      remaining = 0.0f;
    }

    offset = 0.0f;
    spacing = 0.0f;

    switch (node->style.align_main) {
    case CMP_LAYOUT_ALIGN_START:
      break;
    case CMP_LAYOUT_ALIGN_CENTER:
      offset = remaining * 0.5f;
      break;
    case CMP_LAYOUT_ALIGN_END:
      offset = remaining;
      break;
    case CMP_LAYOUT_ALIGN_SPACE_BETWEEN:
      if (line_count > 1) {
        spacing = remaining / (CMPScalar)(line_count - 1);
      }
      break;
    case CMP_LAYOUT_ALIGN_SPACE_AROUND:
      if (line_count > 0) {
        spacing = remaining / (CMPScalar)line_count;
        offset = spacing * 0.5f;
      }
      break;
    case CMP_LAYOUT_ALIGN_SPACE_EVENLY:
      if (line_count > 0) {
        spacing = remaining / (CMPScalar)(line_count + 1);
        offset = spacing;
      }
      break;
    default:
      return CMP_ERR_INVALID_ARGUMENT;
    }

    cursor_main = offset;
    for (i = 0; i < line_count; ++i) {
      CMPLayoutNode *child;
      CMPScalar child_main;
      CMPScalar child_cross;
      CMPScalar cross_offset;
      CMPScalar child_x;
      CMPScalar child_y;

      child = node->children[index + i];
      child_main = child->measured.height;
      child_cross = child->measured.width;

      cross_offset = 0.0f;
      switch (node->style.align_cross) {
      case CMP_LAYOUT_ALIGN_START:
        break;
      case CMP_LAYOUT_ALIGN_CENTER:
        cross_offset = (line_cross - child_cross) * 0.5f;
        break;
      case CMP_LAYOUT_ALIGN_END:
        cross_offset = line_cross - child_cross;
        break;
      case CMP_LAYOUT_ALIGN_STRETCH:
        child_cross = line_cross;
        break;
      default:
        return CMP_ERR_INVALID_ARGUMENT;
      }

      if (cross_sign > 0.0f) {
        child_x = cross_start + cursor_cross + cross_offset;
      } else {
        child_x = cross_start - cursor_cross - cross_offset - child_cross;
      }
      child_y = origin_y + padding_top + cursor_main;

      rc = cmp_layout_layout_node(child, direction, child_x, child_y,
                                  child_cross, child_main);
      if (rc != CMP_OK) {
        return rc;
      }

      cursor_main += child_main + spacing;
    }

    cursor_cross += line_cross;
    index = end;
  }

  return CMP_OK;
}

int CMP_CALL cmp_layout_measure_spec_init(CMPLayoutMeasureSpec *spec,
                                          cmp_u32 mode, CMPScalar size) {
  if (spec == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  spec->mode = mode;
  spec->size = size;
  return cmp_layout_validate_measure_spec(spec);
}

int CMP_CALL cmp_layout_edges_set(CMPLayoutEdges *edges, CMPScalar left,
                                  CMPScalar top, CMPScalar right,
                                  CMPScalar bottom) {
  if (edges == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (left < 0.0f || top < 0.0f || right < 0.0f || bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }

  edges->left = left;
  edges->top = top;
  edges->right = right;
  edges->bottom = bottom;
  return CMP_OK;
}

int CMP_CALL cmp_layout_direction_init(CMPLayoutDirection *direction,
                                       cmp_u32 flow) {
  if (direction == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  direction->flow = flow;
  return cmp_layout_validate_direction(direction);
}

int CMP_CALL cmp_layout_style_init(CMPLayoutStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_layout_force_style_init_fail) {
    return CMP_ERR_UNKNOWN;
  }
#endif

  style->direction = CMP_LAYOUT_DIRECTION_ROW;
  style->wrap = CMP_LAYOUT_WRAP_NO;
  style->align_main = CMP_LAYOUT_ALIGN_START;
  style->align_cross = CMP_LAYOUT_ALIGN_START;
  style->padding.left = 0.0f;
  style->padding.top = 0.0f;
  style->padding.right = 0.0f;
  style->padding.bottom = 0.0f;
  style->width = CMP_LAYOUT_AUTO;
  style->height = CMP_LAYOUT_AUTO;
  return CMP_OK;
}

int CMP_CALL cmp_layout_node_init(CMPLayoutNode *node,
                                  const CMPLayoutStyle *style) {
  int rc;

  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (style != NULL) {
    rc = cmp_layout_validate_style(style);
    if (rc != CMP_OK) {
      return rc;
    }
    node->style = *style;
  } else {
    rc = cmp_layout_style_init(&node->style);
    if (rc != CMP_OK) {
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
  return CMP_OK;
}

int CMP_CALL cmp_layout_node_set_children(CMPLayoutNode *node,
                                          CMPLayoutNode **children,
                                          cmp_usize count) {
  cmp_usize i;

  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (count > 0 && children == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < count; ++i) {
    if (children[i] == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }

  node->children = children;
  node->child_count = count;
  return CMP_OK;
}

int CMP_CALL cmp_layout_node_set_measure(CMPLayoutNode *node,
                                         CMPLayoutMeasureFn measure,
                                         void *ctx) {
  if (node == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  node->measure = measure;
  node->measure_ctx = ctx;
  return CMP_OK;
}

int CMP_CALL cmp_layout_node_get_measured(const CMPLayoutNode *node,
                                          CMPSize *out_size) {
  if (node == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_size = node->measured;
  return CMP_OK;
}

int CMP_CALL cmp_layout_node_get_layout(const CMPLayoutNode *node,
                                        CMPRect *out_rect) {
  if (node == NULL || out_rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_rect = node->layout;
  return CMP_OK;
}

int CMP_CALL cmp_layout_compute(CMPLayoutNode *root,
                                const CMPLayoutDirection *direction,
                                CMPLayoutMeasureSpec width,
                                CMPLayoutMeasureSpec height) {
  int rc;

  if (root == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_layout_validate_direction(direction);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_layout_measure_node(root, direction, width, height);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_layout_layout_node(root, direction, 0.0f, 0.0f, root->measured.width,
                              root->measured.height);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL
cmp_layout_test_validate_measure_spec(const CMPLayoutMeasureSpec *spec) {
  return cmp_layout_validate_measure_spec(spec);
}

int CMP_CALL
cmp_layout_test_validate_direction(const CMPLayoutDirection *direction) {
  return cmp_layout_validate_direction(direction);
}

int CMP_CALL cmp_layout_test_clamp_non_negative(CMPScalar value,
                                                CMPScalar *out_value) {
  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_value = cmp_layout_clamp_non_negative(value);
  return CMP_OK;
}

int CMP_CALL cmp_layout_test_resolve_available(CMPScalar style_size,
                                               CMPLayoutMeasureSpec spec,
                                               CMPScalar *out_available) {
  if (out_available == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_available = cmp_layout_resolve_available(style_size, spec);
  return CMP_OK;
}

int CMP_CALL cmp_layout_test_resolve_horizontal_padding(
    const CMPLayoutDirection *direction, const CMPLayoutEdges *padding,
    CMPScalar *out_left, CMPScalar *out_right) {
  return cmp_layout_resolve_horizontal_padding(direction, padding, out_left,
                                               out_right);
}

int CMP_CALL cmp_layout_test_validate_style(const CMPLayoutStyle *style) {
  return cmp_layout_validate_style(style);
}

int CMP_CALL cmp_layout_test_apply_spec(CMPScalar size,
                                        CMPLayoutMeasureSpec spec,
                                        CMPScalar *out_size) {
  return cmp_layout_apply_spec(size, spec, out_size);
}

int CMP_CALL cmp_layout_test_measure_leaf(CMPLayoutNode *node,
                                          CMPLayoutMeasureSpec width,
                                          CMPLayoutMeasureSpec height,
                                          CMPSize *out_size) {
  return cmp_layout_measure_leaf(node, width, height, out_size);
}

int CMP_CALL cmp_layout_test_measure_row(CMPLayoutNode *node,
                                         CMPLayoutMeasureSpec width,
                                         CMPLayoutMeasureSpec height,
                                         CMPSize *out_size) {
  CMPLayoutDirection direction;
  int rc;

  rc = cmp_layout_direction_init(&direction, CMP_DIRECTION_LTR);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_layout_measure_row(node, &direction, width, height, out_size);
}

int CMP_CALL cmp_layout_test_measure_row_with_direction(
    CMPLayoutNode *node, const CMPLayoutDirection *direction,
    CMPLayoutMeasureSpec width, CMPLayoutMeasureSpec height,
    CMPSize *out_size) {
  return cmp_layout_measure_row(node, direction, width, height, out_size);
}

int CMP_CALL cmp_layout_test_measure_column(CMPLayoutNode *node,
                                            CMPLayoutMeasureSpec width,
                                            CMPLayoutMeasureSpec height,
                                            CMPSize *out_size) {
  CMPLayoutDirection direction;
  int rc;

  rc = cmp_layout_direction_init(&direction, CMP_DIRECTION_LTR);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_layout_measure_column(node, &direction, width, height, out_size);
}

int CMP_CALL cmp_layout_test_measure_column_with_direction(
    CMPLayoutNode *node, const CMPLayoutDirection *direction,
    CMPLayoutMeasureSpec width, CMPLayoutMeasureSpec height,
    CMPSize *out_size) {
  return cmp_layout_measure_column(node, direction, width, height, out_size);
}

int CMP_CALL cmp_layout_test_set_style_init_fail(CMPBool enable) {
  g_cmp_layout_force_style_init_fail = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_layout_test_measure_node_with_direction(
    CMPLayoutNode *node, const CMPLayoutDirection *direction,
    CMPLayoutMeasureSpec width, CMPLayoutMeasureSpec height) {
  return cmp_layout_measure_node(node, direction, width, height);
}

int CMP_CALL cmp_layout_test_layout_node_with_direction(
    CMPLayoutNode *node, const CMPLayoutDirection *direction, CMPScalar x,
    CMPScalar y, CMPScalar width, CMPScalar height) {
  return cmp_layout_layout_node(node, direction, x, y, width, height);
}

int CMP_CALL cmp_layout_test_layout_children_row_with_direction(
    CMPLayoutNode *node, const CMPLayoutDirection *direction, CMPScalar x,
    CMPScalar y, CMPScalar width, CMPScalar height) {
  return cmp_layout_layout_children_row(node, direction, x, y, width, height);
}

int CMP_CALL cmp_layout_test_layout_children_column_with_direction(
    CMPLayoutNode *node, const CMPLayoutDirection *direction, CMPScalar x,
    CMPScalar y, CMPScalar width, CMPScalar height) {
  return cmp_layout_layout_children_column(node, direction, x, y, width,
                                           height);
}

int CMP_CALL cmp_layout_test_set_direction_fail(CMPBool enable) {
  g_cmp_layout_force_direction_fail = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_layout_test_measure_node(CMPLayoutNode *node,
                                          CMPLayoutMeasureSpec width,
                                          CMPLayoutMeasureSpec height) {
  CMPLayoutDirection direction;
  int rc;

  rc = cmp_layout_direction_init(&direction, CMP_DIRECTION_LTR);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_layout_measure_node(node, &direction, width, height);
}

int CMP_CALL cmp_layout_test_layout_node(CMPLayoutNode *node, CMPScalar x,
                                         CMPScalar y, CMPScalar width,
                                         CMPScalar height) {
  CMPLayoutDirection direction;
  int rc;

  rc = cmp_layout_direction_init(&direction, CMP_DIRECTION_LTR);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_layout_layout_node(node, &direction, x, y, width, height);
}

int CMP_CALL cmp_layout_test_layout_children_row(CMPLayoutNode *node,
                                                 CMPScalar x, CMPScalar y,
                                                 CMPScalar width,
                                                 CMPScalar height) {
  CMPLayoutDirection direction;
  int rc;

  rc = cmp_layout_direction_init(&direction, CMP_DIRECTION_LTR);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_layout_layout_children_row(node, &direction, x, y, width, height);
}

int CMP_CALL cmp_layout_test_layout_children_column(CMPLayoutNode *node,
                                                    CMPScalar x, CMPScalar y,
                                                    CMPScalar width,
                                                    CMPScalar height) {
  CMPLayoutDirection direction;
  int rc;

  rc = cmp_layout_direction_init(&direction, CMP_DIRECTION_LTR);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_layout_layout_children_column(node, &direction, x, y, width,
                                           height);
}
#endif
