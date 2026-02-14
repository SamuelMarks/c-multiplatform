#include "m3/m3_dialogs.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestDialogBackend {
  int create_calls;
  int destroy_calls;
  int measure_calls;
  int draw_calls;
  int draw_rect_calls;
  int push_clip_calls;
  int pop_clip_calls;
  int fail_create;
  int fail_create_at;
  int fail_destroy;
  int fail_destroy_at;
  int fail_measure;
  int fail_draw;
  int fail_draw_rect;
  int fail_push_clip;
  int fail_pop_clip;
  CMPHandle last_font;
  CMPRect last_rect;
  CMPColor last_rect_color;
  CMPScalar last_corner;
  CMPScalar last_text_x;
  CMPScalar last_text_y;
  cmp_usize last_text_len;
  CMPColor last_text_color;
} TestDialogBackend;

typedef struct TestActionState {
  int calls;
  cmp_u32 last_action;
  int fail_next;
} TestActionState;

static void test_backend_init(TestDialogBackend *backend) {
  if (backend == NULL) {
    return;
  }
  memset(backend, 0, sizeof(*backend));
  backend->fail_create = CMP_OK;
  backend->fail_destroy = CMP_OK;
  backend->fail_measure = CMP_OK;
  backend->fail_draw = CMP_OK;
  backend->fail_draw_rect = CMP_OK;
  backend->fail_push_clip = CMP_OK;
  backend->fail_pop_clip = CMP_OK;
  backend->fail_create_at = 0;
  backend->fail_destroy_at = 0;
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 cmp_i32 size_px, cmp_i32 weight,
                                 CMPBool italic, CMPHandle *out_font) {
  TestDialogBackend *backend;

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

  backend = (TestDialogBackend *)text;
  backend->create_calls += 1;
  if (backend->fail_create_at > 0 &&
      backend->create_calls == backend->fail_create_at) {
    return CMP_ERR_IO;
  }
  if (backend->fail_create != CMP_OK) {
    return backend->fail_create;
  }

  out_font->id = (cmp_u32)backend->create_calls;
  out_font->generation = (cmp_u32)backend->create_calls;
  backend->last_font = *out_font;
  return CMP_OK;
}

static int test_text_destroy_font(void *text, CMPHandle font) {
  TestDialogBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestDialogBackend *)text;
  backend->destroy_calls += 1;
  if (backend->fail_destroy_at > 0 &&
      backend->destroy_calls == backend->fail_destroy_at) {
    return CMP_ERR_IO;
  }
  if (backend->fail_destroy != CMP_OK) {
    return backend->fail_destroy;
  }
  return CMP_OK;
}

static int test_text_measure_text(void *text, CMPHandle font, const char *utf8,
                                  cmp_usize utf8_len, CMPScalar *out_width,
                                  CMPScalar *out_height,
                                  CMPScalar *out_baseline) {
  TestDialogBackend *backend;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestDialogBackend *)text;
  backend->measure_calls += 1;
  if (backend->fail_measure != CMP_OK) {
    return backend->fail_measure;
  }

  *out_width = (CMPScalar)(utf8_len * 8u);
  *out_height = 20.0f;
  *out_baseline = 14.0f;
  return CMP_OK;
}

static int test_text_draw_text(void *text, CMPHandle font, const char *utf8,
                               cmp_usize utf8_len, CMPScalar x, CMPScalar y,
                               CMPColor color) {
  TestDialogBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestDialogBackend *)text;
  backend->draw_calls += 1;
  if (backend->fail_draw != CMP_OK) {
    return backend->fail_draw;
  }

  backend->last_text_x = x;
  backend->last_text_y = y;
  backend->last_text_len = utf8_len;
  backend->last_text_color = color;
  return CMP_OK;
}

static int test_gfx_draw_rect(void *gfx, const CMPRect *rect, CMPColor color,
                              CMPScalar corner_radius) {
  TestDialogBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestDialogBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect != CMP_OK) {
    return backend->fail_draw_rect;
  }
  return CMP_OK;
}

static int test_gfx_push_clip(void *gfx, const CMPRect *rect) {
  TestDialogBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestDialogBackend *)gfx;
  backend->push_clip_calls += 1;
  if (backend->fail_push_clip != CMP_OK) {
    return backend->fail_push_clip;
  }
  return CMP_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestDialogBackend *backend;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestDialogBackend *)gfx;
  backend->pop_clip_calls += 1;
  if (backend->fail_pop_clip != CMP_OK) {
    return backend->fail_pop_clip;
  }
  return CMP_OK;
}

static const CMPTextVTable g_test_text_vtable = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    test_text_draw_text};

static const CMPTextVTable g_test_text_vtable_no_draw = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    NULL};

static const CMPTextVTable g_test_text_vtable_no_measure = {
    test_text_create_font, test_text_destroy_font, NULL, test_text_draw_text};

static const CMPTextVTable g_test_text_vtable_no_create = {
    NULL, test_text_destroy_font, test_text_measure_text, test_text_draw_text};

static const CMPGfxVTable g_test_gfx_vtable = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static const CMPGfxVTable g_test_gfx_vtable_no_draw = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL};

