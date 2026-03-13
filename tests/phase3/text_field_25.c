/* clang-format off */
#include "cmpc/cmp_text_field.h"
#include "test_utils.h"
#include <string.h>
/* clang-format on */

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
                             CMPScalar *w, CMPScalar *h, CMPScalar *baseline) {
  if (w)
    *w = 10.0f;
  if (h)
    *h = 10.0f;
  if (baseline)
    *baseline = 8.0f;
  return CMP_OK;
}
static int test_draw_text(void *text, CMPHandle font, const char *utf8,
                          cmp_usize utf8_len, cmp_u32 base_direction,
                          CMPScalar x, CMPScalar y, CMPColor color) {
  return CMP_OK;
}
static const CMPTextVTable mock_vtable = {test_create_font,
                                          test_destroy_font,
                                          test_measure_text,
                                          test_draw_text,
                                          NULL,
                                          NULL,
                                          NULL};

static int custom_on_change(void *ctx, CMPTextField *field, const char *utf8,
                            cmp_usize utf8_len) {
  return CMP_ERR_IO;
}

int main(void) {
  CMPTextField field;
  CMPTextFieldStyle style;
  CMPTextBackend text_backend;

  text_backend.ctx = NULL;
  text_backend.vtable = &mock_vtable;

  cmp_text_field_style_init(&style);
  cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0);

  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(NULL, "a", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  {
    cmp_text_field_test_set_fail_point(1u);
    CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, "a", 1), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);
  }
  {
    cmp_text_field_set_text(&field, "abc", 3);
    cmp_text_field_test_set_overflow_fail_after(1u);
    CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, "a", 1),
                    CMP_ERR_OVERFLOW);
    cmp_text_field_test_set_overflow_fail_after(0u);
  }
  {
    cmp_text_field_set_text(&field, "abc", 3);
    cmp_text_field_test_set_overflow_fail_after(2u);
    CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, "a", 1),
                    CMP_ERR_OVERFLOW);
    cmp_text_field_test_set_overflow_fail_after(0u);
  }
  {
    cmp_text_field_set_text(&field, "abc", 3);
    field.cursor = 1;
    cmp_text_field_test_set_fail_point(2u);
    CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, "a", 1), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);
  }
  {
    field.cursor = 100;
    CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, "a", 1), CMP_ERR_RANGE);
    field.cursor = 0;
  }
  {
    cmp_text_field_set_text(&field, "abc", 3);
    field.selection_start = 0;
    field.selection_end = 1;
    cmp_text_field_test_set_fail_point(22u);
    CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, "a", 1), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);
  }
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);

  cmp_text_field_test_set_fail_point(10u);
  cmp_text_field_insert_utf8(&field, "a", 1);

  cmp_text_field_test_set_fail_point(7u);
  {
    field.on_change = custom_on_change;
    CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, "a", 1), CMP_ERR_IO);
    field.on_change = NULL;
  }
  CMP_TEST_EXPECT(
      cmp_text_field_insert_utf8(&field,
                                 "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                 "aaaaaaaaaaaaaaaaaaaaaaaaaaa",
                                 70),
      CMP_ERR_IO);

  cmp_usize val;
  cmp_text_field_test_add_overflow((cmp_usize)-1, 1, &val);

  return CMP_OK;
}
