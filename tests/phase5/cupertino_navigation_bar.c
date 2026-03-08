#include "../phase1/test_utils.h"
#include "cupertino/cupertino_navigation_bar.h"
#include <stddef.h>

/* Mock text backend for testing */
static int CMP_CALL mock_create_font(void *text, const char *utf8_family, cmp_i32 size_px, cmp_i32 weight, CMPBool italic, CMPHandle *out_font) {
    if (out_font) out_font->id = 1;
    return CMP_OK;
}

static int CMP_CALL mock_destroy_font(void *text, CMPHandle font) {
    return CMP_OK;
}

static int CMP_CALL mock_measure_text(void *ctx, CMPHandle font, const char *utf8, cmp_usize len, cmp_u32 direction, CMPScalar *out_width, CMPScalar *out_height, CMPScalar *out_baseline) {
    if (out_width) *out_width = (CMPScalar)len * 8.0f;
    if (out_height) *out_height = 16.0f;
    if (out_baseline) *out_baseline = 12.0f;
    return CMP_OK;
}

static const CMPTextVTable mock_text_vtable = {
    mock_create_font,
    mock_destroy_font,
    mock_measure_text,
    NULL, /* draw_text */
    NULL, /* shape_text */
    NULL, /* free_layout */
    NULL  /* draw_layout */
};

static int test_cupertino_navigation_bar(void) {
    CupertinoNavigationBar bar;
    CupertinoNavigationBarStyle style;
    CMPTextBackend text_backend;
    CMPRect bounds = {0.0f, 0.0f, 320.0f, 0.0f};
    CMPRect safe_area = {0.0f, 44.0f, 0.0f, 0.0f}; /* iPhone Notch/Dynamic Island top inset */
    CupertinoButton left_btn;

    text_backend.ctx = NULL;
    text_backend.vtable = &mock_text_vtable;

    CMP_TEST_EXPECT(cupertino_navigation_bar_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cupertino_navigation_bar_style_init(&style));
    CMP_TEST_EXPECT(style.variant, CUPERTINO_NAV_BAR_STANDARD);

    CMP_TEST_EXPECT(cupertino_navigation_bar_init(NULL, &text_backend), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_navigation_bar_init(&bar, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cupertino_navigation_bar_init(&bar, &text_backend));

    CMP_TEST_EXPECT(cupertino_navigation_bar_set_title(NULL, "Title", 5), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cupertino_navigation_bar_set_title(&bar, "Title", 5));

    /* Layout Standard */
    CMP_TEST_EXPECT(cupertino_navigation_bar_layout(NULL, bounds, safe_area), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cupertino_navigation_bar_layout(&bar, bounds, safe_area));
    CMP_TEST_ASSERT(bar.bounds.height == 88.0f); /* 44 top inset + 44 base */

    /* Layout Large Title fully expanded */
    bar.style.variant = CUPERTINO_NAV_BAR_LARGE_TITLE;
    bar.scroll_offset = 0.0f;
    CMP_TEST_OK(cupertino_navigation_bar_layout(&bar, bounds, safe_area));
    CMP_TEST_ASSERT(bar.bounds.height == 140.0f); /* 44 top inset + 44 base + 52 extension */

    /* Layout Large Title fully collapsed */
    bar.scroll_offset = 52.0f;
    CMP_TEST_OK(cupertino_navigation_bar_layout(&bar, bounds, safe_area));
    CMP_TEST_ASSERT(bar.bounds.height == 88.0f); /* 44 top inset + 44 base + 0 extension */

    /* Layout with left button */
    cupertino_button_init(&left_btn, &text_backend);
    left_btn.bounds.width = 40.0f;
    left_btn.bounds.height = 30.0f;
    bar.left_item = &left_btn;
    CMP_TEST_OK(cupertino_navigation_bar_layout(&bar, bounds, safe_area));
    CMP_TEST_ASSERT(left_btn.bounds.x == 16.0f);
    CMP_TEST_ASSERT(left_btn.bounds.y == 51.0f); /* 44 inset + (44 - 30) / 2 */

    return 0;
}

int main(void) {
    if (test_cupertino_navigation_bar() != 0) return 1;
    return 0;
}
