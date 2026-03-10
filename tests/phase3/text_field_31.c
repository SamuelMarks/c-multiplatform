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
static int test_pop_clip(void *ctx) {
  static int call = 0;
  if (call++ == 1)
    return CMP_ERR_IO;
  return CMP_OK;
}
static int test_destroy_font(void *text, CMPHandle font) {
  if (font.id == 99)
    return CMP_ERR_IO;
  return CMP_OK;
}
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

static int test_draw_rect(void *ctx, const CMPRect *rect, CMPColor color,
                          CMPScalar corner_radius) {
  if (color.r == 0.1f)
    return CMP_ERR_IO;   /* fail handle 1 */
  if (color.r == 0.2f) { /* fail handle 2 */
    static int call = 0;
    if (call++ == 1)
      return CMP_ERR_IO;
  }
  return CMP_OK;
}
static int test_draw_line(void *ctx, CMPScalar x1, CMPScalar y1, CMPScalar x2,
                          CMPScalar y2, CMPColor color, CMPScalar thickness) {
  return CMP_OK;
}
static int test_draw_path(void *ctx, const CMPPath *path, CMPColor color) {
  return CMP_OK;
}
static int test_push_clip(void *ctx, const CMPRect *rect) { return CMP_OK; }
static int test_set_transform(void *ctx, const CMPMat3 *transform) {
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
    NULL,           NULL,           NULL,
    test_draw_rect, test_draw_line, test_draw_path,
    test_push_clip, test_pop_clip,  test_set_transform,
};

