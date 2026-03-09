#include "cupertino/cupertino_stepper.h"
#include "../phase1/test_utils.h"
#include <stddef.h>

static int test_cupertino_stepper(void) {
  CupertinoStepper stepper;
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_stepper_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_stepper_init(&stepper), CMP_OK);

  CMP_TEST_EXPECT(cupertino_stepper_set_value(NULL, 5.0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_stepper_set_value(&stepper, 5.0), CMP_OK);
  CMP_TEST_EXPECT((int)stepper.value, 5);

  CMP_TEST_EXPECT(cupertino_stepper_paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_stepper_paint(&stepper, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_stepper_paint(&stepper, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_stepper();
  return fails > 0 ? 1 : 0;
}
