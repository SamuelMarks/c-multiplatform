#include "../phase1/test_utils.h"
#include "cupertino/cupertino_tab_bar.h"
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

static int test_cupertino_tab_bar(void) {
    CupertinoTabBar bar;
    CupertinoTabBarStyle style;
    CMPTextBackend text_backend;
    CMPRect bounds = {0.0f, 0.0f, 320.0f, 480.0f};
    CMPRect safe_area = {0.0f, 0.0f, 0.0f, 34.0f}; /* iPhone Home Indicator bottom inset */
    CupertinoTabBarItem items[2] = {0};

    text_backend.ctx = NULL;
    text_backend.vtable = &mock_text_vtable;

    CMP_TEST_EXPECT(cupertino_tab_bar_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cupertino_tab_bar_style_init(&style));

    CMP_TEST_EXPECT(cupertino_tab_bar_init(NULL, &text_backend), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_tab_bar_init(&bar, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cupertino_tab_bar_init(&bar, &text_backend));

    items[0].title_utf8 = "Home";
    items[0].title_len = 4;
    items[1].title_utf8 = "Settings";
    items[1].title_len = 8;

    CMP_TEST_EXPECT(cupertino_tab_bar_set_items(NULL, items, 2), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cupertino_tab_bar_set_items(&bar, items, CUPERTINO_TAB_BAR_MAX_ITEMS + 1), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cupertino_tab_bar_set_items(&bar, items, 2));

    CMP_TEST_EXPECT(bar.item_count, 2);

    /* Layout */
    CMP_TEST_EXPECT(cupertino_tab_bar_layout(NULL, bounds, safe_area), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cupertino_tab_bar_layout(&bar, bounds, safe_area));
    
    CMP_TEST_ASSERT(bar.bounds.height == 49.0f + 34.0f);
    CMP_TEST_ASSERT(bar.bounds.y == 480.0f - (49.0f + 34.0f));

    return 0;
}

int main(void) {
    if (test_cupertino_tab_bar() != 0) return 1;
    return 0;
}
