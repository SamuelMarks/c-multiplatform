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

int main(void) {
  CMPTextField field;
  CMPTextFieldStyle style;
  CMPTextBackend text_backend = {NULL, &g_test_text_vtable};

  cmp_text_field_style_init(&style);
  cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0);

  cmp_text_field_set_text(&field, "abcde", 5);
  field.focused = CMP_TRUE;

  CMPInputEvent ev;
  memset(&ev, 0, sizeof(ev));
  CMPBool handled;

  ev.type = CMP_INPUT_KEY_DOWN;

  cmp_text_field_set_selection(&field, 1, 3);
  ev.data.key.key_code = 127;
  field.widget.vtable->event(&field, &ev, &handled);

  ev.modifiers = CMP_MOD_SHIFT;
  ev.data.key.key_code = 4;
  cmp_text_field_set_cursor(&field, 0);
  field.widget.vtable->event(&field, &ev, &handled);

  ev.modifiers = CMP_MOD_SHIFT;
  ev.data.key.key_code = 5;
  cmp_text_field_set_cursor(&field, 5);
  field.widget.vtable->event(&field, &ev, &handled);

  field.widget.vtable->destroy(&field);
  return 0;
}
