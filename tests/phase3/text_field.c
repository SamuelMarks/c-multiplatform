#include "cmpc/cmp_text_field.h"
#include "test_utils.h"
#include <string.h>

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

static int test_style(void) {
    CMPTextFieldStyle style;
    CMP_TEST_EXPECT(cmp_text_field_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_field_style_init(&style));
    return CMP_OK;
}

static int test_init(void) {
    CMPTextField field;
    CMPTextFieldStyle style;
    CMPTextBackend text_backend = {NULL, &g_test_text_vtable};
    cmp_text_field_style_init(&style);
    
    CMP_TEST_EXPECT(cmp_text_field_init(NULL, &text_backend, &style, NULL, NULL, 0), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0));
    
    CMP_TEST_OK(cmp_text_field_set_text(&field, "Hello", 5));
    const char *text;
    cmp_usize len;
    CMP_TEST_OK(cmp_text_field_get_text(&field, &text, &len));
    CMP_TEST_ASSERT(len == 5);
    CMP_TEST_ASSERT(strcmp(text, "Hello") == 0);
    
    CMP_TEST_OK(cmp_text_field_insert_utf8(&field, " World", 6));
    CMP_TEST_OK(cmp_text_field_get_text(&field, &text, &len));
    CMP_TEST_ASSERT(len == 11);
    CMP_TEST_ASSERT(strcmp(text, "Hello World") == 0);
    
    CMP_TEST_OK(cmp_text_field_set_selection(&field, 5, 11));
    CMP_TEST_OK(cmp_text_field_delete_selection(&field));
    CMP_TEST_OK(cmp_text_field_get_text(&field, &text, &len));
    CMP_TEST_ASSERT(len == 5);
    CMP_TEST_ASSERT(strcmp(text, "Hello") == 0);
    
    CMP_TEST_OK(cmp_text_field_set_cursor(&field, 5));
    CMP_TEST_OK(cmp_text_field_backspace(&field));
    CMP_TEST_OK(cmp_text_field_get_text(&field, &text, &len));
    CMP_TEST_ASSERT(len == 4);
    CMP_TEST_ASSERT(strcmp(text, "Hell") == 0);
    
    CMP_TEST_OK(cmp_text_field_set_cursor(&field, 0));
    CMP_TEST_OK(cmp_text_field_delete_forward(&field));
    CMP_TEST_OK(cmp_text_field_get_text(&field, &text, &len));
    CMP_TEST_ASSERT(len == 3);
    CMP_TEST_ASSERT(strcmp(text, "ell") == 0);
    
    field.widget.vtable->destroy(&field);
    return CMP_OK;
}

static int test_interactions(void) {
    CMPTextField field;
    CMPTextFieldStyle style;
    CMPTextBackend text_backend = {NULL, &g_test_text_vtable};
    cmp_text_field_style_init(&style);
    cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0);
    
    CMP_TEST_OK(cmp_text_field_set_label(&field, "Label", 5));
    CMP_TEST_OK(cmp_text_field_set_placeholder(&field, "Placeholder", 11));
    
    CMP_TEST_OK(cmp_text_field_set_focus(&field, CMP_TRUE));
    CMP_TEST_ASSERT(field.focused == CMP_TRUE);
    
    CMPBool needs_redraw;
    CMP_TEST_OK(cmp_text_field_step(&field, 0.1f, &needs_redraw));
    
    field.widget.vtable->destroy(&field);
    return CMP_OK;
}

int main(void) {
    CMP_TEST_ASSERT(test_style() == CMP_OK);
    CMP_TEST_ASSERT(test_init() == CMP_OK);
    CMP_TEST_ASSERT(test_interactions() == CMP_OK);
    return 0;
}
