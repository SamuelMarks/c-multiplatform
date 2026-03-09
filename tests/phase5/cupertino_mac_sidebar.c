#include "cupertino/cupertino_mac_sidebar.h"
#include "../phase1/test_utils.h"
#include <stddef.h>

static int test_cupertino_mac_sidebar(void) {
  CupertinoMacSidebar sidebar;
  CMPTextBackend text_backend = {0};
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_mac_sidebar_init(NULL, &text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_sidebar_init(&sidebar, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_sidebar_init(&sidebar, &text_backend), CMP_OK);

  CMP_TEST_EXPECT(cupertino_mac_sidebar_add_item(NULL, "Item", CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_sidebar_add_item(&sidebar, NULL, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_sidebar_add_item(&sidebar, "Item", CMP_FALSE),
                  CMP_OK);

  CMP_TEST_EXPECT(cupertino_mac_sidebar_set_selected(NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_sidebar_set_selected(&sidebar, -1), CMP_OK);
  CMP_TEST_EXPECT(cupertino_mac_sidebar_set_selected(&sidebar, 0), CMP_OK);
  CMP_TEST_EXPECT(cupertino_mac_sidebar_set_selected(&sidebar, 2),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cupertino_mac_sidebar_paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_sidebar_paint(&sidebar, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_mac_sidebar_paint(&sidebar, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_mac_sidebar();
  return fails > 0 ? 1 : 0;
}
