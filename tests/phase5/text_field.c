#include "cmpc/cmp_text_field.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>
#if defined(__APPLE__)
#include <crt_externs.h>
#endif
#if !defined(_MSC_VER) && !defined(__APPLE__)
extern char **environ;
#endif

#define CMP_TEXT_FIELD_TEST_FAIL_NONE 0u
#define CMP_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE 1u
#define CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT 2u
#define CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT 3u
#define CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE 4u
#define CMP_TEXT_FIELD_TEST_FAIL_FONT_METRICS 5u
#define CMP_TEXT_FIELD_TEST_FAIL_ANIM_START 6u
#define CMP_TEXT_FIELD_TEST_FAIL_RESERVE_REALLOC 7u
#define CMP_TEXT_FIELD_TEST_FAIL_MEASURE_PREFIX 8u
#define CMP_TEXT_FIELD_TEST_FAIL_RESOLVE_COLORS 9u
#define CMP_TEXT_FIELD_TEST_FAIL_RESERVE_BYPASS 10u
#define CMP_TEXT_FIELD_TEST_FAIL_OFFSET_SKIP_EARLY 11u
#define CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND 12u
#define CMP_TEXT_FIELD_TEST_FAIL_TEXT_STYLE_INIT 13u
#define CMP_TEXT_FIELD_TEST_FAIL_LABEL_STYLE_INIT 14u
#define CMP_TEXT_FIELD_TEST_FAIL_ANIM_INIT 15u
#define CMP_TEXT_FIELD_TEST_FAIL_LABEL_FONT_METRICS 16u
#define CMP_TEXT_FIELD_TEST_FAIL_OUTLINE_RANGE 17u
#define CMP_TEXT_FIELD_TEST_FAIL_CORNER_RANGE 18u
#define CMP_TEXT_FIELD_TEST_FAIL_SELECTION_WIDTH_NEGATIVE 19u
#define CMP_TEXT_FIELD_TEST_FAIL_CURSOR_WIDTH_NEGATIVE 20u
#define CMP_TEXT_FIELD_TEST_FAIL_CURSOR_HEIGHT_NEGATIVE 21u
#define CMP_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL 22u

int CMP_CALL cmp_text_field_test_set_fail_point(cmp_u32 fail_point);
int CMP_CALL cmp_text_field_test_set_color_fail_after(cmp_u32 call_count);
int CMP_CALL cmp_text_field_test_set_overflow_fail_after(cmp_u32 call_count);
int CMP_CALL
cmp_text_field_test_set_font_metrics_fail_after(cmp_u32 call_count);
int CMP_CALL cmp_text_field_test_set_offset_skip_early(CMPBool enable);
int CMP_CALL cmp_text_field_test_clear_fail_points(void);
int CMP_CALL cmp_core_test_set_default_allocator_fail(CMPBool fail);
int CMP_CALL cmp_text_field_test_validate_color(const CMPColor *color);
int CMP_CALL cmp_text_field_test_color_set(CMPColor *color, CMPScalar r,
                                           CMPScalar g, CMPScalar b,
                                           CMPScalar a);
int CMP_CALL cmp_text_field_test_validate_text_style(const CMPTextStyle *style,
                                                     CMPBool require_family);
int CMP_CALL cmp_text_field_test_validate_style(const CMPTextFieldStyle *style,
                                                CMPBool require_text_family,
                                                CMPBool require_label_family);
int CMP_CALL
cmp_text_field_test_validate_backend(const CMPTextBackend *backend);
int CMP_CALL cmp_text_field_test_validate_measure_spec(CMPMeasureSpec spec);
int CMP_CALL cmp_text_field_test_validate_rect(const CMPRect *rect);
int CMP_CALL cmp_text_field_test_usize_max(cmp_usize *out_value);
int CMP_CALL cmp_text_field_test_add_overflow(cmp_usize a, cmp_usize b,
                                              cmp_usize *out_value);
int CMP_CALL cmp_text_field_test_reserve(CMPTextField *field,
                                         cmp_usize required);
int CMP_CALL cmp_text_field_test_validate_utf8(const char *utf8,
                                               cmp_usize utf8_len);
int CMP_CALL cmp_text_field_test_validate_offset(const char *utf8,
                                                 cmp_usize utf8_len,
                                                 cmp_usize offset);
int CMP_CALL cmp_text_field_test_prev_offset(const char *utf8,
                                             cmp_usize utf8_len,
                                             cmp_usize offset,
                                             cmp_usize *out_offset);
int CMP_CALL cmp_text_field_test_next_offset(const char *utf8,
                                             cmp_usize utf8_len,
                                             cmp_usize offset,
                                             cmp_usize *out_offset);
int CMP_CALL cmp_text_field_test_update_text_metrics(CMPTextField *field);
int CMP_CALL cmp_text_field_test_update_label_metrics(CMPTextField *field);
int CMP_CALL
cmp_text_field_test_update_placeholder_metrics(CMPTextField *field);
int CMP_CALL cmp_text_field_test_update_font_metrics(CMPTextField *field);
int CMP_CALL cmp_text_field_test_sync_label(CMPTextField *field);
int CMP_CALL cmp_text_field_test_reset_cursor_blink(CMPTextField *field);
int CMP_CALL cmp_text_field_test_set_text_internal(CMPTextField *field,
                                                   const char *utf8_text,
                                                   cmp_usize utf8_len,
                                                   CMPBool notify);
int CMP_CALL cmp_text_field_test_delete_range(CMPTextField *field,
                                              cmp_usize start, cmp_usize end,
                                              CMPBool notify);
int CMP_CALL cmp_text_field_test_measure_prefix(CMPTextField *field,
                                                cmp_usize offset,
                                                CMPScalar *out_width);
int CMP_CALL cmp_text_field_test_offset_for_x(CMPTextField *field, CMPScalar x,
                                              cmp_usize *out_offset);
int CMP_CALL cmp_text_field_test_resolve_colors(
    const CMPTextField *field, CMPColor *out_container, CMPColor *out_outline,
    CMPColor *out_text, CMPColor *out_label, CMPColor *out_placeholder,
    CMPColor *out_cursor, CMPColor *out_selection, CMPColor *out_handle);

typedef struct TestFieldBackend {
  int create_calls;
  int destroy_calls;
  int measure_calls;
  int draw_calls;
  int draw_rect_calls;
  int push_clip_calls;
  int pop_clip_calls;
  int fail_create;
  int fail_destroy;
  int fail_measure;
  int fail_draw;
  int fail_draw_after;
  int fail_draw_rect;
  int fail_draw_rect_after;
  int fail_push_clip;
  int fail_pop_clip;
  int fail_create_after;
  int fail_destroy_after;
  CMPHandle last_font;
  CMPRect last_rect;
  CMPColor last_rect_color;
  CMPScalar last_corner;
  CMPScalar last_text_x;
  CMPScalar last_text_y;
  cmp_usize last_text_len;
  CMPColor last_text_color;
} TestFieldBackend;

static int g_cmp_text_field_trace = 0;

static int cmp_text_field_env_enabled(const char *name) {
#if defined(_MSC_VER)
  char *value = NULL;
  size_t length = 0;
  int enabled = 0;

  if (_dupenv_s(&value, &length, name) == 0 && value != NULL) {
    enabled = (value[0] != '\0') ? 1 : 0;
    free(value);
  }
  return enabled;
#else
  size_t name_len;
  char **env;

  if (name == NULL || name[0] == '\0') {
    return 0;
  }

  name_len = strlen(name);
#if defined(__APPLE__)
  env = *_NSGetEnviron();
#else
  env = environ;
#endif

  if (env == NULL) {
    return 0;
  }

  for (; *env != NULL; ++env) {
    const char *entry = *env;
    if (strncmp(entry, name, name_len) == 0 && entry[name_len] == '=') {
      return entry[name_len + 1] != '\0' ? 1 : 0;
    }
  }
  return 0;
#endif
}

static void cmp_text_field_trace_step(int step) {
  if (g_cmp_text_field_trace) {
    fprintf(stderr, "cmp_phase5_text_field step %d\n", step);
  }
}

static void test_backend_init(TestFieldBackend *backend) {
  memset(backend, 0, sizeof(*backend));
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 cmp_i32 size_px, cmp_i32 weight,
                                 CMPBool italic, CMPHandle *out_font) {
  TestFieldBackend *backend;

  if (text == NULL || utf8_family == NULL || out_font == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (weight < 100 || weight > 900) {
    return CMP_ERR_RANGE;
  }
  if (italic != CMP_FALSE && italic != CMP_TRUE) {
    return CMP_ERR_RANGE;
  }

  backend = (TestFieldBackend *)text;
  backend->create_calls += 1;
  if (backend->fail_create) {
    return CMP_ERR_IO;
  }
  if (backend->fail_create_after > 0 &&
      backend->create_calls >= backend->fail_create_after) {
    return CMP_ERR_IO;
  }

  out_font->id = 1u;
  out_font->generation = 1u;
  backend->last_font = *out_font;
  return CMP_OK;
}

static int test_text_destroy_font(void *text, CMPHandle font) {
  TestFieldBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestFieldBackend *)text;
  backend->destroy_calls += 1;
  if (backend->fail_destroy) {
    return CMP_ERR_IO;
  }
  if (backend->fail_destroy_after > 0 &&
      backend->destroy_calls >= backend->fail_destroy_after) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_text_measure_text(void *text, CMPHandle font, const char *utf8,
                                  cmp_usize utf8_len, CMPScalar *out_width,
                                  CMPScalar *out_height,
                                  CMPScalar *out_baseline) {
  TestFieldBackend *backend;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestFieldBackend *)text;
  backend->measure_calls += 1;
  if (backend->fail_measure) {
    return CMP_ERR_IO;
  }

  *out_width = (CMPScalar)(utf8_len * 10u);
  *out_height = 20.0f;
  *out_baseline = 15.0f;
  return CMP_OK;
}

static int test_text_draw_text(void *text, CMPHandle font, const char *utf8,
                               cmp_usize utf8_len, CMPScalar x, CMPScalar y,
                               CMPColor color) {
  TestFieldBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestFieldBackend *)text;
  backend->draw_calls += 1;
  if (backend->fail_draw) {
    return CMP_ERR_IO;
  }
  if (backend->fail_draw_after > 0 &&
      backend->draw_calls >= backend->fail_draw_after) {
    return CMP_ERR_IO;
  }

  backend->last_text_x = x;
  backend->last_text_y = y;
  backend->last_text_len = utf8_len;
  backend->last_text_color = color;
  return CMP_OK;
}

static int test_gfx_draw_rect(void *gfx, const CMPRect *rect, CMPColor color,
                              CMPScalar corner_radius) {
  TestFieldBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestFieldBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect) {
    return CMP_ERR_IO;
  }
  if (backend->fail_draw_rect_after > 0 &&
      backend->draw_rect_calls >= backend->fail_draw_rect_after) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_gfx_push_clip(void *gfx, const CMPRect *rect) {
  TestFieldBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestFieldBackend *)gfx;
  backend->push_clip_calls += 1;
  if (backend->fail_push_clip) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestFieldBackend *backend;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestFieldBackend *)gfx;
  backend->pop_clip_calls += 1;
  if (backend->fail_pop_clip) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static const CMPTextVTable g_test_text_vtable = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    test_text_draw_text};

static const CMPTextVTable g_test_text_vtable_no_create = {
    NULL, test_text_destroy_font, test_text_measure_text, test_text_draw_text};

static const CMPTextVTable g_test_text_vtable_no_destroy = {
    test_text_create_font, NULL, test_text_measure_text, test_text_draw_text};

