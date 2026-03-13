/* clang-format off */
#include "cupertino/cupertino_list_section.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_list_section(void) {
  CupertinoListSection section;
  CMPTextBackend text_backend = {0};
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_list_section_init(NULL, &text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_list_section_init(&section, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_list_section_init(&section, &text_backend), CMP_OK);

  CMP_TEST_EXPECT(cupertino_list_section_set_headers(NULL, "Header", "Footer"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cupertino_list_section_set_headers(&section, "Header", "Footer"), CMP_OK);
  CMP_TEST_EXPECT((int)section.header_len, 6);
  CMP_TEST_EXPECT((int)section.footer_len, 6);

  CMP_TEST_EXPECT(
      cupertino_list_section_add_row(NULL, "Title", "Value", CMP_TRUE),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cupertino_list_section_add_row(&section, NULL, "Value", CMP_TRUE),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cupertino_list_section_add_row(&section, "Title", "Value", CMP_TRUE),
      CMP_OK);
  CMP_TEST_EXPECT((int)section.row_count, 1);

  CMP_TEST_EXPECT(cupertino_list_section_paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_list_section_paint(&section, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_list_section_paint(&section, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_list_section();
  return fails > 0 ? 1 : 0;
}
