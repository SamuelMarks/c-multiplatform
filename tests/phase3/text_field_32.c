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
                             CMPScalar *out_width, CMPScalar *out_height,
                             CMPScalar *out_baseline) {
  if (out_width)
    *out_width = (CMPScalar)utf8_len * 10.0f;
  if (out_height)
    *out_height = 20.0f;
  if (out_baseline)
    *out_baseline = 16.0f;
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

static CMPGfxVTable g_test_gfx_vtable = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

int main(void) {
  CMPTextField field;
  CMPTextFieldStyle style;
  CMPTextBackend text_backend = {NULL, &g_test_text_vtable};
  CMPGfx gfx = {NULL, &g_test_gfx_vtable, &g_test_text_vtable};
  CMPPaintContext paint_ctx = {&gfx};
  CMPInputEvent event;
  CMPBool handled;

  CMPColor container, outline, text, label, placeholder, cursor, selection,
      handle;

  cmp_text_field_style_init(&style);
  style.is_obscured = CMP_TRUE;
  style.obscure_char = '*';
  cmp_text_field_init(&field, &text_backend, &style, NULL, "abc", 3);

  /* Invalid state colors */
  cmp_text_field_set_invalid(&field, CMP_TRUE);
  cmp_text_field_test_resolve_colors(&field, &container, &outline, &text,
                                     &label, &placeholder, &cursor, &selection,
                                     &handle);
  CMP_TEST_ASSERT(outline.r == style.error_outline_color.r);
  CMP_TEST_ASSERT(outline.g == style.error_outline_color.g);
  CMP_TEST_ASSERT(outline.b == style.error_outline_color.b);
  CMP_TEST_ASSERT(outline.a == style.error_outline_color.a);
  cmp_text_field_set_invalid(&field, CMP_FALSE);

  CMP_TEST_ASSERT(cmp_text_field_set_invalid(NULL, CMP_TRUE) ==
                  CMP_ERR_INVALID_ARGUMENT);

  /* Obscured text measurements */
  cmp_text_field_test_update_text_metrics(&field);
  CMP_TEST_ASSERT(field.obscured_len == 3);
  CMP_TEST_ASSERT(field.obscured_utf8[0] == '*');

  /* Paint */
  field.bounds.width = 100.0f;
  field.bounds.height = 100.0f;
  if (field.widget.vtable && field.widget.vtable->paint) {
    field.widget.vtable->paint(&field, &paint_ctx);
  }

  /* Focus event */
  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_FOCUS;
  if (field.widget.vtable && field.widget.vtable->event) {
    field.widget.vtable->event(&field, &event, &handled);
  }
  CMP_TEST_ASSERT(field.focused == CMP_TRUE);

  /* Blur event */
  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_BLUR;
  if (field.widget.vtable && field.widget.vtable->event) {
    field.widget.vtable->event(&field, &event, &handled);
  }
  CMP_TEST_ASSERT(field.focused == CMP_FALSE);

  /* Disable obscured */
  style.is_obscured = CMP_FALSE;
  cmp_text_field_set_style(&field, &style);
  cmp_text_field_test_update_text_metrics(&field);
  CMP_TEST_ASSERT(field.obscured_utf8 == NULL);

  if (field.widget.vtable && field.widget.vtable->destroy) {
    field.widget.vtable->destroy(&field);
  }

  return 0;
}