int main(void) {
  CMPTextField field;
  CMPTextFieldStyle style;
  CMPTextBackend text_backend = {NULL, &g_test_text_vtable};
  CMPGfx gfx = {NULL, &g_test_gfx_vtable, &g_test_text_vtable};
  CMPPaintContext paint_ctx = {&gfx};

  cmp_text_field_style_init(&style);
  cmp_text_field_init(&field, &text_backend, &style, NULL, NULL, 0);

  /* Setup for line 1386: content.width < 0 */
  field.bounds.width = 0.0f;
  field.bounds.height = 0.0f;
  style.padding_x = 5.0f;
  style.padding_y = 5.0f;
  if (field.widget.vtable && field.widget.vtable->paint) {
    field.widget.vtable->paint(&field, &paint_ctx);
  }

  /* Setup for line 1428, 1433, 1436: selection_rect.width = -1.0f */
  field.bounds.width = 100.0f;
  field.bounds.height = 100.0f;
  cmp_text_field_set_text(&field, "hello", 5);
  cmp_text_field_set_selection(&field, 1, 3);
  style.selection_color.a = 1.0f;
  cmp_text_field_test_set_fail_point(
      19u); /* CMP_TEXT_FIELD_TEST_FAIL_SELECTION_WIDTH_NEGATIVE */
  if (field.widget.vtable && field.widget.vtable->paint) {
    field.widget.vtable->paint(&field, &paint_ctx);
  }
  cmp_text_field_test_set_fail_point(0u);

  /* Setup for line 1549: handle_height < handle_radius * 2 */
  style.handle_radius = 5.0f;
  style.handle_height = 0.0f;
  style.handle_color.a = 1.0f;
  if (field.widget.vtable && field.widget.vtable->paint) {
    field.widget.vtable->paint(&field, &paint_ctx);
  }

  /* Line 1555: measure prefix start fails */
  cmp_text_field_test_set_fail_point(8u); /* MEASURE_PREFIX */
  if (field.widget.vtable && field.widget.vtable->paint) {
    field.widget.vtable->paint(&field, &paint_ctx);
  }
  cmp_text_field_test_set_fail_point(0u);

  /* Line 1575: draw_rect fails for start handle */
  style.handle_color.r = 0.1f;
  if (field.widget.vtable && field.widget.vtable->paint) {
    field.widget.vtable->paint(&field, &paint_ctx);
  }

  /* Line 1582: draw_rect fails for end handle */
  style.handle_color.r = 0.2f;
  if (field.widget.vtable && field.widget.vtable->paint) {
    field.widget.vtable->paint(&field, &paint_ctx);
  }

  /* Line 1590: pop_clip fails */
  style.handle_color.r = 0.3f;
  if (field.widget.vtable && field.widget.vtable->paint) {
    field.widget.vtable->paint(&field, &paint_ctx);
  }

  /* Line 1727: insert fails event sync_label */
  {
    CMPInputEvent event;
    CMPBool handled;
    memset(&event, 0, sizeof(event));
    event.type = CMP_INPUT_TEXT_EDIT;
    event.data.text_edit.utf8 = "abc";
    event.data.text_edit.length = 3;
    cmp_text_field_test_set_fail_point(
        22u); /* CMP_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL */
    if (field.widget.vtable && field.widget.vtable->event) {
      field.widget.vtable->event(&field, &event, &handled);
    }
    cmp_text_field_test_set_fail_point(0u);
  }

  /* Line 1763: backspace fails event sync_label */
  {
    CMPInputEvent event;
    CMPBool handled;
    memset(&event, 0, sizeof(event));
    event.type = CMP_INPUT_KEY_DOWN;
    event.data.key.key_code = 8u; /* backspace */
    cmp_text_field_test_set_fail_point(
        22u); /* CMP_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL */
    if (field.widget.vtable && field.widget.vtable->event) {
      field.widget.vtable->event(&field, &event, &handled);
    }
    cmp_text_field_test_set_fail_point(0u);
  }

  /* Line 1829: get semantics null check */
  if (field.widget.vtable && field.widget.vtable->get_semantics) {
    field.widget.vtable->get_semantics(NULL, NULL);
  }

  /* Line 1857: destroy null check */
  if (field.widget.vtable && field.widget.vtable->destroy) {
    field.widget.vtable->destroy(NULL);
  }

  /* Line 1870, 1872: destroy font */
  {
    CMPTextField f2;
    CMPTextVTable vtable2 = g_test_text_vtable;
    vtable2.destroy_font = NULL;

    cmp_text_field_init(&f2, &text_backend, &style, NULL, NULL, 0);
    f2.owns_fonts = CMP_TRUE;
    f2.text_font.id = 99; /* trigger fail */
    f2.text_font.generation = 1;
    if (f2.widget.vtable && f2.widget.vtable->destroy) {
      f2.widget.vtable->destroy(&f2);
    }

    cmp_text_field_init(&f2, &text_backend, &style, NULL, NULL, 0);
    f2.owns_fonts = CMP_TRUE;
    f2.text_font.id = 1;
    f2.text_font.generation = 1;
    text_backend.vtable = &vtable2; /* trigger unsupported */
    if (f2.widget.vtable && f2.widget.vtable->destroy) {
      f2.widget.vtable->destroy(&f2);
    }
    text_backend.vtable = &g_test_text_vtable; /* restore */
  }

  /* Line 1882, 1884: destroy label font */
  {
    CMPTextField f2;
    CMPTextVTable vtable2 = g_test_text_vtable;
    vtable2.destroy_font = NULL;

    cmp_text_field_init(&f2, &text_backend, &style, NULL, NULL, 0);
    f2.owns_fonts = CMP_TRUE;
    f2.label_font.id = 99; /* trigger fail */
    f2.label_font.generation = 1;
    if (f2.widget.vtable && f2.widget.vtable->destroy) {
      f2.widget.vtable->destroy(&f2);
    }

    cmp_text_field_init(&f2, &text_backend, &style, NULL, NULL, 0);
    f2.owns_fonts = CMP_TRUE;
    f2.label_font.id = 1;
    f2.label_font.generation = 1;
    text_backend.vtable = &vtable2; /* trigger unsupported */
    if (f2.widget.vtable && f2.widget.vtable->destroy) {
      f2.widget.vtable->destroy(&f2);
    }
    text_backend.vtable = &g_test_text_vtable; /* restore */
  }

  return 0;
}