static const CMPGfxVTable g_test_gfx_vtable_clip = {NULL,
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

static void test_setup_text_backend(TestDialogBackend *backend,
                                    CMPTextBackend *out_backend,
                                    const CMPTextVTable *vtable) {
  out_backend->ctx = backend;
  out_backend->vtable = vtable;
}

static void test_setup_gfx_backend(TestDialogBackend *backend, CMPGfx *out_gfx,
                                   const CMPGfxVTable *vtable,
                                   const CMPTextVTable *text_vtable) {
  out_gfx->ctx = backend;
  out_gfx->vtable = vtable;
  out_gfx->text_vtable = text_vtable;
}

static int test_alert_action(void *ctx, struct M3AlertDialog *dialog,
                             cmp_u32 action_id) {
  TestActionState *state;

  if (ctx == NULL || dialog == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestActionState *)ctx;
  state->calls += 1;
  state->last_action = action_id;
  if (state->fail_next) {
    state->fail_next = 0;
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_fullscreen_action(void *ctx,
                                  struct M3FullScreenDialog *dialog) {
  TestActionState *state;

  if (ctx == NULL || dialog == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestActionState *)ctx;
  state->calls += 1;
  state->last_action = 1u;
  if (state->fail_next) {
    state->fail_next = 0;
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_snackbar_action(void *ctx, struct M3Snackbar *snackbar) {
  TestActionState *state;

  if (ctx == NULL || snackbar == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestActionState *)ctx;
  state->calls += 1;
  state->last_action = 1u;
  if (state->fail_next) {
    state->fail_next = 0;
    return CMP_ERR_IO;
  }
  return CMP_OK;
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
  CMPRect rect;
  CMPColor color;
  CMPTextStyle text_style;
  CMPLayoutEdges edges;
  CMPMeasureSpec spec;
  CMPTextBackend backend;
  CMPBool inside;

  CMP_TEST_EXPECT(m3_dialog_test_point_in_rect(NULL, 0.0f, 0.0f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_dialog_test_point_in_rect(NULL, 0.0f, 0.0f, &inside));
  CMP_TEST_ASSERT(inside == CMP_FALSE);

  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = 0.0f;
  rect.height = 10.0f;
  CMP_TEST_OK(m3_dialog_test_point_in_rect(&rect, 0.0f, 0.0f, &inside));
  CMP_TEST_ASSERT(inside == CMP_FALSE);

  rect.width = 10.0f;
  rect.height = 10.0f;
  CMP_TEST_OK(m3_dialog_test_point_in_rect(&rect, 5.0f, 5.0f, &inside));
  CMP_TEST_ASSERT(inside == CMP_TRUE);
  CMP_TEST_OK(m3_dialog_test_point_in_rect(&rect, -1.0f, 5.0f, &inside));
  CMP_TEST_ASSERT(inside == CMP_FALSE);
  CMP_TEST_OK(m3_dialog_test_point_in_rect(&rect, 5.0f, -1.0f, &inside));
  CMP_TEST_ASSERT(inside == CMP_FALSE);
  CMP_TEST_OK(m3_dialog_test_point_in_rect(&rect, 20.0f, 5.0f, &inside));
  CMP_TEST_ASSERT(inside == CMP_FALSE);
  CMP_TEST_OK(m3_dialog_test_point_in_rect(&rect, 5.0f, 20.0f, &inside));
  CMP_TEST_ASSERT(inside == CMP_FALSE);

  CMP_TEST_EXPECT(m3_dialog_test_validate_color(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 2.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = 2.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = -0.1f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = 2.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = -0.1f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = 2.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = -0.1f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 1.0f;
  CMP_TEST_OK(m3_dialog_test_validate_color(&color));

  CMP_TEST_EXPECT(m3_dialog_test_validate_text_style(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_style_init(&text_style));
  text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_dialog_test_validate_text_style(&text_style, CMP_FALSE));
  text_style.utf8_family = "Test";
  text_style.size_px = 0;
  CMP_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.size_px = 12;
  text_style.weight = 50;
  CMP_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.weight = 901;
  CMP_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.weight = 400;
  text_style.italic = 3;
  CMP_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.italic = CMP_FALSE;
  text_style.color.r = -0.5f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.r = 0.0f;
  text_style.color.g = -0.5f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.g = 0.0f;
  text_style.color.b = 2.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.b = 0.0f;
  text_style.color.a = -0.5f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.a = 1.0f;
  CMP_TEST_OK(m3_dialog_test_validate_text_style(&text_style, CMP_TRUE));

  CMP_TEST_EXPECT(m3_dialog_test_validate_edges(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.right = 0.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.left = 0.0f;
  edges.right = -1.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.right = 0.0f;
  edges.top = -1.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.top = 0.0f;
  edges.bottom = -1.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.bottom = 0.0f;
  CMP_TEST_OK(m3_dialog_test_validate_edges(&edges));

  spec.mode = 99u;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_measure_spec(spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_AT_MOST;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_measure_spec(spec), CMP_ERR_RANGE);
  spec.size = 0.0f;
  CMP_TEST_OK(m3_dialog_test_validate_measure_spec(spec));

  CMP_TEST_EXPECT(m3_dialog_test_validate_rect(NULL), CMP_ERR_INVALID_ARGUMENT);
  rect.width = -1.0f;
  rect.height = 0.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 0.0f;
  rect.height = -1.0f;
  CMP_TEST_EXPECT(m3_dialog_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 0.0f;
  rect.height = 0.0f;
  CMP_TEST_OK(m3_dialog_test_validate_rect(&rect));

  backend.ctx = NULL;
  backend.vtable = NULL;
  CMP_TEST_EXPECT(m3_dialog_test_validate_backend(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_dialog_test_validate_backend(&backend),
                  CMP_ERR_INVALID_ARGUMENT);
  backend.vtable = &g_test_text_vtable;
  CMP_TEST_OK(m3_dialog_test_validate_backend(&backend));

  {
    TestDialogBackend backend_state;
    CMPTextBackend text_backend_local;
    CMPTextMetrics metrics;
    CMPHandle font;
    CMPHandle fonts[2];
    M3AlertDialogStyle alert_style;
    M3AlertDialog alert;
    M3FullScreenDialogStyle fs_style;
    M3FullScreenDialog fs;
    M3SnackbarStyle snack_style;
    M3Snackbar snack;
    CMPScalar total_width;
    CMPScalar height;
    CMPScalar confirm_width;
    CMPScalar dismiss_width;
    CMPScalar action_width;
    CMPScalar action_height;
    const char *family;
    const char *title;
    cmp_usize title_len;

    test_backend_init(&backend_state);
    test_setup_text_backend(&backend_state, &text_backend_local,
                            &g_test_text_vtable);
    font.id = 1u;
    font.generation = 1u;

    CMP_TEST_EXPECT(
        m3_dialog_test_measure_optional_text(NULL, font, NULL, 0u, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        m3_dialog_test_measure_optional_text(NULL, font, NULL, 1u, &metrics),
        CMP_ERR_INVALID_ARGUMENT);
    metrics.width = 1.0f;
    metrics.height = 1.0f;
    metrics.baseline = 1.0f;
    CMP_TEST_OK(
        m3_dialog_test_measure_optional_text(NULL, font, NULL, 0u, &metrics));
    CMP_TEST_ASSERT(metrics.width == 0.0f);
    CMP_TEST_OK(m3_dialog_test_measure_optional_text(&text_backend_local, font,
                                                     "Hi", 2u, &metrics));
    CMP_TEST_ASSERT(metrics.width > 0.0f);

    CMP_TEST_EXPECT(m3_dialog_test_destroy_fonts(NULL, fonts, 1u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_dialog_test_destroy_fonts(&text_backend_local, NULL, 1u),
                    CMP_ERR_INVALID_ARGUMENT);
    fonts[0].id = 0u;
    fonts[0].generation = 0u;
    fonts[1].id = 0u;
    fonts[1].generation = 0u;
    CMP_TEST_OK(m3_dialog_test_destroy_fonts(&text_backend_local, fonts, 2u));

    CMP_TEST_EXPECT(m3_dialog_test_alert_metrics_update(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_dialog_test_fullscreen_metrics_update(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_dialog_test_snackbar_metrics_update(NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    family = "Test";
    title = "Title";
    title_len = (cmp_usize)strlen(title);

    CMP_TEST_OK(m3_alert_dialog_style_init(&alert_style));
    test_set_alert_families(&alert_style, family);
    CMP_TEST_OK(m3_alert_dialog_init(&alert, &text_backend_local, &alert_style,
                                     title, title_len, NULL, 0u));
    CMP_TEST_OK(m3_dialog_test_alert_metrics_update(&alert));
    CMP_TEST_EXPECT(
        m3_dialog_test_alert_compute_actions(NULL, &total_width, &height,
                                             &confirm_width, &dismiss_width),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_dialog_test_alert_compute_actions(
                        &alert, NULL, &height, &confirm_width, &dismiss_width),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(m3_dialog_test_alert_compute_actions(
        &alert, &total_width, &height, &confirm_width, &dismiss_width));
    CMP_TEST_EXPECT(m3_dialog_test_alert_layout_actions(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(alert.widget.vtable->destroy(alert.widget.ctx));

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&fs_style));
    test_set_fullscreen_families(&fs_style, family);
    CMP_TEST_OK(m3_fullscreen_dialog_init(&fs, &text_backend_local, &fs_style,
                                          title, title_len, NULL, 0u));
    CMP_TEST_OK(m3_dialog_test_fullscreen_metrics_update(&fs));
    CMP_TEST_EXPECT(m3_dialog_test_fullscreen_compute_action(
                        NULL, &action_width, &action_height),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        m3_dialog_test_fullscreen_compute_action(&fs, NULL, &action_height),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(m3_dialog_test_fullscreen_compute_action(&fs, &action_width,
                                                         &action_height));
    CMP_TEST_EXPECT(m3_dialog_test_fullscreen_layout_action(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(fs.widget.vtable->destroy(fs.widget.ctx));

    CMP_TEST_OK(m3_snackbar_style_init(&snack_style));
    test_set_snackbar_families(&snack_style, family);
    CMP_TEST_OK(m3_snackbar_init(&snack, &text_backend_local, &snack_style,
                                 title, title_len));
    CMP_TEST_OK(m3_dialog_test_snackbar_metrics_update(&snack));
    CMP_TEST_EXPECT(m3_dialog_test_snackbar_compute_action(NULL, &action_width,
                                                           &action_height),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        m3_dialog_test_snackbar_compute_action(&snack, NULL, &action_height),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(m3_dialog_test_snackbar_compute_action(&snack, &action_width,
                                                       &action_height));
    CMP_TEST_EXPECT(m3_dialog_test_snackbar_layout_action(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(snack.widget.vtable->destroy(snack.widget.ctx));
  }

  return 0;
}

static int test_alert_dialog(void) {
  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);

    family = "Test";

    CMP_TEST_EXPECT(m3_alert_dialog_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_test_set_style_init_fail_after(1u));
    CMP_TEST_EXPECT(m3_alert_dialog_style_init(&style), CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_test_set_style_init_fail_after(2u));
    CMP_TEST_EXPECT(m3_alert_dialog_style_init(&style), CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_test_set_style_init_fail_after(3u));
    CMP_TEST_EXPECT(m3_alert_dialog_style_init(&style), CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_test_set_style_init_fail_after(0u));
    CMP_TEST_OK(
        cmp_visuals_test_set_fail_point(CMP_VISUALS_TEST_FAIL_SHADOW_INIT));
    CMP_TEST_EXPECT(m3_alert_dialog_style_init(&style), CMP_ERR_IO);
    CMP_TEST_OK(cmp_visuals_test_clear_fail_points());
    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);

    CMP_TEST_EXPECT(
        m3_alert_dialog_init(NULL, &text_backend, &style, NULL, 0u, NULL, 0u),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        m3_alert_dialog_init(&dialog, NULL, &style, NULL, 0u, NULL, 0u),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        m3_alert_dialog_init(&dialog, &text_backend, NULL, NULL, 0u, NULL, 0u),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         1u, NULL, 0u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 1u),
                    CMP_ERR_INVALID_ARGUMENT);

    text_backend.vtable = NULL;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_INVALID_ARGUMENT);
    test_setup_text_backend(&backend, &text_backend,
                            &g_test_text_vtable_no_create);
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_UNSUPPORTED);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);

    backend.fail_create_at = 3;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_IO);
    backend.fail_create_at = 0;
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.corner_radius = -1.0f;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.min_width = 100.0f;
    style.max_width = 10.0f;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.title_body_spacing = -1.0f;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.shadow_enabled = 5;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.scrim_enabled = 5;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.padding.left = -1.0f;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.title_style.utf8_family = NULL;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.body_style.size_px = 0;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.action_style.weight = 50;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.title_style.italic = 3;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.title_style.color.r = -0.1f;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.background_color.r = 1.5f;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    style.scrim_color.a = -0.1f;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_RANGE);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    backend.fail_create_at = 3;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_IO);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 1;
    CMP_TEST_EXPECT(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL,
                                         0u, NULL, 0u),
                    CMP_ERR_IO);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    CMPMeasureSpec width_spec;
    CMPMeasureSpec height_spec;
    CMPSize size;
    const char *family;
    const char *title;
    cmp_usize title_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend,
                            &g_test_text_vtable_no_measure);
    family = "Test";
    title = "T";
    title_len = (cmp_usize)strlen(title);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    CMP_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, title,
                                     title_len, NULL, 0u));

    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec = width_spec;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_UNSUPPORTED);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    CMPGfx gfx;
    CMPPaintContext ctx;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    CMPMeasureSpec width_spec;
    CMPMeasureSpec height_spec;
    CMPSize size;
    CMPRect bounds;
    CMPInputEvent event;
    CMPBool handled;
    TestActionState state;
    CMPScalar confirm_height;
    const char *family;
    const char *title;
    const char *body;
    const char *long_body;
    const char *confirm;
    const char *dismiss;
    cmp_usize title_len;
    cmp_usize body_len;
    cmp_usize long_body_len;
    cmp_usize confirm_len;
    cmp_usize dismiss_len;
    int measure_before;
    int calls_before;
    CMPScalar saved_min_width;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);

    family = "Test";
    title = "Title";
    body = "Body";
    long_body = "Longer body text";
    confirm = "OK";
    dismiss = "Cancel";
    title_len = (cmp_usize)strlen(title);
    body_len = (cmp_usize)strlen(body);
    long_body_len = (cmp_usize)strlen(long_body);
    confirm_len = (cmp_usize)strlen(confirm);
    dismiss_len = (cmp_usize)strlen(dismiss);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    CMP_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, title,
                                     title_len, body, body_len));

    CMP_TEST_EXPECT(m3_alert_dialog_set_title(&dialog, NULL, 1u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_alert_dialog_set_title(NULL, title, title_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_alert_dialog_set_body(&dialog, NULL, 1u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_alert_dialog_set_body(NULL, body, body_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(m3_alert_dialog_set_body(&dialog, body, body_len));
    CMP_TEST_EXPECT(m3_alert_dialog_set_actions(&dialog, NULL, 1u, NULL, 0u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_alert_dialog_set_actions(NULL, confirm, confirm_len,
                                                dismiss, dismiss_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        m3_alert_dialog_set_actions(&dialog, confirm, confirm_len, NULL, 1u),
        CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(m3_alert_dialog_set_actions(&dialog, confirm, confirm_len,
                                            dismiss, dismiss_len));

    width_spec.mode = 99u;
    width_spec.size = 10.0f;
    height_spec = width_spec;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_INVALID_ARGUMENT);

    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec.mode = 99u;
    height_spec.size = 0.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_INVALID_ARGUMENT);
    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec.mode = 99u;
    height_spec.size = 0.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_INVALID_ARGUMENT);

    width_spec.mode = CMP_MEASURE_EXACTLY;
    width_spec.size = -1.0f;
    height_spec = width_spec;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_RANGE);

    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec = width_spec;
    CMP_TEST_EXPECT(
        dialog.widget.vtable->measure(NULL, width_spec, height_spec, &size),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));
    saved_min_width = dialog.style.min_width;
    dialog.style.min_width = -1.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_RANGE);
    dialog.style.min_width = saved_min_width;
    CMP_TEST_OK(m3_alert_dialog_set_body(&dialog, long_body, long_body_len));
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));

    width_spec.mode = CMP_MEASURE_AT_MOST;
    width_spec.size = 200.0f;
    height_spec = width_spec;
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));

    width_spec.mode = CMP_MEASURE_EXACTLY;
    width_spec.size = 320.0f;
    height_spec.mode = CMP_MEASURE_EXACTLY;
    height_spec.size = 200.0f;
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));

    measure_before = backend.measure_calls;
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));
    CMP_TEST_ASSERT(backend.measure_calls == measure_before);

    CMP_TEST_OK(m3_alert_dialog_set_title(&dialog, title, title_len));
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));
    CMP_TEST_ASSERT(backend.measure_calls > measure_before);

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -10.0f;
    bounds.height = 20.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->layout(dialog.widget.ctx, bounds),
                    CMP_ERR_RANGE);
    CMP_TEST_EXPECT(dialog.widget.vtable->layout(NULL, bounds),
                    CMP_ERR_INVALID_ARGUMENT);

    bounds.width = 300.0f;
    bounds.height = 200.0f;
    CMP_TEST_OK(
        m3_alert_dialog_set_actions(&dialog, confirm, confirm_len, NULL, 0u));
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    CMP_TEST_ASSERT(dialog.confirm_bounds.width > 0.0f);
    CMP_TEST_ASSERT(dialog.dismiss_bounds.width == 0.0f);

    CMP_TEST_OK(
        m3_alert_dialog_set_actions(&dialog, NULL, 0u, dismiss, dismiss_len));
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    CMP_TEST_ASSERT(dialog.dismiss_bounds.width > 0.0f);
    CMP_TEST_ASSERT(dialog.confirm_bounds.width == 0.0f);

    CMP_TEST_OK(m3_alert_dialog_set_actions(&dialog, confirm, confirm_len,
                                            dismiss, dismiss_len));
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    CMP_TEST_ASSERT(dialog.dismiss_bounds.width > 0.0f);
    CMP_TEST_ASSERT(dialog.confirm_bounds.width > 0.0f);

    CMP_TEST_OK(m3_alert_dialog_set_actions(&dialog, NULL, 0u, NULL, 0u));
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    CMP_TEST_ASSERT(dialog.confirm_bounds.width == 0.0f);
    CMP_TEST_ASSERT(dialog.dismiss_bounds.width == 0.0f);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    ctx.clip = bounds;
    ctx.dpi_scale = 1.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_UNSUPPORTED);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable_no_draw);
    CMP_TEST_OK(m3_alert_dialog_set_actions(&dialog, confirm, confirm_len,
                                            dismiss, dismiss_len));
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_UNSUPPORTED);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable, NULL);
    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.utf8_body = NULL;
    dialog.body_len = 0u;
    dialog.utf8_confirm = NULL;
    dialog.confirm_len = 0u;
    dialog.utf8_dismiss = NULL;
    dialog.dismiss_len = 0u;
    dialog.metrics_valid = CMP_FALSE;
    CMP_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));

    dialog.utf8_title = title;
    dialog.title_len = title_len;
    dialog.utf8_title = title;
    dialog.title_len = title_len;
    dialog.utf8_body = body;
    dialog.body_len = body_len;
    dialog.utf8_confirm = confirm;
    dialog.confirm_len = confirm_len;
    dialog.utf8_dismiss = dismiss;
    dialog.dismiss_len = dismiss_len;
    dialog.metrics_valid = CMP_FALSE;

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.metrics_valid = CMP_FALSE;
    backend.fail_draw = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw = CMP_OK;
    dialog.utf8_title = title;
    dialog.title_len = title_len;
    {
      CMPTextMetrics action_metrics;
      CMPRect action_bounds;

      action_metrics.width = 10.0f;
      action_metrics.height = 12.0f;
      action_metrics.baseline = 9.0f;
      action_bounds = dialog.confirm_bounds;
      CMP_TEST_EXPECT(m3_dialog_test_alert_draw_action_text(
                          NULL, &action_bounds, &action_metrics, confirm,
                          confirm_len, &ctx),
                      CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_EXPECT(
          m3_dialog_test_alert_draw_action_text(&dialog, NULL, &action_metrics,
                                                confirm, confirm_len, &ctx),
          CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_EXPECT(
          m3_dialog_test_alert_draw_action_text(&dialog, &action_bounds, NULL,
                                                confirm, confirm_len, &ctx),
          CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_EXPECT(
          m3_dialog_test_alert_draw_action_text(
              &dialog, &action_bounds, &action_metrics, NULL, 1u, &ctx),
          CMP_ERR_INVALID_ARGUMENT);
    }
    CMP_TEST_ASSERT(ctx.gfx == &gfx);
    CMP_TEST_ASSERT(gfx.vtable == &g_test_gfx_vtable);
    CMP_TEST_ASSERT(gfx.vtable != NULL);
    CMP_TEST_ASSERT(gfx.vtable->draw_rect != NULL);
    CMP_TEST_ASSERT(gfx.text_vtable != NULL);
    CMP_TEST_ASSERT(gfx.text_vtable->draw_text != NULL);
    CMP_TEST_ASSERT(dialog.text_backend.vtable != NULL);
    CMP_TEST_ASSERT(dialog.text_backend.vtable->measure_text != NULL);
    CMP_TEST_ASSERT(dialog.title_len > 0u);
    confirm_height = dialog.confirm_bounds.height;
    dialog.confirm_bounds.height = confirm_height + 20.0f;
    CMP_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));
    dialog.confirm_bounds.height = confirm_height;

    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    ctx.gfx = NULL;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_INVALID_ARGUMENT);
    ctx.gfx = &gfx;
    gfx.vtable = NULL;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_INVALID_ARGUMENT);
    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);

    CMP_TEST_OK(m3_alert_dialog_set_actions(&dialog, confirm, confirm_len,
                                            dismiss, dismiss_len));
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));

    memset(&event, 0, sizeof(event));
    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = (cmp_i32)(dialog.confirm_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(dialog.confirm_bounds.y + 1.0f);
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_TRUE);

    CMP_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled),
        CMP_ERR_STATE);

    memset(&state, 0, sizeof(state));
    CMP_TEST_EXPECT(
        m3_alert_dialog_set_on_action(NULL, test_alert_action, &state),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(
        m3_alert_dialog_set_on_action(&dialog, test_alert_action, &state));

    event.type = CMP_INPUT_POINTER_UP;
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_TRUE);
    CMP_TEST_ASSERT(state.calls == 1);
    CMP_TEST_ASSERT(state.last_action == M3_ALERT_DIALOG_ACTION_CONFIRM);

    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = (cmp_i32)(dialog.dismiss_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(dialog.dismiss_bounds.y + 1.0f);
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));

    state.fail_next = 1;
    event.type = CMP_INPUT_POINTER_UP;
    CMP_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled),
        CMP_ERR_IO);

    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = (cmp_i32)(dialog.dismiss_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(dialog.dismiss_bounds.y + 1.0f);
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    event.type = CMP_INPUT_POINTER_UP;
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_TRUE);

    event.type = CMP_INPUT_POINTER_UP;
    event.data.pointer.x = (cmp_i32)(bounds.x - 5.0f);
    event.data.pointer.y = (cmp_i32)(bounds.y - 5.0f);
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_FALSE);

    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = (cmp_i32)(bounds.x - 5.0f);
    event.data.pointer.y = (cmp_i32)(bounds.y - 5.0f);
    calls_before = state.calls;
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_FALSE);

    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = (cmp_i32)(dialog.confirm_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(dialog.confirm_bounds.y + 1.0f);
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    event.type = CMP_INPUT_POINTER_UP;
    event.data.pointer.x = (cmp_i32)(bounds.x - 5.0f);
    event.data.pointer.y = (cmp_i32)(bounds.y - 5.0f);
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(state.calls == calls_before);

    dialog.widget.flags = CMP_WIDGET_FLAG_DISABLED;
    event.type = CMP_INPUT_POINTER_DOWN;
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    dialog.widget.flags = 0u;

    event.type = CMP_INPUT_KEY_DOWN;
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_FALSE);

    CMP_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, NULL, &handled),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(dialog.widget.vtable->event(NULL, &event, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, NULL),
        CMP_ERR_INVALID_ARGUMENT);

    {
      CMPSemantics semantics;
      CMP_TEST_EXPECT(dialog.widget.vtable->get_semantics(NULL, &semantics),
                      CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_EXPECT(
          dialog.widget.vtable->get_semantics(dialog.widget.ctx, NULL),
          CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_OK(
          dialog.widget.vtable->get_semantics(dialog.widget.ctx, &semantics));
      CMP_TEST_ASSERT(semantics.utf8_label == dialog.utf8_title);
      dialog.widget.flags = CMP_WIDGET_FLAG_DISABLED;
      CMP_TEST_OK(
          dialog.widget.vtable->get_semantics(dialog.widget.ctx, &semantics));
      CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0u);
      dialog.widget.flags = 0u;
    }

    CMP_TEST_EXPECT(dialog.widget.vtable->destroy(NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    backend.fail_destroy = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->destroy(dialog.widget.ctx),
                    CMP_ERR_IO);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    CMPGfx gfx;
    CMPPaintContext ctx;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    CMPRect bounds;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    CMP_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, NULL, 0u,
                                     NULL, 0u));

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = 100.0f;
    bounds.height = 100.0f;
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable, NULL);
    ctx.gfx = &gfx;
    ctx.clip = bounds;
    ctx.dpi_scale = 1.0f;
    CMP_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    const char *family;
    const char *title;
    cmp_usize title_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    title = "Title";
    title_len = (cmp_usize)strlen(title);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    CMP_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, title,
                                     title_len, NULL, 0u));

    dialog.text_backend.vtable = NULL;
    dialog.owns_fonts = CMP_TRUE;
    CMP_TEST_EXPECT(dialog.widget.vtable->destroy(dialog.widget.ctx),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    const char *family;
    const char *title;
    cmp_usize title_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    title = "Title";
    title_len = (cmp_usize)strlen(title);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    CMP_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, title,
                                     title_len, NULL, 0u));

    dialog.body_font.id = 0u;
    dialog.body_font.generation = 0u;
    CMP_TEST_OK(dialog.widget.vtable->destroy(dialog.widget.ctx));
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    const char *family;
    const char *title;
    cmp_usize title_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    title = "Title";
    title_len = (cmp_usize)strlen(title);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    CMP_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, title,
                                     title_len, NULL, 0u));

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 1;
    CMP_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, &style), CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 0;
    CMP_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, &style), CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 1;
    CMP_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, &style), CMP_ERR_IO);

    backend.create_calls = 1;
    backend.destroy_calls = 0;
    backend.fail_create_at = 4;
    backend.fail_destroy_at = 0;
    CMP_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, &style), CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 3;
    backend.fail_destroy_at = 1;
    CMP_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, &style), CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 0;
    backend.fail_destroy_at = 1;
    CMP_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, &style), CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_destroy_at = 0;
    CMP_TEST_OK(m3_alert_dialog_set_style(&dialog, &style));

    CMP_TEST_EXPECT(m3_alert_dialog_set_style(NULL, &style),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_alert_dialog_set_style(&dialog, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    CMPGfx gfx;
    CMPPaintContext ctx;
    M3AlertDialogStyle style;
    M3AlertDialog dialog;
    CMPMeasureSpec width_spec;
    CMPMeasureSpec height_spec;
    CMPSize size;
    CMPRect bounds;
    CMPInputEvent event;
    CMPBool handled;
    CMPSemantics semantics;
    const char *family;
    const char *title;
    const char *body;
    const char *confirm;
    const char *dismiss;
    cmp_usize title_len;
    cmp_usize body_len;
    cmp_usize confirm_len;
    cmp_usize dismiss_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    title = "Title";
    body = "Body";
    confirm = "Confirm";
    dismiss = "Dismiss";
    title_len = (cmp_usize)strlen(title);
    body_len = (cmp_usize)strlen(body);
    confirm_len = (cmp_usize)strlen(confirm);
    dismiss_len = (cmp_usize)strlen(dismiss);

    CMP_TEST_OK(m3_alert_dialog_style_init(&style));
    test_set_alert_families(&style, family);
    CMP_TEST_OK(m3_alert_dialog_init(&dialog, &text_backend, &style, title,
                                     title_len, body, body_len));
    CMP_TEST_OK(m3_alert_dialog_set_actions(&dialog, confirm, confirm_len,
                                            dismiss, dismiss_len));

    width_spec.mode = 99u;
    width_spec.size = 0.0f;
    height_spec.mode = CMP_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_INVALID_ARGUMENT);
    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    height_spec.mode = CMP_MEASURE_UNSPECIFIED;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.utf8_body = body;
    dialog.body_len = body_len;
    dialog.utf8_confirm = NULL;
    dialog.confirm_len = 0u;
    dialog.utf8_dismiss = NULL;
    dialog.dismiss_len = 0u;
    dialog.metrics_valid = CMP_FALSE;
    backend.fail_measure = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_IO);

    dialog.utf8_body = NULL;
    dialog.body_len = 0u;
    dialog.utf8_confirm = confirm;
    dialog.confirm_len = confirm_len;
    dialog.metrics_valid = CMP_FALSE;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_IO);

    dialog.utf8_confirm = NULL;
    dialog.confirm_len = 0u;
    dialog.utf8_dismiss = dismiss;
    dialog.dismiss_len = dismiss_len;
    dialog.metrics_valid = CMP_FALSE;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_IO);
    backend.fail_measure = CMP_OK;

    dialog.utf8_title = title;
    dialog.title_len = title_len;
    dialog.utf8_body = body;
    dialog.body_len = body_len;
    dialog.utf8_confirm = confirm;
    dialog.confirm_len = confirm_len;
    dialog.utf8_dismiss = dismiss;
    dialog.dismiss_len = dismiss_len;
    dialog.metrics_valid = CMP_FALSE;
    CMP_TEST_OK(m3_dialog_test_set_fail_point(
        M3_DIALOG_TEST_FAIL_ALERT_COMPUTE_ACTIONS));
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_IO);
    CMP_TEST_OK(m3_dialog_test_clear_fail_points());

    width_spec.mode = CMP_MEASURE_EXACTLY;
    width_spec.size = 300.0f;
    height_spec.mode = CMP_MEASURE_EXACTLY;
    height_spec.size = 200.0f;
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));

    width_spec.mode = CMP_MEASURE_AT_MOST;
    width_spec.size = 80.0f;
    height_spec.mode = CMP_MEASURE_AT_MOST;
    height_spec.size = 80.0f;
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));

    dialog.style.min_width = 400.0f;
    dialog.style.max_width = 0.0f;
    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec.mode = CMP_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));
    CMP_TEST_ASSERT(size.width >= dialog.style.min_width);

    dialog.style.min_width = 0.0f;
    dialog.style.max_width = 50.0f;
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));
    CMP_TEST_ASSERT(size.width <= dialog.style.max_width);

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -1.0f;
    bounds.height = 10.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->layout(dialog.widget.ctx, bounds),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_alert_dialog_set_actions(&dialog, NULL, 0u, NULL, 0u));
    bounds.width = 20.0f;
    bounds.height = 20.0f;
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    CMP_TEST_ASSERT(dialog.confirm_bounds.width == 0.0f);
    CMP_TEST_ASSERT(dialog.dismiss_bounds.width == 0.0f);

    CMP_TEST_OK(
        m3_alert_dialog_set_actions(&dialog, NULL, 0u, dismiss, dismiss_len));
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    CMP_TEST_ASSERT(dialog.dismiss_bounds.width > 0.0f);
    CMP_TEST_ASSERT(dialog.confirm_bounds.width == 0.0f);

    CMP_TEST_OK(
        m3_alert_dialog_set_actions(&dialog, confirm, confirm_len, NULL, 0u));
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    CMP_TEST_ASSERT(dialog.confirm_bounds.width > 0.0f);
    CMP_TEST_ASSERT(dialog.dismiss_bounds.width == 0.0f);

    CMP_TEST_OK(m3_alert_dialog_set_actions(&dialog, confirm, confirm_len,
                                            dismiss, dismiss_len));
    CMP_TEST_OK(m3_dialog_test_set_fail_point(
        M3_DIALOG_TEST_FAIL_ALERT_COMPUTE_ACTIONS));
    CMP_TEST_EXPECT(dialog.widget.vtable->layout(dialog.widget.ctx, bounds),
                    CMP_ERR_IO);
    CMP_TEST_OK(m3_dialog_test_clear_fail_points());

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    ctx.clip = bounds;
    ctx.dpi_scale = 1.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_UNSUPPORTED);

    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.utf8_body = NULL;
    dialog.body_len = 0u;
    dialog.utf8_confirm = NULL;
    dialog.confirm_len = 0u;
    dialog.utf8_dismiss = NULL;
    dialog.dismiss_len = 0u;
    dialog.metrics_valid = CMP_FALSE;
    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable, NULL);
    ctx.gfx = &gfx;
    CMP_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));

    dialog.utf8_title = title;
    dialog.title_len = title_len;
    dialog.metrics_valid = CMP_FALSE;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_UNSUPPORTED);

    dialog.utf8_body = body;
    dialog.body_len = body_len;
    dialog.utf8_confirm = confirm;
    dialog.confirm_len = confirm_len;
    dialog.utf8_dismiss = dismiss;
    dialog.dismiss_len = dismiss_len;
    dialog.metrics_valid = CMP_FALSE;
    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    backend.fail_draw_rect = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw_rect = CMP_OK;

    dialog.style.scrim_enabled = CMP_FALSE;
    dialog.style.shadow_enabled = CMP_FALSE;
    backend.fail_draw_rect = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw_rect = CMP_OK;
    dialog.style.scrim_enabled = CMP_TRUE;
    dialog.style.shadow_enabled = CMP_TRUE;

    backend.fail_draw = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw = CMP_OK;

    {
      CMPScalar saved_padding;
      saved_padding = dialog.style.padding.left;
      dialog.style.padding.left = -1.0f;
      CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                      CMP_ERR_RANGE);
      dialog.style.padding.left = saved_padding;
    }

    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.metrics_valid = CMP_FALSE;
    backend.fail_draw = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw = CMP_OK;
    dialog.utf8_title = title;
    dialog.title_len = title_len;

    dialog.utf8_body = NULL;
    dialog.body_len = 0u;
    dialog.utf8_confirm = NULL;
    dialog.confirm_len = 0u;
    dialog.utf8_dismiss = dismiss;
    dialog.dismiss_len = dismiss_len;
    dialog.metrics_valid = CMP_FALSE;
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    backend.fail_draw = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw = CMP_OK;
    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.metrics_valid = CMP_FALSE;
    backend.fail_draw = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw = CMP_OK;
    dialog.utf8_title = title;
    dialog.title_len = title_len;
    dialog.utf8_body = body;
    dialog.body_len = body_len;
    dialog.utf8_confirm = confirm;
    dialog.confirm_len = confirm_len;

    dialog.style.shadow_enabled = CMP_TRUE;
    CMP_TEST_OK(
        cmp_visuals_test_set_fail_point(CMP_VISUALS_TEST_FAIL_SHADOW_NORM));
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_visuals_test_clear_fail_points());

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_clip,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    dialog.style.scrim_enabled = CMP_TRUE;
    dialog.style.shadow_enabled = CMP_TRUE;
    CMP_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));
    dialog.style.scrim_enabled = CMP_FALSE;
    dialog.style.shadow_enabled = CMP_FALSE;

    bounds.width = 1.0f;
    bounds.height = 1.0f;
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    ctx.clip = bounds;
    CMP_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));

    dialog.pressed_confirm = CMP_TRUE;
    memset(&event, 0, sizeof(event));
    event.type = CMP_INPUT_POINTER_DOWN;
    CMP_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled),
        CMP_ERR_STATE);
    dialog.pressed_confirm = CMP_FALSE;
    dialog.pressed_dismiss = CMP_TRUE;
    CMP_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled),
        CMP_ERR_STATE);
    dialog.pressed_dismiss = CMP_FALSE;

    dialog.widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
    CMP_TEST_OK(
        dialog.widget.vtable->get_semantics(dialog.widget.ctx, &semantics));
    CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE) != 0u);
    dialog.widget.flags = 0u;

    CMP_TEST_OK(dialog.widget.vtable->destroy(dialog.widget.ctx));
  }

  return 0;
}

