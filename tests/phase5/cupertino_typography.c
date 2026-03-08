#include "../phase1/test_utils.h"
#include "cupertino/cupertino_typography.h"
#include <stddef.h>

static int test_cupertino_typography(void) {
    CupertinoTypographyScale scale;
    CMPTextStyle style;
    CMPColor color = {1.0f, 0.0f, 0.0f, 1.0f};

    CMP_TEST_EXPECT(cupertino_typography_scale_init(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cupertino_typography_scale_init(&scale));

    CMP_TEST_EXPECT(cupertino_typography_get_style(NULL, CUPERTINO_TYPOGRAPHY_BODY, &style), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_typography_get_style(&scale, CUPERTINO_TYPOGRAPHY_BODY, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_typography_get_style(&scale, (CupertinoTypographyStyle)999, &style), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_typography_get_style(&scale, (CupertinoTypographyStyle)-1, &style), CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cupertino_typography_get_style(&scale, CUPERTINO_TYPOGRAPHY_LARGE_TITLE, &style));
    CMP_TEST_EXPECT(style.size_px, 34);

    CMP_TEST_EXPECT(cupertino_typography_scale_set_color(NULL, color), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cupertino_typography_scale_set_color(&scale, color));

    CMP_TEST_OK(cupertino_typography_get_style(&scale, CUPERTINO_TYPOGRAPHY_BODY, &style));
    CMP_TEST_EXPECT((int)style.color.r, 1);

    return 0;
}

int main(void) {
    if (test_cupertino_typography() != 0) return 1;
    return 0;
}
