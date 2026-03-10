/* clang-format off */
#include "cupertino/cupertino_detent_sheet.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_detent_sheet(void) {
  CupertinoDetentSheet sheet;
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_detent_sheet_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_detent_sheet_init(&sheet), CMP_OK);

  {
    CMPRect bounds = {0, 0, 400, 800};
    CMP_TEST_EXPECT(cupertino_detent_sheet_set_bounds(NULL, bounds),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_detent_sheet_set_bounds(&sheet, bounds), CMP_OK);
    CMP_TEST_EXPECT((int)sheet.detent_hidden, 800);
    CMP_TEST_EXPECT((int)sheet.detent_medium, 400);
  }

  CMP_TEST_EXPECT(cupertino_detent_sheet_set_detent(NULL, 1, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_detent_sheet_set_detent(&sheet, -1, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_detent_sheet_set_detent(&sheet, 3, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_detent_sheet_set_detent(&sheet, 1, CMP_FALSE),
                  CMP_OK);

  CMP_TEST_EXPECT(cupertino_detent_sheet_update(NULL, 0.1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_detent_sheet_update(&sheet, 0.1), CMP_OK);

  CMP_TEST_EXPECT(cupertino_detent_sheet_paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_detent_sheet_paint(&sheet, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_detent_sheet_paint(&sheet, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_detent_sheet();
  return fails > 0 ? 1 : 0;
}
