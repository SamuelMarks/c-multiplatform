#include "m3/m3_render.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAllocator {
  m3_usize alloc_calls;
  m3_usize realloc_calls;
  m3_usize free_calls;
  m3_usize fail_alloc_on;
  m3_usize fail_realloc_on;
  m3_usize fail_free_on;
} TestAllocator;

static int test_alloc(void *ctx, m3_usize size, void **out_ptr) {
  TestAllocator *alloc;

  if (ctx == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on != 0 && alloc->alloc_calls == alloc->fail_alloc_on) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = malloc(size);
  if (*out_ptr == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }
  return M3_OK;
}

static int test_realloc(void *ctx, void *ptr, m3_usize size, void **out_ptr) {
  TestAllocator *alloc;
  void *mem;

  if (ctx == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->realloc_calls += 1;
  if (alloc->fail_realloc_on != 0 &&
      alloc->realloc_calls == alloc->fail_realloc_on) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  mem = realloc(ptr, size);
  if (mem == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }
  *out_ptr = mem;
  return M3_OK;
}

static int test_free(void *ctx, void *ptr) {
  TestAllocator *alloc;

  if (ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_on != 0 && alloc->free_calls == alloc->fail_free_on) {
    return M3_ERR_UNKNOWN;
  }

  free(ptr);
  return M3_OK;
}

static void test_allocator_init(TestAllocator *alloc) {
  alloc->alloc_calls = 0;
  alloc->realloc_calls = 0;
  alloc->free_calls = 0;
  alloc->fail_alloc_on = 0;
  alloc->fail_realloc_on = 0;
  alloc->fail_free_on = 0;
}

typedef struct TestBackend {
  int begin_frame_calls;
  int end_frame_calls;
  int clear_calls;
  int draw_rect_calls;
  int draw_line_calls;
  int push_clip_calls;
  int pop_clip_calls;
  int set_transform_calls;
  int draw_texture_calls;
  int draw_text_calls;
  int fail_clear;
  int fail_draw_line;
  int fail_draw_rect;
  int fail_draw_text;
  M3Handle last_window;
  m3_i32 last_width;
  m3_i32 last_height;
  M3Scalar last_dpi_scale;
  M3Rect last_rect;
  M3Scalar last_corner;
  M3Scalar last_line_thickness;
  M3Rect last_src;
  M3Rect last_dst;
  M3Scalar last_opacity;
  const char *last_text;
  m3_usize last_text_len;
} TestBackend;

typedef struct FakeRecorder {
  M3RenderList *list;
  M3Gfx gfx;
} FakeRecorder;

static void test_backend_init(TestBackend *backend) {
  memset(backend, 0, sizeof(*backend));
  backend->fail_clear = M3_OK;
  backend->fail_draw_line = M3_OK;
  backend->fail_draw_rect = M3_OK;
  backend->fail_draw_text = M3_OK;
}

static int test_gfx_begin_frame(void *gfx, M3Handle window, m3_i32 width,
                                m3_i32 height, M3Scalar dpi_scale) {
  TestBackend *backend;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestBackend *)gfx;
  backend->begin_frame_calls += 1;
  backend->last_window = window;
  backend->last_width = width;
  backend->last_height = height;
  backend->last_dpi_scale = dpi_scale;
  return M3_OK;
}

static int test_gfx_end_frame(void *gfx, M3Handle window) {
  TestBackend *backend;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestBackend *)gfx;
  backend->end_frame_calls += 1;
  backend->last_window = window;
  return M3_OK;
}

static int test_gfx_clear(void *gfx, M3Color color) {
  TestBackend *backend;

  M3_UNUSED(color);

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestBackend *)gfx;
  backend->clear_calls += 1;
  if (backend->fail_clear != M3_OK) {
    return backend->fail_clear;
  }
  return M3_OK;
}

static int test_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color,
                              M3Scalar corner_radius) {
  TestBackend *backend;

  M3_UNUSED(color);

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect != M3_OK) {
    return backend->fail_draw_rect;
  }
  return M3_OK;
}

static int test_gfx_draw_line(void *gfx, M3Scalar x0, M3Scalar y0, M3Scalar x1,
                              M3Scalar y1, M3Color color, M3Scalar thickness) {
  TestBackend *backend;

  M3_UNUSED(x0);
  M3_UNUSED(y0);
  M3_UNUSED(x1);
  M3_UNUSED(y1);
  M3_UNUSED(color);

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestBackend *)gfx;
  backend->draw_line_calls += 1;
  backend->last_line_thickness = thickness;
  if (backend->fail_draw_line != M3_OK) {
    return backend->fail_draw_line;
  }
  return M3_OK;
}

static int test_gfx_push_clip(void *gfx, const M3Rect *rect) {
  TestBackend *backend;

  M3_UNUSED(rect);

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestBackend *)gfx;
  backend->push_clip_calls += 1;
  return M3_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestBackend *backend;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestBackend *)gfx;
  backend->pop_clip_calls += 1;
  return M3_OK;
}

static int test_gfx_set_transform(void *gfx, const M3Mat3 *transform) {
  TestBackend *backend;

  M3_UNUSED(transform);

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestBackend *)gfx;
  backend->set_transform_calls += 1;
  return M3_OK;
}

static int test_gfx_create_texture(void *gfx, m3_i32 width, m3_i32 height,
                                   m3_u32 format, const void *pixels,
                                   m3_usize size, M3Handle *out_texture) {
  M3_UNUSED(gfx);
  M3_UNUSED(width);
  M3_UNUSED(height);
  M3_UNUSED(format);
  M3_UNUSED(pixels);
  M3_UNUSED(size);
  M3_UNUSED(out_texture);
  return M3_OK;
}

static int test_gfx_update_texture(void *gfx, M3Handle texture, m3_i32 x,
                                   m3_i32 y, m3_i32 width, m3_i32 height,
                                   const void *pixels, m3_usize size) {
  M3_UNUSED(gfx);
  M3_UNUSED(texture);
  M3_UNUSED(x);
  M3_UNUSED(y);
  M3_UNUSED(width);
  M3_UNUSED(height);
  M3_UNUSED(pixels);
  M3_UNUSED(size);
  return M3_OK;
}

static int test_gfx_destroy_texture(void *gfx, M3Handle texture) {
  M3_UNUSED(gfx);
  M3_UNUSED(texture);
  return M3_OK;
}

