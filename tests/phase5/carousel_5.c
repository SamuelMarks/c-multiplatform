/* clang-format off */
#include "m3/m3_carousel.h"
#include "test_utils.h"
#include <string.h>
/* clang-format on */

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
  if (out_handled) {
    *out_handled = CMP_TRUE;
  }
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
  CMPWidget *items[] = {&child, &child, &child};

  m3_carousel_style_init(&style);
  m3_carousel_init(&carousel, &style, items, 3);

  CMPRect bounds = {0, 0, 300, 100};
  carousel.widget.vtable->layout(&carousel, bounds);

  int i;
  for (i = 0; i < 10; ++i) {
    m3_carousel_add_item(&carousel, &child);
  }

  carousel.widget.vtable->destroy(&carousel);
  return 0;
}
