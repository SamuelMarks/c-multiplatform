/* clang-format off */
#include "cmpc/cmp_text_field.h"
#include "test_utils.h"
#include <string.h>
/* clang-format on */

static int dummy_validate(void *ctx, CMPTextField *field, const char *utf8,
                          cmp_usize utf8_len, CMPBool *out_is_invalid) {
  CMPBool *called = (CMPBool *)ctx;
  (void)field;
  (void)utf8;
  (void)utf8_len;
  *called = CMP_TRUE;
  /* If utf8 equals "bad", it's invalid */
  if (utf8_len == 3 && memcmp(utf8, "bad", 3) == 0) {
    *out_is_invalid = CMP_TRUE;
  } else {
    *out_is_invalid = CMP_FALSE;
  }
  return CMP_OK;
}

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
  return CMP_OK;
}
static int test_draw_text(void *text, CMPHandle font, const char *utf8,
                          cmp_usize utf8_len, cmp_u32 base_direction,
                          CMPScalar x, CMPScalar y, CMPColor color) {
  return CMP_OK;
}
static CMPTextVTable g_test_text_vtable = {test_create_font,
                                           test_destroy_font,
                                           test_measure_text,
                                           test_draw_text,
                                           NULL,
                                           NULL,
                                           NULL};
static int test_text_field_validation_pattern(void) {
  CMPTextField field;
  CMPTextFieldStyle style;
  CMPTextBackend tb;
  CMPBool is_invalid;
  char text[] = "http://test";

  memset(&tb, 0, sizeof(tb));
  tb.vtable = &g_test_text_vtable;

  cmp_text_field_style_init(&style);
  cmp_text_field_init(&field, &tb, &style, NULL, NULL, 0);

  CMP_TEST_EXPECT(cmp_text_field_set_validation_pattern(NULL, "^http"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_set_validation_pattern(&field, "^http"));

  cmp_text_field_set_text(&field, text, 11);
  cmp_text_field_validate(&field, &is_invalid);
  CMP_TEST_ASSERT(is_invalid == CMP_FALSE);

  cmp_text_field_set_text(&field, "ftp://test", 10);
  cmp_text_field_validate(&field, &is_invalid);
  CMP_TEST_ASSERT(is_invalid == CMP_TRUE);

  cmp_text_field_set_validation_pattern(&field, "test$");
  cmp_text_field_set_text(&field, "hello test", 10);
  cmp_text_field_validate(&field, &is_invalid);
  CMP_TEST_ASSERT(is_invalid == CMP_FALSE);

  cmp_text_field_set_text(&field, "test hello", 10);
  cmp_text_field_validate(&field, &is_invalid);
  CMP_TEST_ASSERT(is_invalid == CMP_TRUE);

  cmp_text_field_set_validation_pattern(&field, "^exact$");
  cmp_text_field_set_text(&field, "exact", 5);
  cmp_text_field_validate(&field, &is_invalid);
  CMP_TEST_ASSERT(is_invalid == CMP_FALSE);

  cmp_text_field_set_text(&field, "exact2", 6);
  cmp_text_field_validate(&field, &is_invalid);
  CMP_TEST_ASSERT(is_invalid == CMP_TRUE);

  cmp_text_field_set_validation_pattern(&field, "substring");
  cmp_text_field_set_text(&field, "has substring yes", 17);
  cmp_text_field_validate(&field, &is_invalid);
  CMP_TEST_ASSERT(is_invalid == CMP_FALSE);

  cmp_text_field_set_text(&field, "no sub", 6);
  cmp_text_field_validate(&field, &is_invalid);
  CMP_TEST_ASSERT(is_invalid == CMP_TRUE);

  return 0;
}

int main(void) {
  CMPTextField field;
  CMPTextFieldStyle style;
  CMPTextBackend text_backend = {NULL, &g_test_text_vtable};
  CMPBool invalid = CMP_FALSE;
  CMPBool called = CMP_FALSE;

  cmp_text_field_style_init(&style);
  cmp_text_field_init(&field, &text_backend, &style, NULL, "", 0);

  /* Test bad inputs */
  CMP_TEST_EXPECT(cmp_text_field_set_on_validate(NULL, dummy_validate, &called),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_constraints(NULL, CMP_TRUE, 0, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_validate(NULL, &invalid),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test widget_is_text_field */
  CMP_TEST_ASSERT(cmp_widget_is_text_field(NULL) == CMP_FALSE);
  CMP_TEST_ASSERT(cmp_widget_is_text_field(&field.widget) == CMP_TRUE);
  {
    CMPWidget fake;
    fake.vtable = NULL;
    CMP_TEST_ASSERT(cmp_widget_is_text_field(&fake) == CMP_FALSE);
  }

  /* Test basic required constraints */
  cmp_text_field_set_constraints(&field, CMP_TRUE, 0, 0);
  cmp_text_field_validate(&field, &invalid);
  CMP_TEST_ASSERT(invalid == CMP_TRUE);
  CMP_TEST_ASSERT(field.is_invalid == CMP_TRUE);

  cmp_text_field_set_text(&field, "ok", 2);
  cmp_text_field_validate(&field, &invalid);
  CMP_TEST_ASSERT(invalid == CMP_FALSE);
  CMP_TEST_ASSERT(field.is_invalid == CMP_FALSE);

  /* Test min length */
  cmp_text_field_set_constraints(&field, CMP_TRUE, 4, 0);
  cmp_text_field_validate(&field, &invalid);
  CMP_TEST_ASSERT(invalid == CMP_TRUE);

  cmp_text_field_set_text(&field, "good", 4);
  cmp_text_field_validate(&field, &invalid);
  CMP_TEST_ASSERT(invalid == CMP_FALSE);

  /* Test max length */
  cmp_text_field_set_constraints(&field, CMP_TRUE, 0, 3);
  cmp_text_field_set_text(&field, "too long", 8);
  cmp_text_field_validate(&field, &invalid);
  CMP_TEST_ASSERT(invalid == CMP_TRUE);

  cmp_text_field_set_text(&field, "ok", 2);
  cmp_text_field_validate(&field, &invalid);
  CMP_TEST_ASSERT(invalid == CMP_FALSE);

  /* Test custom validate callback */
  cmp_text_field_set_on_validate(&field, dummy_validate, &called);

  cmp_text_field_set_text(&field, "bad", 3);
  cmp_text_field_validate(&field, &invalid);
  CMP_TEST_ASSERT(called == CMP_TRUE);
  CMP_TEST_ASSERT(invalid == CMP_TRUE);

  called = CMP_FALSE;
  cmp_text_field_set_text(&field, "ok", 2);
  cmp_text_field_validate(&field, &invalid);
  CMP_TEST_ASSERT(called == CMP_TRUE);
  CMP_TEST_ASSERT(invalid == CMP_FALSE);

  if (field.widget.vtable && field.widget.vtable->destroy) {
    field.widget.vtable->destroy(&field);
  }

  return 0;
}
