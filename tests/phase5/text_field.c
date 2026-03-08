#include "cmpc/cmp_core.h"
#include "m3/m3_text_field.h"
#include "test_utils.h"
#include <string.h>

static int test_on_change(void *ctx, M3TextField *field, const char *text) {
  int *called = (int *)ctx;
  *called = 1;
  return CMP_OK;
}

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
                             CMPScalar *w, CMPScalar *h, CMPScalar *baseline) {
  if (w)
    *w = 10.0f;
  if (h)
    *h = 10.0f;
  if (baseline)
    *baseline = 8.0f;
  return CMP_OK;
}
static int test_draw_text(void *text, CMPHandle font, const char *utf8,
                          cmp_usize utf8_len, cmp_u32 base_direction,
                          CMPScalar x, CMPScalar y, CMPColor color) {
  return CMP_OK;
}

static const CMPTextVTable mock_vtable = {test_create_font,
                                          test_destroy_font,
                                          test_measure_text,
                                          test_draw_text,
                                          NULL,
                                          NULL,
                                          NULL};

static int test_draw_rect(void *ctx, const CMPRect *r, CMPColor c, CMPScalar radius) { return CMP_OK; }
static int test_push_clip(void *ctx, const CMPRect *r) { return CMP_OK; }
static int test_pop_clip(void *ctx) { return CMP_OK; }