static int test_gfx_draw_texture(void *gfx, M3Handle texture, const M3Rect *src,
                                 const M3Rect *dst, M3Scalar opacity) {
  TestBackend *backend;

  M3_UNUSED(texture);

  if (gfx == NULL || src == NULL || dst == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestBackend *)gfx;
  backend->draw_texture_calls += 1;
  backend->last_src = *src;
  backend->last_dst = *dst;
  backend->last_opacity = opacity;
  return M3_OK;
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 m3_i32 size_px, m3_i32 weight, M3Bool italic,
                                 M3Handle *out_font) {
  M3_UNUSED(text);
  M3_UNUSED(utf8_family);
  M3_UNUSED(size_px);
  M3_UNUSED(weight);
  M3_UNUSED(italic);
  M3_UNUSED(out_font);
  return M3_OK;
}

static int test_text_destroy_font(void *text, M3Handle font) {
  M3_UNUSED(text);
  M3_UNUSED(font);
  return M3_OK;
}

static int test_text_measure_text(void *text, M3Handle font, const char *utf8,
                                  m3_usize utf8_len, M3Scalar *out_width,
                                  M3Scalar *out_height,
                                  M3Scalar *out_baseline) {
  M3_UNUSED(text);
  M3_UNUSED(font);
  M3_UNUSED(utf8);
  M3_UNUSED(utf8_len);
  M3_UNUSED(out_width);
  M3_UNUSED(out_height);
  M3_UNUSED(out_baseline);
  return M3_OK;
}

static int test_text_draw_text(void *text, M3Handle font, const char *utf8,
                               m3_usize utf8_len, M3Scalar x, M3Scalar y,
                               M3Color color) {
  TestBackend *backend;

  M3_UNUSED(font);
  M3_UNUSED(x);
  M3_UNUSED(y);
  M3_UNUSED(color);

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestBackend *)text;
  backend->draw_text_calls += 1;
  backend->last_text = utf8;
  backend->last_text_len = utf8_len;
  if (backend->fail_draw_text != M3_OK) {
    return backend->fail_draw_text;
  }
  return M3_OK;
}

static const M3GfxVTable g_test_gfx_vtable = {
    test_gfx_begin_frame,    test_gfx_end_frame,       test_gfx_clear,
    test_gfx_draw_rect,      test_gfx_draw_line,       test_gfx_push_clip,
    test_gfx_pop_clip,       test_gfx_set_transform,   test_gfx_create_texture,
    test_gfx_update_texture, test_gfx_destroy_texture, test_gfx_draw_texture};

static const M3TextVTable g_test_text_vtable = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    test_text_draw_text};

#define SCENARIO_OK 0
#define SCENARIO_DRAW_RECT_ONLY 1
#define SCENARIO_BAD_BEGIN_FRAME 2
#define SCENARIO_BAD_RECT 3
#define SCENARIO_BAD_RECT_NULL 4
#define SCENARIO_BAD_CORNER 5
#define SCENARIO_BAD_LINE 6
#define SCENARIO_BAD_CLIP 7
#define SCENARIO_BAD_TRANSFORM 8
#define SCENARIO_BAD_TEXTURE_OPACITY 9
#define SCENARIO_BAD_TEXTURE_RECT 10
#define SCENARIO_BAD_TEXT 11
#define SCENARIO_CREATE_TEXTURE_INVALID 12
#define SCENARIO_CREATE_TEXTURE_UNSUPPORTED 13
#define SCENARIO_UPDATE_TEXTURE_INVALID 14
#define SCENARIO_UPDATE_TEXTURE_UNSUPPORTED 15
#define SCENARIO_DESTROY_TEXTURE_INVALID 16
#define SCENARIO_DESTROY_TEXTURE_UNSUPPORTED 17
#define SCENARIO_CREATE_FONT_INVALID 18
#define SCENARIO_CREATE_FONT_UNSUPPORTED 19
#define SCENARIO_DESTROY_FONT_INVALID 20
#define SCENARIO_DESTROY_FONT_UNSUPPORTED 21
#define SCENARIO_MEASURE_TEXT_INVALID 22
#define SCENARIO_MEASURE_TEXT_UNSUPPORTED 23
#define SCENARIO_RECORD_ERRORS 24
#define SCENARIO_NOOP 25

typedef struct PaintScenario {
  int mode;
  int calls;
  M3Rect last_clip;
} PaintScenario;

