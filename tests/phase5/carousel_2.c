#include <string.h>
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
  CMPWidget *items[] = {&child};
  
  m3_carousel_style_init(&style);
  m3_carousel_init(&carousel, &style, items, 1);
  
  CMPRect bounds = {0, 0, 300, 100};
  carousel.widget.vtable->layout(&carousel, bounds);
  
  CMPPaintContext pctx;
  memset(&pctx, 0, sizeof(pctx));
  carousel.widget.vtable->paint(&carousel, &pctx);
  
  CMPInputEvent ev;
  memset(&ev, 0, sizeof(ev));
  CMPBool handled;
  
  ev.type = CMP_INPUT_POINTER_DOWN;
  ev.data.pointer.x = 10;
  carousel.widget.vtable->event(&carousel, &ev, &handled);
  
  ev.type = CMP_INPUT_POINTER_MOVE;
  ev.data.pointer.x = 20;
  carousel.widget.vtable->event(&carousel, &ev, &handled);
  
  ev.type = CMP_INPUT_POINTER_UP;
  carousel.widget.vtable->event(&carousel, &ev, &handled);
  
  ev.type = CMP_INPUT_GESTURE_FLING;
  ev.data.gesture.velocity_x = 100.0f;
  carousel.widget.vtable->event(&carousel, &ev, &handled);
  
  ev.type = CMP_INPUT_KEY_DOWN;
  ev.data.key.key_code = 2; 
  carousel.widget.vtable->event(&carousel, &ev, &handled);
  
  ev.data.key.key_code = 3; 
  carousel.widget.vtable->event(&carousel, &ev, &handled);
  
  carousel.widget.vtable->destroy(&carousel);
  return 0;
}