static const CMPTextVTable g_test_text_vtable_no_draw = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    NULL};

static const CMPTextVTable g_test_text_vtable_no_measure = {
    test_text_create_font, test_text_destroy_font, NULL, test_text_draw_text};

static const CMPGfxVTable g_test_gfx_vtable = {NULL,
                                               NULL,
                                               NULL,
                                               test_gfx_draw_rect,
                                               NULL,
                                               NULL,
                                               test_gfx_push_clip,
                                               test_gfx_pop_clip,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL};

static const CMPGfxVTable g_test_gfx_vtable_no_draw = {
    NULL, NULL, NULL, NULL, NULL, NULL, test_gfx_push_clip, test_gfx_pop_clip,
    NULL, NULL, NULL, NULL, NULL};

static const CMPGfxVTable g_test_gfx_vtable_no_clip = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

typedef struct TestAllocState {
  int fail_realloc;
  void *last_ptr;
} TestAllocState;

static int test_alloc_realloc(void *ctx, void *ptr, cmp_usize size,
                              void **out_ptr) {
  TestAllocState *state;
  void *mem;

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestAllocState *)ctx;
  if (state != NULL && state->fail_realloc) {
    return CMP_ERR_IO;
  }

  if (size == 0u) {
    *out_ptr = NULL;
    return CMP_OK;
  }

  if (ptr == NULL) {
    mem = malloc((size_t)size);
  } else {
    mem = realloc(ptr, (size_t)size);
  }

  if (mem == NULL) {
    return CMP_ERR_IO;
  }

  *out_ptr = mem;
  if (state != NULL) {
    state->last_ptr = mem;
  }
  return CMP_OK;
}

static int test_alloc_realloc_static(void *ctx, void *ptr, cmp_usize size,
                                     void **out_ptr) {
  static char buffer[8];

  CMP_UNUSED(ctx);
  CMP_UNUSED(ptr);
  CMP_UNUSED(size);

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_ptr = buffer;
  return CMP_OK;
}

static int test_alloc_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  return test_alloc_realloc(ctx, NULL, size, out_ptr);
}

static int test_alloc_free(void *ctx, void *ptr) {
  CMP_UNUSED(ctx);
  free(ptr);
  return CMP_OK;
}

typedef struct TestOnChangeState {
  int calls;
  int fail;
} TestOnChangeState;

static int test_on_change(void *ctx, CMPTextField *field, const char *utf8,
                          cmp_usize len) {
  TestOnChangeState *state;

  CMP_UNUSED(utf8);
  CMP_UNUSED(len);

  if (ctx == NULL || field == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestOnChangeState *)ctx;
  state->calls += 1;
  if (state->fail) {
    return CMP_ERR_IO;
  }

  return CMP_OK;
}

