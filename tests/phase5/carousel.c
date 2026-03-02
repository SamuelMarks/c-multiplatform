#include "m3/m3_carousel.h"
#include "test_utils.h"

static int test_measure(void *widget, CMPMeasureSpec width,
                        CMPMeasureSpec height, CMPSize *out_size) {
  if (out_size) {
    out_size->width = 50.0f;
    out_size->height = 50.0f;
  }
  return CMP_OK;
}
static int test_layout(void *widget, CMPRect bounds) { return CMP_OK; }
static int test_paint(void *widget, CMPPaintContext *ctx) { return CMP_OK; }
static int test_event(void *widget, const CMPInputEvent *event,
                      CMPBool *out_handled) {
  return CMP_OK;
}
static int test_semantics(void *widget, CMPSemantics *out_semantics) {
  return CMP_OK;
}
static int test_destroy(void *widget) { return CMP_OK; }

static const CMPWidgetVTable mock_vtable = {test_measure,   test_layout,
                                            test_paint,     test_event,
                                            test_semantics, test_destroy};

int main(void) {
  M3CarouselStyle style;
  M3Carousel carousel;
  CMPWidget child;

  child.vtable = &mock_vtable;

  CMP_TEST_EXPECT(m3_carousel_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_carousel_style_init(&style));
  CMP_TEST_EXPECT(style.item_spacing == 8.0f ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);

  CMP_TEST_EXPECT(m3_carousel_init(NULL, &style, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_carousel_init(&carousel, NULL, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_carousel_init(&carousel, &style, NULL, 0));

  CMP_TEST_EXPECT(m3_carousel_add_item(NULL, &child), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_carousel_add_item(&carousel, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_carousel_add_item(&carousel, &child));

  CMP_TEST_EXPECT(m3_carousel_set_on_select(NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_carousel_set_on_select(&carousel, NULL, NULL));

  CMP_TEST_OK(m3_carousel_test_helper());

  {
    CMPSize size;
    CMPRect bounds = {0, 0, 300, 100};
    CMPInputEvent event = {0};
    CMPBool handled;
    CMPSemantics semantics;
    CMPMeasureSpec m300 = {1, 300.0f};

    CMP_TEST_EXPECT(carousel.widget.vtable->measure(NULL, m300, m300, &size),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        carousel.widget.vtable->measure(&carousel, m300, m300, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(carousel.widget.vtable->measure(&carousel, m300, m300, &size));

    CMP_TEST_EXPECT(carousel.widget.vtable->layout(NULL, bounds),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(carousel.widget.vtable->layout(&carousel, bounds));

    CMP_TEST_EXPECT(carousel.widget.vtable->event(NULL, &event, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(carousel.widget.vtable->event(&carousel, NULL, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(carousel.widget.vtable->event(&carousel, &event, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    event.type = CMP_INPUT_POINTER_DOWN;
    CMP_TEST_OK(carousel.widget.vtable->event(&carousel, &event, &handled));

    CMP_TEST_EXPECT(carousel.widget.vtable->get_semantics(NULL, &semantics),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(carousel.widget.vtable->get_semantics(&carousel, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(carousel.widget.vtable->get_semantics(&carousel, &semantics));

    CMP_TEST_EXPECT(carousel.widget.vtable->destroy(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(carousel.widget.vtable->destroy(&carousel));
  }

  return 0;
}