static int test_fullscreen_dialog(void) {
  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3FullScreenDialogStyle style;
    M3FullScreenDialog dialog;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    CMP_TEST_EXPECT(m3_fullscreen_dialog_style_init(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_test_set_style_init_fail_after(1u));
    CMP_TEST_EXPECT(m3_fullscreen_dialog_style_init(&style), CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_test_set_style_init_fail_after(2u));
    CMP_TEST_EXPECT(m3_fullscreen_dialog_style_init(&style), CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_test_set_style_init_fail_after(0u));
    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);

    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(NULL, &text_backend, &style, NULL,
                                              0u, NULL, 0u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        m3_fullscreen_dialog_init(&dialog, NULL, &style, NULL, 0u, NULL, 0u),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, NULL,
                                              NULL, 0u, NULL, 0u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 1u, NULL, 0u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 0u, NULL, 1u),
                    CMP_ERR_INVALID_ARGUMENT);

    text_backend.vtable = NULL;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 0u, NULL, 0u),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3FullScreenDialogStyle style;
    M3FullScreenDialog dialog;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.corner_radius = -1.0f;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.title_body_spacing = -1.0f;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.shadow_enabled = 5;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.scrim_enabled = 5;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.padding.left = -1.0f;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.title_style.utf8_family = NULL;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 0u, NULL, 0u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.body_style.size_px = 0;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.action_style.weight = 50;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.background_color.r = 1.5f;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 0u, NULL, 0u),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    style.scrim_color.a = -0.1f;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              NULL, 0u, NULL, 0u),
                    CMP_ERR_RANGE);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    CMPGfx gfx;
    CMPPaintContext ctx;
    M3FullScreenDialogStyle style;
    M3FullScreenDialog dialog;
    CMPMeasureSpec width_spec;
    CMPMeasureSpec height_spec;
    CMPSize size;
    CMPRect bounds;
    CMPInputEvent event;
    CMPBool handled;
    TestActionState state;
    const char *family;
    const char *title;
    const char *body;
    const char *action;
    cmp_usize title_len;
    cmp_usize body_len;
    cmp_usize action_len;
    int measure_before;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    title = "Title";
    body = "Body";
    action = "Done";
    title_len = (cmp_usize)strlen(title);
    body_len = (cmp_usize)strlen(body);
    action_len = (cmp_usize)strlen(action);

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    backend.fail_create_at = 1;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              title, title_len, body, body_len),
                    CMP_ERR_IO);
    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              title, title_len, body, body_len),
                    CMP_ERR_IO);
    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_destroy_at = 1;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              title, title_len, body, body_len),
                    CMP_ERR_IO);
    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_destroy_at = 0;
    backend.fail_create_at = 3;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              title, title_len, body, body_len),
                    CMP_ERR_IO);
    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_destroy_at = 1;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_init(&dialog, &text_backend, &style,
                                              title, title_len, body, body_len),
                    CMP_ERR_IO);
    backend.fail_destroy_at = 0;
    backend.fail_create_at = 0;
    CMP_TEST_OK(m3_fullscreen_dialog_init(&dialog, &text_backend, &style, title,
                                          title_len, body, body_len));

    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_title(NULL, title, title_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_title(&dialog, NULL, 1u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_body(&dialog, NULL, 1u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(m3_fullscreen_dialog_set_body(&dialog, body, body_len));
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_action(NULL, action, action_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_action(&dialog, NULL, 1u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(m3_fullscreen_dialog_set_action(&dialog, action, action_len));

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = 300.0f;
    bounds.height = 200.0f;
    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.utf8_body = body;
    dialog.body_len = body_len;
    dialog.metrics_valid = CMP_FALSE;
    backend.fail_measure = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->layout(dialog.widget.ctx, bounds),
                    CMP_ERR_IO);
    backend.fail_measure = CMP_OK;
    dialog.utf8_title = title;
    dialog.title_len = title_len;
    dialog.metrics_valid = CMP_FALSE;

    width_spec.mode = 99u;
    width_spec.size = 10.0f;
    height_spec = width_spec;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_INVALID_ARGUMENT);

    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec.mode = 99u;
    height_spec.size = 0.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_INVALID_ARGUMENT);

    width_spec.mode = CMP_MEASURE_EXACTLY;
    width_spec.size = -1.0f;
    height_spec = width_spec;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_RANGE);

    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec = width_spec;
    CMP_TEST_EXPECT(
        dialog.widget.vtable->measure(NULL, width_spec, height_spec, &size),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));

    width_spec.mode = CMP_MEASURE_AT_MOST;
    width_spec.size = 200.0f;
    height_spec = width_spec;
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));

    width_spec.mode = CMP_MEASURE_EXACTLY;
    width_spec.size = 320.0f;
    height_spec.mode = CMP_MEASURE_EXACTLY;
    height_spec.size = 200.0f;
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));

    measure_before = backend.measure_calls;
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));
    CMP_TEST_ASSERT(backend.measure_calls == measure_before);

    CMP_TEST_OK(m3_fullscreen_dialog_set_title(&dialog, title, title_len));
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));
    CMP_TEST_ASSERT(backend.measure_calls > measure_before);

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -10.0f;
    bounds.height = 20.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->layout(dialog.widget.ctx, bounds),
                    CMP_ERR_RANGE);
    CMP_TEST_EXPECT(dialog.widget.vtable->layout(NULL, bounds),
                    CMP_ERR_INVALID_ARGUMENT);

    bounds.width = 300.0f;
    bounds.height = 200.0f;
    {
      CMPScalar saved_padding;
      saved_padding = dialog.style.padding.left;
      dialog.style.padding.left = -1.0f;
      CMP_TEST_EXPECT(dialog.widget.vtable->layout(dialog.widget.ctx, bounds),
                      CMP_ERR_RANGE);
      dialog.style.padding.left = saved_padding;
    }

    CMP_TEST_OK(m3_fullscreen_dialog_set_action(&dialog, action, action_len));
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    CMP_TEST_ASSERT(dialog.action_bounds.width > 0.0f);

    CMP_TEST_OK(m3_fullscreen_dialog_set_action(&dialog, NULL, 0u));
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    CMP_TEST_ASSERT(dialog.action_bounds.width == 0.0f);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    ctx.clip = bounds;
    ctx.dpi_scale = 1.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    ctx.gfx = NULL;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_INVALID_ARGUMENT);
    ctx.gfx = &gfx;
    gfx.vtable = NULL;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_INVALID_ARGUMENT);
    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_UNSUPPORTED);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable_no_draw);
    CMP_TEST_OK(m3_fullscreen_dialog_set_action(&dialog, action, action_len));
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_UNSUPPORTED);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable, NULL);
    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.utf8_body = NULL;
    dialog.body_len = 0u;
    dialog.utf8_action = NULL;
    dialog.action_len = 0u;
    dialog.metrics_valid = CMP_FALSE;
    CMP_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));

    dialog.utf8_title = title;
    dialog.title_len = title_len;
    dialog.utf8_body = body;
    dialog.body_len = body_len;
    dialog.utf8_action = action;
    dialog.action_len = action_len;
    dialog.metrics_valid = CMP_FALSE;
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    {
      CMPTextMetrics action_metrics;
      CMPRect action_bounds;

      action_metrics.width = 10.0f;
      action_metrics.height = 12.0f;
      action_metrics.baseline = 9.0f;
      action_bounds = dialog.action_bounds;
      CMP_TEST_EXPECT(
          m3_dialog_test_fullscreen_draw_action_text(
              NULL, &action_bounds, &action_metrics, action, action_len, &ctx),
          CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_EXPECT(
          m3_dialog_test_fullscreen_draw_action_text(
              &dialog, NULL, &action_metrics, action, action_len, &ctx),
          CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_EXPECT(
          m3_dialog_test_fullscreen_draw_action_text(
              &dialog, &action_bounds, NULL, action, action_len, &ctx),
          CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_EXPECT(
          m3_dialog_test_fullscreen_draw_action_text(
              &dialog, &action_bounds, &action_metrics, NULL, 1u, &ctx),
          CMP_ERR_INVALID_ARGUMENT);
    }
    dialog.action_bounds.height =
        dialog.action_bounds.height + dialog.style.action_padding_y * 4.0f;
    dialog.style.scrim_enabled = CMP_FALSE;
    dialog.style.shadow_enabled = CMP_FALSE;
    CMP_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));
    dialog.style.scrim_enabled = CMP_FALSE;
    dialog.style.shadow_enabled = CMP_TRUE;
    backend.fail_draw_rect = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw_rect = CMP_OK;
    dialog.style.scrim_enabled = CMP_TRUE;
    dialog.style.shadow_enabled = CMP_TRUE;
    CMP_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));
    dialog.style.scrim_enabled = CMP_FALSE;
    dialog.style.shadow_enabled = CMP_FALSE;

    memset(&event, 0, sizeof(event));
    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = (cmp_i32)(dialog.action_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(dialog.action_bounds.y + 1.0f);
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_TRUE);

    CMP_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled),
        CMP_ERR_STATE);

    memset(&state, 0, sizeof(state));
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_on_action(
                        NULL, test_fullscreen_action, &state),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(m3_fullscreen_dialog_set_on_action(
        &dialog, test_fullscreen_action, &state));

    event.type = CMP_INPUT_POINTER_UP;
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_TRUE);
    CMP_TEST_ASSERT(state.calls == 1);

    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = (cmp_i32)(dialog.action_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(dialog.action_bounds.y + 1.0f);
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    state.fail_next = 1;
    event.type = CMP_INPUT_POINTER_UP;
    CMP_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled),
        CMP_ERR_IO);

    dialog.widget.flags = CMP_WIDGET_FLAG_DISABLED;
    event.type = CMP_INPUT_POINTER_DOWN;
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    dialog.widget.flags = 0u;

    event.type = CMP_INPUT_KEY_DOWN;
    CMP_TEST_OK(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_FALSE);

    {
      CMPSemantics semantics;
      CMP_TEST_EXPECT(dialog.widget.vtable->get_semantics(NULL, &semantics),
                      CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_EXPECT(
          dialog.widget.vtable->get_semantics(dialog.widget.ctx, NULL),
          CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_OK(
          dialog.widget.vtable->get_semantics(dialog.widget.ctx, &semantics));
      CMP_TEST_ASSERT(semantics.utf8_label == dialog.utf8_title);
      dialog.widget.flags = CMP_WIDGET_FLAG_DISABLED;
      CMP_TEST_OK(
          dialog.widget.vtable->get_semantics(dialog.widget.ctx, &semantics));
      CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0u);
      dialog.widget.flags = 0u;
    }

    CMP_TEST_EXPECT(dialog.widget.vtable->event(NULL, &event, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, NULL, &handled),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, NULL),
        CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(dialog.widget.vtable->destroy(NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    backend.fail_destroy = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->destroy(dialog.widget.ctx),
                    CMP_ERR_IO);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3FullScreenDialogStyle style;
    M3FullScreenDialog dialog;
    const char *family;
    const char *title;
    cmp_usize title_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    title = "Title";
    title_len = (cmp_usize)strlen(title);

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    CMP_TEST_OK(m3_fullscreen_dialog_init(&dialog, &text_backend, &style, title,
                                          title_len, NULL, 0u));

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 1;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style),
                    CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 0;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style),
                    CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 1;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style),
                    CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 3;
    backend.fail_destroy_at = 0;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style),
                    CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 3;
    backend.fail_destroy_at = 1;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style),
                    CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 0;
    backend.fail_destroy_at = 1;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style),
                    CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_destroy_at = 0;
    backend.fail_destroy = CMP_ERR_IO;
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, &style),
                    CMP_ERR_IO);
    backend.fail_destroy = CMP_OK;
    CMP_TEST_OK(m3_fullscreen_dialog_set_style(&dialog, &style));

    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_style(NULL, &style),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_fullscreen_dialog_set_style(&dialog, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    CMPGfx gfx;
    CMPPaintContext ctx;
    M3FullScreenDialogStyle style;
    M3FullScreenDialog dialog;
    CMPMeasureSpec width_spec;
    CMPMeasureSpec height_spec;
    CMPSize size;
    CMPRect bounds;
    CMPInputEvent event;
    CMPBool handled;
    CMPSemantics semantics;
    TestActionState state;
    const char *family;
    const char *title;
    const char *body;
    const char *action;
    cmp_usize title_len;
    cmp_usize body_len;
    cmp_usize action_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    title = "Title";
    body = "Body";
    action = "Done";
    title_len = (cmp_usize)strlen(title);
    body_len = (cmp_usize)strlen(body);
    action_len = (cmp_usize)strlen(action);

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&style));
    test_set_fullscreen_families(&style, family);
    CMP_TEST_OK(m3_fullscreen_dialog_init(&dialog, &text_backend, &style, title,
                                          title_len, body, body_len));
    CMP_TEST_OK(m3_fullscreen_dialog_set_action(&dialog, action, action_len));

    width_spec.mode = 99u;
    width_spec.size = 0.0f;
    height_spec.mode = CMP_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_INVALID_ARGUMENT);
    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.utf8_body = body;
    dialog.body_len = body_len;
    dialog.utf8_action = NULL;
    dialog.action_len = 0u;
    dialog.metrics_valid = CMP_FALSE;
    backend.fail_measure = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_IO);

    dialog.utf8_body = NULL;
    dialog.body_len = 0u;
    dialog.utf8_action = action;
    dialog.action_len = action_len;
    dialog.metrics_valid = CMP_FALSE;
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_IO);
    backend.fail_measure = CMP_OK;

    dialog.utf8_title = title;
    dialog.title_len = title_len;
    dialog.utf8_body = body;
    dialog.body_len = body_len;
    dialog.metrics_valid = CMP_FALSE;
    CMP_TEST_OK(m3_dialog_test_set_fail_point(
        M3_DIALOG_TEST_FAIL_FULLSCREEN_COMPUTE_ACTION));
    CMP_TEST_EXPECT(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                                  height_spec, &size),
                    CMP_ERR_IO);
    CMP_TEST_OK(m3_dialog_test_clear_fail_points());

    width_spec.mode = CMP_MEASURE_EXACTLY;
    width_spec.size = 300.0f;
    height_spec.mode = CMP_MEASURE_EXACTLY;
    height_spec.size = 200.0f;
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));

    width_spec.mode = CMP_MEASURE_AT_MOST;
    width_spec.size = 100.0f;
    height_spec.mode = CMP_MEASURE_AT_MOST;
    height_spec.size = 100.0f;
    CMP_TEST_OK(dialog.widget.vtable->measure(dialog.widget.ctx, width_spec,
                                              height_spec, &size));

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -1.0f;
    bounds.height = 10.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->layout(dialog.widget.ctx, bounds),
                    CMP_ERR_RANGE);

    dialog.utf8_action = NULL;
    dialog.action_len = 0u;
    bounds.width = 20.0f;
    bounds.height = 20.0f;
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    CMP_TEST_ASSERT(dialog.action_bounds.width == 0.0f);

    dialog.utf8_action = action;
    dialog.action_len = action_len;
    CMP_TEST_OK(m3_dialog_test_set_fail_point(
        M3_DIALOG_TEST_FAIL_FULLSCREEN_COMPUTE_ACTION));
    CMP_TEST_EXPECT(dialog.widget.vtable->layout(dialog.widget.ctx, bounds),
                    CMP_ERR_IO);
    CMP_TEST_OK(m3_dialog_test_clear_fail_points());

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    ctx.clip = bounds;
    ctx.dpi_scale = 1.0f;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_UNSUPPORTED);

    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.utf8_body = NULL;
    dialog.body_len = 0u;
    dialog.utf8_action = NULL;
    dialog.action_len = 0u;
    dialog.metrics_valid = CMP_FALSE;
    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable, NULL);
    ctx.gfx = &gfx;
    CMP_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));

    dialog.utf8_title = title;
    dialog.title_len = title_len;
    dialog.metrics_valid = CMP_FALSE;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_UNSUPPORTED);

    dialog.utf8_body = body;
    dialog.body_len = body_len;
    dialog.utf8_action = action;
    dialog.action_len = action_len;
    dialog.metrics_valid = CMP_FALSE;
    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    {
      CMPScalar saved_corner;
      saved_corner = dialog.style.corner_radius;
      dialog.style.corner_radius = -1.0f;
      CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                      CMP_ERR_RANGE);
      dialog.style.corner_radius = saved_corner;
    }
    backend.fail_measure = CMP_ERR_IO;
    dialog.metrics_valid = CMP_FALSE;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_measure = CMP_OK;
    backend.fail_draw_rect = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw_rect = CMP_OK;

    backend.fail_draw = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw = CMP_OK;
    dialog.utf8_title = NULL;
    dialog.title_len = 0u;
    dialog.utf8_action = NULL;
    dialog.action_len = 0u;
    dialog.metrics_valid = CMP_FALSE;
    backend.fail_draw = CMP_ERR_IO;
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw = CMP_OK;
    dialog.utf8_title = title;
    dialog.title_len = title_len;
    dialog.utf8_action = action;
    dialog.action_len = action_len;

    dialog.style.shadow_enabled = CMP_TRUE;
    CMP_TEST_OK(
        cmp_visuals_test_set_fail_point(CMP_VISUALS_TEST_FAIL_SHADOW_NORM));
    CMP_TEST_EXPECT(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx),
                    CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_visuals_test_clear_fail_points());

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_clip,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    dialog.style.scrim_enabled = CMP_TRUE;
    dialog.style.shadow_enabled = CMP_TRUE;
    CMP_TEST_OK(dialog.widget.vtable->paint(dialog.widget.ctx, &ctx));
    dialog.style.scrim_enabled = CMP_FALSE;
    dialog.style.shadow_enabled = CMP_FALSE;

    dialog.pressed_action = CMP_TRUE;
    memset(&event, 0, sizeof(event));
    event.type = CMP_INPUT_POINTER_DOWN;
    CMP_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled),
        CMP_ERR_STATE);
    dialog.pressed_action = CMP_FALSE;

    CMP_TEST_OK(m3_fullscreen_dialog_set_on_action(
        &dialog, test_fullscreen_action, &state));
    state.fail_next = 1;
    CMP_TEST_OK(dialog.widget.vtable->layout(dialog.widget.ctx, bounds));
    event.type = CMP_INPUT_POINTER_UP;
    event.data.pointer.x = (cmp_i32)(dialog.action_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(dialog.action_bounds.y + 1.0f);
    dialog.pressed_action = CMP_TRUE;
    CMP_TEST_EXPECT(
        dialog.widget.vtable->event(dialog.widget.ctx, &event, &handled),
        CMP_ERR_IO);

    dialog.widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
    CMP_TEST_OK(
        dialog.widget.vtable->get_semantics(dialog.widget.ctx, &semantics));
    CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE) != 0u);
    dialog.widget.flags = 0u;

    CMP_TEST_OK(dialog.widget.vtable->destroy(dialog.widget.ctx));
  }

  return 0;
}

