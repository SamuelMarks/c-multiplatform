/* clang-format off */
#include "cupertino/cupertino_picker.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_picker(void) {
  CupertinoPicker picker;
  CMPTextBackend text_backend = {0};
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_picker_init(NULL, &text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_picker_init(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_picker_init(&picker, &text_backend), CMP_OK);

  CMP_TEST_EXPECT(cupertino_picker_add_item(NULL, "Item 1"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_picker_add_item(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_picker_add_item(&picker, "Item 1"), CMP_OK);
  CMP_TEST_EXPECT(cupertino_picker_add_item(&picker, "Item 2"), CMP_OK);
  CMP_TEST_EXPECT((int)picker.item_count, 2);

  CMP_TEST_EXPECT(cupertino_picker_set_selected(NULL, 1, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_picker_set_selected(&picker, -1, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_picker_set_selected(&picker, 2, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_picker_set_selected(&picker, 1, CMP_FALSE), CMP_OK);

  CMP_TEST_EXPECT(cupertino_picker_update(NULL, 0.1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_picker_update(&picker, 0.1), CMP_OK);

  CMP_TEST_EXPECT(cupertino_picker_paint(NULL, &ctx), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_picker_paint(&picker, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_picker_paint(&picker, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_picker();
  return fails > 0 ? 1 : 0;
}
