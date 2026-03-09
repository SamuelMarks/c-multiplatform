#include "m3/m3_list.h"
#include "test_utils.h"
#include <string.h>

static int test_measure(void *widget, CMPMeasureSpec width,
                        CMPMeasureSpec height, CMPSize *out_size) {
  if (out_size) {
    out_size->width = 24.0f;
    out_size->height = 24.0f;
  }
  return CMP_OK;
}
static int test_layout(void *widget, CMPRect bounds) { return CMP_OK; }
static int test_paint(void *widget, CMPPaintContext *ctx) { return CMP_OK; }
static int test_event(void *widget, const CMPInputEvent *event,
                      CMPBool *out_handled) {
  if (out_handled)
    *out_handled = CMP_TRUE;
  return CMP_OK;
}
static int test_semantics(void *widget, CMPSemantics *out_semantics) {
  return CMP_OK;
}
static int test_destroy(void *widget) { return CMP_OK; }

static const CMPWidgetVTable mock_vtable = {test_measure,   test_layout,
                                            test_paint,     test_event,
                                            test_semantics, test_destroy};

static int test_on_press(void *ctx, M3ListItem *item) { return CMP_OK; }

static int dummy_draw_rect(void *ctx, const CMPRect *bounds, CMPColor color,
                           CMPScalar corner_radius) {
  return CMP_OK;
}

static int dummy_create_font(void *text, const char *utf8_family,
                             cmp_i32 size_px, cmp_i32 weight, CMPBool italic,
                             CMPHandle *out_font) {
  if (out_font) {
    out_font->id = 1;
    out_font->generation = 1;
  }
  return CMP_OK;
}

static int dummy_destroy_font(void *text, CMPHandle font) { return CMP_OK; }

static int dummy_measure_text(void *text, CMPHandle font, const char *utf8,
                              cmp_usize utf8_len, cmp_u32 base_direction,
                              CMPScalar *out_width, CMPScalar *out_height,
                              CMPScalar *out_baseline) {
  return CMP_OK;
}

static int dummy_draw_text(void *text, CMPHandle font, const char *utf8,
                           cmp_usize utf8_len, cmp_u32 base_direction,
                           CMPScalar x, CMPScalar y, CMPColor color) {
  return CMP_OK;
}

int main(void) {
  M3ListItemStyle style;
  M3ListItem item;
  CMPTextBackend text_backend;
  CMPWidget leading, trailing;

  CMPTextVTable tvt;
  memset(&tvt, 0, sizeof(tvt));
  tvt.create_font = dummy_create_font;
  tvt.destroy_font = dummy_destroy_font;
  tvt.measure_text = dummy_measure_text;
  tvt.draw_text = dummy_draw_text;

  text_backend.ctx = NULL;
  text_backend.vtable = &tvt;

  leading.vtable = &mock_vtable;
  trailing.vtable = &mock_vtable;

  CMP_TEST_OK(m3_list_item_style_init(&style, M3_LIST_ITEM_VARIANT_1_LINE));
  CMP_TEST_OK(m3_list_item_init(&item, &style, text_backend));
  CMP_TEST_OK(m3_list_item_set_widgets(&item, &leading, &trailing));
  CMP_TEST_OK(m3_list_item_set_on_press(&item, test_on_press, NULL));

  CMPMeasureSpec m100 = {CMP_MEASURE_EXACTLY, 100.0f};
  CMPSize size;

  CMP_TEST_OK(item.widget.vtable->measure(&item, m100, m100, &size));

  CMPMeasureSpec matmost = {CMP_MEASURE_AT_MOST, 100.0f};
  CMP_TEST_OK(item.widget.vtable->measure(&item, matmost, m100, &size));

  CMPRect bounds = {0, 0, 100, 100};
  CMP_TEST_OK(item.widget.vtable->layout(&item, bounds));

  CMPPaintContext pctx;
  memset(&pctx, 0, sizeof(pctx));
  CMPGfx gfx;
  CMPGfxVTable gvt;
  memset(&gfx, 0, sizeof(gfx));
  memset(&gvt, 0, sizeof(gvt));
  gvt.draw_rect = dummy_draw_rect;
  gfx.vtable = &gvt;
  pctx.gfx = &gfx;

  style.background_color.a = 1.0f;
  m3_list_item_init(&item, &style, text_backend);
  m3_list_item_set_headline(&item, "hello");
  m3_list_item_set_widgets(&item, &leading, &trailing);

  item.widget.vtable->paint(&item, &pctx);

  CMPInputEvent ev;
  memset(&ev, 0, sizeof(ev));
  CMPBool handled;

  ev.type = CMP_INPUT_POINTER_DOWN;
  ev.data.pointer.x = 10;
  ev.data.pointer.y = 10;
  CMP_TEST_OK(item.widget.vtable->event(&item, &ev, &handled));

  CMP_TEST_OK(m3_list_item_set_widgets(&item, NULL, &trailing));
  CMP_TEST_OK(item.widget.vtable->event(&item, &ev, &handled));

  CMP_TEST_OK(m3_list_item_set_widgets(&item, NULL, NULL));
  CMP_TEST_OK(item.widget.vtable->event(&item, &ev, &handled));

  ev.type = CMP_INPUT_POINTER_UP;
  ev.data.pointer.x = 200;
  ev.data.pointer.y = 200;
  CMP_TEST_OK(item.widget.vtable->event(&item, &ev, &handled));

  item.widget.vtable->destroy(&item);
  return 0;
}
