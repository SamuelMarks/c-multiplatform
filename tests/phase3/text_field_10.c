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
  if (out_width) *out_width = (CMPScalar)utf8_len * 10.0f;
  if (out_height) *out_height = 20.0f;
  if (out_baseline) *out_baseline = 16.0f;
  return CMP_OK;
}
static int test_draw_text(void *text, CMPHandle font, const char *utf8,
                          cmp_usize utf8_len, cmp_u32 base_direction,
                          CMPScalar x, CMPScalar y, CMPColor color) {
  return CMP_OK;
}

static int g_test_draw_rect_fail = 0;
static int test_draw_rect(void *ctx, const CMPRect *bounds, CMPColor color, CMPScalar border_radius) {
  if (g_test_draw_rect_fail) {
      g_test_draw_rect_fail--;
      return CMP_ERR_IO;
  }
  return CMP_OK;
}

static CMPTextVTable g_test_text_vtable = {
    test_create_font, test_destroy_font, test_measure_text, test_draw_text, NULL, NULL, NULL};

int main(void) {
    CMPTextField field;
    CMPTextFieldStyle style;
    CMPTextBackend text_backend = {NULL, &g_test_text_vtable};
    
    cmp_text_field_style_init(&style);
    cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0);
    
    
    CMPMeasureSpec mw = {CMP_MEASURE_EXACTLY, 100.0f};
    CMPMeasureSpec mh = {CMP_MEASURE_EXACTLY, 50.0f};
    CMPMeasureSpec bad_spec = {999, 100.0f};
    CMPSize size;
    
    CMP_TEST_EXPECT(field.widget.vtable->measure(NULL, mw, mh, &size), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, mw, mh, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, bad_spec, mh, &size), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, mw, bad_spec, &size), CMP_ERR_INVALID_ARGUMENT);
    
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, mw, mh, &size), CMP_OK);
    
    mw.mode = CMP_MEASURE_AT_MOST;
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, mw, mh, &size), CMP_OK);
    mh.mode = CMP_MEASURE_AT_MOST;
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, mw, mh, &size), CMP_OK);

    /* The fail points for style init are for text_field_init, validate_style doesn't use them */
    cmp_text_field_test_set_fail_point(0u);
    cmp_text_field_test_set_font_metrics_fail_after(1u); /* font metrics */
    field.font_metrics_valid = 0;
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, mw, mh, &size), CMP_ERR_IO);
    cmp_text_field_test_set_font_metrics_fail_after(0u);

    cmp_text_field_test_set_fail_point(4u); /* text measure */
    field.text_metrics_valid = 0;
    cmp_text_field_test_set_fail_point(4u); /* text measure */
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, mw, mh, &size), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);
    
    /* Make text metrics valid so it skips the fail for text metrics, wait, if I set it to valid, it skips update_text_metrics! But then it goes to label metrics. If I set fail point 4u, it will fail in label_metrics! Because label_metrics ALSO checks 4u! */
    field.text_metrics_valid = 1;
    field.label_metrics_valid = 0;
    field.label_len = 4;
    field.utf8_label = "test";
    field.label_font.id = 1;
    field.label_font.generation = 1;
    cmp_text_field_test_set_fail_point(4u); /* label measure uses same fail point */
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, mw, mh, &size), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);

    field.label_metrics_valid = 1;
    field.placeholder_metrics_valid = 0;
    field.placeholder_len = 4;
    field.utf8_placeholder = "test";
    field.text_font.id = 1;
    field.text_font.generation = 1;
    cmp_text_field_test_set_fail_point(4u); /* placeholder measure uses same fail point */
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, mw, mh, &size), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);
    
    /* Style validation fail inside measure */
    field.style.min_height = -1.0f;
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, mw, mh, &size), CMP_ERR_RANGE);
    field.style.min_height = 0.0f;

    /* Hit the placeholder and label measure paths with large text */
    cmp_text_field_insert_utf8(&field, "x", 1);
    field.label_len = 10;
    field.utf8_label = "longer text";
    field.placeholder_len = 20;
    field.utf8_placeholder = "even longer text here";
    field.label_metrics_valid = 0;
    field.placeholder_metrics_valid = 0;
    field.text_metrics_valid = 0;
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, mw, mh, &size), CMP_OK);
    
    /* Cover label font higher than text font */
    field.label_font_metrics.height = 200.0f;
    field.text_font_metrics.height = 10.0f;
    field.font_metrics_valid = 1;
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, mw, mh, &size), CMP_OK);

    CMP_TEST_EXPECT(field.widget.vtable->layout(NULL, field.bounds), CMP_ERR_INVALID_ARGUMENT);
    CMPRect bad_bounds = field.bounds;
    bad_bounds.width = -1.0f;
    CMP_TEST_EXPECT(field.widget.vtable->layout(&field, bad_bounds), CMP_ERR_RANGE);
    
    field.bounds.width = 100.0f;
    field.bounds.height = 50.0f;
    field.widget.vtable->layout(&field, field.bounds);
    
    CMPPaintContext pctx;
    memset(&pctx, 0, sizeof(pctx));

    CMP_TEST_EXPECT(field.widget.vtable->paint(NULL, &pctx), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_INVALID_ARGUMENT);

    CMPGfx pgfx;
    memset(&pgfx, 0, sizeof(pgfx));
    pctx.gfx = &pgfx;
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_UNSUPPORTED);

    CMPGfxVTable pgfx_vt;
    memset(&pgfx_vt, 0, sizeof(pgfx_vt));
    pgfx_vt.draw_rect = (void*)test_draw_rect; 
    pgfx.vtable = &pgfx_vt;
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_UNSUPPORTED);

    pgfx_vt.draw_line = (void*)test_draw_text; 
    pgfx.text_vtable = &g_test_text_vtable;

    field.style.min_height = -1.0f;
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_RANGE);
    field.style.min_height = 0.0f;

    field.bounds.width = -1.0f;
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_RANGE);
    field.bounds.width = 100.0f;

    field.text_metrics_valid = 0;
    cmp_text_field_test_set_fail_point(4u); /* text measure */
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);

    field.text_metrics_valid = 1;
    field.label_metrics_valid = 0;
    cmp_text_field_test_set_fail_point(4u); /* label measure uses same fail point */
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);

    field.label_metrics_valid = 1;
    field.text_metrics_valid = 1;
    field.label_metrics_valid = 1;
    field.placeholder_metrics_valid = 0;
    field.placeholder_len = 4;
    field.utf8_placeholder = "test";
    field.text_font.id = 1;
    field.text_font.generation = 1;
    /* We also need field.text_metrics.width and others to not be 0 so placeholder is painted? No, placeholder metrics update is done REGARDLESS of whether it's painted! */
    cmp_text_field_test_set_fail_point(4u); /* text measure */
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);

    field.placeholder_metrics_valid = 1;
    field.font_metrics_valid = 0;
    cmp_text_field_test_set_font_metrics_fail_after(1u); /* font metrics */
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_IO);
    cmp_text_field_test_set_font_metrics_fail_after(0u);
    field.font_metrics_valid = 1;

    field.widget.vtable->paint(&field, &pctx);
    
    /* Paint mock failures */
    field.style.outline_color.a = 1.0f;
    field.style.outline_width = 1.0f;
    field.style.corner_radius = 0.5f; /* Smaller than outline_width to hit 1369 */
    g_test_draw_rect_fail = 1;
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_IO);
    
    field.style.container_color.a = 1.0f;
    field.style.outline_color.a = 1.0f;
    g_test_draw_rect_fail = 2; /* Fail on the SECOND draw (container) */
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_IO);
    field.style.container_color.a = 0.0f;
    
    field.style.outline_color.a = 1.0f;
    field.style.outline_width = 100.0f; /* make inner width/height negative */
    CMP_TEST_EXPECT(field.widget.vtable->paint(&field, &pctx), CMP_ERR_RANGE);
    field.style.outline_width = 1.0f;
    
    field.widget.vtable->destroy(&field);
    return 0;
}
