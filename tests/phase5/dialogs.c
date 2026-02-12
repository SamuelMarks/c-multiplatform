#include "m3/m3_dialogs.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestDialogBackend {
  int create_calls;
  int destroy_calls;
  int measure_calls;
  int draw_calls;
  int draw_rect_calls;
  int fail_create;
  int fail_create_at;
  int fail_destroy;
  int fail_destroy_at;
  int fail_measure;
  int fail_draw;
  int fail_draw_rect;
  M3Handle last_font;
  M3Rect last_rect;
  M3Color last_rect_color;
  M3Scalar last_corner;
  M3Scalar last_text_x;
  M3Scalar last_text_y;
  m3_usize last_text_len;
  M3Color last_text_color;
} TestDialogBackend;

typedef struct TestActionState {
  int calls;
  m3_u32 last_action;
  int fail_next;
} TestActionState;

static void test_backend_init(TestDialogBackend *backend) {
  if (backend == NULL) {
    return;
  }
  memset(backend, 0, sizeof(*backend));
  backend->fail_create = M3_OK;
  backend->fail_destroy = M3_OK;
  backend->fail_measure = M3_OK;
  backend->fail_draw = M3_OK;
  backend->fail_draw_rect = M3_OK;
  backend->fail_create_at = 0;
  backend->fail_destroy_at = 0;
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 m3_i32 size_px, m3_i32 weight, M3Bool italic,
                                 M3Handle *out_font) {
  TestDialogBackend *backend;

  if (text == NULL || utf8_family == NULL || out_font == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size_px <= 0) {
    return M3_ERR_RANGE;
  }
  if (weight < 100 || weight > 900) {
    return M3_ERR_RANGE;
  }
  if (italic != M3_FALSE && italic != M3_TRUE) {
    return M3_ERR_RANGE;
  }

  backend = (TestDialogBackend *)text;
  backend->create_calls += 1;
  if (backend->fail_create_at > 0 &&
      backend->create_calls == backend->fail_create_at) {
    return M3_ERR_IO;
  }
  if (backend->fail_create != M3_OK) {
    return backend->fail_create;
  }

  out_font->id = (m3_u32)backend->create_calls;
  out_font->generation = (m3_u32)backend->create_calls;
  backend->last_font = *out_font;
  return M3_OK;
}

static int test_text_destroy_font(void *text, M3Handle font) {
  TestDialogBackend *backend;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestDialogBackend *)text;
  backend->destroy_calls += 1;
  if (backend->fail_destroy_at > 0 &&
      backend->destroy_calls == backend->fail_destroy_at) {
    return M3_ERR_IO;
  }
  if (backend->fail_destroy != M3_OK) {
    return backend->fail_destroy;
  }
  return M3_OK;
}

static int test_text_measure_text(void *text, M3Handle font, const char *utf8,
                                  m3_usize utf8_len, M3Scalar *out_width,
                                  M3Scalar *out_height,
                                  M3Scalar *out_baseline) {
  TestDialogBackend *backend;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_STATE;
  }

  backend = (TestDialogBackend *)text;
  backend->measure_calls += 1;
  if (backend->fail_measure != M3_OK) {
    return backend->fail_measure;
  }

  *out_width = (M3Scalar)(utf8_len * 8u);
  *out_height = 20.0f;
  *out_baseline = 14.0f;
  return M3_OK;
}

static int test_text_draw_text(void *text, M3Handle font, const char *utf8,
                               m3_usize utf8_len, M3Scalar x, M3Scalar y,
                               M3Color color) {
  TestDialogBackend *backend;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_STATE;
  }

  backend = (TestDialogBackend *)text;
  backend->draw_calls += 1;
  if (backend->fail_draw != M3_OK) {
    return backend->fail_draw;
  }

  backend->last_text_x = x;
  backend->last_text_y = y;
  backend->last_text_len = utf8_len;
  backend->last_text_color = color;
  return M3_OK;
}

static int test_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color,
                              M3Scalar corner_radius) {
  TestDialogBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestDialogBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect != M3_OK) {
    return backend->fail_draw_rect;
  }
  return M3_OK;
}

static const M3TextVTable g_test_text_vtable = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    test_text_draw_text};

static const M3TextVTable g_test_text_vtable_no_draw = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    NULL};

static const M3TextVTable g_test_text_vtable_no_measure = {
    test_text_create_font, test_text_destroy_font, NULL, test_text_draw_text};

static const M3TextVTable g_test_text_vtable_no_create = {
    NULL, test_text_destroy_font, test_text_measure_text, test_text_draw_text};

static const M3GfxVTable g_test_gfx_vtable = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static const M3GfxVTable g_test_gfx_vtable_no_draw = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL};

static void test_setup_text_backend(TestDialogBackend *backend,
                                    M3TextBackend *out_backend,
                                    const M3TextVTable *vtable) {
  out_backend->ctx = backend;
  out_backend->vtable = vtable;
}

static void test_setup_gfx_backend(TestDialogBackend *backend, M3Gfx *out_gfx,
                                   const M3GfxVTable *vtable,
                                   const M3TextVTable *text_vtable) {
  out_gfx->ctx = backend;
  out_gfx->vtable = vtable;
  out_gfx->text_vtable = text_vtable;
}

