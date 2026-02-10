#include "m3/m3_text_field.h"

#include "m3/m3_utf8.h"

#include <string.h>

#define M3_TEXT_FIELD_DEFAULT_CAPACITY 16u

#ifdef M3_TESTING
#define M3_TEXT_FIELD_TEST_FAIL_NONE 0u
#define M3_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE 1u
#define M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT 2u
#define M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT 3u
#define M3_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE 4u
#define M3_TEXT_FIELD_TEST_FAIL_FONT_METRICS 5u
#define M3_TEXT_FIELD_TEST_FAIL_ANIM_START 6u
#define M3_TEXT_FIELD_TEST_FAIL_RESERVE_REALLOC 7u
#define M3_TEXT_FIELD_TEST_FAIL_MEASURE_PREFIX 8u
#define M3_TEXT_FIELD_TEST_FAIL_RESOLVE_COLORS 9u
#define M3_TEXT_FIELD_TEST_FAIL_RESERVE_BYPASS 10u
#define M3_TEXT_FIELD_TEST_FAIL_OFFSET_SKIP_EARLY 11u
#define M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND 12u
#define M3_TEXT_FIELD_TEST_FAIL_TEXT_STYLE_INIT 13u
#define M3_TEXT_FIELD_TEST_FAIL_LABEL_STYLE_INIT 14u
#define M3_TEXT_FIELD_TEST_FAIL_ANIM_INIT 15u
#define M3_TEXT_FIELD_TEST_FAIL_LABEL_FONT_METRICS 16u
#define M3_TEXT_FIELD_TEST_FAIL_OUTLINE_RANGE 17u
#define M3_TEXT_FIELD_TEST_FAIL_CORNER_RANGE 18u
#define M3_TEXT_FIELD_TEST_FAIL_SELECTION_WIDTH_NEGATIVE 19u
#define M3_TEXT_FIELD_TEST_FAIL_CURSOR_WIDTH_NEGATIVE 20u
#define M3_TEXT_FIELD_TEST_FAIL_CURSOR_HEIGHT_NEGATIVE 21u
#define M3_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL 22u

static m3_u32 g_m3_text_field_test_fail_point = M3_TEXT_FIELD_TEST_FAIL_NONE;
static m3_u32 g_m3_text_field_test_color_fail_after = 0u;
static m3_u32 g_m3_text_field_test_overflow_fail_after = 0u;
static m3_u32 g_m3_text_field_test_font_metrics_fail_after = 0u;
static M3Bool g_m3_text_field_test_offset_skip_early = M3_FALSE;

int M3_CALL m3_text_field_test_set_fail_point(m3_u32 fail_point) {
  g_m3_text_field_test_fail_point = fail_point;
  return M3_OK;
}

int M3_CALL m3_text_field_test_set_color_fail_after(m3_u32 call_count) {
  g_m3_text_field_test_color_fail_after = call_count;
  return M3_OK;
}

int M3_CALL m3_text_field_test_set_overflow_fail_after(m3_u32 call_count) {
  g_m3_text_field_test_overflow_fail_after = call_count;
  return M3_OK;
}

int M3_CALL m3_text_field_test_set_font_metrics_fail_after(m3_u32 call_count) {
  g_m3_text_field_test_font_metrics_fail_after = call_count;
  return M3_OK;
}

int M3_CALL m3_text_field_test_set_offset_skip_early(M3Bool enable) {
  g_m3_text_field_test_offset_skip_early = enable;
  return M3_OK;
}

int M3_CALL m3_text_field_test_clear_fail_points(void) {
  g_m3_text_field_test_fail_point = M3_TEXT_FIELD_TEST_FAIL_NONE;
  g_m3_text_field_test_color_fail_after = 0u;
  g_m3_text_field_test_overflow_fail_after = 0u;
  g_m3_text_field_test_font_metrics_fail_after = 0u;
  g_m3_text_field_test_offset_skip_early = M3_FALSE;
  return M3_OK;
}

static int m3_text_field_test_fail_point_match(m3_u32 point) {
  if (g_m3_text_field_test_fail_point != point) {
    return 0;
  }
  g_m3_text_field_test_fail_point = M3_TEXT_FIELD_TEST_FAIL_NONE;
  return 1;
}

static int m3_text_field_test_color_should_fail(void) {
  if (g_m3_text_field_test_color_fail_after == 0u) {
    return 0;
  }
  g_m3_text_field_test_color_fail_after -= 1u;
  return (g_m3_text_field_test_color_fail_after == 0u) ? 1 : 0;
}

static int m3_text_field_test_overflow_should_fail(void) {
  if (g_m3_text_field_test_overflow_fail_after == 0u) {
    return 0;
  }
  g_m3_text_field_test_overflow_fail_after -= 1u;
  return (g_m3_text_field_test_overflow_fail_after == 0u) ? 1 : 0;
}

static int m3_text_field_test_font_metrics_should_fail(void) {
  if (g_m3_text_field_test_font_metrics_fail_after == 0u) {
    return 0;
  }
  g_m3_text_field_test_font_metrics_fail_after -= 1u;
  return (g_m3_text_field_test_font_metrics_fail_after == 0u) ? 1 : 0;
}
#endif

