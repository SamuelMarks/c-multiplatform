#include "cmpc/cmp_text_field.h"
#include "test_utils.h"
#include <string.h>

int main(void) {
    CMPTextStyle ts;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_text_style(NULL, CMP_FALSE), CMP_ERR_INVALID_ARGUMENT);
    memset(&ts, 0, sizeof(ts));
    CMP_TEST_EXPECT(cmp_text_field_test_validate_text_style(&ts, CMP_TRUE), CMP_ERR_INVALID_ARGUMENT);
    ts.utf8_family = "test";
    ts.size_px = 0;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_text_style(&ts, CMP_FALSE), CMP_ERR_RANGE);
    ts.size_px = 12;
    ts.weight = 0;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_text_style(&ts, CMP_FALSE), CMP_ERR_RANGE);
    ts.weight = 1000;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_text_style(&ts, CMP_FALSE), CMP_ERR_RANGE);
    ts.weight = 400;
    ts.italic = 99;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_text_style(&ts, CMP_FALSE), CMP_ERR_RANGE);
    ts.italic = CMP_FALSE;
    ts.color.r = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_text_style(&ts, CMP_FALSE), CMP_ERR_RANGE);
    ts.color.r = 1.0f; ts.color.g = 1.0f; ts.color.b = 1.0f; ts.color.a = 1.0f;
    CMP_TEST_OK(cmp_text_field_test_validate_text_style(&ts, CMP_FALSE));
    
    CMPTextFieldStyle style;
    cmp_text_field_style_init(&style);
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(NULL, CMP_FALSE, CMP_FALSE), CMP_ERR_INVALID_ARGUMENT);
    
    style.outline_width = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.outline_width = 1.0f; style.corner_radius = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.corner_radius = 1.0f;
    
    style.padding_x = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.padding_x = 1.0f; style.padding_y = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.padding_y = 1.0f;
    
    style.min_height = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.min_height = 1.0f;
    
    style.label_float_offset = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.label_float_offset = 1.0f; style.label_anim_duration = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.label_anim_duration = 1.0f;
    
    style.cursor_width = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.cursor_width = 1.0f; style.cursor_blink_period = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.cursor_blink_period = 1.0f;
    
    style.handle_radius = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.handle_radius = 1.0f; style.handle_height = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.handle_height = 1.0f;
    
    style.text_style.size_px = -1;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.text_style.size_px = 12;
    
    style.label_style.size_px = -1;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.label_style.size_px = 12;
    
    
    style.container_color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.container_color.a = 1.0f;
    
    style.outline_color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.outline_color.a = 1.0f;
    
    style.focused_outline_color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.focused_outline_color.a = 1.0f;
    
    style.disabled_container_color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.disabled_container_color.a = 1.0f;
    
    style.disabled_outline_color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.disabled_outline_color.a = 1.0f;
    
    style.handle_color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.handle_color.a = 1.0f;
    
    style.disabled_text_color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.disabled_text_color.a = 1.0f;
    
    style.disabled_label_color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.disabled_label_color.a = 1.0f;
    
    style.placeholder_color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.placeholder_color.a = 1.0f;
    
    style.disabled_placeholder_color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.disabled_placeholder_color.a = 1.0f;
    
    style.cursor_color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.cursor_color.a = 1.0f;
    
    style.selection_color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_style(&style, CMP_FALSE, CMP_FALSE), CMP_ERR_RANGE);
    style.selection_color.a = 1.0f;

    return CMP_OK;
}
