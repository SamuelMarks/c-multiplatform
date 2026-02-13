#include "m3/m3_time_picker.h"

#include <math.h>
#include <string.h>

#define M3_TIME_PICKER_PI 3.14159265358979323846f

#ifdef CMP_TESTING
static CMPBool g_m3_time_picker_test_force_dial_size_zero = CMP_FALSE;
static CMPBool g_m3_time_picker_test_force_angle_error = CMP_FALSE;
static CMPBool g_m3_time_picker_test_force_angle_index_error = CMP_FALSE;
static CMPBool g_m3_time_picker_test_force_hour_invalid = CMP_FALSE;
static CMPBool g_m3_time_picker_test_force_hour_to_index_error = CMP_FALSE;
static CMPBool g_m3_time_picker_test_force_rect_error = CMP_FALSE;
static CMPBool g_m3_time_picker_test_force_resolve_colors_error = CMP_FALSE;
static CMPBool g_m3_time_picker_test_skip_time_minute = CMP_FALSE;
static CMPBool g_m3_time_picker_test_force_minute_error = CMP_FALSE;
#endif

static int m3_time_picker_validate_color(const CMPColor *color) {
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

static int m3_time_picker_validate_edges(const CMPLayoutEdges *edges) {
  if (edges == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_time_picker_validate_style(const M3TimePickerStyle *style) {
  CMPScalar outer_radius;
  CMPScalar inner_radius;
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->diameter <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->ring_thickness <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (!(style->inner_ring_ratio > 0.0f && style->inner_ring_ratio < 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (style->hand_thickness <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->hand_center_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }

  outer_radius = style->diameter * 0.5f;
  inner_radius = outer_radius * style->inner_ring_ratio;
  if (style->ring_thickness > outer_radius) {
    return CMP_ERR_RANGE;
  }
  if (style->ring_thickness >= inner_radius) {
    return CMP_ERR_RANGE;
  }
  if (style->hand_center_radius > outer_radius) {
    return CMP_ERR_RANGE;
  }

  rc = m3_time_picker_validate_edges(&style->padding);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_time_picker_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(&style->ring_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(&style->hand_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(&style->selection_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(&style->disabled_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_time_picker_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.mode != CMP_MEASURE_EXACTLY &&
      spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_time_picker_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_time_picker_validate_format(cmp_u32 format) {
  if (format != M3_TIME_PICKER_FORMAT_12H &&
      format != M3_TIME_PICKER_FORMAT_24H) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_time_picker_validate_field(cmp_u32 field) {
  if (field != M3_TIME_PICKER_FIELD_HOUR &&
      field != M3_TIME_PICKER_FIELD_MINUTE) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_time_picker_validate_period(cmp_u32 period) {
  if (period != M3_TIME_PICKER_PERIOD_AM &&
      period != M3_TIME_PICKER_PERIOD_PM) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_time_picker_validate_hour(cmp_u32 hour) {
  if (hour < M3_TIME_PICKER_MIN_HOUR || hour > M3_TIME_PICKER_MAX_HOUR) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_time_picker_validate_minute(cmp_u32 minute) {
#ifdef CMP_TESTING
  if (g_m3_time_picker_test_force_minute_error == CMP_TRUE) {
    g_m3_time_picker_test_force_minute_error = CMP_FALSE;
    return CMP_ERR_RANGE;
  }
#endif
  if (minute < M3_TIME_PICKER_MIN_MINUTE ||
      minute > M3_TIME_PICKER_MAX_MINUTE) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_time_picker_validate_time(const CMPTime *time) {
  int rc;

  if (time == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  rc = m3_time_picker_validate_hour(time->hour);
  if (rc != CMP_OK) {
    return rc;
  }
#ifdef CMP_TESTING
  if (g_m3_time_picker_test_skip_time_minute == CMP_TRUE) {
    g_m3_time_picker_test_skip_time_minute = CMP_FALSE;
    return CMP_OK;
  }
#endif
  rc = m3_time_picker_validate_minute(time->minute);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int m3_time_picker_compute_metrics(const M3TimePicker *picker,
                                          M3TimePickerMetrics *out_metrics) {
  CMPScalar avail_width;
  CMPScalar avail_height;
  CMPScalar dial_size;
  CMPScalar offset_x;
  CMPScalar offset_y;
  CMPScalar radius;
  int rc;

  if (picker == NULL || out_metrics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_style(&picker->style);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_rect(&picker->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  avail_width = picker->bounds.width - picker->style.padding.left -
                picker->style.padding.right;
  avail_height = picker->bounds.height - picker->style.padding.top -
                 picker->style.padding.bottom;

  if (avail_width <= 0.0f || avail_height <= 0.0f) {
    memset(out_metrics, 0, sizeof(*out_metrics));
    return CMP_OK;
  }

  dial_size = (avail_width < avail_height) ? avail_width : avail_height;
#ifdef CMP_TESTING
  if (g_m3_time_picker_test_force_dial_size_zero == CMP_TRUE) {
    g_m3_time_picker_test_force_dial_size_zero = CMP_FALSE;
    dial_size = 0.0f;
  }
#endif
  if (dial_size <= 0.0f) {
    memset(out_metrics, 0, sizeof(*out_metrics));
    return CMP_OK;
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

  return CMP_OK;
}

static int m3_time_picker_angle_from_point(CMPScalar cx, CMPScalar cy, CMPScalar x,
                                           CMPScalar y, CMPScalar *out_angle) {
  double angle;
  double full;

  if (out_angle == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_m3_time_picker_test_force_angle_error == CMP_TRUE) {
    g_m3_time_picker_test_force_angle_error = CMP_FALSE;
    return CMP_ERR_RANGE;
  }
#endif

  angle = atan2((double)(y - cy), (double)(x - cx));
  angle += (double)(M3_TIME_PICKER_PI * 0.5f);
  full = (double)(M3_TIME_PICKER_PI * 2.0f);
  while (angle < 0.0) {
    angle += full;
  }
  *out_angle = (CMPScalar)angle;
  return CMP_OK;
}

static int m3_time_picker_angle_to_index(CMPScalar angle, cmp_u32 count,
                                         cmp_u32 *out_index) {
  double full;
  double step;
  double norm;
  cmp_u32 index;

  if (out_index == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_m3_time_picker_test_force_angle_index_error == CMP_TRUE) {
    g_m3_time_picker_test_force_angle_index_error = CMP_FALSE;
    return CMP_ERR_RANGE;
  }
#endif
  if (count == 0u) {
    return CMP_ERR_RANGE;
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
  index = (cmp_u32)((norm + step * 0.5) / step);
  if (index >= count) {
    index = 0u;
  }
  *out_index = index;
  return CMP_OK;
}

static int m3_time_picker_hour_from_index(cmp_u32 index, cmp_u32 format,
                                          CMPBool inner_ring,
                                          cmp_u32 current_hour,
                                          cmp_u32 *out_hour) {
  cmp_u32 hour12;
  cmp_u32 hour;
  int rc;

  if (out_hour == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (index >= 12u) {
    return CMP_ERR_RANGE;
  }

  rc = m3_time_picker_validate_format(format);
  if (rc != CMP_OK) {
    return rc;
  }

  if (format == M3_TIME_PICKER_FORMAT_12H) {
    rc = m3_time_picker_validate_hour(current_hour);
    if (rc != CMP_OK) {
      return rc;
    }
    hour12 = (index == 0u) ? 12u : index;
    hour = hour12 % 12u;
    if (current_hour >= 12u) {
      hour += 12u;
    }
  } else {
    if (inner_ring == CMP_TRUE) {
      hour = (index == 0u) ? 0u : (index + 12u);
    } else {
      hour = (index == 0u) ? 12u : index;
    }
  }

#ifdef CMP_TESTING
  if (g_m3_time_picker_test_force_hour_invalid == CMP_TRUE) {
    g_m3_time_picker_test_force_hour_invalid = CMP_FALSE;
    hour = M3_TIME_PICKER_MAX_HOUR + 1u;
  }
#endif
  rc = m3_time_picker_validate_hour(hour);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_hour = hour;
  return CMP_OK;
}

static int m3_time_picker_hour_to_index(cmp_u32 hour, cmp_u32 format,
                                        cmp_u32 *out_index, CMPBool *out_inner) {
  cmp_u32 index;
  CMPBool inner;
  int rc;

  if (out_index == NULL || out_inner == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_m3_time_picker_test_force_hour_to_index_error == CMP_TRUE) {
    g_m3_time_picker_test_force_hour_to_index_error = CMP_FALSE;
    return CMP_ERR_RANGE;
  }
#endif

  rc = m3_time_picker_validate_hour(hour);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_format(format);
  if (rc != CMP_OK) {
    return rc;
  }

  if (format == M3_TIME_PICKER_FORMAT_12H) {
    index = hour % 12u;
    inner = CMP_FALSE;
  } else {
    if (hour == 0u) {
      index = 0u;
      inner = CMP_TRUE;
    } else if (hour <= 12u) {
      index = hour % 12u;
      inner = CMP_FALSE;
    } else {
      index = hour - 12u;
      inner = CMP_TRUE;
    }
  }

  *out_index = index;
  *out_inner = inner;
  return CMP_OK;
}

static int m3_time_picker_pick_time(const M3TimePicker *picker, CMPScalar x,
                                    CMPScalar y, cmp_u32 field, CMPTime *out_time,
                                    CMPBool *out_valid) { /* GCOVR_EXCL_LINE */
  M3TimePickerMetrics metrics;
  CMPTime time;
  CMPScalar dx;
  CMPScalar dy;
  CMPScalar dist;
  CMPScalar angle = 0.0f;
  CMPScalar threshold;
  cmp_u32 index;
  CMPBool inner_ring;
  int rc;

  if (picker == NULL || out_time == NULL || out_valid == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_field(field);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_format(picker->format);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_time(&picker->time);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_time_picker_compute_metrics(picker, &metrics);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_valid = CMP_FALSE;
  time = picker->time;

  if (metrics.outer_radius <= 0.0f) {
    *out_time = time;
    return CMP_OK;
  }

  dx = x - metrics.center_x;
  dy = y - metrics.center_y;
  dist = (CMPScalar)sqrt((double)dx * (double)dx + (double)dy * (double)dy);

  if (dist > metrics.outer_radius) {
    *out_time = time;
    return CMP_OK;
  }

  rc = m3_time_picker_angle_from_point(metrics.center_x, metrics.center_y, x, y,
                                       &angle);
  if (rc != CMP_OK) {
    return rc;
  }

  if (field == M3_TIME_PICKER_FIELD_HOUR) {
    rc = m3_time_picker_angle_to_index(angle, 12u, &index);
    if (rc != CMP_OK) {
      return rc;
    }

    inner_ring = CMP_FALSE;
    if (picker->format == M3_TIME_PICKER_FORMAT_24H &&
        metrics.inner_radius > 0.0f) {
      threshold = (metrics.outer_radius + metrics.inner_radius) * 0.5f;
      if (dist < threshold) {
        inner_ring = CMP_TRUE;
      }
    }

    rc = m3_time_picker_hour_from_index(index, picker->format, inner_ring,
                                        picker->time.hour, &time.hour);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    rc = m3_time_picker_angle_to_index(angle, 60u, &index);
    if (rc != CMP_OK) {
      return rc;
    }
    time.minute = index;
  }

  *out_time = time;
  *out_valid = CMP_TRUE;
  return CMP_OK;
}

static int m3_time_picker_apply_time(M3TimePicker *picker, const CMPTime *time) {
  CMPTime prev;
  int rc;

  if (picker == NULL || time == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_time(time);
  if (rc != CMP_OK) {
    return rc;
  }

  prev = picker->time;
  picker->time = *time;

  if (picker->on_change != NULL) {
    rc = picker->on_change(picker->on_change_ctx, picker, &picker->time);
    if (rc != CMP_OK) {
      picker->time = prev;
      return rc;
    }
  }

  return CMP_OK;
}

static int m3_time_picker_update_metrics(M3TimePicker *picker) {
  M3TimePickerMetrics metrics;
  int rc;

  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_time(&picker->time);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_time_picker_compute_metrics(picker, &metrics);
  if (rc != CMP_OK) {
    return rc;
  }

  picker->metrics = metrics;
  return CMP_OK;
}

static int m3_time_picker_draw_circle(CMPGfx *gfx, CMPScalar cx, CMPScalar cy,
                                      CMPScalar radius, CMPColor color) {
  CMPRect rect;
  int rc;

  if (gfx == NULL || gfx->vtable == NULL || gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (radius < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (radius == 0.0f) {
    return CMP_OK;
  }

  rect.x = cx - radius;
  rect.y = cy - radius;
  rect.width = radius * 2.0f;
  rect.height = radius * 2.0f;

#ifdef CMP_TESTING
  if (g_m3_time_picker_test_force_rect_error == CMP_TRUE) {
    g_m3_time_picker_test_force_rect_error = CMP_FALSE;
    rect.width = -1.0f;
  }
#endif
  rc = m3_time_picker_validate_rect(&rect);
  if (rc != CMP_OK) {
    return rc;
  }

  return gfx->vtable->draw_rect(gfx->ctx, &rect, color, radius);
}

static int m3_time_picker_draw_ring(CMPGfx *gfx, CMPScalar cx, CMPScalar cy,
                                    CMPScalar radius, CMPScalar thickness,
                                    CMPColor ring_color, CMPColor fill_color) {
  CMPScalar inner_radius = 0.0f;
  int rc;

  if (thickness <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (radius <= 0.0f) {
    return CMP_OK;
  }

  if (thickness > radius) {
    thickness = radius;
  }

  rc = m3_time_picker_draw_circle(gfx, cx, cy, radius, ring_color);
  if (rc != CMP_OK) {
    return rc;
  }

  inner_radius = radius - thickness;
  if (inner_radius <= 0.0f) {
    return CMP_OK;
  }

  return m3_time_picker_draw_circle(gfx, cx, cy, inner_radius, fill_color);
}

static int m3_time_picker_resolve_colors(const M3TimePicker *picker,
                                         CMPColor *out_background,
                                         CMPColor *out_ring, CMPColor *out_hand,
                                         CMPColor *out_selection) {
  int rc;

  if (picker == NULL || out_background == NULL || out_ring == NULL ||
      out_hand == NULL || out_selection == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#ifdef CMP_TESTING
  if (g_m3_time_picker_test_force_resolve_colors_error == CMP_TRUE) {
    g_m3_time_picker_test_force_resolve_colors_error = CMP_FALSE;
    return CMP_ERR_RANGE;
  }
#endif
  if (picker->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
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
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(out_ring);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(out_hand);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_color(out_selection);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_time_picker_compute_hand(const M3TimePicker *picker,
                                       const M3TimePickerMetrics *metrics,
                                       CMPScalar *out_angle,
                                       CMPScalar *out_radius) {
  CMPScalar step;
  CMPScalar radius;
  cmp_u32 index;
  CMPBool inner_ring = CMP_FALSE;
  int rc;

  if (picker == NULL || metrics == NULL || out_angle == NULL ||
      out_radius == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_field(picker->active_field);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_format(picker->format);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_time(&picker->time);
  if (rc != CMP_OK) {
    return rc;
  }

  radius = metrics->outer_radius;
  if (radius <= 0.0f) {
    *out_angle = 0.0f;
    *out_radius = 0.0f;
    return CMP_OK;
  }

  if (picker->active_field == M3_TIME_PICKER_FIELD_HOUR) {
    rc = m3_time_picker_hour_to_index(picker->time.hour, picker->format, &index,
                                      &inner_ring);
    if (rc != CMP_OK) {
      return rc;
    }
    step = (CMPScalar)(M3_TIME_PICKER_PI * 2.0f) / 12.0f;
    *out_angle = (CMPScalar)index * step;
    if (picker->format == M3_TIME_PICKER_FORMAT_24H && inner_ring == CMP_TRUE &&
        metrics->inner_radius > 0.0f) {
      radius = metrics->inner_radius;
    }
  } else {
    rc = m3_time_picker_validate_minute(picker->time.minute);
    if (rc != CMP_OK) {
      return rc;
    }
    step = (CMPScalar)(M3_TIME_PICKER_PI * 2.0f) / 60.0f;
    *out_angle = (CMPScalar)picker->time.minute * step;
  }

  radius -= picker->style.ring_thickness * 0.5f;
  if (radius < 0.0f) {
    radius = 0.0f;
  }

  *out_radius = radius;
  return CMP_OK;
}

static int m3_time_picker_widget_measure(void *widget, CMPMeasureSpec width,
                                         CMPMeasureSpec height,
                                         CMPSize *out_size) {
  M3TimePicker *picker;
  CMPScalar desired;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  picker = (M3TimePicker *)widget;
  rc = m3_time_picker_validate_style(&picker->style);
  if (rc != CMP_OK) {
    return rc;
  }

  desired = picker->style.diameter + picker->style.padding.left +
            picker->style.padding.right;
  rc = m3_time_picker_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
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
  if (rc != CMP_OK) {
    return rc;
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == CMP_MEASURE_AT_MOST) {
    out_size->height = desired;
    if (out_size->height > height.size) {
      out_size->height = height.size;
    }
  } else {
    out_size->height = desired;
  }

  return CMP_OK;
}

static int m3_time_picker_widget_layout(void *widget, CMPRect bounds) {
  M3TimePicker *picker;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  picker = (M3TimePicker *)widget;
  picker->bounds = bounds;
  return m3_time_picker_update_metrics(picker);
}

static int m3_time_picker_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3TimePicker *picker;
  M3TimePickerMetrics metrics;
  CMPColor background = {0.0f, 0.0f, 0.0f, 0.0f};
  CMPColor ring;
  CMPColor hand;
  CMPColor selection;
  CMPScalar angle;
  CMPScalar hand_radius;
  CMPScalar angle_std;
  CMPScalar end_x;
  CMPScalar end_y;
  int rc;

  if (widget == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL ||
      ctx->gfx->vtable->draw_line == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  picker = (M3TimePicker *)widget;
  rc = m3_time_picker_validate_style(&picker->style);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_rect(&picker->bounds);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_update_metrics(picker);
  if (rc != CMP_OK) {
    return rc;
  }

  metrics = picker->metrics;
  if (metrics.outer_radius <= 0.0f) {
    return CMP_OK;
  }

  rc = m3_time_picker_resolve_colors(picker, &background, &ring, &hand,
                                     &selection);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_time_picker_draw_ring(ctx->gfx, metrics.center_x, metrics.center_y,
                                metrics.outer_radius,
                                picker->style.ring_thickness, ring, background);
  if (rc != CMP_OK) {
    return rc;
  }

  if (picker->format == M3_TIME_PICKER_FORMAT_24H &&
      metrics.inner_radius > 0.0f) {
    rc = m3_time_picker_draw_ring(
        ctx->gfx, metrics.center_x, metrics.center_y, metrics.inner_radius,
        picker->style.ring_thickness, ring, background);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = m3_time_picker_compute_hand(picker, &metrics, &angle, &hand_radius);
  if (rc != CMP_OK) {
    return rc;
  }

  angle_std = angle - (M3_TIME_PICKER_PI * 0.5f);
  end_x = metrics.center_x +
          (CMPScalar)(cos((double)angle_std) * (double)hand_radius);
  end_y = metrics.center_y +
          (CMPScalar)(sin((double)angle_std) * (double)hand_radius);

  rc = ctx->gfx->vtable->draw_line(ctx->gfx->ctx, metrics.center_x,
                                   metrics.center_y, end_x, end_y, hand,
                                   picker->style.hand_thickness);
  if (rc != CMP_OK) {
    return rc;
  }

  if (picker->style.hand_center_radius > 0.0f) {
    rc =
        m3_time_picker_draw_circle(ctx->gfx, metrics.center_x, metrics.center_y,
                                   picker->style.hand_center_radius, selection);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

static int m3_time_picker_widget_event(void *widget, const CMPInputEvent *event,
                                       CMPBool *out_handled) {
  M3TimePicker *picker;
  CMPTime next_time;
  CMPBool valid;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;
  picker = (M3TimePicker *)widget;

  if (picker->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN:
    if (picker->pressed == CMP_TRUE) {
      return CMP_ERR_STATE;
    }
    rc = m3_time_picker_pick_time(picker, (CMPScalar)event->data.pointer.x,
                                  (CMPScalar)event->data.pointer.y,
                                  picker->active_field, &next_time, &valid);
    if (rc != CMP_OK) {
      return rc;
    }
    if (valid == CMP_TRUE) {
      rc = m3_time_picker_apply_time(picker, &next_time);
      if (rc != CMP_OK) {
        return rc;
      }
      picker->pressed = CMP_TRUE;
      *out_handled = CMP_TRUE;
    }
    return CMP_OK;
  case CMP_INPUT_POINTER_MOVE:
    if (picker->pressed == CMP_FALSE) {
      return CMP_OK;
    }
    rc = m3_time_picker_pick_time(picker, (CMPScalar)event->data.pointer.x,
                                  (CMPScalar)event->data.pointer.y,
                                  picker->active_field, &next_time, &valid);
    if (rc != CMP_OK) {
      return rc;
    }
    if (valid == CMP_TRUE) {
      rc = m3_time_picker_apply_time(picker, &next_time);
      if (rc != CMP_OK) {
        return rc;
      }
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_POINTER_UP:
    if (picker->pressed == CMP_FALSE) {
      return CMP_OK;
    }
    picker->pressed = CMP_FALSE;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  default:
    return CMP_OK;
  }
}

static int m3_time_picker_widget_get_semantics(void *widget,
                                               CMPSemantics *out_semantics) {
  M3TimePicker *picker;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  picker = (M3TimePicker *)widget;
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0u;
  if (picker->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_time_picker_widget_destroy(void *widget) {
  M3TimePicker *picker;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  picker = (M3TimePicker *)widget;
  memset(&picker->style, 0, sizeof(picker->style));
  memset(&picker->metrics, 0, sizeof(picker->metrics));
  memset(&picker->bounds, 0, sizeof(picker->bounds));
  picker->format = 0u;
  picker->active_field = 0u;
  picker->time.hour = 0u;
  picker->time.minute = 0u;
  picker->pressed = CMP_FALSE;
  picker->on_change = NULL;
  picker->on_change_ctx = NULL;
  picker->widget.ctx = NULL;
  picker->widget.vtable = NULL;
  picker->widget.handle.id = 0u;
  picker->widget.handle.generation = 0u;
  picker->widget.flags = 0u;
  return CMP_OK;
}

static const CMPWidgetVTable g_m3_time_picker_widget_vtable = {
    m3_time_picker_widget_measure,       m3_time_picker_widget_layout,
    m3_time_picker_widget_paint,         m3_time_picker_widget_event,
    m3_time_picker_widget_get_semantics, m3_time_picker_widget_destroy};

int CMP_CALL m3_time_picker_style_init(M3TimePickerStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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
  return CMP_OK;
}

int CMP_CALL m3_time_picker_init(M3TimePicker *picker,
                                const M3TimePickerStyle *style, cmp_u32 hour,
                                cmp_u32 minute, cmp_u32 format) {
  int rc;

  if (picker == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_hour(hour);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_minute(minute);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_time_picker_validate_format(format);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(picker, 0, sizeof(*picker));
  picker->style = *style;
  picker->format = format;
  picker->active_field = M3_TIME_PICKER_FIELD_HOUR;
  picker->time.hour = hour;
  picker->time.minute = minute;
  picker->pressed = CMP_FALSE;
  picker->widget.ctx = picker;
  picker->widget.vtable = &g_m3_time_picker_widget_vtable;
  picker->widget.handle.id = 0u;
  picker->widget.handle.generation = 0u;
  picker->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;

  return m3_time_picker_update_metrics(picker);
}

int CMP_CALL m3_time_picker_set_style(M3TimePicker *picker,
                                     const M3TimePickerStyle *style) {
  M3TimePickerStyle prev;
  int rc;

  if (picker == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  prev = picker->style;
  picker->style = *style;

  rc = m3_time_picker_update_metrics(picker);
  if (rc != CMP_OK) {
    picker->style = prev;
    m3_time_picker_update_metrics(picker);
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_time_picker_set_format(M3TimePicker *picker, cmp_u32 format) {
  cmp_u32 prev;
  int rc;

  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_format(format);
  if (rc != CMP_OK) {
    return rc;
  }

  prev = picker->format;
  picker->format = format;

  rc = m3_time_picker_update_metrics(picker);
  if (rc != CMP_OK) {
    picker->format = prev;
    m3_time_picker_update_metrics(picker);
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_time_picker_get_format(const M3TimePicker *picker,
                                      cmp_u32 *out_format) {
  if (picker == NULL || out_format == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_format = picker->format;
  return CMP_OK;
}

int CMP_CALL m3_time_picker_set_active_field(M3TimePicker *picker,
                                            cmp_u32 field) {
  int rc;

  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_field(field);
  if (rc != CMP_OK) {
    return rc;
  }

  picker->active_field = field;
  return CMP_OK;
}

int CMP_CALL m3_time_picker_get_active_field(const M3TimePicker *picker,
                                            cmp_u32 *out_field) {
  if (picker == NULL || out_field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_field = picker->active_field;
  return CMP_OK;
}

int CMP_CALL m3_time_picker_set_time(M3TimePicker *picker, const CMPTime *time) {
  return m3_time_picker_apply_time(picker, time);
}

int CMP_CALL m3_time_picker_get_time(const M3TimePicker *picker,
                                    CMPTime *out_time) {
  if (picker == NULL || out_time == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_time = picker->time;
  return CMP_OK;
}

int CMP_CALL m3_time_picker_set_period(M3TimePicker *picker, cmp_u32 period) {
  CMPTime next;
  int rc;

  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_time_picker_validate_format(picker->format);
  if (rc != CMP_OK) {
    return rc;
  }
  if (picker->format != M3_TIME_PICKER_FORMAT_12H) {
    return CMP_ERR_STATE;
  }

  rc = m3_time_picker_validate_period(period);
  if (rc != CMP_OK) {
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

int CMP_CALL m3_time_picker_get_period(const M3TimePicker *picker,
                                      cmp_u32 *out_period) {
  if (picker == NULL || out_period == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (picker->time.hour >= 12u) {
    *out_period = M3_TIME_PICKER_PERIOD_PM;
  } else {
    *out_period = M3_TIME_PICKER_PERIOD_AM;
  }
  return CMP_OK;
}

int CMP_CALL m3_time_picker_set_on_change(M3TimePicker *picker,
                                         CMPTimePickerOnChange on_change,
                                         void *ctx) {
  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  picker->on_change = on_change;
  picker->on_change_ctx = ctx;
  return CMP_OK;
}

int CMP_CALL m3_time_picker_update(M3TimePicker *picker) {
  return m3_time_picker_update_metrics(picker);
}

#ifdef CMP_TESTING
int CMP_CALL m3_time_picker_test_set_force_dial_size_zero(CMPBool enable) {
  g_m3_time_picker_test_force_dial_size_zero = enable;
  return CMP_OK;
}

int CMP_CALL m3_time_picker_test_set_force_angle_error(CMPBool enable) {
  g_m3_time_picker_test_force_angle_error = enable;
  return CMP_OK;
}

int CMP_CALL m3_time_picker_test_set_force_angle_index_error(CMPBool enable) {
  g_m3_time_picker_test_force_angle_index_error = enable;
  return CMP_OK;
}

int CMP_CALL m3_time_picker_test_set_force_hour_invalid(CMPBool enable) {
  g_m3_time_picker_test_force_hour_invalid = enable;
  return CMP_OK;
}

int CMP_CALL m3_time_picker_test_set_force_hour_to_index_error(CMPBool enable) {
  g_m3_time_picker_test_force_hour_to_index_error = enable;
  return CMP_OK;
}

int CMP_CALL m3_time_picker_test_set_force_rect_error(CMPBool enable) {
  g_m3_time_picker_test_force_rect_error = enable;
  return CMP_OK;
}

int CMP_CALL m3_time_picker_test_set_force_resolve_colors_error(CMPBool enable) {
  g_m3_time_picker_test_force_resolve_colors_error = enable;
  return CMP_OK;
}

int CMP_CALL
m3_time_picker_test_set_force_compute_hand_minute_error(CMPBool enable) {
  g_m3_time_picker_test_skip_time_minute = enable;
  g_m3_time_picker_test_force_minute_error = enable;
  return CMP_OK;
}

int CMP_CALL m3_time_picker_test_validate_color(const CMPColor *color) {
  return m3_time_picker_validate_color(color);
}

int CMP_CALL m3_time_picker_test_validate_edges(const CMPLayoutEdges *edges) {
  return m3_time_picker_validate_edges(edges);
}

int CMP_CALL m3_time_picker_test_validate_style(const M3TimePickerStyle *style) {
  return m3_time_picker_validate_style(style);
}

int CMP_CALL m3_time_picker_test_validate_measure_spec(CMPMeasureSpec spec) {
  return m3_time_picker_validate_measure_spec(spec);
}

int CMP_CALL m3_time_picker_test_validate_rect(const CMPRect *rect) {
  return m3_time_picker_validate_rect(rect);
}

int CMP_CALL m3_time_picker_test_validate_time(const CMPTime *time) {
  return m3_time_picker_validate_time(time);
}

int CMP_CALL m3_time_picker_test_angle_from_point(CMPScalar cx, CMPScalar cy,
                                                 CMPScalar x, CMPScalar y,
                                                 CMPScalar *out_angle) {
  return m3_time_picker_angle_from_point(cx, cy, x, y, out_angle);
}

int CMP_CALL m3_time_picker_test_angle_to_index(CMPScalar angle, cmp_u32 count,
                                               cmp_u32 *out_index) {
  return m3_time_picker_angle_to_index(angle, count, out_index);
}

int CMP_CALL m3_time_picker_test_hour_from_index(cmp_u32 index, cmp_u32 format,
                                                CMPBool inner_ring,
                                                cmp_u32 current_hour,
                                                cmp_u32 *out_hour) {
  return m3_time_picker_hour_from_index(index, format, inner_ring, current_hour,
                                        out_hour);
}

int CMP_CALL m3_time_picker_test_hour_to_index(cmp_u32 hour, cmp_u32 format,
                                              cmp_u32 *out_index,
                                              CMPBool *out_inner) {
  return m3_time_picker_hour_to_index(hour, format, out_index, out_inner);
}

int CMP_CALL m3_time_picker_test_compute_metrics(
    const M3TimePicker *picker, M3TimePickerMetrics *out_metrics) {
  return m3_time_picker_compute_metrics(picker, out_metrics);
}

int CMP_CALL m3_time_picker_test_pick_time(const M3TimePicker *picker,
                                          CMPScalar x, CMPScalar y, cmp_u32 field,
                                          CMPTime *out_time, CMPBool *out_valid) {
  return m3_time_picker_pick_time(picker, x, y, field, out_time, out_valid);
}

int CMP_CALL m3_time_picker_test_resolve_colors(const M3TimePicker *picker,
                                               CMPColor *out_background,
                                               CMPColor *out_ring,
                                               CMPColor *out_hand,
                                               CMPColor *out_selection) {
  return m3_time_picker_resolve_colors(picker, out_background, out_ring,
                                       out_hand, out_selection);
}

int CMP_CALL m3_time_picker_test_compute_hand(const M3TimePicker *picker,
                                             const M3TimePickerMetrics *metrics,
                                             CMPScalar *out_angle,
                                             CMPScalar *out_radius) {
  return m3_time_picker_compute_hand(picker, metrics, out_angle, out_radius);
}

int CMP_CALL m3_time_picker_test_draw_circle(CMPGfx *gfx, CMPScalar cx,
                                            CMPScalar cy, CMPScalar radius,
                                            CMPColor color) {
  return m3_time_picker_draw_circle(gfx, cx, cy, radius, color);
}

int CMP_CALL m3_time_picker_test_draw_ring(CMPGfx *gfx, CMPScalar cx, CMPScalar cy,
                                          CMPScalar radius, CMPScalar thickness,
                                          CMPColor ring_color,
                                          CMPColor fill_color) {
  return m3_time_picker_draw_ring(gfx, cx, cy, radius, thickness, ring_color,
                                  fill_color);
}

int CMP_CALL m3_time_picker_test_update_metrics(M3TimePicker *picker) {
  return m3_time_picker_update_metrics(picker);
}
#endif