static int m3_text_field_validate_color(const M3Color *color) {
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

static int m3_text_field_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                   M3Scalar b, M3Scalar a) {
  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(r >= 0.0f && r <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(g >= 0.0f && g <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(b >= 0.0f && b <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(a >= 0.0f && a <= 1.0f)) {
    return M3_ERR_RANGE;
  }
#ifdef M3_TESTING
  if (m3_text_field_test_color_should_fail()) {
    return M3_ERR_IO;
  }
#endif
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return M3_OK;
}

static int m3_text_field_validate_text_style(const M3TextStyle *style,
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

  rc = m3_text_field_validate_color(&style->color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_text_field_validate_style(const M3TextFieldStyle *style,
                                        M3Bool require_text_family,
                                        M3Bool require_label_family) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (style->outline_width < 0.0f || style->corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->padding_x < 0.0f || style->padding_y < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->min_height < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->label_float_offset < 0.0f || style->label_anim_duration < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->cursor_width < 0.0f || style->cursor_blink_period < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->handle_radius < 0.0f || style->handle_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_text_field_validate_text_style(&style->text_style,
                                         require_text_family);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_text_style(&style->label_style,
                                         require_label_family);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_field_validate_color(&style->container_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(&style->outline_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(&style->focused_outline_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(&style->disabled_container_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(&style->disabled_outline_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(&style->disabled_text_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(&style->disabled_label_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(&style->placeholder_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(&style->disabled_placeholder_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(&style->cursor_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(&style->selection_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(&style->handle_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_text_field_validate_backend(const M3TextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->destroy_font == NULL ||
      backend->vtable->measure_text == NULL ||
      backend->vtable->draw_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }
  return M3_OK;
}

static int m3_text_field_validate_measure_spec(M3MeasureSpec spec) {
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY &&
      spec.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_text_field_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static m3_usize m3_text_field_usize_max(void) {
  return (m3_usize) ~(m3_usize)0;
}

static int m3_text_field_add_overflow(m3_usize a, m3_usize b,
                                      m3_usize *out_value) {
  m3_usize max_value;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

#ifdef M3_TESTING
  if (m3_text_field_test_overflow_should_fail()) {
    return M3_ERR_OVERFLOW;
  }
#endif

  max_value = m3_text_field_usize_max();
  if (a > max_value - b) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = a + b;
  return M3_OK;
}

static int m3_text_field_reserve(M3TextField *field, m3_usize required) {
  m3_usize new_capacity;
  void *mem;
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (required == 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (field->allocator.realloc == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (required <= field->utf8_capacity) {
#ifdef M3_TESTING
    if (!m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_RESERVE_BYPASS)) {
      return M3_OK;
    }
#else
    return M3_OK;
#endif
  }

  if (field->utf8_capacity == 0u) {
    new_capacity = M3_TEXT_FIELD_DEFAULT_CAPACITY;
  } else if (field->utf8_capacity > m3_text_field_usize_max() / 2u) {
    if (field->utf8_capacity < required) {
      return M3_ERR_OVERFLOW;
    }
    new_capacity = field->utf8_capacity;
  } else {
    new_capacity = field->utf8_capacity * 2u;
  }

  while (new_capacity < required) {
    if (new_capacity > m3_text_field_usize_max() / 2u) {
      new_capacity = required;
      break;
    }
    new_capacity *= 2u;
  }

  rc = field->allocator.realloc(field->allocator.ctx, field->utf8, new_capacity,
                                &mem);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_RESERVE_REALLOC)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  field->utf8 = (char *)mem;
  field->utf8_capacity = new_capacity;
  return M3_OK;
}

static int m3_text_field_validate_utf8(const char *utf8, m3_usize utf8_len) {
  M3Bool valid;
  int rc;

  if (utf8 == NULL) {
    if (utf8_len == 0u) {
      return M3_OK;
    }
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_utf8_validate(utf8, utf8_len, &valid);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }
  if (!valid) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

static int m3_text_field_validate_offset(const char *utf8, m3_usize utf8_len,
                                         m3_usize offset) {
  M3Utf8Iter iter;
  m3_u32 codepoint;
  m3_usize last_offset;
  int rc;

  if (offset > utf8_len) {
    return M3_ERR_RANGE;
  }
#ifdef M3_TESTING
  if (offset == utf8_len || offset == 0u) {
    if (g_m3_text_field_test_offset_skip_early != M3_TRUE) {
      return M3_OK;
    }
  }
#else
  if (offset == utf8_len || offset == 0u) {
    return M3_OK;
  }
#endif

  rc = m3_utf8_iter_init(&iter, utf8, utf8_len);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  last_offset = 0u;
  while (1) {
    rc = m3_utf8_iter_next(&iter, &codepoint);
#ifdef M3_TESTING
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT)) {
      rc = M3_ERR_IO;
    }
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND)) {
      rc = M3_ERR_NOT_FOUND;
    }
#endif
    if (rc == M3_ERR_NOT_FOUND) {
      break;
    }
    if (rc != M3_OK) {
      return rc;
    }
    if (iter.offset == offset) {
      return M3_OK;
    }
    if (iter.offset > offset) {
      break;
    }
    last_offset = iter.offset;
    M3_UNUSED(codepoint);
  }

  if (last_offset == offset) {
    return M3_OK;
  }
  return M3_ERR_RANGE;
}

static int m3_text_field_prev_offset(const char *utf8, m3_usize utf8_len,
                                     m3_usize offset, m3_usize *out_offset) {
  M3Utf8Iter iter;
  m3_u32 codepoint;
  m3_usize last_offset;
  int rc;

  if (out_offset == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (offset == 0u) {
    *out_offset = 0u;
    return M3_OK;
  }

  rc = m3_text_field_validate_offset(utf8, utf8_len, offset);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_utf8_iter_init(&iter, utf8, utf8_len);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  last_offset = 0u;
  while (1) {
    rc = m3_utf8_iter_next(&iter, &codepoint);
#ifdef M3_TESTING
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT)) {
      rc = M3_ERR_IO;
    }
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND)) {
      rc = M3_ERR_NOT_FOUND;
    }
#endif
    if (rc == M3_ERR_NOT_FOUND) {
      break;
    }
    if (rc != M3_OK) {
      return rc;
    }
    if (iter.offset >= offset) {
      break;
    }
    last_offset = iter.offset;
    M3_UNUSED(codepoint);
  }

  *out_offset = last_offset;
  return M3_OK;
}

static int m3_text_field_next_offset(const char *utf8, m3_usize utf8_len,
                                     m3_usize offset, m3_usize *out_offset) {
  M3Utf8Iter iter;
  m3_u32 codepoint;
  int rc;

  if (out_offset == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (offset >= utf8_len) {
    *out_offset = utf8_len;
    return M3_OK;
  }

  rc = m3_text_field_validate_offset(utf8, utf8_len, offset);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_utf8_iter_init(&iter, utf8, utf8_len);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  while (1) {
    rc = m3_utf8_iter_next(&iter, &codepoint);
#ifdef M3_TESTING
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT)) {
      rc = M3_ERR_IO;
    }
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND)) {
      rc = M3_ERR_NOT_FOUND;
    }
#endif
    if (rc == M3_ERR_NOT_FOUND) {
      break;
    }
    if (rc != M3_OK) {
      return rc;
    }
    if (iter.offset > offset) {
      *out_offset = iter.offset;
      return M3_OK;
    }
    M3_UNUSED(codepoint);
  }

  *out_offset = utf8_len;
  return M3_OK;
}

static int m3_text_field_update_text_metrics(M3TextField *field) {
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (field->text_metrics_valid == M3_TRUE) {
    return M3_OK;
  }

  rc = m3_text_measure_utf8(&field->text_backend, field->text_font, field->utf8,
                            field->utf8_len, &field->text_metrics);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  field->text_metrics_valid = M3_TRUE;
  return M3_OK;
}

static int m3_text_field_update_label_metrics(M3TextField *field) {
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (field->label_metrics_valid == M3_TRUE) {
    return M3_OK;
  }

  if (field->label_font.id == 0u && field->label_font.generation == 0u) {
    field->label_metrics.width = 0.0f;
    field->label_metrics.height = 0.0f;
    field->label_metrics.baseline = 0.0f;
    field->label_metrics_valid = M3_TRUE;
    return M3_OK;
  }

  rc = m3_text_measure_utf8(&field->text_backend, field->label_font,
                            field->utf8_label, field->label_len,
                            &field->label_metrics);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  field->label_metrics_valid = M3_TRUE;
  return M3_OK;
}

static int m3_text_field_update_placeholder_metrics(M3TextField *field) {
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (field->placeholder_metrics_valid == M3_TRUE) {
    return M3_OK;
  }

  rc = m3_text_measure_utf8(&field->text_backend, field->text_font,
                            field->utf8_placeholder, field->placeholder_len,
                            &field->placeholder_metrics);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  field->placeholder_metrics_valid = M3_TRUE;
  return M3_OK;
}

static int m3_text_field_update_font_metrics(M3TextField *field) {
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (field->font_metrics_valid == M3_TRUE) {
    return M3_OK;
  }

  rc = m3_text_font_metrics(&field->text_backend, field->text_font,
                            &field->text_font_metrics);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_FONT_METRICS)) {
    rc = M3_ERR_IO;
  }
  if (m3_text_field_test_font_metrics_should_fail()) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  if (field->label_font.id != 0u || field->label_font.generation != 0u) {
    rc = m3_text_font_metrics(&field->text_backend, field->label_font,
                              &field->label_font_metrics);
#ifdef M3_TESTING
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_LABEL_FONT_METRICS)) {
      rc = M3_ERR_IO;
    }
    if (m3_text_field_test_font_metrics_should_fail()) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    field->label_font_metrics.width = 0.0f;
    field->label_font_metrics.height = 0.0f;
    field->label_font_metrics.baseline = 0.0f;
  }

  field->font_metrics_valid = M3_TRUE;
  return M3_OK;
}

static int m3_text_field_sync_label(M3TextField *field) {
  M3Scalar target;
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (field->utf8_label == NULL || field->label_len == 0u) {
    field->label_value = 0.0f;
    rc = m3_anim_controller_start_timing(&field->label_anim, 0.0f, 0.0f, 0.0f,
                                         M3_ANIM_EASE_OUT);
#ifdef M3_TESTING
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_ANIM_START)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    return M3_OK;
  }

  target = (field->focused == M3_TRUE || field->utf8_len > 0u) ? 1.0f : 0.0f;
  if (field->label_value == target) {
    return M3_OK;
  }

  rc = m3_anim_controller_start_timing(&field->label_anim, field->label_value,
                                       target, field->style.label_anim_duration,
                                       M3_ANIM_EASE_OUT);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(M3_TEXT_FIELD_TEST_FAIL_ANIM_START)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  if (field->style.label_anim_duration == 0.0f) {
    field->label_value = target;
  }

  return M3_OK;
}

static void m3_text_field_reset_cursor_blink(M3TextField *field) {
  if (field == NULL) {
    return;
  }
  field->cursor_visible = M3_TRUE;
  field->cursor_timer = 0.0f;
}

static int m3_text_field_set_text_internal(M3TextField *field,
                                           const char *utf8_text,
                                           m3_usize utf8_len, M3Bool notify) {
  m3_usize required;
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_field_validate_utf8(utf8_text, utf8_len);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_field_add_overflow(utf8_len, 1u, &required);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_field_reserve(field, required);
  if (rc != M3_OK) {
    return rc;
  }

  if (utf8_len > 0u && utf8_text != NULL) {
    memcpy(field->utf8, utf8_text, utf8_len);
  }
  field->utf8[utf8_len] = '\0';
  field->utf8_len = utf8_len;
  field->cursor = utf8_len;
  field->selection_start = utf8_len;
  field->selection_end = utf8_len;
  field->text_metrics_valid = M3_FALSE;
  field->placeholder_metrics_valid = M3_FALSE;
  m3_text_field_reset_cursor_blink(field);

  rc = m3_text_field_sync_label(field);
  if (rc != M3_OK) {
    return rc;
  }

  if (notify == M3_TRUE && field->on_change != NULL) {
    rc =
        field->on_change(field->on_change_ctx, field,
                         field->utf8_len ? field->utf8 : NULL, field->utf8_len);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

static int m3_text_field_delete_range(M3TextField *field, m3_usize start,
                                      m3_usize end, M3Bool notify) {
  m3_usize tail_len;
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (start > end) {
    return M3_ERR_RANGE;
  }
  if (end > field->utf8_len) {
    return M3_ERR_RANGE;
  }

  tail_len = field->utf8_len - end;
  if (tail_len > 0u) {
    memmove(field->utf8 + start, field->utf8 + end, tail_len);
  }
  field->utf8_len = start + tail_len;
  field->utf8[field->utf8_len] = '\0';

  field->cursor = start;
  field->selection_start = start;
  field->selection_end = start;
  field->text_metrics_valid = M3_FALSE;
  field->placeholder_metrics_valid = M3_FALSE;
  m3_text_field_reset_cursor_blink(field);

  rc = m3_text_field_sync_label(field);
  if (rc != M3_OK) {
    return rc;
  }

  if (notify == M3_TRUE && field->on_change != NULL) {
    rc =
        field->on_change(field->on_change_ctx, field,
                         field->utf8_len ? field->utf8 : NULL, field->utf8_len);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

static int m3_text_field_measure_prefix(M3TextField *field, m3_usize offset,
                                        M3Scalar *out_width) {
  M3TextMetrics metrics;
  int rc;

  if (field == NULL || out_width == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_MEASURE_PREFIX)) {
    return M3_ERR_IO;
  }
#endif
  if (offset > field->utf8_len) {
    return M3_ERR_RANGE;
  }

  rc = m3_text_field_validate_offset(field->utf8, field->utf8_len, offset);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_measure_utf8(&field->text_backend, field->text_font, field->utf8,
                            offset, &metrics);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  *out_width = metrics.width;
  return M3_OK;
}

static int m3_text_field_offset_for_x(M3TextField *field, M3Scalar x,
                                      m3_usize *out_offset) {
  M3Utf8Iter iter;
  m3_u32 codepoint;
  m3_usize prev_offset;
  m3_usize candidate;
  M3Scalar prev_width;
  M3Scalar width;
  int rc;

  if (field == NULL || out_offset == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (field->utf8_len == 0u) {
    *out_offset = 0u;
    return M3_OK;
  }

  if (x <= 0.0f) {
    *out_offset = 0u;
    return M3_OK;
  }

  rc = m3_utf8_iter_init(&iter, field->utf8, field->utf8_len);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  prev_offset = 0u;
  prev_width = 0.0f;
  while (1) {
    rc = m3_utf8_iter_next(&iter, &codepoint);
#ifdef M3_TESTING
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT)) {
      rc = M3_ERR_IO;
    }
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND)) {
      rc = M3_ERR_NOT_FOUND;
    }
#endif
    if (rc == M3_ERR_NOT_FOUND) {
      break;
    }
    if (rc != M3_OK) {
      return rc;
    }
    candidate = iter.offset;
    rc = m3_text_field_measure_prefix(field, candidate, &width);
    if (rc != M3_OK) {
      return rc;
    }
    if (width >= x) {
      if ((x - prev_width) <= (width - x)) {
        *out_offset = prev_offset;
      } else {
        *out_offset = candidate;
      }
      return M3_OK;
    }
    prev_offset = candidate;
    prev_width = width;
    M3_UNUSED(codepoint);
  }

  *out_offset = field->utf8_len;
  return M3_OK;
}

static int m3_text_field_resolve_colors(
    const M3TextField *field, M3Color *out_container, M3Color *out_outline,
    M3Color *out_text, M3Color *out_label, M3Color *out_placeholder,
    M3Color *out_cursor, M3Color *out_selection, M3Color *out_handle) {
  int rc;

  if (field == NULL || out_container == NULL || out_outline == NULL ||
      out_text == NULL || out_label == NULL || out_placeholder == NULL ||
      out_cursor == NULL || out_selection == NULL || out_handle == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_RESOLVE_COLORS)) {
    return M3_ERR_IO;
  }
#endif

  if (field->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    *out_container = field->style.disabled_container_color;
    *out_outline = field->style.disabled_outline_color;
    *out_text = field->style.disabled_text_color;
    *out_label = field->style.disabled_label_color;
    *out_placeholder = field->style.disabled_placeholder_color;
  } else {
    *out_container = field->style.container_color;
    *out_outline = (field->focused == M3_TRUE)
                       ? field->style.focused_outline_color
                       : field->style.outline_color;
    *out_text = field->style.text_style.color;
    *out_label = field->style.label_style.color;
    *out_placeholder = field->style.placeholder_color;
  }

  *out_cursor = field->style.cursor_color;
  *out_selection = field->style.selection_color;
  *out_handle = field->style.handle_color;

  rc = m3_text_field_validate_color(out_container);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(out_outline);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(out_text);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(out_label);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(out_placeholder);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(out_cursor);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(out_selection);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_color(out_handle);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_text_field_widget_measure(void *widget, M3MeasureSpec width,
                                        M3MeasureSpec height,
                                        M3Size *out_size) {
  M3TextField *field;
  M3Scalar desired_width;
  M3Scalar desired_height;
  M3TextMetrics text_metrics;
  M3TextMetrics label_metrics;
  M3TextMetrics placeholder_metrics;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_field_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc;
  }

  field = (M3TextField *)widget;

  rc = m3_text_field_validate_style(
      &field->style, M3_TRUE,
      (field->utf8_label != NULL && field->label_len > 0u) ? M3_TRUE
                                                           : M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_field_update_text_metrics(field);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_update_label_metrics(field);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_update_placeholder_metrics(field);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_update_font_metrics(field);
  if (rc != M3_OK) {
    return rc;
  }

  text_metrics = field->text_metrics;
  label_metrics = field->label_metrics;
  placeholder_metrics = field->placeholder_metrics;

  desired_width = text_metrics.width;
  if (label_metrics.width > desired_width) {
    desired_width = label_metrics.width;
  }
  if (placeholder_metrics.width > desired_width) {
    desired_width = placeholder_metrics.width;
  }
  desired_width += field->style.padding_x * 2.0f;

  desired_height = field->text_font_metrics.height;
  if (field->label_font_metrics.height > desired_height) {
    desired_height = field->label_font_metrics.height;
  }
  desired_height += field->style.padding_y * 2.0f;
  if (desired_height < field->style.min_height) {
    desired_height = field->style.min_height;
  }

  if (width.mode == M3_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == M3_MEASURE_AT_MOST) {
    out_size->width = (desired_width > width.size) ? width.size : desired_width;
  } else {
    out_size->width = desired_width;
  }

  if (height.mode == M3_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == M3_MEASURE_AT_MOST) {
    out_size->height =
        (desired_height > height.size) ? height.size : desired_height;
  } else {
    out_size->height = desired_height;
  }

  return M3_OK;
}

static int m3_text_field_widget_layout(void *widget, M3Rect bounds) {
  M3TextField *field;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_field_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  field = (M3TextField *)widget;
  field->bounds = bounds;
  return M3_OK;
}

static int m3_text_field_widget_paint(void *widget, M3PaintContext *ctx) {
  M3TextField *field;
  M3Rect bounds;
  M3Rect inner;
  M3Rect content;
  M3Rect selection_rect;
  M3Rect handle_rect;
  M3Scalar outline_width;
  M3Scalar corner_radius;
  M3Scalar inner_corner;
  M3Scalar available_height;
  M3Scalar text_top;
  M3Scalar text_baseline;
  M3Scalar label_baseline;
  M3Scalar label_rest_y;
  M3Scalar label_float_y;
  M3Scalar label_y;
  M3Scalar label_x;
  M3Scalar text_x;
  M3Scalar caret_x;
  M3Scalar start_x;
  M3Scalar end_x;
  M3Scalar temp_scalar;
  M3Scalar handle_radius;
  M3Scalar handle_height;
  M3Color container;
  M3Color outline;
  M3Color text_color;
  M3Color label_color;
  M3Color placeholder_color;
  M3Color cursor_color;
  M3Color selection_color;
  M3Color handle_color;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL || ctx->gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }
  if (ctx->gfx->text_vtable == NULL ||
      ctx->gfx->text_vtable->draw_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  field = (M3TextField *)widget;

  rc = m3_text_field_validate_style(
      &field->style, M3_TRUE,
      (field->utf8_label != NULL && field->label_len > 0u) ? M3_TRUE
                                                           : M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_field_validate_rect(&field->bounds);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_field_update_text_metrics(field);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_update_label_metrics(field);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_update_placeholder_metrics(field);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_update_font_metrics(field);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_field_resolve_colors(
      field, &container, &outline, &text_color, &label_color,
      &placeholder_color, &cursor_color, &selection_color, &handle_color);
  if (rc != M3_OK) {
    return rc;
  }

  bounds = field->bounds;
  outline_width = field->style.outline_width;
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_OUTLINE_RANGE)) {
    outline_width = -1.0f;
  }
#endif
  if (outline_width < 0.0f) {
    return M3_ERR_RANGE;
  }

  corner_radius = field->style.corner_radius;
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_CORNER_RANGE)) {
    corner_radius = -1.0f;
  }
#endif
  if (corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (outline_width > 0.0f) {
    inner.x = bounds.x + outline_width;
    inner.y = bounds.y + outline_width;
    inner.width = bounds.width - outline_width * 2.0f;
    inner.height = bounds.height - outline_width * 2.0f;
    if (inner.width < 0.0f || inner.height < 0.0f) {
      return M3_ERR_RANGE;
    }

    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bounds, outline,
                                     corner_radius);
    if (rc != M3_OK) {
      return rc;
    }

    inner_corner = corner_radius - outline_width;
    if (inner_corner < 0.0f) {
      inner_corner = 0.0f;
    }
  } else {
    inner = bounds;
    inner_corner = corner_radius;
  }

  if (container.a > 0.0f) {
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &inner, container,
                                     inner_corner);
    if (rc != M3_OK) {
      return rc;
    }
  }

  content.x = bounds.x + field->style.padding_x;
  content.y = bounds.y + field->style.padding_y;
  content.width = bounds.width - field->style.padding_x * 2.0f;
  content.height = bounds.height - field->style.padding_y * 2.0f;
  if (content.width < 0.0f) {
    content.width = 0.0f;
  }
  if (content.height < 0.0f) {
    content.height = 0.0f;
  }

  available_height = content.height - field->text_font_metrics.height;
  if (available_height < 0.0f) {
    available_height = 0.0f;
  }
  text_top = content.y + available_height * 0.5f;
  text_baseline = field->text_font_metrics.baseline;
  text_x = content.x;

  if (ctx->gfx->vtable->push_clip != NULL) {
    rc = ctx->gfx->vtable->push_clip(ctx->gfx->ctx, &inner);
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (field->selection_start != field->selection_end &&
      selection_color.a > 0.0f) {
    rc = m3_text_field_measure_prefix(field, field->selection_start, &start_x);
    if (rc != M3_OK) {
      return rc;
    }
    rc = m3_text_field_measure_prefix(field, field->selection_end, &end_x);
    if (rc != M3_OK) {
      return rc;
    }
    if (end_x < start_x) {
      temp_scalar = start_x;
      start_x = end_x;
      end_x = temp_scalar;
    }
    selection_rect.x = text_x + start_x;
    selection_rect.y = text_top;
    selection_rect.width = end_x - start_x;
#ifdef M3_TESTING
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_SELECTION_WIDTH_NEGATIVE)) {
      selection_rect.width = -1.0f;
    }
