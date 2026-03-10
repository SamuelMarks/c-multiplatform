/* clang-format off */
#include "cmpc/cmp_text_field.h"
#include "test_utils.h"
#include <string.h>
/* clang-format on */

static int test_create_font(void *text, const char *utf8_family,
                            cmp_i32 size_px, cmp_i32 weight, CMPBool italic,
                            CMPHandle *out_font) {
  if (out_font) {
    out_font->id = 1;
    out_font->generation = 1;
  }
  return CMP_OK;
}
static int test_destroy_font(void *text, CMPHandle font) { return CMP_OK; }
static int test_measure_text(void *text, CMPHandle font, const char *utf8,
                             cmp_usize utf8_len, cmp_u32 base_direction,
                             CMPScalar *out_width, CMPScalar *out_height,
                             CMPScalar *out_baseline) {
  if (out_width)
    *out_width = (CMPScalar)utf8_len * 10.0f;
  if (out_height)
    *out_height = 20.0f;
  if (out_baseline)
    *out_baseline = 16.0f;
  return CMP_OK;
}
static int test_draw_text(void *text, CMPHandle font, const char *utf8,
                          cmp_usize utf8_len, cmp_u32 base_direction,
                          CMPScalar x, CMPScalar y, CMPColor color) {
  return CMP_OK;
}

static CMPTextVTable g_test_text_vtable = {test_create_font,
                                           test_destroy_font,
                                           test_measure_text,
                                           test_draw_text,
                                           NULL,
                                           NULL,
                                           NULL};

int main(void) {
  CMPTextField field;
  CMPTextFieldStyle style;
  CMPTextBackend text_backend = {NULL, &g_test_text_vtable};

  cmp_text_field_style_init(&style);
  cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0);

  CMP_TEST_EXPECT(cmp_text_field_test_validate_color(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMPColor color;
  cmp_text_field_test_color_set(&color, 1.0f, 1.0f, 1.0f, 1.0f);
  CMP_TEST_OK(cmp_text_field_test_validate_color(&color));

  color.r = -1.0f;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 1.0f;

  CMP_TEST_EXPECT(cmp_text_field_test_validate_text_style(NULL, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);

  CMPTextStyle tstyle;
  cmp_text_style_init(&tstyle);
  CMP_TEST_OK(cmp_text_field_test_validate_text_style(&tstyle, CMP_FALSE));

  tstyle.size_px = 0;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_text_style(&tstyle, CMP_FALSE),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(NULL, CMP_FALSE, CMP_FALSE),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE));

  CMP_TEST_EXPECT(cmp_text_field_test_validate_rect(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMPRect rect = {0, 0, 10, 10};
  CMP_TEST_OK(cmp_text_field_test_validate_rect(&rect));

  rect.width = -1.0f;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_rect(&rect), CMP_ERR_RANGE);

  cmp_usize val;
  CMP_TEST_EXPECT(cmp_text_field_test_usize_max(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_usize_max(&val));

  CMP_TEST_EXPECT(cmp_text_field_test_add_overflow(val, 1, &val),
                  CMP_ERR_OVERFLOW);

  CMP_TEST_EXPECT(cmp_text_field_test_reserve(NULL, 10),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_reserve(&field, 10));

  CMP_TEST_EXPECT(cmp_text_field_test_validate_utf8(NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_validate_utf8("a", 1));

  cmp_text_field_test_set_offset_skip_early(CMP_TRUE);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset(NULL, 1, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  cmp_text_field_test_set_offset_skip_early(CMP_FALSE);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset("a", 1, 2),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_text_field_test_validate_offset("a", 1, 0));
  CMP_TEST_OK(cmp_text_field_test_validate_offset("a", 1, 1));

  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset("a", 1, 1), CMP_OK);

  CMP_TEST_EXPECT(cmp_text_field_test_prev_offset(NULL, 1, 1, &val),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_next_offset(NULL, 1, 0, &val),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_text_field_test_sync_label(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_sync_label(&field));

  cmp_text_field_test_set_fail_point(12u);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset("ab", 2, 1),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(
      cmp_text_field_test_set_text_internal(NULL, "a", 1, CMP_FALSE),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_text_field_test_delete_range(NULL, 0, 1, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);

  cmp_text_field_test_set_fail_point(17u);
  CMPPaintContext pctx;
  memset(&pctx, 0, sizeof(pctx));
  CMPGfx pgfx;
  CMPGfxVTable pgfx_vt;
  memset(&pgfx, 0, sizeof(pgfx));
  memset(&pgfx_vt, 0, sizeof(pgfx_vt));
  pgfx_vt.draw_rect = (void *)1;
  pgfx.vtable = &pgfx_vt;
  pgfx.text_vtable = &g_test_text_vtable;
  pctx.gfx = &pgfx;
  CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_RANGE);

  cmp_text_field_test_set_fail_point(18u);
  CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_RANGE);
  cmp_text_field_test_set_fail_point(13u);
  CMP_TEST_EXPECT(cmp_text_field_style_init(&style), CMP_ERR_IO);
  {
    cmp_u32 i;
    for (i = 1; i <= 12; ++i) {
      cmp_text_field_test_set_color_fail_after(i);
      CMP_TEST_EXPECT(cmp_text_field_style_init(&style), CMP_ERR_IO);
    }
  }
  cmp_text_field_test_set_color_fail_after(0);
  cmp_text_field_test_set_fail_point(14u);
  CMP_TEST_EXPECT(cmp_text_field_style_init(&style), CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(15u);
  CMPTextField field2;
  CMP_TEST_EXPECT(
      cmp_text_field_init(&field2, &text_backend, &style, NULL, NULL, 0),
      CMP_ERR_IO);
  {
    cmp_core_test_set_default_allocator_fail(CMP_TRUE);
    CMP_TEST_EXPECT(
        cmp_text_field_init(&field2, &text_backend, &style, NULL, NULL, 0),
        CMP_ERR_UNKNOWN);
    cmp_core_test_set_default_allocator_fail(CMP_FALSE);
  }
  {
    CMPTextFieldStyle s = style;
    s.text_style.size_px = -1;
    CMP_TEST_EXPECT(
        cmp_text_field_init(&field2, &text_backend, &s, NULL, NULL, 0),
        CMP_ERR_RANGE);
  }
  {
    CMPTextFieldStyle s = style;
    s.label_style.size_px = -1;
    s.label_style.utf8_family = "Test";
    CMP_TEST_EXPECT(
        cmp_text_field_init(&field2, &text_backend, &s, NULL, NULL, 0),
        CMP_ERR_RANGE);
  }

  cmp_text_field_test_set_fail_point(9u);
  CMPColor c1, c2, c3, c4, c5, c6, c7, c8;
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_IO);

  cmp_text_field_test_set_color_fail_after(1u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(2u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(3u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(4u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(5u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(6u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(7u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(8u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(0u);

  field.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_OK);
  cmp_text_field_test_set_color_fail_after(1u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(2u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(3u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(4u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(5u);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(&field, &c1, &c2, &c3, &c4,
                                                     &c5, &c6, &c7, &c8),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_color_fail_after(0u);
  field.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  CMPScalar width;
  CMP_TEST_EXPECT(cmp_text_field_test_measure_prefix(NULL, 0, &width),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_text_field_test_offset_for_x(NULL, 0.0f, &val),
                  CMP_ERR_INVALID_ARGUMENT);

  field.widget.vtable->destroy(&field);
  return 0;
}