static int cmp_near(CMPScalar a, CMPScalar b, CMPScalar tol) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static int init_pointer_event(CMPInputEvent *event, cmp_u32 type, cmp_i32 x,
                              cmp_i32 y) {
  if (event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
  return CMP_OK;
}

static int init_key_event(CMPInputEvent *event, cmp_u32 key_code) {
  if (event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(event, 0, sizeof(*event));
  event->type = CMP_INPUT_KEY_DOWN;
  event->data.key.key_code = key_code;
  return CMP_OK;
}

int main(void) {
  TestFieldBackend backend;
  TestFieldBackend backend_alt;
  CMPTextBackend text_backend;
  CMPTextBackend text_backend_alt;
  CMPTextBackend text_backend_bad;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  CMPTextFieldStyle style;
  CMPTextFieldStyle style_with_label;
  CMPTextFieldStyle invalid_style;
  CMPTextFieldStyle style_temp;
  CMPTextStyle text_style;
  CMPTextField field;
  CMPTextField event_field;
  CMPTextField reserve_field;
  CMPTextField temp_field;
  CMPTextField destroy_field;
  CMPTextField destroy_field_no_destroy;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPRect temp_rect;
  CMPInputEvent event;
  CMPSemantics semantics;
  TestAllocState alloc_state;
  CMPAllocator custom_alloc;
  TestOnChangeState on_change_state;
  CMPColor test_color;
  CMPColor container;
  CMPColor outline;
  CMPColor text_color;
  CMPColor label_color;
  CMPColor placeholder_color;
  CMPColor cursor_color;
  CMPColor selection_color;
  CMPColor handle_color;
  CMPScalar width_value;
  const char *text_ptr;
  cmp_usize text_len;
  cmp_usize sel_start;
  cmp_usize sel_end;
  cmp_usize max_value;
  cmp_usize large_value;
  cmp_usize required_value;
  cmp_usize temp_value;
  cmp_usize offset_value;
  cmp_usize temp_utf8_len;
  cmp_usize temp_utf8_capacity;
  char *temp_utf8_ptr;
  cmp_u32 fail_index;
  int draw_rect_target;
  CMPBool handled;
  CMPBool changed;
  unsigned char invalid_utf8[2];
  unsigned char multi_utf8[3];
  char ascii_utf8[4];
  const char *invalid_utf8_str;
  const char *multi_utf8_str;

  g_cmp_text_field_trace = cmp_text_field_env_enabled("CMP_TEXT_FIELD_TRACE");
  cmp_text_field_trace_step(1);

  test_backend_init(&backend);
  text_backend.ctx = &backend;
  text_backend.vtable = &g_test_text_vtable;

  gfx.ctx = &backend;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = &g_test_text_vtable;

  paint_ctx.gfx = &gfx;
  paint_ctx.dpi_scale = 1.0f;
  paint_ctx.clip.x = 0.0f;
  paint_ctx.clip.y = 0.0f;
  paint_ctx.clip.width = 200.0f;
  paint_ctx.clip.height = 200.0f;

  test_backend_init(&backend_alt);
  text_backend_alt.ctx = &backend_alt;
  text_backend_alt.vtable = &g_test_text_vtable;

  text_backend_bad.ctx = &backend;
  text_backend_bad.vtable = NULL;

  memset(&alloc_state, 0, sizeof(alloc_state));
  custom_alloc.ctx = &alloc_state;
  custom_alloc.alloc = test_alloc_alloc;
  custom_alloc.realloc = test_alloc_realloc;
  custom_alloc.free = test_alloc_free;

  on_change_state.calls = 0;
  on_change_state.fail = 0;

  cmp_text_field_trace_step(2);

  CMP_TEST_EXPECT(cmp_text_field_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_style_init(&style));
  CMP_TEST_ASSERT(
      cmp_near(style.padding_x, CMP_TEXT_FIELD_DEFAULT_PADDING_X, 0.001f));

  for (fail_index = 1u; fail_index <= 12u; fail_index++) {
    CMP_TEST_OK(cmp_text_field_test_set_color_fail_after(fail_index));
    CMP_TEST_EXPECT(cmp_text_field_style_init(&style_temp), CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  }
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_TEXT_STYLE_INIT));
  CMP_TEST_EXPECT(cmp_text_field_style_init(&style_temp), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_LABEL_STYLE_INIT));
  CMP_TEST_EXPECT(cmp_text_field_style_init(&style_temp), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_EXPECT(cmp_text_field_test_validate_color(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  test_color = style.container_color;
  test_color.r = -0.1f;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_color(&test_color),
                  CMP_ERR_RANGE);
  test_color = style.container_color;
  test_color.g = 1.5f;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_color(&test_color),
                  CMP_ERR_RANGE);
  test_color = style.container_color;
  test_color.b = -0.2f;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_color(&test_color),
                  CMP_ERR_RANGE);
  test_color = style.container_color;
  test_color.a = 2.0f;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_color(&test_color),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_text_field_test_validate_color(&style.container_color));

  CMP_TEST_EXPECT(cmp_text_field_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_text_field_test_color_set(&test_color, -0.1f, 0.0f, 0.0f, 1.0f),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      cmp_text_field_test_color_set(&test_color, 0.0f, 1.5f, 0.0f, 1.0f),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      cmp_text_field_test_color_set(&test_color, 0.0f, 0.0f, -0.2f, 1.0f),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      cmp_text_field_test_color_set(&test_color, 0.0f, 0.0f, 0.0f, 2.0f),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_text_field_test_set_color_fail_after(1u));
  CMP_TEST_EXPECT(
      cmp_text_field_test_color_set(&test_color, 0.1f, 0.2f, 0.3f, 0.4f),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(
      cmp_text_field_test_color_set(&test_color, 0.1f, 0.2f, 0.3f, 0.4f));

  CMP_TEST_EXPECT(cmp_text_field_test_validate_text_style(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  text_style = style.text_style;
  text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_text_style(&text_style, CMP_TRUE),
      CMP_ERR_INVALID_ARGUMENT);
  text_style = style.text_style;
  text_style.utf8_family = "Test";
  text_style.size_px = 0;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_text_style(&text_style, CMP_TRUE),
      CMP_ERR_RANGE);
  text_style = style.text_style;
  text_style.utf8_family = "Test";
  text_style.weight = 50;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_text_style(&text_style, CMP_TRUE),
      CMP_ERR_RANGE);
  text_style = style.text_style;
  text_style.utf8_family = "Test";
  text_style.italic = 2;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_text_style(&text_style, CMP_TRUE),
      CMP_ERR_RANGE);
  text_style = style.text_style;
  text_style.utf8_family = "Test";
  text_style.color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_text_style(&text_style, CMP_TRUE),
      CMP_ERR_RANGE);
  text_style = style.text_style;
  text_style.utf8_family = NULL;
  CMP_TEST_OK(cmp_text_field_test_validate_text_style(&text_style, CMP_FALSE));
  text_style.utf8_family = "Test";
  CMP_TEST_OK(cmp_text_field_test_validate_text_style(&text_style, CMP_TRUE));

  cmp_text_field_trace_step(3);

  CMP_TEST_EXPECT(cmp_text_field_test_validate_style(NULL, CMP_TRUE, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.outline_width = -1.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.corner_radius = -1.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.padding_x = -1.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.padding_y = -1.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.min_height = -1.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.label_float_offset = -1.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.label_anim_duration = -1.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.cursor_width = -1.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.cursor_blink_period = -1.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.handle_radius = -1.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.handle_height = -1.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = NULL;
  style_temp.label_style.utf8_family = "Test";
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_INVALID_ARGUMENT);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = NULL;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_INVALID_ARGUMENT);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.text_style.size_px = 0;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.label_style.size_px = 0;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.container_color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.outline_color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.focused_outline_color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.disabled_container_color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.disabled_outline_color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.disabled_text_color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.disabled_label_color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.placeholder_color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.disabled_placeholder_color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.cursor_color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.selection_color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);
  style_temp = style;
  style_temp.text_style.utf8_family = "Test";
  style_temp.label_style.utf8_family = "Test";
  style_temp.handle_color.r = 2.0f;
  CMP_TEST_EXPECT(
      cmp_text_field_test_validate_style(&style_temp, CMP_TRUE, CMP_TRUE),
      CMP_ERR_RANGE);

  cmp_text_field_trace_step(4);

  CMP_TEST_EXPECT(cmp_text_field_test_validate_backend(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_backend(&text_backend_bad),
                  CMP_ERR_INVALID_ARGUMENT);
  text_backend_bad.vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_backend(&text_backend_bad),
                  CMP_ERR_UNSUPPORTED);
  text_backend_bad.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_backend(&text_backend_bad),
                  CMP_ERR_UNSUPPORTED);
  text_backend_bad.vtable = &g_test_text_vtable_no_create;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_backend(&text_backend_bad),
                  CMP_ERR_UNSUPPORTED);
  text_backend_bad.vtable = &g_test_text_vtable_no_destroy;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_backend(&text_backend_bad),
                  CMP_ERR_UNSUPPORTED);
  text_backend_bad.vtable = &g_test_text_vtable;
  CMP_TEST_OK(cmp_text_field_test_validate_backend(&text_backend_bad));

  cmp_text_field_trace_step(5);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_measure_spec(width_spec),
                  CMP_ERR_INVALID_ARGUMENT);
  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_measure_spec(width_spec),
                  CMP_ERR_RANGE);
  width_spec.size = 0.0f;
  CMP_TEST_OK(cmp_text_field_test_validate_measure_spec(width_spec));

  temp_rect.x = 0.0f;
  temp_rect.y = 0.0f;
  temp_rect.width = -1.0f;
  temp_rect.height = 1.0f;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_rect(&temp_rect), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_rect(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_text_field_test_usize_max(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_usize_max(&max_value));
  CMP_TEST_EXPECT(cmp_text_field_test_add_overflow(1u, 2u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_add_overflow(max_value, 1u, &temp_value),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_text_field_test_add_overflow(1u, 2u, &temp_value));
  CMP_TEST_OK(cmp_text_field_test_set_overflow_fail_after(1u));
  CMP_TEST_EXPECT(cmp_text_field_test_add_overflow(1u, 2u, &temp_value),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_ASSERT(temp_value == 3u);

  memset(&reserve_field, 0, sizeof(reserve_field));
  reserve_field.allocator = custom_alloc;
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(NULL, 1u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&reserve_field, 0u),
                  CMP_ERR_INVALID_ARGUMENT);
  reserve_field.allocator.realloc = NULL;
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&reserve_field, 1u),
                  CMP_ERR_INVALID_ARGUMENT);
  reserve_field.allocator.realloc = test_alloc_realloc;
  reserve_field.utf8_capacity = 8u;
  CMP_TEST_OK(cmp_text_field_test_reserve(&reserve_field, 4u));
  reserve_field.utf8 = NULL;
  reserve_field.utf8_capacity = 4u;
  CMP_TEST_OK(cmp_text_field_test_reserve(&reserve_field, 20u));
  CMP_TEST_ASSERT(reserve_field.utf8_capacity >= 20u);
  if (reserve_field.utf8 != NULL) {
    test_alloc_free(&alloc_state, reserve_field.utf8);
    reserve_field.utf8 = NULL;
    reserve_field.utf8_capacity = 0u;
  }
  large_value = max_value / 2u + 1u;
  CMP_TEST_OK(
      cmp_text_field_test_add_overflow(large_value, 1u, &required_value));
  reserve_field.utf8_capacity = large_value;
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&reserve_field, required_value),
                  CMP_ERR_OVERFLOW);
  reserve_field.utf8 = NULL;
  reserve_field.utf8_capacity = large_value;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_RESERVE_BYPASS));
  reserve_field.allocator.realloc = test_alloc_realloc_static;
  CMP_TEST_OK(cmp_text_field_test_reserve(&reserve_field, large_value));
  reserve_field.allocator.realloc = test_alloc_realloc;
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  reserve_field.utf8 = NULL;
  reserve_field.utf8_capacity = 0u;
  CMP_TEST_OK(cmp_text_field_test_reserve(&reserve_field, 1u));
  CMP_TEST_ASSERT(reserve_field.utf8_capacity >= 1u);
  reserve_field.utf8 = NULL;
  reserve_field.utf8_capacity = 0u;
  alloc_state.last_ptr = NULL;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_RESERVE_REALLOC));
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&reserve_field, 1u), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  {
    cmp_usize required_large;

    reserve_field.utf8 = NULL;
    reserve_field.utf8_capacity = 1u;
    reserve_field.allocator.realloc = test_alloc_realloc_static;
    CMP_TEST_OK(
        cmp_text_field_test_add_overflow(max_value / 2u, 2u, &required_large));
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_RESERVE_REALLOC));
    CMP_TEST_EXPECT(cmp_text_field_test_reserve(&reserve_field, required_large),
                    CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
    reserve_field.allocator.realloc = test_alloc_realloc;
  }
  if (alloc_state.last_ptr != NULL) {
    test_alloc_free(&alloc_state, alloc_state.last_ptr);
    alloc_state.last_ptr = NULL;
  }
  reserve_field.utf8 = NULL;
  reserve_field.utf8_capacity = 0u;
  alloc_state.fail_realloc = 1;
  CMP_TEST_EXPECT(cmp_text_field_test_reserve(&reserve_field, 1u), CMP_ERR_IO);
  alloc_state.fail_realloc = 0;
  if (reserve_field.utf8 != NULL) {
    test_alloc_free(&alloc_state, reserve_field.utf8);
    reserve_field.utf8 = NULL;
    reserve_field.utf8_capacity = 0u;
  }

  invalid_utf8[0] = 0xC0u;
  invalid_utf8[1] = '\0';
  invalid_utf8_str = (const char *)invalid_utf8;
  ascii_utf8[0] = 'A';
  ascii_utf8[1] = 'B';
  ascii_utf8[2] = 'C';
  ascii_utf8[3] = '\0';
  CMP_TEST_EXPECT(cmp_text_field_test_validate_utf8(NULL, 1u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_validate_utf8(invalid_utf8_str, 1u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_validate_utf8(ascii_utf8, 3u));
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE));
  CMP_TEST_EXPECT(cmp_text_field_test_validate_utf8(ascii_utf8, 3u),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  multi_utf8[0] = 0xC3u;
  multi_utf8[1] = 0xA9u;
  multi_utf8[2] = '\0';
  multi_utf8_str = (const char *)multi_utf8;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset(ascii_utf8, 3u, 4u),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_text_field_test_validate_offset(ascii_utf8, 3u, 0u));
  CMP_TEST_OK(cmp_text_field_test_validate_offset(ascii_utf8, 3u, 3u));
  CMP_TEST_OK(cmp_text_field_test_validate_offset(ascii_utf8, 3u, 1u));
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset(multi_utf8_str, 2u, 1u),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset(ascii_utf8, 3u, 1u),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT));
  CMP_TEST_EXPECT(cmp_text_field_test_validate_offset(ascii_utf8, 3u, 1u),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_offset_skip_early(CMP_TRUE));
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND));
  CMP_TEST_OK(cmp_text_field_test_validate_offset(ascii_utf8, 3u, 0u));
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_EXPECT(cmp_text_field_test_prev_offset(ascii_utf8, 3u, 1u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(
      cmp_text_field_test_prev_offset(ascii_utf8, 3u, 0u, &offset_value));
  CMP_TEST_ASSERT(offset_value == 0u);
  CMP_TEST_OK(
      cmp_text_field_test_prev_offset(ascii_utf8, 3u, 2u, &offset_value));
  CMP_TEST_ASSERT(offset_value == 1u);
  CMP_TEST_EXPECT(
      cmp_text_field_test_prev_offset(multi_utf8_str, 2u, 1u, &offset_value),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
  CMP_TEST_EXPECT(
      cmp_text_field_test_prev_offset(ascii_utf8, 3u, 3u, &offset_value),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT));
  CMP_TEST_EXPECT(
      cmp_text_field_test_prev_offset(ascii_utf8, 3u, 3u, &offset_value),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND));
  CMP_TEST_OK(
      cmp_text_field_test_prev_offset(ascii_utf8, 3u, 3u, &offset_value));
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_EXPECT(cmp_text_field_test_next_offset(ascii_utf8, 3u, 1u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(
      cmp_text_field_test_next_offset(ascii_utf8, 3u, 3u, &offset_value));
  CMP_TEST_ASSERT(offset_value == 3u);
  CMP_TEST_OK(
      cmp_text_field_test_next_offset(ascii_utf8, 3u, 1u, &offset_value));
  CMP_TEST_ASSERT(offset_value == 2u);
  CMP_TEST_EXPECT(
      cmp_text_field_test_next_offset(multi_utf8_str, 2u, 1u, &offset_value),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
  CMP_TEST_EXPECT(
      cmp_text_field_test_next_offset(ascii_utf8, 3u, 0u, &offset_value),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT));
  CMP_TEST_EXPECT(
      cmp_text_field_test_next_offset(ascii_utf8, 3u, 0u, &offset_value),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND));
  CMP_TEST_OK(
      cmp_text_field_test_next_offset(ascii_utf8, 3u, 0u, &offset_value));
  CMP_TEST_ASSERT(offset_value == 3u);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  cmp_text_field_trace_step(6);

  CMP_TEST_EXPECT(cmp_text_field_init(NULL, &text_backend, &style, NULL, "", 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_init(&field, NULL, &style, NULL, "", 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_init(&field, &text_backend, NULL, NULL, "", 0),
                  CMP_ERR_INVALID_ARGUMENT);

  text_backend.vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(
      cmp_text_field_init(&field, &text_backend, &style, NULL, "", 0),
      CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(
      cmp_text_field_init(&field, &text_backend, &style, NULL, "", 0),
      CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable;

  style.text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(
      cmp_text_field_init(&field, &text_backend, &style, NULL, "", 0),
      CMP_ERR_INVALID_ARGUMENT);
  style.text_style.utf8_family = "Test";
  style.label_style.utf8_family = NULL;

  custom_alloc.alloc = NULL;
  custom_alloc.realloc = test_alloc_realloc;
  custom_alloc.free = test_alloc_free;
  CMP_TEST_EXPECT(cmp_text_field_init(&temp_field, &text_backend, &style,
                                      &custom_alloc, "", 0),
                  CMP_ERR_INVALID_ARGUMENT);
  custom_alloc.alloc = test_alloc_alloc;
  custom_alloc.realloc = NULL;
  custom_alloc.free = test_alloc_free;
  CMP_TEST_EXPECT(cmp_text_field_init(&temp_field, &text_backend, &style,
                                      &custom_alloc, "", 0),
                  CMP_ERR_INVALID_ARGUMENT);
  custom_alloc.alloc = test_alloc_alloc;
  custom_alloc.realloc = test_alloc_realloc;
  custom_alloc.free = NULL;
  CMP_TEST_EXPECT(cmp_text_field_init(&temp_field, &text_backend, &style,
                                      &custom_alloc, "", 0),
                  CMP_ERR_INVALID_ARGUMENT);
  custom_alloc.alloc = test_alloc_alloc;
  custom_alloc.realloc = test_alloc_realloc;
  custom_alloc.free = test_alloc_free;
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  CMP_TEST_EXPECT(
      cmp_text_field_init(&temp_field, &text_backend, &style, NULL, "", 0),
      CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_INIT));
  CMP_TEST_EXPECT(
      cmp_text_field_init(&temp_field, &text_backend, &style, NULL, "", 0),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE));
  CMP_TEST_EXPECT(
      cmp_text_field_init(&temp_field, &text_backend, &style, NULL, "X", 1),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_init(&temp_field, &text_backend, &style,
                                  &custom_alloc, "", 0));
  CMP_TEST_OK(temp_field.widget.vtable->destroy(temp_field.widget.ctx));

  backend.fail_create = 1;
  CMP_TEST_EXPECT(
      cmp_text_field_init(&temp_field, &text_backend, &style, NULL, "", 0),
      CMP_ERR_IO);
  backend.fail_create = 0;

  test_backend_init(&backend_alt);
  text_backend_alt.ctx = &backend_alt;
  text_backend_alt.vtable = &g_test_text_vtable;
  style_temp = style;
  style_temp.label_style.utf8_family = "Test";
  backend_alt.fail_create_after = 2;
  CMP_TEST_EXPECT(cmp_text_field_init(&temp_field, &text_backend_alt,
                                      &style_temp, NULL, "", 0),
                  CMP_ERR_IO);
  backend_alt.fail_create_after = 0;

  test_backend_init(&backend);
  text_backend.ctx = &backend;
  gfx.ctx = &backend;
  CMP_TEST_OK(
      cmp_text_field_init(&field, &text_backend, &style, NULL, "Hi", 2));
  CMP_TEST_ASSERT(backend.create_calls == 1);

  cmp_text_field_trace_step(7);

  CMP_TEST_EXPECT(cmp_text_field_test_update_text_metrics(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_update_label_metrics(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_update_placeholder_metrics(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_update_font_metrics(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_sync_label(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_reset_cursor_blink(NULL));

  CMP_TEST_OK(cmp_text_field_init(&temp_field, &text_backend, &style,
                                  &custom_alloc, "AB", 2));
  CMP_TEST_OK(cmp_text_field_test_update_text_metrics(&temp_field));
  temp_field.text_metrics_valid = CMP_TRUE;
  CMP_TEST_OK(cmp_text_field_test_update_text_metrics(&temp_field));
  temp_field.text_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
  CMP_TEST_EXPECT(cmp_text_field_test_update_text_metrics(&temp_field),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  temp_field.label_metrics_valid = CMP_FALSE;
  temp_field.label_font.id = 0u;
  temp_field.label_font.generation = 0u;
  CMP_TEST_OK(cmp_text_field_test_update_label_metrics(&temp_field));
  CMP_TEST_ASSERT(temp_field.label_metrics.width == 0.0f);
  temp_field.utf8_label = "Lbl";
  temp_field.label_len = 3u;
  temp_field.label_font = temp_field.text_font;
  temp_field.label_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
  CMP_TEST_EXPECT(cmp_text_field_test_update_label_metrics(&temp_field),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  temp_field.placeholder_metrics_valid = CMP_TRUE;
  CMP_TEST_OK(cmp_text_field_test_update_placeholder_metrics(&temp_field));
  temp_field.placeholder_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
  CMP_TEST_EXPECT(cmp_text_field_test_update_placeholder_metrics(&temp_field),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  temp_field.font_metrics_valid = CMP_FALSE;
  temp_field.label_font.id = 0u;
  temp_field.label_font.generation = 0u;
  CMP_TEST_OK(cmp_text_field_test_update_font_metrics(&temp_field));
  temp_field.font_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_FONT_METRICS));
  CMP_TEST_EXPECT(cmp_text_field_test_update_font_metrics(&temp_field),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  temp_field.font_metrics_valid = CMP_FALSE;
  temp_field.label_font = temp_field.text_font;
  CMP_TEST_OK(cmp_text_field_test_set_font_metrics_fail_after(2u));
  CMP_TEST_EXPECT(cmp_text_field_test_update_font_metrics(&temp_field),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  temp_field.font_metrics_valid = CMP_FALSE;
  temp_field.label_font = temp_field.text_font;
  CMP_TEST_OK(cmp_text_field_test_set_font_metrics_fail_after(1u));
  CMP_TEST_EXPECT(cmp_text_field_test_update_font_metrics(&temp_field),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  temp_field.font_metrics_valid = CMP_FALSE;
  temp_field.label_font = temp_field.text_font;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_LABEL_FONT_METRICS));
  CMP_TEST_EXPECT(cmp_text_field_test_update_font_metrics(&temp_field),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  temp_field.utf8_label = NULL;
  temp_field.label_len = 0u;
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(cmp_text_field_test_sync_label(&temp_field), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_sync_label(&temp_field));
  temp_field.utf8_label = "Label";
  temp_field.label_len = 5u;
  temp_field.label_value = 0.0f;
  temp_field.focused = CMP_TRUE;
  temp_field.style.label_anim_duration = 0.0f;
  CMP_TEST_OK(cmp_text_field_test_sync_label(&temp_field));
  temp_field.focused = CMP_FALSE;
  temp_field.label_value = 0.0f;
  CMP_TEST_OK(cmp_text_field_test_sync_label(&temp_field));
  temp_field.label_value = 0.5f;
  temp_field.focused = CMP_TRUE;
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(cmp_text_field_test_sync_label(&temp_field), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  cmp_text_field_trace_step(8);

  CMP_TEST_EXPECT(
      cmp_text_field_test_set_text_internal(NULL, "", 0u, CMP_FALSE),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE));
  CMP_TEST_EXPECT(
      cmp_text_field_test_set_text_internal(&temp_field, "X", 1u, CMP_FALSE),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_overflow_fail_after(1u));
  CMP_TEST_EXPECT(
      cmp_text_field_test_set_text_internal(&temp_field, "X", 1u, CMP_FALSE),
      CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  temp_field.utf8 = NULL;
  temp_field.utf8_capacity = 0u;
  alloc_state.last_ptr = NULL;
  temp_field.allocator = custom_alloc;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_RESERVE_REALLOC));
  CMP_TEST_EXPECT(
      cmp_text_field_test_set_text_internal(&temp_field, "X", 1u, CMP_FALSE),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(
      cmp_text_field_test_set_text_internal(&temp_field, "X", 1u, CMP_FALSE),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  if (alloc_state.last_ptr != NULL && temp_field.utf8 == NULL) {
    test_alloc_free(&alloc_state, alloc_state.last_ptr);
  }
  alloc_state.last_ptr = NULL;
  on_change_state.calls = 0;
  on_change_state.fail = 1;
  temp_field.on_change = test_on_change;
  temp_field.on_change_ctx = &on_change_state;
  CMP_TEST_EXPECT(
      cmp_text_field_test_set_text_internal(&temp_field, "Y", 1u, CMP_TRUE),
      CMP_ERR_IO);
  on_change_state.fail = 0;
  CMP_TEST_OK(
      cmp_text_field_test_set_text_internal(&temp_field, "Z", 1u, CMP_TRUE));
  CMP_TEST_ASSERT(on_change_state.calls >= 2);

  CMP_TEST_EXPECT(cmp_text_field_test_delete_range(NULL, 0u, 0u, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_text_field_test_delete_range(&temp_field, 2u, 1u, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_text_field_test_delete_range(
                      &temp_field, 0u, temp_field.utf8_len + 1u, CMP_FALSE),
                  CMP_ERR_RANGE);
  on_change_state.fail = 1;
  CMP_TEST_EXPECT(cmp_text_field_test_delete_range(
                      &temp_field, 0u, temp_field.utf8_len, CMP_TRUE),
                  CMP_ERR_IO);
  on_change_state.fail = 0;
  CMP_TEST_OK(cmp_text_field_test_delete_range(&temp_field, 0u,
                                               temp_field.utf8_len, CMP_TRUE));

  cmp_text_field_trace_step(9);

  CMP_TEST_EXPECT(cmp_text_field_test_measure_prefix(NULL, 0u, &width_value),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_measure_prefix(&temp_field, 0u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_MEASURE_PREFIX));
  CMP_TEST_EXPECT(
      cmp_text_field_test_measure_prefix(&temp_field, 0u, &width_value),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_EXPECT(cmp_text_field_test_measure_prefix(
                      &temp_field, temp_field.utf8_len + 1u, &width_value),
                  CMP_ERR_RANGE);
  temp_utf8_ptr = temp_field.utf8;
  temp_utf8_len = temp_field.utf8_len;
  temp_utf8_capacity = temp_field.utf8_capacity;
  temp_field.utf8 = (char *)multi_utf8;
  temp_field.utf8_len = 2u;
  CMP_TEST_EXPECT(
      cmp_text_field_test_measure_prefix(&temp_field, 1u, &width_value),
      CMP_ERR_RANGE);
  temp_field.utf8 = ascii_utf8;
  temp_field.utf8_len = 3u;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
  CMP_TEST_EXPECT(
      cmp_text_field_test_measure_prefix(&temp_field, 1u, &width_value),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(
      cmp_text_field_test_measure_prefix(&temp_field, 1u, &width_value));

  cmp_text_field_trace_step(10);

  CMP_TEST_EXPECT(cmp_text_field_test_offset_for_x(NULL, 0.0f, &offset_value),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_offset_for_x(&temp_field, 0.0f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  temp_field.utf8_len = 0u;
  CMP_TEST_OK(
      cmp_text_field_test_offset_for_x(&temp_field, 10.0f, &offset_value));
  CMP_TEST_ASSERT(offset_value == 0u);
  temp_field.utf8 = ascii_utf8;
  temp_field.utf8_len = 3u;
  CMP_TEST_OK(
      cmp_text_field_test_offset_for_x(&temp_field, -1.0f, &offset_value));
  CMP_TEST_ASSERT(offset_value == 0u);
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
  CMP_TEST_EXPECT(
      cmp_text_field_test_offset_for_x(&temp_field, 5.0f, &offset_value),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT));
  CMP_TEST_EXPECT(
      cmp_text_field_test_offset_for_x(&temp_field, 5.0f, &offset_value),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_MEASURE_PREFIX));
  CMP_TEST_EXPECT(
      cmp_text_field_test_offset_for_x(&temp_field, 5.0f, &offset_value),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND));
  CMP_TEST_OK(
      cmp_text_field_test_offset_for_x(&temp_field, 50.0f, &offset_value));
  CMP_TEST_ASSERT(offset_value == temp_field.utf8_len);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(
      cmp_text_field_test_offset_for_x(&temp_field, 5.0f, &offset_value));
  CMP_TEST_ASSERT(offset_value <= temp_field.utf8_len);

  cmp_text_field_trace_step(11);

  temp_field.utf8 = temp_utf8_ptr;
  temp_field.utf8_len = temp_utf8_len;
  temp_field.utf8_capacity = temp_utf8_capacity;
  cmp_text_field_trace_step(12);
  CMP_TEST_OK(temp_field.widget.vtable->destroy(temp_field.widget.ctx));

  cmp_text_field_trace_step(13);

  CMP_TEST_OK(cmp_text_field_get_text(&field, &text_ptr, &text_len));
  CMP_TEST_ASSERT(text_len == 2);
  CMP_TEST_ASSERT(text_ptr != NULL);
  CMP_TEST_EXPECT(cmp_text_field_get_text(NULL, &text_ptr, &text_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_get_text(&field, NULL, &text_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_get_text(&field, &text_ptr, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_text_field_set_label(NULL, "Name", 4),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_label(&field, NULL, 1u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE));
  CMP_TEST_EXPECT(cmp_text_field_set_label(&field, "Bad", 3), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_EXPECT(cmp_text_field_set_label(&field, "Name", 4),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_text_field_style_init(&style_with_label));
  style_with_label.text_style.utf8_family = "Test";
  style_with_label.label_style.utf8_family = "Test";
  CMP_TEST_EXPECT(cmp_text_field_set_style(NULL, &style_with_label),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_set_style(&field, &style_with_label));
  CMP_TEST_ASSERT(backend.create_calls >= 2);

  CMP_TEST_OK(cmp_text_field_set_label(&field, "Name", 4));
  CMP_TEST_ASSERT(backend.create_calls >= 3);
  field.focused = CMP_TRUE;
  field.label_value = 0.0f;
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(cmp_text_field_set_label(&field, "Name", 4), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  backend.fail_create = 1;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &style_with_label),
                  CMP_ERR_IO);
  backend.fail_create = 0;

  backend.fail_destroy = 1;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &style_with_label),
                  CMP_ERR_IO);
  backend.fail_destroy = 0;
  backend.destroy_calls = 0;
  backend.fail_destroy_after = 2;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &style_with_label),
                  CMP_ERR_IO);
  backend.fail_destroy_after = 0;
  field.focused = CMP_TRUE;
  field.label_value = 0.0f;
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &style_with_label),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  test_backend_init(&backend_alt);
  text_backend_alt.ctx = &backend_alt;
  text_backend_alt.vtable = &g_test_text_vtable;
  CMP_TEST_OK(cmp_text_field_init(&temp_field, &text_backend_alt,
                                  &style_with_label, NULL, "", 0));
  temp_field.utf8_label = "Label";
  temp_field.label_len = 5u;
  backend_alt.create_calls = 0;
  backend_alt.fail_create_after = 2;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&temp_field, &style_with_label),
                  CMP_ERR_IO);
  backend_alt.fail_create_after = 0;
  CMP_TEST_OK(temp_field.widget.vtable->destroy(temp_field.widget.ctx));

  test_backend_init(&backend_alt);
  text_backend_alt.ctx = &backend_alt;
  text_backend_alt.vtable = &g_test_text_vtable;
  CMP_TEST_OK(cmp_text_field_init(&temp_field, &text_backend_alt,
                                  &style_with_label, NULL, "", 0));
  temp_field.label_font.id = 0u;
  temp_field.label_font.generation = 0u;
  backend_alt.fail_create = 1;
  CMP_TEST_EXPECT(cmp_text_field_set_label(&temp_field, "Name", 4), CMP_ERR_IO);
  backend_alt.fail_create = 0;
  CMP_TEST_OK(temp_field.widget.vtable->destroy(temp_field.widget.ctx));

  invalid_style = style_with_label;
  invalid_style.padding_x = -1.0f;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &invalid_style),
                  CMP_ERR_RANGE);
  invalid_style = style_with_label;
  invalid_style.min_height = -1.0f;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &invalid_style),
                  CMP_ERR_RANGE);
  invalid_style = style_with_label;
  invalid_style.cursor_width = -1.0f;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &invalid_style),
                  CMP_ERR_RANGE);
  invalid_style = style_with_label;
  invalid_style.handle_radius = -1.0f;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &invalid_style),
                  CMP_ERR_RANGE);
  invalid_style = style_with_label;
  invalid_style.text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &invalid_style),
                  CMP_ERR_INVALID_ARGUMENT);
  invalid_style = style_with_label;
  invalid_style.label_style.utf8_family = NULL;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &invalid_style),
                  CMP_ERR_INVALID_ARGUMENT);
  invalid_style = style_with_label;
  invalid_style.text_style.size_px = 0;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &invalid_style),
                  CMP_ERR_RANGE);
  invalid_style = style_with_label;
  invalid_style.text_style.weight = 50;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &invalid_style),
                  CMP_ERR_RANGE);
  invalid_style = style_with_label;
  invalid_style.text_style.italic = 2;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &invalid_style),
                  CMP_ERR_RANGE);
  invalid_style = style_with_label;
  invalid_style.label_style.size_px = 0;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &invalid_style),
                  CMP_ERR_RANGE);
  invalid_style = style_with_label;
  invalid_style.container_color.a = 1.5f;
  CMP_TEST_EXPECT(cmp_text_field_set_style(&field, &invalid_style),
                  CMP_ERR_RANGE);

  memset(&destroy_field_no_destroy, 0, sizeof(destroy_field_no_destroy));
  destroy_field_no_destroy.style = style_with_label;
  destroy_field_no_destroy.widget.flags = 0;
  destroy_field_no_destroy.focused = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_resolve_colors(
      &destroy_field_no_destroy, &container, &outline, &text_color,
      &label_color, &placeholder_color, &cursor_color, &selection_color,
      &handle_color));
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      NULL, &container, &outline, &text_color, &label_color,
                      &placeholder_color, &cursor_color, &selection_color,
                      &handle_color),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      &destroy_field_no_destroy, NULL, &outline, &text_color,
                      &label_color, &placeholder_color, &cursor_color,
                      &selection_color, &handle_color),
                  CMP_ERR_INVALID_ARGUMENT);
  destroy_field_no_destroy.focused = CMP_TRUE;
  CMP_TEST_OK(cmp_text_field_test_resolve_colors(
      &destroy_field_no_destroy, &container, &outline, &text_color,
      &label_color, &placeholder_color, &cursor_color, &selection_color,
      &handle_color));
  destroy_field_no_destroy.widget.flags = CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(cmp_text_field_test_resolve_colors(
      &destroy_field_no_destroy, &container, &outline, &text_color,
      &label_color, &placeholder_color, &cursor_color, &selection_color,
      &handle_color));
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_RESOLVE_COLORS));
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      &destroy_field_no_destroy, &container, &outline,
                      &text_color, &label_color, &placeholder_color,
                      &cursor_color, &selection_color, &handle_color),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  destroy_field_no_destroy.widget.flags = 0;
  destroy_field_no_destroy.focused = CMP_FALSE;
  style_temp = style_with_label;
  style_temp.container_color.r = 2.0f;
  destroy_field_no_destroy.style = style_temp;
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      &destroy_field_no_destroy, &container, &outline,
                      &text_color, &label_color, &placeholder_color,
                      &cursor_color, &selection_color, &handle_color),
                  CMP_ERR_RANGE);
  style_temp = style_with_label;
  style_temp.outline_color.r = 2.0f;
  destroy_field_no_destroy.style = style_temp;
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      &destroy_field_no_destroy, &container, &outline,
                      &text_color, &label_color, &placeholder_color,
                      &cursor_color, &selection_color, &handle_color),
                  CMP_ERR_RANGE);
  style_temp = style_with_label;
  style_temp.text_style.color.r = 2.0f;
  destroy_field_no_destroy.style = style_temp;
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      &destroy_field_no_destroy, &container, &outline,
                      &text_color, &label_color, &placeholder_color,
                      &cursor_color, &selection_color, &handle_color),
                  CMP_ERR_RANGE);
  style_temp = style_with_label;
  style_temp.label_style.color.r = 2.0f;
  destroy_field_no_destroy.style = style_temp;
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      &destroy_field_no_destroy, &container, &outline,
                      &text_color, &label_color, &placeholder_color,
                      &cursor_color, &selection_color, &handle_color),
                  CMP_ERR_RANGE);
  style_temp = style_with_label;
  style_temp.placeholder_color.r = 2.0f;
  destroy_field_no_destroy.style = style_temp;
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      &destroy_field_no_destroy, &container, &outline,
                      &text_color, &label_color, &placeholder_color,
                      &cursor_color, &selection_color, &handle_color),
                  CMP_ERR_RANGE);
  style_temp = style_with_label;
  style_temp.cursor_color.r = 2.0f;
  destroy_field_no_destroy.style = style_temp;
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      &destroy_field_no_destroy, &container, &outline,
                      &text_color, &label_color, &placeholder_color,
                      &cursor_color, &selection_color, &handle_color),
                  CMP_ERR_RANGE);
  style_temp = style_with_label;
  style_temp.selection_color.r = 2.0f;
  destroy_field_no_destroy.style = style_temp;
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      &destroy_field_no_destroy, &container, &outline,
                      &text_color, &label_color, &placeholder_color,
                      &cursor_color, &selection_color, &handle_color),
                  CMP_ERR_RANGE);
  style_temp = style_with_label;
  style_temp.handle_color.r = 2.0f;
  destroy_field_no_destroy.style = style_temp;
  CMP_TEST_EXPECT(cmp_text_field_test_resolve_colors(
                      &destroy_field_no_destroy, &container, &outline,
                      &text_color, &label_color, &placeholder_color,
                      &cursor_color, &selection_color, &handle_color),
                  CMP_ERR_RANGE);
  destroy_field_no_destroy.style = style_with_label;

  CMP_TEST_EXPECT(cmp_text_field_set_placeholder(NULL, "Hint", 4),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_set_placeholder(&field, "Hint", 4));
  CMP_TEST_EXPECT(cmp_text_field_set_placeholder(&field, NULL, 1u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE));
  CMP_TEST_EXPECT(cmp_text_field_set_placeholder(&field, "Bad", 3), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_EXPECT(cmp_text_field_set_text(NULL, "", 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_text(&field, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  invalid_utf8[0] = 0xC0u;
  invalid_utf8[1] = '\0';
  invalid_utf8_str = (const char *)invalid_utf8;
  CMP_TEST_EXPECT(cmp_text_field_set_text(&field, invalid_utf8_str, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, invalid_utf8_str, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(NULL, "X", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_insert_utf8(&field, NULL, 0));

  multi_utf8[0] = 0xC3u;
  multi_utf8[1] = 0xA9u;
  multi_utf8[2] = '\0';
  multi_utf8_str = (const char *)multi_utf8;
  CMP_TEST_OK(cmp_text_field_set_text(&field, multi_utf8_str, 2));
  CMP_TEST_EXPECT(cmp_text_field_set_cursor(NULL, 0u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_cursor(&field, 1), CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_text_field_set_cursor(&field, 2));
  CMP_TEST_EXPECT(cmp_text_field_set_cursor(&field, field.utf8_len + 1u),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_text_field_set_selection(&field, 0, 3), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_text_field_set_selection(NULL, 0u, 0u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
  CMP_TEST_EXPECT(cmp_text_field_set_selection(&field, 1u, 1u), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
  CMP_TEST_EXPECT(cmp_text_field_set_selection(&field, 0u, 1u), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_EXPECT(cmp_text_field_get_selection(NULL, &sel_start, &sel_end),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_get_selection(&field, NULL, &sel_end),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_get_selection(&field, &sel_start, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_set_selection(&field, 2, 0));
  CMP_TEST_OK(cmp_text_field_get_selection(&field, &sel_start, &sel_end));
  CMP_TEST_ASSERT(sel_start <= sel_end);
  CMP_TEST_OK(cmp_text_field_get_cursor(&field, &sel_start));
  CMP_TEST_EXPECT(cmp_text_field_get_cursor(NULL, &sel_start),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_get_cursor(&field, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_focus(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_set_focus(&field, 2), CMP_ERR_RANGE);
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(cmp_text_field_set_focus(&field, CMP_TRUE), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE));
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, "X", 1), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  field.cursor = field.utf8_len;
  field.selection_start = field.cursor;
  field.selection_end = field.cursor;
  field.cursor = field.utf8_len + 1u;
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, "X", 1), CMP_ERR_RANGE);
  field.cursor = field.utf8_len;
  field.selection_start = 5u;
  field.selection_end = 1u;
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, "X", 1), CMP_ERR_RANGE);
  field.selection_start = field.cursor;
  field.selection_end = field.cursor;
  field.cursor = 1u;
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&field, "X", 1), CMP_ERR_RANGE);
  field.cursor = field.utf8_len;

  CMP_TEST_OK(cmp_text_field_init(&temp_field, &text_backend, &style,
                                  &custom_alloc, "A", 1));
  if (temp_field.utf8 != NULL && temp_field.allocator.free != NULL) {
    temp_field.allocator.free(temp_field.allocator.ctx, temp_field.utf8);
  }
  temp_field.utf8 = NULL;
  temp_field.utf8_len = 0u;
  temp_field.utf8_capacity = 0u;
  temp_field.cursor = 0u;
  temp_field.selection_start = 0u;
  temp_field.selection_end = 0u;
  CMP_TEST_OK(cmp_text_field_test_set_overflow_fail_after(1u));
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&temp_field, "B", 1),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_overflow_fail_after(2u));
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&temp_field, "B", 1),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_RESERVE_REALLOC));
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&temp_field, "B", 1), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&temp_field, "C", 1), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  on_change_state.calls = 0;
  on_change_state.fail = 1;
  temp_field.on_change = test_on_change;
  temp_field.on_change_ctx = &on_change_state;
  CMP_TEST_EXPECT(cmp_text_field_insert_utf8(&temp_field, "D", 1), CMP_ERR_IO);
  on_change_state.fail = 0;
  CMP_TEST_OK(cmp_text_field_insert_utf8(&temp_field, "E", 1));
  temp_field.on_change = NULL;
  temp_field.on_change_ctx = NULL;
  CMP_TEST_OK(temp_field.widget.vtable->destroy(temp_field.widget.ctx));

  CMP_TEST_EXPECT(cmp_text_field_delete_selection(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_backspace(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_delete_forward(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_set_text(&field, "Hi", 2));
  CMP_TEST_OK(cmp_text_field_set_selection(&field, 1, 1));
  CMP_TEST_OK(cmp_text_field_delete_selection(&field));
  CMP_TEST_OK(cmp_text_field_set_selection(&field, 0, 1));
  CMP_TEST_OK(cmp_text_field_delete_selection(&field));
  CMP_TEST_OK(cmp_text_field_set_text(&field, "Hi", 2));
  CMP_TEST_OK(cmp_text_field_set_selection(&field, 0, 1));
  CMP_TEST_OK(cmp_text_field_backspace(&field));
  CMP_TEST_OK(cmp_text_field_set_text(&field, "Hi", 2));
  CMP_TEST_OK(cmp_text_field_set_selection(&field, 0, 1));
  CMP_TEST_OK(cmp_text_field_delete_forward(&field));
  CMP_TEST_OK(cmp_text_field_set_text(&field, "Hi", 2));
  CMP_TEST_OK(cmp_text_field_set_cursor(&field, 1));
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
  CMP_TEST_EXPECT(cmp_text_field_backspace(&field), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_set_cursor(&field, 0));
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
  CMP_TEST_EXPECT(cmp_text_field_delete_forward(&field), CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_set_cursor(&field, 0));
  CMP_TEST_OK(cmp_text_field_delete_selection(&field));
  CMP_TEST_OK(cmp_text_field_backspace(&field));
  CMP_TEST_OK(cmp_text_field_set_cursor(&field, field.utf8_len));
  CMP_TEST_OK(cmp_text_field_delete_forward(&field));
  CMP_TEST_OK(cmp_text_field_insert_utf8(&field, "!", 1));
  CMP_TEST_OK(cmp_text_field_get_text(&field, &text_ptr, &text_len));
  CMP_TEST_ASSERT(text_len == 3);
  CMP_TEST_OK(cmp_text_field_set_selection(&field, 0, 1));
  CMP_TEST_OK(cmp_text_field_insert_utf8(&field, "X", 1));

  CMP_TEST_OK(cmp_text_field_backspace(&field));
  CMP_TEST_OK(cmp_text_field_get_text(&field, &text_ptr, &text_len));
  CMP_TEST_ASSERT(text_len == 2);

  CMP_TEST_OK(cmp_text_field_set_cursor(&field, 0));
  CMP_TEST_OK(cmp_text_field_delete_forward(&field));
  CMP_TEST_OK(cmp_text_field_get_text(&field, &text_ptr, &text_len));
  CMP_TEST_ASSERT(text_len == 1);

  CMP_TEST_OK(cmp_text_field_set_selection(&field, 0, 1));
  CMP_TEST_OK(cmp_text_field_delete_selection(&field));
  CMP_TEST_OK(cmp_text_field_get_text(&field, &text_ptr, &text_len));
  CMP_TEST_ASSERT(text_len == 0);

  CMP_TEST_OK(cmp_text_field_set_text(&field, "ABCD", 4));
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 200.0f;
  bounds.height = 50.0f;
  bounds.width = -1.0f;
  CMP_TEST_EXPECT(field.widget.vtable->layout(field.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  bounds.width = 200.0f;
  CMP_TEST_EXPECT(field.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(field.widget.vtable->layout(field.widget.ctx, bounds));

  CMP_TEST_EXPECT(field.widget.vtable->event(field.widget.ctx, NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(field.widget.vtable->event(field.widget.ctx, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(field.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 30, 10));
  CMP_TEST_EXPECT(
      field.widget.vtable->event(field.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 30, 10));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 30, 10));
  CMP_TEST_EXPECT(
      field.widget.vtable->event(field.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 30, 10));
  CMP_TEST_EXPECT(
      field.widget.vtable->event(field.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 60, 10));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 60, 10));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 60, 10));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 60, 10));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  field.selecting = CMP_TRUE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 60, 10));
  CMP_TEST_EXPECT(
      field.widget.vtable->event(field.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  field.selecting = CMP_FALSE;

  field.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 30, 10));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  field.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_TEXT;
  event.data.text.utf8[0] = 'Z';
  event.data.text.utf8[1] = '\0';
  event.data.text.length = 1u;
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE));
  CMP_TEST_EXPECT(
      field.widget.vtable->event(field.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(
      field.widget.vtable->event(field.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_TEXT;
  event.data.text.length = 0u;
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_TEXT_UTF8;
  event.data.text_utf8.utf8 = "Q";
  event.data.text_utf8.length = 1u;
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(
      field.widget.vtable->event(field.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_TEXT_UTF8;
  event.data.text_utf8.utf8 = "";
  event.data.text_utf8.length = 0u;
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_TEXT_EDIT;
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(
      field.widget.vtable->event(field.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_OK(
      cmp_text_field_init(&event_field, &text_backend, &style, NULL, "A", 1));
  event_field.utf8_label = NULL;
  event_field.label_len = 0u;
  event_field.label_value = 1.0f;
  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_TEXT;
  event.data.text.utf8[0] = 'B';
  event.data.text.utf8[1] = '\0';
  event.data.text.length = 1u;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL));
  CMP_TEST_EXPECT(event_field.widget.vtable->event(event_field.widget.ctx,
                                                   &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_TEXT_UTF8;
  event.data.text_utf8.utf8 = "C";
  event.data.text_utf8.length = 1u;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_EVENT_SYNC_LABEL));
  CMP_TEST_EXPECT(event_field.widget.vtable->event(event_field.widget.ctx,
                                                   &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_OK(init_key_event(&event, 46u));
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(event_field.widget.vtable->event(event_field.widget.ctx,
                                                   &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(event_field.widget.vtable->destroy(event_field.widget.ctx));

  CMP_TEST_OK(init_key_event(&event, 8u));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_OK(
      cmp_text_field_test_set_fail_point(CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
  CMP_TEST_EXPECT(
      field.widget.vtable->event(field.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_OK(init_key_event(&event, 46u));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_OK(init_key_event(&event, 37u));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
  CMP_TEST_EXPECT(
      field.widget.vtable->event(field.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_OK(cmp_text_field_set_cursor(&field, 0));
  CMP_TEST_OK(init_key_event(&event, 39u));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
  CMP_TEST_EXPECT(
      field.widget.vtable->event(field.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  CMP_TEST_OK(init_key_event(&event, 36u));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_OK(init_key_event(&event, 35u));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_OK(init_key_event(&event, 123u));
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_KEY_UP;
  CMP_TEST_OK(field.widget.vtable->event(field.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_EXPECT(field.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(field.widget.vtable->get_semantics(field.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  field.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  field.focused = CMP_TRUE;
  CMP_TEST_OK(field.widget.vtable->get_semantics(field.widget.ctx, &semantics));
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0);
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE) != 0);
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSED) != 0);
  field.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;
  field.focused = CMP_FALSE;

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(field.widget.vtable->measure(field.widget.ctx, width_spec,
                                               height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  CMP_TEST_EXPECT(field.widget.vtable->measure(field.widget.ctx, width_spec,
                                               height_spec, &size),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(field.widget.vtable->measure(field.widget.ctx, width_spec,
                                               height_spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      field.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = -1.0f;
  CMP_TEST_EXPECT(field.widget.vtable->measure(field.widget.ctx, width_spec,
                                               height_spec, &size),
                  CMP_ERR_RANGE);
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 10.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = -1.0f;
  CMP_TEST_EXPECT(field.widget.vtable->measure(field.widget.ctx, width_spec,
                                               height_spec, &size),
                  CMP_ERR_RANGE);
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  style_temp = field.style;
  field.style.text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(field.widget.vtable->measure(field.widget.ctx, width_spec,
                                               height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  field.style = style_temp;
  field.text_metrics_valid = CMP_FALSE;
  field.label_metrics_valid = CMP_FALSE;
  field.placeholder_metrics_valid = CMP_FALSE;
  field.font_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
  CMP_TEST_EXPECT(field.widget.vtable->measure(field.widget.ctx, width_spec,
                                               height_spec, &size),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  field.text_metrics_valid = CMP_TRUE;
  field.label_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
  CMP_TEST_EXPECT(field.widget.vtable->measure(field.widget.ctx, width_spec,
                                               height_spec, &size),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  field.text_metrics_valid = CMP_TRUE;
  field.label_metrics_valid = CMP_TRUE;
  field.placeholder_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
  CMP_TEST_EXPECT(field.widget.vtable->measure(field.widget.ctx, width_spec,
                                               height_spec, &size),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  field.text_metrics_valid = CMP_TRUE;
  field.label_metrics_valid = CMP_TRUE;
  field.placeholder_metrics_valid = CMP_TRUE;
  field.font_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_FONT_METRICS));
  CMP_TEST_EXPECT(field.widget.vtable->measure(field.widget.ctx, width_spec,
                                               height_spec, &size),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 60.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(field.widget.vtable->measure(field.widget.ctx, width_spec,
                                           height_spec, &size));
  CMP_TEST_ASSERT(size.width <= 60.0f);
  CMP_TEST_ASSERT(size.height >= style_with_label.min_height);
  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 80.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 40.0f;
  CMP_TEST_OK(field.widget.vtable->measure(field.widget.ctx, width_spec,
                                           height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 80.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 40.0f, 0.001f));
  {
    CMPTextField measure_field;

    measure_field = field;
    measure_field.widget.ctx = &measure_field;
    measure_field.text_metrics_valid = CMP_TRUE;
    measure_field.label_metrics_valid = CMP_TRUE;
    measure_field.placeholder_metrics_valid = CMP_TRUE;
    measure_field.font_metrics_valid = CMP_TRUE;
    measure_field.text_metrics.width = 10.0f;
    measure_field.label_metrics.width = 5.0f;
    measure_field.placeholder_metrics.width = 30.0f;
    measure_field.text_font_metrics.height = 10.0f;
    measure_field.label_font_metrics.height = 25.0f;
    measure_field.utf8_label = NULL;
    measure_field.label_len = 0u;
    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec.mode = CMP_MEASURE_EXACTLY;
    height_spec.size = 22.0f;
    CMP_TEST_OK(measure_field.widget.vtable->measure(
        measure_field.widget.ctx, width_spec, height_spec, &size));
    CMP_TEST_ASSERT(size.width >= 30.0f);
    CMP_TEST_ASSERT(cmp_near(size.height, 22.0f, 0.001f));
  }
  {
    CMPTextField measure_field;

    measure_field = field;
    measure_field.widget.ctx = &measure_field;
    measure_field.text_metrics_valid = CMP_TRUE;
    measure_field.label_metrics_valid = CMP_TRUE;
    measure_field.placeholder_metrics_valid = CMP_TRUE;
    measure_field.font_metrics_valid = CMP_TRUE;
    measure_field.text_metrics.width = 10.0f;
    measure_field.label_metrics.width = 25.0f;
    measure_field.placeholder_metrics.width = 5.0f;
    measure_field.text_font_metrics.height = 8.0f;
    measure_field.label_font_metrics.height = 9.0f;
    width_spec.mode = CMP_MEASURE_AT_MOST;
    width_spec.size = 20.0f;
    height_spec.mode = CMP_MEASURE_AT_MOST;
    height_spec.size = 15.0f;
    CMP_TEST_OK(measure_field.widget.vtable->measure(
        measure_field.widget.ctx, width_spec, height_spec, &size));
    CMP_TEST_ASSERT(size.width <= 20.0f);
    CMP_TEST_ASSERT(size.height <= 15.0f);
  }

  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(field.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = NULL;
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;
  temp_rect = field.bounds;
  field.bounds.width = -1.0f;
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  field.bounds = temp_rect;
  style_temp = field.style;
  field.style.padding_x = -1.0f;
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  field.style = style_temp;
  field.text_metrics_valid = CMP_FALSE;
  field.label_metrics_valid = CMP_FALSE;
  field.placeholder_metrics_valid = CMP_FALSE;
  field.font_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  field.text_metrics_valid = CMP_TRUE;
  field.label_metrics_valid = CMP_FALSE;
  field.placeholder_metrics_valid = CMP_FALSE;
  field.font_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  field.text_metrics_valid = CMP_TRUE;
  field.label_metrics_valid = CMP_TRUE;
  field.placeholder_metrics_valid = CMP_FALSE;
  field.font_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  field.text_metrics_valid = CMP_TRUE;
  field.label_metrics_valid = CMP_TRUE;
  field.placeholder_metrics_valid = CMP_TRUE;
  field.font_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_FONT_METRICS));
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  field.text_metrics_valid = CMP_FALSE;
  field.label_metrics_valid = CMP_FALSE;
  field.placeholder_metrics_valid = CMP_FALSE;
  field.font_metrics_valid = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_RESOLVE_COLORS));
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_OUTLINE_RANGE));
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_CORNER_RANGE));
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  style_temp = field.style;
  field.style.outline_width = 50.0f;
  field.bounds.width = 20.0f;
  field.bounds.height = 20.0f;
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  field.style = style_temp;
  field.bounds = temp_rect;
  backend.fail_draw_rect = 1;
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect = 0;
  backend.fail_draw = 1;
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_draw = 0;
  CMP_TEST_OK(cmp_text_field_set_selection(&field, 0, 2));
  CMP_TEST_OK(cmp_text_field_test_set_fail_point(
      CMP_TEXT_FIELD_TEST_FAIL_MEASURE_PREFIX));
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
  CMP_TEST_OK(cmp_text_field_set_text(&field, "", 0));
  field.label_value = 1.0f;
  gfx.vtable = &g_test_gfx_vtable_no_clip;
  CMP_TEST_OK(field.widget.vtable->paint(field.widget.ctx, &paint_ctx));
  gfx.vtable = &g_test_gfx_vtable;
  CMP_TEST_OK(cmp_text_field_set_text(&field, "AB", 2));

  {
    CMPTextField paint_field;

    paint_field = field;
    paint_field.widget.ctx = &paint_field;
    paint_field.text_metrics_valid = CMP_TRUE;
    paint_field.label_metrics_valid = CMP_TRUE;
    paint_field.placeholder_metrics_valid = CMP_TRUE;
    paint_field.font_metrics_valid = CMP_TRUE;
    paint_field.text_metrics.width = 10.0f;
    paint_field.label_metrics.width = 5.0f;
    paint_field.placeholder_metrics.width = 6.0f;
    paint_field.text_font_metrics.height = 12.0f;
    paint_field.text_font_metrics.baseline = 8.0f;
    paint_field.label_font_metrics.height = 12.0f;
    paint_field.label_font_metrics.baseline = 8.0f;
    paint_field.utf8_label = NULL;
    paint_field.label_len = 0u;
    paint_field.utf8_placeholder = NULL;
    paint_field.placeholder_len = 0u;
    paint_field.selection_start = 0u;
    paint_field.selection_end = 0u;
    paint_field.focused = CMP_FALSE;
    paint_field.cursor_visible = CMP_FALSE;

    paint_field.style.outline_width = 0.0f;
    paint_field.style.container_color.a = 0.0f;
    backend.draw_rect_calls = 0;
    backend.draw_calls = 0;
    gfx.vtable = &g_test_gfx_vtable;
    gfx.text_vtable = &g_test_text_vtable;
    CMP_TEST_OK(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx));

    paint_field.style.outline_width = 6.0f;
    paint_field.style.corner_radius = 2.0f;
    paint_field.bounds.width = 100.0f;
    paint_field.bounds.height = 40.0f;
    CMP_TEST_OK(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx));

    paint_field.style.outline_width = 0.0f;
    paint_field.style.padding_x = 10.0f;
    paint_field.style.padding_y = 10.0f;
    paint_field.bounds.width = 5.0f;
    paint_field.bounds.height = 5.0f;
    CMP_TEST_OK(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx));

    paint_field.bounds.width = 50.0f;
    paint_field.bounds.height = 20.0f;
    paint_field.style.padding_x = 0.0f;
    paint_field.style.padding_y = 0.0f;
    paint_field.text_font_metrics.height = -5.0f;
    paint_field.text_font_metrics.baseline = 0.0f;
    paint_field.selection_start = 2u;
    paint_field.selection_end = 0u;
    paint_field.style.selection_color.a = 1.0f;
    CMP_TEST_OK(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx));
    paint_field.text_font_metrics.height = 12.0f;
    paint_field.text_font_metrics.baseline = 8.0f;

    paint_field.selection_start = 0u;
    paint_field.selection_end = 0u;
    paint_field.style.selection_color.a = 0.0f;
    paint_field.style.handle_color.a = 0.0f;
    paint_field.utf8_len = 0u;
    paint_field.utf8_placeholder = "PH";
    paint_field.placeholder_len = 2u;
    paint_field.label_value = 1.0f;
    backend.fail_draw_after = 1;
    CMP_TEST_EXPECT(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx),
        CMP_ERR_IO);
    backend.fail_draw_after = 0;
    paint_field.utf8_placeholder = NULL;
    paint_field.placeholder_len = 0u;

    paint_field.utf8_label = "Lbl";
    paint_field.label_len = 3u;
    paint_field.label_font.id = 1u;
    paint_field.label_font.generation = 1u;
    backend.fail_draw_after = 1;
    CMP_TEST_EXPECT(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx),
        CMP_ERR_IO);
    backend.fail_draw_after = 0;
    paint_field.utf8_label = NULL;
    paint_field.label_len = 0u;

    paint_field.selection_start = 0u;
    paint_field.selection_end = 0u;
    paint_field.focused = CMP_FALSE;
    paint_field.cursor_visible = CMP_FALSE;
    paint_field.style.selection_color.a = 0.0f;
    paint_field.style.handle_color.a = 0.0f;
    paint_field.style.outline_width = 0.0f;
    paint_field.style.container_color.a = 1.0f;
    backend.fail_draw_rect_after = 1;
    CMP_TEST_EXPECT(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx),
        CMP_ERR_IO);
    backend.fail_draw_rect_after = 0;
    paint_field.style.container_color.a = 0.0f;

    paint_field.utf8 = "AB";
    paint_field.utf8_len = 2u;
    paint_field.selection_start = 0u;
    paint_field.selection_end = 2u;
    paint_field.style.selection_color.a = 1.0f;
    paint_field.style.handle_color.a = 0.0f;
    backend.fail_draw_rect_after = 1;
    CMP_TEST_EXPECT(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx),
        CMP_ERR_IO);
    backend.fail_draw_rect_after = 0;
    paint_field.style.selection_color.a = 0.0f;

    paint_field.selection_start = 0u;
    paint_field.selection_end = 5u;
    paint_field.style.handle_color.a = 1.0f;
    CMP_TEST_EXPECT(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx),
        CMP_ERR_RANGE);
    paint_field.selection_end = 2u;

    backend.fail_draw_rect_after = 2;
    CMP_TEST_EXPECT(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx),
        CMP_ERR_IO);
    backend.fail_draw_rect_after = 0;

    paint_field.selection_start = 0u;
    paint_field.selection_end = 0u;
    paint_field.cursor = 0u;
    paint_field.focused = CMP_TRUE;
    paint_field.cursor_visible = CMP_TRUE;
    paint_field.style.cursor_width = 1.0f;
    paint_field.style.selection_color.a = 0.0f;
    paint_field.style.handle_color.a = 0.0f;
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_MEASURE_PREFIX));
    CMP_TEST_EXPECT(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx),
        CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_CURSOR_WIDTH_NEGATIVE));
    CMP_TEST_OK(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx));
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_CURSOR_HEIGHT_NEGATIVE));
    CMP_TEST_OK(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx));
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
    backend.fail_draw_rect_after = 1;
    CMP_TEST_EXPECT(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx),
        CMP_ERR_IO);
    backend.fail_draw_rect_after = 0;
    paint_field.focused = CMP_FALSE;
    paint_field.cursor_visible = CMP_FALSE;

    paint_field.utf8 = "AB";
    paint_field.utf8_len = 2u;
    paint_field.cursor = 0u;
    paint_field.selection_start = 0u;
    paint_field.selection_end = 2u;
    paint_field.style.handle_radius = 6.0f;
    paint_field.style.handle_height = 4.0f;
    paint_field.style.handle_color.a = 1.0f;
    backend.fail_draw_rect_after = 2;
    CMP_TEST_EXPECT(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx),
        CMP_ERR_IO);
    backend.fail_draw_rect_after = 0;

    backend.draw_rect_calls = 0;
    backend.fail_draw_rect_after = 0;
    CMP_TEST_OK(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx));
    draw_rect_target = backend.draw_rect_calls;
    CMP_TEST_ASSERT(draw_rect_target > 0);
    backend.draw_rect_calls = 0;
    backend.fail_draw_rect_after = draw_rect_target;
    CMP_TEST_EXPECT(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx),
        CMP_ERR_IO);
    backend.fail_draw_rect_after = 0;

    paint_field.selection_start = 0u;
    paint_field.selection_end = 2u;
    paint_field.style.selection_color.a = 1.0f;
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_SELECTION_WIDTH_NEGATIVE));
    CMP_TEST_OK(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx));
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    paint_field.selection_start = 0u;
    paint_field.selection_end = 5u;
    paint_field.style.selection_color.a = 1.0f;
    paint_field.style.handle_color.a = 0.0f;
    CMP_TEST_EXPECT(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx),
        CMP_ERR_RANGE);
    paint_field.selection_start = 0u;
    paint_field.selection_end = 2u;
    paint_field.style.selection_color.a = 0.0f;
    paint_field.style.handle_color.a = 1.0f;
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_MEASURE_PREFIX));
    CMP_TEST_EXPECT(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx),
        CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    paint_field.selection_start = 0u;
    paint_field.selection_end = 0u;
    paint_field.style.selection_color.a = 0.0f;
    paint_field.style.handle_color.a = 0.0f;
    paint_field.cursor = 0u;
    paint_field.focused = CMP_TRUE;
    paint_field.cursor_visible = CMP_TRUE;
    paint_field.style.cursor_width = 1.0f;
    CMP_TEST_OK(
        paint_field.widget.vtable->paint(paint_field.widget.ctx, &paint_ctx));
    paint_field.focused = CMP_FALSE;
    paint_field.cursor_visible = CMP_FALSE;
  }

  backend.draw_rect_calls = 0;
  backend.draw_calls = 0;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = &g_test_text_vtable;
  CMP_TEST_OK(cmp_text_field_set_selection(&field, 0, 2));
  CMP_TEST_OK(cmp_text_field_set_focus(&field, CMP_TRUE));
  CMP_TEST_OK(field.widget.vtable->paint(field.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(backend.draw_rect_calls > 0);
  CMP_TEST_ASSERT(backend.draw_calls > 0);

  backend.fail_push_clip = 1;
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_push_clip = 0;
  backend.fail_pop_clip = 1;
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_pop_clip = 0;

  gfx.vtable = &g_test_gfx_vtable_no_draw;
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);

  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(field.widget.vtable->paint(field.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  CMP_TEST_EXPECT(cmp_text_field_step(NULL, 0.1f, &changed),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_step(&field, 0.1f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_field_step(&field, -0.1f, &changed), CMP_ERR_RANGE);
  field.label_anim.mode = 99u;
  field.label_anim.running = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_text_field_step(&field, 0.1f, &changed),
                  CMP_ERR_INVALID_ARGUMENT);
  field.label_anim.mode = CMP_ANIM_MODE_NONE;
  field.label_anim.running = CMP_FALSE;
  field.focused = CMP_TRUE;
  field.style.cursor_blink_period = 0.1f;
  field.cursor_timer = 0.0f;
  field.cursor_visible = CMP_TRUE;
  CMP_TEST_OK(cmp_text_field_step(&field, 0.25f, &changed));
  field.style.cursor_blink_period = 0.0f;
  field.cursor_visible = CMP_FALSE;
  CMP_TEST_OK(cmp_text_field_step(&field, 0.1f, &changed));
  field.focused = CMP_FALSE;
  field.cursor_visible = CMP_TRUE;
  CMP_TEST_OK(cmp_text_field_step(&field, 0.1f, &changed));
  CMP_TEST_OK(cmp_text_field_step(&field, 0.1f, &changed));
  CMP_TEST_ASSERT(changed == CMP_TRUE || changed == CMP_FALSE);
  CMP_TEST_OK(cmp_anim_controller_start_timing(&field.label_anim, 0.0f, 1.0f,
                                               1.0f, CMP_ANIM_EASE_LINEAR));
  field.label_value = -1.0f;
  CMP_TEST_OK(cmp_text_field_step(&field, 0.1f, &changed));
  CMP_TEST_ASSERT(changed == CMP_TRUE);

  CMP_TEST_EXPECT(cmp_text_field_set_on_change(NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(
      cmp_text_field_set_on_change(&field, test_on_change, &on_change_state));
  CMP_TEST_OK(cmp_text_field_set_on_change(&field, NULL, NULL));
  CMP_TEST_OK(cmp_text_field_set_focus(&field, CMP_FALSE));
  CMP_TEST_OK(cmp_text_field_step(&field, 0.2f, &changed));

  {
    CMPTextField coverage_field;
    CMPTextField reserve_local;
    CMPInputEvent event_local;
    CMPSemantics sem_local;
    cmp_usize out_offset_local;
    cmp_usize max_local;
    int rc_local;

    CMP_TEST_EXPECT(cmp_text_field_test_usize_max(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_field_test_usize_max(&max_local));
    CMP_TEST_ASSERT(max_local > 0u);

    CMP_TEST_OK(cmp_text_field_test_set_offset_skip_early(CMP_TRUE));
    CMP_TEST_OK(cmp_text_field_test_validate_offset("A", 1u, 1u));
    CMP_TEST_OK(cmp_text_field_test_set_offset_skip_early(CMP_FALSE));
    CMP_TEST_OK(cmp_text_field_test_validate_offset("A", 1u, 0u));
    CMP_TEST_EXPECT(cmp_text_field_test_validate_offset("A", 1u, 2u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_UTF8_VALIDATE));
    CMP_TEST_EXPECT(cmp_text_field_test_validate_utf8("A", 1u), CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_INIT));
    CMP_TEST_EXPECT(
        cmp_text_field_test_prev_offset("A", 1u, 1u, &out_offset_local),
        CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NEXT));
    CMP_TEST_EXPECT(
        cmp_text_field_test_prev_offset("A", 1u, 1u, &out_offset_local),
        CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_UTF8_ITER_NOT_FOUND));
    CMP_TEST_OK(
        cmp_text_field_test_next_offset("A", 1u, 0u, &out_offset_local));
    CMP_TEST_ASSERT(out_offset_local == 1u);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    memset(&reserve_local, 0, sizeof(reserve_local));
    reserve_local.allocator = custom_alloc;
    CMP_TEST_EXPECT(cmp_text_field_test_reserve(NULL, 1u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_text_field_test_reserve(&reserve_local, 0u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_field_test_reserve(&reserve_local, 4u));
    reserve_local.utf8_capacity = 8u;
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_RESERVE_BYPASS));
    CMP_TEST_OK(cmp_text_field_test_reserve(&reserve_local, 4u));
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_RESERVE_REALLOC));
    CMP_TEST_EXPECT(cmp_text_field_test_reserve(&reserve_local, 32u),
                    CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());
    if (reserve_local.utf8 != NULL && reserve_local.allocator.free != NULL) {
      reserve_local.allocator.free(reserve_local.allocator.ctx,
                                   reserve_local.utf8);
      reserve_local.utf8 = NULL;
    }

    CMP_TEST_OK(cmp_text_field_init(&coverage_field, &text_backend,
                                    &style_with_label, NULL, "Hi", 2u));
    coverage_field.bounds.x = 0.0f;
    coverage_field.bounds.y = 0.0f;
    coverage_field.bounds.width = 200.0f;
    coverage_field.bounds.height = 60.0f;
    coverage_field.text_metrics_valid = CMP_FALSE;
    CMP_TEST_OK(cmp_text_field_test_update_text_metrics(&coverage_field));
    coverage_field.text_metrics_valid = CMP_FALSE;
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
    CMP_TEST_EXPECT(cmp_text_field_test_update_text_metrics(&coverage_field),
                    CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    coverage_field.utf8_label = "Lbl";
    coverage_field.label_len = 3u;
    coverage_field.label_font = coverage_field.text_font;
    coverage_field.label_metrics_valid = CMP_FALSE;
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
    CMP_TEST_EXPECT(cmp_text_field_test_update_label_metrics(&coverage_field),
                    CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    coverage_field.utf8_placeholder = "PH";
    coverage_field.placeholder_len = 2u;
    coverage_field.placeholder_metrics_valid = CMP_FALSE;
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_TEXT_MEASURE));
    CMP_TEST_EXPECT(
        cmp_text_field_test_update_placeholder_metrics(&coverage_field),
        CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    coverage_field.font_metrics_valid = CMP_FALSE;
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_FONT_METRICS));
    CMP_TEST_EXPECT(cmp_text_field_test_update_font_metrics(&coverage_field),
                    CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    coverage_field.focused = CMP_FALSE;
    coverage_field.label_value = 0.0f;
    CMP_TEST_OK(cmp_text_field_test_sync_label(&coverage_field));
    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_ANIM_START));
    CMP_TEST_EXPECT(cmp_text_field_test_sync_label(&coverage_field),
                    CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    CMP_TEST_EXPECT(cmp_text_field_set_focus(&coverage_field, 2),
                    CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_text_field_set_focus(&coverage_field, CMP_TRUE));

    coverage_field.selection_start = 0u;
    coverage_field.selection_end = 1u;
    CMP_TEST_OK(cmp_text_field_backspace(&coverage_field));
    coverage_field.selection_start = 0u;
    coverage_field.selection_end = 1u;
    CMP_TEST_OK(cmp_text_field_delete_forward(&coverage_field));
    coverage_field.cursor = 0u;
    CMP_TEST_OK(cmp_text_field_backspace(&coverage_field));
    coverage_field.cursor = coverage_field.utf8_len;
    CMP_TEST_OK(cmp_text_field_delete_forward(&coverage_field));
    CMP_TEST_OK(cmp_text_field_set_text(&coverage_field, "Hi", 2u));

    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_OUTLINE_RANGE));
    rc_local = coverage_field.widget.vtable->paint(coverage_field.widget.ctx,
                                                   &paint_ctx);
    if (rc_local != CMP_OK) {
      CMP_TEST_ASSERT(rc_local == CMP_ERR_RANGE);
    }
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    CMP_TEST_OK(cmp_text_field_test_set_fail_point(
        CMP_TEXT_FIELD_TEST_FAIL_SELECTION_WIDTH_NEGATIVE));
    coverage_field.selection_start = 0u;
    coverage_field.selection_end = 1u;
    CMP_TEST_OK(coverage_field.widget.vtable->paint(coverage_field.widget.ctx,
                                                    &paint_ctx));
    CMP_TEST_OK(cmp_text_field_test_clear_fail_points());

    event_local.type = CMP_INPUT_POINTER_MOVE;
    event_local.data.pointer.x = 1;
    event_local.data.pointer.y = 1;
    coverage_field.selecting = CMP_FALSE;
    handled = CMP_FALSE;
    CMP_TEST_OK(coverage_field.widget.vtable->event(coverage_field.widget.ctx,
                                                    &event_local, &handled));

    event_local.type = CMP_INPUT_POINTER_UP;
    CMP_TEST_OK(coverage_field.widget.vtable->event(coverage_field.widget.ctx,
                                                    &event_local, &handled));

    event_local.type = CMP_INPUT_TEXT_EDIT;
    CMP_TEST_OK(coverage_field.widget.vtable->event(coverage_field.widget.ctx,
                                                    &event_local, &handled));

    coverage_field.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
    CMP_TEST_OK(coverage_field.widget.vtable->get_semantics(
        coverage_field.widget.ctx, &sem_local));

    CMP_TEST_OK(
        coverage_field.widget.vtable->destroy(coverage_field.widget.ctx));
  }

  CMP_TEST_EXPECT(field.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_field_init(&destroy_field, &text_backend,
                                  &style_with_label, NULL, "", 0));
  backend.fail_destroy = 1;
  CMP_TEST_EXPECT(
      destroy_field.widget.vtable->destroy(destroy_field.widget.ctx),
      CMP_ERR_IO);
  backend.fail_destroy = 0;
  backend.destroy_calls = 0;
  backend.fail_destroy_after = 2;
  CMP_TEST_OK(cmp_text_field_init(&destroy_field, &text_backend,
                                  &style_with_label, NULL, "", 0));
  CMP_TEST_EXPECT(
      destroy_field.widget.vtable->destroy(destroy_field.widget.ctx),
      CMP_ERR_IO);
  backend.fail_destroy_after = 0;
  CMP_TEST_OK(cmp_text_field_init(&destroy_field_no_destroy, &text_backend,
                                  &style_with_label, NULL, "", 0));
  destroy_field_no_destroy.text_backend.vtable = &g_test_text_vtable_no_destroy;
  CMP_TEST_EXPECT(destroy_field_no_destroy.widget.vtable->destroy(
                      destroy_field_no_destroy.widget.ctx),
                  CMP_ERR_UNSUPPORTED);

  CMP_TEST_OK(cmp_text_field_init(&destroy_field_no_destroy, &text_backend,
                                  &style_with_label, NULL, "", 0));
  destroy_field_no_destroy.text_font.id = 0u;
  destroy_field_no_destroy.text_font.generation = 0u;
  destroy_field_no_destroy.text_backend.vtable = &g_test_text_vtable_no_destroy;
  CMP_TEST_EXPECT(destroy_field_no_destroy.widget.vtable->destroy(
                      destroy_field_no_destroy.widget.ctx),
                  CMP_ERR_UNSUPPORTED);

  CMP_TEST_OK(field.widget.vtable->destroy(field.widget.ctx));
  return 0;
}
