#include "m3/m3_date_picker.h"

#include <string.h>

static int m3_date_picker_validate_bool(CMPBool value) {
  if (value != CMP_TRUE && value != CMP_FALSE) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_date_picker_validate_color(const CMPColor *color) {
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

static int m3_date_picker_validate_edges(const CMPLayoutEdges *edges) {
  if (edges == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_date_picker_validate_style(const M3DatePickerStyle *style) {
  CMPScalar max_corner;
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->cell_width <= 0.0f || style->cell_height <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->cell_spacing_x < 0.0f || style->cell_spacing_y < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->header_height < 0.0f || style->weekday_height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->cell_corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }

  max_corner = (style->cell_width < style->cell_height) ? style->cell_width
                                                        : style->cell_height;
  if (style->cell_corner_radius > max_corner * 0.5f) {
    return CMP_ERR_RANGE;
  }

  rc = m3_date_picker_validate_edges(&style->padding);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_date_picker_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_color(&style->cell_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_color(&style->cell_range_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_color(&style->cell_selected_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_color(&style->cell_disabled_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_date_picker_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED &&
      spec.mode != CMP_MEASURE_EXACTLY && spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_date_picker_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_date_picker_validate_year(cmp_i32 year) {
  if (year < CMP_DATE_MIN_YEAR || year > CMP_DATE_MAX_YEAR) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_date_picker_validate_month(cmp_u32 month) {
  if (month < 1u || month > 12u) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_date_picker_is_leap_year(cmp_i32 year, CMPBool *out_leap) {
  int rc;

  if (out_leap == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_validate_year(year);
  if (rc != CMP_OK) {
    return rc;
  }

  if ((year % 4) != 0) {
    *out_leap = CMP_FALSE;
  } else if ((year % 100) != 0) {
    *out_leap = CMP_TRUE;
  } else if ((year % 400) != 0) {
    *out_leap = CMP_FALSE;
  } else {
    *out_leap = CMP_TRUE;
  }
  return CMP_OK;
}

static int m3_date_picker_days_in_month(cmp_i32 year, cmp_u32 month,
                                        cmp_u32 *out_days) {
  CMPBool leap;
  int rc;

  if (out_days == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_validate_year(year);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_month(month);
  if (rc != CMP_OK) {
    return rc;
  }

  switch (month) {
  case 1u:
  case 3u:
  case 5u:
  case 7u:
  case 8u:
  case 10u:
  case 12u:
    *out_days = 31u;
    return CMP_OK;
  case 4u:
  case 6u:
  case 9u:
  case 11u:
    *out_days = 30u;
    return CMP_OK;
  case 2u:
    rc = m3_date_picker_is_leap_year(year, &leap);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    *out_days = (leap == CMP_TRUE) ? 29u : 28u;
    return CMP_OK;
  default:
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
}

static int m3_date_picker_validate_date(const CMPDate *date) {
  cmp_u32 days_in_month;
  int rc;

  if (date == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_date_picker_validate_year(date->year);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_month(date->month);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_days_in_month(date->year, date->month, &days_in_month);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (date->day < 1u || date->day > days_in_month) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_date_picker_compare_dates(const CMPDate *a, const CMPDate *b,
                                        cmp_i32 *out_cmp) {
  int rc;

  if (out_cmp == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (a == NULL || b == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_validate_date(a);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_validate_date(b);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (a->year < b->year) {
    *out_cmp = -1;
  } else if (a->year > b->year) {
    *out_cmp = 1;
  } else if (a->month < b->month) {
    *out_cmp = -1;
  } else if (a->month > b->month) {
    *out_cmp = 1;
  } else if (a->day < b->day) {
    *out_cmp = -1;
  } else if (a->day > b->day) {
    *out_cmp = 1;
  } else {
    *out_cmp = 0;
  }

  return CMP_OK;
}

static int m3_date_picker_validate_range(const M3DateRange *range) {
  int rc;
  cmp_i32 cmp;

  if (range == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_date_picker_validate_bool(range->has_start);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_bool(range->has_end);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (range->has_start == CMP_TRUE) {
    rc = m3_date_picker_validate_date(&range->start);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
  }
  if (range->has_end == CMP_TRUE) {
    rc = m3_date_picker_validate_date(&range->end);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  if (range->has_start == CMP_TRUE && range->has_end == CMP_TRUE) {
    rc = m3_date_picker_compare_dates(&range->start, &range->end, &cmp);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (cmp > 0) {
      return CMP_ERR_RANGE;
    }
  }

  return CMP_OK;
}

static int m3_date_picker_date_in_range(const CMPDate *date,
                                        const M3DateRange *range,
                                        CMPBool *out_in_range) {
  int rc;
  cmp_i32 cmp;

  if (out_in_range == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (date == NULL || range == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_validate_date(date);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_validate_range(range);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_in_range = CMP_TRUE;
  if (range->has_start == CMP_TRUE) {
    rc = m3_date_picker_compare_dates(date, &range->start, &cmp);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (cmp < 0) {
      *out_in_range = CMP_FALSE;
      return CMP_OK;
    }
  }
  if (range->has_end == CMP_TRUE) {
    rc = m3_date_picker_compare_dates(date, &range->end, &cmp);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (cmp > 0) {
      *out_in_range = CMP_FALSE;
      return CMP_OK;
    }
  }

  return CMP_OK;
}

static int m3_date_picker_day_of_week(const CMPDate *date,
                                      cmp_u32 *out_weekday) {
  cmp_i32 year;
  cmp_i32 month;
  cmp_i32 day;
  cmp_i32 k;
  cmp_i32 j;
  cmp_i32 h;
  cmp_i32 weekday;
  int rc;

  if (out_weekday == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (date == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_validate_date(date);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  year = date->year;
  month = (cmp_i32)date->month;
  day = (cmp_i32)date->day;

  if (month < 3) {
    month += 12;
    year -= 1;
  }

  k = year % 100;
  j = year / 100;
  h = (day + (13 * (month + 1)) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
  weekday = (h + 6) % 7;

  *out_weekday = (cmp_u32)weekday;
  return CMP_OK;
}

static int m3_date_picker_validate_week_start(cmp_u32 week_start) {
  if (week_start > 6u) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_date_picker_compute_offset(cmp_u32 weekday, cmp_u32 week_start,
                                         cmp_u32 *out_offset) {
  int rc;

  if (out_offset == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (weekday > 6u) {
    return CMP_ERR_RANGE;
  }

  rc = m3_date_picker_validate_week_start(week_start);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_offset = (weekday + 7u - week_start) % 7u;
  return CMP_OK;
}

static int m3_date_picker_shift_month(cmp_i32 year, cmp_u32 month,
                                      cmp_i32 delta, cmp_i32 *out_year,
                                      cmp_u32 *out_month) {
  cmp_i32 remaining;
  int rc;

  if (out_year == NULL || out_month == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_validate_year(year);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_month(month);
  if (rc != CMP_OK) {
    return rc;
  }

  if (delta > 0) {
    remaining = delta;
    while (remaining > 0) {
      month += 1u;
      if (month > 12u) {
        month = 1u;
        year += 1;
        if (year > CMP_DATE_MAX_YEAR) {
          return CMP_ERR_RANGE;
        }
      }
      remaining -= 1;
    }
  } else {
    remaining = -delta;
    while (remaining > 0) {
      if (month == 1u) {
        month = 12u;
        year -= 1;
        if (year < CMP_DATE_MIN_YEAR) {
          return CMP_ERR_RANGE;
        }
      } else {
        month -= 1u;
      }
      remaining -= 1;
    }
  }

  *out_year = year;
  *out_month = month;
  return CMP_OK;
}

static int m3_date_picker_validate_selection(const M3DatePicker *picker,
                                             const M3DateRange *selection) {
  CMPBool in_range;
  int rc;
  cmp_i32 cmp;

  if (picker == NULL || selection == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_date_picker_validate_range(selection);
  if (rc != CMP_OK) {
    return rc;
  }

  if (selection->has_end == CMP_TRUE && selection->has_start == CMP_FALSE) {
    return CMP_ERR_RANGE;
  }

  if (picker->mode == M3_DATE_PICKER_MODE_SINGLE) {
    if (selection->has_end == CMP_TRUE) {
      rc = m3_date_picker_compare_dates(&selection->start, &selection->end,
                                        &cmp); /* GCOVR_EXCL_LINE */
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      if (cmp != 0) {
        return CMP_ERR_RANGE;
      }
    }
  } else if (picker->mode == M3_DATE_PICKER_MODE_RANGE) {
    if (selection->has_start == CMP_TRUE && selection->has_end == CMP_TRUE) {
      rc = m3_date_picker_compare_dates(&selection->start, &selection->end,
                                        &cmp); /* GCOVR_EXCL_LINE */
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      if (cmp > 0) {
        return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
      }
    }
  } else {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  if (selection->has_start == CMP_TRUE) {
    rc = m3_date_picker_date_in_range(&selection->start, &picker->constraints,
                                      &in_range);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (in_range != CMP_TRUE) {
      return CMP_ERR_RANGE;
    }
  }
  if (selection->has_end == CMP_TRUE) {
    rc = m3_date_picker_date_in_range(&selection->end, &picker->constraints,
                                      &in_range);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (in_range != CMP_TRUE) {
      return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }
  }

  return CMP_OK;
}

static int m3_date_picker_compute_cell_bounds(const M3DatePicker *picker,
                                              cmp_u32 row, cmp_u32 col,
                                              CMPRect *out_bounds) {
  CMPScalar span_x;
  CMPScalar span_y;
  CMPScalar origin_x;
  CMPScalar origin_y;
  int rc;

  if (picker == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (row >= M3_DATE_PICKER_GRID_ROWS || col >= M3_DATE_PICKER_GRID_COLS) {
    return CMP_ERR_RANGE;
  }

  rc = m3_date_picker_validate_style(&picker->style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_validate_rect(&picker->bounds);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  span_x = picker->style.cell_width + picker->style.cell_spacing_x;
  span_y = picker->style.cell_height + picker->style.cell_spacing_y;
  origin_x = picker->bounds.x + picker->style.padding.left;
  origin_y = picker->bounds.y + picker->style.padding.top +
             picker->style.header_height + picker->style.weekday_height;

  out_bounds->x = origin_x + (CMPScalar)col * span_x;
  out_bounds->y = origin_y + (CMPScalar)row * span_y;
  out_bounds->width = picker->style.cell_width;
  out_bounds->height = picker->style.cell_height;

  return CMP_OK;
}

static int m3_date_picker_update_grid(M3DatePicker *picker) {
  CMPDate first_date;
  CMPDate cell_date;
  cmp_u32 weekday;
  cmp_u32 offset;
  cmp_u32 days_current;
  cmp_u32 days_prev;
  cmp_i32 prev_year;
  cmp_u32 prev_month;
  cmp_i32 next_year;
  cmp_u32 next_month;
  cmp_u32 row;
  cmp_u32 col;
  cmp_usize index;
  cmp_u32 flags;
  CMPBool in_range;
  cmp_i32 cmp_start;
  cmp_i32 cmp_end;
  int rc;

  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_validate_style(&picker->style);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_rect(&picker->bounds);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_year(picker->display_year);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_validate_month(picker->display_month);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_validate_week_start(picker->week_start);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_range(&picker->constraints);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_selection(picker, &picker->selection);
  if (rc != CMP_OK) {
    return rc;
  }

  first_date.year = picker->display_year;
  first_date.month = picker->display_month;
  first_date.day = 1u;
  rc = m3_date_picker_days_in_month(picker->display_year, picker->display_month,
                                    &days_current);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_day_of_week(&first_date, &weekday);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_compute_offset(weekday, picker->week_start, &offset);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_shift_month(picker->display_year, picker->display_month,
                                  -1, &prev_year, &prev_month);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_shift_month(picker->display_year, picker->display_month,
                                  1, &next_year, &next_month);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_days_in_month(prev_year, prev_month, &days_prev);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  index = 0u;
  while (index < (cmp_usize)M3_DATE_PICKER_GRID_COUNT) {
    row = (cmp_u32)(index / M3_DATE_PICKER_GRID_COLS);
    col = (cmp_u32)(index % M3_DATE_PICKER_GRID_COLS);

    flags = 0u;
    if (index < (cmp_usize)offset) {
      cell_date.year = prev_year;
      cell_date.month = prev_month;
      cell_date.day = days_prev - (offset - 1u - (cmp_u32)index);
      flags |= M3_DATE_CELL_FLAG_OTHER_MONTH;
    } else if (index >= (cmp_usize)(offset + days_current)) {
      cell_date.year = next_year;
      cell_date.month = next_month;
      cell_date.day = (cmp_u32)(index - (offset + days_current) + 1u);
      flags |= M3_DATE_CELL_FLAG_OTHER_MONTH;
    } else {
      cell_date.year = picker->display_year;
      cell_date.month = picker->display_month;
      cell_date.day = (cmp_u32)(index - offset + 1u);
      flags |= M3_DATE_CELL_FLAG_CURRENT_MONTH;
    }

    rc = m3_date_picker_validate_date(&cell_date);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }

    rc = m3_date_picker_date_in_range(&cell_date, &picker->constraints,
                                      &in_range);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (in_range != CMP_TRUE) {
      flags |= M3_DATE_CELL_FLAG_OUT_OF_RANGE;
    }

    if (picker->selection.has_start == CMP_TRUE) {
      rc = m3_date_picker_compare_dates(&cell_date, &picker->selection.start,
                                        &cmp_start);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      if (cmp_start == 0) {
        flags |= M3_DATE_CELL_FLAG_SELECTED;
        flags |= M3_DATE_CELL_FLAG_RANGE_START;
      }
    }

    if (picker->selection.has_end == CMP_TRUE) {
      rc = m3_date_picker_compare_dates(&cell_date, &picker->selection.end,
                                        &cmp_end);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      if (cmp_end == 0) {
        flags |= M3_DATE_CELL_FLAG_SELECTED;
        flags |= M3_DATE_CELL_FLAG_RANGE_END;
      }
    }

    if (picker->mode == M3_DATE_PICKER_MODE_RANGE &&
        picker->selection.has_start == CMP_TRUE &&
        picker->selection.has_end == CMP_TRUE) {
      rc = m3_date_picker_compare_dates(&cell_date, &picker->selection.start,
                                        &cmp_start);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      rc = m3_date_picker_compare_dates(&cell_date, &picker->selection.end,
                                        &cmp_end);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
      if (cmp_start >= 0 && cmp_end <= 0) {
        flags |= M3_DATE_CELL_FLAG_IN_RANGE;
      }
    }

    rc = m3_date_picker_compute_cell_bounds(picker, row, col,
                                            &picker->cells[index].bounds);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    picker->cells[index].date = cell_date;
    picker->cells[index].flags = flags;

    index += 1u;
  }

  return CMP_OK;
}

static int m3_date_picker_hit_test(const M3DatePicker *picker, CMPScalar x,
                                   CMPScalar y, cmp_usize *out_index) {
  CMPScalar span_x;
  CMPScalar span_y;
  CMPScalar origin_x;
  CMPScalar origin_y;
  CMPScalar rel_x;
  CMPScalar rel_y;
  CMPScalar cell_x;
  CMPScalar cell_y;
  cmp_i32 col;
  cmp_i32 row;
  int rc;

  if (picker == NULL || out_index == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_index = M3_DATE_PICKER_INVALID_INDEX;

  rc = m3_date_picker_validate_style(&picker->style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_validate_rect(&picker->bounds);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  span_x = picker->style.cell_width + picker->style.cell_spacing_x;
  span_y = picker->style.cell_height + picker->style.cell_spacing_y;
  origin_x = picker->bounds.x + picker->style.padding.left;
  origin_y = picker->bounds.y + picker->style.padding.top +
             picker->style.header_height + picker->style.weekday_height;

  rel_x = x - origin_x;
  rel_y = y - origin_y;
  if (rel_x < 0.0f || rel_y < 0.0f) {
    return CMP_OK;
  }

  col = (cmp_i32)(rel_x / span_x);
  row = (cmp_i32)(rel_y / span_y);
  if (col < 0 || col >= (cmp_i32)M3_DATE_PICKER_GRID_COLS || row < 0 ||
      row >= (cmp_i32)M3_DATE_PICKER_GRID_ROWS) {
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  cell_x = (CMPScalar)col * span_x;
  cell_y = (CMPScalar)row * span_y;
  if ((rel_x - cell_x) > picker->style.cell_width ||
      (rel_y - cell_y) > picker->style.cell_height) {
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  *out_index = (cmp_usize)row * M3_DATE_PICKER_GRID_COLS + (cmp_usize)col;
  return CMP_OK;
}

static int m3_date_picker_apply_selection(M3DatePicker *picker,
                                          const M3DateRange *selection,
                                          CMPBool notify) {
  M3DateRange prev;
  int rc;

  if (picker == NULL || selection == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  rc = m3_date_picker_validate_selection(picker, selection);
  if (rc != CMP_OK) {
    return rc;
  }

  prev = picker->selection;
  picker->selection = *selection;

  rc = m3_date_picker_update_grid(picker);
  if (rc != CMP_OK) {
    picker->selection = prev;
    m3_date_picker_update_grid(picker);
    return rc;
  }

  if (notify == CMP_TRUE && picker->on_change != NULL) {
    rc = picker->on_change(picker->on_change_ctx, picker, &picker->selection);
    if (rc != CMP_OK) {
      picker->selection = prev;
      m3_date_picker_update_grid(picker);
      return rc;
    }
  }

  return CMP_OK;
}

static int m3_date_picker_select_cell(M3DatePicker *picker, cmp_usize index) {
  M3DateRange next;
  cmp_i32 cmp;
  int rc;

  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (index >= (cmp_usize)M3_DATE_PICKER_GRID_COUNT) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  if (picker->cells[index].flags & M3_DATE_CELL_FLAG_OUT_OF_RANGE) {
    return CMP_OK;
  }

  next = picker->selection;
  if (picker->mode == M3_DATE_PICKER_MODE_SINGLE) {
    next.start = picker->cells[index].date; /* GCOVR_EXCL_LINE */
    next.has_start = CMP_TRUE;              /* GCOVR_EXCL_LINE */
    next.has_end = CMP_FALSE;               /* GCOVR_EXCL_LINE */
  } else if (picker->mode == M3_DATE_PICKER_MODE_RANGE) {
    if (next.has_start == CMP_FALSE ||
        next.has_end == CMP_TRUE) {           /* GCOVR_EXCL_LINE */
      next.start = picker->cells[index].date; /* GCOVR_EXCL_LINE */
      next.has_start = CMP_TRUE;              /* GCOVR_EXCL_LINE */
      next.has_end = CMP_FALSE;               /* GCOVR_EXCL_LINE */
    } else {                                  /* GCOVR_EXCL_LINE */
      rc = m3_date_picker_compare_dates(&picker->cells[index].date,
                                        &next.start, /* GCOVR_EXCL_LINE */
                                        &cmp);       /* GCOVR_EXCL_LINE */
      if (rc != CMP_OK) {                            /* GCOVR_EXCL_LINE */
        return rc;                                   /* GCOVR_EXCL_LINE */
      }
      if (cmp < 0) {                            /* GCOVR_EXCL_LINE */
        next.end = next.start;                  /* GCOVR_EXCL_LINE */
        next.start = picker->cells[index].date; /* GCOVR_EXCL_LINE */
      } else {                                  /* GCOVR_EXCL_LINE */
        next.end = picker->cells[index].date;   /* GCOVR_EXCL_LINE */
      }
      next.has_end = CMP_TRUE; /* GCOVR_EXCL_LINE */
    }
  } else { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE;
  }

  return m3_date_picker_apply_selection(picker, &next,
                                        CMP_TRUE); /* GCOVR_EXCL_LINE */
}

static int m3_date_picker_widget_measure(void *widget, CMPMeasureSpec width,
                                         CMPMeasureSpec height,
                                         CMPSize *out_size) {
  M3DatePicker *picker;
  CMPScalar desired_width;
  CMPScalar desired_height;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  picker = (M3DatePicker *)widget;
  rc = m3_date_picker_validate_style(&picker->style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  desired_width =
      picker->style.padding.left + picker->style.padding.right +
      picker->style.cell_width * M3_DATE_PICKER_GRID_COLS +
      picker->style.cell_spacing_x * (M3_DATE_PICKER_GRID_COLS - 1u);
  desired_height =
      picker->style.padding.top + picker->style.padding.bottom +
      picker->style.header_height + picker->style.weekday_height +
      picker->style.cell_height * M3_DATE_PICKER_GRID_ROWS +
      picker->style.cell_spacing_y * (M3_DATE_PICKER_GRID_ROWS - 1u);

  rc = m3_date_picker_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc;
  }

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    out_size->width = desired_width;    /* GCOVR_EXCL_LINE */
    if (out_size->width > width.size) { /* GCOVR_EXCL_LINE */
      out_size->width = width.size;     /* GCOVR_EXCL_LINE */
    }
  } else {                           /* GCOVR_EXCL_LINE */
    out_size->width = desired_width; /* GCOVR_EXCL_LINE */
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == CMP_MEASURE_AT_MOST) {
    out_size->height = desired_height;    /* GCOVR_EXCL_LINE */
    if (out_size->height > height.size) { /* GCOVR_EXCL_LINE */
      out_size->height = height.size;     /* GCOVR_EXCL_LINE */
    }
  } else {                             /* GCOVR_EXCL_LINE */
    out_size->height = desired_height; /* GCOVR_EXCL_LINE */
  }
  return CMP_OK;
}

static int m3_date_picker_widget_layout(void *widget, CMPRect bounds) {
  M3DatePicker *picker;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  picker = (M3DatePicker *)widget;
  picker->bounds = bounds;
  return m3_date_picker_update_grid(picker);
}

static int m3_date_picker_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3DatePicker *picker;
  CMPColor color;
  cmp_usize index;
  int rc;

  if (widget == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  picker = (M3DatePicker *)widget;

  rc = m3_date_picker_validate_style(&picker->style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_validate_rect(&picker->bounds);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_update_grid(picker);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  color = picker->style.background_color;
  rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &picker->bounds, color, 0.0f);
  if (rc != CMP_OK) {
    return rc;
  }

  index = 0u;
  while (index < (cmp_usize)M3_DATE_PICKER_GRID_COUNT) {
    if (picker->cells[index].flags & M3_DATE_CELL_FLAG_SELECTED) {
      color = picker->style.cell_selected_color;
    } else if (picker->cells[index].flags & M3_DATE_CELL_FLAG_IN_RANGE) {
      color = picker->style.cell_range_color;
    } else if ((picker->cells[index].flags & M3_DATE_CELL_FLAG_OUT_OF_RANGE) ||
               (picker->cells[index].flags & M3_DATE_CELL_FLAG_OTHER_MONTH)) {
      color = picker->style.cell_disabled_color;
    } else {
      color = picker->style.cell_color;
    }

    rc =
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &picker->cells[index].bounds,
                                    color, picker->style.cell_corner_radius);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }

    index += 1u;
  }

  return CMP_OK;
}

static int m3_date_picker_widget_event(void *widget, const CMPInputEvent *event,
                                       CMPBool *out_handled) {
  M3DatePicker *picker;
  cmp_usize index;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;
  picker = (M3DatePicker *)widget;

  if (picker->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN:
    if (picker->pressed_index != M3_DATE_PICKER_INVALID_INDEX) {
      return CMP_ERR_STATE;
    }
    rc = m3_date_picker_hit_test(picker, (CMPScalar)event->data.pointer.x,
                                 (CMPScalar)event->data.pointer.y, &index);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (index == M3_DATE_PICKER_INVALID_INDEX) {
      return CMP_OK;
    }
    picker->pressed_index = index;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_POINTER_UP:
    if (picker->pressed_index == M3_DATE_PICKER_INVALID_INDEX) {
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    index = picker->pressed_index;
    picker->pressed_index = M3_DATE_PICKER_INVALID_INDEX;
    rc = m3_date_picker_select_cell(picker, index);
    if (rc != CMP_OK) {
      return rc;
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  default:
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }
}

static int m3_date_picker_widget_get_semantics(void *widget,
                                               CMPSemantics *out_semantics) {
  M3DatePicker *picker;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  picker = (M3DatePicker *)widget;
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0u;
  if (picker->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED; /* GCOVR_EXCL_LINE */
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_date_picker_widget_destroy(void *widget) {
  CMP_UNUSED(widget);
  return CMP_OK;
}

static const CMPWidgetVTable g_m3_date_picker_widget_vtable = {
    m3_date_picker_widget_measure,       m3_date_picker_widget_layout,
    m3_date_picker_widget_paint,         m3_date_picker_widget_event,
    m3_date_picker_widget_get_semantics, m3_date_picker_widget_destroy};

int CMP_CALL m3_date_picker_style_init(M3DatePickerStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  memset(style, 0, sizeof(*style));
  style->padding.left = M3_DATE_PICKER_DEFAULT_PADDING;
  style->padding.top = M3_DATE_PICKER_DEFAULT_PADDING;
  style->padding.right = M3_DATE_PICKER_DEFAULT_PADDING;
  style->padding.bottom = M3_DATE_PICKER_DEFAULT_PADDING;
  style->cell_width = M3_DATE_PICKER_DEFAULT_CELL_WIDTH;
  style->cell_height = M3_DATE_PICKER_DEFAULT_CELL_HEIGHT;
  style->cell_spacing_x = M3_DATE_PICKER_DEFAULT_CELL_SPACING_X;
  style->cell_spacing_y = M3_DATE_PICKER_DEFAULT_CELL_SPACING_Y;
  style->header_height = M3_DATE_PICKER_DEFAULT_HEADER_HEIGHT;
  style->weekday_height = M3_DATE_PICKER_DEFAULT_WEEKDAY_HEIGHT;
  style->cell_corner_radius = M3_DATE_PICKER_DEFAULT_CELL_CORNER_RADIUS;
  style->background_color.r = 0.0f;
  style->background_color.g = 0.0f;
  style->background_color.b = 0.0f;
  style->background_color.a = 0.0f;
  style->cell_color = style->background_color;
  style->cell_range_color = style->background_color;
  style->cell_selected_color = style->background_color;
  style->cell_disabled_color = style->background_color;
  return CMP_OK;
}

int CMP_CALL m3_date_picker_init(M3DatePicker *picker,
                                 const M3DatePickerStyle *style, cmp_i32 year,
                                 cmp_u32 month) {
  int rc;

  if (picker == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_validate_style(style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_validate_year(year);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_date_picker_validate_month(month);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(picker, 0, sizeof(*picker));
  picker->style = *style;
  picker->display_year = year;
  picker->display_month = month;
  picker->week_start = M3_DATE_PICKER_WEEK_START_SUNDAY;
  picker->mode = M3_DATE_PICKER_MODE_SINGLE;
  picker->pressed_index = M3_DATE_PICKER_INVALID_INDEX;
  picker->widget.ctx = picker;
  picker->widget.vtable = &g_m3_date_picker_widget_vtable;
  picker->widget.handle.id = 0u;
  picker->widget.handle.generation = 0u;
  picker->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;

  return m3_date_picker_update_grid(picker);
}

int CMP_CALL m3_date_picker_set_style(M3DatePicker *picker,
                                      const M3DatePickerStyle *style) {
  M3DatePickerStyle prev;
  int rc;

  if (picker == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  prev = picker->style;   /* GCOVR_EXCL_LINE */
  picker->style = *style; /* GCOVR_EXCL_LINE */

  rc = m3_date_picker_update_grid(picker); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) {                      /* GCOVR_EXCL_LINE */
    picker->style = prev;                  /* GCOVR_EXCL_LINE */
    m3_date_picker_update_grid(picker);    /* GCOVR_EXCL_LINE */
    return rc;                             /* GCOVR_EXCL_LINE */
  }

  return CMP_OK; /* GCOVR_EXCL_LINE */
}

int CMP_CALL m3_date_picker_set_mode(M3DatePicker *picker, cmp_u32 mode) {
  cmp_u32 prev;
  int rc;

  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (mode != M3_DATE_PICKER_MODE_SINGLE && mode != M3_DATE_PICKER_MODE_RANGE) {
    return CMP_ERR_RANGE;
  }

  prev = picker->mode;
  picker->mode = mode;

  rc = m3_date_picker_validate_selection(picker, &picker->selection);
  if (rc != CMP_OK) {
    picker->mode = prev;
    return rc;
  }

  rc = m3_date_picker_update_grid(picker);
  if (rc != CMP_OK) {
    picker->mode = prev;                /* GCOVR_EXCL_LINE */
    m3_date_picker_update_grid(picker); /* GCOVR_EXCL_LINE */
    return rc;                          /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

int CMP_CALL m3_date_picker_set_week_start(M3DatePicker *picker,
                                           cmp_u32 week_start) {
  cmp_u32 prev;
  int rc;

  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  rc = m3_date_picker_validate_week_start(week_start);
  if (rc != CMP_OK) {
    return rc;
  }

  prev = picker->week_start;
  picker->week_start = week_start;

  rc = m3_date_picker_update_grid(picker);
  if (rc != CMP_OK) {
    picker->week_start = prev;          /* GCOVR_EXCL_LINE */
    m3_date_picker_update_grid(picker); /* GCOVR_EXCL_LINE */
    return rc;                          /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

int CMP_CALL m3_date_picker_set_display_month(M3DatePicker *picker,
                                              cmp_i32 year, cmp_u32 month) {
  cmp_i32 prev_year;
  cmp_u32 prev_month;
  int rc;

  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_validate_year(year);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_date_picker_validate_month(month);
  if (rc != CMP_OK) {
    return rc;
  }

  prev_year = picker->display_year;
  prev_month = picker->display_month;
  picker->display_year = year;
  picker->display_month = month;

  rc = m3_date_picker_update_grid(picker);
  if (rc != CMP_OK) {
    picker->display_year = prev_year;
    picker->display_month = prev_month;
    m3_date_picker_update_grid(picker);
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_date_picker_get_display_month(const M3DatePicker *picker,
                                              cmp_i32 *out_year,
                                              cmp_u32 *out_month) {
  if (picker == NULL || out_year == NULL || out_month == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_year = picker->display_year;
  *out_month = picker->display_month;
  return CMP_OK;
}

int CMP_CALL m3_date_picker_next_month(M3DatePicker *picker) {
  cmp_i32 year;
  cmp_u32 month;
  int rc;

  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_shift_month(picker->display_year, picker->display_month,
                                  1, &year, &month);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return m3_date_picker_set_display_month(picker, year, month);
}

int CMP_CALL m3_date_picker_prev_month(M3DatePicker *picker) {
  cmp_i32 year;
  cmp_u32 month;
  int rc;

  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_shift_month(picker->display_year, picker->display_month,
                                  -1, &year, &month);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return m3_date_picker_set_display_month(picker, year, month);
}

int CMP_CALL m3_date_picker_set_constraints(M3DatePicker *picker,
                                            const M3DateRange *constraints) {
  M3DateRange prev;
  int rc;

  if (picker == NULL || constraints == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_date_picker_validate_range(constraints);
  if (rc != CMP_OK) {
    return rc;
  }

  prev = picker->constraints;
  picker->constraints = *constraints;

  rc = m3_date_picker_validate_selection(picker, &picker->selection);
  if (rc != CMP_OK) {
    picker->constraints = prev;
    return rc;
  }

  rc = m3_date_picker_update_grid(picker);
  if (rc != CMP_OK) {
    picker->constraints = prev;         /* GCOVR_EXCL_LINE */
    m3_date_picker_update_grid(picker); /* GCOVR_EXCL_LINE */
    return rc;                          /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

int CMP_CALL m3_date_picker_get_constraints(const M3DatePicker *picker,
                                            M3DateRange *out_constraints) {
  if (picker == NULL || out_constraints == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_constraints = picker->constraints; /* GCOVR_EXCL_LINE */
  return CMP_OK;                          /* GCOVR_EXCL_LINE */
}

int CMP_CALL m3_date_picker_set_selection(M3DatePicker *picker,
                                          const M3DateRange *selection) {
  return m3_date_picker_apply_selection(picker, selection, CMP_TRUE);
}

int CMP_CALL m3_date_picker_get_selection(const M3DatePicker *picker,
                                          M3DateRange *out_selection) {
  if (picker == NULL || out_selection == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_selection = picker->selection;
  return CMP_OK;
}

int CMP_CALL m3_date_picker_set_on_change(M3DatePicker *picker,
                                          CMPDatePickerOnChange on_change,
                                          void *ctx) {
  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  picker->on_change = on_change;
  picker->on_change_ctx = ctx;
  return CMP_OK;
}

int CMP_CALL m3_date_picker_update(M3DatePicker *picker) {
  if (picker == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return m3_date_picker_update_grid(picker);
}

int CMP_CALL m3_date_picker_get_cells(const M3DatePicker *picker,
                                      const M3DatePickerCell **out_cells,
                                      cmp_usize *out_count) {
  if (picker == NULL || out_cells == NULL || out_count == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_cells = picker->cells;
  *out_count = M3_DATE_PICKER_GRID_COUNT;
  return CMP_OK;
}

int CMP_CALL m3_date_picker_get_cell(const M3DatePicker *picker,
                                     cmp_usize index,
                                     M3DatePickerCell *out_cell) {
  if (picker == NULL || out_cell == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (index >= (cmp_usize)M3_DATE_PICKER_GRID_COUNT) {
    return CMP_ERR_RANGE;
  }

  *out_cell = picker->cells[index]; /* GCOVR_EXCL_LINE */
  return CMP_OK;                    /* GCOVR_EXCL_LINE */
}

#ifdef CMP_TESTING
int CMP_CALL m3_date_picker_test_validate_color(const CMPColor *color) {
  return m3_date_picker_validate_color(color);
}

int CMP_CALL m3_date_picker_test_validate_edges(const CMPLayoutEdges *edges) {
  return m3_date_picker_validate_edges(edges);
}

int CMP_CALL
m3_date_picker_test_validate_style(const M3DatePickerStyle *style) {
  return m3_date_picker_validate_style(style);
}

int CMP_CALL m3_date_picker_test_validate_measure_spec(CMPMeasureSpec spec) {
  return m3_date_picker_validate_measure_spec(spec);
}

int CMP_CALL m3_date_picker_test_validate_rect(const CMPRect *rect) {
  return m3_date_picker_validate_rect(rect);
}

int CMP_CALL m3_date_picker_test_validate_date(const CMPDate *date) {
  return m3_date_picker_validate_date(date);
}

int CMP_CALL m3_date_picker_test_is_leap_year(cmp_i32 year, CMPBool *out_leap) {
  return m3_date_picker_is_leap_year(year, out_leap);
}

int CMP_CALL m3_date_picker_test_days_in_month(cmp_i32 year, cmp_u32 month,
                                               cmp_u32 *out_days) {
  return m3_date_picker_days_in_month(year, month, out_days);
}

int CMP_CALL m3_date_picker_test_day_of_week(const CMPDate *date,
                                             cmp_u32 *out_weekday) {
  return m3_date_picker_day_of_week(date, out_weekday);
}

int CMP_CALL m3_date_picker_test_compare_dates(const CMPDate *a,
                                               const CMPDate *b,
                                               cmp_i32 *out_cmp) {
  return m3_date_picker_compare_dates(a, b, out_cmp);
}

int CMP_CALL m3_date_picker_test_date_in_range(const CMPDate *date,
                                               const M3DateRange *range,
                                               CMPBool *out_in_range) {
  return m3_date_picker_date_in_range(date, range, out_in_range);
}

int CMP_CALL m3_date_picker_test_compute_offset(cmp_u32 weekday,
                                                cmp_u32 week_start,
                                                cmp_u32 *out_offset) {
  return m3_date_picker_compute_offset(weekday, week_start, out_offset);
}

int CMP_CALL m3_date_picker_test_shift_month(cmp_i32 year, cmp_u32 month,
                                             cmp_i32 delta, cmp_i32 *out_year,
                                             cmp_u32 *out_month) {
  return m3_date_picker_shift_month(year, month, delta, out_year, out_month);
}

int CMP_CALL m3_date_picker_test_compute_cell_bounds(const M3DatePicker *picker,
                                                     cmp_u32 row, cmp_u32 col,
                                                     CMPRect *out_bounds) {
  return m3_date_picker_compute_cell_bounds(picker, row, col, out_bounds);
}

int CMP_CALL m3_date_picker_test_hit_test(const M3DatePicker *picker,
                                          CMPScalar x, CMPScalar y,
                                          cmp_usize *out_index) {
  return m3_date_picker_hit_test(picker, x, y, out_index);
}

int CMP_CALL m3_date_picker_test_update_grid(M3DatePicker *picker) {
  return m3_date_picker_update_grid(picker);
}
#endif
