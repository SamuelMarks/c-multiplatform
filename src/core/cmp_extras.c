#include "cmpc/cmp_extras.h"

#include <string.h>

#ifdef CMP_TESTING
static CMPBool g_cmp_extras_test_force_tooltip_size_range = CMP_FALSE;
static CMPBool g_cmp_extras_test_force_badge_size_range = CMP_FALSE;

int CMP_CALL cmp_extras_test_set_tooltip_size_range_fail(CMPBool enable) {
  g_cmp_extras_test_force_tooltip_size_range = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_extras_test_set_badge_size_range_fail(CMPBool enable) {
  g_cmp_extras_test_force_badge_size_range = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}
#endif

static int cmp_extras_validate_bool(CMPBool value) {
  if (value != CMP_FALSE && value != CMP_TRUE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int cmp_extras_validate_color(const CMPColor *color) {
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

static int cmp_extras_validate_edges(const CMPLayoutEdges *edges) {
  if (edges == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_extras_validate_text_style(const CMPTextStyle *style,
                                         CMPBool require_family) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (require_family == CMP_TRUE && style->utf8_family == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return CMP_ERR_RANGE;
  }
  if (style->italic != CMP_FALSE && style->italic != CMP_TRUE) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_extras_validate_color(&style->color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int cmp_extras_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_extras_validate_size(const CMPSize *size) {
  if (size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size->width < 0.0f || size->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_extras_validate_text_metrics(const CMPTextMetrics *metrics) {
  if (metrics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (metrics->width < 0.0f || metrics->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (metrics->baseline < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_extras_validate_tooltip_anchor(const CMPTooltipAnchor *anchor) {
  int rc;

  if (anchor == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (anchor->type == CMP_TOOLTIP_ANCHOR_RECT) {
    rc = cmp_extras_validate_rect(&anchor->rect);
    if (rc != CMP_OK) {
      return rc;
    }
    return CMP_OK;
  }
  if (anchor->type == CMP_TOOLTIP_ANCHOR_POINT) {
    return CMP_OK;
  }
  return CMP_ERR_RANGE;
}

static int
cmp_extras_validate_tooltip_placement(const CMPTooltipPlacement *placement) {
  if (placement == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (placement->direction != CMP_TOOLTIP_DIRECTION_UP &&
      placement->direction != CMP_TOOLTIP_DIRECTION_DOWN &&
      placement->direction != CMP_TOOLTIP_DIRECTION_LEFT &&
      placement->direction != CMP_TOOLTIP_DIRECTION_RIGHT) {
    return CMP_ERR_RANGE;
  }
  if (placement->align != CMP_TOOLTIP_ALIGN_START &&
      placement->align != CMP_TOOLTIP_ALIGN_CENTER &&
      placement->align != CMP_TOOLTIP_ALIGN_END) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_extras_validate_tooltip_style(const CMPTooltipStyle *style,
                                            CMPBool require_family) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (style->variant != CMP_TOOLTIP_VARIANT_PLAIN &&
      style->variant != CMP_TOOLTIP_VARIANT_RICH) {
    return CMP_ERR_RANGE;
  }
  if (style->min_width < 0.0f || style->min_height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->max_width < 0.0f || style->max_height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->max_width > 0.0f && style->max_width < style->min_width) {
    return CMP_ERR_RANGE;
  }
  if (style->max_height > 0.0f && style->max_height < style->min_height) {
    return CMP_ERR_RANGE;
  }
  if (style->corner_radius < 0.0f || style->anchor_gap < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->title_body_spacing < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_extras_validate_edges(&style->padding);
  if (rc != CMP_OK) {
    return rc;
  }

  if (style->variant == CMP_TOOLTIP_VARIANT_PLAIN) {
    rc = cmp_extras_validate_text_style(&style->text_style, require_family);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    rc = cmp_extras_validate_text_style(&style->title_style, require_family);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_extras_validate_text_style(&style->body_style, require_family);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = cmp_extras_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int cmp_extras_validate_tooltip_content(const CMPTooltipStyle *style,
                                              const CMPTooltipContent *content) {
  int rc;

  if (style == NULL || content == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_extras_validate_bool(content->has_title);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_extras_validate_bool(content->has_body);
  if (rc != CMP_OK) {
    return rc;
  }

  if (content->has_title == CMP_TRUE) {
    rc = cmp_extras_validate_text_metrics(&content->title_metrics);
    if (rc != CMP_OK) {
      return rc;
    }
  }
  if (content->has_body == CMP_TRUE) {
    rc = cmp_extras_validate_text_metrics(&content->body_metrics);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  if (style->variant == CMP_TOOLTIP_VARIANT_PLAIN) {
    if (content->has_body != CMP_TRUE) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
  } else {
    if (content->has_title != CMP_TRUE && content->has_body != CMP_TRUE) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }

  return CMP_OK;
}

static int cmp_extras_validate_badge_style(const CMPBadgeStyle *style,
                                          CMPBool require_family) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->min_size < 0.0f || style->dot_diameter < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->padding_x < 0.0f || style->padding_y < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_extras_validate_text_style(&style->text_style, require_family);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_extras_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int cmp_extras_validate_badge_content(const CMPBadgeContent *content) {
  int rc;

  if (content == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  rc = cmp_extras_validate_bool(content->has_text);
  if (rc != CMP_OK) {
    return rc;
  }
  if (content->has_text == CMP_TRUE) {
    rc = cmp_extras_validate_text_metrics(&content->text_metrics);
    if (rc != CMP_OK) {
      return rc;
    }
  }
  return CMP_OK;
}

static int
cmp_extras_validate_badge_placement(const CMPBadgePlacement *placement) {
  if (placement == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  switch (placement->corner) {
  case CMP_BADGE_CORNER_TOP_LEFT:
  case CMP_BADGE_CORNER_TOP_RIGHT:
  case CMP_BADGE_CORNER_BOTTOM_RIGHT:
  case CMP_BADGE_CORNER_BOTTOM_LEFT:
    return CMP_OK;
  default:
    return CMP_ERR_RANGE;
  }
}

int CMP_CALL cmp_tooltip_style_init_plain(CMPTooltipStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->variant = CMP_TOOLTIP_VARIANT_PLAIN;
  style->padding.left = CMP_TOOLTIP_PLAIN_DEFAULT_PADDING_X;
  style->padding.right = CMP_TOOLTIP_PLAIN_DEFAULT_PADDING_X;
  style->padding.top = CMP_TOOLTIP_PLAIN_DEFAULT_PADDING_Y;
  style->padding.bottom = CMP_TOOLTIP_PLAIN_DEFAULT_PADDING_Y;
  style->min_width = CMP_TOOLTIP_PLAIN_DEFAULT_MIN_WIDTH;
  style->min_height = CMP_TOOLTIP_PLAIN_DEFAULT_MIN_HEIGHT;
  style->max_width = CMP_TOOLTIP_PLAIN_DEFAULT_MAX_WIDTH;
  style->max_height = CMP_TOOLTIP_PLAIN_DEFAULT_MAX_HEIGHT;
  style->corner_radius = CMP_TOOLTIP_PLAIN_DEFAULT_CORNER_RADIUS;
  style->anchor_gap = CMP_TOOLTIP_PLAIN_DEFAULT_ANCHOR_GAP;
  style->title_body_spacing = CMP_TOOLTIP_RICH_DEFAULT_TITLE_BODY_SPACING;

  style->text_style.utf8_family = NULL;
  style->text_style.size_px = 12;
  style->text_style.weight = 500;
  style->text_style.italic = CMP_FALSE;
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

  return CMP_OK;
}

int CMP_CALL cmp_tooltip_style_init_rich(CMPTooltipStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->variant = CMP_TOOLTIP_VARIANT_RICH;
  style->padding.left = CMP_TOOLTIP_RICH_DEFAULT_PADDING_X;
  style->padding.right = CMP_TOOLTIP_RICH_DEFAULT_PADDING_X;
  style->padding.top = CMP_TOOLTIP_RICH_DEFAULT_PADDING_Y;
  style->padding.bottom = CMP_TOOLTIP_RICH_DEFAULT_PADDING_Y;
  style->min_width = CMP_TOOLTIP_RICH_DEFAULT_MIN_WIDTH;
  style->min_height = CMP_TOOLTIP_RICH_DEFAULT_MIN_HEIGHT;
  style->max_width = CMP_TOOLTIP_RICH_DEFAULT_MAX_WIDTH;
  style->max_height = CMP_TOOLTIP_RICH_DEFAULT_MAX_HEIGHT;
  style->corner_radius = CMP_TOOLTIP_RICH_DEFAULT_CORNER_RADIUS;
  style->anchor_gap = CMP_TOOLTIP_RICH_DEFAULT_ANCHOR_GAP;
  style->title_body_spacing = CMP_TOOLTIP_RICH_DEFAULT_TITLE_BODY_SPACING;

  style->title_style.utf8_family = NULL;
  style->title_style.size_px = 14;
  style->title_style.weight = 500;
  style->title_style.italic = CMP_FALSE;
  style->title_style.color.r = 1.0f;
  style->title_style.color.g = 1.0f;
  style->title_style.color.b = 1.0f;
  style->title_style.color.a = 1.0f;

  style->body_style.utf8_family = NULL;
  style->body_style.size_px = 12;
  style->body_style.weight = 400;
  style->body_style.italic = CMP_FALSE;
  style->body_style.color.r = 1.0f;
  style->body_style.color.g = 1.0f;
  style->body_style.color.b = 1.0f;
  style->body_style.color.a = 1.0f;

  style->text_style = style->body_style;

  style->background_color.r = 0.2f;
  style->background_color.g = 0.2f;
  style->background_color.b = 0.2f;
  style->background_color.a = 0.9f;

  return CMP_OK;
}

int CMP_CALL cmp_tooltip_compute_content_size(const CMPTooltipStyle *style,
                                            const CMPTooltipContent *content,
                                            CMPSize *out_size) {
  CMPScalar content_width;
  CMPScalar content_height;
  CMPScalar width;
  CMPScalar height;
  int rc;

  if (style == NULL || content == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_extras_validate_tooltip_style(style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_extras_validate_tooltip_content(style, content);
  if (rc != CMP_OK) {
    return rc;
  }

  content_width = 0.0f;
  content_height = 0.0f;
  if (style->variant == CMP_TOOLTIP_VARIANT_PLAIN) {
    content_width = content->body_metrics.width;
    content_height = content->body_metrics.height;
  } else {
    if (content->has_title == CMP_TRUE) {
      content_width = content->title_metrics.width;
      content_height = content->title_metrics.height;
    }
    if (content->has_body == CMP_TRUE) {
      if (content->body_metrics.width > content_width) {
        content_width = content->body_metrics.width;
      }
      if (content->has_title == CMP_TRUE) {
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

#ifdef CMP_TESTING
  if (g_cmp_extras_test_force_tooltip_size_range == CMP_TRUE) {
    g_cmp_extras_test_force_tooltip_size_range = CMP_FALSE;
    width = -1.0f;
  }
#endif

  if (width < 0.0f || height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  out_size->width = width;
  out_size->height = height;
  return CMP_OK;
}

int CMP_CALL cmp_tooltip_compute_bounds(
    const CMPTooltipStyle *style, const CMPTooltipAnchor *anchor,
    const CMPTooltipPlacement *placement, const CMPRect *overlay,
    const CMPSize *tooltip_size, CMPRect *out_bounds, cmp_u32 *out_direction) {
  CMPRect anchor_rect;
  CMPScalar width;
  CMPScalar height;
  CMPScalar anchor_x; /* GCOVR_EXCL_LINE */
  CMPScalar anchor_y;
  CMPScalar anchor_w;
  CMPScalar anchor_h;
  CMPScalar space_before;
  CMPScalar space_after;
  CMPScalar x;
  CMPScalar y;
  CMPScalar min_x;
  CMPScalar max_x;
  CMPScalar min_y;
  CMPScalar max_y;
  cmp_u32 direction;
  int rc;

  if (style == NULL || anchor == NULL || placement == NULL || overlay == NULL ||
      tooltip_size == NULL || out_bounds == NULL || out_direction == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_extras_validate_tooltip_style(style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_extras_validate_tooltip_anchor(anchor);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_extras_validate_tooltip_placement(placement);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_extras_validate_rect(overlay);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_extras_validate_size(tooltip_size);
  if (rc != CMP_OK) {
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

  if (anchor->type == CMP_TOOLTIP_ANCHOR_RECT) {
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
  if (direction == CMP_TOOLTIP_DIRECTION_UP ||
      direction == CMP_TOOLTIP_DIRECTION_DOWN) {
    space_before = anchor_y - overlay->y;
    space_after = overlay->y + overlay->height - (anchor_y + anchor_h);

    if (direction == CMP_TOOLTIP_DIRECTION_DOWN) {
      if (height <= space_after) {
        direction = CMP_TOOLTIP_DIRECTION_DOWN;
      } else if (height <= space_before) {
        direction = CMP_TOOLTIP_DIRECTION_UP;
      } else if (space_before > space_after) {
        direction = CMP_TOOLTIP_DIRECTION_UP;
      }
    } else {
      if (height <= space_before) {
        direction = CMP_TOOLTIP_DIRECTION_UP;
      } else if (height <= space_after) {
        direction = CMP_TOOLTIP_DIRECTION_DOWN;
      } else if (space_after > space_before) {
        direction = CMP_TOOLTIP_DIRECTION_DOWN;
      }
    }

    if (placement->align == CMP_TOOLTIP_ALIGN_START) {
      x = anchor_x;
    } else if (placement->align == CMP_TOOLTIP_ALIGN_CENTER) {
      x = anchor_x + (anchor_w - width) * 0.5f;
    } else {
      x = anchor_x + anchor_w - width;
    }

    if (direction == CMP_TOOLTIP_DIRECTION_DOWN) {
      y = anchor_y + anchor_h + style->anchor_gap;
    } else {
      y = anchor_y - height - style->anchor_gap;
    }
  } else {
    space_before = anchor_x - overlay->x;
    space_after = overlay->x + overlay->width - (anchor_x + anchor_w);

    if (direction == CMP_TOOLTIP_DIRECTION_RIGHT) {
      if (width <= space_after) {
        direction = CMP_TOOLTIP_DIRECTION_RIGHT;
      } else if (width <= space_before) {
        direction = CMP_TOOLTIP_DIRECTION_LEFT;
      } else if (space_before > space_after) {
        direction = CMP_TOOLTIP_DIRECTION_LEFT;
      }
    } else {
      if (width <= space_before) {
        direction = CMP_TOOLTIP_DIRECTION_LEFT;
      } else if (width <= space_after) {
        direction = CMP_TOOLTIP_DIRECTION_RIGHT;
      } else if (space_after > space_before) {
        direction = CMP_TOOLTIP_DIRECTION_RIGHT;
      }
    }

    if (placement->align == CMP_TOOLTIP_ALIGN_START) {
      y = anchor_y;
    } else if (placement->align == CMP_TOOLTIP_ALIGN_CENTER) {
      y = anchor_y + (anchor_h - height) * 0.5f;
    } else {
      y = anchor_y + anchor_h - height;
    }

    if (direction == CMP_TOOLTIP_DIRECTION_RIGHT) {
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
  return CMP_OK;
}

int CMP_CALL cmp_badge_style_init(CMPBadgeStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->min_size = CMP_BADGE_DEFAULT_MIN_SIZE;
  style->dot_diameter = CMP_BADGE_DEFAULT_DOT_DIAMETER;
  style->padding_x = CMP_BADGE_DEFAULT_PADDING_X;
  style->padding_y = CMP_BADGE_DEFAULT_PADDING_Y;
  style->corner_radius = CMP_BADGE_DEFAULT_CORNER_RADIUS;

  style->text_style.utf8_family = NULL;
  style->text_style.size_px = 10;
  style->text_style.weight = 500;
  style->text_style.italic = CMP_FALSE;
  style->text_style.color.r = 1.0f;
  style->text_style.color.g = 1.0f;
  style->text_style.color.b = 1.0f;
  style->text_style.color.a = 1.0f;

  style->background_color.r = 0.86f;
  style->background_color.g = 0.14f;
  style->background_color.b = 0.14f;
  style->background_color.a = 1.0f;

  return CMP_OK;
}

int CMP_CALL cmp_badge_placement_init_icon(CMPBadgePlacement *placement) {
  if (placement == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  placement->corner = CMP_BADGE_CORNER_TOP_RIGHT;
  placement->offset_x = CMP_BADGE_DEFAULT_ICON_OFFSET_X;
  placement->offset_y = CMP_BADGE_DEFAULT_ICON_OFFSET_Y;
  return CMP_OK;
}

int CMP_CALL cmp_badge_placement_init_navigation(CMPBadgePlacement *placement) {
  if (placement == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  placement->corner = CMP_BADGE_CORNER_TOP_RIGHT;
  placement->offset_x = CMP_BADGE_DEFAULT_NAV_OFFSET_X;
  placement->offset_y = CMP_BADGE_DEFAULT_NAV_OFFSET_Y;
  return CMP_OK;
}

int CMP_CALL cmp_badge_compute_size(const CMPBadgeStyle *style,
                                  const CMPBadgeContent *content,
                                  CMPSize *out_size) {
  CMPScalar width;
  CMPScalar height;
  int rc;

  if (style == NULL || content == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_extras_validate_badge_style(style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_extras_validate_badge_content(content);
  if (rc != CMP_OK) {
    return rc;
  }

  if (content->has_text == CMP_TRUE) {
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

#ifdef CMP_TESTING
  if (g_cmp_extras_test_force_badge_size_range == CMP_TRUE) {
    g_cmp_extras_test_force_badge_size_range = CMP_FALSE;
    width = -1.0f;
  }
#endif

  if (width < 0.0f || height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  out_size->width = width;
  out_size->height = height;
  return CMP_OK;
}

int CMP_CALL cmp_badge_compute_bounds(const CMPBadgeStyle *style,
                                    const CMPBadgeContent *content,
                                    const CMPRect *anchor,
                                    const CMPBadgePlacement *placement,
                                    CMPRect *out_bounds) {
  CMPSize size;
  CMPScalar anchor_x;
  CMPScalar anchor_y;
  CMPScalar center_x;
  CMPScalar center_y;
  int rc;

  if (style == NULL || content == NULL || anchor == NULL || placement == NULL ||
      out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_extras_validate_rect(anchor);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_extras_validate_badge_placement(placement);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_badge_compute_size(style, content, &size);
  if (rc != CMP_OK) {
    return rc;
  }

  switch (placement->corner) {
  case CMP_BADGE_CORNER_TOP_LEFT:
    anchor_x = anchor->x;
    anchor_y = anchor->y;
    break;
  case CMP_BADGE_CORNER_TOP_RIGHT:
    anchor_x = anchor->x + anchor->width;
    anchor_y = anchor->y;
    break;
  case CMP_BADGE_CORNER_BOTTOM_RIGHT:
    anchor_x = anchor->x + anchor->width;
    anchor_y = anchor->y + anchor->height;
    break;
  case CMP_BADGE_CORNER_BOTTOM_LEFT:
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
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_extras_test_validate_color(const CMPColor *color) {
  return cmp_extras_validate_color(color);
}

int CMP_CALL cmp_extras_test_validate_edges(const CMPLayoutEdges *edges) {
  return cmp_extras_validate_edges(edges);
}

int CMP_CALL cmp_extras_test_validate_text_style(const CMPTextStyle *style,
                                               CMPBool require_family) {
  return cmp_extras_validate_text_style(style, require_family);
}

int CMP_CALL cmp_extras_test_validate_rect(const CMPRect *rect) {
  return cmp_extras_validate_rect(rect);
}

int CMP_CALL cmp_extras_test_validate_size(const CMPSize *size) {
  return cmp_extras_validate_size(size);
}

int CMP_CALL cmp_extras_test_validate_text_metrics(const CMPTextMetrics *metrics) {
  return cmp_extras_validate_text_metrics(metrics);
}

int CMP_CALL
cmp_extras_test_validate_tooltip_anchor(const CMPTooltipAnchor *anchor) {
  return cmp_extras_validate_tooltip_anchor(anchor);
}

int CMP_CALL
cmp_extras_test_validate_tooltip_placement(const CMPTooltipPlacement *placement) {
  return cmp_extras_validate_tooltip_placement(placement);
}

int CMP_CALL cmp_extras_test_validate_tooltip_style(const CMPTooltipStyle *style,
                                                  CMPBool require_family) {
  return cmp_extras_validate_tooltip_style(style, require_family);
}

int CMP_CALL cmp_extras_test_validate_tooltip_content(
    const CMPTooltipStyle *style, const CMPTooltipContent *content) {
  return cmp_extras_validate_tooltip_content(style, content);
}

int CMP_CALL cmp_extras_test_validate_badge_style(const CMPBadgeStyle *style,
                                                CMPBool require_family) {
  return cmp_extras_validate_badge_style(style, require_family);
}

int CMP_CALL
cmp_extras_test_validate_badge_content(const CMPBadgeContent *content) {
  return cmp_extras_validate_badge_content(content);
}

int CMP_CALL
cmp_extras_test_validate_badge_placement(const CMPBadgePlacement *placement) {
  return cmp_extras_validate_badge_placement(placement);
}
#endif
