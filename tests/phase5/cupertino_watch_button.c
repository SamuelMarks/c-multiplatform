#include "../phase1/test_utils.h"
#include "cupertino/cupertino_watch_button.h"
#include <stddef.h>

static int test_cupertino_watch_button(void) {
    CupertinoWatchButton button;
    CMPTextBackend text_backend = {0};
    CMPPaintContext ctx = {0};
    CMPGfx gfx = {0};
    CMPGfxVTable vtable = {0};
    
    gfx.vtable = &vtable;
    ctx.gfx = &gfx;

    CMP_TEST_EXPECT(cupertino_watch_button_init(NULL, &text_backend), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_watch_button_init(&button, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_watch_button_init(&button, &text_backend), CMP_OK);

    CMP_TEST_EXPECT(cupertino_watch_button_set_title(NULL, "Title"), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_watch_button_set_title(&button, "Title"), CMP_OK);

    CMP_TEST_EXPECT(cupertino_watch_button_paint(NULL, &ctx), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_watch_button_paint(&button, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_watch_button_paint(&button, &ctx), CMP_OK);

    return 0;
}

int main(void) {
    int fails = 0;
    fails += test_cupertino_watch_button();
    return fails > 0 ? 1 : 0;
}