static int test_widget_paint(void *ctx, M3PaintContext *paint_ctx) {
  PaintScenario *scenario;
  M3Rect rect;
  M3Rect src;
  M3Rect dst;
  M3Mat3 transform;
  M3Handle handle;
  M3Handle out_handle;
  M3Color color;
  M3Scalar out_width;
  M3Scalar out_height;
  M3Scalar out_baseline;
  int rc;

  if (ctx == NULL || paint_ctx == NULL || paint_ctx->gfx == NULL ||
      paint_ctx->gfx->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  scenario = (PaintScenario *)ctx;
  scenario->calls += 1;
  scenario->last_clip = paint_ctx->clip;

  rect.x = 1.0f;
  rect.y = 2.0f;
  rect.width = 3.0f;
  rect.height = 4.0f;
  src.x = 0.0f;
  src.y = 0.0f;
  src.width = 1.0f;
  src.height = 1.0f;
  dst.x = 5.0f;
  dst.y = 6.0f;
  dst.width = 7.0f;
  dst.height = 8.0f;
  handle.id = 1;
  handle.generation = 1;
  out_handle.id = 0;
  out_handle.generation = 0;
  color.r = 0.1f;
  color.g = 0.2f;
  color.b = 0.3f;
  color.a = 1.0f;
  out_width = 0.0f;
  out_height = 0.0f;
  out_baseline = 0.0f;

  switch (scenario->mode) {
  case SCENARIO_OK:
    transform.m[0] = 1.0f;
    transform.m[1] = 0.0f;
    transform.m[2] = 0.0f;
    transform.m[3] = 0.0f;
    transform.m[4] = 1.0f;
    transform.m[5] = 0.0f;
    transform.m[6] = 0.0f;
    transform.m[7] = 0.0f;
    transform.m[8] = 1.0f;

    rc = paint_ctx->gfx->vtable->begin_frame(paint_ctx->gfx->ctx, handle, 100,
                                             200, paint_ctx->dpi_scale);
    if (rc != M3_OK) {
      return rc;
    }
    rc = paint_ctx->gfx->vtable->clear(paint_ctx->gfx->ctx, color);
    if (rc != M3_OK) {
      return rc;
    }
    rc = paint_ctx->gfx->vtable->draw_rect(paint_ctx->gfx->ctx, &rect, color,
                                           2.0f);
    if (rc != M3_OK) {
      return rc;
    }
    rc = paint_ctx->gfx->vtable->draw_line(paint_ctx->gfx->ctx, 0.0f, 1.0f,
                                           2.0f, 3.0f, color, 1.0f);
    if (rc != M3_OK) {
      return rc;
    }
    rc = paint_ctx->gfx->vtable->set_transform(paint_ctx->gfx->ctx, &transform);
    if (rc != M3_OK) {
      return rc;
    }
    rc = paint_ctx->gfx->vtable->draw_texture(paint_ctx->gfx->ctx, handle, &src,
                                              &dst, 1.0f);
    if (rc != M3_OK) {
      return rc;
    }
    if (paint_ctx->gfx->text_vtable == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    rc = paint_ctx->gfx->text_vtable->draw_text(paint_ctx->gfx->ctx, handle,
                                                "Hi", 2, 10.0f, 12.0f, color);
    if (rc != M3_OK) {
      return rc;
    }
    rc = paint_ctx->gfx->vtable->end_frame(paint_ctx->gfx->ctx, handle);
    if (rc != M3_OK) {
      return rc;
    }
    return M3_OK;
  case SCENARIO_DRAW_RECT_ONLY:
    return paint_ctx->gfx->vtable->draw_rect(paint_ctx->gfx->ctx, &rect, color,
                                             0.0f);
  case SCENARIO_BAD_BEGIN_FRAME:
    return paint_ctx->gfx->vtable->begin_frame(paint_ctx->gfx->ctx, handle, 0,
                                               10, 1.0f);
  case SCENARIO_BAD_RECT:
    rect.width = -1.0f;
    return paint_ctx->gfx->vtable->draw_rect(paint_ctx->gfx->ctx, &rect, color,
                                             0.0f);
  case SCENARIO_BAD_RECT_NULL:
    return paint_ctx->gfx->vtable->draw_rect(paint_ctx->gfx->ctx, NULL, color,
                                             0.0f);
  case SCENARIO_BAD_CORNER:
    return paint_ctx->gfx->vtable->draw_rect(paint_ctx->gfx->ctx, &rect, color,
                                             -1.0f);
  case SCENARIO_BAD_LINE:
    return paint_ctx->gfx->vtable->draw_line(paint_ctx->gfx->ctx, 0.0f, 0.0f,
                                             1.0f, 1.0f, color, -1.0f);
  case SCENARIO_BAD_CLIP:
    rect.width = -1.0f;
    return paint_ctx->gfx->vtable->push_clip(paint_ctx->gfx->ctx, &rect);
  case SCENARIO_BAD_TRANSFORM:
    return paint_ctx->gfx->vtable->set_transform(paint_ctx->gfx->ctx, NULL);
  case SCENARIO_BAD_TEXTURE_OPACITY:
    return paint_ctx->gfx->vtable->draw_texture(paint_ctx->gfx->ctx, handle,
                                                &src, &dst, 2.0f);
  case SCENARIO_BAD_TEXTURE_RECT:
    src.width = -1.0f;
    return paint_ctx->gfx->vtable->draw_texture(paint_ctx->gfx->ctx, handle,
                                                &src, &dst, 1.0f);
  case SCENARIO_BAD_TEXT:
    if (paint_ctx->gfx->text_vtable == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    return paint_ctx->gfx->text_vtable->draw_text(paint_ctx->gfx->ctx, handle,
                                                  NULL, 4, 0.0f, 0.0f, color);
  case SCENARIO_CREATE_TEXTURE_INVALID:
    return paint_ctx->gfx->vtable->create_texture(
        paint_ctx->gfx->ctx, 1, 1, M3_TEX_FORMAT_RGBA8, NULL, 0, NULL);
  case SCENARIO_CREATE_TEXTURE_UNSUPPORTED:
    return paint_ctx->gfx->vtable->create_texture(
        paint_ctx->gfx->ctx, 1, 1, M3_TEX_FORMAT_RGBA8, NULL, 0, &out_handle);
  case SCENARIO_UPDATE_TEXTURE_INVALID:
    return paint_ctx->gfx->vtable->update_texture(NULL, handle, 0, 0, 1, 1,
                                                  NULL, 0);
  case SCENARIO_UPDATE_TEXTURE_UNSUPPORTED:
    return paint_ctx->gfx->vtable->update_texture(paint_ctx->gfx->ctx, handle,
                                                  0, 0, 1, 1, NULL, 0);
  case SCENARIO_DESTROY_TEXTURE_INVALID:
    return paint_ctx->gfx->vtable->destroy_texture(NULL, handle);
  case SCENARIO_DESTROY_TEXTURE_UNSUPPORTED:
    return paint_ctx->gfx->vtable->destroy_texture(paint_ctx->gfx->ctx, handle);
  case SCENARIO_CREATE_FONT_INVALID:
    if (paint_ctx->gfx->text_vtable == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    return paint_ctx->gfx->text_vtable->create_font(paint_ctx->gfx->ctx, "Test",
                                                    12, 400, M3_FALSE, NULL);
  case SCENARIO_CREATE_FONT_UNSUPPORTED:
    if (paint_ctx->gfx->text_vtable == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    return paint_ctx->gfx->text_vtable->create_font(
        paint_ctx->gfx->ctx, "Test", 12, 400, M3_FALSE, &out_handle);
  case SCENARIO_DESTROY_FONT_INVALID:
    if (paint_ctx->gfx->text_vtable == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    return paint_ctx->gfx->text_vtable->destroy_font(NULL, handle);
  case SCENARIO_DESTROY_FONT_UNSUPPORTED:
    if (paint_ctx->gfx->text_vtable == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    return paint_ctx->gfx->text_vtable->destroy_font(paint_ctx->gfx->ctx,
                                                     handle);
  case SCENARIO_MEASURE_TEXT_INVALID:
    if (paint_ctx->gfx->text_vtable == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    return paint_ctx->gfx->text_vtable->measure_text(
        paint_ctx->gfx->ctx, handle, "Hi", 2, NULL, &out_height, &out_baseline);
  case SCENARIO_MEASURE_TEXT_UNSUPPORTED:
    if (paint_ctx->gfx->text_vtable == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    return paint_ctx->gfx->text_vtable->measure_text(
        paint_ctx->gfx->ctx, handle, "Hi", 2, &out_width, &out_height,
        &out_baseline);
  case SCENARIO_RECORD_ERRORS: {
    FakeRecorder fake;
    M3Rect bad_dst;
    int ok;

    fake.list = NULL;
    ok = 1;

    if (paint_ctx->gfx->vtable->begin_frame(NULL, handle, 1, 1, 1.0f) !=
        M3_ERR_INVALID_ARGUMENT) {
      ok = 0;
    }
    if (paint_ctx->gfx->vtable->begin_frame(paint_ctx->gfx->ctx, handle, 1, 1,
                                            0.0f) != M3_ERR_RANGE) {
      ok = 0;
    }
    if (paint_ctx->gfx->vtable->begin_frame(&fake, handle, 1, 1, 1.0f) !=
        M3_ERR_STATE) {
      ok = 0;
    }

    if (paint_ctx->gfx->vtable->end_frame(NULL, handle) !=
        M3_ERR_INVALID_ARGUMENT) {
      ok = 0;
    }
    if (paint_ctx->gfx->vtable->end_frame(&fake, handle) != M3_ERR_STATE) {
      ok = 0;
    }

    if (paint_ctx->gfx->vtable->clear(NULL, color) != M3_ERR_INVALID_ARGUMENT) {
      ok = 0;
    }
    if (paint_ctx->gfx->vtable->clear(&fake, color) != M3_ERR_STATE) {
      ok = 0;
    }

    if (paint_ctx->gfx->vtable->draw_rect(NULL, &rect, color, 1.0f) !=
        M3_ERR_INVALID_ARGUMENT) {
      ok = 0;
    }
    if (paint_ctx->gfx->vtable->draw_rect(&fake, &rect, color, 1.0f) !=
        M3_ERR_STATE) {
      ok = 0;
    }

    if (paint_ctx->gfx->vtable->draw_line(NULL, 0.0f, 0.0f, 1.0f, 1.0f, color,
                                          1.0f) != M3_ERR_INVALID_ARGUMENT) {
      ok = 0;
    }
    if (paint_ctx->gfx->vtable->draw_line(&fake, 0.0f, 0.0f, 1.0f, 1.0f, color,
                                          1.0f) != M3_ERR_STATE) {
      ok = 0;
    }

    if (paint_ctx->gfx->vtable->push_clip(NULL, &rect) !=
        M3_ERR_INVALID_ARGUMENT) {
      ok = 0;
    }
    if (paint_ctx->gfx->vtable->push_clip(&fake, &rect) != M3_ERR_STATE) {
      ok = 0;
    }

    if (paint_ctx->gfx->vtable->pop_clip(NULL) != M3_ERR_INVALID_ARGUMENT) {
      ok = 0;
    }
    if (paint_ctx->gfx->vtable->pop_clip(&fake) != M3_ERR_STATE) {
      ok = 0;
    }

    if (paint_ctx->gfx->vtable->set_transform(NULL, &transform) !=
        M3_ERR_INVALID_ARGUMENT) {
      ok = 0;
    }
    if (paint_ctx->gfx->vtable->set_transform(paint_ctx->gfx->ctx, NULL) !=
        M3_ERR_INVALID_ARGUMENT) {
      ok = 0;
    }
    if (paint_ctx->gfx->vtable->set_transform(&fake, &transform) !=
        M3_ERR_STATE) {
      ok = 0;
    }

    if (paint_ctx->gfx->vtable->draw_texture(NULL, handle, &src, &dst, 1.0f) !=
        M3_ERR_INVALID_ARGUMENT) {
      ok = 0;
    }
    if (paint_ctx->gfx->vtable->draw_texture(&fake, handle, &src, &dst, 1.0f) !=
        M3_ERR_STATE) {
      ok = 0;
    }
    bad_dst = dst;
    bad_dst.width = -1.0f;
    if (paint_ctx->gfx->vtable->draw_texture(paint_ctx->gfx->ctx, handle, &src,
                                             &bad_dst, 1.0f) != M3_ERR_RANGE) {
      ok = 0;
    }

    if (paint_ctx->gfx->text_vtable != NULL) {
      if (paint_ctx->gfx->text_vtable->draw_text(NULL, handle, "Hi", 2, 0.0f,
                                                 0.0f, color) !=
          M3_ERR_INVALID_ARGUMENT) {
        ok = 0;
      }
      if (paint_ctx->gfx->text_vtable->draw_text(&fake, handle, "Hi", 2, 0.0f,
                                                 0.0f, color) != M3_ERR_STATE) {
        ok = 0;
      }
    }

    if (!ok) {
      return M3_ERR_UNKNOWN;
    }
    return M3_OK;
  }
  case SCENARIO_NOOP:
    return M3_OK;
  default:
    return M3_ERR_UNKNOWN;
  }
}

static int run_paint_scenario(int mode, int expected_rc) {
  M3RenderList list;
  M3RenderNode node;
  M3Widget widget;
  M3WidgetVTable vtable;
  PaintScenario scenario;
  M3Rect bounds;
  int rc;

  memset(&list, 0, sizeof(list));
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 10.0f;
  bounds.height = 10.0f;

  scenario.mode = mode;
  scenario.calls = 0;
  scenario.last_clip = bounds;

  memset(&vtable, 0, sizeof(vtable));
  vtable.paint = test_widget_paint;

  widget.ctx = &scenario;
  widget.vtable = &vtable;
  widget.flags = 0;
  widget.handle.id = 1;
  widget.handle.generation = 1;

  M3_TEST_OK(m3_render_list_init(&list, NULL, 4));
  M3_TEST_OK(m3_render_node_init(&node, &widget, &bounds));
  rc = m3_render_build(&node, &list, 1.0f);
  if (rc != expected_rc) {
    fprintf(stderr, "TEST FAIL scenario %d: expected %d got %d\n", mode,
            expected_rc, rc);
    m3_render_list_shutdown(&list);
    return 1;
  }
  if (scenario.calls != 1) {
    fprintf(stderr, "TEST FAIL scenario %d: expected paint call\n", mode);
    m3_render_list_shutdown(&list);
    return 1;
  }
  M3_TEST_OK(m3_render_list_shutdown(&list));
  return 0;
}

int main(void) {
  {
    M3RenderList list;
    M3Allocator bad_alloc;
    TestAllocator alloc;
    M3Allocator alloc_iface;
    M3RenderCmd cmd;
    M3Color color;
    m3_usize max_size;
    m3_usize cmd_size;

    memset(&list, 0, sizeof(list));

    M3_TEST_EXPECT(m3_render_list_init(NULL, NULL, 0), M3_ERR_INVALID_ARGUMENT);
#ifdef M3_TESTING
    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
    M3_TEST_EXPECT(m3_render_list_init(&list, NULL, 1), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));
#endif
    M3_TEST_EXPECT(m3_render_test_add_overflow(1, 1, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_render_test_mul_overflow(1, 1, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_render_test_list_reserve(NULL, 1),
                   M3_ERR_INVALID_ARGUMENT);

    list.commands = (M3RenderCmd *)1;
    list.capacity = 1;
    list.count = 0;
    M3_TEST_EXPECT(m3_render_list_init(&list, NULL, 0), M3_ERR_STATE);

    memset(&list, 0, sizeof(list));
    bad_alloc.ctx = NULL;
    bad_alloc.alloc = NULL;
    bad_alloc.realloc = NULL;
    bad_alloc.free = NULL;
    M3_TEST_EXPECT(m3_render_list_init(&list, &bad_alloc, 0),
                   M3_ERR_INVALID_ARGUMENT);

    test_allocator_init(&alloc);
    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;

    alloc.fail_alloc_on = 1;
    M3_TEST_EXPECT(m3_render_list_init(&list, &alloc_iface, 4),
                   M3_ERR_OUT_OF_MEMORY);
    alloc.fail_alloc_on = 0;

    memset(&list, 0, sizeof(list));
    max_size = (m3_usize) ~(m3_usize)0;
    M3_TEST_EXPECT(m3_render_list_init(&list, &alloc_iface, max_size),
                   M3_ERR_OVERFLOW);

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, &alloc_iface, 0));
    M3_TEST_ASSERT(list.capacity >= 1);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, &alloc_iface, 1));
    color.r = 0.0f;
    color.g = 0.0f;
    color.b = 0.0f;
    color.a = 1.0f;
    cmd.type = M3_RENDER_CMD_CLEAR;
    cmd.data.clear.color = color;
    list.count = 2;
    list.capacity = 1;
    M3_TEST_EXPECT(m3_render_list_append(&list, &cmd), M3_ERR_STATE);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, &alloc_iface, 1));

    color.r = 0.0f;
    color.g = 0.0f;
    color.b = 0.0f;
    color.a = 1.0f;
    cmd.type = M3_RENDER_CMD_CLEAR;
    cmd.data.clear.color = color;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    M3_TEST_ASSERT(list.count == 2);
    M3_TEST_ASSERT(list.capacity >= 2);

    M3_TEST_OK(m3_render_list_reset(&list));
    M3_TEST_ASSERT(list.count == 0);

    M3_TEST_OK(m3_render_list_shutdown(&list));
    M3_TEST_EXPECT(m3_render_list_shutdown(&list), M3_ERR_STATE);

    test_allocator_init(&alloc);
    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;
    memset(&list, 0, sizeof(list));
    list.allocator = alloc_iface;
    M3_TEST_OK(m3_render_test_list_reserve(&list, 1));
    M3_TEST_ASSERT(list.capacity >= 1);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, &alloc_iface, 1));
    list.count = 1;
    M3_TEST_OK(m3_render_test_list_reserve(&list, 4));
    M3_TEST_ASSERT(list.capacity >= 5);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    max_size = (m3_usize) ~(m3_usize)0;
    cmd_size = (m3_usize)sizeof(M3RenderCmd);
    memset(&list, 0, sizeof(list));
    list.allocator = alloc_iface;
    list.capacity = max_size / (cmd_size * 2) + 1;
    list.count = list.capacity;
    M3_TEST_EXPECT(m3_render_test_list_reserve(&list, 1), M3_ERR_OVERFLOW);

#ifdef M3_TESTING
    M3_TEST_OK(m3_render_test_set_force_reserve(M3_TRUE));
    memset(&list, 0, sizeof(list));
    list.allocator = alloc_iface;
    list.capacity = max_size / 2 + 1;
    list.count = list.capacity;
    M3_TEST_EXPECT(m3_render_test_list_reserve(&list, 0), M3_ERR_OVERFLOW);
    M3_TEST_OK(m3_render_test_set_force_reserve(M3_FALSE));
#endif

    M3_TEST_EXPECT(m3_render_list_reset(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_render_list_reset(&list), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_render_list_shutdown(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_render_list_append(NULL, &cmd), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_render_list_append(&list, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_render_list_append(&list, &cmd), M3_ERR_STATE);

    memset(&list, 0, sizeof(list));
    test_allocator_init(&alloc);
    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;
    M3_TEST_OK(m3_render_list_init(&list, &alloc_iface, 1));
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    alloc.fail_realloc_on = 1;
    M3_TEST_EXPECT(m3_render_list_append(&list, &cmd), M3_ERR_OUT_OF_MEMORY);
    M3_TEST_ASSERT(list.count == 1);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    test_allocator_init(&alloc);
    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;
    M3_TEST_OK(m3_render_list_init(&list, &alloc_iface, 1));
    alloc.fail_free_on = 1;
    M3_TEST_EXPECT(m3_render_list_shutdown(&list), M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_render_list_shutdown(&list), M3_ERR_STATE);

    memset(&list, 0, sizeof(list));
    list.commands = (M3RenderCmd *)1;
    list.capacity = max_size / 2 + 1;
    list.count = list.capacity;
    M3_TEST_EXPECT(m3_render_list_append(&list, &cmd), M3_ERR_OVERFLOW);
    list.capacity = max_size;
    list.count = max_size;
    M3_TEST_EXPECT(m3_render_list_append(&list, &cmd), M3_ERR_OVERFLOW);
  }

  {
    M3RenderList list;
    M3RenderCmd cmd;
    TestBackend backend;
    M3Gfx gfx;
    M3GfxVTable gfx_vtable;
    M3TextVTable text_vtable;
    M3Handle handle;
    M3Color color;
    M3Rect rect;
    M3Rect src;
    M3Rect dst;
    M3Mat3 transform;
    const char *text;

    memset(&list, 0, sizeof(list));
    test_backend_init(&backend);
    gfx_vtable = g_test_gfx_vtable;
    text_vtable = g_test_text_vtable;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 8));

    handle.id = 7;
    handle.generation = 2;
    color.r = 0.2f;
    color.g = 0.3f;
    color.b = 0.4f;
    color.a = 1.0f;
    rect.x = 1.0f;
    rect.y = 2.0f;
    rect.width = 3.0f;
    rect.height = 4.0f;
    src.x = 0.0f;
    src.y = 0.0f;
    src.width = 5.0f;
    src.height = 6.0f;
    dst.x = 7.0f;
    dst.y = 8.0f;
    dst.width = 9.0f;
    dst.height = 10.0f;
    transform.m[0] = 1.0f;
    transform.m[1] = 0.0f;
    transform.m[2] = 0.0f;
    transform.m[3] = 0.0f;
    transform.m[4] = 1.0f;
    transform.m[5] = 0.0f;
    transform.m[6] = 0.0f;
    transform.m[7] = 0.0f;
    transform.m[8] = 1.0f;
    text = "Hello";

    cmd.type = M3_RENDER_CMD_BEGIN_FRAME;
    cmd.data.begin_frame.window = handle;
    cmd.data.begin_frame.width = 320;
    cmd.data.begin_frame.height = 240;
    cmd.data.begin_frame.dpi_scale = 2.0f;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));

    cmd.type = M3_RENDER_CMD_CLEAR;
    cmd.data.clear.color = color;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));

    cmd.type = M3_RENDER_CMD_DRAW_RECT;
    cmd.data.draw_rect.rect = rect;
    cmd.data.draw_rect.color = color;
    cmd.data.draw_rect.corner_radius = 1.5f;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));

    cmd.type = M3_RENDER_CMD_DRAW_LINE;
    cmd.data.draw_line.x0 = 0.0f;
    cmd.data.draw_line.y0 = 1.0f;
    cmd.data.draw_line.x1 = 2.0f;
    cmd.data.draw_line.y1 = 3.0f;
    cmd.data.draw_line.color = color;
    cmd.data.draw_line.thickness = 2.5f;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));

    cmd.type = M3_RENDER_CMD_PUSH_CLIP;
    cmd.data.push_clip.rect = rect;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));

    cmd.type = M3_RENDER_CMD_POP_CLIP;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));

    cmd.type = M3_RENDER_CMD_SET_TRANSFORM;
    cmd.data.set_transform.transform = transform;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));

    cmd.type = M3_RENDER_CMD_DRAW_TEXTURE;
    cmd.data.draw_texture.texture = handle;
    cmd.data.draw_texture.src = src;
    cmd.data.draw_texture.dst = dst;
    cmd.data.draw_texture.opacity = 0.8f;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));

    cmd.type = M3_RENDER_CMD_DRAW_TEXT;
    cmd.data.draw_text.font = handle;
    cmd.data.draw_text.utf8 = text;
    cmd.data.draw_text.utf8_len = 5;
    cmd.data.draw_text.x = 12.0f;
    cmd.data.draw_text.y = 13.0f;
    cmd.data.draw_text.color = color;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));

    cmd.type = M3_RENDER_CMD_END_FRAME;
    cmd.data.end_frame.window = handle;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));

    test_backend_init(&backend);
    gfx_vtable = g_test_gfx_vtable;
    text_vtable = g_test_text_vtable;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;

    M3_TEST_OK(m3_render_list_execute(&list, &gfx));
    M3_TEST_ASSERT(backend.begin_frame_calls == 1);
    M3_TEST_ASSERT(backend.end_frame_calls == 1);
    M3_TEST_ASSERT(backend.clear_calls == 1);
    M3_TEST_ASSERT(backend.draw_rect_calls == 1);
    M3_TEST_ASSERT(backend.draw_line_calls == 1);
    M3_TEST_ASSERT(backend.push_clip_calls == 1);
    M3_TEST_ASSERT(backend.pop_clip_calls == 1);
    M3_TEST_ASSERT(backend.set_transform_calls == 1);
    M3_TEST_ASSERT(backend.draw_texture_calls == 1);
    M3_TEST_ASSERT(backend.draw_text_calls == 1);
    M3_TEST_ASSERT(backend.last_width == 320);
    M3_TEST_ASSERT(backend.last_height == 240);
    M3_TEST_ASSERT(backend.last_rect.width == rect.width);
    M3_TEST_ASSERT(backend.last_corner == 1.5f);
    M3_TEST_ASSERT(backend.last_text == text);
    M3_TEST_ASSERT(backend.last_text_len == 5);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 1));
    cmd.type = M3_RENDER_CMD_CLEAR;
    cmd.data.clear.color = color;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    test_backend_init(&backend);
    backend.fail_clear = M3_ERR_UNKNOWN;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 1));
    cmd.type = M3_RENDER_CMD_DRAW_RECT;
    cmd.data.draw_rect.rect = rect;
    cmd.data.draw_rect.color = color;
    cmd.data.draw_rect.corner_radius = 0.0f;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    gfx_vtable = g_test_gfx_vtable;
    gfx_vtable.draw_rect = NULL;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx), M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 1));
    cmd.type = M3_RENDER_CMD_BEGIN_FRAME;
    cmd.data.begin_frame.window = handle;
    cmd.data.begin_frame.width = 1;
    cmd.data.begin_frame.height = 1;
    cmd.data.begin_frame.dpi_scale = 1.0f;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    gfx_vtable = g_test_gfx_vtable;
    gfx_vtable.begin_frame = NULL;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx), M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 1));
    cmd.type = M3_RENDER_CMD_END_FRAME;
    cmd.data.end_frame.window = handle;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    gfx_vtable = g_test_gfx_vtable;
    gfx_vtable.end_frame = NULL;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx), M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 1));
    cmd.type = M3_RENDER_CMD_CLEAR;
    cmd.data.clear.color = color;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    gfx_vtable = g_test_gfx_vtable;
    gfx_vtable.clear = NULL;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx), M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 1));
    cmd.type = M3_RENDER_CMD_DRAW_LINE;
    cmd.data.draw_line.x0 = 0.0f;
    cmd.data.draw_line.y0 = 1.0f;
    cmd.data.draw_line.x1 = 2.0f;
    cmd.data.draw_line.y1 = 3.0f;
    cmd.data.draw_line.color = color;
    cmd.data.draw_line.thickness = 1.0f;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    gfx_vtable = g_test_gfx_vtable;
    gfx_vtable.draw_line = NULL;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx), M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 1));
    cmd.type = M3_RENDER_CMD_PUSH_CLIP;
    cmd.data.push_clip.rect = rect;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    gfx_vtable = g_test_gfx_vtable;
    gfx_vtable.push_clip = NULL;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx), M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 1));
    cmd.type = M3_RENDER_CMD_POP_CLIP;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    gfx_vtable = g_test_gfx_vtable;
    gfx_vtable.pop_clip = NULL;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx), M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 1));
    cmd.type = M3_RENDER_CMD_SET_TRANSFORM;
    cmd.data.set_transform.transform = transform;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    gfx_vtable = g_test_gfx_vtable;
    gfx_vtable.set_transform = NULL;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx), M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 1));
    cmd.type = M3_RENDER_CMD_DRAW_TEXTURE;
    cmd.data.draw_texture.texture = handle;
    cmd.data.draw_texture.src = src;
    cmd.data.draw_texture.dst = dst;
    cmd.data.draw_texture.opacity = 1.0f;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    gfx_vtable = g_test_gfx_vtable;
    gfx_vtable.draw_texture = NULL;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx), M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 1));
    cmd.type = M3_RENDER_CMD_DRAW_TEXT;
    cmd.data.draw_text.font = handle;
    cmd.data.draw_text.utf8 = text;
    cmd.data.draw_text.utf8_len = 5;
    cmd.data.draw_text.x = 1.0f;
    cmd.data.draw_text.y = 2.0f;
    cmd.data.draw_text.color = color;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    gfx_vtable = g_test_gfx_vtable;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = NULL;
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx), M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 1));
    cmd.type = 999;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    gfx_vtable = g_test_gfx_vtable;
    gfx.ctx = &backend;
    gfx.vtable = &gfx_vtable;
    gfx.text_vtable = &text_vtable;
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    memset(&list, 0, sizeof(list));
    M3_TEST_EXPECT(m3_render_list_execute(&list, &gfx), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_render_list_execute(NULL, &gfx), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_render_list_execute(&list, NULL),
                   M3_ERR_INVALID_ARGUMENT);
  }

  {
    M3RenderNode node;
    M3RenderNode child_node;
    M3RenderNode *children[1];
    M3Widget widget;
    M3Widget child_widget;
    M3WidgetVTable vtable;
    PaintScenario scenario;
    PaintScenario child_scenario;
    M3Rect bounds;
    M3Rect child_bounds;
    M3RenderList list;
    M3RenderCmd cmd;
    M3Color color;
    TestAllocator alloc;
    M3Allocator alloc_iface;

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.width = 10.0f;
    bounds.height = 10.0f;

    M3_TEST_EXPECT(m3_render_node_init(NULL, &widget, &bounds),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_render_node_init(&node, NULL, &bounds),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_render_node_init(&node, &widget, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    bounds.width = -1.0f;
    M3_TEST_EXPECT(m3_render_node_init(&node, &widget, &bounds), M3_ERR_RANGE);
    bounds.width = 10.0f;

    M3_TEST_EXPECT(m3_render_node_set_bounds(NULL, &bounds),
                   M3_ERR_INVALID_ARGUMENT);
    bounds.height = -1.0f;
    M3_TEST_EXPECT(m3_render_node_set_bounds(&node, &bounds), M3_ERR_RANGE);
    bounds.height = 10.0f;

    M3_TEST_EXPECT(m3_render_node_set_children(NULL, children, 1),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_render_node_set_children(&node, NULL, 1),
                   M3_ERR_INVALID_ARGUMENT);
    children[0] = NULL;
    M3_TEST_EXPECT(m3_render_node_set_children(&node, children, 1),
                   M3_ERR_INVALID_ARGUMENT);

    memset(&vtable, 0, sizeof(vtable));
    vtable.paint = test_widget_paint;

    scenario.mode = SCENARIO_DRAW_RECT_ONLY;
    scenario.calls = 0;
    scenario.last_clip = bounds;
    widget.ctx = &scenario;
    widget.vtable = &vtable;
    widget.flags = 0;
    widget.handle.id = 1;
    widget.handle.generation = 1;

    M3_TEST_OK(m3_render_node_init(&node, &widget, &bounds));

    M3_TEST_EXPECT(m3_render_test_validate_node(NULL), M3_ERR_INVALID_ARGUMENT);
    bounds.width = -1.0f;
    node.bounds = bounds;
    M3_TEST_EXPECT(m3_render_test_validate_node(&node), M3_ERR_RANGE);
    bounds.width = 10.0f;
    node.bounds = bounds;
    node.child_count = 1;
    node.children = NULL;
    M3_TEST_EXPECT(m3_render_test_validate_node(&node),
                   M3_ERR_INVALID_ARGUMENT);
    children[0] = NULL;
    node.children = children;
    M3_TEST_EXPECT(m3_render_test_validate_node(&node),
                   M3_ERR_INVALID_ARGUMENT);
    node.child_count = 0;
    node.children = NULL;

    memset(&list, 0, sizeof(list));
    M3_TEST_EXPECT(m3_render_build(NULL, &list, 1.0f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_render_build(&node, NULL, 1.0f), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_render_build(&node, &list, 0.0f), M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_render_build(&node, &list, -1.0f), M3_ERR_RANGE);
    M3_TEST_EXPECT(m3_render_build(&node, &list, 1.0f), M3_ERR_STATE);

    M3_TEST_OK(m3_render_list_init(&list, NULL, 8));
    scenario.mode = SCENARIO_OK;
    scenario.calls = 0;
    child_scenario.mode = SCENARIO_DRAW_RECT_ONLY;
    child_scenario.calls = 0;
    child_scenario.last_clip = bounds;
    child_widget.ctx = &child_scenario;
    child_widget.vtable = &vtable;
    child_widget.flags = 0;
    child_widget.handle.id = 2;
    child_widget.handle.generation = 1;
    child_bounds.x = 1.0f;
    child_bounds.y = 1.0f;
    child_bounds.width = 2.0f;
    child_bounds.height = 2.0f;
    M3_TEST_OK(m3_render_node_init(&child_node, &child_widget, &child_bounds));
    children[0] = &child_node;
    M3_TEST_OK(m3_render_node_set_children(&node, children, 1));
    M3_TEST_OK(m3_render_build(&node, &list, 1.0f));
    M3_TEST_ASSERT(scenario.calls == 1);
    M3_TEST_ASSERT(child_scenario.calls == 1);
    M3_TEST_ASSERT(list.count == 13);
    M3_TEST_ASSERT(list.commands[0].type == M3_RENDER_CMD_PUSH_CLIP);
    M3_TEST_ASSERT(list.commands[1].type == M3_RENDER_CMD_BEGIN_FRAME);
    M3_TEST_ASSERT(list.commands[list.count - 1].type ==
                   M3_RENDER_CMD_POP_CLIP);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    test_allocator_init(&alloc);
    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;
    alloc.fail_realloc_on = 1;
    M3_TEST_OK(m3_render_list_init(&list, &alloc_iface, 1));
    list.count = list.capacity;
    scenario.mode = SCENARIO_NOOP;
    scenario.calls = 0;
    M3_TEST_EXPECT(m3_render_build(&node, &list, 1.0f), M3_ERR_OUT_OF_MEMORY);
    alloc.fail_realloc_on = 0;
    M3_TEST_OK(m3_render_list_shutdown(&list));

    test_allocator_init(&alloc);
    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;
    alloc.fail_realloc_on = 1;
    M3_TEST_OK(m3_render_list_init(&list, &alloc_iface, 1));
    node.child_count = 0;
    node.children = NULL;
    scenario.mode = SCENARIO_NOOP;
    scenario.calls = 0;
    M3_TEST_EXPECT(m3_render_build(&node, &list, 1.0f), M3_ERR_OUT_OF_MEMORY);
    alloc.fail_realloc_on = 0;
    M3_TEST_OK(m3_render_list_shutdown(&list));

    node.child_count = 1;
    node.children = children;

    scenario.mode = SCENARIO_NOOP;
    child_bounds.width = -1.0f;
    child_node.bounds = child_bounds;
    M3_TEST_OK(m3_render_list_init(&list, NULL, 4));
    M3_TEST_EXPECT(m3_render_build(&node, &list, 1.0f), M3_ERR_RANGE);
    M3_TEST_OK(m3_render_list_shutdown(&list));
    child_bounds.width = 2.0f;
    child_node.bounds = child_bounds;

#ifdef M3_TESTING
    M3_TEST_OK(m3_render_test_set_force_intersect_fail(M3_TRUE));
    M3_TEST_OK(m3_render_list_init(&list, NULL, 4));
    M3_TEST_EXPECT(m3_render_build(&node, &list, 1.0f), M3_ERR_RANGE);
    M3_TEST_OK(m3_render_list_shutdown(&list));
    M3_TEST_OK(m3_render_test_set_force_intersect_fail(M3_FALSE));
#endif

    M3_TEST_OK(m3_render_list_init(&list, NULL, 2));
    widget.flags = M3_WIDGET_FLAG_HIDDEN;
    scenario.calls = 0;
    M3_TEST_OK(m3_render_node_set_children(&node, NULL, 0));
    M3_TEST_OK(m3_render_build(&node, &list, 1.0f));
    M3_TEST_ASSERT(scenario.calls == 0);
    M3_TEST_ASSERT(list.count == 0);
    M3_TEST_OK(m3_render_list_shutdown(&list));
    widget.flags = 0;

    bounds.width = 0.0f;
    bounds.height = 10.0f;
    M3_TEST_OK(m3_render_node_set_bounds(&node, &bounds));
    scenario.mode = SCENARIO_DRAW_RECT_ONLY;
    scenario.calls = 0;
    M3_TEST_OK(m3_render_list_init(&list, NULL, 2));
    M3_TEST_OK(m3_render_build(&node, &list, 1.0f));
    M3_TEST_ASSERT(scenario.calls == 0);
    M3_TEST_ASSERT(list.count == 0);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    bounds.width = 10.0f;
    bounds.height = 10.0f;
    M3_TEST_OK(m3_render_node_set_bounds(&node, &bounds));
    child_bounds.x = 20.0f;
    child_bounds.y = 20.0f;
    child_bounds.width = 1.0f;
    child_bounds.height = 1.0f;
    M3_TEST_OK(m3_render_node_set_bounds(&child_node, &child_bounds));
    scenario.mode = SCENARIO_DRAW_RECT_ONLY;
    scenario.calls = 0;
    child_scenario.mode = SCENARIO_DRAW_RECT_ONLY;
    child_scenario.calls = 0;
    M3_TEST_OK(m3_render_list_init(&list, NULL, 4));
    M3_TEST_OK(m3_render_node_set_children(&node, children, 1));
    M3_TEST_OK(m3_render_build(&node, &list, 1.0f));
    M3_TEST_ASSERT(scenario.calls == 1);
    M3_TEST_ASSERT(child_scenario.calls == 0);
    M3_TEST_ASSERT(list.count == 3);
    M3_TEST_OK(m3_render_list_shutdown(&list));

    M3_TEST_OK(m3_render_list_init(&list, NULL, 2));
    widget.vtable = NULL;
    M3_TEST_EXPECT(m3_render_build(&node, &list, 1.0f),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_render_list_shutdown(&list));
    widget.vtable = &vtable;

    M3_TEST_OK(m3_render_list_init(&list, NULL, 4));
    color.r = 0.0f;
    color.g = 0.0f;
    color.b = 0.0f;
    color.a = 1.0f;
    cmd.type = M3_RENDER_CMD_CLEAR;
    cmd.data.clear.color = color;
    M3_TEST_OK(m3_render_list_append(&list, &cmd));
    scenario.mode = SCENARIO_BAD_RECT;
    scenario.calls = 0;
    M3_TEST_EXPECT(m3_render_build(&node, &list, 1.0f), M3_ERR_RANGE);
    M3_TEST_ASSERT(list.count == 1);
    M3_TEST_OK(m3_render_list_shutdown(&list));
  }

  if (run_paint_scenario(SCENARIO_BAD_BEGIN_FRAME, M3_ERR_RANGE)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_BAD_RECT, M3_ERR_RANGE)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_BAD_RECT_NULL, M3_ERR_INVALID_ARGUMENT)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_BAD_CORNER, M3_ERR_RANGE)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_BAD_LINE, M3_ERR_RANGE)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_BAD_CLIP, M3_ERR_RANGE)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_BAD_TRANSFORM, M3_ERR_INVALID_ARGUMENT)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_BAD_TEXTURE_OPACITY, M3_ERR_RANGE)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_BAD_TEXTURE_RECT, M3_ERR_RANGE)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_BAD_TEXT, M3_ERR_INVALID_ARGUMENT)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_CREATE_TEXTURE_INVALID,
                         M3_ERR_INVALID_ARGUMENT)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_CREATE_TEXTURE_UNSUPPORTED,
                         M3_ERR_UNSUPPORTED)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_UPDATE_TEXTURE_INVALID,
                         M3_ERR_INVALID_ARGUMENT)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_UPDATE_TEXTURE_UNSUPPORTED,
                         M3_ERR_UNSUPPORTED)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_DESTROY_TEXTURE_INVALID,
                         M3_ERR_INVALID_ARGUMENT)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_DESTROY_TEXTURE_UNSUPPORTED,
                         M3_ERR_UNSUPPORTED)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_CREATE_FONT_INVALID,
                         M3_ERR_INVALID_ARGUMENT)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_CREATE_FONT_UNSUPPORTED,
                         M3_ERR_UNSUPPORTED)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_DESTROY_FONT_INVALID,
                         M3_ERR_INVALID_ARGUMENT)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_DESTROY_FONT_UNSUPPORTED,
                         M3_ERR_UNSUPPORTED)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_MEASURE_TEXT_INVALID,
                         M3_ERR_INVALID_ARGUMENT)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_MEASURE_TEXT_UNSUPPORTED,
                         M3_ERR_UNSUPPORTED)) {
    return 1;
  }
  if (run_paint_scenario(SCENARIO_RECORD_ERRORS, M3_OK)) {
    return 1;
  }

  return 0;
}
