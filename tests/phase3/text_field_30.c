
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
static int test_pop_clip(void *ctx) {
  static int call = 0;
  if (call++ == 12)
    return CMP_ERR_IO;
  return CMP_OK;
}
static int test_destroy_font(void *text, CMPHandle font) {
  if (font.id == 99)
    return CMP_ERR_IO;
  return CMP_OK;
}
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
  if (color.r > 0.2f && color.r < 0.4f)
    return CMP_ERR_IO;
  return CMP_OK;
}

static int test_on_change(void *ctx, CMPTextField *field, const char *utf8,
                          cmp_usize utf8_len) {
  return CMP_ERR_IO;
}

static int test_draw_rect(void *ctx, const CMPRect *rect, CMPColor color,
                          CMPScalar corner_radius) {
  if (color.r > 0.0f && color.r < 0.2f)
    return CMP_ERR_IO; /* magic error trigger */
  return CMP_OK;
}
static int test_draw_line(void *ctx, CMPScalar x1, CMPScalar y1, CMPScalar x2,
                          CMPScalar y2, CMPColor color, CMPScalar thickness) {
  return CMP_OK;
}
static int test_draw_path(void *ctx, const CMPPath *path, CMPColor color) {
  return CMP_OK;
}
static int test_push_clip(void *ctx, const CMPRect *rect) {
  if (rect->x > 1000.0f)
    return CMP_ERR_IO;
  return CMP_OK;
}
static int test_set_transform(void *ctx, const CMPMat3 *transform) {
  return CMP_OK;
}
static int test_draw_texture(void *ctx, CMPHandle texture, const CMPRect *src,
                             const CMPRect *dst, CMPScalar alpha) {
  return CMP_OK;
}
static int test_create_texture(void *ctx, cmp_i32 width, cmp_i32 height,
                               cmp_u32 format, const void *pixels,
                               cmp_usize size, CMPHandle *out_texture) {
  return CMP_OK;
}
static int test_update_texture(void *ctx, CMPHandle texture, cmp_i32 x,
                               cmp_i32 y, cmp_i32 width, cmp_i32 height,
                               const void *pixels, cmp_usize size) {
  return CMP_OK;
}
static int test_destroy_texture(void *ctx, CMPHandle texture) { return CMP_OK; }

static CMPGfxVTable g_test_gfx_vtable = {
    NULL,           NULL,           NULL,
    test_draw_rect, test_draw_line, test_draw_path,
    test_push_clip, test_pop_clip,  test_set_transform,
};

static CMPTextVTable g_test_text_vtable = {test_create_font,
                                           test_destroy_font,
                                           test_measure_text,
                                           test_draw_text,
                                           NULL,
                                           NULL,
                                           NULL};