static int test_alert_action(void *ctx, struct M3AlertDialog *dialog,
                             m3_u32 action_id) {
  TestActionState *state;

  if (ctx == NULL || dialog == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestActionState *)ctx;
  state->calls += 1;
  state->last_action = action_id;
  if (state->fail_next) {
    state->fail_next = 0;
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_fullscreen_action(void *ctx,
                                  struct M3FullScreenDialog *dialog) {
  TestActionState *state;

  if (ctx == NULL || dialog == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestActionState *)ctx;
  state->calls += 1;
  state->last_action = 1u;
  if (state->fail_next) {
    state->fail_next = 0;
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_snackbar_action(void *ctx, struct M3Snackbar *snackbar) {
  TestActionState *state;

  if (ctx == NULL || snackbar == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestActionState *)ctx;
  state->calls += 1;
  state->last_action = 1u;
  if (state->fail_next) {
    state->fail_next = 0;
    return M3_ERR_IO;
  }
  return M3_OK;
}

static void test_set_alert_families(M3AlertDialogStyle *style,
                                    const char *family) {
  style->title_style.utf8_family = family;
  style->body_style.utf8_family = family;
  style->action_style.utf8_family = family;
}

static void test_set_fullscreen_families(M3FullScreenDialogStyle *style,
                                         const char *family) {
  style->title_style.utf8_family = family;
  style->body_style.utf8_family = family;
  style->action_style.utf8_family = family;
}

static void test_set_snackbar_families(M3SnackbarStyle *style,
                                       const char *family) {
  style->message_style.utf8_family = family;
  style->action_style.utf8_family = family;
}

static int test_dialog_helpers(void) {
  M3Rect rect;
  M3Color color;
  M3TextStyle text_style;
  M3LayoutEdges edges;
  M3MeasureSpec spec;
  M3TextBackend backend;
  M3Bool inside;

  M3_TEST_EXPECT(m3_dialog_test_point_in_rect(NULL, 0.0f, 0.0f, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_dialog_test_point_in_rect(NULL, 0.0f, 0.0f, &inside));
  M3_TEST_ASSERT(inside == M3_FALSE);

  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = 0.0f;
  rect.height = 10.0f;
  M3_TEST_OK(m3_dialog_test_point_in_rect(&rect, 0.0f, 0.0f, &inside));
  M3_TEST_ASSERT(inside == M3_FALSE);

  rect.width = 10.0f;
  rect.height = 10.0f;
  M3_TEST_OK(m3_dialog_test_point_in_rect(&rect, 5.0f, 5.0f, &inside));
  M3_TEST_ASSERT(inside == M3_TRUE);
  M3_TEST_OK(m3_dialog_test_point_in_rect(&rect, -1.0f, 5.0f, &inside));
  M3_TEST_ASSERT(inside == M3_FALSE);
  M3_TEST_OK(m3_dialog_test_point_in_rect(&rect, 5.0f, -1.0f, &inside));
  M3_TEST_ASSERT(inside == M3_FALSE);
  M3_TEST_OK(m3_dialog_test_point_in_rect(&rect, 20.0f, 5.0f, &inside));
  M3_TEST_ASSERT(inside == M3_FALSE);
  M3_TEST_OK(m3_dialog_test_point_in_rect(&rect, 5.0f, 20.0f, &inside));
  M3_TEST_ASSERT(inside == M3_FALSE);

  M3_TEST_EXPECT(m3_dialog_test_validate_color(NULL), M3_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  M3_TEST_EXPECT(m3_dialog_test_validate_color(&color), M3_ERR_RANGE);
  color.r = 0.0f;
  color.g = 2.0f;
  M3_TEST_EXPECT(m3_dialog_test_validate_color(&color), M3_ERR_RANGE);
  color.g = 0.0f;
  color.b = 2.0f;
  M3_TEST_EXPECT(m3_dialog_test_validate_color(&color), M3_ERR_RANGE);
  color.b = 0.0f;
  color.a = 2.0f;
  M3_TEST_EXPECT(m3_dialog_test_validate_color(&color), M3_ERR_RANGE);
  color.a = 1.0f;
  M3_TEST_OK(m3_dialog_test_validate_color(&color));

  M3_TEST_EXPECT(m3_dialog_test_validate_text_style(NULL, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_text_style_init(&text_style));
  text_style.utf8_family = NULL;
  M3_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  text_style.utf8_family = "Test";
  text_style.size_px = 0;
  M3_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.size_px = 12;
  text_style.weight = 50;
  M3_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.weight = 400;
  text_style.italic = 3;
  M3_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.italic = M3_FALSE;
  text_style.color.r = -0.5f;
  M3_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.color.r = 0.0f;
  M3_TEST_OK(m3_dialog_test_validate_text_style(&text_style, M3_TRUE));

  M3_TEST_EXPECT(m3_dialog_test_validate_edges(NULL), M3_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.right = 0.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  M3_TEST_EXPECT(m3_dialog_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.left = 0.0f;
  edges.bottom = -1.0f;
  M3_TEST_EXPECT(m3_dialog_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.bottom = 0.0f;
  M3_TEST_OK(m3_dialog_test_validate_edges(&edges));

  spec.mode = 99u;
  spec.size = 0.0f;
  M3_TEST_EXPECT(m3_dialog_test_validate_measure_spec(spec),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_AT_MOST;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_dialog_test_validate_measure_spec(spec), M3_ERR_RANGE);
  spec.size = 0.0f;
  M3_TEST_OK(m3_dialog_test_validate_measure_spec(spec));

  M3_TEST_EXPECT(m3_dialog_test_validate_rect(NULL), M3_ERR_INVALID_ARGUMENT);
  rect.width = -1.0f;
  rect.height = 0.0f;
  M3_TEST_EXPECT(m3_dialog_test_validate_rect(&rect), M3_ERR_RANGE);
  rect.width = 0.0f;
  rect.height = 0.0f;
  M3_TEST_OK(m3_dialog_test_validate_rect(&rect));

  backend.ctx = NULL;
  backend.vtable = NULL;
  M3_TEST_EXPECT(m3_dialog_test_validate_backend(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_dialog_test_validate_backend(&backend),
                 M3_ERR_INVALID_ARGUMENT);

  return 0;
}

static int test_alert_dialog(void) {
  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);

    family = "Test";

    M3_TEST_EXPECT(m3_alert_dialog_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);

    M3_TEST_EXPECT(
        m3_alert_dialog_init(NULL, &text_backend, &style, NULL, 0u, NULL, 0u),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_alert_dialog_init(&dialog, NULL, &style, NULL, 0u, NULL, 0u),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_alert_dialog_init(&dialog, &text_backend, NULL, NULL, 0u, NULL, 0u),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        1u, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 1u),
                   M3_ERR_INVALID_ARGUMENT);

    text_backend.vtable = NULL;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);
    test_setup_text_backend(&backend, &text_backend,
                            &g_test_text_vtable_no_create);
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_UNSUPPORTED);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.corner_radius = -1.0f;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.min_width = 100.0f;
    style.max_width = 10.0f;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.title_body_spacing = -1.0f;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.shadow_enabled = 5;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.scrim_enabled = 5;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.padding.left = -1.0f;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.title_style.utf8_family = NULL;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.body_style.size_px = 0;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.action_style.weight = 50;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.title_style.italic = 3;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.title_style.color.r = -0.1f;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.background_color.r = 1.5f;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.scrim_color.a = -0.1f;
    M3_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                        0u, NULL, 0u),
                   M3_ERR_RANGE);
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    M3MeasureSpec width_spec;
    M3MeasureSpec height_spec;
    M3Size size;
    const char *family;
    const char *title;
    m3_usize title_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend,
                            &g_test_text_vtable_no_measure);
    family = "Test";
    title = "T";
    title_len = (m3_usize)strlen(title);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    M3_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, title,
                                    title_len, NULL, 0u));

    width_spec.mode = M3_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec = width_spec;
    M3_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                 height_spec, &size),
                   M3_ERR_UNSUPPORTED);
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3Gfx gfx;
    M3PaintContext ctx;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    M3MeasureSpec width_spec;
    M3MeasureSpec height_spec;
    M3Size size;
    M3Rect bounds;
    M3InputEvent event;
    M3Bool handled;
    TestActionState state;
    const char *family;
    const char *title;
    const char *body;
    const char *confirm;
    const char *dismiss;
    m3_usize title_len;
    m3_usize body_len;
    m3_usize confirm_len;
    m3_usize dismiss_len;
    int measure_before;
    int calls_before;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);

    family = "Test";
    title = "Title";
    body = "Body";
    confirm = "OK";
    dismiss = "Cancel";
    title_len = (m3_usize)strlen(title);
    body_len = (m3_usize)strlen(body);
    confirm_len = (m3_usize)strlen(confirm);
    dismiss_len = (m3_usize)strlen(dismiss);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    M3_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, title,
                                    title_len, body, body_len));

    M3_TEST_EXPECT(m3_alert_dialog_set_title(&dialog, NULL, 1u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_alert_dialog_set_body(&dialog, NULL, 1u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_alert_dialog_set_actions(&dialog, NULL, 1u, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_alert_dialog_set_actions(&dialog, confirm, confirm_len, NULL, 1u),
        M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_alert_dialog_set_actions(&dialog, confirm, confirm_len,
                                           dismiss, dismiss_len));

    width_spec.mode = 99u;
    width_spec.size = 10.0f;
    height_spec = width_spec;
    M3_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                 height_spec, &size),
                   M3_ERR_INVALID_ARGUMENT);

    width_spec.mode = M3_MEASURE_EXACTLY;
    width_spec.size = -1.0f;
    height_spec = width_spec;
    M3_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                 height_spec, &size),
                   M3_ERR_RANGE);

    width_spec.mode = M3_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec = width_spec;
    M3_TEST_EXPECT(
        dialog.widget.vtable->measure(NULL, width_spec, height_spec, &size),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                 height_spec, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                             height_spec, &size));

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = 200.0f;
    height_spec = width_spec;
    M3_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                             height_spec, &size));

    width_spec.mode = M3_MEASURE_EXACTLY;
    width_spec.size = 320.0f;
    height_spec.mode = M3_MEASURE_EXACTLY;
    height_spec.size = 200.0f;
    M3_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                             height_spec, &size));

    measure_before = backend.measure_calls;
    M3_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                             height_spec, &size));
    M3_TEST_ASSERT(backend.measure_calls == measure_before);

    M3_TEST_OK(m3_alert_dialog_set_title(&dialog, title, title_len));
    M3_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                             height_spec, &size));
    M3_TEST_ASSERT(backend.measure_calls > measure_before);

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -10.0f;
    bounds.height = 20.0f;
    M3_TEST_EXPECT(dialog.widget.vtable->layout(dialog.widget.ctx, bounds),
                   M3_ERR_RANGE);
    M3_TEST_EXPECT(dialog.widget.vtable->layout(NULL, bounds),
                   M3_ERR_INVALID_ARGUMENT);

    bounds.width = 300.0f;
    bounds.height = 200.0f;
    M3_TEST_OK(
        m3_alert_dialog_set_actions(&dialog, confirm, confirm_len, NULL, 0u));
    M3_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    M3_TEST_ASSERT(dialog.confirm_bounds.width > 0.0f);
    M3_TEST_ASSERT(dialog.dismiss_bounds.width == 0.0f);

    M3_TEST_OK(
        m3_alert_dialog_set_actions(&dialog, NULL, 0u, dismiss, dismiss_len));
    M3_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    M3_TEST_ASSERT(dialog.dismiss_bounds.width > 0.0f);
    M3_TEST_ASSERT(dialog.confirm_bounds.width == 0.0f);

    M3_TEST_OK(m3_alert_dialog_set_actions(&dialog, confirm, confirm_len,
                                           dismiss, dismiss_len));
    M3_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    M3_TEST_ASSERT(dialog.dismiss_bounds.width > 0.0f);
    M3_TEST_ASSERT(dialog.confirm_bounds.width > 0.0f);

    M3_TEST_OK(m3_alert_dialog_set_actions(&dialog, NULL, 0u, NULL, 0u));
    M3_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    M3_TEST_ASSERT(dialog.confirm_bounds.width == 0.0f);
    M3_TEST_ASSERT(dialog.dismiss_bounds.width == 0.0f);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    ctx.clip = bounds;
    ctx.dpi_scale = 1.0f;
    M3_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                   M3_ERR_UNSUPPORTED);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable_no_draw);
    M3_TEST_OK(m3_alert_dialog_set_actions(&dialog, confirm, confirm_len,
                                           dismiss, dismiss_len));
    M3_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                   M3_ERR_UNSUPPORTED);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable, NULL);
    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.utf8_body = NULL;
    dialog.body_len = 0u;
    dialog.utf8_confirm = NULL;
    dialog.confirm_len = 0u;
    dialog.utf8_dismiss = NULL;
    dialog.dismiss_len = 0u;
    dialog.metrics_valid = M3_FALSE;
    M3_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));

    dialog.utf8_title = title;
    dialog.title_len = title_len;
    dialog.utf8_body = body;
    dialog.body_len = body_len;
    dialog.utf8_confirm = confirm;
    dialog.confirm_len = confirm_len;
    dialog.utf8_dismiss = dismiss;
    dialog.dismiss_len = dismiss_len;
    dialog.metrics_valid = M3_FALSE;

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    M3_TEST_ASSERT(ctx.gfx == &gfx);
    M3_TEST_ASSERT(gfx.vtable == &g_test_gfx_vtable);
    M3_TEST_ASSERT(gfx.vtable != NULL);
    M3_TEST_ASSERT(gfx.vtable->draw_rect != NULL);
    M3_TEST_ASSERT(gfx.text_vtable != NULL);
    M3_TEST_ASSERT(gfx.text_vtable->draw_text != NULL);
    M3_TEST_ASSERT(dialog.text_backend.vtable != NULL);
    M3_TEST_ASSERT(dialog.text_backend.vtable->measure_text != NULL);
    M3_TEST_ASSERT(dialog.title_len > 0u);
    M3_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));

    M3_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    ctx.gfx = NULL;
    M3_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                   M3_ERR_INVALID_ARGUMENT);
    ctx.gfx = &gfx;
    gfx.vtable = NULL;
    M3_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                   M3_ERR_INVALID_ARGUMENT);
    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);

    M3_TEST_OK(m3_alert_dialog_set_actions(&dialog, confirm, confirm_len,
                                           dismiss, dismiss_len));
    M3_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));

    memset(&event, 0, sizeof(event));
    event.type = M3_INPUT_POINTER_DOWN;
    event.data.pointer.x = (m3_i32)(dialog.confirm_bounds.x + 1.0f);
    event.data.pointer.y = (m3_i32)(dialog.confirm_bounds.y + 1.0f);
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);

    M3_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled),
        M3_ERR_STATE);

    memset(&state, 0, sizeof(state));
    M3_TEST_OK(
        m3_alert_dialog_set_on_action(&dialog, test_alert_action, &state));

    event.type = M3_INPUT_POINTER_UP;
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(state.calls == 1);
    M3_TEST_ASSERT(state.last_action == M3_ALERT_DIALOG_ACTION_CONFIRM);

    event.type = M3_INPUT_POINTER_DOWN;
    event.data.pointer.x = (m3_i32)(dialog.dismiss_bounds.x + 1.0f);
    event.data.pointer.y = (m3_i32)(dialog.dismiss_bounds.y + 1.0f);
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));

    state.fail_next = 1;
    event.type = M3_INPUT_POINTER_UP;
    M3_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled),
        M3_ERR_IO);

    event.type = M3_INPUT_POINTER_DOWN;
    event.data.pointer.x = (m3_i32)(bounds.x - 5.0f);
    event.data.pointer.y = (m3_i32)(bounds.y - 5.0f);
    calls_before = state.calls;
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    event.type = M3_INPUT_POINTER_DOWN;
    event.data.pointer.x = (m3_i32)(dialog.confirm_bounds.x + 1.0f);
    event.data.pointer.y = (m3_i32)(dialog.confirm_bounds.y + 1.0f);
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    event.type = M3_INPUT_POINTER_UP;
    event.data.pointer.x = (m3_i32)(bounds.x - 5.0f);
    event.data.pointer.y = (m3_i32)(bounds.y - 5.0f);
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(state.calls == calls_before);

    dialog.widget.flags = M3_WIDGET_FLAG_DISABLED;
    event.type = M3_INPUT_POINTER_DOWN;
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    dialog.widget.flags = 0u;

    event.type = M3_INPUT_KEY_DOWN;
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    M3_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, NULL, &handled),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(dialog.widget.vtable->event(NULL, &event, &handled),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(dialog.widget.vtable->event(dialog.widget.ctx, &event, NULL),
                   M3_ERR_INVALID_ARGUMENT);

    {
      M3Semantics semantics;
      M3_TEST_EXPECT(dialog.widget.vtable->get_semantics(NULL, &semantics),
                     M3_ERR_INVALID_ARGUMENT);
      M3_TEST_EXPECT(
          dialog.widget.vtable->get_semantics(dialog.widget.ctx, NULL),
          M3_ERR_INVALID_ARGUMENT);
      M3_TEST_OK(
          dialog.widget.vtable->get_semantics(dialog.widget.ctx, &semantics));
      M3_TEST_ASSERT(semantics.utf8_label == dialog.utf8_title);
    }

    M3_TEST_EXPECT(dialog.widget.vtable->destroy(NULL),
                   M3_ERR_INVALID_ARGUMENT);

    backend.fail_destroy = M3_ERR_IO;
    M3_TEST_EXPECT(dialog.widget.vtable->destroy(dialog.widget.ctx), M3_ERR_IO);
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3Gfx gfx;
    M3PaintContext ctx;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    M3Rect bounds;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    M3_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL, 0u,
                                    NULL, 0u));

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = 100.0f;
    bounds.height = 100.0f;
    M3_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable, NULL);
    ctx.gfx = &gfx;
    ctx.clip = bounds;
    ctx.dpi_scale = 1.0f;
    M3_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    const char *family;
    const char *title;
    m3_usize title_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    title = "Title";
    title_len = (m3_usize)strlen(title);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    M3_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, title,
                                    title_len, NULL, 0u));

    dialog.text_backend.vtable = NULL;
    dialog.owns_fonts = M3_TRUE;
    M3_TEST_EXPECT(dialog.widget.vtable->destroy(dialog.widget.ctx),
                   M3_ERR_INVALID_ARGUMENT);
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    const char *family;
    const char *title;
    m3_usize title_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    title = "Title";
    title_len = (m3_usize)strlen(title);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    M3_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, title,
                                    title_len, NULL, 0u));

    dialog.body_font.id = 0u;
    dialog.body_font.generation = 0u;
    M3_TEST_OK(dialog.widget.vtable->destroy(dialog.widget.ctx));
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    const char *family;
    const char *title;
    m3_usize title_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    title = "Title";
    title_len = (m3_usize)strlen(title);

    M3_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    M3_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, title,
                                    title_len, NULL, 0u));

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 1;
    M3_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 0;
    M3_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 1;
    M3_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 3;
    backend.fail_destroy_at = 0;
    M3_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 3;
    backend.fail_destroy_at = 1;
    M3_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 0;
    backend.fail_destroy_at = 1;
    M3_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_destroy_at = 0;
    M3_TEST_OK(m3_alert_dialog_set_style(&dialog, &style));

    M3_TEST_EXPECT(m3_alert_dialog_set_style(NULL, &style),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, NULL),
                   M3_ERR_INVALID_ARGUMENT);
  }

  return 0;
}

