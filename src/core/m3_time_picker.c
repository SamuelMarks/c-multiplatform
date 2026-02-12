#include "m3/m3_time_picker.h"

#include <math.h>
#include <string.h>

#define M3_TIME_PICKER_PI 3.14159265358979323846f

static int m3_time_picker_validate_color(const M3Color *color) {
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

static int m3_time_picker_validate_edges(const M3LayoutEdges *edges) {
  if (edges == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_time_picker_validate_style(const M3TimePickerStyle *style) {
  M3Scalar outer_radius;
  M3Scalar inner_radius;
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->diameter <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->ring_thickness <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (!(style->inner_ring_ratio > 0.0f && style->inner_ring_ratio < 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (style->hand_thickness <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->hand_center_radius < 0.0f) {
    return M3_ERR_RANGE;
  }

  outer_radius = style->diameter * 0.5f;
  inner_radius = outer_radius * style->inner_ring_ratio;
  if (style->ring_thickness > outer_radius) {
    return M3_ERR_RANGE;
  }
  if (style->ring_thickness >= inner_radius) {
    return M3_ERR_RANGE;
  }
  if (style->hand_center_radius > outer_radius) {
    return M3_ERR_RANGE;
  }

  rc = m3_time_picker_validate_edges(&style->padding);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_time_picker_validate_color(&style->background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(&style->ring_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(&style->hand_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(&style->selection_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(&style->disabled_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_time_picker_validate_measure_spec(M3MeasureSpec spec) {
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY &&
      spec.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_time_picker_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_time_picker_validate_format(m3_u32 format) {
  if (format != M3_TIME_PICKER_FORMAT_12H &&
      format != M3_TIME_PICKER_FORMAT_24H) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_time_picker_validate_field(m3_u32 field) {
  if (field != M3_TIME_PICKER_FIELD_HOUR &&
      field != M3_TIME_PICKER_FIELD_MINUTE) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_time_picker_validate_period(m3_u32 period) {
  if (period != M3_TIME_PICKER_PERIOD_AM &&
      period != M3_TIME_PICKER_PERIOD_PM) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_time_picker_validate_hour(m3_u32 hour) {
  if (hour < M3_TIME_PICKER_MIN_HOUR || hour > M3_TIME_PICKER_MAX_HOUR) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_time_picker_validate_minute(m3_u32 minute) {
  if (minute < M3_TIME_PICKER_MIN_MINUTE ||
      minute > M3_TIME_PICKER_MAX_MINUTE) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_time_picker_validate_time(const M3Time *time) {
  int rc;

  if (time == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  rc = m3_time_picker_validate_hour(time->hour);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_minute(time->minute);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

static int m3_time_picker_compute_metrics(const M3TimePicker *picker,
                                          M3TimePickerMetrics *out_metrics) {
  M3Scalar avail_width;
  M3Scalar avail_height;
  M3Scalar dial_size;
  M3Scalar offset_x;
  M3Scalar offset_y;
  M3Scalar radius;
  int rc;

  if (picker == NULL || out_metrics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_style(&picker->style);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_rect(&picker->bounds);
  if (rc != M3_OK) {
    return rc;
  }

  avail_width = picker->bounds.width - picker->style.padding.left -
                picker->style.padding.right;
  avail_height = picker->bounds.height - picker->style.padding.top -
                 picker->style.padding.bottom;

  if (avail_width <= 0.0f || avail_height <= 0.0f) {
    memset(out_metrics, 0, sizeof(*out_metrics));
    return M3_OK;
  }

  dial_size = (avail_width < avail_height) ? avail_width : avail_height;
  if (dial_size <= 0.0f) {
    memset(out_metrics, 0, sizeof(*out_metrics));
    return M3_OK;
  }

  offset_x = picker->bounds.x + picker->style.padding.left +
             (avail_width - dial_size) * 0.5f;
  offset_y = picker->bounds.y + picker->style.padding.top +
             (avail_height - dial_size) * 0.5f;
  radius = dial_size * 0.5f;

  out_metrics->dial_bounds.x = offset_x;
  out_metrics->dial_bounds.y = offset_y;
  out_metrics->dial_bounds.width = dial_size;
  out_metrics->dial_bounds.height = dial_size;
  out_metrics->center_x = offset_x + radius;
  out_metrics->center_y = offset_y + radius;
  out_metrics->outer_radius = radius;
  out_metrics->inner_radius = radius * picker->style.inner_ring_ratio;

  return M3_OK;
}

static int m3_time_picker_angle_from_point(M3Scalar cx, M3Scalar cy, M3Scalar x,
                                           M3Scalar y, M3Scalar *out_angle) {
  double angle;
  double full;

  if (out_angle == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  angle = atan2((double)(y - cy), (double)(x - cx));
  angle += (double)(M3_TIME_PICKER_PI * 0.5f);
  full = (double)(M3_TIME_PICKER_PI * 2.0f);
  while (angle < 0.0) {
    angle += full;
  }
  *out_angle = (M3Scalar)angle;
  return M3_OK;
}

static int m3_time_picker_angle_to_index(M3Scalar angle, m3_u32 count,
                                         m3_u32 *out_index) {
  double full;
  double step;
  double norm;
  m3_u32 index;

  if (out_index == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (count == 0u) {
    return M3_ERR_RANGE;
  }

  full = (double)(M3_TIME_PICKER_PI * 2.0f);
  norm = (double)angle;
  while (norm < 0.0) {
    norm += full;
  }
  while (norm >= full) {
    norm -= full;
  }

  step = full / (double)count;
  index = (m3_u32)((norm + step * 0.5) / step);
  if (index >= count) {
    index = 0u;
  }
  *out_index = index;
  return M3_OK;
}

static int m3_time_picker_hour_from_index(m3_u32 index, m3_u32 format,
                                          M3Bool inner_ring,
                                          m3_u32 current_hour,
                                          m3_u32 *out_hour) {
  m3_u32 hour12;
  m3_u32 hour;
  int rc;

  if (out_hour == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (index >= 12u) {
    return M3_ERR_RANGE;
  }

  rc = m3_time_picker_validate_format(format);
  if (rc != M3_OK) {
    return rc;
  }

  if (format == M3_TIME_PICKER_FORMAT_12H) {
    rc = m3_time_picker_validate_hour(current_hour);
    if (rc != M3_OK) {
      return rc;
    }
    hour12 = (index == 0u) ? 12u : index;
    hour = hour12 % 12u;
    if (current_hour >= 12u) {
      hour += 12u;
    }
  } else {
    if (inner_ring == M3_TRUE) {
      hour = (index == 0u) ? 0u : (index + 12u);
    } else {
      hour = (index == 0u) ? 12u : index;
    }
  }

  rc = m3_time_picker_validate_hour(hour);
  if (rc != M3_OK) {
    return rc;
  }

  *out_hour = hour;
  return M3_OK;
}

static int m3_time_picker_hour_to_index(m3_u32 hour, m3_u32 format,
                                        m3_u32 *out_index, M3Bool *out_inner) {
  m3_u32 index;
  M3Bool inner;
  int rc;

  if (out_index == NULL || out_inner == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_hour(hour);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_format(format);
  if (rc != M3_OK) {
    return rc;
  }

  if (format == M3_TIME_PICKER_FORMAT_12H) {
    index = hour % 12u;
    inner = M3_FALSE;
  } else {
    if (hour == 0u) {
      index = 0u;
      inner = M3_TRUE;
    } else if (hour <= 12u) {
      index = hour % 12u;
      inner = M3_FALSE;
    } else {
      index = hour - 12u;
      inner = M3_TRUE;
    }
  }

  *out_index = index;
  *out_inner = inner;
  return M3_OK;
}

static int m3_time_picker_pick_time(const M3TimePicker *picker, M3Scalar x,
                                    M3Scalar y, m3_u32 field, M3Time *out_time,
                                    M3Bool *out_valid) {
  M3TimePickerMetrics metrics;
  M3Time time;
  M3Scalar dx;
  M3Scalar dy;
  M3Scalar dist;
  M3Scalar angle;
  M3Scalar threshold;
  m3_u32 index;
  M3Bool inner_ring;
  int rc;

  if (picker == NULL || out_time == NULL || out_valid == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_field(field);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_format(picker->format);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_time(&picker->time);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_time_picker_compute_metrics(picker, &metrics);
  if (rc != M3_OK) {
    return rc;
  }

  *out_valid = M3_FALSE;
  time = picker->time;

  if (metrics.outer_radius <= 0.0f) {
    *out_time = time;
    return M3_OK;
  }

  dx = x - metrics.center_x;
  dy = y - metrics.center_y;
  dist = (M3Scalar)sqrt((double)dx * (double)dx + (double)dy * (double)dy);

  if (dist > metrics.outer_radius) {
    *out_time = time;
    return M3_OK;
  }

  rc = m3_time_picker_angle_from_point(metrics.center_x, metrics.center_y, x, y,
                                       &angle);
  if (rc != M3_OK) {
    return rc;
  }

  if (field == M3_TIME_PICKER_FIELD_HOUR) {
    rc = m3_time_picker_angle_to_index(angle, 12u, &index);
    if (rc != M3_OK) {
      return rc;
    }

    inner_ring = M3_FALSE;
    if (picker->format == M3_TIME_PICKER_FORMAT_24H &&
        metrics.inner_radius > 0.0f) {
      threshold = (metrics.outer_radius + metrics.inner_radius) * 0.5f;
      if (dist < threshold) {
        inner_ring = M3_TRUE;
      }
    }

    rc = m3_time_picker_hour_from_index(index, picker->format, inner_ring,
                                        picker->time.hour, &time.hour);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    rc = m3_time_picker_angle_to_index(angle, 60u, &index);
    if (rc != M3_OK) {
      return rc;
    }
    time.minute = index;
  }

  *out_time = time;
  *out_valid = M3_TRUE;
  return M3_OK;
}

static int m3_time_picker_apply_time(M3TimePicker *picker, const M3Time *time) {
  M3Time prev;
  int rc;

  if (picker == NULL || time == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_time(time);
  if (rc != M3_OK) {
    return rc;
  }

  prev = picker->time;
  picker->time = *time;

  if (picker->on_change != NULL) {
    rc = picker->on_change(picker->on_change_ctx, picker, &picker->time);
    if (rc != M3_OK) {
      picker->time = prev;
      return rc;
    }
  }

  return M3_OK;
}

static int m3_time_picker_update_metrics(M3TimePicker *picker) {
  M3TimePickerMetrics metrics;
  int rc;

  if (picker == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_time(&picker->time);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_time_picker_compute_metrics(picker, &metrics);
  if (rc != M3_OK) {
    return rc;
  }

  picker->metrics = metrics;
  return M3_OK;
}

static int m3_time_picker_draw_circle(M3Gfx *gfx, M3Scalar cx, M3Scalar cy,
                                      M3Scalar radius, M3Color color) {
  M3Rect rect;
  int rc;

  if (gfx == NULL || gfx->vtable == NULL || gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }
  if (radius < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (radius == 0.0f) {
    return M3_OK;
  }

  rect.x = cx - radius;
  rect.y = cy - radius;
  rect.width = radius * 2.0f;
  rect.height = radius * 2.0f;

  rc = m3_time_picker_validate_rect(&rect);
  if (rc != M3_OK) {
    return rc;
  }

  return gfx->vtable->draw_rect(gfx->ctx, &rect, color, radius);
}

static int m3_time_picker_draw_ring(M3Gfx *gfx, M3Scalar cx, M3Scalar cy,
                                    M3Scalar radius, M3Scalar thickness,
                                    M3Color ring_color, M3Color fill_color) {
  M3Scalar inner_radius;
  int rc;

  if (thickness <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (radius <= 0.0f) {
    return M3_OK;
  }

  if (thickness > radius) {
    thickness = radius;
  }

  rc = m3_time_picker_draw_circle(gfx, cx, cy, radius, ring_color);
  if (rc != M3_OK) {
    return rc;
  }

  inner_radius = radius - thickness;
  if (inner_radius <= 0.0f) {
    return M3_OK;
  }

  return m3_time_picker_draw_circle(gfx, cx, cy, inner_radius, fill_color);
}

static int m3_time_picker_resolve_colors(const M3TimePicker *picker,
                                         M3Color *out_background,
                                         M3Color *out_ring, M3Color *out_hand,
                                         M3Color *out_selection) {
  int rc;

  if (picker == NULL || out_background == NULL || out_ring == NULL ||
      out_hand == NULL || out_selection == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (picker->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    *out_background = picker->style.disabled_color;
    *out_ring = picker->style.disabled_color;
    *out_hand = picker->style.disabled_color;
    *out_selection = picker->style.disabled_color;
  } else {
    *out_background = picker->style.background_color;
    *out_ring = picker->style.ring_color;
    *out_hand = picker->style.hand_color;
    *out_selection = picker->style.selection_color;
  }

  rc = m3_time_picker_validate_color(out_background);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(out_ring);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(out_hand);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(out_selection);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_time_picker_compute_hand(const M3TimePicker *picker,
                                       const M3TimePickerMetrics *metrics,
                                       M3Scalar *out_angle,
                                       M3Scalar *out_radius) {
  M3Scalar step;
  M3Scalar radius;
  m3_u32 index;
  M3Bool inner_ring;
  int rc;

  if (picker == NULL || metrics == NULL || out_angle == NULL ||
      out_radius == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_field(picker->active_field);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_format(picker->format);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_time(&picker->time);
  if (rc != M3_OK) {
    return rc;
  }

  radius = metrics->outer_radius;
  if (radius <= 0.0f) {
    *out_angle = 0.0f;
    *out_radius = 0.0f;
    return M3_OK;
  }

  if (picker->active_field == M3_TIME_PICKER_FIELD_HOUR) {
    rc = m3_time_picker_hour_to_index(picker->time.hour, picker->format, &index,
                                      &inner_ring);
    if (rc != M3_OK) {
      return rc;
    }
    step = (M3Scalar)(M3_TIME_PICKER_PI * 2.0f) / 12.0f;
    *out_angle = (M3Scalar)index * step;
    if (picker->format == M3_TIME_PICKER_FORMAT_24H && inner_ring == M3_TRUE &&
        metrics->inner_radius > 0.0f) {
      radius = metrics->inner_radius;
    }
  } else {
    rc = m3_time_picker_validate_minute(picker->time.minute);
    if (rc != M3_OK) {
      return rc;
    }
    step = (M3Scalar)(M3_TIME_PICKER_PI * 2.0f) / 60.0f;
    *out_angle = (M3Scalar)picker->time.minute * step;
  }

  radius -= picker->style.ring_thickness * 0.5f;
  if (radius < 0.0f) {
    radius = 0.0f;
  }

  *out_radius = radius;
  return M3_OK;
}

static int m3_time_picker_widget_measure(void *widget, M3MeasureSpec width,
                                         M3MeasureSpec height,
                                         M3Size *out_size) {
  M3TimePicker *picker;
  M3Scalar desired;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  picker = (M3TimePicker *)widget;
  rc = m3_time_picker_validate_style(&picker->style);
  if (rc != M3_OK) {
    return rc;
  }

  desired = picker->style.diameter + picker->style.padding.left +
            picker->style.padding.right;
  rc = m3_time_picker_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }

  if (width.mode == M3_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == M3_MEASURE_AT_MOST) {
    out_size->width = desired;
    if (out_size->width > width.size) {
      out_size->width = width.size;
    }
  } else {
    out_size->width = desired;
  }

  desired = picker->style.diameter + picker->style.padding.top +
            picker->style.padding.bottom;
  rc = m3_time_picker_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc;
  }

  if (height.mode == M3_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == M3_MEASURE_AT_MOST) {
    out_size->height = desired;
    if (out_size->height > height.size) {
      out_size->height = height.size;
    }
  } else {
    out_size->height = desired;
  }

  return M3_OK;
}

static int m3_time_picker_widget_layout(void *widget, M3Rect bounds) {
  M3TimePicker *picker;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  picker = (M3TimePicker *)widget;
  picker->bounds = bounds;
  return m3_time_picker_update_metrics(picker);
}

static int m3_time_picker_widget_paint(void *widget, M3PaintContext *ctx) {
  M3TimePicker *picker;
  M3TimePickerMetrics metrics;
  M3Color background;
  M3Color ring;
  M3Color hand;
  M3Color selection;
  M3Scalar angle;
  M3Scalar hand_radius;
  M3Scalar angle_std;
  M3Scalar end_x;
  M3Scalar end_y;
  int rc;

  if (widget == NULL || ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL ||
      ctx->gfx->vtable->draw_line == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  picker = (M3TimePicker *)widget;
  rc = m3_time_picker_validate_style(&picker->style);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_rect(&picker->bounds);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_update_metrics(picker);
  if (rc != M3_OK) {
    return rc;
  }

  metrics = picker->metrics;
  if (metrics.outer_radius <= 0.0f) {
    return M3_OK;
  }

  rc = m3_time_picker_resolve_colors(picker, &background, &ring, &hand,
                                     &selection);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_time_picker_draw_ring(ctx->gfx, metrics.center_x, metrics.center_y,
                                metrics.outer_radius,
                                picker->style.ring_thickness, ring, background);
  if (rc != M3_OK) {
    return rc;
  }

  if (picker->format == M3_TIME_PICKER_FORMAT_24H &&
      metrics.inner_radius > 0.0f) {
    rc = m3_time_picker_draw_ring(
        ctx->gfx, metrics.center_x, metrics.center_y, metrics.inner_radius,
        picker->style.ring_thickness, ring, background);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_time_picker_compute_hand(picker, &metrics, &angle, &hand_radius);
  if (rc != M3_OK) {
    return rc;
  }

  angle_std = angle - (M3_TIME_PICKER_PI * 0.5f);
  end_x = metrics.center_x +
          (M3Scalar)(cos((double)angle_std) * (double)hand_radius);
  end_y = metrics.center_y +
          (M3Scalar)(sin((double)angle_std) * (double)hand_radius);

  rc = ctx->gfx->vtable->draw_line(ctx->gfx->ctx, metrics.center_x,
                                   metrics.center_y, end_x, end_y, hand,
                                   picker->style.hand_thickness);
  if (rc != M3_OK) {
    return rc;
  }

  if (picker->style.hand_center_radius > 0.0f) {
    rc =
        m3_time_picker_draw_circle(ctx->gfx, metrics.center_x, metrics.center_y,
                                   picker->style.hand_center_radius, selection);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

static int m3_time_picker_widget_event(void *widget, const M3InputEvent *event,
                                       M3Bool *out_handled) {
  M3TimePicker *picker;
  M3Time next_time;
  M3Bool valid;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;
  picker = (M3TimePicker *)widget;

  if (picker->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_OK;
  }

  switch (event->type) {
  case M3_INPUT_POINTER_DOWN:
    if (picker->pressed == M3_TRUE) {
      return M3_ERR_STATE;
    }
    rc = m3_time_picker_pick_time(picker, (M3Scalar)event->data.pointer.x,
                                  (M3Scalar)event->data.pointer.y,
                                  picker->active_field, &next_time, &valid);
    if (rc != M3_OK) {
      return rc;
    }
    if (valid == M3_TRUE) {
      rc = m3_time_picker_apply_time(picker, &next_time);
      if (rc != M3_OK) {
        return rc;
      }
      picker->pressed = M3_TRUE;
      *out_handled = M3_TRUE;
    }
    return M3_OK;
  case M3_INPUT_POINTER_MOVE:
    if (picker->pressed == M3_FALSE) {
      return M3_OK;
    }
    rc = m3_time_picker_pick_time(picker, (M3Scalar)event->data.pointer.x,
                                  (M3Scalar)event->data.pointer.y,
                                  picker->active_field, &next_time, &valid);
    if (rc != M3_OK) {
      return rc;
    }
    if (valid == M3_TRUE) {
      rc = m3_time_picker_apply_time(picker, &next_time);
      if (rc != M3_OK) {
        return rc;
      }
    }
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_POINTER_UP:
    if (picker->pressed == M3_FALSE) {
      return M3_OK;
    }
    picker->pressed = M3_FALSE;
    *out_handled = M3_TRUE;
    return M3_OK;
  default:
    return M3_OK;
  }
}

static int m3_time_picker_widget_get_semantics(void *widget,
                                               M3Semantics *out_semantics) {
  M3TimePicker *picker;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  picker = (M3TimePicker *)widget;
  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0u;
  if (picker->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_time_picker_widget_destroy(void *widget) {
  M3TimePicker *picker;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  picker = (M3TimePicker *)widget;
  memset(&picker->style, 0, sizeof(picker->style));
  memset(&picker->metrics, 0, sizeof(picker->metrics));
  memset(&picker->bounds, 0, sizeof(picker->bounds));
  picker->format = 0u;
  picker->active_field = 0u;
  picker->time.hour = 0u;
  picker->time.minute = 0u;
  picker->pressed = M3_FALSE;
  picker->on_change = NULL;
  picker->on_change_ctx = NULL;
  picker->widget.ctx = NULL;
  picker->widget.vtable = NULL;
  picker->widget.handle.id = 0u;
  picker->widget.handle.generation = 0u;
  picker->widget.flags = 0u;
  return M3_OK;
}

static const M3WidgetVTable g_m3_time_picker_widget_vtable = {
    m3_time_picker_widget_measure,       m3_time_picker_widget_layout,
    m3_time_picker_widget_paint,         m3_time_picker_widget_event,
    m3_time_picker_widget_get_semantics, m3_time_picker_widget_destroy};

int M3_CALL m3_time_picker_style_init(M3TimePickerStyle *style) {
  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->padding.left = M3_TIME_PICKER_DEFAULT_PADDING;
  style->padding.top = M3_TIME_PICKER_DEFAULT_PADDING;
  style->padding.right = M3_TIME_PICKER_DEFAULT_PADDING;
  style->padding.bottom = M3_TIME_PICKER_DEFAULT_PADDING;
  style->diameter = M3_TIME_PICKER_DEFAULT_DIAMETER;
  style->ring_thickness = M3_TIME_PICKER_DEFAULT_RING_THICKNESS;
  style->inner_ring_ratio = M3_TIME_PICKER_DEFAULT_INNER_RING_RATIO;
  style->hand_thickness = M3_TIME_PICKER_DEFAULT_HAND_THICKNESS;
  style->hand_center_radius = M3_TIME_PICKER_DEFAULT_HAND_CENTER_RADIUS;
  style->background_color.r = 0.0f;
  style->background_color.g = 0.0f;
  style->background_color.b = 0.0f;
  style->background_color.a = 0.0f;
  style->ring_color = style->background_color;
  style->hand_color = style->background_color;
  style->selection_color = style->background_color;
  style->disabled_color = style->background_color;
  return M3_OK;
}

int M3_CALL m3_time_picker_init(M3TimePicker *picker,
                                const M3TimePickerStyle *style, m3_u32 hour,
                                m3_u32 minute, m3_u32 format) {
  int rc;

  if (picker == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_hour(hour);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_minute(minute);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_format(format);
  if (rc != M3_OK) {
    return rc;
  }

  memset(picker, 0, sizeof(*picker));
  picker->style = *style;
  picker->format = format;
  picker->active_field = M3_TIME_PICKER_FIELD_HOUR;
  picker->time.hour = hour;
  picker->time.minute = minute;
  picker->pressed = M3_FALSE;
  picker->widget.ctx = picker;
  picker->widget.vtable = &g_m3_time_picker_widget_vtable;
  picker->widget.handle.id = 0u;
  picker->widget.handle.generation = 0u;
  picker->widget.flags = M3_WIDGET_FLAG_FOCUSABLE;

  return m3_time_picker_update_metrics(picker);
}

int M3_CALL m3_time_picker_set_style(M3TimePicker *picker,
                                     const M3TimePickerStyle *style) {
  M3TimePickerStyle prev;
  int rc;

  if (picker == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  prev = picker->style;
  picker->style = *style;

  rc = m3_time_picker_update_metrics(picker);
  if (rc != M3_OK) {
    picker->style = prev;
    m3_time_picker_update_metrics(picker);
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_time_picker_set_format(M3TimePicker *picker, m3_u32 format) {
  m3_u32 prev;
  int rc;

  if (picker == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_format(format);
  if (rc != M3_OK) {
    return rc;
  }

  prev = picker->format;
  picker->format = format;

  rc = m3_time_picker_update_metrics(picker);
  if (rc != M3_OK) {
    picker->format = prev;
    m3_time_picker_update_metrics(picker);
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_time_picker_get_format(const M3TimePicker *picker,
                                      m3_u32 *out_format) {
  if (picker == NULL || out_format == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_format = picker->format;
  return M3_OK;
}

int M3_CALL m3_time_picker_set_active_field(M3TimePicker *picker,
                                            m3_u32 field) {
  int rc;

  if (picker == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_field(field);
  if (rc != M3_OK) {
    return rc;
  }

  picker->active_field = field;
  return M3_OK;
}

int M3_CALL m3_time_picker_get_active_field(const M3TimePicker *picker,
                                            m3_u32 *out_field) {
  if (picker == NULL || out_field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_field = picker->active_field;
  return M3_OK;
}

int M3_CALL m3_time_picker_set_time(M3TimePicker *picker, const M3Time *time) {
  return m3_time_picker_apply_time(picker, time);
}

int M3_CALL m3_time_picker_get_time(const M3TimePicker *picker,
                                    M3Time *out_time) {
  if (picker == NULL || out_time == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_time = picker->time;
  return M3_OK;
}

int M3_CALL m3_time_picker_set_period(M3TimePicker *picker, m3_u32 period) {
  M3Time next;
  int rc;

  if (picker == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_format(picker->format);
  if (rc != M3_OK) {
    return rc;
  }
  if (picker->format != M3_TIME_PICKER_FORMAT_12H) {
    return M3_ERR_STATE;
  }

  rc = m3_time_picker_validate_period(period);
  if (rc != M3_OK) {
    return rc;
  }

  next = picker->time;
  if (period == M3_TIME_PICKER_PERIOD_AM) {
    if (next.hour >= 12u) {
      next.hour -= 12u;
    }
  } else {
    if (next.hour < 12u) {
      next.hour += 12u;
    }
  }

  return m3_time_picker_apply_time(picker, &next);
}

int M3_CALL m3_time_picker_get_period(const M3TimePicker *picker,
                                      m3_u32 *out_period) {
  if (picker == NULL || out_period == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (picker->time.hour >= 12u) {
    *out_period = M3_TIME_PICKER_PERIOD_PM;
  } else {
    *out_period = M3_TIME_PICKER_PERIOD_AM;
  }
  return M3_OK;
}

int M3_CALL m3_time_picker_set_on_change(M3TimePicker *picker,
                                         M3TimePickerOnChange on_change,
                                         void *ctx) {
  if (picker == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  picker->on_change = on_change;
  picker->on_change_ctx = ctx;
  return M3_OK;
}

int M3_CALL m3_time_picker_update(M3TimePicker *picker) {
  return m3_time_picker_update_metrics(picker);
}

#ifdef M3_TESTING
int M3_CALL m3_time_picker_test_validate_color(const M3Color *color) {
  return m3_time_picker_validate_color(color);
}

int M3_CALL m3_time_picker_test_validate_edges(const M3LayoutEdges *edges) {
  return m3_time_picker_validate_edges(edges);
}

int M3_CALL m3_time_picker_test_validate_style(const M3TimePickerStyle *style) {
  return m3_time_picker_validate_style(style);
}

int M3_CALL m3_time_picker_test_validate_measure_spec(M3MeasureSpec spec) {
  return m3_time_picker_validate_measure_spec(spec);
}

int M3_CALL m3_time_picker_test_validate_rect(const M3Rect *rect) {
  return m3_time_picker_validate_rect(rect);
}

int M3_CALL m3_time_picker_test_validate_time(const M3Time *time) {
  return m3_time_picker_validate_time(time);
}

int M3_CALL m3_time_picker_test_angle_from_point(M3Scalar cx, M3Scalar cy,
                                                 M3Scalar x, M3Scalar y,
                                                 M3Scalar *out_angle) {
  return m3_time_picker_angle_from_point(cx, cy, x, y, out_angle);
}

int M3_CALL m3_time_picker_test_angle_to_index(M3Scalar angle, m3_u32 count,
                                               m3_u32 *out_index) {
  return m3_time_picker_angle_to_index(angle, count, out_index);
}

int M3_CALL m3_time_picker_test_hour_from_index(m3_u32 index, m3_u32 format,
                                                M3Bool inner_ring,
                                                m3_u32 current_hour,
                                                m3_u32 *out_hour) {
  return m3_time_picker_hour_from_index(index, format, inner_ring, current_hour,
                                        out_hour);
}

int M3_CALL m3_time_picker_test_hour_to_index(m3_u32 hour, m3_u32 format,
                                              m3_u32 *out_index,
                                              M3Bool *out_inner) {
  return m3_time_picker_hour_to_index(hour, format, out_index, out_inner);
}

int M3_CALL m3_time_picker_test_compute_metrics(
    const M3TimePicker *picker, M3TimePickerMetrics *out_metrics) {
  return m3_time_picker_compute_metrics(picker, out_metrics);
}

int M3_CALL m3_time_picker_test_pick_time(const M3TimePicker *picker,
                                          M3Scalar x, M3Scalar y, m3_u32 field,
                                          M3Time *out_time, M3Bool *out_valid) {
  return m3_time_picker_pick_time(picker, x, y, field, out_time, out_valid);
}

int M3_CALL m3_time_picker_test_resolve_colors(const M3TimePicker *picker,
                                               M3Color *out_background,
                                               M3Color *out_ring,
                                               M3Color *out_hand,
                                               M3Color *out_selection) {
  return m3_time_picker_resolve_colors(picker, out_background, out_ring,
                                       out_hand, out_selection);
}

int M3_CALL m3_time_picker_test_compute_hand(const M3TimePicker *picker,
                                             const M3TimePickerMetrics *metrics,
                                             M3Scalar *out_angle,
                                             M3Scalar *out_radius) {
  return m3_time_picker_compute_hand(picker, metrics, out_angle, out_radius);
}

int M3_CALL m3_time_picker_test_draw_circle(M3Gfx *gfx, M3Scalar cx,
                                            M3Scalar cy, M3Scalar radius,
                                            M3Color color) {
  return m3_time_picker_draw_circle(gfx, cx, cy, radius, color);
}

int M3_CALL m3_time_picker_test_draw_ring(M3Gfx *gfx, M3Scalar cx, M3Scalar cy,
                                          M3Scalar radius, M3Scalar thickness,
                                          M3Color ring_color,
                                          M3Color fill_color) {
  return m3_time_picker_draw_ring(gfx, cx, cy, radius, thickness, ring_color,
                                  fill_color);
}

int M3_CALL m3_time_picker_test_update_metrics(M3TimePicker *picker) {
  return m3_time_picker_update_metrics(picker);
}
#endif