int main(void) {
  M3TextFieldStyle style;
  M3TextField field;
  CMPAllocator allocator;
  CMPTextBackend text_backend;
  int called = 0;
  cmp_usize len;

  CMP_TEST_EXPECT(m3_text_field_style_init(NULL, M3_TEXT_FIELD_VARIANT_FILLED),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_text_field_style_init(&style, 999),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_text_field_style_init(&style, M3_TEXT_FIELD_VARIANT_FILLED));
  CMP_TEST_OK(m3_text_field_style_init(&style, M3_TEXT_FIELD_VARIANT_OUTLINED));

  cmp_get_default_allocator(&allocator);
  text_backend.ctx = NULL;
  text_backend.vtable = &mock_vtable;

  CMP_TEST_EXPECT(m3_text_field_init(NULL, &style, allocator, text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_text_field_init(&field, NULL, allocator, text_backend),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_text_field_init(&field, &style, allocator, text_backend));

  CMP_TEST_EXPECT(m3_text_field_set_style(NULL, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_text_field_set_style(&field, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_text_field_set_style(&field, &style));

  CMP_TEST_EXPECT(m3_text_field_set_label(NULL, "Label"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_text_field_set_label(&field, "Label"));

  CMP_TEST_EXPECT(m3_text_field_set_placeholder(NULL, "Placeholder"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_text_field_set_placeholder(&field, "Placeholder"));

  CMP_TEST_EXPECT(m3_text_field_set_supporting_text(NULL, "Support"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_text_field_set_supporting_text(&field, "Support"));

  CMP_TEST_EXPECT(m3_text_field_set_error(NULL, "Error"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_text_field_set_error(&field, "Error"));
  CMP_TEST_OK(m3_text_field_set_error(&field, NULL));

  CMP_TEST_EXPECT(m3_text_field_get_text_len(NULL, &len), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_text_field_get_text_len(&field, &len));

  {
    CMPSize size;
    CMPMeasureSpec m200 = {CMP_MEASURE_EXACTLY, 200.0f};
    CMPRect bounds = {0, 0, 200, 50};
    CMPPaintContext pctx;
    CMPGfx gfx;
    CMPGfxVTable gvt;
    memset(&pctx, 0, sizeof(pctx));
    memset(&gfx, 0, sizeof(gfx));
    memset(&gvt, 0, sizeof(gvt));
    gvt.draw_rect = test_draw_rect;
    gvt.push_clip = test_push_clip;
    gvt.pop_clip = test_pop_clip;
    gfx.vtable = &gvt;
    gfx.text_vtable = &mock_vtable;
    pctx.gfx = &gfx;

    CMP_TEST_OK(m3_text_field_set_supporting_text(&field, "Support"));
    CMP_TEST_OK(field.widget.vtable->measure(&field, m200, m200, &size));
    CMP_TEST_OK(field.widget.vtable->layout(&field, bounds));
    CMP_TEST_OK(field.widget.vtable->paint(&field, &pctx));
    
    field.error = CMP_TRUE;
    CMP_TEST_OK(m3_text_field_set_error(&field, "Error"));
    CMP_TEST_OK(field.widget.vtable->measure(&field, m200, m200, &size));
    CMP_TEST_OK(field.widget.vtable->layout(&field, bounds));
    CMP_TEST_OK(field.widget.vtable->paint(&field, &pctx));
    field.error = CMP_FALSE;
    CMP_TEST_OK(m3_text_field_set_supporting_text(&field, NULL));
    CMP_TEST_OK(m3_text_field_set_error(&field, NULL));
  }

  CMP_TEST_EXPECT(m3_text_field_set_disabled(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_text_field_set_disabled(&field, CMP_TRUE));
  CMP_TEST_OK(m3_text_field_set_disabled(&field, CMP_FALSE));

  CMP_TEST_EXPECT(m3_text_field_set_on_change(NULL, test_on_change, &called),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_text_field_set_on_change(&field, test_on_change, &called));

  CMP_TEST_EXPECT(m3_text_field_set_text(NULL, "Text"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_text_field_set_text(&field, "Text"));

  CMP_TEST_EXPECT(m3_text_field_get_text_len(NULL, &len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_text_field_get_text_len(&field, &len));
  CMP_TEST_EXPECT(len, 4);

  CMP_TEST_OK(m3_text_field_test_helper());
  /* Test init failure */
  {
      CMPTextBackend bad_backend = {0};
      CMP_TEST_EXPECT(m3_text_field_init(&field, &style, allocator, bad_backend), CMP_ERR_INVALID_ARGUMENT);
  }

  /* Test measure failure */
  {
      CMPSize size;
      CMPMeasureSpec m_bad = {999, -1.0f};
      CMP_TEST_EXPECT(field.widget.vtable->measure(&field, m_bad, m_bad, &size), CMP_ERR_INVALID_ARGUMENT);
  }

  {
      CMPMeasureSpec m100 = {CMP_MEASURE_EXACTLY, 100.0f};
      CMPSize size;
      CMP_TEST_OK(m3_text_field_set_supporting_text(&field, "Support"));
      CMP_TEST_OK(field.widget.vtable->measure(&field, m100, m100, &size));
      CMP_TEST_OK(m3_text_field_set_supporting_text(&field, NULL));
  }

  /* Test layout < 0 */
  {
      CMPRect bounds = {0, 0, 100, 5};
      m3_text_field_set_supporting_text(&field, "Support");
      CMP_TEST_OK(field.widget.vtable->layout(&field, bounds));
      m3_text_field_set_supporting_text(&field, NULL);
  }

  /* Test painting with supporting text */
  {
      CMPRect bounds = {0, 0, 100, 100};
      CMPPaintContext paint_ctx = {0};
      CMPGfx gfx = {0};
      static const CMPGfxVTable dummy_gfx_vtable = {NULL, NULL, NULL, test_draw_rect, NULL, NULL, test_push_clip, test_pop_clip};
      paint_ctx.gfx = &gfx;
      gfx.vtable = &dummy_gfx_vtable;
      gfx.text_vtable = &mock_vtable;

      m3_text_field_set_supporting_text(&field, "Support");
      CMP_TEST_OK(field.widget.vtable->layout(&field, bounds));
      CMP_TEST_OK(field.widget.vtable->paint(&field, &paint_ctx));
      m3_text_field_set_supporting_text(&field, NULL);
  }

  /* Test event disabled */
  {
      CMPBool handled = CMP_TRUE;
      CMPInputEvent event = {0};
      event.type = CMP_INPUT_POINTER_DOWN;
      m3_text_field_set_disabled(&field, CMP_TRUE);
      CMP_TEST_OK(field.widget.vtable->event(&field, &event, &handled));
      CMP_TEST_EXPECT(handled, CMP_FALSE);
      m3_text_field_set_disabled(&field, CMP_FALSE);
  }

  /* Test on_change */
  {
      CMPInputEvent event = {0};
      CMPBool handled;
      int rc;
      m3_text_field_set_on_change(&field, test_on_change, &called);
      event.type = CMP_INPUT_TEXT_UTF8;
      event.data.text_utf8.utf8 = "abc";
      event.data.text_utf8.length = 3;
      called = 0;
      rc = field.widget.vtable->event(&field, &event, &handled);
      CMP_TEST_EXPECT(called, 1);

      m3_text_field_set_on_change(&field, NULL, NULL);
      rc = field.widget.vtable->event(&field, &event, &handled);
  }

  {
    CMPSize size;
    CMPRect bounds = {0, 0, 100, 100};
    CMPInputEvent event = {0};
    CMPBool handled;
    CMPSemantics semantics;

    CMPMeasureSpec m100 = {1, 100.0f};
    CMP_TEST_EXPECT(field.widget.vtable->measure(NULL, m100, m100, &size),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(field.widget.vtable->measure(&field, m100, m100, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(field.widget.vtable->measure(&field, m100, m100, &size));

    CMP_TEST_EXPECT(field.widget.vtable->layout(NULL, bounds),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(field.widget.vtable->layout(&field, bounds));

    CMP_TEST_EXPECT(field.widget.vtable->event(NULL, &event, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(field.widget.vtable->event(&field, NULL, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(field.widget.vtable->event(&field, &event, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    event.type = CMP_INPUT_POINTER_DOWN;
    CMP_TEST_OK(field.widget.vtable->event(&field, &event, &handled));

    CMP_TEST_EXPECT(field.widget.vtable->get_semantics(NULL, &semantics),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(field.widget.vtable->get_semantics(&field, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(field.widget.vtable->get_semantics(&field, &semantics));

    {
      static const CMPGfxVTable dummy_gfx_vtable = {NULL, NULL, NULL, test_draw_rect, NULL, NULL, test_push_clip, test_pop_clip};
      CMPGfx gfx = {0};
      CMPPaintContext pctx = {0};
      gfx.vtable = &dummy_gfx_vtable;
      gfx.text_vtable = &mock_vtable;
      pctx.gfx = &gfx;
      CMP_TEST_EXPECT(field.widget.vtable->paint(NULL, &pctx),
                      CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_EXPECT(field.widget.vtable->paint(&field, NULL),
                      CMP_ERR_INVALID_ARGUMENT);

      CMP_TEST_OK(m3_text_field_set_error(&field, "Error"));
      CMP_TEST_OK(field.widget.vtable->paint(&field, &pctx));
      
      CMP_TEST_OK(m3_text_field_set_error(&field, NULL));
      CMP_TEST_OK(field.widget.vtable->paint(&field, &pctx));
    }

    CMP_TEST_EXPECT(field.widget.vtable->destroy(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(field.widget.vtable->destroy(&field));
  }

  return 0;
}