static int test_fullscreen_dialog(void) {
  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3FullScreenDialogStyle style;
    M3FullScreenDialog dialog;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    M3_TEST_EXPECT(m3_fullscreen_dialog_style_init(NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);

    M3_TEST_EXPECT(m3_fullscreen_dialog_init(NULL, &text_backend, &style, NULL,
                                             0u, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_fullscreen_dialog_init(&dialog, NULL, &style, NULL, 0u, NULL, 0u),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, NULL, NULL,
                                             0u, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 1u, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 0u, NULL, 1u),
                   M3_ERR_INVALID_ARGUMENT);

    text_backend.vtable = NULL;
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 0u, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3FullScreenDialogStyle style;
    M3FullScreenDialog dialog;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.corner_radius = -1.0f;
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.title_body_spacing = -1.0f;
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.shadow_enabled = 5;
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.scrim_enabled = 5;
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.padding.left = -1.0f;
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.title_style.utf8_family = NULL;
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 0u, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.body_style.size_px = 0;
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.action_style.weight = 50;
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.background_color.r = 1.5f;
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 0u, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.scrim_color.a = -0.1f;
    M3_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                             NULL, 0u, NULL, 0u),
                   M3_ERR_RANGE);
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3Gfx gfx;
    M3PaintContext ctx;
    M3FullScreenDialogStyle style;
    M3FullScreenDialog dialog;
    M3MeasureSpec width_spec;
    M3MeasureSpec height_spec;
    M3Size size;
    M3Rect bounds;
    M3InputEvent event;
    M3Bool handled;
    TestActionState state;
    const char *family;
    const char *title;
    const char *body;
    const char *action;
    m3_usize title_len;
    m3_usize body_len;
    m3_usize action_len;
    int measure_before;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    title = "Title";
    body = "Body";
    action = "Done";
    title_len = (m3_usize)strlen(title);
    body_len = (m3_usize)strlen(body);
    action_len = (m3_usize)strlen(action);

    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    M3_TEST_OK(m3_fullscreen_dialog_init(&dialog, &text_backend, &style, title,
                                         title_len, body, body_len));

    M3_TEST_EXPECT(m3_fullscreen_dialog_set_title(&dialog, NULL, 1u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_fullscreen_dialog_set_body(&dialog, NULL, 1u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_fullscreen_dialog_set_action(&dialog, NULL, 1u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_fullscreen_dialog_set_action(&dialog, action, action_len));

    width_spec.mode = 99u;
    width_spec.size = 10.0f;
    height_spec = width_spec;
    M3_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                 height_spec, &size),
                   M3_ERR_INVALID_ARGUMENT);

    width_spec.mode = M3_MEASURE_EXACTLY;
    width_spec.size = -1.0f;
    height_spec = width_spec;
    M3_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                 height_spec, &size),
                   M3_ERR_RANGE);

    width_spec.mode = M3_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec = width_spec;
    M3_TEST_EXPECT(
        dialog.widget.vtable->measure(NULL, width_spec, height_spec, &size),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                 height_spec, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                             height_spec, &size));

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = 200.0f;
    height_spec = width_spec;
    M3_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                             height_spec, &size));

    width_spec.mode = M3_MEASURE_EXACTLY;
    width_spec.size = 320.0f;
    height_spec.mode = M3_MEASURE_EXACTLY;
    height_spec.size = 200.0f;
    M3_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                             height_spec, &size));

    measure_before = backend.measure_calls;
    M3_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                             height_spec, &size));
    M3_TEST_ASSERT(backend.measure_calls == measure_before);

    M3_TEST_OK(m3_fullscreen_dialog_set_title(&dialog, title, title_len));
    M3_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                             height_spec, &size));
    M3_TEST_ASSERT(backend.measure_calls > measure_before);

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -10.0f;
    bounds.height = 20.0f;
    M3_TEST_EXPECT(dialog.widget.vtable->layout(dialog.widget.ctx, bounds),
                   M3_ERR_RANGE);
    M3_TEST_EXPECT(dialog.widget.vtable->layout(NULL, bounds),
                   M3_ERR_INVALID_ARGUMENT);

    bounds.width = 300.0f;
    bounds.height = 200.0f;
    M3_TEST_OK(m3_fullscreen_dialog_set_action(&dialog, action, action_len));
    M3_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    M3_TEST_ASSERT(dialog.action_bounds.width > 0.0f);

    M3_TEST_OK(m3_fullscreen_dialog_set_action(&dialog, NULL, 0u));
    M3_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    M3_TEST_ASSERT(dialog.action_bounds.width == 0.0f);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    ctx.clip = bounds;
    ctx.dpi_scale = 1.0f;
    M3_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    ctx.gfx = NULL;
    M3_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                   M3_ERR_INVALID_ARGUMENT);
    ctx.gfx = &gfx;
    gfx.vtable = NULL;
    M3_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                   M3_ERR_INVALID_ARGUMENT);
    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    M3_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                   M3_ERR_UNSUPPORTED);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable_no_draw);
    M3_TEST_OK(m3_fullscreen_dialog_set_action(&dialog, action, action_len));
    M3_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                   M3_ERR_UNSUPPORTED);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable, NULL);
    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.utf8_body = NULL;
    dialog.body_len = 0u;
    dialog.utf8_action = NULL;
    dialog.action_len = 0u;
    dialog.metrics_valid = M3_FALSE;
    M3_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));

    dialog.utf8_title = title;
    dialog.title_len = title_len;
    dialog.utf8_body = body;
    dialog.body_len = body_len;
    dialog.utf8_action = action;
    dialog.action_len = action_len;
    dialog.metrics_valid = M3_FALSE;
    M3_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    M3_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));

    memset(&event, 0, sizeof(event));
    event.type = M3_INPUT_POINTER_DOWN;
    event.data.pointer.x = (m3_i32)(dialog.action_bounds.x + 1.0f);
    event.data.pointer.y = (m3_i32)(dialog.action_bounds.y + 1.0f);
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);

    M3_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled),
        M3_ERR_STATE);

    memset(&state, 0, sizeof(state));
    M3_TEST_OK(m3_fullscreen_dialog_set_on_action(
        &dialog, test_fullscreen_action, &state));

    event.type = M3_INPUT_POINTER_UP;
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(state.calls == 1);

    event.type = M3_INPUT_POINTER_DOWN;
    event.data.pointer.x = (m3_i32)(dialog.action_bounds.x + 1.0f);
    event.data.pointer.y = (m3_i32)(dialog.action_bounds.y + 1.0f);
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    state.fail_next = 1;
    event.type = M3_INPUT_POINTER_UP;
    M3_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled),
        M3_ERR_IO);

    dialog.widget.flags = M3_WIDGET_FLAG_DISABLED;
    event.type = M3_INPUT_POINTER_DOWN;
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    dialog.widget.flags = 0u;

    event.type = M3_INPUT_KEY_DOWN;
    M3_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    {
      M3Semantics semantics;
      M3_TEST_EXPECT(dialog.widget.vtable->get_semantics(NULL, &semantics),
                     M3_ERR_INVALID_ARGUMENT);
      M3_TEST_EXPECT(
          dialog.widget.vtable->get_semantics(dialog.widget.ctx, NULL),
          M3_ERR_INVALID_ARGUMENT);
      M3_TEST_OK(
          dialog.widget.vtable->get_semantics(dialog.widget.ctx, &semantics));
      M3_TEST_ASSERT(semantics.utf8_label == dialog.utf8_title);
    }

    M3_TEST_EXPECT(dialog.widget.vtable->event(NULL, &event, &handled),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, NULL, &handled),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(dialog.widget.vtable->event(dialog.widget.ctx, &event, NULL),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(dialog.widget.vtable->destroy(NULL),
                   M3_ERR_INVALID_ARGUMENT);

    backend.fail_destroy = M3_ERR_IO;
    M3_TEST_EXPECT(dialog.widget.vtable->destroy(dialog.widget.ctx), M3_ERR_IO);
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3FullScreenDialogStyle style;
    M3FullScreenDialog dialog;
    const char *family;
    const char *title;
    m3_usize title_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    title = "Title";
    title_len = (m3_usize)strlen(title);

    M3_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    M3_TEST_OK(m3_fullscreen_dialog_init(&dialog, &text_backend, &style, title,
                                         title_len, NULL, 0u));

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 1;
    M3_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 0;
    M3_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 1;
    M3_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 3;
    backend.fail_destroy_at = 0;
    M3_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 3;
    backend.fail_destroy_at = 1;
    M3_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 0;
    backend.fail_destroy_at = 1;
    M3_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_destroy_at = 0;
    M3_TEST_OK(m3_fullscreen_dialog_set_style(&dialog, &style));

    M3_TEST_EXPECT(m3_fullscreen_dialog_set_style(NULL, &style),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, NULL),
                   M3_ERR_INVALID_ARGUMENT);
  }

  return 0;
}

