/* clang-format off */
#include "cmpc/cmp_text_field.h"
#include "test_utils.h"
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

static cmp_u32 g_create_font_calls = 0;
static int fail_create_font(void *t, const char *f, cmp_i32 s, cmp_i32 w,
                            CMPBool i, CMPHandle *o) {
  g_create_font_calls++;
  if (g_create_font_calls == 2) {
    g_create_font_calls = 0;
    return CMP_ERR_IO;
  }
  if (o) {
    o->id = 1;
    o->generation = 1;
  }
  return CMP_OK;
}

static cmp_u32 g_destroy_font_calls = 0;
static int fail_destroy_font_1(void *t, CMPHandle f) {
  g_destroy_font_calls++;
  if (g_destroy_font_calls == 1) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}
static int fail_destroy_font_2(void *t, CMPHandle f) {
  g_destroy_font_calls++;
  if (g_destroy_font_calls == 2) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

int main(void) {
  CMPTextField field;
  CMPTextFieldStyle style;
  CMPTextBackend text_backend = {NULL, &g_test_text_vtable};

  cmp_text_field_style_init(&style);
  cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0);

  CMP_TEST_EXPECT(cmp_text_field_set_style(NULL, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_set_style(&field, &style));
  {
    CMPTextFieldStyle s = style;
    s.text_style.size_px = -1;
    CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &s), CMP_ERR_RANGE);
  }
  {
    cmp_text_field_set_label(&field, "abc", 3);
    cmp_text_field_test_set_fail_point(1u);
    CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &style), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);
  }
  {
    CMPTextVTable v = g_test_text_vtable;
    v.create_font = fail_create_font;
    CMPTextBackend tb = {NULL, &v};
    field.text_backend = tb;
    CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &style), CMP_ERR_IO);
    field.text_backend = text_backend;
  }
  {
    CMPTextVTable v = g_test_text_vtable;
    v.destroy_font = fail_destroy_font_1;
    CMPTextBackend tb = {NULL, &v};
    field.text_backend = tb;
    CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &style), CMP_ERR_IO);
    field.text_backend = text_backend;
  }
  {
    CMPTextVTable v = g_test_text_vtable;
    g_destroy_font_calls = 0;
    v.destroy_font = fail_destroy_font_2;
    CMPTextBackend tb = {NULL, &v};
    field.text_backend = tb;
    CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &style), CMP_ERR_IO);
    field.text_backend = text_backend;
  }
  {
    cmp_text_field_test_set_fail_point(6u);
    CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &style), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);
  }

  CMP_TEST_EXPECT(cmp_text_field_set_selection(NULL, 0, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_selection(&field, 1, 0), CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_text_field_set_selection(&field, 0, 0));
  {
    cmp_text_field_set_text(&field, "abc", 3);
    cmp_text_field_test_set_fail_point(2u);
    CMP_TEST_EXPECT(cmp_text_field_set_selection(&field, 1, 0), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);
  }
  {
    cmp_text_field_set_text(&field, "abc", 3);
    cmp_text_field_test_set_fail_point(2u);
    CMP_TEST_EXPECT(cmp_text_field_set_selection(&field, 0, 1), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);
  }
  {
    cmp_text_field_set_text(&field, "abc", 3);
    CMP_TEST_OK(cmp_text_field_set_selection(&field, 3, 0));
    CMP_TEST_ASSERT(field.selection_start == 0);
    CMP_TEST_ASSERT(field.selection_end == 3);
    CMP_TEST_ASSERT(field.cursor == 0);
    cmp_text_field_set_text(&field, NULL, 0);
  }

  cmp_usize start, end;
  CMP_TEST_EXPECT(cmp_text_field_get_selection(NULL, &start, &end),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_get_selection(&field, NULL, &end),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_get_selection(&field, &start, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_get_selection(&field, &start, &end));

  CMP_TEST_EXPECT(cmp_text_field_set_cursor(NULL, 0), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_cursor(&field, 1), CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_text_field_set_cursor(&field, 0));
  {
    cmp_text_field_set_text(&field, "abc", 3);
    cmp_text_field_test_set_fail_point(2u);
    CMP_TEST_EXPECT(cmp_text_field_set_cursor(&field, 1), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);
  }

  CMP_TEST_EXPECT(cmp_text_field_get_cursor(NULL, &start),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_get_cursor(&field, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_get_cursor(&field, &start));

  CMP_TEST_EXPECT(cmp_text_field_set_focus(NULL, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_focus(&field, 2), CMP_ERR_RANGE);
  {
    cmp_text_field_test_set_fail_point(22u);
    CMP_TEST_EXPECT(cmp_text_field_set_focus(&field, CMP_TRUE), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);
  }

  CMPBool nr;
  CMP_TEST_EXPECT(cmp_text_field_step(NULL, 0.1f, &nr),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_step(&field, -0.1f, &nr), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_text_field_step(&field, 0.1f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  {
    cmp_text_field_set_focus(&field, CMP_TRUE);
    field.style.cursor_blink_period = 1.0f;
    CMP_TEST_OK(cmp_text_field_step(&field, 2.0f, &nr));

    field.cursor_timer = 0.4f;
    CMP_TEST_OK(cmp_text_field_step(&field, 0.2f, &nr));

    field.style.cursor_blink_period = 0.0f;
    field.cursor_visible = CMP_FALSE;
    CMP_TEST_OK(cmp_text_field_step(&field, 0.1f, &nr));

    cmp_text_field_set_focus(&field, CMP_FALSE);
    field.cursor_visible = CMP_TRUE;
    CMP_TEST_OK(cmp_text_field_step(&field, 0.1f, &nr));
  }
  {
    field.label_anim.mode = 3; /* Invalid mode */
    cmp_text_field_test_set_fail_point(6u);
    CMP_TEST_EXPECT(cmp_text_field_step(&field, 0.1f, &nr), CMP_ERR_IO);
    cmp_text_field_test_set_fail_point(0u);
    field.label_anim.mode = CMP_ANIM_MODE_NONE;
  }

  CMP_TEST_EXPECT(cmp_text_field_set_on_change(NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_set_on_change(&field, NULL, NULL));

  CMP_TEST_EXPECT(cmp_text_field_set_text(NULL, "a", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_text(&field, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);

  const char *out_text;
  CMP_TEST_EXPECT(cmp_text_field_get_text(NULL, &out_text, &start),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_get_text(&field, NULL, &start),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_get_text(&field, &out_text, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(NULL, "a", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_insert_utf8(&field, "a", 0));

  CMP_TEST_EXPECT(cmp_text_field_delete_selection(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_backspace(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_delete_forward(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_text_field_set_label(NULL, "a", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_label(&field, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_set_label(&field, NULL, 0));

  CMP_TEST_EXPECT(cmp_text_field_set_placeholder(NULL, "a", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_placeholder(&field, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_set_placeholder(&field, NULL, 0));

  field.widget.vtable->destroy(&field);
  return 0;
}
