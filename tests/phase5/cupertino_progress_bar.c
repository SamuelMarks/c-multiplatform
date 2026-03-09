#include "cupertino/cupertino_progress_bar.h"
#include "../phase1/test_utils.h"
#include <stddef.h>

static int test_cupertino_progress_bar(void) {
  CupertinoProgressBar bar;
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_progress_bar_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_progress_bar_init(&bar), CMP_OK);

  CMP_TEST_EXPECT(cupertino_progress_bar_set_progress(NULL, 0.5f, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_progress_bar_set_progress(&bar, 0.5f, CMP_FALSE),
                  CMP_OK);
  CMP_TEST_EXPECT((int)(bar.progress * 10.0f), 5);

  CMP_TEST_EXPECT(cupertino_progress_bar_update(NULL, 0.1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_progress_bar_update(&bar, 0.1), CMP_OK);

  CMP_TEST_EXPECT(cupertino_progress_bar_paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_progress_bar_paint(&bar, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_progress_bar_paint(&bar, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_progress_bar();
  return fails > 0 ? 1 : 0;
}
