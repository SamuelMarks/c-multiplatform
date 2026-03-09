#include "cmpc/cmp_text_field.h"
#include "test_utils.h"
#include <string.h>

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

  cmp_text_field_test_set_fail_point(4u);
  CMP_TEST_EXPECT(cmp_text_field_test_update_text_metrics(&field), CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(16u);
  field.label_font.id = 1;
  CMP_TEST_EXPECT(cmp_text_field_test_update_font_metrics(&field), CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(5u);
  CMP_TEST_EXPECT(cmp_text_field_test_update_font_metrics(&field), CMP_ERR_IO);

  cmp_text_field_test_set_fail_point(8u);
  CMPScalar w;
  cmp_text_field_set_text(&field, "hello", 5);
  CMP_TEST_EXPECT(cmp_text_field_test_measure_prefix(&field, 2, &w),
                  CMP_ERR_IO);

  cmp_text_field_test_set_fail_point(7u);
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&field, 100), CMP_ERR_IO);

  cmp_text_field_test_set_fail_point(10u);
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&field, 100), CMP_OK);

  cmp_text_field_test_set_fail_point(1u);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_utf8("\xFF", 1), CMP_ERR_IO);

  cmp_text_field_test_set_fail_point(2u);
  cmp_text_field_test_set_offset_skip_early(CMP_TRUE);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset("a", 1, 1), CMP_ERR_IO);
  cmp_text_field_test_set_offset_skip_early(CMP_FALSE);

  cmp_text_field_test_set_fail_point(3u);
  cmp_text_field_test_set_offset_skip_early(CMP_TRUE);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset("a", 1, 1), CMP_ERR_IO);
  cmp_text_field_test_set_offset_skip_early(CMP_FALSE);

  cmp_text_field_test_set_fail_point(19u);
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

  cmp_text_field_test_set_fail_point(20u);
  CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_RANGE);

  cmp_text_field_test_set_fail_point(21u);
  CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_RANGE);

  cmp_text_field_test_set_fail_point(22u);
  CMPInputEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = CMP_INPUT_POINTER_DOWN;
  CMPBool handled;
  field.widget.vtable->event(&field, &ev, &handled);

  cmp_text_field_test_set_fail_point(22u);
  ev.type = CMP_INPUT_TEXT_UTF8;
  ev.data.text_utf8.utf8 = "A";
  ev.data.text_utf8.length = 1;
  CMP_TEST_EXPECT(field.widget.vtable->event(&field, &ev, &handled),
                  CMP_ERR_IO);

  cmp_text_field_test_set_fail_point(22u);
  ev.type = CMP_INPUT_TEXT_EDIT;
  ev.data.text_edit.utf8 = "A";
  ev.data.text_edit.length = 1;
  CMP_TEST_EXPECT(field.widget.vtable->event(&field, &ev, &handled),
                  CMP_ERR_IO);

  field.widget.vtable->destroy(&field);
  return 0;
}