#endif
    selection_rect.height = field->text_font_metrics.height;
    if (selection_rect.width < 0.0f) {
      selection_rect.width = 0.0f;
    }
    if (selection_rect.height < 0.0f) {
      selection_rect.height = 0.0f;
    }
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &selection_rect,
                                     selection_color, 0.0f);
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (field->utf8_len > 0u) {
    rc = ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, field->text_font,
                                          field->utf8, field->utf8_len, text_x,
                                          text_top + text_baseline, text_color);
    if (rc != M3_OK) {
      return rc;
    }
  } else if ((field->utf8_placeholder != NULL && field->placeholder_len > 0u) &&
             (field->utf8_label == NULL || field->label_len == 0u ||
              field->label_value > 0.0f)) {
    rc = ctx->gfx->text_vtable->draw_text(
        ctx->gfx->ctx, field->text_font, field->utf8_placeholder,
        field->placeholder_len, text_x, text_top + text_baseline,
        placeholder_color);
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (field->utf8_label != NULL && field->label_len > 0u &&
      field->label_font.id != 0u) {
    label_baseline = field->label_font_metrics.baseline;
    label_x = content.x;
    label_rest_y = text_top + text_baseline;
    label_float_y = bounds.y + field->style.label_float_offset + label_baseline;
    label_y =
        label_rest_y + (label_float_y - label_rest_y) * field->label_value;
    rc = ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, field->label_font,
                                          field->utf8_label, field->label_len,
                                          label_x, label_y, label_color);
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (field->focused == M3_TRUE &&
      (field->widget.flags & M3_WIDGET_FLAG_DISABLED) == 0 &&
      field->cursor_visible == M3_TRUE && field->style.cursor_width > 0.0f &&
      cursor_color.a > 0.0f) {
    rc = m3_text_field_measure_prefix(field, field->cursor, &caret_x);
    if (rc != M3_OK) {
      return rc;
    }
    selection_rect.x = text_x + caret_x;
    selection_rect.y = text_top;
    selection_rect.width = field->style.cursor_width;
    selection_rect.height = field->text_font_metrics.height;
#ifdef M3_TESTING
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_CURSOR_WIDTH_NEGATIVE)) {
      selection_rect.width = -1.0f;
    }
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_CURSOR_HEIGHT_NEGATIVE)) {
      selection_rect.height = -1.0f;
    }
