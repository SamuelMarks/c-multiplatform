#include "cupertino/cupertino_focus_ring.h"
#include "../phase1/test_utils.h"
#include <stddef.h>

static int test_cupertino_focus_ring(void) {
  CMPRect bounds = {10, 10, 100, 30};
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_focus_ring_paint(NULL, bounds, 5.0f, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Successful paint */
  CMP_TEST_EXPECT(cupertino_focus_ring_paint(&ctx, bounds, 5.0f, CMP_FALSE),
                  CMP_OK);
  CMP_TEST_EXPECT(cupertino_focus_ring_paint(&ctx, bounds, 5.0f, CMP_TRUE),
                  CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_focus_ring();
  return fails > 0 ? 1 : 0;
}
