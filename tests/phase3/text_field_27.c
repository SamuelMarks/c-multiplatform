#include "cmpc/cmp_text_field.h"
#include "test_utils.h"

int main(void) {
    CMPColor color;
    color.r = 0.5f; color.g = 0.5f; color.b = 0.5f; color.a = 0.5f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_color(NULL), CMP_ERR_INVALID_ARGUMENT);
    color.r = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_color(&color), CMP_ERR_RANGE);
    color.r = 0.5f; color.g = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_color(&color), CMP_ERR_RANGE);
    color.g = 0.5f; color.b = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_color(&color), CMP_ERR_RANGE);
    color.b = 0.5f; color.a = -1.0f;
    CMP_TEST_EXPECT(cmp_text_field_test_validate_color(&color), CMP_ERR_RANGE);
    
    CMP_TEST_EXPECT(cmp_text_field_test_color_set(NULL, 0, 0, 0, 0), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_test_color_set(&color, -1.0f, 0, 0, 0), CMP_ERR_RANGE);
    CMP_TEST_EXPECT(cmp_text_field_test_color_set(&color, 0, -1.0f, 0, 0), CMP_ERR_RANGE);
    CMP_TEST_EXPECT(cmp_text_field_test_color_set(&color, 0, 0, -1.0f, 0), CMP_ERR_RANGE);
    CMP_TEST_EXPECT(cmp_text_field_test_color_set(&color, 0, 0, 0, -1.0f), CMP_ERR_RANGE);
    
    return CMP_OK;
}
