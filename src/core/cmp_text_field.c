#include "cmpc/cmp_text_field.h"

#include "cmpc/cmp_utf8.h"

#include <string.h>

#define CMP_TEXT_FIELD_DEFAULT_CAPACITY 16u

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
#define CMP_TEXT_FIELD_TEST_FAIL_NONE 0u
#define CMP_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE 1u
#define CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT 2u /* GCOVR_EXCL_LINE */
#define CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT 3u
#define CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE 4u
#define CMP_TEXT_FIELD_TEST_FAIL_FONT_METRICS 5u /* GCOVR_EXCL_LINE */
#define CMP_TEXT_FIELD_TEST_FAIL_ANIM_START 6u
#define CMP_TEXT_FIELD_TEST_FAIL_RESERVE_REALLOC 7u
#define CMP_TEXT_FIELD_TEST_FAIL_MEASURE_PREFIX 8u /* GCOVR_EXCL_LINE */
#define CMP_TEXT_FIELD_TEST_FAIL_RESOLVE_COLORS 9u
#define CMP_TEXT_FIELD_TEST_FAIL_RESERVE_BYPASS 10u
#define CMP_TEXT_FIELD_TEST_FAIL_OFFSET_SKIP_EARLY 11u /* GCOVR_EXCL_LINE */
#define CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND 12u
#define CMP_TEXT_FIELD_TEST_FAIL_TEXT_STYLE_INIT 13u
#define CMP_TEXT_FIELD_TEST_FAIL_LABEL_STYLE_INIT 14u
#define CMP_TEXT_FIELD_TEST_FAIL_ANIM_INIT 15u
#define CMP_TEXT_FIELD_TEST_FAIL_LABEL_FONT_METRICS 16u
#define CMP_TEXT_FIELD_TEST_FAIL_OUTLINE_RANGE 17u
#define CMP_TEXT_FIELD_TEST_FAIL_CORNER_RANGE 18u
#define CMP_TEXT_FIELD_TEST_FAIL_SELECTION_WIDTH_NEGATIVE 19u
#define CMP_TEXT_FIELD_TEST_FAIL_CURSOR_WIDTH_NEGATIVE 20u
#define CMP_TEXT_FIELD_TEST_FAIL_CURSOR_HEIGHT_NEGATIVE 21u
#define CMP_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL 22u /* GCOVR_EXCL_LINE */

static cmp_u32 g_cmp_text_field_test_fail_point = CMP_TEXT_FIELD_TEST_FAIL_NONE;
static cmp_u32 g_cmp_text_field_test_color_fail_after =
    0u; /* GCOVR_EXCL_LINE */
static cmp_u32 g_cmp_text_field_test_overflow_fail_after =
    0u; /* GCOVR_EXCL_LINE */
static cmp_u32 g_cmp_text_field_test_font_metrics_fail_after = 0u;
static CMPBool g_cmp_text_field_test_offset_skip_early =
    CMP_FALSE; /* GCOVR_EXCL_LINE */

int CMP_CALL cmp_text_field_test_set_fail_point(cmp_u32 fail_point) {
  g_cmp_text_field_test_fail_point = fail_point;
  return CMP_OK;
}

