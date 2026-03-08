#include "../phase1/test_utils.h"
#include "cupertino/cupertino_slider.h"
#include <stddef.h>

static int test_cupertino_slider(void) {
    CupertinoSlider slider;
    CMPPaintContext ctx = {0};
    CMPGfx gfx = {0};
    CMPGfxVTable vtable = {0};
    
    gfx.vtable = &vtable;
    ctx.gfx = &gfx;

    CMP_TEST_EXPECT(cupertino_slider_init(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_slider_init(&slider), CMP_OK);

    CMP_TEST_EXPECT(cupertino_slider_set_value(NULL, 0.5f), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_slider_set_value(&slider, 0.5f), CMP_OK);
    CMP_TEST_EXPECT((int)(slider.value * 10.0f), 5);

    CMP_TEST_EXPECT(cupertino_slider_paint(NULL, &ctx), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_slider_paint(&slider, NULL), CMP_ERR_INVALID_ARGUMENT);
    
    CMP_TEST_EXPECT(cupertino_slider_paint(&slider, &ctx), CMP_OK);

    return 0;
}

int main(void) {
    int fails = 0;
    fails += test_cupertino_slider();
    return fails > 0 ? 1 : 0;
}
