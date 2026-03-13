/* clang-format off */
#include "cupertino/cupertino_text_field.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_text_field(void) {
  CupertinoTextField field;
  CMPTextBackend text_backend = {0};
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_text_field_init(NULL, &text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_text_field_init(&field, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_text_field_init(&field, &text_backend), CMP_OK);

  CMP_TEST_EXPECT(cupertino_text_field_set_text(NULL, "Hello"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_text_field_set_text(&field, "Hello"), CMP_OK);
  CMP_TEST_EXPECT((int)field.text_len, 5);

  CMP_TEST_EXPECT(cupertino_text_field_set_placeholder(NULL, "Enter text"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_text_field_set_placeholder(&field, "Enter text"),
                  CMP_OK);
  CMP_TEST_EXPECT((int)field.placeholder_len, 10);

  CMP_TEST_EXPECT(cupertino_text_field_update(NULL, 0.1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_text_field_update(&field, 0.1), CMP_OK);

  CMP_TEST_EXPECT(cupertino_text_field_paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_text_field_paint(&field, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_text_field_paint(&field, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_text_field();
  return fails > 0 ? 1 : 0;
}
