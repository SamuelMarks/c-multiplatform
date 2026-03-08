#include "../phase1/test_utils.h"
#include "cupertino/cupertino_date_picker.h"
#include <stddef.h>

static int test_cupertino_date_picker(void) {
    CupertinoDatePicker picker;
    CMPTextBackend text_backend = {0};
    CMPPaintContext ctx = {0};
    CMPGfx gfx = {0};
    CMPGfxVTable vtable = {0};
    
    gfx.vtable = &vtable;
    ctx.gfx = &gfx;

    CMP_TEST_EXPECT(cupertino_date_picker_init(NULL, &text_backend), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_date_picker_init(&picker, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_date_picker_init(&picker, &text_backend), CMP_OK);

    CMP_TEST_EXPECT(cupertino_date_picker_set_datetime(NULL, 2026, 3, 8, 12, 0), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_date_picker_set_datetime(&picker, 2026, 3, 8, 12, 0), CMP_OK);

    CMP_TEST_EXPECT(cupertino_date_picker_paint(NULL, &ctx), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_date_picker_paint(&picker, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_date_picker_paint(&picker, &ctx), CMP_OK);

    return 0;
}

int main(void) {
    int fails = 0;
    fails += test_cupertino_date_picker();
    return fails > 0 ? 1 : 0;
}