static int test_snackbar(void) {
  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3SnackbarStyle style;
    M3Snackbar snackbar;
    const char *family;
    const char *message;
    cmp_usize message_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    message = "Msg";
    message_len = (cmp_usize)strlen(message);

    CMP_TEST_EXPECT(m3_snackbar_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_text_test_set_style_init_fail_after(1u));
    CMP_TEST_EXPECT(m3_snackbar_style_init(&style), CMP_ERR_IO);
    CMP_TEST_OK(cmp_text_test_set_style_init_fail_after(0u));
    CMP_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);

    CMP_TEST_EXPECT(m3_snackbar_init(NULL, &text_backend, &style, NULL, 0u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_snackbar_init(&snackbar, NULL, &style, NULL, 0u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, NULL, NULL, 0u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 1u),
        CMP_ERR_INVALID_ARGUMENT);

    backend.fail_create_at = 2;
    CMP_TEST_EXPECT(m3_snackbar_init(&snackbar, &text_backend, &style, message,
                                     message_len),
                    CMP_ERR_IO);
    backend.fail_create_at = 0;

    text_backend.vtable = NULL;
    CMP_TEST_EXPECT(
        m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
        CMP_ERR_INVALID_ARGUMENT);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3SnackbarStyle style;
    M3Snackbar snackbar;
    const char *family;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";

    CMP_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.corner_radius = -1.0f;
    CMP_TEST_EXPECT(
        m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
        CMP_ERR_RANGE);

    CMP_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.min_width = 10.0f;
    style.max_width = 5.0f;
    CMP_TEST_EXPECT(
        m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
        CMP_ERR_RANGE);

    CMP_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.action_spacing = -1.0f;
    CMP_TEST_EXPECT(
        m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
        CMP_ERR_RANGE);

    CMP_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.padding.left = -1.0f;
    CMP_TEST_EXPECT(
        m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
        CMP_ERR_RANGE);

    CMP_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.message_style.utf8_family = NULL;
    CMP_TEST_EXPECT(
        m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
        CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.message_style.size_px = 0;
    CMP_TEST_EXPECT(
        m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
        CMP_ERR_RANGE);

    CMP_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.action_style.weight = 50;
    CMP_TEST_EXPECT(
        m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
        CMP_ERR_RANGE);

    CMP_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    style.background_color.r = 1.5f;
    CMP_TEST_EXPECT(
        m3_snackbar_init(&snackbar, &text_backend, &style, NULL, 0u),
        CMP_ERR_RANGE);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    CMPGfx gfx;
    CMPPaintContext ctx;
    M3SnackbarStyle style;
    M3Snackbar snackbar;
    CMPMeasureSpec width_spec;
    CMPMeasureSpec height_spec;
    CMPSize size;
    CMPRect bounds;
    CMPScalar max_width_backup;
    CMPScalar min_width_backup;
    CMPInputEvent event;
    CMPBool handled;
    TestActionState state;
    const char *family;
    const char *message;
    const char *action;
    cmp_usize message_len;
    cmp_usize action_len;
    int measure_before;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    message = "Snack";
    action = "Undo";
    message_len = (cmp_usize)strlen(message);
    action_len = (cmp_usize)strlen(action);

    CMP_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    CMP_TEST_OK(m3_snackbar_init(&snackbar, &text_backend, &style, message,
                                 message_len));

    CMP_TEST_EXPECT(m3_snackbar_set_message(&snackbar, NULL, 1u),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_snackbar_set_action(&snackbar, NULL, 1u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(m3_snackbar_set_action(&snackbar, action, action_len));

    width_spec.mode = 99u;
    width_spec.size = 10.0f;
    height_spec = width_spec;
    CMP_TEST_EXPECT(snackbar.widget.vtable->measure(
                        snackbar.widget.ctx, width_spec, height_spec, &size),
                    CMP_ERR_INVALID_ARGUMENT);

    width_spec.mode = CMP_MEASURE_EXACTLY;
    width_spec.size = -1.0f;
    height_spec = width_spec;
    CMP_TEST_EXPECT(snackbar.widget.vtable->measure(
                        snackbar.widget.ctx, width_spec, height_spec, &size),
                    CMP_ERR_RANGE);
    width_spec.mode = CMP_MEASURE_EXACTLY;
    width_spec.size = 10.0f;
    height_spec.mode = 999u;
    height_spec.size = 0.0f;
    CMP_TEST_EXPECT(snackbar.widget.vtable->measure(
                        snackbar.widget.ctx, width_spec, height_spec, &size),
                    CMP_ERR_INVALID_ARGUMENT);
    height_spec.mode = CMP_MEASURE_EXACTLY;
    height_spec.size = 40.0f;

    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec = width_spec;
    CMP_TEST_EXPECT(
        snackbar.widget.vtable->measure(NULL, width_spec, height_spec, &size),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(snackbar.widget.vtable->measure(
                        snackbar.widget.ctx, width_spec, height_spec, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                                height_spec, &size));

    max_width_backup = snackbar.style.max_width;
    min_width_backup = snackbar.style.min_width;
    snackbar.style.min_width = 0.0f;
    snackbar.style.max_width = 20.0f;
    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec = width_spec;
    CMP_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                                height_spec, &size));
    CMP_TEST_ASSERT(size.width == 20.0f);
    snackbar.style.max_width = max_width_backup;
    snackbar.style.min_width = min_width_backup;

    width_spec.mode = CMP_MEASURE_AT_MOST;
    width_spec.size = 200.0f;
    height_spec = width_spec;
    CMP_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                                height_spec, &size));

    width_spec.mode = CMP_MEASURE_EXACTLY;
    width_spec.size = 320.0f;
    height_spec.mode = CMP_MEASURE_EXACTLY;
    height_spec.size = 80.0f;
    CMP_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                                height_spec, &size));

    measure_before = backend.measure_calls;
    CMP_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                                height_spec, &size));
    CMP_TEST_ASSERT(backend.measure_calls == measure_before);

    CMP_TEST_OK(m3_snackbar_set_message(&snackbar, message, message_len));
    CMP_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                                height_spec, &size));
    CMP_TEST_ASSERT(backend.measure_calls > measure_before);

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -10.0f;
    bounds.height = 20.0f;
    CMP_TEST_EXPECT(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds),
                    CMP_ERR_RANGE);
    CMP_TEST_EXPECT(snackbar.widget.vtable->layout(NULL, bounds),
                    CMP_ERR_INVALID_ARGUMENT);

    bounds.width = 300.0f;
    bounds.height = 80.0f;
    {
      CMPScalar saved_padding;
      saved_padding = snackbar.style.padding.left;
      snackbar.style.padding.left = -1.0f;
      CMP_TEST_EXPECT(
          snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds),
          CMP_ERR_RANGE);
      snackbar.style.padding.left = saved_padding;
    }

    snackbar.metrics_valid = CMP_FALSE;
    backend.fail_measure = CMP_ERR_IO;
    CMP_TEST_EXPECT(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds),
                    CMP_ERR_IO);
    backend.fail_measure = CMP_OK;
    CMP_TEST_OK(m3_snackbar_set_action(&snackbar, action, action_len));
    CMP_TEST_OK(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds));
    CMP_TEST_ASSERT(snackbar.action_bounds.width > 0.0f);

    CMP_TEST_OK(m3_snackbar_set_action(&snackbar, NULL, 0u));
    CMP_TEST_OK(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds));
    CMP_TEST_ASSERT(snackbar.action_bounds.width == 0.0f);

    CMP_TEST_OK(m3_snackbar_set_action(&snackbar, action, action_len));
    CMP_TEST_OK(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds));
    snackbar.action_bounds.height = snackbar.style.action_padding_y;

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    ctx.clip = bounds;
    ctx.dpi_scale = 1.0f;
    CMP_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    ctx.gfx = NULL;
    CMP_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx),
                    CMP_ERR_INVALID_ARGUMENT);
    ctx.gfx = &gfx;
    gfx.vtable = NULL;
    CMP_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx),
                    CMP_ERR_INVALID_ARGUMENT);
    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    CMP_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx),
                    CMP_ERR_UNSUPPORTED);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable_no_draw);
    CMP_TEST_OK(m3_snackbar_set_action(&snackbar, action, action_len));
    CMP_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx),
                    CMP_ERR_UNSUPPORTED);

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable, NULL);
    snackbar.utf8_message = NULL;
    snackbar.message_len = 0u;
    snackbar.utf8_action = NULL;
    snackbar.action_len = 0u;
    snackbar.metrics_valid = CMP_FALSE;
    CMP_TEST_OK(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx));

    snackbar.utf8_message = message;
    snackbar.message_len = message_len;
    snackbar.utf8_action = action;
    snackbar.action_len = action_len;
    snackbar.metrics_valid = CMP_FALSE;
    CMP_TEST_OK(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds));
    snackbar.action_bounds.height = snackbar.style.action_padding_y;
    snackbar.action_bounds.width =
        snackbar.action_bounds.width + snackbar.style.action_padding_x * 4.0f;

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    CMP_TEST_OK(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx));

    memset(&event, 0, sizeof(event));
    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = (cmp_i32)(snackbar.action_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(snackbar.action_bounds.y + 1.0f);
    CMP_TEST_OK(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_TRUE);

    CMP_TEST_EXPECT(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled),
        CMP_ERR_STATE);

    memset(&state, 0, sizeof(state));
    CMP_TEST_OK(
        m3_snackbar_set_on_action(&snackbar, test_snackbar_action, &state));

    event.type = CMP_INPUT_POINTER_UP;
    CMP_TEST_OK(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_TRUE);
    CMP_TEST_ASSERT(state.calls == 1);

    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = (cmp_i32)(snackbar.action_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(snackbar.action_bounds.y + 1.0f);
    CMP_TEST_OK(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled));
    state.fail_next = 1;
    event.type = CMP_INPUT_POINTER_UP;
    CMP_TEST_EXPECT(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled),
        CMP_ERR_IO);

    snackbar.widget.flags = CMP_WIDGET_FLAG_DISABLED;
    event.type = CMP_INPUT_POINTER_DOWN;
    CMP_TEST_OK(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled));
    snackbar.widget.flags = 0u;

    event.type = CMP_INPUT_KEY_DOWN;
    CMP_TEST_OK(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled));
    CMP_TEST_ASSERT(handled == CMP_FALSE);

    {
      CMPSemantics semantics;
      CMP_TEST_EXPECT(snackbar.widget.vtable->get_semantics(NULL, &semantics),
                      CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_EXPECT(
          snackbar.widget.vtable->get_semantics(snackbar.widget.ctx, NULL),
          CMP_ERR_INVALID_ARGUMENT);
      CMP_TEST_OK(snackbar.widget.vtable->get_semantics(snackbar.widget.ctx,
                                                        &semantics));
      CMP_TEST_ASSERT(semantics.utf8_label == snackbar.utf8_message);
    }

    CMP_TEST_EXPECT(snackbar.widget.vtable->event(NULL, &event, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        snackbar.widget.vtable->event(snackbar.widget.ctx, NULL, &handled),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, NULL),
        CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(snackbar.widget.vtable->destroy(NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    backend.fail_destroy = CMP_ERR_IO;
    CMP_TEST_EXPECT(snackbar.widget.vtable->destroy(snackbar.widget.ctx),
                    CMP_ERR_IO);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    M3SnackbarStyle style;
    M3Snackbar snackbar;
    const char *family;
    const char *message;
    cmp_usize message_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    message = "Snack";
    message_len = (cmp_usize)strlen(message);

    CMP_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    CMP_TEST_OK(m3_snackbar_init(&snackbar, &text_backend, &style, message,
                                 message_len));

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 1;
    CMP_TEST_EXPECT(m3_snackbar_set_style(&snackbar, &style), CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 0;
    CMP_TEST_EXPECT(m3_snackbar_set_style(&snackbar, &style), CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 2;
    backend.fail_destroy_at = 1;
    CMP_TEST_EXPECT(m3_snackbar_set_style(&snackbar, &style), CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_create_at = 0;
    backend.fail_destroy_at = 1;
    CMP_TEST_EXPECT(m3_snackbar_set_style(&snackbar, &style), CMP_ERR_IO);

    backend.create_calls = 0;
    backend.destroy_calls = 0;
    backend.fail_destroy_at = 0;
    CMP_TEST_OK(m3_snackbar_set_style(&snackbar, &style));

    CMP_TEST_EXPECT(m3_snackbar_set_style(NULL, &style),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(m3_snackbar_set_style(&snackbar, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  {
    TestDialogBackend backend;
    CMPTextBackend text_backend;
    CMPGfx gfx;
    CMPPaintContext ctx;
    M3SnackbarStyle style;
    M3Snackbar snackbar;
    CMPMeasureSpec width_spec;
    CMPMeasureSpec height_spec;
    CMPSize size;
    CMPRect bounds;
    CMPInputEvent event;
    CMPBool handled;
    CMPSemantics semantics;
    TestActionState state;
    const char *family;
    const char *message;
    const char *action;
    cmp_usize message_len;
    cmp_usize action_len;

    test_backend_init(&backend);
    test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
    family = "Test";
    message = "Snack";
    action = "Act";
    message_len = (cmp_usize)strlen(message);
    action_len = (cmp_usize)strlen(action);

    CMP_TEST_OK(m3_snackbar_style_init(&style));
    test_set_snackbar_families(&style, family);
    CMP_TEST_OK(m3_snackbar_init(&snackbar, &text_backend, &style, message,
                                 message_len));
    CMP_TEST_OK(m3_snackbar_set_action(&snackbar, action, action_len));

    width_spec.mode = 99u;
    width_spec.size = 0.0f;
    height_spec.mode = CMP_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    CMP_TEST_EXPECT(snackbar.widget.vtable->measure(
                        snackbar.widget.ctx, width_spec, height_spec, &size),
                    CMP_ERR_INVALID_ARGUMENT);
    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    CMP_TEST_EXPECT(snackbar.widget.vtable->measure(
                        snackbar.widget.ctx, width_spec, height_spec, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    snackbar.utf8_message = NULL;
    snackbar.message_len = 0u;
    snackbar.utf8_action = action;
    snackbar.action_len = action_len;
    snackbar.metrics_valid = CMP_FALSE;
    backend.fail_measure = CMP_ERR_IO;
    CMP_TEST_EXPECT(snackbar.widget.vtable->measure(
                        snackbar.widget.ctx, width_spec, height_spec, &size),
                    CMP_ERR_IO);
    backend.fail_measure = CMP_OK;

    snackbar.utf8_message = message;
    snackbar.message_len = message_len;
    snackbar.metrics_valid = CMP_FALSE;
    CMP_TEST_OK(m3_dialog_test_set_fail_point(
        M3_DIALOG_TEST_FAIL_SNACKBAR_COMPUTE_ACTION));
    CMP_TEST_EXPECT(snackbar.widget.vtable->measure(
                        snackbar.widget.ctx, width_spec, height_spec, &size),
                    CMP_ERR_IO);
    CMP_TEST_OK(m3_dialog_test_clear_fail_points());

    width_spec.mode = CMP_MEASURE_EXACTLY;
    width_spec.size = 300.0f;
    height_spec.mode = CMP_MEASURE_EXACTLY;
    height_spec.size = 200.0f;
    CMP_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                                height_spec, &size));

    width_spec.mode = CMP_MEASURE_AT_MOST;
    width_spec.size = 80.0f;
    height_spec.mode = CMP_MEASURE_AT_MOST;
    height_spec.size = 80.0f;
    CMP_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                                height_spec, &size));

    style.min_width = 400.0f;
    style.max_width = 0.0f;
    snackbar.style = style;
    width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    width_spec.size = 0.0f;
    height_spec.mode = CMP_MEASURE_UNSPECIFIED;
    height_spec.size = 0.0f;
    CMP_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                                height_spec, &size));

    style.min_width = 0.0f;
    style.max_width = 50.0f;
    snackbar.style = style;
    CMP_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                                height_spec, &size));

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = -1.0f;
    bounds.height = 10.0f;
    CMP_TEST_EXPECT(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(m3_snackbar_set_action(&snackbar, NULL, 0u));
    bounds.width = 20.0f;
    bounds.height = 20.0f;
    CMP_TEST_OK(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds));
    CMP_TEST_ASSERT(snackbar.action_bounds.width == 0.0f);

    CMP_TEST_OK(m3_snackbar_set_action(&snackbar, action, action_len));
    CMP_TEST_OK(m3_dialog_test_set_fail_point(
        M3_DIALOG_TEST_FAIL_SNACKBAR_COMPUTE_ACTION));
    CMP_TEST_EXPECT(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds),
                    CMP_ERR_IO);
    CMP_TEST_OK(m3_dialog_test_clear_fail_points());

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    ctx.clip = bounds;
    ctx.dpi_scale = 1.0f;
    CMP_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx),
                    CMP_ERR_UNSUPPORTED);

    snackbar.utf8_message = NULL;
    snackbar.message_len = 0u;
    snackbar.utf8_action = NULL;
    snackbar.action_len = 0u;
    snackbar.metrics_valid = CMP_FALSE;
    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable, NULL);
    ctx.gfx = &gfx;
    CMP_TEST_OK(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx));

    snackbar.utf8_message = message;
    snackbar.message_len = message_len;
    snackbar.metrics_valid = CMP_FALSE;
    CMP_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx),
                    CMP_ERR_UNSUPPORTED);

    snackbar.utf8_action = action;
    snackbar.action_len = action_len;
    snackbar.metrics_valid = CMP_FALSE;
    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    backend.fail_draw_rect = CMP_ERR_IO;
    CMP_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw_rect = CMP_OK;

    backend.fail_draw = CMP_ERR_IO;
    CMP_TEST_EXPECT(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx),
                    CMP_ERR_IO);
    backend.fail_draw = CMP_OK;

    test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_clip,
                           &g_test_text_vtable);
    ctx.gfx = &gfx;
    CMP_TEST_OK(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx));
    CMP_TEST_OK(m3_dialog_test_set_fail_point(
        M3_DIALOG_TEST_FAIL_SNACKBAR_AVAILABLE_HEIGHT_NEGATIVE));
    CMP_TEST_OK(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx));
    CMP_TEST_OK(m3_dialog_test_clear_fail_points());

    snackbar.pressed_action = CMP_TRUE;
    memset(&event, 0, sizeof(event));
    event.type = CMP_INPUT_POINTER_DOWN;
    CMP_TEST_EXPECT(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled),
        CMP_ERR_STATE);
    snackbar.pressed_action = CMP_FALSE;

    CMP_TEST_OK(
        m3_snackbar_set_on_action(&snackbar, test_snackbar_action, &state));
    state.fail_next = 1;
    CMP_TEST_OK(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds));
    event.type = CMP_INPUT_POINTER_UP;
    event.data.pointer.x = (cmp_i32)(snackbar.action_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(snackbar.action_bounds.y + 1.0f);
    snackbar.pressed_action = CMP_TRUE;
    CMP_TEST_EXPECT(
        snackbar.widget.vtable->event(snackbar.widget.ctx, &event, &handled),
        CMP_ERR_IO);

    snackbar.widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
    CMP_TEST_OK(
        snackbar.widget.vtable->get_semantics(snackbar.widget.ctx, &semantics));
    CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE) != 0u);
    snackbar.widget.flags = 0u;

    CMP_TEST_OK(snackbar.widget.vtable->destroy(snackbar.widget.ctx));
  }

  return 0;
}