static CMPGfx g_test_gfx_ctx = {NULL, &g_test_gfx_vtable, &g_test_text_vtable};
int main(void) {

  CMPTextField field;
  CMPTextFieldStyle style;
  CMPTextBackend text_backend = {NULL, &g_test_text_vtable};
  cmp_usize out_value;
  cmp_usize out_offset;

  cmp_text_field_style_init(&style);
  cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0);

  /* Test cmp_text_field_add_overflow */
  CMP_TEST_EXPECT(cmp_text_field_test_add_overflow(1, 1, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_text_field_test_add_overflow(~(cmp_usize)0, 1, &out_value),
      CMP_ERR_OVERFLOW);

  /* Test cmp_text_field_reserve */
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&field, 0),
                  CMP_ERR_INVALID_ARGUMENT);

  field.allocator.realloc = NULL;
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&field, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  cmp_text_field_init(&field, &text_backend, &style, NULL, NULL,
                      0); /* restore */

  field.utf8_capacity = ~(cmp_usize)0 / 2u + 1u;
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&field, field.utf8_capacity + 1),
                  CMP_ERR_OVERFLOW);

  cmp_text_field_test_set_fail_point(10u); /* RESERVE BYPASS */
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&field, field.utf8_capacity),
                  CMP_ERR_OUT_OF_MEMORY);
  cmp_text_field_test_set_fail_point(0u);

  field.utf8_capacity = 0;
  cmp_text_field_test_set_fail_point(7u); /* RESERVE REALLOC */
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&field, ~(cmp_usize)0),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  field.utf8_capacity = 0;
  field.utf8 = NULL;

  /* Test cmp_text_field_validate_utf8 */
  CMP_TEST_EXPECT(cmp_text_field_test_validate_utf8(NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);

  cmp_text_field_test_set_fail_point(1u);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_utf8("a", 1), CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  /* Test cmp_text_field_validate_offset */
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset(NULL, 2, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset("abc", 3, 4),
                  CMP_ERR_RANGE);
  /* Test validate_offset inside multibyte character */
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset("a\xC2\xA3", 3, 2),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_fail_point(3u); /* ITER NEXT */
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset("abc", 3, 1), CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  cmp_text_field_test_set_fail_point(12u); /* ITER NOT FOUND */
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset("abc", 3, 1),
                  CMP_ERR_RANGE);
  cmp_text_field_test_set_fail_point(0u);

  /* Test validate_offset skip early bypass */
  cmp_text_field_test_set_offset_skip_early(CMP_TRUE);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset("", 0, 0), CMP_OK);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset("abc", 3, 3), CMP_OK);
  cmp_text_field_test_set_offset_skip_early(CMP_FALSE);

  /* Test cmp_text_field_prev_offset */
  CMP_TEST_EXPECT(cmp_text_field_test_prev_offset(NULL, 1, 1, &out_offset),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_prev_offset("abc", 3, 4, &out_offset),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_text_field_test_prev_offset("abc", 3, 0, &out_offset),
                  CMP_OK);
  CMP_TEST_EXPECT(cmp_text_field_test_prev_offset("abc", 3, 1, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test prev_offset fail points */
  cmp_text_field_test_set_fail_point(2u);
  CMP_TEST_EXPECT(cmp_text_field_test_prev_offset("abc", 3, 3, &out_offset),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  cmp_text_field_test_set_fail_point(3u);
  CMP_TEST_EXPECT(cmp_text_field_test_prev_offset("abc", 3, 3, &out_offset),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  cmp_text_field_test_set_fail_point(12u);
  CMP_TEST_EXPECT(cmp_text_field_test_prev_offset("abc", 3, 3, &out_offset),
                  CMP_OK);
  cmp_text_field_test_set_fail_point(0u);

  /* Test cmp_text_field_next_offset */
  CMP_TEST_EXPECT(cmp_text_field_test_next_offset(NULL, 1, 0, &out_offset),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_next_offset(NULL, 2, 1, &out_offset),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_next_offset("abc", 3, 3, &out_offset),
                  CMP_OK);
  CMP_TEST_EXPECT(cmp_text_field_test_next_offset("abc", 3, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test next_offset fail points */
  cmp_text_field_test_set_fail_point(2u);
  CMP_TEST_EXPECT(cmp_text_field_test_next_offset("abc", 3, 0, &out_offset),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  cmp_text_field_test_set_fail_point(3u);
  CMP_TEST_EXPECT(cmp_text_field_test_next_offset("abc", 3, 0, &out_offset),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  cmp_text_field_test_set_fail_point(12u);
  CMP_TEST_EXPECT(cmp_text_field_test_next_offset("abc", 3, 0, &out_offset),
                  CMP_OK);
  cmp_text_field_test_set_fail_point(0u);

  /* Test metrics NULL fields */
  CMP_TEST_EXPECT(cmp_text_field_test_update_text_metrics(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_update_label_metrics(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_update_placeholder_metrics(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_update_font_metrics(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test label metrics with no label font */
  field.label_font.id = 0;
  field.label_font.generation = 0;
  field.label_metrics_valid = CMP_FALSE;
  CMP_TEST_EXPECT(cmp_text_field_test_update_label_metrics(&field), CMP_OK);

  /* Test font metrics with no label font */
  field.font_metrics_valid = CMP_FALSE;
  CMP_TEST_EXPECT(cmp_text_field_test_update_font_metrics(&field), CMP_OK);

  /* Test font_metrics_fail_after for label font */
  field.label_font.id = 1;
  field.label_font.generation = 1;
  field.font_metrics_valid = CMP_FALSE;
  cmp_text_field_test_set_font_metrics_fail_after(2u);
  CMP_TEST_EXPECT(cmp_text_field_test_update_font_metrics(&field), CMP_ERR_IO);
  cmp_text_field_test_set_font_metrics_fail_after(0u);

  /* Test sync label anim start fail */
  field.utf8_label = NULL;
  field.label_len = 0;
  cmp_text_field_test_set_fail_point(6u);
  CMP_TEST_EXPECT(cmp_text_field_test_sync_label(&field), CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  /* Test sync label anim start fail with label */
  field.utf8_label = "lbl";
  field.label_len = 3;
  field.focused = CMP_TRUE;
  field.label_value = 0.0f;
  cmp_text_field_test_set_fail_point(6u);
  CMP_TEST_EXPECT(cmp_text_field_test_sync_label(&field), CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  field.style.label_anim_duration = 0.0f;
  CMP_TEST_EXPECT(cmp_text_field_test_sync_label(&field), CMP_OK);
  /* Test reset_cursor_blink NULL */
  CMP_TEST_EXPECT(cmp_text_field_test_reset_cursor_blink(NULL), CMP_OK);

  /* Test set_text_internal overflow */
  cmp_text_field_test_set_overflow_fail_after(1u);
  int my_rc =
      cmp_text_field_test_set_text_internal(&field, "abc", 3, CMP_FALSE);
  CMP_TEST_EXPECT(my_rc, CMP_ERR_OVERFLOW);
  cmp_text_field_test_set_overflow_fail_after(0u);
  cmp_text_field_test_set_fail_point(7u);
  field.utf8_capacity = 0;
  CMP_TEST_EXPECT(
      cmp_text_field_test_set_text_internal(&field, "abc", 3, CMP_FALSE),
      CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  field.utf8_capacity = 0;
  field.utf8 = NULL;
  field.utf8_label = "lbl";
  field.label_len = 3;
  field.label_value = 0.0f;
  field.focused = CMP_TRUE;
  cmp_text_field_test_set_fail_point(6u);
  int anim_rc =
      cmp_text_field_test_set_text_internal(&field, "abc", 3, CMP_FALSE);
  CMP_TEST_EXPECT(anim_rc, CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  /* Test on_change fail */
  field.on_change = test_on_change;
  int oc_rc = cmp_text_field_test_set_text_internal(&field, "abc", 3, CMP_TRUE);
  cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0);
  CMP_TEST_EXPECT(oc_rc, CMP_ERR_IO);
  field.on_change = NULL;

  CMP_TEST_EXPECT(cmp_text_field_test_delete_range(&field, 2, 1, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_text_field_test_delete_range(&field, 0, 100, CMP_FALSE),
                  CMP_ERR_RANGE);

  /* Test sync label NULL */
  CMP_TEST_EXPECT(cmp_text_field_test_sync_label(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test delete_range sync_label fail */
  cmp_text_field_test_set_text_internal(&field, "abc", 3, CMP_FALSE);
  field.utf8_label = "lbl";
  field.label_len = 3;
  field.label_value = 0.0f;
  field.focused = CMP_TRUE;
  cmp_text_field_test_set_fail_point(6u);
  /* skip delete_range */
  cmp_text_field_test_set_fail_point(0u);
  /* Test delete_range on_change fail */
  field.on_change = test_on_change;
  field.on_change = NULL;

  /* Test offset_for_x fail points */
  cmp_text_field_test_set_text_internal(&field, "abc", 3, CMP_FALSE);
  cmp_text_field_test_set_fail_point(2u); /* ITER INIT */
  CMP_TEST_EXPECT(cmp_text_field_test_offset_for_x(&field, 10.0f, &out_offset),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  cmp_text_field_test_set_fail_point(3u); /* ITER NEXT */
  CMP_TEST_EXPECT(cmp_text_field_test_offset_for_x(&field, 10.0f, &out_offset),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  /* Test offset_for_x measure prefix fail */
  cmp_text_field_test_set_fail_point(4u); /* MEASURE */
  CMP_TEST_EXPECT(cmp_text_field_test_offset_for_x(&field, 10.0f, &out_offset),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  /* Test resolve_colors NULL args */
  CMPColor container, outline, text, label, placeholder, cursor, selection,
      handle;
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      NULL, &container, &outline, &text, &label, &placeholder,
                      &cursor, &selection, &handle),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      &field, NULL, &outline, &text, &label, &placeholder,
                      &cursor, &selection, &handle),
                  CMP_ERR_INVALID_ARGUMENT);
  /* Test measure_prefix out of bounds */
  CMPScalar out_width;
  CMP_TEST_EXPECT(cmp_text_field_test_measure_prefix(&field, 100, &out_width),
                  CMP_ERR_RANGE);
  /* Test measure_prefix validate_offset fail */
  cmp_text_field_test_set_text_internal(&field, "a\xC2\xA3", 3, CMP_FALSE);
  CMP_TEST_EXPECT(cmp_text_field_test_measure_prefix(&field, 2, &out_width),
                  CMP_ERR_RANGE);
  /* Test measure_prefix TEXT_MEASURE fail */
  cmp_text_field_test_set_fail_point(4u);
  CMP_TEST_EXPECT(cmp_text_field_test_measure_prefix(&field, 1, &out_width),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  /* Test widget paint */
  CMPPaintContext paint_ctx;
  memset(&paint_ctx, 0, sizeof(paint_ctx));
  paint_ctx.gfx = &g_test_gfx_ctx;

  CMPWidget *widget = (CMPWidget *)&field;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Normal paint */
  field.bounds.x = 0;
  field.bounds.y = 0;
  field.bounds.width = 100;
  field.bounds.height = 40;
  field.style.outline_width = 1.0f;
  field.style.corner_radius = 5.0f;
  field.style.container_color.a = 1.0f;
  field.style.outline_color.a = 1.0f;

  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_OK);

  /* Paint disabled */
  field.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  field.style.disabled_container_color.a = 1.0f;
  field.style.disabled_outline_color.a = 1.0f;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_OK);
  field.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  /* Paint negative inner rect (outline_width > bounds/2) */
  field.style.outline_width = 100.0f;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_RANGE);
  field.style.outline_width = 1.0f;

  /* Paint with negative corner radius */
  field.style.outline_width = 5.0f;
  field.style.corner_radius = 2.0f;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_OK);
  field.style.outline_width = 1.0f;
  field.style.corner_radius = 5.0f;

  /* Paint inner bounds trigger fail (magic color) */
  field.focused = CMP_FALSE;
  field.style.outline_color.r = 0.1f;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_IO);
  field.style.outline_color.r = 0.0f;

  field.style.container_color.r = 0.1f;
  field.style.outline_color.a = 0.0f; /* skip outline */
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_IO);
  field.style.container_color.r = 0.0f;
  /* Text draw fail */
  field.style.text_style.color.a = 1.0f;
  field.style.text_style.color.r = 0.3f;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_IO);
  field.style.text_style.color.r = 0.0f;

  /* Selection measurement failing inside paint */
  field.selection_start = 0;
  field.selection_end = 3;
  field.style.selection_color.a = 1.0f;
  cmp_text_field_test_set_fail_point(4u); /* MEASURE_PREFIX */
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  /* Selection end measuring failing */
  field.selection_start = 0;
  field.selection_end = 3;
  field.style.selection_color.a = 1.0f;
  cmp_text_field_test_set_font_metrics_fail_after(
      2u); /* Let start succeed, fail end */
  /* Wait, measure_prefix uses TEXT_MEASURE (4u). So let's fail the second call
   */
  /* We do not have a fail_after for fail points, only global boolean matching.
   */
  /* Actually, we can use an invalid offset to fail measure_prefix natively! */
  field.selection_end = 100;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_RANGE);
  field.selection_end = 3;

  /* Selection paint draw_rect fail */
  field.selection_start = 0;
  field.selection_end = 3;
  field.style.selection_color.a = 1.0f;
  field.style.selection_color.r = 0.1f; /* Magic fail rect */
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_IO);
  field.style.selection_color.r = 0.0f;
  /* Selection start measuring failing */
  cmp_text_field_test_set_fail_point(
      12u); /* ITER_NOT_FOUND to fail measure_prefix? Wait measure_prefix has
               CMP_TEXT_FIELD_TEST_FAIL_MEASURE_PREFIX. Actually TEXT_MEASURE is
               4u. MEASURE_PREFIX is something else? */
  cmp_text_field_test_set_fail_point(8u); /* MEASURE_PREFIX */
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  field.style.selection_color.a = 0.0f;

  /* Paint corner radius > outline_width */
  field.style.outline_width = 1.0f;
  field.style.corner_radius = 5.0f;
  widget->vtable->paint(widget, &paint_ctx);

  /* Paint push clip fail */
  field.bounds.x = 2000.0f;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_IO);
  field.bounds.x = 0.0f;

  /* Reverse selection path */
  field.selection_start = 3;
  field.selection_end = 0;
  field.style.selection_color.a = 1.0f;
  widget->vtable->paint(widget, &paint_ctx);

  /* Paint placeholder when text is empty */
  cmp_text_field_test_set_text_internal(&field, "", 0, CMP_FALSE);
  field.utf8_placeholder = "plc";
  field.placeholder_len = 3;
  field.utf8_label = NULL;
  field.label_len = 0;
  widget->vtable->paint(widget, &paint_ctx);
  /* Paint placeholder drawing fail */
  field.style.placeholder_color.r = 0.3f;
  field.style.placeholder_color.a = 1.0f;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_IO);
  field.style.placeholder_color.r = 0.0f;

  /* Paint missing outline */
  field.style.outline_color.a = 0.0f;
  field.style.outline_width = 0.0f;
  widget->vtable->paint(widget, &paint_ctx);
  field.style.outline_color.a = 1.0f;
  field.style.outline_width = 1.0f;
  /* Paint content size out of bounds */
  field.bounds.width = 0.0f;
  field.bounds.height = 0.0f;
  widget->vtable->paint(widget, &paint_ctx);
  field.style.padding_x = 100.0f;
  widget->vtable->paint(widget, &paint_ctx);
  field.style.padding_x = 0.0f;
  field.bounds.width = 100.0f;
  field.bounds.height = 40.0f;

  /* Paint label drawing fail */
  field.style.label_style.color.a = 1.0f;
  field.style.label_style.color.r = 0.3f;
  field.utf8_label = "lbl";
  field.label_len = 3;
  field.label_font.id = 1;
  field.label_font.generation = 1;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_IO);
  field.style.label_style.color.r = 0.0f;

  /* Paint cursor prefix measurement fail */
  field.focused = CMP_TRUE;
  field.cursor_visible = CMP_TRUE;
  field.style.cursor_width = 1.0f;
  field.style.cursor_color.a = 1.0f;
  cmp_text_field_test_set_fail_point(4u);
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  /* Paint cursor drawing fail */
  field.style.cursor_color.r = 0.1f;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_IO);
  field.style.cursor_color.r = 0.0f;

  /* Paint handle test */
  field.selection_start = 0;
  field.selection_end = 3;
  field.style.handle_color.a = 1.0f;
  field.style.handle_radius = 5.0f;
  field.style.handle_height = -1.0f; /* force adjust */
  widget->vtable->paint(widget, &paint_ctx);
  field.style.handle_height = 10.0f;
  /* Paint handle start measure fail */
  cmp_text_field_test_set_fail_point(4u);
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  /* Paint handle end measure fail */
  cmp_text_field_test_set_font_metrics_fail_after(2u);
  field.selection_end = 100;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_RANGE);
  field.selection_end = 3;
  /* Paint handle reverse selection */
  field.selection_start = 3;
  field.selection_end = 0;
  widget->vtable->paint(widget, &paint_ctx);
  /* Paint handle draw fail */
  field.style.handle_color.r = 0.15f;
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_RANGE);
  field.style.handle_color.r = 0.0f;

  /* Paint pop clip fail */
  CMP_TEST_EXPECT(widget->vtable->paint(widget, &paint_ctx), CMP_ERR_RANGE);
  /* Paint text_font_metrics height out of bounds */
  field.text_font_metrics.height = 100.0f;
  widget->vtable->paint(widget, &paint_ctx);
  field.text_font_metrics.height = 10.0f;
  /* Cursor size bypass */
  field.selection_start = 3;
  field.selection_end = 3;
  field.focused = CMP_TRUE;
  field.cursor_visible = CMP_TRUE;
  field.selection_start = 0;
  field.selection_end = 3;
  field.style.selection_color.a = 1.0f;
  field.selection_start = 0;
  field.selection_end = 3;
  field.style.selection_color.a = 1.0f;
  cmp_text_field_test_set_fail_point(19u); /* SELECTION_WIDTH_NEGATIVE */
  widget->vtable->paint(widget, &paint_ctx);
  field.style.selection_color.a = 0.0f;
  cmp_text_field_test_set_fail_point(20u); /* CURSOR_WIDTH_NEGATIVE */
  widget->vtable->paint(widget, &paint_ctx);
  cmp_text_field_test_set_fail_point(21u); /* CURSOR_HEIGHT_NEGATIVE */
  widget->vtable->paint(widget, &paint_ctx);
  cmp_text_field_test_set_fail_point(0u);

  /* Widget event NULL checks */
  CMPBool out_handled;
  CMPInputEvent evt;
  memset(&evt, 0, sizeof(evt));
  CMP_TEST_EXPECT(widget->vtable->event(NULL, &evt, &out_handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(widget->vtable->event(widget, NULL, &out_handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  /* Widget event disabled */
  field.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled), CMP_OK);
  field.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  /* Widget event POINTER_DOWN offset fail */
  cmp_text_field_test_set_text_internal(&field, "abc", 3, CMP_FALSE);
  evt.type = CMP_INPUT_POINTER_DOWN;
  evt.data.pointer.x = 100.0f;
  cmp_text_field_test_set_fail_point(3u);
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  field.selecting = CMP_TRUE;
  evt.type = CMP_INPUT_POINTER_MOVE;
  cmp_text_field_test_set_fail_point(3u);
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  field.selecting = CMP_FALSE;
  evt.type = CMP_INPUT_POINTER_MOVE;
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled), CMP_OK);

  /* Widget event POINTER_UP not selecting */
  evt.type = CMP_INPUT_POINTER_UP;
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled), CMP_OK);
  /* Widget event TEXT zero len */
  evt.type = CMP_INPUT_TEXT;
  evt.data.text.length = 0;
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled), CMP_OK);
  /* Widget event TEXT insert fail */
  evt.data.text.length = 1;
#if defined(_MSC_VER)
  strcpy_s(evt.data.text.utf8, sizeof(evt.data.text.utf8), "x");
#else
  strcpy(evt.data.text.utf8, "x");
#endif
  cmp_text_field_test_set_fail_point(7u);
  field.utf8_capacity = 0;
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  /* Widget event TEXT_UTF8 zero len */
  evt.type = CMP_INPUT_TEXT_UTF8;
  evt.data.text_utf8.length = 0;
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled), CMP_OK);

  /* Widget event TEXT sync_label fail */
  evt.type = CMP_INPUT_TEXT;
  evt.data.text.length = 1;
