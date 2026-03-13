/* clang-format off */
#include "cupertino/cupertino_mac_traffic_lights.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_mac_traffic_lights(void) {
  CupertinoMacTrafficLights lights;
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_mac_traffic_lights_init(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_traffic_lights_init(&lights), CMP_OK);

  CMP_TEST_EXPECT(cupertino_mac_traffic_lights_paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_traffic_lights_paint(&lights, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_traffic_lights_paint(&lights, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_mac_traffic_lights();
  return fails > 0 ? 1 : 0;
}
