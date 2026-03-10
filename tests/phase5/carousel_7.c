/* clang-format off */
#include "m3/m3_carousel.h"
#include "test_utils.h"
#include <string.h>
/* clang-format on */

static int test_event(void *widget, const CMPInputEvent *event,
                      CMPBool *out_handled) {
  return CMP_OK;
}

int main(void) {
  M3CarouselStyle style;
  M3Carousel carousel;
  CMPWidget child;
  CMPWidget *items[] = {&child};

  CMPWidgetVTable dummy_vt;
  memset(&dummy_vt, 0, sizeof(dummy_vt));
  dummy_vt.event = test_event;
  child.vtable = &dummy_vt;

  m3_carousel_style_init(&style);
  m3_carousel_init(&carousel, &style, items, 1);

  CMPInputEvent ev = {0};
  CMPBool handled;

  ev.type = CMP_INPUT_POINTER_MOVE;
  carousel.dragging = CMP_FALSE;
  carousel.widget.vtable->event(&carousel, &ev, &handled);

  carousel.dragging = CMP_TRUE;
  ev.time_ms = 100;
  carousel.last_time_ms = 100;
  carousel.widget.vtable->event(&carousel, &ev, &handled);

  carousel.content_extent = 100;
  carousel.bounds.width = 50;
  carousel.scroll_offset = 60;
  ev.data.pointer.x = 0;
  carousel.last_pointer_x = 0;
  ev.time_ms = 1000;
  carousel.widget.vtable->event(&carousel, &ev, &handled);

  carousel.widget.vtable->destroy(&carousel);
  return 0;
}