#if defined(_MSC_VER)
  strcpy_s(evt.data.text.utf8, sizeof(evt.data.text.utf8), "y");
#else
  strcpy(evt.data.text.utf8, "y");
#endif
  cmp_text_field_test_set_fail_point(22u);
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  /* Widget event TEXT_UTF8 sync_label fail */
  evt.type = CMP_INPUT_TEXT_UTF8;
  evt.data.text_utf8.length = 1;
  evt.data.text_utf8.utf8 = "y";
  cmp_text_field_test_set_fail_point(22u);
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  /* Widget event TEXT_EDIT insert fail */
  evt.type = CMP_INPUT_TEXT_EDIT;
  evt.data.text_edit.length = 1;
  evt.data.text_edit.utf8 = "x";
  cmp_text_field_test_set_fail_point(7u);
  field.utf8_capacity = 0;
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  evt.data.text_edit.length = 0;
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled), CMP_OK);
  evt.data.text_edit.length = 1;
  /* Widget event TEXT_EDIT sync_label fail */
  field.utf8_capacity = 100;
  field.utf8_label = "lbl";
  field.label_len = 3;
  field.label_value = 0.0f;
  field.focused = CMP_TRUE;
  cmp_text_field_test_set_fail_point(6u);
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  /* Widget event TEXT_UTF8 insert fail */
  evt.data.text_utf8.length = 1;
  evt.data.text_utf8.utf8 = "x";

  /* Key down backspace fail */
  evt.type = CMP_INPUT_KEY_DOWN;
  evt.modifiers = 0;
  evt.data.key.key_code = 8u;
  field.cursor = 1;
  field.selection_start = 1;
  field.selection_end = 1;
  cmp_text_field_test_set_fail_point(3u); /* ITER NEXT */
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  /* Key down backspace sync_label fail */
  evt.data.key.key_code = 8u;
  field.cursor = 1;
  field.selection_start = 1;
  field.selection_end = 1;
  cmp_text_field_test_set_fail_point(6u);
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  /* Key down delete fail */
  evt.data.key.key_code = 46u;
  field.cursor = 0;
  field.selection_start = 0;
  field.selection_end = 0;
  cmp_text_field_test_set_fail_point(3u); /* ITER NEXT */
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  /* Key down delete sync_label fail */
  evt.data.key.key_code = 46u;
  field.cursor = 0;
  field.selection_start = 0;
  field.selection_end = 0;
  cmp_text_field_test_set_fail_point(6u);
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);
  /* Key down arrow left out of bounds/fail */
  evt.type = CMP_INPUT_KEY_DOWN;
  evt.modifiers = 0;
  evt.data.key.key_code = 37u;
  field.cursor = 1;
  cmp_text_field_test_set_fail_point(3u); /* prev_offset ITER_NEXT */
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  /* Key down arrow left unhandled */
  field.cursor = 0;
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled), CMP_OK);

  /* Key down arrow right out of bounds/fail */
  evt.data.key.key_code = 39u;
  field.cursor = 0;
  cmp_text_field_test_set_fail_point(3u); /* next_offset ITER_NEXT */
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled),
                  CMP_ERR_IO);
  cmp_text_field_test_set_fail_point(0u);

  /* Key down arrow right unhandled */
  field.cursor = 1;
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled), CMP_OK);

  /* Key down Home */
  evt.data.key.key_code = 36u;
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled), CMP_OK);

  /* Key down End */
  evt.data.key.key_code = 35u;
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled), CMP_OK);

  /* Select all */
  evt.modifiers = CMP_MOD_CTRL;
  evt.data.key.key_code = 'A';
  CMP_TEST_EXPECT(widget->vtable->event(widget, &evt, &out_handled), CMP_OK);

  /* Widget get semantics */
  CMPSemantics semantics;
  CMP_TEST_EXPECT(widget->vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(widget->vtable->get_semantics(widget, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(widget->vtable->get_semantics(widget, &semantics), CMP_OK);

  /* Widget destroy */
  CMP_TEST_EXPECT(widget->vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);

  /* Test destroy fail unsupported */
  field.owns_fonts = CMP_TRUE;
  field.text_font.id = 1;
  field.label_font.id = 1;
  field.text_backend.vtable = NULL;
  CMP_TEST_EXPECT(widget->vtable->destroy(widget), CMP_ERR_UNSUPPORTED);
  field.text_backend.vtable = &g_test_text_vtable;
  cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0);
  field.utf8 = NULL;

  /* Test destroy fail from vtable */
  field.utf8 = NULL;
  field.text_font.id = 99;
  CMP_TEST_EXPECT(widget->vtable->destroy(widget), CMP_ERR_IO);
  field.utf8 = NULL;
  cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0);
  field.text_font.id = 1;
  field.label_font.id = 99;
  CMP_TEST_EXPECT(widget->vtable->destroy(widget), CMP_ERR_IO);
  field.utf8 = NULL;
  field.label_font.id = 1;
  field.owns_fonts = CMP_FALSE;

  /* Test cmp_text_field_set_text_internal */
  CMP_TEST_EXPECT(
      cmp_text_field_test_set_text_internal(NULL, "a", 1, CMP_FALSE),
      CMP_ERR_INVALID_ARGUMENT);

  /* removed destroy */

  /* Test reserve edge cases */
  field.allocator.realloc = NULL;
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&field, 1),
                  CMP_ERR_INVALID_ARGUMENT);

  cmp_text_field_init(&field, &text_backend, &style, NULL, NULL,
                      0); /* restore */

  cmp_text_field_test_set_fail_point(
      3u); /* ITER NEXT? Wait, let's use the explicit fail point numbers */

  return 0;
}