#endif
    if (selection_rect.width < 0.0f) {
      selection_rect.width = 0.0f;
    }
    if (selection_rect.height < 0.0f) {
      selection_rect.height = 0.0f;
    }
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &selection_rect,
                                     cursor_color, 0.0f);
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (field->selection_start != field->selection_end && handle_color.a > 0.0f) {
    handle_radius = field->style.handle_radius;
    handle_height = field->style.handle_height;
    if (handle_height < handle_radius * 2.0f) {
      handle_height = handle_radius * 2.0f;
    }
    if (handle_radius > 0.0f && handle_height > 0.0f) {
      rc =
          m3_text_field_measure_prefix(field, field->selection_start, &start_x);
      if (rc != M3_OK) {
        return rc;
      }
      rc = m3_text_field_measure_prefix(field, field->selection_end, &end_x);
      if (rc != M3_OK) {
        return rc;
      }
      if (end_x < start_x) {
        temp_scalar = start_x;
        start_x = end_x;
        end_x = temp_scalar;
      }

      handle_rect.width = handle_radius * 2.0f;
      handle_rect.height = handle_height;
      handle_rect.x = text_x + start_x - handle_radius;
      handle_rect.y =
          text_top + field->text_font_metrics.height - handle_height * 0.5f;
      rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &handle_rect,
                                       handle_color, handle_radius);
      if (rc != M3_OK) {
        return rc;
      }

      handle_rect.x = text_x + end_x - handle_radius;
      rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &handle_rect,
                                       handle_color, handle_radius);
      if (rc != M3_OK) {
        return rc;
      }
    }
  }

  if (ctx->gfx->vtable->pop_clip != NULL) {
    rc = ctx->gfx->vtable->pop_clip(ctx->gfx->ctx);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

static int m3_text_field_widget_event(void *widget, const M3InputEvent *event,
                                      M3Bool *out_handled) {
  M3TextField *field;
  m3_usize offset;
  m3_usize other;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;

  field = (M3TextField *)widget;

  if (field->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_OK;
  }

  switch (event->type) {
  case M3_INPUT_POINTER_DOWN:
    rc =
        m3_text_field_offset_for_x(field,
                                   (M3Scalar)event->data.pointer.x -
                                       field->bounds.x - field->style.padding_x,
                                   &offset);
    if (rc != M3_OK) {
      return rc;
    }
    field->cursor = offset;
    field->selection_start = offset;
    field->selection_end = offset;
    field->selecting = M3_TRUE;
    field->focused = M3_TRUE;
    rc = m3_text_field_sync_label(field);
#ifdef M3_TESTING
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    m3_text_field_reset_cursor_blink(field);
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_POINTER_MOVE:
    if (field->selecting == M3_FALSE) {
      return M3_OK;
    }
    rc =
        m3_text_field_offset_for_x(field,
                                   (M3Scalar)event->data.pointer.x -
                                       field->bounds.x - field->style.padding_x,
                                   &offset);
    if (rc != M3_OK) {
      return rc;
    }
    field->cursor = offset;
    field->selection_end = offset;
    m3_text_field_reset_cursor_blink(field);
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_POINTER_UP:
    if (field->selecting == M3_FALSE) {
      return M3_OK;
    }
    field->selecting = M3_FALSE;
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_TEXT:
    if (event->data.text.length == 0u) {
      return M3_OK;
    }
    rc = m3_text_field_insert_utf8(field, event->data.text.utf8,
                                   (m3_usize)event->data.text.length);
    if (rc != M3_OK) {
      return rc;
    }
    field->focused = M3_TRUE;
    rc = m3_text_field_sync_label(field);
#ifdef M3_TESTING
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_TEXT_UTF8:
    if (event->data.text_utf8.length == 0u) {
      return M3_OK;
    }
    rc = m3_text_field_insert_utf8(field, event->data.text_utf8.utf8,
                                   event->data.text_utf8.length);
    if (rc != M3_OK) {
      return rc;
    }
    field->focused = M3_TRUE;
    rc = m3_text_field_sync_label(field);
#ifdef M3_TESTING
    if (m3_text_field_test_fail_point_match(
            M3_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_TEXT_EDIT:
    field->focused = M3_TRUE;
    rc = m3_text_field_sync_label(field);
    if (rc != M3_OK) {
      return rc;
    }
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_KEY_DOWN:
    if (event->data.key.key_code == 8u || event->data.key.key_code == 127u ||
        event->data.key.key_code == 46u) {
      rc = (event->data.key.key_code == 46u)
               ? m3_text_field_delete_forward(field)
               : m3_text_field_backspace(field);
      if (rc != M3_OK) {
        return rc;
      }
      field->focused = M3_TRUE;
      rc = m3_text_field_sync_label(field);
      if (rc != M3_OK) {
        return rc;
      }
      *out_handled = M3_TRUE;
      return M3_OK;
    }
    if (event->data.key.key_code == 37u || event->data.key.key_code == 0x25u ||
        event->data.key.key_code == 1073741904u) {
      rc = m3_text_field_prev_offset(field->utf8, field->utf8_len,
                                     field->cursor, &other);
      if (rc != M3_OK) {
        return rc;
      }
      field->cursor = other;
      field->selection_start = other;
      field->selection_end = other;
      m3_text_field_reset_cursor_blink(field);
      field->focused = M3_TRUE;
      *out_handled = M3_TRUE;
      return M3_OK;
    }
    if (event->data.key.key_code == 39u || event->data.key.key_code == 0x27u ||
        event->data.key.key_code == 1073741903u) {
      rc = m3_text_field_next_offset(field->utf8, field->utf8_len,
                                     field->cursor, &other);
      if (rc != M3_OK) {
        return rc;
      }
      field->cursor = other;
      field->selection_start = other;
      field->selection_end = other;
      m3_text_field_reset_cursor_blink(field);
      field->focused = M3_TRUE;
      *out_handled = M3_TRUE;
      return M3_OK;
    }
    if (event->data.key.key_code == 36u || event->data.key.key_code == 0x24u ||
        event->data.key.key_code == 1073741898u) {
      field->cursor = 0u;
      field->selection_start = 0u;
      field->selection_end = 0u;
      m3_text_field_reset_cursor_blink(field);
      field->focused = M3_TRUE;
      *out_handled = M3_TRUE;
      return M3_OK;
    }
    if (event->data.key.key_code == 35u || event->data.key.key_code == 0x23u ||
        event->data.key.key_code == 1073741901u) {
      field->cursor = field->utf8_len;
      field->selection_start = field->utf8_len;
      field->selection_end = field->utf8_len;
      m3_text_field_reset_cursor_blink(field);
      field->focused = M3_TRUE;
      *out_handled = M3_TRUE;
      return M3_OK;
    }
    return M3_OK;
  default:
    return M3_OK;
  }
}

static int m3_text_field_widget_get_semantics(void *widget,
                                              M3Semantics *out_semantics) {
  M3TextField *field;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  field = (M3TextField *)widget;
  out_semantics->role = M3_SEMANTIC_TEXT_FIELD;
  out_semantics->flags = 0;
  if (field->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  if (field->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
  }
  if (field->focused == M3_TRUE) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSED;
  }
  out_semantics->utf8_label = field->utf8_label;
  out_semantics->utf8_hint = field->utf8_placeholder;
  out_semantics->utf8_value = field->utf8_len ? field->utf8 : NULL;
  return M3_OK;
}

static int m3_text_field_widget_destroy(void *widget) {
  M3TextField *field;
  int rc;
  int rc2;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  field = (M3TextField *)widget;
  rc = M3_OK;

  if (field->owns_fonts == M3_TRUE) {
    if (field->text_font.id != 0u || field->text_font.generation != 0u) {
      if (field->text_backend.vtable != NULL &&
          field->text_backend.vtable->destroy_font != NULL) {
        rc2 = field->text_backend.vtable->destroy_font(field->text_backend.ctx,
                                                       field->text_font);
        if (rc2 != M3_OK && rc == M3_OK) {
          rc = rc2;
        }
      } else if (rc == M3_OK) {
        rc = M3_ERR_UNSUPPORTED;
      }
    }
    if (field->label_font.id != 0u || field->label_font.generation != 0u) {
      if (field->text_backend.vtable != NULL &&
          field->text_backend.vtable->destroy_font != NULL) {
        rc2 = field->text_backend.vtable->destroy_font(field->text_backend.ctx,
                                                       field->label_font);
        if (rc2 != M3_OK && rc == M3_OK) {
          rc = rc2;
        }
      } else if (rc == M3_OK) {
        rc = M3_ERR_UNSUPPORTED;
      }
    }
  }

  if (field->utf8 != NULL && field->allocator.free != NULL) {
    field->allocator.free(field->allocator.ctx, field->utf8);
  }

  field->utf8 = NULL;
  field->utf8_len = 0u;
  field->utf8_capacity = 0u;
  field->cursor = 0u;
  field->selection_start = 0u;
  field->selection_end = 0u;
  field->selecting = M3_FALSE;
  field->text_metrics_valid = M3_FALSE;
  field->label_metrics_valid = M3_FALSE;
  field->placeholder_metrics_valid = M3_FALSE;
  field->font_metrics_valid = M3_FALSE;
  field->owns_fonts = M3_FALSE;
  field->focused = M3_FALSE;
  field->cursor_visible = M3_FALSE;
  field->cursor_timer = 0.0f;
  field->label_value = 0.0f;
  field->utf8_label = NULL;
  field->label_len = 0u;
  field->utf8_placeholder = NULL;
  field->placeholder_len = 0u;
  field->on_change = NULL;
  field->on_change_ctx = NULL;
  field->text_backend.ctx = NULL;
  field->text_backend.vtable = NULL;
  field->widget.ctx = NULL;
  field->widget.vtable = NULL;
  field->text_font.id = 0u;
  field->text_font.generation = 0u;
  field->label_font.id = 0u;
  field->label_font.generation = 0u;
  return rc;
}

static const M3WidgetVTable g_m3_text_field_widget_vtable = {
    m3_text_field_widget_measure,       m3_text_field_widget_layout,
    m3_text_field_widget_paint,         m3_text_field_widget_event,
    m3_text_field_widget_get_semantics, m3_text_field_widget_destroy};

int M3_CALL m3_text_field_style_init(M3TextFieldStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));

  rc = m3_text_style_init(&style->text_style);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_TEXT_STYLE_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_style_init(&style->label_style);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(
          M3_TEXT_FIELD_TEST_FAIL_LABEL_STYLE_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  style->padding_x = M3_TEXT_FIELD_DEFAULT_PADDING_X;
  style->padding_y = M3_TEXT_FIELD_DEFAULT_PADDING_Y;
  style->min_height = M3_TEXT_FIELD_DEFAULT_MIN_HEIGHT;
  style->corner_radius = M3_TEXT_FIELD_DEFAULT_CORNER_RADIUS;
  style->outline_width = M3_TEXT_FIELD_DEFAULT_OUTLINE_WIDTH;
  style->label_float_offset = M3_TEXT_FIELD_DEFAULT_LABEL_FLOAT_OFFSET;
  style->label_anim_duration = M3_TEXT_FIELD_DEFAULT_LABEL_ANIM_DURATION;
  style->cursor_width = M3_TEXT_FIELD_DEFAULT_CURSOR_WIDTH;
  style->cursor_blink_period = M3_TEXT_FIELD_DEFAULT_CURSOR_BLINK;
  style->handle_radius = M3_TEXT_FIELD_DEFAULT_HANDLE_RADIUS;
  style->handle_height = M3_TEXT_FIELD_DEFAULT_HANDLE_HEIGHT;

  rc = m3_text_field_color_set(&style->container_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.38f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_color_set(&style->focused_outline_color, 0.2f, 0.4f, 0.9f,
                               1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_color_set(&style->disabled_container_color, 0.95f, 0.95f,
                               0.95f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_color_set(&style->disabled_outline_color, 0.0f, 0.0f, 0.0f,
                               0.12f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_color_set(&style->disabled_text_color, 0.0f, 0.0f, 0.0f,
                               0.38f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_color_set(&style->disabled_label_color, 0.0f, 0.0f, 0.0f,
                               0.38f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_color_set(&style->placeholder_color, 0.0f, 0.0f, 0.0f,
                               0.6f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_color_set(&style->disabled_placeholder_color, 0.0f, 0.0f,
                               0.0f, 0.38f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_color_set(&style->cursor_color, 0.2f, 0.4f, 0.9f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_color_set(&style->selection_color, 0.2f, 0.4f, 0.9f, 0.3f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_color_set(&style->handle_color, 0.2f, 0.4f, 0.9f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_text_field_init(M3TextField *field, const M3TextBackend *backend,
                               const M3TextFieldStyle *style,
                               const M3Allocator *allocator,
                               const char *utf8_text, m3_usize utf8_len) {
  M3Allocator alloc;
  int rc;

  if (field == NULL || backend == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_field_validate_backend(backend);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_field_validate_style(style, M3_TRUE, M3_FALSE);
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

  memset(field, 0, sizeof(*field));
  field->text_backend = *backend;
  field->style = *style;
  field->allocator = alloc;
  field->utf8 = NULL;
  field->utf8_len = 0u;
  field->utf8_capacity = 0u;
  field->cursor = 0u;
  field->selection_start = 0u;
  field->selection_end = 0u;
  field->selecting = M3_FALSE;
  field->text_metrics_valid = M3_FALSE;
  field->label_metrics_valid = M3_FALSE;
  field->placeholder_metrics_valid = M3_FALSE;
  field->font_metrics_valid = M3_FALSE;
  field->owns_fonts = M3_TRUE;
  field->focused = M3_FALSE;
  field->cursor_visible = M3_FALSE;
  field->cursor_timer = 0.0f;
  field->label_value = 0.0f;
  field->utf8_label = NULL;
  field->label_len = 0u;
  field->utf8_placeholder = NULL;
  field->placeholder_len = 0u;
  field->on_change = NULL;
  field->on_change_ctx = NULL;

  rc = m3_anim_controller_init(&field->label_anim);
#ifdef M3_TESTING
  if (m3_text_field_test_fail_point_match(M3_TEXT_FIELD_TEST_FAIL_ANIM_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  field->text_font.id = 0u;
  field->text_font.generation = 0u;
  field->label_font.id = 0u;
  field->label_font.generation = 0u;

  rc = m3_text_font_create(backend, &style->text_style, &field->text_font);
  if (rc != M3_OK) {
    return rc;
  }

  if (style->label_style.utf8_family != NULL) {
    rc = m3_text_font_create(backend, &style->label_style, &field->label_font);
    if (rc != M3_OK) {
      m3_text_font_destroy(backend, field->text_font);
      return rc;
    }
  }

  rc = m3_text_field_set_text_internal(field, utf8_text, utf8_len, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }

  field->widget.ctx = field;
  field->widget.vtable = &g_m3_text_field_widget_vtable;
  field->widget.handle.id = 0u;
  field->widget.handle.generation = 0u;
  field->widget.flags = M3_WIDGET_FLAG_FOCUSABLE;
  return M3_OK;
}

int M3_CALL m3_text_field_set_text(M3TextField *field, const char *utf8_text,
                                   m3_usize utf8_len) {
  return m3_text_field_set_text_internal(field, utf8_text, utf8_len, M3_TRUE);
}

int M3_CALL m3_text_field_get_text(const M3TextField *field,
                                   const char **out_utf8, m3_usize *out_len) {
  if (field == NULL || out_utf8 == NULL || out_len == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_utf8 = (field->utf8_len > 0u) ? field->utf8 : NULL;
  *out_len = field->utf8_len;
  return M3_OK;
}

int M3_CALL m3_text_field_insert_utf8(M3TextField *field, const char *utf8_text,
                                      m3_usize utf8_len) {
  m3_usize insert_at;
  m3_usize tail_len;
  m3_usize new_len;
  m3_usize required;
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8_text == NULL && utf8_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_field_validate_utf8(utf8_text, utf8_len);
  if (rc != M3_OK) {
    return rc;
  }

  if (field->selection_start != field->selection_end) {
    rc = m3_text_field_delete_range(field, field->selection_start,
                                    field->selection_end, M3_FALSE);
    if (rc != M3_OK) {
      return rc;
    }
  }

  insert_at = field->cursor;
  if (insert_at > field->utf8_len) {
    return M3_ERR_RANGE;
  }

  rc = m3_text_field_validate_offset(field->utf8, field->utf8_len, insert_at);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_field_add_overflow(field->utf8_len, utf8_len, &new_len);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_add_overflow(new_len, 1u, &required);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_field_reserve(field, required);
  if (rc != M3_OK) {
    return rc;
  }

  tail_len = field->utf8_len - insert_at;
  if (tail_len > 0u && utf8_len > 0u) {
    memmove(field->utf8 + insert_at + utf8_len, field->utf8 + insert_at,
            tail_len);
  }
  if (utf8_len > 0u && utf8_text != NULL) {
    memcpy(field->utf8 + insert_at, utf8_text, utf8_len);
  }

  field->utf8_len = new_len;
  field->utf8[new_len] = '\0';
  field->cursor = insert_at + utf8_len;
  field->selection_start = field->cursor;
  field->selection_end = field->cursor;
  field->text_metrics_valid = M3_FALSE;
  field->placeholder_metrics_valid = M3_FALSE;
  m3_text_field_reset_cursor_blink(field);

  rc = m3_text_field_sync_label(field);
  if (rc != M3_OK) {
    return rc;
  }

  if (field->on_change != NULL) {
    rc =
        field->on_change(field->on_change_ctx, field,
                         field->utf8_len ? field->utf8 : NULL, field->utf8_len);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

int M3_CALL m3_text_field_delete_selection(M3TextField *field) {
  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (field->selection_start == field->selection_end) {
    return M3_OK;
  }

  return m3_text_field_delete_range(field, field->selection_start,
                                    field->selection_end, M3_TRUE);
}

int M3_CALL m3_text_field_backspace(M3TextField *field) {
  m3_usize prev;
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (field->selection_start != field->selection_end) {
    return m3_text_field_delete_selection(field);
  }

  if (field->cursor == 0u) {
    return M3_OK;
  }

  rc = m3_text_field_prev_offset(field->utf8, field->utf8_len, field->cursor,
                                 &prev);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_text_field_delete_range(field, prev, field->cursor, M3_TRUE);
}

int M3_CALL m3_text_field_delete_forward(M3TextField *field) {
  m3_usize next;
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (field->selection_start != field->selection_end) {
    return m3_text_field_delete_selection(field);
  }

  if (field->cursor >= field->utf8_len) {
    return M3_OK;
  }

  rc = m3_text_field_next_offset(field->utf8, field->utf8_len, field->cursor,
                                 &next);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_text_field_delete_range(field, field->cursor, next, M3_TRUE);
}

int M3_CALL m3_text_field_set_label(M3TextField *field, const char *utf8_label,
                                    m3_usize label_len) {
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && label_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_field_validate_utf8(utf8_label, label_len);
  if (rc != M3_OK) {
    return rc;
  }

  if ((utf8_label != NULL && label_len > 0u) &&
      (field->label_font.id == 0u && field->label_font.generation == 0u)) {
    if (field->style.label_style.utf8_family == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    rc = m3_text_font_create(&field->text_backend, &field->style.label_style,
                             &field->label_font);
    if (rc != M3_OK) {
      return rc;
    }
  }

  field->utf8_label = utf8_label;
  field->label_len = label_len;
  field->label_metrics_valid = M3_FALSE;
  field->font_metrics_valid = M3_FALSE;

  rc = m3_text_field_sync_label(field);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_text_field_set_placeholder(M3TextField *field,
                                          const char *utf8_placeholder,
                                          m3_usize placeholder_len) {
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8_placeholder == NULL && placeholder_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_text_field_validate_utf8(utf8_placeholder, placeholder_len);
  if (rc != M3_OK) {
    return rc;
  }

  field->utf8_placeholder = utf8_placeholder;
  field->placeholder_len = placeholder_len;
  field->placeholder_metrics_valid = M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_text_field_set_style(M3TextField *field,
                                    const M3TextFieldStyle *style) {
  M3Handle new_text_font;
  M3Handle new_label_font;
  M3Bool need_label;
  int rc_destroy;
  int rc2;
  int rc;

  if (field == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  need_label =
      (field->utf8_label != NULL && field->label_len > 0u) ? M3_TRUE : M3_FALSE;
  rc = m3_text_field_validate_style(style, M3_TRUE, need_label);
  if (rc != M3_OK) {
    return rc;
  }

  rc_destroy = M3_OK;

  new_text_font.id = 0u;
  new_text_font.generation = 0u;
  new_label_font.id = 0u;
  new_label_font.generation = 0u;

  rc = m3_text_font_create(&field->text_backend, &style->text_style,
                           &new_text_font);
  if (rc != M3_OK) {
    return rc;
  }

  if (need_label == M3_TRUE) {
    rc = m3_text_font_create(&field->text_backend, &style->label_style,
                             &new_label_font);
    if (rc != M3_OK) {
      m3_text_font_destroy(&field->text_backend, new_text_font);
      return rc;
    }
  }

  if (field->owns_fonts == M3_TRUE) {
    rc2 = m3_text_font_destroy(&field->text_backend, field->text_font);
    if (rc2 != M3_OK) {
      rc_destroy = rc2;
    }
    if (field->label_font.id != 0u || field->label_font.generation != 0u) {
      rc2 = m3_text_font_destroy(&field->text_backend, field->label_font);
      if (rc2 != M3_OK && rc_destroy == M3_OK) {
        rc_destroy = rc2;
      }
    }
  }

  field->style = *style;
  field->text_font = new_text_font;
  field->label_font = new_label_font;
  field->owns_fonts = M3_TRUE;
  field->text_metrics_valid = M3_FALSE;
  field->label_metrics_valid = M3_FALSE;
  field->placeholder_metrics_valid = M3_FALSE;
  field->font_metrics_valid = M3_FALSE;

  rc = m3_text_field_sync_label(field);
  if (rc != M3_OK) {
    return rc;
  }
  if (rc_destroy != M3_OK) {
    return rc_destroy;
  }
  return M3_OK;
}

int M3_CALL m3_text_field_set_selection(M3TextField *field, m3_usize start,
                                        m3_usize end) {
  m3_usize tmp;
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (start > field->utf8_len || end > field->utf8_len) {
    return M3_ERR_RANGE;
  }

  rc = m3_text_field_validate_offset(field->utf8, field->utf8_len, start);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_text_field_validate_offset(field->utf8, field->utf8_len, end);
  if (rc != M3_OK) {
    return rc;
  }

  if (start > end) {
    tmp = start;
    start = end;
    end = tmp;
  }

  field->selection_start = start;
  field->selection_end = end;
  field->cursor = end;
  m3_text_field_reset_cursor_blink(field);
  return M3_OK;
}

int M3_CALL m3_text_field_get_selection(const M3TextField *field,
                                        m3_usize *out_start,
                                        m3_usize *out_end) {
  if (field == NULL || out_start == NULL || out_end == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_start = field->selection_start;
  *out_end = field->selection_end;
  return M3_OK;
}

int M3_CALL m3_text_field_set_cursor(M3TextField *field, m3_usize cursor) {
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (cursor > field->utf8_len) {
    return M3_ERR_RANGE;
  }

  rc = m3_text_field_validate_offset(field->utf8, field->utf8_len, cursor);
  if (rc != M3_OK) {
    return rc;
  }

  field->cursor = cursor;
  field->selection_start = cursor;
  field->selection_end = cursor;
  m3_text_field_reset_cursor_blink(field);
  return M3_OK;
}

int M3_CALL m3_text_field_get_cursor(const M3TextField *field,
                                     m3_usize *out_cursor) {
  if (field == NULL || out_cursor == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_cursor = field->cursor;
  return M3_OK;
}

int M3_CALL m3_text_field_set_focus(M3TextField *field, M3Bool focused) {
  int rc;

  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (focused != M3_FALSE && focused != M3_TRUE) {
    return M3_ERR_RANGE;
  }

  field->focused = focused;
  if (focused == M3_TRUE) {
    m3_text_field_reset_cursor_blink(field);
  }

  rc = m3_text_field_sync_label(field);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_text_field_step(M3TextField *field, M3Scalar dt,
                               M3Bool *out_changed) {
  M3Scalar value;
  M3Bool finished;
  M3Bool changed;
  M3Bool previous;
  int rc;

  if (field == NULL || out_changed == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (dt < 0.0f) {
    return M3_ERR_RANGE;
  }

  changed = M3_FALSE;

  if (field->label_anim.mode != M3_ANIM_MODE_NONE) {
    rc = m3_anim_controller_step(&field->label_anim, dt, &value, &finished);
    if (rc != M3_OK) {
      return rc;
    }
    if (value != field->label_value) {
      field->label_value = value;
      changed = M3_TRUE;
    }
  }

  if (field->focused == M3_TRUE && field->style.cursor_blink_period > 0.0f) {
    field->cursor_timer += dt;
    while (field->cursor_timer >= field->style.cursor_blink_period) {
      field->cursor_timer -= field->style.cursor_blink_period;
    }
    previous = field->cursor_visible;
    field->cursor_visible =
        (field->cursor_timer < field->style.cursor_blink_period * 0.5f)
            ? M3_TRUE
            : M3_FALSE;
    if (previous != field->cursor_visible) {
      changed = M3_TRUE;
    }
  } else if (field->focused == M3_TRUE) {
    if (field->cursor_visible == M3_FALSE) {
      field->cursor_visible = M3_TRUE;
      changed = M3_TRUE;
    }
  } else {
    if (field->cursor_visible == M3_TRUE) {
      field->cursor_visible = M3_FALSE;
      changed = M3_TRUE;
    }
  }

  *out_changed = changed;
  return M3_OK;
}

int M3_CALL m3_text_field_set_on_change(M3TextField *field,
                                        M3TextFieldOnChange on_change,
                                        void *ctx) {
  if (field == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  field->on_change = on_change;
  field->on_change_ctx = ctx;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_text_field_test_validate_color(const M3Color *color) {
  return m3_text_field_validate_color(color);
}

int M3_CALL m3_text_field_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                         M3Scalar b, M3Scalar a) {
  return m3_text_field_color_set(color, r, g, b, a);
}

int M3_CALL m3_text_field_test_validate_text_style(const M3TextStyle *style,
                                                   M3Bool require_family) {
  return m3_text_field_validate_text_style(style, require_family);
}

int M3_CALL m3_text_field_test_validate_style(const M3TextFieldStyle *style,
                                              M3Bool require_text_family,
                                              M3Bool require_label_family) {
  return m3_text_field_validate_style(style, require_text_family,
                                      require_label_family);
}

int M3_CALL m3_text_field_test_validate_backend(const M3TextBackend *backend) {
  return m3_text_field_validate_backend(backend);
}

int M3_CALL m3_text_field_test_validate_measure_spec(M3MeasureSpec spec) {
  return m3_text_field_validate_measure_spec(spec);
}

int M3_CALL m3_text_field_test_validate_rect(const M3Rect *rect) {
  return m3_text_field_validate_rect(rect);
}

int M3_CALL m3_text_field_test_usize_max(m3_usize *out_value) {
  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_value = m3_text_field_usize_max();
  return M3_OK;
}

int M3_CALL m3_text_field_test_add_overflow(m3_usize a, m3_usize b,
                                            m3_usize *out_value) {
  return m3_text_field_add_overflow(a, b, out_value);
}

int M3_CALL m3_text_field_test_reserve(M3TextField *field, m3_usize required) {
  return m3_text_field_reserve(field, required);
}

int M3_CALL m3_text_field_test_validate_utf8(const char *utf8,
                                             m3_usize utf8_len) {
  return m3_text_field_validate_utf8(utf8, utf8_len);
}

int M3_CALL m3_text_field_test_validate_offset(const char *utf8,
                                               m3_usize utf8_len,
                                               m3_usize offset) {
  return m3_text_field_validate_offset(utf8, utf8_len, offset);
}

int M3_CALL m3_text_field_test_prev_offset(const char *utf8, m3_usize utf8_len,
                                           m3_usize offset,
                                           m3_usize *out_offset) {
  return m3_text_field_prev_offset(utf8, utf8_len, offset, out_offset);
}

int M3_CALL m3_text_field_test_next_offset(const char *utf8, m3_usize utf8_len,
                                           m3_usize offset,
                                           m3_usize *out_offset) {
  return m3_text_field_next_offset(utf8, utf8_len, offset, out_offset);
}

int M3_CALL m3_text_field_test_update_text_metrics(M3TextField *field) {
  return m3_text_field_update_text_metrics(field);
}

int M3_CALL m3_text_field_test_update_label_metrics(M3TextField *field) {
  return m3_text_field_update_label_metrics(field);
}

int M3_CALL m3_text_field_test_update_placeholder_metrics(M3TextField *field) {
  return m3_text_field_update_placeholder_metrics(field);
}

int M3_CALL m3_text_field_test_update_font_metrics(M3TextField *field) {
  return m3_text_field_update_font_metrics(field);
}

int M3_CALL m3_text_field_test_sync_label(M3TextField *field) {
  return m3_text_field_sync_label(field);
}

int M3_CALL m3_text_field_test_reset_cursor_blink(M3TextField *field) {
  m3_text_field_reset_cursor_blink(field);
  return M3_OK;
}

int M3_CALL m3_text_field_test_set_text_internal(M3TextField *field,
                                                 const char *utf8_text,
                                                 m3_usize utf8_len,
                                                 M3Bool notify) {
  return m3_text_field_set_text_internal(field, utf8_text, utf8_len, notify);
}

int M3_CALL m3_text_field_test_delete_range(M3TextField *field, m3_usize start,
                                            m3_usize end, M3Bool notify) {
  return m3_text_field_delete_range(field, start, end, notify);
}

int M3_CALL m3_text_field_test_measure_prefix(M3TextField *field,
                                              m3_usize offset,
                                              M3Scalar *out_width) {
  return m3_text_field_measure_prefix(field, offset, out_width);
}

int M3_CALL m3_text_field_test_offset_for_x(M3TextField *field, M3Scalar x,
                                            m3_usize *out_offset) {
  return m3_text_field_offset_for_x(field, x, out_offset);
}

int M3_CALL m3_text_field_test_resolve_colors(
    const M3TextField *field, M3Color *out_container, M3Color *out_outline,
    M3Color *out_text, M3Color *out_label, M3Color *out_placeholder,
    M3Color *out_cursor, M3Color *out_selection, M3Color *out_handle) {
  return m3_text_field_resolve_colors(field, out_container, out_outline,
                                      out_text, out_label, out_placeholder,
                                      out_cursor, out_selection, out_handle);
}
#endif
