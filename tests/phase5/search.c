#include "m3/m3_search.h"
#include "test_utils.h"
#include <string.h>

static int test_on_change(void *ctx, M3SearchBar *bar, const char *query) {
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

static int mock_measure(void *w, CMPMeasureSpec x, CMPMeasureSpec y, CMPSize *sz) { return CMP_OK; }
static int mock_layout(void *w, CMPRect bounds) { return CMP_OK; }
static int mock_paint(void *w, CMPPaintContext *ctx) { return CMP_OK; }
static int mock_event(void *w, const CMPInputEvent *e, CMPBool *h) { *h = CMP_TRUE; return CMP_OK; }

static const CMPTextVTable mock_vtable = {test_create_font,
                                          test_destroy_font,
                                          test_measure_text,
                                          test_draw_text,
                                          NULL,
                                          NULL,
                                          NULL};

static int mock_draw_rect(void *ctx, const CMPRect *rect, CMPColor color, CMPScalar radius) {
  return CMP_OK;
}

int main(void) {
  M3SearchBarStyle style;
  M3SearchBar bar;
  CMPAllocator allocator;
  CMPTextBackend text_backend;
  int called = 0;

  static const CMPWidgetVTable mock_widget_vtable = {
      mock_measure, mock_layout, mock_paint, mock_event, NULL, NULL
  };
  CMPWidget mock_widget = {0};
  mock_widget.vtable = &mock_widget_vtable;

  CMP_TEST_EXPECT(m3_search_bar_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_search_bar_style_init(&style));
  CMP_TEST_EXPECT(style.min_height == 56.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  cmp_get_default_allocator(&allocator);
  text_backend.ctx = NULL;
  text_backend.vtable = &mock_vtable;

  CMP_TEST_EXPECT(m3_search_bar_init(NULL, &style, allocator, text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_search_bar_init(&bar, NULL, allocator, text_backend),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_search_bar_init(&bar, &style, allocator, text_backend));

  CMP_TEST_EXPECT(m3_search_bar_set_placeholder(NULL, "Search..."),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_search_bar_set_placeholder(&bar, "Search..."));

  CMP_TEST_EXPECT(m3_search_bar_set_icons(NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  
  static const CMPWidgetVTable mock_icon_vtable = {
      mock_measure, mock_layout, mock_paint, mock_event, NULL, NULL
  };
  static CMPWidget leading;
  static CMPWidget trailing;
  memset(&leading, 0, sizeof(leading));
  memset(&trailing, 0, sizeof(trailing));
  leading.vtable = &mock_icon_vtable;
  trailing.vtable = &mock_icon_vtable;
  CMP_TEST_OK(m3_search_bar_set_icons(&bar, &leading, &trailing));

  {
    CMPSize size;
    CMPRect bounds = {0, 0, 300, 56};
    CMPInputEvent event = {0};
    CMPBool handled;
    CMPMeasureSpec m100 = {1, 300.0f};
    CMPPaintContext ctx = {0};
    CMP_TEST_OK(bar.widget.vtable->measure(&bar, m100, m100, &size));
    CMP_TEST_OK(bar.widget.vtable->layout(&bar, bounds));
    CMP_TEST_OK(bar.widget.vtable->paint(&bar, &ctx));
    CMP_TEST_OK(bar.widget.vtable->event(&bar, &event, &handled));
  }

  CMP_TEST_EXPECT(m3_search_bar_set_on_change(NULL, test_on_change, &called),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_search_bar_set_on_change(&bar, test_on_change, &called));

  /* Manually trigger on_change */
  if (bar.field.on_change != NULL) {
    bar.field.on_change(bar.field.on_change_ctx, &bar.field, "hello");
    CMP_TEST_EXPECT(called, 1);
  }

  CMP_TEST_OK(m3_search_test_helper());

  {
    CMPSize size;
    CMPRect bounds = {0, 0, 300, 56};
    CMPInputEvent event = {0};
    CMPBool handled;
    CMPSemantics semantics;
    CMPMeasureSpec m100 = {1, 300.0f};
    CMPPaintContext ctx = {0};

    static const CMPGfxVTable mock_gfx_vtable = {
        NULL, NULL, NULL, mock_draw_rect, NULL, NULL, NULL, NULL
    };
    CMPGfx mock_gfx = {0};

    mock_gfx.vtable = &mock_gfx_vtable;
    ctx.gfx = &mock_gfx;

    CMP_TEST_EXPECT(bar.widget.vtable->measure(NULL, m100, m100, &size),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(bar.widget.vtable->measure(&bar, m100, m100, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(bar.widget.vtable->measure(&bar, m100, m100, &size));

    CMP_TEST_EXPECT(bar.widget.vtable->layout(NULL, bounds),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(bar.widget.vtable->layout(&bar, bounds));

    CMP_TEST_EXPECT(bar.widget.vtable->paint(NULL, &ctx),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(bar.widget.vtable->paint(&bar, &ctx));

    CMP_TEST_EXPECT(bar.widget.vtable->event(NULL, &event, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(bar.widget.vtable->event(&bar, NULL, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(bar.widget.vtable->event(&bar, &event, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(bar.widget.vtable->event(&bar, &event, &handled));

    /* Test event with icons */
    m3_search_bar_set_icons(&bar, &mock_widget, &mock_widget);
    CMP_TEST_OK(bar.widget.vtable->event(&bar, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_TRUE);

    CMP_TEST_EXPECT(bar.widget.vtable->get_semantics(NULL, &semantics),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(bar.widget.vtable->get_semantics(&bar, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(bar.widget.vtable->get_semantics(&bar, &semantics));
  }

  {
    M3SearchView view;
    CMPSize size;
    CMPRect bounds = {0, 0, 300, 800};
    CMPInputEvent event = {0};
    CMPBool handled;
    CMPSemantics semantics;
    CMPMeasureSpec m100 = {1, 300.0f};
    CMPPaintContext pctx = {0};

    CMP_TEST_EXPECT(m3_search_view_init(NULL, &bar.widget, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_search_view_init(&view, NULL, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(m3_search_view_init(&view, &bar.widget, &bar.widget));

    CMP_TEST_EXPECT(m3_search_view_set_active(NULL, CMP_TRUE, CMP_FALSE), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(m3_search_view_set_active(&view, CMP_TRUE, CMP_FALSE));
    CMP_TEST_OK(m3_search_view_set_active(&view, CMP_FALSE, CMP_TRUE));
    CMP_TEST_OK(m3_search_view_set_active(&view, CMP_TRUE, CMP_TRUE));

    CMP_TEST_EXPECT(view.widget.vtable->measure(NULL, m100, m100, &size),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(view.widget.vtable->measure(&view, m100, m100, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(view.widget.vtable->measure(&view, m100, m100, &size));

    CMP_TEST_EXPECT(view.widget.vtable->layout(NULL, bounds),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(view.widget.vtable->layout(&view, bounds));

    CMP_TEST_EXPECT(view.widget.vtable->paint(NULL, &pctx),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(view.widget.vtable->paint(&view, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(view.widget.vtable->paint(&view, &pctx));

    CMP_TEST_EXPECT(view.widget.vtable->event(NULL, &event, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(view.widget.vtable->event(&view, NULL, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(view.widget.vtable->event(&view, &event, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(view.widget.vtable->event(&view, &event, &handled));

    view.search_bar = &mock_widget;
    CMP_TEST_OK(view.widget.vtable->event(&view, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_TRUE);

    CMP_TEST_EXPECT(view.widget.vtable->get_semantics(NULL, &semantics),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(view.widget.vtable->get_semantics(&view, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(view.widget.vtable->get_semantics(&view, &semantics));

    CMP_TEST_EXPECT(view.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(view.widget.vtable->destroy(&view));
  }

  CMP_TEST_EXPECT(bar.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(bar.widget.vtable->destroy(&bar));

  return 0;
}
