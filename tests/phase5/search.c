#include "m3/m3_search.h"
#include "test_utils.h"

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
                             cmp_usize utf8_len, CMPScalar *w, CMPScalar *h,
                             CMPScalar *baseline) {
  if (w)
    *w = 10.0f;
  if (h)
    *h = 10.0f;
  if (baseline)
    *baseline = 8.0f;
  return CMP_OK;
}
static int test_draw_text(void *text, CMPHandle font, const char *utf8,
                          cmp_usize utf8_len, CMPScalar x, CMPScalar y,
                          CMPColor color) {
  return CMP_OK;
}

static const CMPTextVTable mock_vtable = {test_create_font, test_destroy_font,
                                          test_measure_text, test_draw_text};

int main(void) {
  M3SearchBarStyle style;
  M3SearchBar bar;
  CMPAllocator allocator;
  CMPTextBackend text_backend;
  int called = 0;

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
  CMP_TEST_OK(m3_search_bar_set_icons(&bar, NULL, NULL));

  CMP_TEST_EXPECT(m3_search_bar_set_on_change(NULL, test_on_change, &called),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_search_bar_set_on_change(&bar, test_on_change, &called));

  CMP_TEST_OK(m3_search_test_helper());

  {
    CMPSize size;
    CMPRect bounds = {0, 0, 300, 56};
    CMPInputEvent event = {0};
    CMPBool handled;
    CMPSemantics semantics;
    CMPMeasureSpec m100 = {1, 300.0f};

    CMP_TEST_EXPECT(bar.widget.vtable->measure(NULL, m100, m100, &size),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(bar.widget.vtable->measure(&bar, m100, m100, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(bar.widget.vtable->measure(&bar, m100, m100, &size));

    CMP_TEST_EXPECT(bar.widget.vtable->layout(NULL, bounds),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(bar.widget.vtable->layout(&bar, bounds));

    CMP_TEST_EXPECT(bar.widget.vtable->event(NULL, &event, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(bar.widget.vtable->event(&bar, NULL, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(bar.widget.vtable->event(&bar, &event, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(bar.widget.vtable->get_semantics(NULL, &semantics),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(bar.widget.vtable->get_semantics(&bar, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(bar.widget.vtable->get_semantics(&bar, &semantics));

    CMP_TEST_EXPECT(bar.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(bar.widget.vtable->destroy(&bar));
  }

  return 0;
}