static int test_snackbar_paint_valid(void) {
  TestDialogBackend backend;
  CMPTextBackend text_backend;
  CMPGfx gfx;
  CMPPaintContext ctx;
  M3SnackbarStyle style;
  M3Snackbar snackbar;
  CMPRect bounds;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  const char *family;
  const char *message;

  test_backend_init(&backend);
  test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
  test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                         &g_test_text_vtable);

  family = "Test";
  message = "Hello";
  CMP_TEST_OK(m3_snackbar_style_init(&style));
  test_set_snackbar_families(&style, family);
  CMP_TEST_OK(m3_snackbar_init(&snackbar, &text_backend, &style, message,
                               (cmp_usize)strlen(message)));

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 200.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 80.0f;
  CMP_TEST_OK(snackbar.widget.vtable->measure(snackbar.widget.ctx, width_spec,
                                              height_spec, &size));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = size.width;
  bounds.height = size.height;
  CMP_TEST_OK(snackbar.widget.vtable->layout(snackbar.widget.ctx, bounds));

  ctx.gfx = &gfx;
  ctx.clip = bounds;
  ctx.dpi_scale = 1.0f;
  snackbar.metrics_valid = CMP_FALSE;
  CMP_TEST_OK(snackbar.widget.vtable->paint(snackbar.widget.ctx, &ctx));

  CMP_TEST_OK(snackbar.widget.vtable->destroy(snackbar.widget.ctx));
  return 0;
}

