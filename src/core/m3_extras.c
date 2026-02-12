#include "m3/m3_extras.h"

#include <string.h>

static int m3_extras_validate_bool(M3Bool value) {
  if (value != M3_FALSE && value != M3_TRUE) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

static int m3_extras_validate_color(const M3Color *color) {
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

static int m3_extras_validate_edges(const M3LayoutEdges *edges) {
  if (edges == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_extras_validate_text_style(const M3TextStyle *style,
                                         M3Bool require_family) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (require_family == M3_TRUE && style->utf8_family == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return M3_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return M3_ERR_RANGE;
  }
  if (style->italic != M3_FALSE && style->italic != M3_TRUE) {
    return M3_ERR_RANGE;
  }

  rc = m3_extras_validate_color(&style->color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_extras_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_extras_validate_size(const M3Size *size) {
  if (size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size->width < 0.0f || size->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_extras_validate_text_metrics(const M3TextMetrics *metrics) {
  if (metrics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (metrics->width < 0.0f || metrics->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (metrics->baseline < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_extras_validate_tooltip_anchor(const M3TooltipAnchor *anchor) {
  int rc;

  if (anchor == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (anchor->type == M3_TOOLTIP_ANCHOR_RECT) {
    rc = m3_extras_validate_rect(&anchor->rect);
    if (rc != M3_OK) {
      return rc;
    }
    return M3_OK;
  }
  if (anchor->type == M3_TOOLTIP_ANCHOR_POINT) {
    return M3_OK;
  }
  return M3_ERR_RANGE;
}

static int
m3_extras_validate_tooltip_placement(const M3TooltipPlacement *placement) {
  if (placement == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (placement->direction != M3_TOOLTIP_DIRECTION_UP &&
      placement->direction != M3_TOOLTIP_DIRECTION_DOWN &&
      placement->direction != M3_TOOLTIP_DIRECTION_LEFT &&
      placement->direction != M3_TOOLTIP_DIRECTION_RIGHT) {
    return M3_ERR_RANGE;
  }
  if (placement->align != M3_TOOLTIP_ALIGN_START &&
      placement->align != M3_TOOLTIP_ALIGN_CENTER &&
      placement->align != M3_TOOLTIP_ALIGN_END) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_extras_validate_tooltip_style(const M3TooltipStyle *style,
                                            M3Bool require_family) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (style->variant != M3_TOOLTIP_VARIANT_PLAIN &&
      style->variant != M3_TOOLTIP_VARIANT_RICH) {
    return M3_ERR_RANGE;
  }
  if (style->min_width < 0.0f || style->min_height < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->max_width < 0.0f || style->max_height < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->max_width > 0.0f && style->max_width < style->min_width) {
    return M3_ERR_RANGE;
  }
  if (style->max_height > 0.0f && style->max_height < style->min_height) {
    return M3_ERR_RANGE;
  }
  if (style->corner_radius < 0.0f || style->anchor_gap < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->title_body_spacing < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_extras_validate_edges(&style->padding);
  if (rc != M3_OK) {
    return rc;
  }

  if (style->variant == M3_TOOLTIP_VARIANT_PLAIN) {
    rc = m3_extras_validate_text_style(&style->text_style, require_family);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    rc = m3_extras_validate_text_style(&style->title_style, require_family);
    if (rc != M3_OK) {
      return rc;
    }
    rc = m3_extras_validate_text_style(&style->body_style, require_family);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_extras_validate_color(&style->background_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_extras_validate_tooltip_content(const M3TooltipStyle *style,
                                              const M3TooltipContent *content) {
  int rc;

  if (style == NULL || content == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_extras_validate_bool(content->has_title);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_extras_validate_bool(content->has_body);
  if (rc != M3_OK) {
    return rc;
  }

  if (content->has_title == M3_TRUE) {
    rc = m3_extras_validate_text_metrics(&content->title_metrics);
    if (rc != M3_OK) {
      return rc;
    }
  }
  if (content->has_body == M3_TRUE) {
    rc = m3_extras_validate_text_metrics(&content->body_metrics);
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (style->variant == M3_TOOLTIP_VARIANT_PLAIN) {
    if (content->has_body != M3_TRUE) {
      return M3_ERR_INVALID_ARGUMENT;
    }
  } else {
    if (content->has_title != M3_TRUE && content->has_body != M3_TRUE) {
      return M3_ERR_INVALID_ARGUMENT;
    }
  }

  return M3_OK;
}

static int m3_extras_validate_badge_style(const M3BadgeStyle *style,
                                          M3Bool require_family) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->min_size < 0.0f || style->dot_diameter < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->padding_x < 0.0f || style->padding_y < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_extras_validate_text_style(&style->text_style, require_family);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_extras_validate_color(&style->background_color);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

static int m3_extras_validate_badge_content(const M3BadgeContent *content) {
  int rc;

  if (content == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  rc = m3_extras_validate_bool(content->has_text);
  if (rc != M3_OK) {
    return rc;
  }
  if (content->has_text == M3_TRUE) {
    rc = m3_extras_validate_text_metrics(&content->text_metrics);
    if (rc != M3_OK) {
      return rc;
    }
  }
  return M3_OK;
}

static int
m3_extras_validate_badge_placement(const M3BadgePlacement *placement) {
  if (placement == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  switch (placement->corner) {
  case M3_BADGE_CORNER_TOP_LEFT:
  case M3_BADGE_CORNER_TOP_RIGHT:
  case M3_BADGE_CORNER_BOTTOM_RIGHT:
  case M3_BADGE_CORNER_BOTTOM_LEFT:
    return M3_OK;
  default:
    return M3_ERR_RANGE;
  }
}

int M3_CALL m3_tooltip_style_init_plain(M3TooltipStyle *style) {
  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->variant = M3_TOOLTIP_VARIANT_PLAIN;
  style->padding.left = M3_TOOLTIP_PLAIN_DEFAULT_PADDING_X;
  style->padding.right = M3_TOOLTIP_PLAIN_DEFAULT_PADDING_X;
  style->padding.top = M3_TOOLTIP_PLAIN_DEFAULT_PADDING_Y;
  style->padding.bottom = M3_TOOLTIP_PLAIN_DEFAULT_PADDING_Y;
  style->min_width = M3_TOOLTIP_PLAIN_DEFAULT_MIN_WIDTH;
  style->min_height = M3_TOOLTIP_PLAIN_DEFAULT_MIN_HEIGHT;
  style->max_width = M3_TOOLTIP_PLAIN_DEFAULT_MAX_WIDTH;
  style->max_height = M3_TOOLTIP_PLAIN_DEFAULT_MAX_HEIGHT;
  style->corner_radius = M3_TOOLTIP_PLAIN_DEFAULT_CORNER_RADIUS;
  style->anchor_gap = M3_TOOLTIP_PLAIN_DEFAULT_ANCHOR_GAP;
  style->title_body_spacing = M3_TOOLTIP_RICH_DEFAULT_TITLE_BODY_SPACING;

  style->text_style.utf8_family = NULL;
  style->text_style.size_px = 12;
  style->text_style.weight = 500;
  style->text_style.italic = M3_FALSE;
  style->text_style.color.r = 1.0f;
  style->text_style.color.g = 1.0f;
  style->text_style.color.b = 1.0f;
  style->text_style.color.a = 1.0f;

  style->title_style = style->text_style;
  style->body_style = style->text_style;

  style->background_color.r = 0.2f;
  style->background_color.g = 0.2f;
  style->background_color.b = 0.2f;
  style->background_color.a = 0.9f;

  return M3_OK;
}

int M3_CALL m3_tooltip_style_init_rich(M3TooltipStyle *style) {
  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->variant = M3_TOOLTIP_VARIANT_RICH;
  style->padding.left = M3_TOOLTIP_RICH_DEFAULT_PADDING_X;
  style->padding.right = M3_TOOLTIP_RICH_DEFAULT_PADDING_X;
  style->padding.top = M3_TOOLTIP_RICH_DEFAULT_PADDING_Y;
  style->padding.bottom = M3_TOOLTIP_RICH_DEFAULT_PADDING_Y;
  style->min_width = M3_TOOLTIP_RICH_DEFAULT_MIN_WIDTH;
  style->min_height = M3_TOOLTIP_RICH_DEFAULT_MIN_HEIGHT;
  style->max_width = M3_TOOLTIP_RICH_DEFAULT_MAX_WIDTH;
  style->max_height = M3_TOOLTIP_RICH_DEFAULT_MAX_HEIGHT;
  style->corner_radius = M3_TOOLTIP_RICH_DEFAULT_CORNER_RADIUS;
  style->anchor_gap = M3_TOOLTIP_RICH_DEFAULT_ANCHOR_GAP;
  style->title_body_spacing = M3_TOOLTIP_RICH_DEFAULT_TITLE_BODY_SPACING;

  style->title_style.utf8_family = NULL;
  style->title_style.size_px = 14;
  style->title_style.weight = 500;
  style->title_style.italic = M3_FALSE;
  style->title_style.color.r = 1.0f;
  style->title_style.color.g = 1.0f;
  style->title_style.color.b = 1.0f;
  style->title_style.color.a = 1.0f;

  style->body_style.utf8_family = NULL;
  style->body_style.size_px = 12;
  style->body_style.weight = 400;
  style->body_style.italic = M3_FALSE;
  style->body_style.color.r = 1.0f;
  style->body_style.color.g = 1.0f;
  style->body_style.color.b = 1.0f;
  style->body_style.color.a = 1.0f;

  style->text_style = style->body_style;

  style->background_color.r = 0.2f;
  style->background_color.g = 0.2f;
  style->background_color.b = 0.2f;
  style->background_color.a = 0.9f;

  return M3_OK;
}

int M3_CALL m3_tooltip_compute_content_size(const M3TooltipStyle *style,
                                            const M3TooltipContent *content,
                                            M3Size *out_size) {
  M3Scalar content_width;
  M3Scalar content_height;
  M3Scalar width;
  M3Scalar height;
  int rc;

  if (style == NULL || content == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_extras_validate_tooltip_style(style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_extras_validate_tooltip_content(style, content);
  if (rc != M3_OK) {
    return rc;
  }

  content_width = 0.0f;
  content_height = 0.0f;
  if (style->variant == M3_TOOLTIP_VARIANT_PLAIN) {
    content_width = content->body_metrics.width;
    content_height = content->body_metrics.height;
  } else {
    if (content->has_title == M3_TRUE) {
      content_width = content->title_metrics.width;
      content_height = content->title_metrics.height;
    }
    if (content->has_body == M3_TRUE) {
      if (content->body_metrics.width > content_width) {
        content_width = content->body_metrics.width;
      }
      if (content->has_title == M3_TRUE) {
        content_height += style->title_body_spacing;
      }
      content_height += content->body_metrics.height;
    }
  }

  width = style->padding.left + style->padding.right + content_width;
  height = style->padding.top + style->padding.bottom + content_height;

  if (style->min_width > 0.0f && width < style->min_width) {
    width = style->min_width;
  }
  if (style->min_height > 0.0f && height < style->min_height) {
    height = style->min_height;
  }
  if (style->max_width > 0.0f && width > style->max_width) {
    width = style->max_width;
  }
  if (style->max_height > 0.0f && height > style->max_height) {
    height = style->max_height;
  }

  if (width < 0.0f || height < 0.0f) {
    return M3_ERR_RANGE;
  }

  out_size->width = width;
  out_size->height = height;
  return M3_OK;
}

int M3_CALL m3_tooltip_compute_bounds(
    const M3TooltipStyle *style, const M3TooltipAnchor *anchor,
    const M3TooltipPlacement *placement, const M3Rect *overlay,
    const M3Size *tooltip_size, M3Rect *out_bounds, m3_u32 *out_direction) {
  M3Rect anchor_rect;
  M3Scalar width;
  M3Scalar height;
  M3Scalar anchor_x;
  M3Scalar anchor_y;
  M3Scalar anchor_w;
  M3Scalar anchor_h;
  M3Scalar space_before;
  M3Scalar space_after;
  M3Scalar x;
  M3Scalar y;
  M3Scalar min_x;
  M3Scalar max_x;
  M3Scalar min_y;
  M3Scalar max_y;
  m3_u32 direction;
  int rc;

  if (style == NULL || anchor == NULL || placement == NULL || overlay == NULL ||
      tooltip_size == NULL || out_bounds == NULL || out_direction == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_extras_validate_tooltip_style(style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_extras_validate_tooltip_anchor(anchor);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_extras_validate_tooltip_placement(placement);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_extras_validate_rect(overlay);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_extras_validate_size(tooltip_size);
  if (rc != M3_OK) {
    return rc;
  }

  width = tooltip_size->width;
  height = tooltip_size->height;
  if (overlay->width >= 0.0f && width > overlay->width) {
    width = overlay->width;
  }
  if (overlay->height >= 0.0f && height > overlay->height) {
    height = overlay->height;
  }

  if (anchor->type == M3_TOOLTIP_ANCHOR_RECT) {
    anchor_rect = anchor->rect;
  } else {
    anchor_rect.x = anchor->point.x;
    anchor_rect.y = anchor->point.y;
    anchor_rect.width = 0.0f;
    anchor_rect.height = 0.0f;
  }

  anchor_x = anchor_rect.x;
  anchor_y = anchor_rect.y;
  anchor_w = anchor_rect.width;
  anchor_h = anchor_rect.height;

  direction = placement->direction;
  if (direction == M3_TOOLTIP_DIRECTION_UP ||
      direction == M3_TOOLTIP_DIRECTION_DOWN) {
    space_before = anchor_y - overlay->y;
    space_after = overlay->y + overlay->height - (anchor_y + anchor_h);

    if (direction == M3_TOOLTIP_DIRECTION_DOWN) {
      if (height <= space_after) {
        direction = M3_TOOLTIP_DIRECTION_DOWN;
      } else if (height <= space_before) {
        direction = M3_TOOLTIP_DIRECTION_UP;
      } else if (space_before > space_after) {
        direction = M3_TOOLTIP_DIRECTION_UP;
      }
    } else {
      if (height <= space_before) {
        direction = M3_TOOLTIP_DIRECTION_UP;
      } else if (height <= space_after) {
        direction = M3_TOOLTIP_DIRECTION_DOWN;
      } else if (space_after > space_before) {
        direction = M3_TOOLTIP_DIRECTION_DOWN;
      }
    }

    if (placement->align == M3_TOOLTIP_ALIGN_START) {
      x = anchor_x;
    } else if (placement->align == M3_TOOLTIP_ALIGN_CENTER) {
      x = anchor_x + (anchor_w - width) * 0.5f;
    } else {
      x = anchor_x + anchor_w - width;
    }

    if (direction == M3_TOOLTIP_DIRECTION_DOWN) {
      y = anchor_y + anchor_h + style->anchor_gap;
    } else {
      y = anchor_y - height - style->anchor_gap;
    }
  } else {
    space_before = anchor_x - overlay->x;
    space_after = overlay->x + overlay->width - (anchor_x + anchor_w);

    if (direction == M3_TOOLTIP_DIRECTION_RIGHT) {
      if (width <= space_after) {
        direction = M3_TOOLTIP_DIRECTION_RIGHT;
      } else if (width <= space_before) {
        direction = M3_TOOLTIP_DIRECTION_LEFT;
      } else if (space_before > space_after) {
        direction = M3_TOOLTIP_DIRECTION_LEFT;
      }
    } else {
      if (width <= space_before) {
        direction = M3_TOOLTIP_DIRECTION_LEFT;
      } else if (width <= space_after) {
        direction = M3_TOOLTIP_DIRECTION_RIGHT;
      } else if (space_after > space_before) {
        direction = M3_TOOLTIP_DIRECTION_RIGHT;
      }
    }

    if (placement->align == M3_TOOLTIP_ALIGN_START) {
      y = anchor_y;
    } else if (placement->align == M3_TOOLTIP_ALIGN_CENTER) {
      y = anchor_y + (anchor_h - height) * 0.5f;
    } else {
      y = anchor_y + anchor_h - height;
    }

    if (direction == M3_TOOLTIP_DIRECTION_RIGHT) {
      x = anchor_x + anchor_w + style->anchor_gap;
    } else {
      x = anchor_x - width - style->anchor_gap;
    }
  }

  min_x = overlay->x;
  max_x = overlay->x + overlay->width - width;
  min_y = overlay->y;
  max_y = overlay->y + overlay->height - height;

  if (x < min_x) {
    x = min_x;
  }
  if (x > max_x) {
    x = max_x;
  }
  if (y < min_y) {
    y = min_y;
  }
  if (y > max_y) {
    y = max_y;
  }

  out_bounds->x = x;
  out_bounds->y = y;
  out_bounds->width = width;
  out_bounds->height = height;
  *out_direction = direction;
  return M3_OK;
}

int M3_CALL m3_badge_style_init(M3BadgeStyle *style) {
  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->min_size = M3_BADGE_DEFAULT_MIN_SIZE;
  style->dot_diameter = M3_BADGE_DEFAULT_DOT_DIAMETER;
  style->padding_x = M3_BADGE_DEFAULT_PADDING_X;
  style->padding_y = M3_BADGE_DEFAULT_PADDING_Y;
  style->corner_radius = M3_BADGE_DEFAULT_CORNER_RADIUS;

  style->text_style.utf8_family = NULL;
  style->text_style.size_px = 10;
  style->text_style.weight = 500;
  style->text_style.italic = M3_FALSE;
  style->text_style.color.r = 1.0f;
  style->text_style.color.g = 1.0f;
  style->text_style.color.b = 1.0f;
  style->text_style.color.a = 1.0f;

  style->background_color.r = 0.86f;
  style->background_color.g = 0.14f;
  style->background_color.b = 0.14f;
  style->background_color.a = 1.0f;

  return M3_OK;
}

int M3_CALL m3_badge_placement_init_icon(M3BadgePlacement *placement) {
  if (placement == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  placement->corner = M3_BADGE_CORNER_TOP_RIGHT;
  placement->offset_x = M3_BADGE_DEFAULT_ICON_OFFSET_X;
  placement->offset_y = M3_BADGE_DEFAULT_ICON_OFFSET_Y;
  return M3_OK;
}

int M3_CALL m3_badge_placement_init_navigation(M3BadgePlacement *placement) {
  if (placement == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  placement->corner = M3_BADGE_CORNER_TOP_RIGHT;
  placement->offset_x = M3_BADGE_DEFAULT_NAV_OFFSET_X;
  placement->offset_y = M3_BADGE_DEFAULT_NAV_OFFSET_Y;
  return M3_OK;
}

int M3_CALL m3_badge_compute_size(const M3BadgeStyle *style,
                                  const M3BadgeContent *content,
                                  M3Size *out_size) {
  M3Scalar width;
  M3Scalar height;
  int rc;

  if (style == NULL || content == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_extras_validate_badge_style(style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_extras_validate_badge_content(content);
  if (rc != M3_OK) {
    return rc;
  }

  if (content->has_text == M3_TRUE) {
    width = content->text_metrics.width + style->padding_x * 2.0f;
    height = content->text_metrics.height + style->padding_y * 2.0f;
    if (height < style->min_size) {
      height = style->min_size;
    }
    if (width < height) {
      width = height;
    }
  } else {
    width = style->dot_diameter;
    height = style->dot_diameter;
  }

  if (width < 0.0f || height < 0.0f) {
    return M3_ERR_RANGE;
  }

  out_size->width = width;
  out_size->height = height;
  return M3_OK;
}

int M3_CALL m3_badge_compute_bounds(const M3BadgeStyle *style,
                                    const M3BadgeContent *content,
                                    const M3Rect *anchor,
                                    const M3BadgePlacement *placement,
                                    M3Rect *out_bounds) {
  M3Size size;
  M3Scalar anchor_x;
  M3Scalar anchor_y;
  M3Scalar center_x;
  M3Scalar center_y;
  int rc;

  if (style == NULL || content == NULL || anchor == NULL || placement == NULL ||
      out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_extras_validate_rect(anchor);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_extras_validate_badge_placement(placement);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_badge_compute_size(style, content, &size);
  if (rc != M3_OK) {
    return rc;
  }

  switch (placement->corner) {
  case M3_BADGE_CORNER_TOP_LEFT:
    anchor_x = anchor->x;
    anchor_y = anchor->y;
    break;
  case M3_BADGE_CORNER_TOP_RIGHT:
    anchor_x = anchor->x + anchor->width;
    anchor_y = anchor->y;
    break;
  case M3_BADGE_CORNER_BOTTOM_RIGHT:
    anchor_x = anchor->x + anchor->width;
    anchor_y = anchor->y + anchor->height;
    break;
  case M3_BADGE_CORNER_BOTTOM_LEFT:
  default:
    anchor_x = anchor->x;
    anchor_y = anchor->y + anchor->height;
    break;
  }

  center_x = anchor_x + placement->offset_x;
  center_y = anchor_y + placement->offset_y;

  out_bounds->x = center_x - size.width * 0.5f;
  out_bounds->y = center_y - size.height * 0.5f;
  out_bounds->width = size.width;
  out_bounds->height = size.height;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_extras_test_validate_color(const M3Color *color) {
  return m3_extras_validate_color(color);
}

int M3_CALL m3_extras_test_validate_edges(const M3LayoutEdges *edges) {
  return m3_extras_validate_edges(edges);
}

int M3_CALL m3_extras_test_validate_text_style(const M3TextStyle *style,
                                               M3Bool require_family) {
  return m3_extras_validate_text_style(style, require_family);
}

int M3_CALL m3_extras_test_validate_rect(const M3Rect *rect) {
  return m3_extras_validate_rect(rect);
}

int M3_CALL
m3_extras_test_validate_tooltip_anchor(const M3TooltipAnchor *anchor) {
  return m3_extras_validate_tooltip_anchor(anchor);
}

int M3_CALL
m3_extras_test_validate_tooltip_placement(const M3TooltipPlacement *placement) {
  return m3_extras_validate_tooltip_placement(placement);
}

int M3_CALL m3_extras_test_validate_tooltip_style(const M3TooltipStyle *style,
                                                  M3Bool require_family) {
  return m3_extras_validate_tooltip_style(style, require_family);
}

int M3_CALL m3_extras_test_validate_tooltip_content(
    const M3TooltipStyle *style, const M3TooltipContent *content) {
  return m3_extras_validate_tooltip_content(style, content);
}

int M3_CALL m3_extras_test_validate_badge_style(const M3BadgeStyle *style,
                                                M3Bool require_family) {
  return m3_extras_validate_badge_style(style, require_family);
}

int M3_CALL
m3_extras_test_validate_badge_content(const M3BadgeContent *content) {
  return m3_extras_validate_badge_content(content);
}

int M3_CALL
m3_extras_test_validate_badge_placement(const M3BadgePlacement *placement) {
  return m3_extras_validate_badge_placement(placement);
}
#endif
