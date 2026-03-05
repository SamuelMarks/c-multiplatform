#include "cmpc/cmp_text_field.h"
#include "test_utils.h"

static int test_create_font(void *text, const char *utf8_family,
                            cmp_i32 size_px, cmp_i32 weight, CMPBool italic,
                            CMPHandle *out_font) {
  if (out_font) {
    out_font->id = 1;
    out_font->generation = 1;
  }
  return CMP_OK;
}
static int test_destroy_font(void *text, CMPHandle font) { return CMP_OK; }
static int test_measure_text(void *text, CMPHandle font, const char *utf8,
                             cmp_usize utf8_len, cmp_u32 base_direction,
                             CMPScalar *out_width, CMPScalar *out_height,
                             CMPScalar *out_baseline) {
  if (out_width) *out_width = (CMPScalar)utf8_len * 10.0f;
  if (out_height) *out_height = 20.0f;
  if (out_baseline) *out_baseline = 16.0f;
  return CMP_OK;
}
static int test_draw_text(void *text, CMPHandle font, const char *utf8,
                          cmp_usize utf8_len, cmp_u32 base_direction,
                          CMPScalar x, CMPScalar y, CMPColor color) {
  return CMP_OK;
}

static CMPTextVTable g_test_text_vtable = {
    test_create_font, test_destroy_font, test_measure_text, test_draw_text, NULL, NULL, NULL};

int main(void) {
    CMPTextField field;
    CMPTextFieldStyle style;
    CMPTextBackend text_backend = {NULL, &g_test_text_vtable};
    
    cmp_text_field_style_init(&style);
    cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0);
    
    CMP_TEST_EXPECT(cmp_text_field_set_style(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_set_style(&field, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_field_set_style(&field, &style));
    
    CMP_TEST_EXPECT(cmp_text_field_set_selection(NULL, 0, 0), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_set_selection(&field, 1, 0), CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_text_field_set_selection(&field, 0, 0));
    
    cmp_usize start, end;
    CMP_TEST_EXPECT(cmp_text_field_get_selection(NULL, &start, &end), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_get_selection(&field, NULL, &end), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_get_selection(&field, &start, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_field_get_selection(&field, &start, &end));
    
    CMP_TEST_EXPECT(cmp_text_field_set_cursor(NULL, 0), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_set_cursor(&field, 1), CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_text_field_set_cursor(&field, 0));
    
    CMP_TEST_EXPECT(cmp_text_field_get_cursor(NULL, &start), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_get_cursor(&field, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_field_get_cursor(&field, &start));
    
    CMP_TEST_EXPECT(cmp_text_field_set_focus(NULL, CMP_FALSE), CMP_ERR_INVALID_ARGUMENT);
    
    CMPBool nr;
    CMP_TEST_EXPECT(cmp_text_field_step(NULL, 0.1f, &nr), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_step(&field, -0.1f, &nr), CMP_ERR_RANGE);
    CMP_TEST_EXPECT(cmp_text_field_step(&field, 0.1f, NULL), CMP_ERR_INVALID_ARGUMENT);
    
    CMP_TEST_EXPECT(cmp_text_field_set_on_change(NULL, NULL, NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_field_set_on_change(&field, NULL, NULL));
    
    CMP_TEST_EXPECT(cmp_text_field_set_text(NULL, "a", 1), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_set_text(&field, NULL, 1), CMP_ERR_INVALID_ARGUMENT);
    
    const char *out_text;
    CMP_TEST_EXPECT(cmp_text_field_get_text(NULL, &out_text, &start), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_get_text(&field, NULL, &start), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_get_text(&field, &out_text, NULL), CMP_ERR_INVALID_ARGUMENT);
    
    CMP_TEST_EXPECT(cmp_text_field_insert_utf8(NULL, "a", 1), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, NULL, 1), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_field_insert_utf8(&field, "a", 0)); 
    
    CMP_TEST_EXPECT(cmp_text_field_delete_selection(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_backspace(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_delete_forward(NULL), CMP_ERR_INVALID_ARGUMENT);
    
    CMP_TEST_EXPECT(cmp_text_field_set_label(NULL, "a", 1), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_set_label(&field, NULL, 1), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_field_set_label(&field, NULL, 0));
    
    CMP_TEST_EXPECT(cmp_text_field_set_placeholder(NULL, "a", 1), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_set_placeholder(&field, NULL, 1), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_field_set_placeholder(&field, NULL, 0));
    
    field.widget.vtable->destroy(&field);
    return 0;
}