static int test_snackbar(void) {
  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3SnackbarStyle style;
    M3Snackbar snackbar;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    M3_TEST_EXPECT(m3_snackbar_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);

    M3_TEST_EXPECT(m3_snackbar_init(NULL, &text_backend, &style, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_snackbar_init(&snackbar, NULL, &style, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, NULL, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 1u),
                   M3_ERR_INVALID_ARGUMENT);

    text_backend.vtable = NULL;
    M3_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3SnackbarStyle style;
    M3Snackbar snackbar;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    M3_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.corner_radius = -1.0f;
    M3_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.min_width = 10.0f;
    style.max_width = 5.0f;
    M3_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.action_spacing = -1.0f;
    M3_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.padding.left = -1.0f;
    M3_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.message_style.utf8_family = NULL;
    M3_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.message_style.size_px = 0;
    M3_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.action_style.weight = 50;
    M3_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
                   M3_ERR_RANGE);

    M3_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.background_color.r = 1.5f;
    M3_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
                   M3_ERR_RANGE);
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3Gfx gfx;
    M3PaintContext ctx;
    M3SnackbarStyle style;
    M3Snackbar snackbar;
    M3MeasureSpec width_spec;
    M3MeasureSpec height_spec;
    M3Size size;
    M3Rect bounds;
    M3InputEvent event;
    M3Bool handled;
    TestActionState state;
    const char *family;
    const char *message;
    const char *action;
    m3_usize message_len;
    m3_usize action_len;
    int measure_before;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    message = "Snack";
    action = "Undo";
    message_len = (m3_usize)strlen(message);
    action_len = (m3_usize)strlen(action);

    M3_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    M3_TEST_OK(m3_snackbar_init(&snackbar, &text_backend, &style, message,
                                message_len));

    M3_TEST_EXPECT(m3_snackbar_set_message(&snackbar, NULL, 1u),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_snackbar_set_action(&snackbar, NULL, 1u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_snackbar_set_action(&snackbar, action, action_len));

    width_spec.mode = 99u;
    width_spec.size = 10.0f;
    height_spec = width_spec;
    M3_TEST_EXPECT(snackbar.widget.vtable->measure(
                       snackbar.widget.ctx, width_spec, height_spec, &size),
                   M3_ERR_INVALID_ARGUMENT);

    width_spec.mode = M3_MEASURE_EXACTLY;
    width_spec.size = -1.0f;
    height_spec = width_spec;
    M3_TEST_EXPECT(snackbar.widget.vtable->measure(
                       snackbar.widget.ctx, width_spec, height_spec, &size),
                   M3_ERR_RANGE);

    width_spec.mode = M3_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec = width_spec;
    M3_TEST_EXPECT(
        snackbar.widget.vtable->measure(NULL, width_spec, height_spec, &size),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(snackbar.widget.vtable->measure(
                       snackbar.widget.ctx, width_spec, height_spec, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                               height_spec, &size));

    width_spec.mode = M3_MEASURE_AT_MOST;
    width_spec.size = 200.0f;
    height_spec = width_spec;
    M3_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                               height_spec, &size));

    width_spec.mode = M3_MEASURE_EXACTLY;
    width_spec.size = 320.0f;
    height_spec.mode = M3_MEASURE_EXACTLY;
    height_spec.size = 80.0f;
    M3_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                               height_spec, &size));

    measure_before = backend.measure_calls;
    M3_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                               height_spec, &size));
    M3_TEST_ASSERT(backend.measure_calls == measure_before);

    M3_TEST_OK(m3_snackbar_set_message(&snackbar, message, message_len));
    M3_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                               height_spec, &size));
    M3_TEST_ASSERT(backend.measure_calls > measure_before);

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -10.0f;
    bounds.height = 20.0f;
    M3_TEST_EXPECT(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds),
                   M3_ERR_RANGE);
    M3_TEST_EXPECT(snackbar.widget.vtable->layout(NULL, bounds),
                   M3_ERR_INVALID_ARGUMENT);

    bounds.width = 300.0f;
    bounds.height = 80.0f;
    M3_TEST_OK(m3_snackbar_set_action(&snackbar, action, action_len));
    M3_TEST_OK(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds));
    M3_TEST_ASSERT(snackbar.action_bounds.width > 0.0f);

    M3_TEST_OK(m3_snackbar_set_action(&snackbar, NULL, 0u));
    M3_TEST_OK(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds));
    M3_TEST_ASSERT(snackbar.action_bounds.width == 0.0f);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    ctx.clip = bounds;
    ctx.dpi_scale = 1.0f;
    M3_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    ctx.gfx = NULL;
    M3_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx),
                   M3_ERR_INVALID_ARGUMENT);
    ctx.gfx = &gfx;
    gfx.vtable = NULL;
    M3_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx),
                   M3_ERR_INVALID_ARGUMENT);
    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    M3_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx),
                   M3_ERR_UNSUPPORTED);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable_no_draw);
    M3_TEST_OK(m3_snackbar_set_action(&snackbar, action, action_len));
    M3_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx),
                   M3_ERR_UNSUPPORTED);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable, NULL);
    snackbar.utf8_message = NULL;
    snackbar.message_len = 0u;
    snackbar.utf8_action = NULL;
    snackbar.action_len = 0u;
    snackbar.metrics_valid = M3_FALSE;
    M3_TEST_OK(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx));

    snackbar.utf8_message = message;
    snackbar.message_len = message_len;
    snackbar.utf8_action = action;
    snackbar.action_len = action_len;
    snackbar.metrics_valid = M3_FALSE;
    M3_TEST_OK(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds));

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    M3_TEST_OK(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx));

    memset(&event, 0, sizeof(event));
    event.type = M3_INPUT_POINTER_DOWN;
    event.data.pointer.x = (m3_i32)(snackbar.action_bounds.x + 1.0f);
    event.data.pointer.y = (m3_i32)(snackbar.action_bounds.y + 1.0f);
    M3_TEST_OK(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);

    M3_TEST_EXPECT(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled),
        M3_ERR_STATE);

    memset(&state, 0, sizeof(state));
    M3_TEST_OK(
        m3_snackbar_set_on_action(&snackbar, test_snackbar_action, &state));

    event.type = M3_INPUT_POINTER_UP;
    M3_TEST_OK(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(state.calls == 1);

    event.type = M3_INPUT_POINTER_DOWN;
    event.data.pointer.x = (m3_i32)(snackbar.action_bounds.x + 1.0f);
    event.data.pointer.y = (m3_i32)(snackbar.action_bounds.y + 1.0f);
    M3_TEST_OK(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled));
    state.fail_next = 1;
    event.type = M3_INPUT_POINTER_UP;
    M3_TEST_EXPECT(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled),
        M3_ERR_IO);

    snackbar.widget.flags = M3_WIDGET_FLAG_DISABLED;
    event.type = M3_INPUT_POINTER_DOWN;
    M3_TEST_OK(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled));
    snackbar.widget.flags = 0u;

    event.type = M3_INPUT_KEY_DOWN;
    M3_TEST_OK(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    {
      M3Semantics semantics;
      M3_TEST_EXPECT(snackbar.widget.vtable->get_semantics(NULL, &semantics),
                     M3_ERR_INVALID_ARGUMENT);
      M3_TEST_EXPECT(
          snackbar.widget.vtable->get_semantics(snackbar.widget.ctx, NULL),
          M3_ERR_INVALID_ARGUMENT);
      M3_TEST_OK(snackbar.widget.vtable->get_semantics(snackbar.widget.ctx,
                                                       &semantics));
      M3_TEST_ASSERT(semantics.utf8_label == snackbar.utf8_message);
    }

    M3_TEST_EXPECT(snackbar.widget.vtable->event(NULL, &event, &handled),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        snackbar.widget.vtable->event(snackbar.widget.ctx, NULL, &handled),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, NULL),
        M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(snackbar.widget.vtable->destroy(NULL),
                   M3_ERR_INVALID_ARGUMENT);

    backend.fail_destroy = M3_ERR_IO;
    M3_TEST_EXPECT(snackbar.widget.vtable->destroy(snackbar.widget.ctx),
                   M3_ERR_IO);
  }

  {
    TestDialogBackend backend;
    M3TextBackend text_backend;
    M3SnackbarStyle style;
    M3Snackbar snackbar;
    const char *family;
    const char *message;
    m3_usize message_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    message = "Snack";
    message_len = (m3_usize)strlen(message);

    M3_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    M3_TEST_OK(m3_snackbar_init(&snackbar, &text_backend, &style, message,
                                message_len));

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 1;
    M3_TEST_EXPECT(m3_snackbar_set_style(&snackbar, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 0;
    M3_TEST_EXPECT(m3_snackbar_set_style(&snackbar, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 1;
    M3_TEST_EXPECT(m3_snackbar_set_style(&snackbar, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 0;
    backend.fail_destroy_at = 1;
    M3_TEST_EXPECT(m3_snackbar_set_style(&snackbar, &style), M3_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_destroy_at = 0;
    M3_TEST_OK(m3_snackbar_set_style(&snackbar, &style));

    M3_TEST_EXPECT(m3_snackbar_set_style(NULL, &style),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_snackbar_set_style(&snackbar, NULL),
                   M3_ERR_INVALID_ARGUMENT);
  }

  return 0;
}

int main(void) {
  if (test_dialog_helpers() != 0) {
    return 1;
  }
  if (test_alert_dialog() != 0) {
    return 1;
  }
  if (test_fullscreen_dialog() != 0) {
    return 1;
  }
  if (test_snackbar() != 0) {
    return 1;
  }
  return 0;
}
