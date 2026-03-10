/* clang-format off */
#include "cupertino/cupertino_segmented_control.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_segmented_control(void) {
  CupertinoSegmentedControl control;
  CMPTextBackend text_backend = {0};
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_segmented_control_init(NULL, &text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_segmented_control_init(&control, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_segmented_control_init(&control, &text_backend),
                  CMP_OK);

  CMP_TEST_EXPECT(cupertino_segmented_control_add_segment(NULL, "Tab"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_segmented_control_add_segment(&control, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_segmented_control_add_segment(&control, "Tab 1"),
                  CMP_OK);
  CMP_TEST_EXPECT(cupertino_segmented_control_add_segment(&control, "Tab 2"),
                  CMP_OK);
  CMP_TEST_EXPECT((int)control.segment_count, 2);

  CMP_TEST_EXPECT(cupertino_segmented_control_set_selected(NULL, 1, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cupertino_segmented_control_set_selected(&control, -1, CMP_FALSE),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cupertino_segmented_control_set_selected(&control, 2, CMP_FALSE),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cupertino_segmented_control_set_selected(&control, 1, CMP_FALSE), CMP_OK);
  CMP_TEST_EXPECT((int)control.selected_index, 1);

  CMP_TEST_EXPECT(cupertino_segmented_control_update(NULL, 0.1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_segmented_control_update(&control, 0.1), CMP_OK);

  CMP_TEST_EXPECT(cupertino_segmented_control_paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_segmented_control_paint(&control, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_segmented_control_paint(&control, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_segmented_control();
  return fails > 0 ? 1 : 0;
}
