/* clang-format off */
#include "cupertino/cupertino_switch.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_switch(void) {
  CupertinoSwitch sw;
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_switch_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_switch_init(&sw), CMP_OK);

  CMP_TEST_EXPECT(cupertino_switch_set_on(NULL, CMP_TRUE, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_switch_set_on(&sw, CMP_TRUE, CMP_FALSE), CMP_OK);
  CMP_TEST_EXPECT(sw.is_on, CMP_TRUE);

  CMP_TEST_EXPECT(cupertino_switch_update(NULL, 0.1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_switch_update(&sw, 0.1), CMP_OK);

  CMP_TEST_EXPECT(cupertino_switch_paint(NULL, &ctx), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_switch_paint(&sw, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cupertino_switch_paint(&sw, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_switch();
  return fails > 0 ? 1 : 0;
}
