/* clang-format off */
#include "cupertino/cupertino_activity_indicator.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_activity_indicator(void) {
  CupertinoActivityIndicator indicator;
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_activity_indicator_init(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_activity_indicator_init(&indicator), CMP_OK);

  CMP_TEST_EXPECT(cupertino_activity_indicator_start_animating(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_activity_indicator_start_animating(&indicator),
                  CMP_OK);
  CMP_TEST_EXPECT(indicator.is_animating, CMP_TRUE);

  CMP_TEST_EXPECT(cupertino_activity_indicator_update(NULL, 0.1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_activity_indicator_update(&indicator, 0.1), CMP_OK);

  CMP_TEST_EXPECT(cupertino_activity_indicator_stop_animating(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_activity_indicator_stop_animating(&indicator),
                  CMP_OK);
  CMP_TEST_EXPECT(indicator.is_animating, CMP_FALSE);

  CMP_TEST_EXPECT(cupertino_activity_indicator_paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_activity_indicator_paint(&indicator, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cupertino_activity_indicator_paint(&indicator, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_activity_indicator();
  return fails > 0 ? 1 : 0;
}