static int test_dialog_branch_sweep(void) {
  CMPTextStyle text_style;
  CMPRect rect;
  CMPBool inside;
  TestDialogBackend backend;
  CMPTextBackend text_backend;
  CMPGfx gfx;
  CMPPaintContext ctx;
  M3AlertDialogStyle alert_style;
  M3AlertDialog dialog;
  M3AlertDialog dialog2;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPScalar total_width;
  CMPScalar actions_height;
  CMPScalar confirm_width;
  CMPScalar dismiss_width;
  CMPInputEvent event;
  CMPBool handled;
  TestActionState action_state;
  CMPSemantics semantics;
  CMPHandle fonts[3];
  const char *title = "Title";
  const char *body = "Body";
  const char *confirm = "OK";
  const char *dismiss = "Cancel";
  cmp_usize title_len = 5u;
  cmp_usize body_len = 4u;
  cmp_usize confirm_len = 2u;
  cmp_usize dismiss_len = 6u;

  memset(&action_state, 0, sizeof(action_state));
  CMP_TEST_OK(cmp_text_style_init(&text_style));
  text_style.utf8_family = "Test";
  text_style.italic = CMP_TRUE;
  CMP_TEST_OK(m3_dialog_test_validate_text_style(&text_style, CMP_TRUE));

  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = 10.0f;
  rect.height = 0.0f;
  CMP_TEST_OK(m3_dialog_test_point_in_rect(&rect, 5.0f, 0.0f, &inside));
  CMP_TEST_ASSERT(inside == CMP_FALSE);

  test_backend_init(&backend);
  test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
  fonts[0].id = 1u;
  fonts[0].generation = 1u;
  fonts[1].id = 0u;
  fonts[1].generation = 0u;
  fonts[2].id = 2u;
  fonts[2].generation = 2u;
  backend.fail_destroy = CMP_ERR_IO;
  CMP_TEST_EXPECT(m3_dialog_test_destroy_fonts(&text_backend, fonts, 3u),
                  CMP_ERR_IO);
  backend.fail_destroy = CMP_OK;
  fonts[0].id = 1u;
  fonts[0].generation = 1u;
  fonts[1].id = 0u;
  fonts[1].generation = 0u;
  fonts[2].id = 2u;
  fonts[2].generation = 2u;
  CMP_TEST_OK(m3_dialog_test_destroy_fonts(&text_backend, fonts, 3u));

  CMP_TEST_OK(m3_alert_dialog_style_init(&alert_style));
  memset(&dialog, 0, sizeof(dialog));
  dialog.style = alert_style;
  dialog.confirm_metrics.width = 20.0f;
  dialog.confirm_metrics.height = 10.0f;
  dialog.confirm_metrics.baseline = 7.0f;
  dialog.dismiss_metrics.width = 24.0f;
  dialog.dismiss_metrics.height = 12.0f;
  dialog.dismiss_metrics.baseline = 8.0f;

  dialog.confirm_len = confirm_len;
  dialog.dismiss_len = dismiss_len;
  CMP_TEST_OK(m3_dialog_test_alert_compute_actions(
      &dialog, &total_width, &actions_height, &confirm_width, &dismiss_width));

  dialog.dismiss_len = 0u;
  CMP_TEST_OK(m3_dialog_test_alert_compute_actions(
      &dialog, &total_width, &actions_height, &confirm_width, &dismiss_width));

  dialog.confirm_len = 0u;
  dialog.dismiss_len = dismiss_len;
  CMP_TEST_OK(m3_dialog_test_alert_compute_actions(
      &dialog, &total_width, &actions_height, &confirm_width, &dismiss_width));

  dialog.confirm_len = 0u;
  dialog.dismiss_len = 0u;
  CMP_TEST_OK(m3_dialog_test_alert_compute_actions(
      &dialog, &total_width, &actions_height, &confirm_width, &dismiss_width));

  dialog.confirm_len = confirm_len;
  dialog.dismiss_len = dismiss_len;
  CMP_TEST_OK(
      m3_dialog_test_set_fail_point(M3_DIALOG_TEST_FAIL_ALERT_COMPUTE_ACTIONS));
  CMP_TEST_EXPECT(m3_dialog_test_alert_compute_actions(
                      &dialog, &total_width, &actions_height, &confirm_width,
                      &dismiss_width),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_dialog_test_clear_fail_points());

  dialog.title_len = 1u;
  dialog.body_len = 1u;
  dialog.title_metrics.height = 10.0f;
  dialog.body_metrics.height = 8.0f;
  dialog.style.padding.left = 10.0f;
  dialog.style.padding.right = 10.0f;
  dialog.style.padding.top = 5.0f;
  dialog.style.padding.bottom = 5.0f;
  dialog.style.title_body_spacing = 4.0f;
  dialog.style.body_action_spacing = 6.0f;

  dialog.bounds.x = 0.0f;
  dialog.bounds.y = 0.0f;
  dialog.bounds.width = 40.0f;
  dialog.bounds.height = 60.0f;
  dialog.confirm_len = 0u;
  dialog.dismiss_len = 0u;
  CMP_TEST_OK(m3_dialog_test_alert_layout_actions(&dialog));
  CMP_TEST_ASSERT(dialog.confirm_bounds.width == 0.0f);
  CMP_TEST_ASSERT(dialog.dismiss_bounds.width == 0.0f);

  dialog.confirm_len = confirm_len;
  dialog.dismiss_len = 0u;
  CMP_TEST_OK(m3_dialog_test_alert_layout_actions(&dialog));
  CMP_TEST_ASSERT(dialog.confirm_bounds.width > 0.0f);
  CMP_TEST_ASSERT(dialog.dismiss_bounds.width == 0.0f);

  dialog.confirm_len = 0u;
  dialog.dismiss_len = dismiss_len;
  CMP_TEST_OK(m3_dialog_test_alert_layout_actions(&dialog));
  CMP_TEST_ASSERT(dialog.dismiss_bounds.width > 0.0f);
  CMP_TEST_ASSERT(dialog.confirm_bounds.width == 0.0f);

  dialog.confirm_len = confirm_len;
  dialog.dismiss_len = dismiss_len;
  dialog.bounds.width = 200.0f;
  CMP_TEST_OK(m3_dialog_test_alert_layout_actions(&dialog));
  CMP_TEST_ASSERT(dialog.dismiss_bounds.width > 0.0f);
  CMP_TEST_ASSERT(dialog.confirm_bounds.width > 0.0f);

  test_backend_init(&backend);
  test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
  CMP_TEST_OK(m3_alert_dialog_style_init(&alert_style));
  test_set_alert_families(&alert_style, "Test");
  CMP_TEST_OK(m3_alert_dialog_init(&dialog2, &text_backend, &alert_style, title,
                                   title_len, body, body_len));
  CMP_TEST_OK(m3_alert_dialog_set_actions(&dialog2, confirm, confirm_len,
                                          dismiss, dismiss_len));

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 120.0f;
  height_spec = width_spec;
  CMP_TEST_OK(dialog2.widget.vtable->measure(dialog2.widget.ctx, width_spec,
                                             height_spec, &size));

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 80.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 40.0f;
  CMP_TEST_OK(dialog2.widget.vtable->measure(dialog2.widget.ctx, width_spec,
                                             height_spec, &size));

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec = width_spec;
  CMP_TEST_OK(dialog2.widget.vtable->measure(dialog2.widget.ctx, width_spec,
                                             height_spec, &size));

  CMP_TEST_OK(
      m3_dialog_test_set_fail_point(M3_DIALOG_TEST_FAIL_ALERT_COMPUTE_ACTIONS));
  CMP_TEST_EXPECT(dialog2.widget.vtable->measure(dialog2.widget.ctx, width_spec,
                                                 height_spec, &size),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_dialog_test_clear_fail_points());

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 200.0f;
  bounds.height = 120.0f;
  CMP_TEST_OK(dialog2.widget.vtable->layout(dialog2.widget.ctx, bounds));

  test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable_no_draw,
                         &g_test_text_vtable);
  ctx.gfx = &gfx;
  ctx.clip = bounds;
  ctx.dpi_scale = 1.0f;
  CMP_TEST_EXPECT(dialog2.widget.vtable->paint(dialog2.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);

  test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable, NULL);
  ctx.gfx = &gfx;
  dialog2.utf8_title = NULL;
  dialog2.title_len = 0u;
  dialog2.utf8_body = NULL;
  dialog2.body_len = 0u;
  dialog2.utf8_confirm = NULL;
  dialog2.confirm_len = 0u;
  dialog2.utf8_dismiss = NULL;
  dialog2.dismiss_len = 0u;
  dialog2.metrics_valid = CMP_TRUE;
  CMP_TEST_OK(dialog2.widget.vtable->paint(dialog2.widget.ctx, &ctx));

  dialog2.utf8_title = title;
  dialog2.title_len = title_len;
  dialog2.metrics_valid = CMP_TRUE;
  CMP_TEST_EXPECT(dialog2.widget.vtable->paint(dialog2.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);

  test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                         &g_test_text_vtable);
  ctx.gfx = &gfx;
  backend.fail_draw_rect = CMP_ERR_IO;
  CMP_TEST_EXPECT(dialog2.widget.vtable->paint(dialog2.widget.ctx, &ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect = CMP_OK;
  backend.fail_draw = CMP_ERR_IO;
  CMP_TEST_EXPECT(dialog2.widget.vtable->paint(dialog2.widget.ctx, &ctx),
                  CMP_ERR_IO);
  backend.fail_draw = CMP_OK;

  dialog2.metrics_valid = CMP_TRUE;
  dialog2.confirm_metrics.width = 1.0f;
  dialog2.confirm_metrics.height = 1.0f;
  dialog2.confirm_metrics.baseline = 1.0f;
  dialog2.dismiss_metrics.width = 1.0f;
  dialog2.dismiss_metrics.height = 1.0f;
  dialog2.dismiss_metrics.baseline = 1.0f;
  CMP_TEST_OK(dialog2.widget.vtable->paint(dialog2.widget.ctx, &ctx));

  {
    CMPTextMetrics action_metrics;
    CMPRect action_bounds;

    action_metrics.width = 6.0f;
    action_metrics.height = 10.0f;
    action_metrics.baseline = 7.0f;
    action_bounds = dialog2.confirm_bounds;
    CMP_TEST_OK(m3_dialog_test_alert_draw_action_text(
        &dialog2, &action_bounds, &action_metrics, confirm, confirm_len, &ctx));
    CMP_TEST_OK(m3_dialog_test_alert_draw_action_text(
        &dialog2, &action_bounds, &action_metrics, confirm, 0u, &ctx));
  }

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_POINTER_DOWN;
  dialog2.pressed_confirm = CMP_TRUE;
  CMP_TEST_EXPECT(
      dialog2.widget.vtable->event(dialog2.widget.ctx, &event, &handled),
      CMP_ERR_STATE);
  dialog2.pressed_confirm = CMP_FALSE;

  event.data.pointer.x = (cmp_i32)(dialog2.confirm_bounds.x + 1.0f);
  event.data.pointer.y = (cmp_i32)(dialog2.confirm_bounds.y + 1.0f);
  CMP_TEST_OK(
      dialog2.widget.vtable->event(dialog2.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(dialog2.pressed_confirm == CMP_TRUE);

  CMP_TEST_OK(m3_alert_dialog_set_on_action(&dialog2, test_alert_action,
                                            &action_state));
  action_state.fail_next = 1;
  event.type = CMP_INPUT_POINTER_UP;
  CMP_TEST_EXPECT(
      dialog2.widget.vtable->event(dialog2.widget.ctx, &event, &handled),
      CMP_ERR_IO);

  dialog2.widget.flags = CMP_WIDGET_FLAG_DISABLED | CMP_WIDGET_FLAG_FOCUSABLE;
  memset(&semantics, 0, sizeof(semantics));
  CMP_TEST_OK(
      dialog2.widget.vtable->get_semantics(dialog2.widget.ctx, &semantics));
  dialog2.widget.flags = 0u;

  dialog2.owns_fonts = CMP_FALSE;
  CMP_TEST_OK(dialog2.widget.vtable->destroy(dialog2.widget.ctx));

  {
    TestDialogBackend fs_backend;
    CMPTextBackend fs_text_backend;
    CMPGfx fs_gfx;
    CMPPaintContext fs_ctx;
    M3FullScreenDialogStyle fs_style;
    M3FullScreenDialog fs;
    CMPMeasureSpec fs_width_spec;
    CMPMeasureSpec fs_height_spec;
    CMPSize fs_size;
    CMPRect fs_bounds;
    CMPTextMetrics fs_action_metrics;
    CMPRect fs_action_bounds;
    const char *fs_title = "T";
    const char *fs_body = "Longer body text";
    const char *fs_action = "Action";
    cmp_usize fs_title_len = (cmp_usize)strlen(fs_title);
    cmp_usize fs_body_len = (cmp_usize)strlen(fs_body);
    cmp_usize fs_action_len = (cmp_usize)strlen(fs_action);

    test_backend_init(&fs_backend);
    test_setup_text_backend(&fs_backend, &fs_text_backend, &g_test_text_vtable);
    test_setup_gfx_backend(&fs_backend, &fs_gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);
    fs_ctx.gfx = &fs_gfx;
    fs_ctx.clip.x = 0.0f;
    fs_ctx.clip.y = 0.0f;
    fs_ctx.clip.width = 300.0f;
    fs_ctx.clip.height = 200.0f;
    fs_ctx.dpi_scale = 1.0f;

    CMP_TEST_OK(m3_fullscreen_dialog_style_init(&fs_style));
    test_set_fullscreen_families(&fs_style, "Test");
    CMP_TEST_OK(m3_fullscreen_dialog_init(&fs, &fs_text_backend, &fs_style,
                                          fs_title, fs_title_len, fs_body,
                                          fs_body_len));
    CMP_TEST_OK(m3_fullscreen_dialog_set_action(&fs, fs_action, fs_action_len));

    fs_width_spec.mode = CMP_MEASURE_UNSPECIFIED;
    fs_width_spec.size = 0.0f;
    fs_height_spec = fs_width_spec;
    CMP_TEST_OK(fs.widget.vtable->measure(fs.widget.ctx, fs_width_spec,
                                          fs_height_spec, &fs_size));

    fs_bounds.x = 0.0f;
    fs_bounds.y = 0.0f;
    fs_bounds.width = 200.0f;
    fs_bounds.height = 100.0f;
    CMP_TEST_OK(fs.widget.vtable->layout(fs.widget.ctx, fs_bounds));

    test_setup_gfx_backend(&fs_backend, &fs_gfx, &g_test_gfx_vtable, NULL);
    fs_ctx.gfx = &fs_gfx;
    fs.utf8_title = NULL;
    fs.title_len = 0u;
    fs.utf8_body = NULL;
    fs.body_len = 0u;
    fs.utf8_action = NULL;
    fs.action_len = 0u;
    fs.metrics_valid = CMP_FALSE;
    CMP_TEST_OK(fs.widget.vtable->paint(fs.widget.ctx, &fs_ctx));

    test_setup_gfx_backend(&fs_backend, &fs_gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);
    fs_ctx.gfx = &fs_gfx;
    fs.utf8_title = fs_title;
    fs.title_len = fs_title_len;
    fs.utf8_body = fs_body;
    fs.body_len = fs_body_len;
    fs.utf8_action = fs_action;
    fs.action_len = fs_action_len;
    fs.metrics_valid = CMP_FALSE;
    CMP_TEST_OK(fs.widget.vtable->layout(fs.widget.ctx, fs_bounds));
    CMP_TEST_OK(fs.widget.vtable->paint(fs.widget.ctx, &fs_ctx));

    fs_action_metrics.width = 8.0f;
    fs_action_metrics.height = 12.0f;
    fs_action_metrics.baseline = 9.0f;
    fs_action_bounds = fs.action_bounds;
    CMP_TEST_OK(m3_dialog_test_fullscreen_draw_action_text(
        &fs, &fs_action_bounds, &fs_action_metrics, fs_action, 0u, &fs_ctx));
    fs_action_bounds.height =
        fs_action_metrics.height + fs_style.action_padding_y * 4.0f + 10.0f;
    CMP_TEST_OK(m3_dialog_test_fullscreen_draw_action_text(
        &fs, &fs_action_bounds, &fs_action_metrics, fs_action, fs_action_len,
        &fs_ctx));

    CMP_TEST_OK(fs.widget.vtable->destroy(fs.widget.ctx));
  }

  {
    TestDialogBackend snack_backend;
    CMPTextBackend snack_text_backend;
    CMPGfx snack_gfx;
    CMPPaintContext snack_ctx;
    M3SnackbarStyle snack_style;
    M3Snackbar snack;
    CMPRect snack_bounds;
    const char *message = "Msg";
    const char *action = "Act";
    cmp_usize message_len = (cmp_usize)strlen(message);
    cmp_usize action_len = (cmp_usize)strlen(action);

    test_backend_init(&snack_backend);
    test_setup_text_backend(&snack_backend, &snack_text_backend,
                            &g_test_text_vtable);
    CMP_TEST_OK(m3_snackbar_style_init(&snack_style));
    test_set_snackbar_families(&snack_style, "Test");
    CMP_TEST_OK(m3_snackbar_init(&snack, &snack_text_backend, &snack_style,
                                 message, message_len));
    CMP_TEST_OK(m3_snackbar_set_action(&snack, action, action_len));

    snack_bounds.x = 0.0f;
    snack_bounds.y = 0.0f;
    snack_bounds.width = 10.0f;
    snack_bounds.height = 10.0f;
    CMP_TEST_OK(snack.widget.vtable->layout(snack.widget.ctx, snack_bounds));

    test_setup_gfx_backend(&snack_backend, &snack_gfx, &g_test_gfx_vtable,
                           &g_test_text_vtable);
    snack_ctx.gfx = &snack_gfx;
    snack_ctx.clip = snack_bounds;
    snack_ctx.dpi_scale = 1.0f;
    snack.style.padding.left = 20.0f;
    snack.style.padding.right = 20.0f;
    snack.style.padding.top = 20.0f;
    snack.style.padding.bottom = 20.0f;
    snack.metrics_valid = CMP_FALSE;
    CMP_TEST_OK(snack.widget.vtable->paint(snack.widget.ctx, &snack_ctx));

    snack.action_bounds.height = snack.action_metrics.height +
                                 snack.style.action_padding_y * 4.0f + 6.0f;
    snack.metrics_valid = CMP_TRUE;
    CMP_TEST_OK(snack.widget.vtable->paint(snack.widget.ctx, &snack_ctx));

    CMP_TEST_OK(snack.widget.vtable->destroy(snack.widget.ctx));
  }

  return 0;
}

static int test_dialog_extra_coverage(void) {
  TestDialogBackend backend;
  CMPTextBackend text_backend;
  CMPGfx gfx;
  CMPPaintContext ctx;
  M3AlertDialogStyle alert_style;
  M3AlertDialog alert;
  M3FullScreenDialogStyle fs_style;
  M3FullScreenDialog fs;
  M3SnackbarStyle snack_style;
  M3Snackbar snack;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPRect action_bounds;
  CMPTextMetrics metrics;
  CMPInputEvent event;
  CMPBool handled;
  CMPScalar action_width;
  CMPScalar action_height;
  CMPLayoutEdges saved_padding;
  CMPScalar saved_min_height;
  const char *family;
  const char *title;
  const char *body;
  const char *long_body;
  const char *confirm;
  const char *dismiss;
  const char *action;
  cmp_usize title_len;
  cmp_usize body_len;
  cmp_usize long_body_len;
  cmp_usize confirm_len;
  cmp_usize dismiss_len;
  cmp_usize action_len;

  test_backend_init(&backend);
  test_setup_text_backend(&backend, &text_backend, &g_test_text_vtable);
  test_setup_gfx_backend(&backend, &gfx, &g_test_gfx_vtable,
                         &g_test_text_vtable);

  family = "Test";
  title = "Title";
  body = "Body";
  long_body = "Longer body text";
  confirm = "OK";
  dismiss = "No";
  action = "Action";
  title_len = (cmp_usize)strlen(title);
  body_len = (cmp_usize)strlen(body);
  long_body_len = (cmp_usize)strlen(long_body);
  confirm_len = (cmp_usize)strlen(confirm);
  dismiss_len = (cmp_usize)strlen(dismiss);
  action_len = (cmp_usize)strlen(action);

  CMP_TEST_OK(m3_alert_dialog_style_init(&alert_style));
  test_set_alert_families(&alert_style, family);
  CMP_TEST_OK(m3_alert_dialog_init(&alert, &text_backend, &alert_style, title,
                                   title_len, body, body_len));
  CMP_TEST_OK(m3_alert_dialog_set_actions(&alert, confirm, confirm_len, dismiss,
                                          dismiss_len));

  CMP_TEST_OK(m3_fullscreen_dialog_style_init(&fs_style));
  test_set_fullscreen_families(&fs_style, family);
  CMP_TEST_OK(m3_fullscreen_dialog_init(&fs, &text_backend, &fs_style, title,
                                        title_len, body, body_len));
  CMP_TEST_OK(m3_fullscreen_dialog_set_action(&fs, action, action_len));

  CMP_TEST_OK(m3_snackbar_style_init(&snack_style));
  test_set_snackbar_families(&snack_style, family);
  CMP_TEST_OK(
      m3_snackbar_init(&snack, &text_backend, &snack_style, body, body_len));
  CMP_TEST_OK(m3_snackbar_set_action(&snack, action, action_len));

  CMP_TEST_OK(
      m3_dialog_test_set_fail_point(M3_DIALOG_TEST_FAIL_ALERT_COMPUTE_ACTIONS));
  CMP_TEST_OK(m3_dialog_test_fullscreen_compute_action(&fs, &action_width,
                                                       &action_height));
  CMP_TEST_OK(m3_dialog_test_clear_fail_points());

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec = width_spec;
  backend.fail_measure = CMP_ERR_IO;
  alert.metrics_valid = CMP_FALSE;
  CMP_TEST_EXPECT(alert.widget.vtable->measure(alert.widget.ctx, width_spec,
                                               height_spec, &size),
                  CMP_ERR_IO);
  backend.fail_measure = CMP_OK;

  alert.style.min_width = 200.0f;
  alert.style.max_width = 0.0f;
  CMP_TEST_OK(alert.widget.vtable->measure(alert.widget.ctx, width_spec,
                                           height_spec, &size));
  CMP_TEST_ASSERT(size.width >= alert.style.min_width);

  alert.style.min_width = 0.0f;
  alert.style.max_width = 30.0f;
  CMP_TEST_OK(alert.widget.vtable->measure(alert.widget.ctx, width_spec,
                                           height_spec, &size));
  CMP_TEST_ASSERT(size.width <= alert.style.max_width);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 25.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 25.0f;
  CMP_TEST_OK(alert.widget.vtable->measure(alert.widget.ctx, width_spec,
                                           height_spec, &size));

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 40.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 50.0f;
  CMP_TEST_OK(alert.widget.vtable->measure(alert.widget.ctx, width_spec,
                                           height_spec, &size));

  CMP_TEST_OK(m3_alert_dialog_set_body(&alert, long_body, long_body_len));
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec = width_spec;
  CMP_TEST_OK(alert.widget.vtable->measure(alert.widget.ctx, width_spec,
                                           height_spec, &size));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 10.0f;
  bounds.height = 10.0f;
  saved_padding = alert.style.padding;
  alert.style.padding.left = 20.0f;
  alert.style.padding.right = 20.0f;
  alert.style.padding.top = 20.0f;
  alert.style.padding.bottom = 20.0f;
  CMP_TEST_OK(alert.widget.vtable->layout(alert.widget.ctx, bounds));

  ctx.gfx = &gfx;
  ctx.clip = bounds;
  ctx.dpi_scale = 1.0f;

  alert.style.scrim_enabled = CMP_TRUE;
  alert.style.shadow_enabled = CMP_FALSE;
  backend.fail_draw_rect = CMP_ERR_IO;
  CMP_TEST_EXPECT(alert.widget.vtable->paint(alert.widget.ctx, &ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect = CMP_OK;
  CMP_TEST_OK(alert.widget.vtable->paint(alert.widget.ctx, &ctx));
  alert.style.scrim_enabled = CMP_FALSE;
  alert.style.padding = saved_padding;

  metrics.width = 10.0f;
  metrics.height = 8.0f;
  metrics.baseline = 6.0f;
  action_bounds.x = 0.0f;
  action_bounds.y = 0.0f;
  action_bounds.width =
      metrics.width + alert.style.action_padding_x * 2.0f + 20.0f;
  action_bounds.height =
      metrics.height + alert.style.action_padding_y * 2.0f + 20.0f;
  CMP_TEST_OK(m3_dialog_test_alert_draw_action_text(
      &alert, &action_bounds, &metrics, confirm, confirm_len, &ctx));
  action_bounds.width =
      metrics.width + alert.style.action_padding_x * 2.0f - 1.0f;
  action_bounds.height =
      metrics.height + alert.style.action_padding_y * 2.0f - 1.0f;
  CMP_TEST_OK(m3_dialog_test_alert_draw_action_text(
      &alert, &action_bounds, &metrics, confirm, confirm_len, &ctx));

  CMP_TEST_OK(alert.widget.vtable->layout(alert.widget.ctx, bounds));
  alert.on_action = NULL;
  alert.pressed_confirm = CMP_TRUE;
  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_POINTER_UP;
  event.data.pointer.x = (cmp_i32)(alert.confirm_bounds.x + 1.0f);
  event.data.pointer.y = (cmp_i32)(alert.confirm_bounds.y + 1.0f);
  CMP_TEST_OK(alert.widget.vtable->event(alert.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  alert.pressed_confirm = CMP_FALSE;

  event.type = CMP_INPUT_POINTER_DOWN;
  event.data.pointer.x = (cmp_i32)(bounds.x - 5.0f);
  event.data.pointer.y = (cmp_i32)(bounds.y - 5.0f);
  CMP_TEST_OK(alert.widget.vtable->event(alert.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  {
    TestActionState action_state;

    action_state.calls = 0;
    action_state.last_action = 0u;
    action_state.fail_next = 1;
    CMP_TEST_OK(m3_alert_dialog_set_on_action(&alert, test_alert_action,
                                              &action_state));
    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = (cmp_i32)(alert.confirm_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(alert.confirm_bounds.y + 1.0f);
    CMP_TEST_OK(alert.widget.vtable->event(alert.widget.ctx, &event, &handled));
    event.type = CMP_INPUT_POINTER_UP;
    CMP_TEST_EXPECT(
        alert.widget.vtable->event(alert.widget.ctx, &event, &handled),
        CMP_ERR_IO);
  }

  CMP_TEST_OK(alert.widget.vtable->destroy(alert.widget.ctx));

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec = width_spec;
  CMP_TEST_OK(
      fs.widget.vtable->measure(fs.widget.ctx, width_spec, height_spec, &size));
  CMP_TEST_OK(m3_fullscreen_dialog_set_body(&fs, long_body, long_body_len));
  CMP_TEST_OK(
      fs.widget.vtable->measure(fs.widget.ctx, width_spec, height_spec, &size));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 10.0f;
  bounds.height = 10.0f;
  saved_padding = fs.style.padding;
  fs.style.padding.left = 20.0f;
  fs.style.padding.right = 20.0f;
  fs.style.padding.top = 20.0f;
  fs.style.padding.bottom = 20.0f;
  CMP_TEST_OK(fs.widget.vtable->layout(fs.widget.ctx, bounds));

  ctx.clip = bounds;
  fs.style.scrim_enabled = CMP_TRUE;
  fs.style.shadow_enabled = CMP_FALSE;
  backend.fail_draw_rect = CMP_ERR_IO;
  CMP_TEST_EXPECT(fs.widget.vtable->paint(fs.widget.ctx, &ctx), CMP_ERR_IO);
  backend.fail_draw_rect = CMP_OK;
  CMP_TEST_OK(fs.widget.vtable->paint(fs.widget.ctx, &ctx));
  fs.style.scrim_enabled = CMP_FALSE;
  fs.style.padding = saved_padding;

  metrics.width = 12.0f;
  metrics.height = 10.0f;
  metrics.baseline = 7.0f;
  action_bounds.x = 0.0f;
  action_bounds.y = 0.0f;
  action_bounds.width =
      metrics.width + fs.style.action_padding_x * 2.0f + 20.0f;
  action_bounds.height =
      metrics.height + fs.style.action_padding_y * 2.0f + 20.0f;
  CMP_TEST_OK(m3_dialog_test_fullscreen_draw_action_text(
      &fs, &action_bounds, &metrics, action, action_len, &ctx));
  action_bounds.width = metrics.width + fs.style.action_padding_x * 2.0f - 1.0f;
  action_bounds.height =
      metrics.height + fs.style.action_padding_y * 2.0f - 1.0f;
  CMP_TEST_OK(m3_dialog_test_fullscreen_draw_action_text(
      &fs, &action_bounds, &metrics, action, action_len, &ctx));

  {
    TestActionState action_state;

    action_state.calls = 0;
    action_state.last_action = 0u;
    action_state.fail_next = 1;
    CMP_TEST_OK(m3_fullscreen_dialog_set_on_action(&fs, test_fullscreen_action,
                                                   &action_state));
    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = (cmp_i32)(fs.action_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(fs.action_bounds.y + 1.0f);
    CMP_TEST_OK(fs.widget.vtable->event(fs.widget.ctx, &event, &handled));
    event.type = CMP_INPUT_POINTER_UP;
    CMP_TEST_EXPECT(fs.widget.vtable->event(fs.widget.ctx, &event, &handled),
                    CMP_ERR_IO);
  }

  CMP_TEST_OK(fs.widget.vtable->destroy(fs.widget.ctx));

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec = width_spec;
  saved_min_height = snack.style.min_height;
  snack.style.min_height = 60.0f;
  CMP_TEST_OK(snack.widget.vtable->measure(snack.widget.ctx, width_spec,
                                           height_spec, &size));
  CMP_TEST_ASSERT(size.height >= snack.style.min_height);
  snack.style.min_height = saved_min_height;

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 40.0f;
  bounds.height = 20.0f;
  CMP_TEST_OK(snack.widget.vtable->layout(snack.widget.ctx, bounds));

  ctx.clip = bounds;
  snack.metrics_valid = CMP_TRUE;
  snack.action_bounds.width = bounds.width * 2.0f;
  snack.action_bounds.height = snack.action_metrics.height;
  CMP_TEST_OK(snack.widget.vtable->paint(snack.widget.ctx, &ctx));

  bounds.width = 200.0f;
  bounds.height = 100.0f;
  CMP_TEST_OK(snack.widget.vtable->layout(snack.widget.ctx, bounds));
  ctx.clip = bounds;
  snack.metrics_valid = CMP_TRUE;
  snack.action_bounds.width = snack.action_metrics.width;
  snack.action_bounds.height =
      snack.action_metrics.height + snack.style.action_padding_y * 4.0f;
  CMP_TEST_OK(snack.widget.vtable->paint(snack.widget.ctx, &ctx));

  {
    TestActionState action_state;

    action_state.calls = 0;
    action_state.last_action = 0u;
    action_state.fail_next = 1;
    CMP_TEST_OK(
        m3_snackbar_set_on_action(&snack, test_snackbar_action, &action_state));
    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = (cmp_i32)(snack.action_bounds.x + 1.0f);
    event.data.pointer.y = (cmp_i32)(snack.action_bounds.y + 1.0f);
    CMP_TEST_OK(snack.widget.vtable->event(snack.widget.ctx, &event, &handled));
    event.type = CMP_INPUT_POINTER_UP;
    CMP_TEST_EXPECT(
        snack.widget.vtable->event(snack.widget.ctx, &event, &handled),
        CMP_ERR_IO);
  }

  CMP_TEST_OK(m3_snackbar_set_style(&snack, &snack_style));
  CMP_TEST_OK(snack.widget.vtable->destroy(snack.widget.ctx));

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
  if (test_snackbar_paint_valid() != 0) {
    return 1;
  }
  if (test_dialog_branch_sweep() != 0) {
    return 1;
  }
  if (test_dialog_extra_coverage() != 0) {
    return 1;
  }
  return 0;
}