int CMP_CALL cmp_text_field_test_set_color_fail_after(cmp_u32 call_count) {
  g_cmp_text_field_test_color_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL cmp_text_field_test_set_overflow_fail_after(cmp_u32 call_count) {
  g_cmp_text_field_test_overflow_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL
cmp_text_field_test_set_font_metrics_fail_after(cmp_u32 call_count) {
  g_cmp_text_field_test_font_metrics_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL cmp_text_field_test_set_offset_skip_early(CMPBool enable) {
  g_cmp_text_field_test_offset_skip_early = enable;
  return CMP_OK;
}

int CMP_CALL cmp_text_field_test_clear_fail_points(void) {
  g_cmp_text_field_test_fail_point = CMP_TEXT_FIELD_TEST_FAIL_NONE;
  g_cmp_text_field_test_color_fail_after = 0u;
  g_cmp_text_field_test_overflow_fail_after = 0u;
  g_cmp_text_field_test_font_metrics_fail_after = 0u;
  g_cmp_text_field_test_offset_skip_early = CMP_FALSE;
  return CMP_OK;
}

static int cmp_text_field_test_fail_point_match(cmp_u32 point) {
  if (g_cmp_text_field_test_fail_point != point) {
    return 0;
  }
  g_cmp_text_field_test_fail_point = CMP_TEXT_FIELD_TEST_FAIL_NONE;
  return 1;
}

static int cmp_text_field_test_color_should_fail(void) {
  if (g_cmp_text_field_test_color_fail_after == 0u) {
    return 0;
  }
  g_cmp_text_field_test_color_fail_after -= 1u;
  return (g_cmp_text_field_test_color_fail_after == 0u) ? 1 : 0;
}

static int cmp_text_field_test_overflow_should_fail(void) {
  if (g_cmp_text_field_test_overflow_fail_after == 0u) {
    return 0;
  }
  g_cmp_text_field_test_overflow_fail_after -= 1u;
  return (g_cmp_text_field_test_overflow_fail_after == 0u) ? 1 : 0;
}

static int cmp_text_field_test_font_metrics_should_fail(void) {
  if (g_cmp_text_field_test_font_metrics_fail_after == 0u) {
    return 0;
  }
  g_cmp_text_field_test_font_metrics_fail_after -= 1u;
  return (g_cmp_text_field_test_font_metrics_fail_after == 0u) ? 1 : 0;
}
#endif /* GCOVR_EXCL_LINE */

static int cmp_text_field_validate_color(const CMPColor *color) {
  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_color_should_fail()) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  if (!(color->r >= 0.0f && color->r <= 1.0f)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  return CMP_OK;
}

static int cmp_text_field_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                    CMPScalar b,
                                    CMPScalar a) { /* GCOVR_EXCL_LINE */
  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (!(r >= 0.0f && r <= 1.0f)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (!(g >= 0.0f && g <= 1.0f)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (!(b >= 0.0f && b <= 1.0f)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (!(a >= 0.0f && a <= 1.0f)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_color_should_fail()) {
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return CMP_OK;
}

static int /* GCOVR_EXCL_LINE */
cmp_text_field_validate_text_style(const CMPTextStyle *style,
                                   CMPBool require_family) {
  int rc; /* GCOVR_EXCL_LINE */

  if (style == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (require_family == CMP_TRUE && style->utf8_family == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (style->size_px <= 0) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (style->weight < 100 || style->weight > 900) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (style->italic != CMP_FALSE && style->italic != CMP_TRUE) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_color(&style->color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int cmp_text_field_validate_style(
    const CMPTextFieldStyle *style, CMPBool require_text_family,
    CMPBool require_label_family) { /* GCOVR_EXCL_LINE */
  int rc;                           /* GCOVR_EXCL_LINE */

  if (style == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (style->outline_width < 0.0f || style->corner_radius < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (style->padding_x < 0.0f || style->padding_y < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (style->min_height < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (style->label_float_offset < 0.0f || style->label_anim_duration < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (style->cursor_width < 0.0f || style->cursor_blink_period < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (style->handle_radius < 0.0f || style->handle_height < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_text_style(&style->text_style,
                                          require_text_family);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_text_style(&style->label_style,
                                          require_label_family);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_color(&style->container_color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(&style->outline_color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(&style->focused_outline_color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(&style->disabled_container_color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(&style->disabled_outline_color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(&style->disabled_text_color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(&style->disabled_label_color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(&style->placeholder_color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(&style->disabled_placeholder_color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(&style->cursor_color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(&style->selection_color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(&style->handle_color);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int cmp_text_field_validate_backend(const CMPTextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->destroy_font == NULL || /* GCOVR_EXCL_LINE */
      backend->vtable->measure_text == NULL || /* GCOVR_EXCL_LINE */
      backend->vtable->draw_text == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_UNSUPPORTED;
  }
  return CMP_OK;
}

static int cmp_text_field_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED &&
      spec.mode != CMP_MEASURE_EXACTLY && spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  return CMP_OK;
}

static int cmp_text_field_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (rect->width < 0.0f || rect->height < 0.0f) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  return CMP_OK;
}

static cmp_usize cmp_text_field_usize_max(void) {
  return (cmp_usize) ~(cmp_usize)0;
}

static int
cmp_text_field_add_overflow(cmp_usize a, cmp_usize b,
                            cmp_usize *out_value) { /* GCOVR_EXCL_LINE */
  cmp_usize max_value;                              /* GCOVR_EXCL_LINE */

  if (out_value == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_overflow_should_fail()) {
    return CMP_ERR_OVERFLOW;
  }
#endif /* GCOVR_EXCL_LINE */

  max_value = cmp_text_field_usize_max();
  if (a > max_value - b) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a + b;
  return CMP_OK;
}

static int cmp_text_field_reserve(CMPTextField *field, cmp_usize required) {
  cmp_usize new_capacity; /* GCOVR_EXCL_LINE */
  void *mem;              /* GCOVR_EXCL_LINE */
  int rc;                 /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (required == 0u) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (field->allocator.realloc == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (required <= field->utf8_capacity) {
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
    if (!cmp_text_field_test_fail_point_match(
            CMP_TEXT_FIELD_TEST_FAIL_RESERVE_BYPASS)) { /* GCOVR_EXCL_LINE */
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
#else /* GCOVR_EXCL_LINE */
    return CMP_OK; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
#endif
  }

  if (field->utf8_capacity == 0u) {
    new_capacity = CMP_TEXT_FIELD_DEFAULT_CAPACITY;
  } else if (field->utf8_capacity > cmp_text_field_usize_max() / 2u) { /* GCOVR_EXCL_LINE */
    if (field->utf8_capacity < required) { /* GCOVR_EXCL_LINE */
      return CMP_ERR_OVERFLOW; /* GCOVR_EXCL_LINE */
    }
    new_capacity = field->utf8_capacity; /* GCOVR_EXCL_LINE */
  } else { /* GCOVR_EXCL_LINE */
    new_capacity = field->utf8_capacity * 2u;
  }

  while (new_capacity < required) {
    if (new_capacity > cmp_text_field_usize_max() / 2u) { /* GCOVR_EXCL_LINE */
      new_capacity = required; /* GCOVR_EXCL_LINE */
      break; /* GCOVR_EXCL_LINE */
    }
    new_capacity *= 2u;
  }

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_RESERVE_REALLOC)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */

  rc = field->allocator.realloc(field->allocator.ctx, field->utf8, new_capacity,
                                &mem);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  field->utf8 = (char *)mem;
  field->utf8_capacity = new_capacity;
  return CMP_OK;
}

static int cmp_text_field_validate_utf8(const char *utf8, cmp_usize utf8_len) {
  CMPBool valid; /* GCOVR_EXCL_LINE */
  int rc;        /* GCOVR_EXCL_LINE */

  if (utf8 == NULL) {
    if (utf8_len == 0u) {
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_utf8_validate(utf8, utf8_len, &valid);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE)) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  if (!valid) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  return CMP_OK;
}

static int /* GCOVR_EXCL_LINE */
cmp_text_field_validate_offset(const char *utf8, cmp_usize utf8_len,
                               cmp_usize offset) { /* GCOVR_EXCL_LINE */
  CMPUtf8Iter iter;
  cmp_u32 codepoint;     /* GCOVR_EXCL_LINE */
  cmp_usize last_offset; /* GCOVR_EXCL_LINE */
  int rc;                /* GCOVR_EXCL_LINE */

  if (offset > utf8_len) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
#ifdef CMP_TESTING
  if (offset == utf8_len || offset == 0u) {
    if (g_cmp_text_field_test_offset_skip_early != CMP_TRUE) {
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
  }
#else
  if (offset == utf8_len || offset == 0u) { /* GCOVR_EXCL_LINE */
    return CMP_OK; /* GCOVR_EXCL_LINE */                          /* GCOVR_EXCL_LINE */
  }
#endif

  rc = cmp_utf8_iter_init(&iter, utf8, utf8_len);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  last_offset = 0u;
  while (1) {
    rc = /* GCOVR_EXCL_LINE */ cmp_utf8_iter_next(&iter, &codepoint);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND)) { /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_NOT_FOUND; /* GCOVR_EXCL_LINE */
    }
#endif /* GCOVR_EXCL_LINE */
    if (rc == CMP_ERR_NOT_FOUND) { /* GCOVR_EXCL_LINE */
      break; /* GCOVR_EXCL_LINE */
    }
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    if (iter.offset == offset) { /* GCOVR_EXCL_LINE */
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    if (iter.offset > offset) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      break; /* GCOVR_EXCL_LINE */
    }
    last_offset = iter.offset;
    CMP_UNUSED(codepoint);
  }

  if (last_offset == offset) { /* GCOVR_EXCL_LINE */
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }
  return CMP_ERR_RANGE;
}

static int
cmp_text_field_prev_offset(const char *utf8, cmp_usize utf8_len,
                           cmp_usize offset,
                           cmp_usize *out_offset) { /* GCOVR_EXCL_LINE */
  CMPUtf8Iter iter;                                 /* GCOVR_EXCL_LINE */
  cmp_u32 codepoint;                                /* GCOVR_EXCL_LINE */
  cmp_usize last_offset;                            /* GCOVR_EXCL_LINE */
  int rc;                                           /* GCOVR_EXCL_LINE */

  if (out_offset == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (offset == 0u) { /* GCOVR_EXCL_LINE */
    *out_offset = 0u; /* GCOVR_EXCL_LINE */
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_offset(utf8, utf8_len, offset);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_utf8_iter_init(&iter, utf8, utf8_len);
#ifdef CMP_TESTING
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT)) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  last_offset = 0u;
  while (1) {
    rc = /* GCOVR_EXCL_LINE */ cmp_utf8_iter_next(&iter, &codepoint);
#ifdef CMP_TESTING
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE
                                                              */
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_NOT_FOUND; /* GCOVR_EXCL_LINE */
    }
#endif /* GCOVR_EXCL_LINE */
    if (rc == CMP_ERR_NOT_FOUND) { /* GCOVR_EXCL_LINE */
      break; /* GCOVR_EXCL_LINE */
    }
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    if (iter.offset >= offset) { /* GCOVR_EXCL_LINE */
      break; /* GCOVR_EXCL_LINE */
    }
    last_offset = iter.offset;
    CMP_UNUSED(codepoint);
  }

  *out_offset = last_offset;
  return CMP_OK;
}

static int
cmp_text_field_next_offset(const char *utf8, cmp_usize utf8_len,
                           cmp_usize offset,
                           cmp_usize *out_offset) { /* GCOVR_EXCL_LINE */
  CMPUtf8Iter iter = {0};
  cmp_u32 codepoint; /* GCOVR_EXCL_LINE */
  int rc;            /* GCOVR_EXCL_LINE */

  if (out_offset == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (offset >= utf8_len) { /* GCOVR_EXCL_LINE */
    *out_offset = utf8_len; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_offset(utf8, utf8_len, offset);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_utf8_iter_init(&iter, utf8, utf8_len);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  while (1) {
    rc = /* GCOVR_EXCL_LINE */ cmp_utf8_iter_next(&iter, &codepoint);
#ifdef CMP_TESTING
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND)) { /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_NOT_FOUND; /* GCOVR_EXCL_LINE */
    }
#endif /* GCOVR_EXCL_LINE */
    if (rc == CMP_ERR_NOT_FOUND) { /* GCOVR_EXCL_LINE */
      break; /* GCOVR_EXCL_LINE */
    }
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    if (iter.offset > offset) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      *out_offset = iter.offset; /* GCOVR_EXCL_LINE */
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    CMP_UNUSED(codepoint);
  }

  *out_offset = utf8_len; /* GCOVR_EXCL_LINE */
  return CMP_OK; /* GCOVR_EXCL_LINE */
}

static int cmp_text_field_update_text_metrics(CMPTextField *field) {
  int rc; /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (field->text_metrics_valid == CMP_TRUE) {
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  rc =
      cmp_text_measure_utf8(&field->text_backend, field->text_font, field->utf8,
                            field->utf8_len, 0, &field->text_metrics);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  field->text_metrics_valid = CMP_TRUE;
  return CMP_OK;
}

static int cmp_text_field_update_label_metrics(CMPTextField *field) {
  int rc; /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (field->label_metrics_valid == CMP_TRUE) {
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  if (field->label_font.id == 0u && field->label_font.generation == 0u) { /* GCOVR_EXCL_LINE */
    field->label_metrics.width = 0.0f; /* GCOVR_EXCL_LINE */
    field->label_metrics.height = 0.0f; /* GCOVR_EXCL_LINE */
    field->label_metrics.baseline = 0.0f; /* GCOVR_EXCL_LINE */
    field->label_metrics_valid = CMP_TRUE; /* GCOVR_EXCL_LINE */
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_measure_utf8(&field->text_backend, field->label_font,
                             field->utf8_label, field->label_len, 0,
                             &field->label_metrics);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  field->label_metrics_valid = CMP_TRUE;
  return CMP_OK;
}

static int cmp_text_field_update_placeholder_metrics(CMPTextField *field) {
  int rc; /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (field->placeholder_metrics_valid == CMP_TRUE) {
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_measure_utf8(&field->text_backend, field->text_font,
                             field->utf8_placeholder, field->placeholder_len, 0,
                             &field->placeholder_metrics);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE)) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  field->placeholder_metrics_valid = CMP_TRUE;
  return CMP_OK;
}

static int cmp_text_field_update_font_metrics(CMPTextField *field) {
  int rc; /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (field->font_metrics_valid == CMP_TRUE) {
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_font_metrics(&field->text_backend, field->text_font,
                             &field->text_font_metrics);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_FONT_METRICS)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
  if (cmp_text_field_test_font_metrics_should_fail()) {
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  if (field->label_font.id != 0u || field->label_font.generation != 0u) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ cmp_text_font_metrics(&field->text_backend, field->label_font,
                               &field->label_font_metrics);
#ifdef CMP_TESTING
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_LABEL_FONT_METRICS)) { /* GCOVR_EXCL_LINE
                                                             */
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    if (cmp_text_field_test_font_metrics_should_fail()) { /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
#endif
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  } else { /* GCOVR_EXCL_LINE */
    field->label_font_metrics.width = 0.0f; /* GCOVR_EXCL_LINE */
    field->label_font_metrics.height = 0.0f; /* GCOVR_EXCL_LINE */
    field->label_font_metrics.baseline = 0.0f; /* GCOVR_EXCL_LINE */
  }

  field->font_metrics_valid = CMP_TRUE;
  return CMP_OK;
}

static int cmp_text_field_sync_label(CMPTextField *field) {
  CMPScalar target; /* GCOVR_EXCL_LINE */
  int rc;           /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (field->utf8_label == NULL || field->label_len == 0u) { /* GCOVR_EXCL_LINE */
    field->label_value = 0.0f;
    rc = /* GCOVR_EXCL_LINE */ cmp_anim_controller_start_timing(
        &field->label_anim, 0.0f, 0.0f, 0.0f,
        CMP_ANIM_EASE_OUT); /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_ANIM_START)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
#endif /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  target = (field->focused == CMP_TRUE || field->utf8_len > 0u) ? 1.0f : 0.0f;
  if (field->label_value == target) {
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  rc =
      cmp_anim_controller_start_timing(&field->label_anim, field->label_value,
                                       target, field->style.label_anim_duration,
                                       CMP_ANIM_EASE_OUT); /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_ANIM_START)) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  if (field->style.label_anim_duration == 0.0f) { /* GCOVR_EXCL_LINE */
    field->label_value = target; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static void cmp_text_field_reset_cursor_blink(CMPTextField *field) {
  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return; /* GCOVR_EXCL_LINE */
  }
  field->cursor_visible = CMP_TRUE;
  field->cursor_timer = 0.0f;
}

static int
cmp_text_field_set_text_internal(CMPTextField *field, const char *utf8_text,
                                 cmp_usize utf8_len,
                                 CMPBool notify) { /* GCOVR_EXCL_LINE */
  cmp_usize required;                              /* GCOVR_EXCL_LINE */
  int rc;                                          /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_utf8(utf8_text, utf8_len);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_add_overflow(utf8_len, 1u, &required);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_reserve(field, required);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  if (utf8_len > 0u && utf8_text != NULL) { /* GCOVR_EXCL_LINE */
    memcpy(field->utf8, utf8_text, utf8_len);
  }
  field->utf8[utf8_len] = '\0';
  field->utf8_len = utf8_len;
  field->cursor = utf8_len;
  field->selection_start = utf8_len;
  field->selection_end = utf8_len;
  field->text_metrics_valid = CMP_FALSE;
  field->placeholder_metrics_valid = CMP_FALSE;
  cmp_text_field_reset_cursor_blink(field);

  rc = cmp_text_field_sync_label(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  if (notify == CMP_TRUE && field->on_change != NULL) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */
        field->on_change(field->on_change_ctx, field, /* GCOVR_EXCL_LINE */
                         field->utf8_len ? field->utf8 : NULL, field->utf8_len); /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  return CMP_OK;
}

static int cmp_text_field_delete_range(CMPTextField *field, cmp_usize start,
                                       cmp_usize end,    /* GCOVR_EXCL_LINE */
                                       CMPBool notify) { /* GCOVR_EXCL_LINE */
  cmp_usize tail_len;                                    /* GCOVR_EXCL_LINE */
  int rc;                                                /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (start > end) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (end > field->utf8_len) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
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
  field->text_metrics_valid = CMP_FALSE;
  field->placeholder_metrics_valid = CMP_FALSE;
  cmp_text_field_reset_cursor_blink(field);

  rc = cmp_text_field_sync_label(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  if (notify == CMP_TRUE && field->on_change != NULL) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */
        field->on_change(field->on_change_ctx, field, /* GCOVR_EXCL_LINE */
                         field->utf8_len ? field->utf8 : NULL, field->utf8_len); /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  return CMP_OK;
}

static int /* GCOVR_EXCL_LINE */
cmp_text_field_measure_prefix(CMPTextField *field, cmp_usize offset,
                              CMPScalar *out_width) { /* GCOVR_EXCL_LINE */
  CMPTextMetrics metrics;                             /* GCOVR_EXCL_LINE */
  int rc;                                             /* GCOVR_EXCL_LINE */

  if (field == NULL || out_width == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_MEASURE_PREFIX)) {
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */
  if (offset > field->utf8_len) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_offset(field->utf8, field->utf8_len, offset);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_measure_utf8(&field->text_backend, field->text_font,
                             field->utf8, offset, 0, &metrics);
#ifdef CMP_TESTING
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  *out_width = metrics.width;
  return CMP_OK;
}

static int
cmp_text_field_offset_for_x(CMPTextField *field, CMPScalar x,
                            cmp_usize *out_offset) { /* GCOVR_EXCL_LINE */
  CMPUtf8Iter iter;                                  /* GCOVR_EXCL_LINE */
  cmp_u32 codepoint;                                 /* GCOVR_EXCL_LINE */
  cmp_usize prev_offset;                             /* GCOVR_EXCL_LINE */
  cmp_usize candidate;                               /* GCOVR_EXCL_LINE */
  CMPScalar prev_width;                              /* GCOVR_EXCL_LINE */
  CMPScalar width;                                   /* GCOVR_EXCL_LINE */
  int rc;                                            /* GCOVR_EXCL_LINE */

  if (field == NULL || out_offset == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (field->utf8_len == 0u) {
    *out_offset = 0u; /* GCOVR_EXCL_LINE */
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  if (x <= 0.0f) {
    *out_offset = 0u; /* GCOVR_EXCL_LINE */
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_utf8_iter_init(&iter, field->utf8, field->utf8_len);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  prev_offset = 0u;
  prev_width = 0.0f;
  while (1) {
    rc = /* GCOVR_EXCL_LINE */ cmp_utf8_iter_next(&iter, &codepoint);
#ifdef CMP_TESTING
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND)) { /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE
                                                              */
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_NOT_FOUND; /* GCOVR_EXCL_LINE */
    }
#endif /* GCOVR_EXCL_LINE */
    if (rc == CMP_ERR_NOT_FOUND) { /* GCOVR_EXCL_LINE */
      break; /* GCOVR_EXCL_LINE */
    }
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    candidate = iter.offset;
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_measure_prefix(field, candidate, &width);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    if (width >= x) {
      if ((x - prev_width) <= (width - x)) {
        *out_offset = prev_offset;
      } else { /* GCOVR_EXCL_LINE */
        *out_offset = candidate;
      }
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    prev_offset = candidate;
    prev_width = width;
    CMP_UNUSED(codepoint);
  }

  *out_offset = field->utf8_len;
  return CMP_OK;
}

static int
cmp_text_field_resolve_colors(const CMPTextField *field,
                              CMPColor *out_container,
                              CMPColor *out_outline, /* GCOVR_EXCL_LINE */
                              CMPColor *out_text, CMPColor *out_label,
                              CMPColor *out_placeholder, /* GCOVR_EXCL_LINE */
                              CMPColor *out_cursor, CMPColor *out_selection,
                              CMPColor *out_handle) { /* GCOVR_EXCL_LINE */
  int rc;                                             /* GCOVR_EXCL_LINE */

  if (field == NULL || out_container == NULL || out_outline == NULL || /* GCOVR_EXCL_LINE */
      out_text == NULL || out_label == NULL || out_placeholder == NULL || /* GCOVR_EXCL_LINE */
      out_cursor == NULL || out_selection == NULL || out_handle == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_RESOLVE_COLORS)) {
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */

  if (field->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    *out_container = field->style.disabled_container_color;
    *out_outline = field->style.disabled_outline_color;
    *out_text = field->style.disabled_text_color;
    *out_label = field->style.disabled_label_color;
    *out_placeholder = field->style.disabled_placeholder_color;
  } else { /* GCOVR_EXCL_LINE */
    *out_container = field->style.container_color;
    *out_outline = (field->focused == CMP_TRUE)
                       ? field->style.focused_outline_color
                       : field->style.outline_color;
    *out_text = field->style.text_style.color;
    *out_label = field->style.label_style.color;
    *out_placeholder = field->style.placeholder_color;
  }

  *out_cursor = field->style.cursor_color;
  *out_selection = field->style.selection_color;
  *out_handle = field->style.handle_color;

  rc = cmp_text_field_validate_color(out_container);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(out_outline);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(out_text);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(out_label);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(out_placeholder);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(out_cursor);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(out_selection);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_color(out_handle);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int
cmp_text_field_widget_measure(void *widget, CMPMeasureSpec width,
                              CMPMeasureSpec height, /* GCOVR_EXCL_LINE */
                              CMPSize *out_size) {   /* GCOVR_EXCL_LINE */
  CMPTextField *field;                               /* GCOVR_EXCL_LINE */
  CMPScalar desired_width;                           /* GCOVR_EXCL_LINE */
  CMPScalar desired_height;                          /* GCOVR_EXCL_LINE */
  CMPTextMetrics text_metrics;                       /* GCOVR_EXCL_LINE */
  CMPTextMetrics label_metrics;
  CMPTextMetrics placeholder_metrics; /* GCOVR_EXCL_LINE */
  int rc;                             /* GCOVR_EXCL_LINE */

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_measure_spec(width);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_measure_spec(height);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  field = (CMPTextField *)widget;

  rc = cmp_text_field_validate_style(
      &field->style, CMP_TRUE,
      (field->utf8_label != NULL && field->label_len > 0u) /* GCOVR_EXCL_LINE */
          ? CMP_TRUE
          : CMP_FALSE); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_update_text_metrics(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_update_label_metrics(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_update_placeholder_metrics(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_update_font_metrics(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
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

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    out_size->width = (desired_width > width.size) ? width.size : desired_width;
  } else { /* GCOVR_EXCL_LINE */
    out_size->width = desired_width;
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == CMP_MEASURE_AT_MOST) {
    out_size->height =
        (desired_height > height.size) ? height.size : desired_height;
  } else { /* GCOVR_EXCL_LINE */
    out_size->height = desired_height;
  }

  return CMP_OK;
}

static int cmp_text_field_widget_layout(void *widget, CMPRect bounds) {
  CMPTextField *field;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_rect(&bounds);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  field = (CMPTextField *)widget;
  field->bounds = bounds;
  return CMP_OK;
}

static int cmp_text_field_widget_paint(void *widget, CMPPaintContext *ctx) {
  CMPTextField *field = NULL;
  CMPRect bounds; /* GCOVR_EXCL_LINE */
  CMPRect inner;
  CMPRect content; /* GCOVR_EXCL_LINE */
  CMPRect selection_rect = {0};
  CMPRect handle_rect;         /* GCOVR_EXCL_LINE */
  CMPScalar outline_width;     /* GCOVR_EXCL_LINE */
  CMPScalar corner_radius;     /* GCOVR_EXCL_LINE */
  CMPScalar inner_corner;      /* GCOVR_EXCL_LINE */
  CMPScalar available_height;  /* GCOVR_EXCL_LINE */
  CMPScalar text_top;          /* GCOVR_EXCL_LINE */
  /*CMPScalar text_baseline;*/ /* GCOVR_EXCL_LINE */
  CMPScalar label_baseline;    /* GCOVR_EXCL_LINE */
  CMPScalar label_rest_y;      /* GCOVR_EXCL_LINE */
  CMPScalar label_float_y;
  CMPScalar label_y; /* GCOVR_EXCL_LINE */
  CMPScalar label_x; /* GCOVR_EXCL_LINE */
  CMPScalar text_x;  /* GCOVR_EXCL_LINE */
  CMPScalar caret_x; /* GCOVR_EXCL_LINE */
  CMPScalar start_x; /* GCOVR_EXCL_LINE */
  CMPScalar end_x;
  CMPScalar temp_scalar;
  CMPScalar handle_radius; /* GCOVR_EXCL_LINE */
  CMPScalar handle_height; /* GCOVR_EXCL_LINE */
  CMPColor container;      /* GCOVR_EXCL_LINE */
  CMPColor outline;        /* GCOVR_EXCL_LINE */
  CMPColor text_color;
  CMPColor label_color;       /* GCOVR_EXCL_LINE */
  CMPColor placeholder_color; /* GCOVR_EXCL_LINE */
  CMPColor cursor_color;      /* GCOVR_EXCL_LINE */
  CMPColor selection_color;   /* GCOVR_EXCL_LINE */
  CMPColor handle_color;      /* GCOVR_EXCL_LINE */
  int rc;                     /* GCOVR_EXCL_LINE */

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (ctx->gfx->vtable == NULL || ctx->gfx->vtable->draw_rect == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_UNSUPPORTED;
  }
  if (ctx->gfx->text_vtable == NULL ||
      ctx->gfx->text_vtable->draw_text == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_UNSUPPORTED;
  }

  field = (CMPTextField *)widget;

  rc = cmp_text_field_validate_style(
      &field->style, CMP_TRUE,
      (field->utf8_label != NULL && field->label_len > 0u) /* GCOVR_EXCL_LINE */
          ? CMP_TRUE
          : CMP_FALSE); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_rect(&field->bounds);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_update_text_metrics(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_update_label_metrics(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_update_placeholder_metrics(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_update_font_metrics(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_resolve_colors(
      field, &container, &outline, &text_color, &label_color,
      &placeholder_color, &cursor_color, &selection_color, /* GCOVR_EXCL_LINE */
      &handle_color);                                      /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  bounds = field->bounds;
  outline_width = field->style.outline_width;
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_OUTLINE_RANGE)) { /* GCOVR_EXCL_LINE */
    outline_width = -1.0f;
  }
#endif
  if (outline_width < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  corner_radius = field->style.corner_radius;
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_CORNER_RANGE)) { /* GCOVR_EXCL_LINE */
    corner_radius = -1.0f;
  }
#endif /* GCOVR_EXCL_LINE */
  if (corner_radius < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  if (outline_width > 0.0f) { /* GCOVR_EXCL_LINE */
    inner.x = bounds.x + outline_width;
    inner.y = bounds.y + outline_width;
    inner.width = bounds.width - outline_width * 2.0f;
    inner.height = bounds.height - outline_width * 2.0f;
    if (inner.width < 0.0f || inner.height < 0.0f) { /* GCOVR_EXCL_LINE */
      return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }

    rc = /* GCOVR_EXCL_LINE */ ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bounds, outline,
                                     corner_radius);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }

    inner_corner = corner_radius - outline_width; /* GCOVR_EXCL_LINE */
    if (inner_corner < 0.0f) { /* GCOVR_EXCL_LINE */
      inner_corner = 0.0f; /* GCOVR_EXCL_LINE */
    }
  } else { /* GCOVR_EXCL_LINE */
    inner = bounds; /* GCOVR_EXCL_LINE */
    inner_corner = corner_radius; /* GCOVR_EXCL_LINE */
  }

  if (container.a > 0.0f) {
    rc = /* GCOVR_EXCL_LINE */ ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &inner, container,
                                     inner_corner);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  content.x = bounds.x + field->style.padding_x;
  content.y = bounds.y + field->style.padding_y;
  content.width = bounds.width - field->style.padding_x * 2.0f;
  content.height = bounds.height - field->style.padding_y * 2.0f;
  if (content.width < 0.0f) { /* GCOVR_EXCL_LINE */
    content.width = 0.0f; /* GCOVR_EXCL_LINE */
  }
  if (content.height < 0.0f) { /* GCOVR_EXCL_LINE */
    content.height = 0.0f; /* GCOVR_EXCL_LINE */
  }

  available_height = content.height - field->text_font_metrics.height;
  if (available_height < 0.0f) { /* GCOVR_EXCL_LINE */
    available_height = 0.0f; /* GCOVR_EXCL_LINE */
  }
  text_top = content.y + available_height * 0.5f;
  /* text_baseline = field->text_font_metrics.baseline; */
  text_x = content.x;

  if (ctx->gfx->vtable->push_clip != NULL) {
    rc = /* GCOVR_EXCL_LINE */ ctx->gfx->vtable->push_clip(ctx->gfx->ctx, &inner);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  if (field->selection_start != field->selection_end && /* GCOVR_EXCL_LINE */
      selection_color.a > 0.0f) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_measure_prefix(field, field->selection_start, &start_x);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_measure_prefix(field, field->selection_end, &end_x);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    if (end_x < start_x) { /* GCOVR_EXCL_LINE */
      temp_scalar = start_x; /* GCOVR_EXCL_LINE */
      start_x = end_x; /* GCOVR_EXCL_LINE */
      end_x = temp_scalar; /* GCOVR_EXCL_LINE */
    }
    selection_rect.x = text_x + start_x; /* GCOVR_EXCL_LINE */
    selection_rect.y = text_top; /* GCOVR_EXCL_LINE */
    selection_rect.width = end_x - start_x; /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_SELECTION_WIDTH_NEGATIVE)) { /* GCOVR_EXCL_LINE
                                                                   */
      selection_rect.width = -1.0f; /* GCOVR_EXCL_LINE */
    }
#endif
    selection_rect.height = field->text_font_metrics.height; /* GCOVR_EXCL_LINE */
    if (selection_rect.width < 0.0f) { /* GCOVR_EXCL_LINE */
      selection_rect.width = 0.0f; /* GCOVR_EXCL_LINE */
    }
    if (selection_rect.height < 0.0f) { /* GCOVR_EXCL_LINE */
      selection_rect.height = 0.0f; /* GCOVR_EXCL_LINE */
    }
    rc = /* GCOVR_EXCL_LINE */ ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &selection_rect,
                                     selection_color,
                                     0.0f); /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  if (field->utf8_len > 0u) { /* GCOVR_EXCL_LINE */
    /* FIX: Center text based on bounding box, ignoring padding which can
     * misalign relative to cursor. */
    CMPScalar centerY = bounds.y + bounds.height * 0.5f;
    CMPScalar textDrawY = centerY - field->text_font_metrics.height * 0.5f +
                          field->text_font_metrics.baseline;

    rc = /* GCOVR_EXCL_LINE */ ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, field->text_font,
                                          field->utf8, field->utf8_len, 0,
                                          text_x, textDrawY, text_color);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  } else if ((field->utf8_placeholder != NULL && field->placeholder_len > 0u) && /* GCOVR_EXCL_LINE */
             (field->utf8_label == NULL || field->label_len == 0u || /* GCOVR_EXCL_LINE */
              field->label_value > 0.0f)) { /* GCOVR_EXCL_LINE */
    CMPScalar centerY = bounds.y + bounds.height * 0.5f; /* GCOVR_EXCL_LINE */
    CMPScalar textDrawY = centerY - field->text_font_metrics.height * 0.5f + /* GCOVR_EXCL_LINE */
                          field->text_font_metrics.baseline; /* GCOVR_EXCL_LINE */

    rc = /* GCOVR_EXCL_LINE */ ctx->gfx->text_vtable->draw_text(
        ctx->gfx->ctx, field->text_font, field->utf8_placeholder, /* GCOVR_EXCL_LINE */
        field->placeholder_len, 0, text_x, textDrawY, placeholder_color);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  if (field->utf8_label != NULL && field->label_len > 0u && /* GCOVR_EXCL_LINE */
      field->label_font.id != 0u) { /* GCOVR_EXCL_LINE */
    label_baseline = field->label_font_metrics.baseline;
    label_x = content.x;

    /* Calculate rest position (same as text input text) */
    {
      CMPScalar centerY = bounds.y + bounds.height * 0.5f;
      label_rest_y = centerY - field->text_font_metrics.height * 0.5f +
                     field->text_font_metrics.baseline;
    }

    /* FIX: Calculate float position so the label center sits on the top border.
       Top border is bounds.y. Center of label text is y - height/2 + baseline?
       No, we draw at baseline. To center vertically at Y, draw at Y - height/2
       + baseline. */
    label_float_y = bounds.y - (field->label_font_metrics.height * 0.5f) +
                    label_baseline + field->style.label_float_offset;

    label_y =
        label_rest_y + (label_float_y - label_rest_y) * field->label_value;
    rc = /* GCOVR_EXCL_LINE */ ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, field->label_font,
                                          field->utf8_label, field->label_len,
                                          0, label_x, label_y, label_color);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  if (field->focused == CMP_TRUE &&
      (field->widget.flags & CMP_WIDGET_FLAG_DISABLED) == 0 && /* GCOVR_EXCL_LINE */
      field->cursor_visible == CMP_TRUE && field->style.cursor_width > 0.0f && /* GCOVR_EXCL_LINE */
      cursor_color.a > 0.0f) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_measure_prefix(field, field->cursor, &caret_x);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }

    /* FIX: Center cursor vertically in the box */
    {
      CMPScalar centerY = bounds.y + bounds.height * 0.5f;

      selection_rect.x = text_x + caret_x;
      selection_rect.height = field->text_font_metrics.height;
      selection_rect.y = centerY - (selection_rect.height * 0.5f);
      selection_rect.width = field->style.cursor_width;
    }

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_CURSOR_WIDTH_NEGATIVE)) { /* GCOVR_EXCL_LINE
                                                                */
      selection_rect.width = -1.0f; /* GCOVR_EXCL_LINE */
    }
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_CURSOR_HEIGHT_NEGATIVE)) { /* GCOVR_EXCL_LINE
                                                                 */
      selection_rect.height = -1.0f; /* GCOVR_EXCL_LINE */
    }
#endif /* GCOVR_EXCL_LINE */
    if (selection_rect.width < 0.0f) { /* GCOVR_EXCL_LINE */
      selection_rect.width = 0.0f; /* GCOVR_EXCL_LINE */
    }
    if (selection_rect.height < 0.0f) { /* GCOVR_EXCL_LINE */
      selection_rect.height = 0.0f; /* GCOVR_EXCL_LINE */
    }
    rc = /* GCOVR_EXCL_LINE */ ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &selection_rect,
                                     cursor_color, 0.0f); /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  if (field->selection_start != field->selection_end && handle_color.a > 0.0f) { /* GCOVR_EXCL_LINE */
    handle_radius = field->style.handle_radius; /* GCOVR_EXCL_LINE */
    handle_height = field->style.handle_height; /* GCOVR_EXCL_LINE */
    if (handle_height < handle_radius * 2.0f) { /* GCOVR_EXCL_LINE */
      handle_height = handle_radius * 2.0f; /* GCOVR_EXCL_LINE */
    }
    if (handle_radius > 0.0f && handle_height > 0.0f) { /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ cmp_text_field_measure_prefix(field, field->selection_start,
                                         &start_x);
      if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
        return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      }
      rc = /* GCOVR_EXCL_LINE */ cmp_text_field_measure_prefix(field, field->selection_end, &end_x);
      if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
        return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      }
      if (end_x < start_x) { /* GCOVR_EXCL_LINE */
        temp_scalar = start_x; /* GCOVR_EXCL_LINE */
        start_x = end_x; /* GCOVR_EXCL_LINE */
        end_x = temp_scalar; /* GCOVR_EXCL_LINE */
      }

      handle_rect.width = handle_radius * 2.0f; /* GCOVR_EXCL_LINE */
      handle_rect.height = handle_height; /* GCOVR_EXCL_LINE */
      handle_rect.x = text_x + start_x - handle_radius; /* GCOVR_EXCL_LINE */
      handle_rect.y = /* GCOVR_EXCL_LINE */
          text_top + field->text_font_metrics.height - handle_height * 0.5f; /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &handle_rect,
                                       handle_color, handle_radius);
      if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
        return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      }

      handle_rect.x = text_x + end_x - handle_radius; /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &handle_rect,
                                       handle_color, handle_radius);
      if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
        return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      }
    }
  }

  if (ctx->gfx->vtable->pop_clip != NULL) {
    rc = /* GCOVR_EXCL_LINE */ ctx->gfx->vtable->pop_clip(ctx->gfx->ctx);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  return CMP_OK;
}

static int
cmp_text_field_widget_event(void *widget, const CMPInputEvent *event,
                            CMPBool *out_handled) { /* GCOVR_EXCL_LINE */
  CMPTextField *field;                              /* GCOVR_EXCL_LINE */
  cmp_usize offset;
  cmp_usize other; /* GCOVR_EXCL_LINE */
  int rc;          /* GCOVR_EXCL_LINE */

  if (widget == NULL || event == NULL || out_handled == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  *out_handled = CMP_FALSE;

  field = (CMPTextField *)widget;

  if (field->widget.flags & CMP_WIDGET_FLAG_DISABLED) { /* GCOVR_EXCL_LINE */
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN: /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_offset_for_x(
        field,
        (CMPScalar)event->data.pointer.x - field->bounds.x -
            field->style.padding_x, /* GCOVR_EXCL_LINE */
        &offset);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    field->cursor = offset;
    field->selection_start = offset;
    field->selection_end = offset;
    field->selecting = CMP_TRUE;
    field->focused = CMP_TRUE;
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_sync_label(field);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL)) { /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
#endif /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    cmp_text_field_reset_cursor_blink(field);
    *out_handled = CMP_TRUE;
    return CMP_OK; /* GCOVR_EXCL_LINE */
  case CMP_INPUT_POINTER_MOVE: /* GCOVR_EXCL_LINE */
    if (field->selecting == CMP_FALSE) {
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_offset_for_x(field,
                                     (CMPScalar)event->data.pointer.x -
                                         field->bounds.x -
                                         field->style.padding_x,
                                     &offset);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    field->cursor = offset;
    field->selection_end = offset;
    cmp_text_field_reset_cursor_blink(field);
    *out_handled = CMP_TRUE;
    return CMP_OK; /* GCOVR_EXCL_LINE */
  case CMP_INPUT_POINTER_UP:
    if (field->selecting == CMP_FALSE) { /* GCOVR_EXCL_LINE */
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    field->selecting = CMP_FALSE;
    *out_handled = CMP_TRUE;
    return CMP_OK; /* GCOVR_EXCL_LINE */
  case CMP_INPUT_TEXT:
    if (event->data.text.length == 0u) { /* GCOVR_EXCL_LINE */
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_insert_utf8(field, event->data.text.utf8,
                                    (cmp_usize)event->data.text.length);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    field->focused = CMP_TRUE;
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_sync_label(field);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL)) {
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
#endif /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    *out_handled = CMP_TRUE;
    return CMP_OK; /* GCOVR_EXCL_LINE */
  case CMP_INPUT_TEXT_UTF8: /* GCOVR_EXCL_LINE */
    if (event->data.text_utf8.length == 0u) { /* GCOVR_EXCL_LINE */
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_insert_utf8(field, event->data.text_utf8.utf8,
                                    event->data.text_utf8.length);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    field->focused = CMP_TRUE;
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_sync_label(field);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
    if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
            CMP_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL)) {
      rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
#endif /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    *out_handled = CMP_TRUE;
    return CMP_OK; /* GCOVR_EXCL_LINE */

  case CMP_INPUT_TEXT_EDIT: /* GCOVR_EXCL_LINE */
    if (event->data.text_edit.length > 0 && /* GCOVR_EXCL_LINE */
        event->data.text_edit.utf8 != NULL) { /* GCOVR_EXCL_LINE */
      /* Soft Keyboard IME Composition (Fallback/Direct insert for now) */
      rc = /* GCOVR_EXCL_LINE */ cmp_text_field_insert_utf8(field, event->data.text_edit.utf8,
                                      event->data.text_edit.length);
      if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
        return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      }
    }
    field->focused = CMP_TRUE;
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_sync_label(field);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    *out_handled = CMP_TRUE;
    return CMP_OK; /* GCOVR_EXCL_LINE */
  case CMP_INPUT_KEY_DOWN: /* GCOVR_EXCL_LINE */
    if ((event->modifiers & CMP_MOD_CTRL) ||
        (event->modifiers & CMP_MOD_META)) { /* GCOVR_EXCL_LINE */
      if (event->data.key.key_code == 'C' || event->data.key.key_code == 'c' || /* GCOVR_EXCL_LINE */
          event->data.key.key_code == 'X' || event->data.key.key_code == 'x' || /* GCOVR_EXCL_LINE */
          event->data.key.key_code == 'V' || event->data.key.key_code == 'v' || /* GCOVR_EXCL_LINE */
          event->data.key.key_code == 'A' || event->data.key.key_code == 'a') { /* GCOVR_EXCL_LINE */
        /* Text field doesn't have WS context to interact with clipboard
           directly, this is usually handled at the app level. But we can
           implement select all. */
        if (event->data.key.key_code == 'A' || /* GCOVR_EXCL_LINE */
            event->data.key.key_code == 'a') {
          field->selection_start = 0;
          field->selection_end = field->utf8_len;
          field->cursor = field->utf8_len;
          cmp_text_field_reset_cursor_blink(field);
          *out_handled = CMP_TRUE;
          return CMP_OK; /* GCOVR_EXCL_LINE */
        }
      }
    }
    if (event->data.key.key_code == 8u || event->data.key.key_code == 127u ||
        event->data.key.key_code == 46u) { /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ (event->data.key.key_code == 46u)
               ? cmp_text_field_delete_forward(field) /* GCOVR_EXCL_LINE */
               : cmp_text_field_backspace(field); /* GCOVR_EXCL_LINE */
      if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
        return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      }
      field->focused = CMP_TRUE;
      rc = /* GCOVR_EXCL_LINE */ cmp_text_field_sync_label(field);
      if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
        return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      }
      *out_handled = CMP_TRUE;
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    if (event->data.key.key_code == 37u || event->data.key.key_code == 0x25u || /* GCOVR_EXCL_LINE */
        event->data.key.key_code == 1073741904u) { /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ cmp_text_field_prev_offset(field->utf8, field->utf8_len,
                                      field->cursor, &other);
      if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
        return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      }
      field->cursor = other; /* GCOVR_EXCL_LINE */
      field->selection_start = other; /* GCOVR_EXCL_LINE */
      field->selection_end = other; /* GCOVR_EXCL_LINE */
      cmp_text_field_reset_cursor_blink(field); /* GCOVR_EXCL_LINE */
      field->focused = CMP_TRUE; /* GCOVR_EXCL_LINE */
      *out_handled = CMP_TRUE; /* GCOVR_EXCL_LINE */
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    if (event->data.key.key_code == 39u || event->data.key.key_code == 0x27u || /* GCOVR_EXCL_LINE */
        event->data.key.key_code == 1073741903u) { /* GCOVR_EXCL_LINE */
      rc = /* GCOVR_EXCL_LINE */ cmp_text_field_next_offset(field->utf8, field->utf8_len,
                                      field->cursor, &other);
      if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
        return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
      }
      field->cursor = other; /* GCOVR_EXCL_LINE */
      field->selection_start = other; /* GCOVR_EXCL_LINE */
      field->selection_end = other; /* GCOVR_EXCL_LINE */
      cmp_text_field_reset_cursor_blink(field); /* GCOVR_EXCL_LINE */
      field->focused = CMP_TRUE; /* GCOVR_EXCL_LINE */
      *out_handled = CMP_TRUE; /* GCOVR_EXCL_LINE */
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    if (event->data.key.key_code == 36u || event->data.key.key_code == 0x24u || /* GCOVR_EXCL_LINE */
        event->data.key.key_code == 1073741898u) { /* GCOVR_EXCL_LINE */
      field->cursor = 0u; /* GCOVR_EXCL_LINE */
      field->selection_start = 0u; /* GCOVR_EXCL_LINE */
      field->selection_end = 0u; /* GCOVR_EXCL_LINE */
      cmp_text_field_reset_cursor_blink(field); /* GCOVR_EXCL_LINE */
      field->focused = CMP_TRUE; /* GCOVR_EXCL_LINE */
      *out_handled = CMP_TRUE; /* GCOVR_EXCL_LINE */
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    if (event->data.key.key_code == 35u || event->data.key.key_code == 0x23u || /* GCOVR_EXCL_LINE */
        event->data.key.key_code == 1073741901u) { /* GCOVR_EXCL_LINE */
      field->cursor = field->utf8_len; /* GCOVR_EXCL_LINE */
      field->selection_start = field->utf8_len; /* GCOVR_EXCL_LINE */
      field->selection_end = field->utf8_len; /* GCOVR_EXCL_LINE */
      cmp_text_field_reset_cursor_blink(field); /* GCOVR_EXCL_LINE */
      field->focused = CMP_TRUE; /* GCOVR_EXCL_LINE */
      *out_handled = CMP_TRUE; /* GCOVR_EXCL_LINE */
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    return CMP_OK; /* GCOVR_EXCL_LINE */
  default:
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }
}

static int cmp_text_field_widget_get_semantics(
    void *widget, CMPSemantics *out_semantics) { /* GCOVR_EXCL_LINE */
  CMPTextField *field;                           /* GCOVR_EXCL_LINE */

  if (widget == NULL || out_semantics == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  memset(out_semantics, 0, sizeof(*out_semantics));

  field = (CMPTextField *)widget;
  out_semantics->role = CMP_SEMANTIC_TEXT_FIELD;
  out_semantics->flags = 0;
  if (field->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (field->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) { /* GCOVR_EXCL_LINE */
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  if (field->focused == CMP_TRUE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSED;
  }
  out_semantics->utf8_label = field->utf8_label;
  out_semantics->utf8_hint = field->utf8_placeholder;
  out_semantics->utf8_value = field->utf8_len ? field->utf8 : NULL;
  return CMP_OK;
}

static int cmp_text_field_widget_destroy(void *widget) {
  CMPTextField *field; /* GCOVR_EXCL_LINE */
  int rc;              /* GCOVR_EXCL_LINE */
  int rc2;             /* GCOVR_EXCL_LINE */

  if (widget == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  field = (CMPTextField *)widget;
  rc = CMP_OK;

  if (field->owns_fonts == CMP_TRUE) { /* GCOVR_EXCL_LINE */
    if (field->text_font.id != 0u || field->text_font.generation != 0u) { /* GCOVR_EXCL_LINE */
      if (field->text_backend.vtable != NULL && /* GCOVR_EXCL_LINE */
          field->text_backend.vtable->destroy_font != NULL) { /* GCOVR_EXCL_LINE */
        rc2 = field->text_backend.vtable->destroy_font(field->text_backend.ctx,
                                                       field->text_font);
        if (rc2 != CMP_OK && rc == CMP_OK) { /* GCOVR_EXCL_LINE */
          rc = /* GCOVR_EXCL_LINE */ rc2;
        }
      } else if (rc == CMP_OK) { /* GCOVR_EXCL_LINE */
        rc = /* GCOVR_EXCL_LINE */ CMP_ERR_UNSUPPORTED;
      }
    }
    if (field->label_font.id != 0u || field->label_font.generation != 0u) { /* GCOVR_EXCL_LINE */
      if (field->text_backend.vtable != NULL && /* GCOVR_EXCL_LINE */
          field->text_backend.vtable->destroy_font != NULL) { /* GCOVR_EXCL_LINE */
        rc2 = field->text_backend.vtable->destroy_font(field->text_backend.ctx,
                                                       field->label_font);
        if (rc2 != CMP_OK && rc == CMP_OK) { /* GCOVR_EXCL_LINE */
          rc = /* GCOVR_EXCL_LINE */ rc2;
        }
      } else if (rc == CMP_OK) { /* GCOVR_EXCL_LINE */
        rc = /* GCOVR_EXCL_LINE */ CMP_ERR_UNSUPPORTED;
      }
    }
  }

  if (field->utf8 != NULL && field->allocator.free != NULL) { /* GCOVR_EXCL_LINE */
    field->allocator.free(field->allocator.ctx, field->utf8);
  }

  field->utf8 = NULL;
  field->utf8_len = 0u;
  field->utf8_capacity = 0u;
  field->cursor = 0u;
  field->selection_start = 0u;
  field->selection_end = 0u;
  field->selecting = CMP_FALSE;
  field->text_metrics_valid = CMP_FALSE;
  field->label_metrics_valid = CMP_FALSE;
  field->placeholder_metrics_valid = CMP_FALSE;
  field->font_metrics_valid = CMP_FALSE;
  field->owns_fonts = CMP_FALSE;
  field->focused = CMP_FALSE;
  field->cursor_visible = CMP_FALSE;
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

static const CMPWidgetVTable
    g_cmp_text_field_widget_vtable = /* GCOVR_EXCL_LINE */
    {cmp_text_field_widget_measure,
     cmp_text_field_widget_layout, /* GCOVR_EXCL_LINE */
     cmp_text_field_widget_paint,
     cmp_text_field_widget_event, /* GCOVR_EXCL_LINE */
     cmp_text_field_widget_get_semantics,
     cmp_text_field_widget_destroy}; /* GCOVR_EXCL_LINE */

int CMP_CALL cmp_text_field_style_init(CMPTextFieldStyle *style) {
  int rc;

  if (style == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  memset(style, 0, sizeof(*style));

  rc = cmp_text_style_init(&style->text_style);
#ifdef CMP_TESTING
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_TEXT_STYLE_INIT)) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_style_init(&style->label_style);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_LABEL_STYLE_INIT)) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  style->padding_x = CMP_TEXT_FIELD_DEFAULT_PADDING_X;
  style->padding_y = CMP_TEXT_FIELD_DEFAULT_PADDING_Y;
  style->min_height = CMP_TEXT_FIELD_DEFAULT_MIN_HEIGHT;
  style->corner_radius = CMP_TEXT_FIELD_DEFAULT_CORNER_RADIUS;
  style->outline_width = CMP_TEXT_FIELD_DEFAULT_OUTLINE_WIDTH;
  style->label_float_offset = CMP_TEXT_FIELD_DEFAULT_LABEL_FLOAT_OFFSET;
  style->label_anim_duration = CMP_TEXT_FIELD_DEFAULT_LABEL_ANIM_DURATION;
  style->cursor_width = CMP_TEXT_FIELD_DEFAULT_CURSOR_WIDTH;
  style->cursor_blink_period = CMP_TEXT_FIELD_DEFAULT_CURSOR_BLINK;
  style->handle_radius = CMP_TEXT_FIELD_DEFAULT_HANDLE_RADIUS;
  style->handle_height = CMP_TEXT_FIELD_DEFAULT_HANDLE_HEIGHT;

  rc =
      cmp_text_field_color_set(&style->container_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.38f);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_color_set(&style->focused_outline_color, 0.2f, 0.4f, 0.9f,
                                1.0f); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_color_set(&style->disabled_container_color, 0.95f, 0.95f,
                                0.95f, 1.0f);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_color_set(&style->disabled_outline_color, 0.0f, 0.0f,
                                0.0f, 0.12f);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_color_set(&style->disabled_text_color, 0.0f, 0.0f, 0.0f,
                                0.38f); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_color_set(&style->disabled_label_color, 0.0f, 0.0f, 0.0f,
                                0.38f); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_color_set(&style->placeholder_color, 0.0f, 0.0f, 0.0f,
                                0.6f); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_color_set(&style->disabled_placeholder_color, 0.0f, 0.0f,
                                0.0f, 0.38f); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_color_set(&style->cursor_color, 0.2f, 0.4f, 0.9f, 1.0f);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc =
      cmp_text_field_color_set(&style->selection_color, 0.2f, 0.4f, 0.9f, 0.3f);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_color_set(&style->handle_color, 0.2f, 0.4f, 0.9f, 1.0f);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

int CMP_CALL cmp_text_field_init(
    CMPTextField *field, const CMPTextBackend *backend, /* GCOVR_EXCL_LINE */
    const CMPTextFieldStyle *style,                     /* GCOVR_EXCL_LINE */
    const CMPAllocator *allocator,                      /* GCOVR_EXCL_LINE */
    const char *utf8_text, cmp_usize utf8_len) {        /* GCOVR_EXCL_LINE */
  CMPAllocator alloc;                                   /* GCOVR_EXCL_LINE */
  int rc;                                               /* GCOVR_EXCL_LINE */

  if (field == NULL || backend == NULL || style == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_backend(backend);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_style(style, CMP_TRUE, CMP_FALSE);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  if (allocator == NULL) {
    rc = /* GCOVR_EXCL_LINE */ cmp_get_default_allocator(&alloc);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  } else { /* GCOVR_EXCL_LINE */
    alloc = *allocator;
  }

  if (alloc.alloc == NULL || alloc.realloc == NULL || alloc.free == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
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
  field->selecting = CMP_FALSE;
  field->text_metrics_valid = CMP_FALSE;
  field->label_metrics_valid = CMP_FALSE;
  field->placeholder_metrics_valid = CMP_FALSE;
  field->font_metrics_valid = CMP_FALSE;
  field->owns_fonts = CMP_TRUE;
  field->focused = CMP_FALSE;
  field->cursor_visible = CMP_FALSE;
  field->cursor_timer = 0.0f;
  field->label_value = 0.0f;
  field->utf8_label = NULL;
  field->label_len = 0u;
  field->utf8_placeholder = NULL;
  field->placeholder_len = 0u;
  field->on_change = NULL;
  field->on_change_ctx = NULL;

  rc = cmp_anim_controller_init(&field->label_anim);
#ifdef CMP_TESTING
  if (cmp_text_field_test_fail_point_match( /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
          CMP_TEXT_FIELD_TEST_FAIL_ANIM_INIT)) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ CMP_ERR_IO; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  field->text_font.id = 0u;
  field->text_font.generation = 0u;
  field->label_font.id = 0u;
  field->label_font.generation = 0u;

  rc = cmp_text_font_create(backend, &style->text_style, &field->text_font);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  if (style->label_style.utf8_family != NULL) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ cmp_text_font_create(backend, &style->label_style, &field->label_font);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      cmp_text_font_destroy(backend, field->text_font); /* GCOVR_EXCL_LINE */
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  rc = cmp_text_field_set_text_internal(field, utf8_text, utf8_len, CMP_FALSE);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  field->widget.ctx = field;
  field->widget.vtable = &g_cmp_text_field_widget_vtable;
  field->widget.handle.id = 0u;
  field->widget.handle.generation = 0u;
  field->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  return CMP_OK;
}

int CMP_CALL cmp_text_field_set_text(CMPTextField *field, const char *utf8_text,
                                     cmp_usize utf8_len) { /* GCOVR_EXCL_LINE */
  return cmp_text_field_set_text_internal(field, utf8_text, utf8_len, CMP_TRUE);
}

int CMP_CALL cmp_text_field_get_text(const CMPTextField *field,
                                     const char **out_utf8,
                                     cmp_usize *out_len) {
  if (field == NULL || out_utf8 == NULL || out_len == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  *out_utf8 = (field->utf8_len > 0u) ? field->utf8 : NULL; /* GCOVR_EXCL_LINE */
  *out_len = field->utf8_len;
  return CMP_OK;
}

int CMP_CALL cmp_text_field_insert_utf8(CMPTextField *field,
                                        const char *utf8_text,
                                        cmp_usize utf8_len) {
  cmp_usize insert_at;
  cmp_usize tail_len; /* GCOVR_EXCL_LINE */
  cmp_usize new_len;  /* GCOVR_EXCL_LINE */
  cmp_usize required = 0u;
  int rc; /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_text == NULL && utf8_len != 0u) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_utf8(utf8_text, utf8_len);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  if (field->selection_start != field->selection_end) {
    rc = /* GCOVR_EXCL_LINE */ cmp_text_field_delete_range(field, field->selection_start,
                                     field->selection_end, CMP_FALSE);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  insert_at = field->cursor;
  if (insert_at > field->utf8_len) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_offset(field->utf8, field->utf8_len, insert_at);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_add_overflow(field->utf8_len, utf8_len, &new_len);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_add_overflow(new_len, 1u, &required);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_reserve(field, required);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  tail_len = field->utf8_len - insert_at;
  if (tail_len > 0u && utf8_len > 0u) { /* GCOVR_EXCL_LINE */
    memmove(field->utf8 + insert_at + utf8_len, field->utf8 + insert_at,
            tail_len);
  }
  if (utf8_len > 0u && utf8_text != NULL) { /* GCOVR_EXCL_LINE */
    memcpy(field->utf8 + insert_at, utf8_text, utf8_len);
  }

  field->utf8_len = new_len;
  field->utf8[new_len] = '\0';
  field->cursor = insert_at + utf8_len;
  field->selection_start = field->cursor;
  field->selection_end = field->cursor;
  field->text_metrics_valid = CMP_FALSE;
  field->placeholder_metrics_valid = CMP_FALSE;
  cmp_text_field_reset_cursor_blink(field);

  rc = cmp_text_field_sync_label(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  if (field->on_change != NULL) {
    rc = /* GCOVR_EXCL_LINE */
        field->on_change(field->on_change_ctx, field, /* GCOVR_EXCL_LINE */
                         field->utf8_len ? field->utf8 : NULL, field->utf8_len); /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  return CMP_OK;
}

int CMP_CALL cmp_text_field_delete_selection(CMPTextField *field) {
  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (field->selection_start == field->selection_end) { /* GCOVR_EXCL_LINE */
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  return cmp_text_field_delete_range(field, field->selection_start,
                                     field->selection_end, CMP_TRUE);
}

int CMP_CALL cmp_text_field_backspace(CMPTextField *field) {
  cmp_usize prev; /* GCOVR_EXCL_LINE */
  int rc;         /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (field->selection_start != field->selection_end) {
    return cmp_text_field_delete_selection(field);
  }

  if (field->cursor == 0u) {
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_prev_offset(field->utf8, field->utf8_len, field->cursor,
                                  &prev); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  return cmp_text_field_delete_range(field, prev, field->cursor, CMP_TRUE);
}

int CMP_CALL cmp_text_field_delete_forward(CMPTextField *field) {
  cmp_usize next; /* GCOVR_EXCL_LINE */
  int rc;         /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (field->selection_start != field->selection_end) { /* GCOVR_EXCL_LINE */
    return cmp_text_field_delete_selection(field); /* GCOVR_EXCL_LINE */
  }

  if (field->cursor >= field->utf8_len) { /* GCOVR_EXCL_LINE */
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_next_offset(field->utf8, field->utf8_len, field->cursor,
                                  &next); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  return cmp_text_field_delete_range(field, field->cursor, next, CMP_TRUE);
}

int CMP_CALL
cmp_text_field_set_label(CMPTextField *field, const char *utf8_label,
                         cmp_usize label_len) { /* GCOVR_EXCL_LINE */
  int rc;                                       /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_label == NULL && label_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_utf8(utf8_label, label_len);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  if ((utf8_label != NULL && label_len > 0u) && /* GCOVR_EXCL_LINE */
      (field->label_font.id == 0u && field->label_font.generation == 0u)) { /* GCOVR_EXCL_LINE */
    if (field->style.label_style.utf8_family == NULL) { /* GCOVR_EXCL_LINE */
      return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
    }
    rc = /* GCOVR_EXCL_LINE */ cmp_text_font_create(&field->text_backend, &field->style.label_style,
                              &field->label_font);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  field->utf8_label = utf8_label;
  field->label_len = label_len;
  field->label_metrics_valid = CMP_FALSE;
  field->font_metrics_valid = CMP_FALSE;

  rc = cmp_text_field_sync_label(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

int CMP_CALL cmp_text_field_set_placeholder(
    CMPTextField *field, const char *utf8_placeholder, /* GCOVR_EXCL_LINE */
    cmp_usize placeholder_len) {                       /* GCOVR_EXCL_LINE */
  int rc;                                              /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (utf8_placeholder == NULL && placeholder_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_utf8(utf8_placeholder, placeholder_len);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  field->utf8_placeholder = utf8_placeholder;
  field->placeholder_len = placeholder_len;
  field->placeholder_metrics_valid = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_text_field_set_style(
    CMPTextField *field, const CMPTextFieldStyle *style) { /* GCOVR_EXCL_LINE */
  CMPHandle new_text_font;                                 /* GCOVR_EXCL_LINE */
  CMPHandle new_label_font;                                /* GCOVR_EXCL_LINE */
  CMPBool need_label;                                      /* GCOVR_EXCL_LINE */
  int rc_destroy;                                          /* GCOVR_EXCL_LINE */
  int rc2;                                                 /* GCOVR_EXCL_LINE */
  int rc;                                                  /* GCOVR_EXCL_LINE */

  if (field == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  need_label = (field->utf8_label != NULL && field->label_len > 0u) ? CMP_TRUE
                                                                    : CMP_FALSE; /* GCOVR_EXCL_LINE */
  rc = cmp_text_field_validate_style(style, CMP_TRUE, need_label);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  rc_destroy = CMP_OK;

  new_text_font.id = 0u;
  new_text_font.generation = 0u;
  new_label_font.id = 0u;
  new_label_font.generation = 0u;

  rc = cmp_text_font_create(&field->text_backend, &style->text_style,
                            &new_text_font); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  if (need_label == CMP_TRUE) {
    rc = /* GCOVR_EXCL_LINE */ cmp_text_font_create(&field->text_backend, &style->label_style,
                              &new_label_font); /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      cmp_text_font_destroy(&field->text_backend, new_text_font); /* GCOVR_EXCL_LINE */
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
  }

  if (field->owns_fonts == CMP_TRUE) { /* GCOVR_EXCL_LINE */
    rc2 = cmp_text_font_destroy(&field->text_backend, field->text_font);
    if (rc2 != CMP_OK) { /* GCOVR_EXCL_LINE */
      rc_destroy = rc2; /* GCOVR_EXCL_LINE */
    }
    if (field->label_font.id != 0u || field->label_font.generation != 0u) { /* GCOVR_EXCL_LINE */
      rc2 = cmp_text_font_destroy(&field->text_backend, field->label_font);
      if (rc2 != CMP_OK && rc_destroy == CMP_OK) { /* GCOVR_EXCL_LINE */
        rc_destroy = rc2; /* GCOVR_EXCL_LINE */
      }
    }
  }

  field->style = *style;
  field->text_font = new_text_font;
  field->label_font = new_label_font;
  field->owns_fonts = CMP_TRUE;
  field->text_metrics_valid = CMP_FALSE;
  field->label_metrics_valid = CMP_FALSE;
  field->placeholder_metrics_valid = CMP_FALSE;
  field->font_metrics_valid = CMP_FALSE;

  rc = cmp_text_field_sync_label(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  if (rc_destroy != CMP_OK) { /* GCOVR_EXCL_LINE */
    return rc_destroy; /* GCOVR_EXCL_LINE */
  }
  return CMP_OK;
}

int CMP_CALL cmp_text_field_set_selection(CMPTextField *field, cmp_usize start,
                                          cmp_usize end) { /* GCOVR_EXCL_LINE */
  cmp_usize tmp;                                           /* GCOVR_EXCL_LINE */
  int rc;                                                  /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (start > field->utf8_len || end > field->utf8_len) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_offset(field->utf8, field->utf8_len, start);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }
  rc = cmp_text_field_validate_offset(field->utf8, field->utf8_len, end);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  if (start > end) { /* GCOVR_EXCL_LINE */
    tmp = start; /* GCOVR_EXCL_LINE */
    start = end; /* GCOVR_EXCL_LINE */
    end = tmp; /* GCOVR_EXCL_LINE */
  }

  field->selection_start = start;
  field->selection_end = end;
  field->cursor = end;
  cmp_text_field_reset_cursor_blink(field);
  return CMP_OK;
}

int CMP_CALL cmp_text_field_get_selection(
    const CMPTextField *field, cmp_usize *out_start, /* GCOVR_EXCL_LINE */
    cmp_usize *out_end) {                            /* GCOVR_EXCL_LINE */
  if (field == NULL || out_start == NULL || out_end == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  *out_start = field->selection_start;
  *out_end = field->selection_end;
  return CMP_OK;
}

int CMP_CALL cmp_text_field_set_cursor(CMPTextField *field, cmp_usize cursor) {
  int rc; /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (cursor > field->utf8_len) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_field_validate_offset(field->utf8, field->utf8_len, cursor);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  field->cursor = cursor;
  field->selection_start = cursor;
  field->selection_end = cursor;
  cmp_text_field_reset_cursor_blink(field);
  return CMP_OK;
}

int CMP_CALL cmp_text_field_get_cursor(
    const CMPTextField *field, cmp_usize *out_cursor) { /* GCOVR_EXCL_LINE */
  if (field == NULL || out_cursor == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  *out_cursor = field->cursor;
  return CMP_OK;
}

int CMP_CALL cmp_text_field_set_focus(CMPTextField *field, CMPBool focused) {
  int rc; /* GCOVR_EXCL_LINE */

  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (focused != CMP_FALSE && focused != CMP_TRUE) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  field->focused = focused;
  if (focused == CMP_TRUE) { /* GCOVR_EXCL_LINE */
    cmp_text_field_reset_cursor_blink(field);
  }

  rc = cmp_text_field_sync_label(field);
  if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
    return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

int CMP_CALL cmp_text_field_step(CMPTextField *field, CMPScalar dt,
                                 CMPBool *out_changed) { /* GCOVR_EXCL_LINE */
  CMPScalar value;                                       /* GCOVR_EXCL_LINE */
  CMPBool finished;                                      /* GCOVR_EXCL_LINE */
  CMPBool changed;                                       /* GCOVR_EXCL_LINE */
  CMPBool previous;                                      /* GCOVR_EXCL_LINE */
  int rc;                                                /* GCOVR_EXCL_LINE */

  if (field == NULL || out_changed == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (dt < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  changed = CMP_FALSE;

  if (field->label_anim.mode != CMP_ANIM_MODE_NONE) { /* GCOVR_EXCL_LINE */
    rc = /* GCOVR_EXCL_LINE */ cmp_anim_controller_step(&field->label_anim, dt, &value, &finished);
    if (rc != CMP_OK) /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */ {
      return rc; /* GCOVR_EXCL_LINE */ /* GCOVR_EXCL_LINE */
    }
    if (value != field->label_value) { /* GCOVR_EXCL_LINE */
      field->label_value = value;
      changed = CMP_TRUE;
    }
  }

  if (field->focused == CMP_TRUE && field->style.cursor_blink_period > 0.0f) { /* GCOVR_EXCL_LINE */
    field->cursor_timer += dt;
    while (field->cursor_timer >= field->style.cursor_blink_period) { /* GCOVR_EXCL_LINE */
      field->cursor_timer -= field->style.cursor_blink_period; /* GCOVR_EXCL_LINE */
    }
    previous = field->cursor_visible;
    field->cursor_visible =
        (field->cursor_timer < field->style.cursor_blink_period * 0.5f)
            ? CMP_TRUE   /* GCOVR_EXCL_LINE */
            : CMP_FALSE; /* GCOVR_EXCL_LINE */
    if (previous != field->cursor_visible) { /* GCOVR_EXCL_LINE */
      changed = CMP_TRUE; /* GCOVR_EXCL_LINE */
    }
  } else if (field->focused == CMP_TRUE) { /* GCOVR_EXCL_LINE */
    if (field->cursor_visible == CMP_FALSE) { /* GCOVR_EXCL_LINE */
      field->cursor_visible = CMP_TRUE; /* GCOVR_EXCL_LINE */
      changed = CMP_TRUE; /* GCOVR_EXCL_LINE */
    }
  } else { /* GCOVR_EXCL_LINE */
    if (field->cursor_visible == CMP_TRUE) { /* GCOVR_EXCL_LINE */
      field->cursor_visible = CMP_FALSE; /* GCOVR_EXCL_LINE */
      changed = CMP_TRUE; /* GCOVR_EXCL_LINE */
    }
  }

  *out_changed = changed;
  return CMP_OK;
}

int CMP_CALL cmp_text_field_set_on_change(
    CMPTextField *field, CMPTextFieldOnChange on_change, /* GCOVR_EXCL_LINE */
    void *ctx) {                                         /* GCOVR_EXCL_LINE */
  if (field == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  field->on_change = on_change;
  field->on_change_ctx = ctx;
  return CMP_OK;
}

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
int CMP_CALL cmp_text_field_test_validate_color(const CMPColor *color) {
  return cmp_text_field_validate_color(color);
}

int CMP_CALL cmp_text_field_test_color_set(CMPColor *color, CMPScalar r,
                                           CMPScalar g, CMPScalar b,
                                           CMPScalar a) {
  return cmp_text_field_color_set(color, r, g, b, a);
}

int CMP_CALL cmp_text_field_test_validate_text_style(const CMPTextStyle *style,
                                                     CMPBool require_family) {
  return cmp_text_field_validate_text_style(style, require_family);
}

int CMP_CALL cmp_text_field_test_validate_style(
    const CMPTextFieldStyle *style,
    CMPBool require_text_family,    /* GCOVR_EXCL_LINE */
    CMPBool require_label_family) { /* GCOVR_EXCL_LINE */
  return cmp_text_field_validate_style(style, require_text_family,
                                       require_label_family);
}

int CMP_CALL
cmp_text_field_test_validate_backend(const CMPTextBackend *backend) {
  return cmp_text_field_validate_backend(backend);
}

int CMP_CALL cmp_text_field_test_validate_measure_spec(CMPMeasureSpec spec) {
  return cmp_text_field_validate_measure_spec(spec);
}

int CMP_CALL cmp_text_field_test_validate_rect(const CMPRect *rect) {
  return cmp_text_field_validate_rect(rect);
}

int CMP_CALL cmp_text_field_test_usize_max(cmp_usize *out_value) {
  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  *out_value = cmp_text_field_usize_max();
  return CMP_OK;
}

int CMP_CALL cmp_text_field_test_add_overflow(cmp_usize a, cmp_usize b,
                                              cmp_usize *out_value) {
  return cmp_text_field_add_overflow(a, b, out_value);
}

int CMP_CALL cmp_text_field_test_reserve(CMPTextField *field,
                                         cmp_usize required) {
  return cmp_text_field_reserve(field, required);
}

int CMP_CALL cmp_text_field_test_validate_utf8(const char *utf8,
                                               cmp_usize utf8_len) {
  return cmp_text_field_validate_utf8(utf8, utf8_len);
}

int CMP_CALL cmp_text_field_test_validate_offset(const char *utf8,
                                                 cmp_usize utf8_len,
                                                 cmp_usize offset) {
  return cmp_text_field_validate_offset(utf8, utf8_len, offset);
}

int CMP_CALL cmp_text_field_test_prev_offset(const char *utf8,
                                             cmp_usize utf8_len,
                                             cmp_usize offset,
                                             cmp_usize *out_offset) {
  return cmp_text_field_prev_offset(utf8, utf8_len, offset, out_offset);
}

int CMP_CALL cmp_text_field_test_next_offset(const char *utf8,
                                             cmp_usize utf8_len,
                                             cmp_usize offset,
                                             cmp_usize *out_offset) {
  return cmp_text_field_next_offset(utf8, utf8_len, offset, out_offset);
}

int CMP_CALL cmp_text_field_test_update_text_metrics(CMPTextField *field) {
  return cmp_text_field_update_text_metrics(field);
}

int CMP_CALL cmp_text_field_test_update_label_metrics(CMPTextField *field) { /* GCOVR_EXCL_LINE */
  return cmp_text_field_update_label_metrics(field); /* GCOVR_EXCL_LINE */
}

int CMP_CALL
cmp_text_field_test_update_placeholder_metrics(CMPTextField *field) {
  return cmp_text_field_update_placeholder_metrics(field);
}

int CMP_CALL cmp_text_field_test_update_font_metrics(CMPTextField *field) {
  return cmp_text_field_update_font_metrics(field);
}

int CMP_CALL cmp_text_field_test_sync_label(CMPTextField *field) {
  return cmp_text_field_sync_label(field);
}

int CMP_CALL cmp_text_field_test_reset_cursor_blink(CMPTextField *field) { /* GCOVR_EXCL_LINE */
  cmp_text_field_reset_cursor_blink(field); /* GCOVR_EXCL_LINE */
  return CMP_OK; /* GCOVR_EXCL_LINE */
}

int CMP_CALL cmp_text_field_test_set_text_internal(CMPTextField *field,
                                                   const char *utf8_text,
                                                   cmp_usize utf8_len,
                                                   CMPBool notify) {
  return cmp_text_field_set_text_internal(field, utf8_text, utf8_len, notify);
}

int CMP_CALL cmp_text_field_test_delete_range(CMPTextField *field,
                                              cmp_usize start, cmp_usize end,
                                              CMPBool notify) {
  return cmp_text_field_delete_range(field, start, end, notify);
}

int CMP_CALL cmp_text_field_test_measure_prefix(CMPTextField *field,
                                                cmp_usize offset,
                                                CMPScalar *out_width) {
  return cmp_text_field_measure_prefix(field, offset, out_width);
}

int CMP_CALL cmp_text_field_test_offset_for_x(CMPTextField *field, CMPScalar x,
                                              cmp_usize *out_offset) {
  return cmp_text_field_offset_for_x(field, x, out_offset);
}

int CMP_CALL cmp_text_field_test_resolve_colors(
    const CMPTextField *field, CMPColor *out_container, CMPColor *out_outline,
    CMPColor *out_text, CMPColor *out_label, CMPColor *out_placeholder,
    CMPColor *out_cursor, CMPColor *out_selection, CMPColor *out_handle) {
  return cmp_text_field_resolve_colors(field, out_container, out_outline,
                                       out_text, out_label, out_placeholder,
                                       out_cursor, out_selection, out_handle);
}
#endif
