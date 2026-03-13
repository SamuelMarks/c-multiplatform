/* clang-format off */
#include "cupertino/cupertino_mac_toolbar.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_mac_toolbar(void) {
  CupertinoMacToolbar toolbar;
  CMPTextBackend text_backend = {0};
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_mac_toolbar_init(NULL, &text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_toolbar_init(&toolbar, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_toolbar_init(&toolbar, &text_backend), CMP_OK);

  CMP_TEST_EXPECT(cupertino_mac_toolbar_add_item(NULL, "Button"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_toolbar_add_item(&toolbar, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_toolbar_add_item(&toolbar, "Back"), CMP_OK);

  CMP_TEST_EXPECT(cupertino_mac_toolbar_add_flexible_space(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_toolbar_add_flexible_space(&toolbar), CMP_OK);

  CMP_TEST_EXPECT(cupertino_mac_toolbar_add_search(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_toolbar_add_search(&toolbar), CMP_OK);

  CMP_TEST_EXPECT(cupertino_mac_toolbar_paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_toolbar_paint(&toolbar, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_toolbar_paint(&toolbar, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_mac_toolbar();
  return fails > 0 ? 1 : 0;
}
