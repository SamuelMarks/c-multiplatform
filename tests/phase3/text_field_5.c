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

  cmp_usize val;
  CMP_TEST_EXPECT(cmp_text_field_test_offset_for_x(&field, 15.0f, &val),
                  CMP_OK);

  cmp_text_field_set_text(&field, "hello", 5);
  CMP_TEST_EXPECT(cmp_text_field_test_offset_for_x(&field, 15.0f, &val),
                  CMP_OK);

  cmp_text_field_test_set_fail_point(12u);
  CMP_TEST_EXPECT(cmp_text_field_test_offset_for_x(&field, 15.0f, &val),
                  CMP_OK);

  cmp_text_field_test_set_fail_point(3u);
  CMP_TEST_EXPECT(cmp_text_field_test_offset_for_x(&field, 15.0f, &val),
                  CMP_ERR_IO);

  CMPInputEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = CMP_INPUT_POINTER_DOWN;
  ev.data.pointer.x = 10.0f;
  ev.data.pointer.y = 10.0f;
  CMPBool handled;

  field.bounds.width = 100.0f;
  field.bounds.height = 50.0f;
  field.widget.vtable->event(&field, &ev, &handled);

  ev.type = CMP_INPUT_POINTER_UP;
  field.widget.vtable->event(&field, &ev, &handled);

  ev.type = CMP_INPUT_POINTER_MOVE;
  field.widget.vtable->event(&field, &ev, &handled);

  ev.type = CMP_INPUT_KEY_DOWN;
  ev.data.key.key_code = 127;
  field.widget.vtable->event(&field, &ev, &handled);

  ev.data.key.key_code = 8;
  field.widget.vtable->event(&field, &ev, &handled);

  ev.data.key.key_code = 2;
  field.widget.vtable->event(&field, &ev, &handled);

  ev.data.key.key_code = 3;
  field.widget.vtable->event(&field, &ev, &handled);

  ev.type = CMP_INPUT_TEXT_UTF8;
  ev.data.text_utf8.utf8 = "a";
  ev.data.text_utf8.length = 1;
  field.widget.vtable->event(&field, &ev, &handled);

  ev.type = CMP_INPUT_TEXT;
  ev.data.text.length = 1;
  ev.data.text.utf8[0] = 'b';
  field.widget.vtable->event(&field, &ev, &handled);

  field.widget.vtable->destroy(&field);
  return 0;
}
