/**
 * @file ui_renderer_gles_fallback.c
 * @brief ui_renderer_gles_fallback.c implementation.
 */
/* clang-format off */
#include "../include/ui_renderer.h"
#include "../include/ui_error.h"

/* GLES fallback wrapper for the new vtable interface. */

/* We will reuse the old `ui_renderer_backend` implementation as the context for now. */
#include "../include/ui_renderer_gles2.h"
#include "../include/ui_renderer_gl1.h"
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @struct gles_fallback_context
 * \brief gles_fallback_context
 */
struct gles_fallback_context {
  struct ui_renderer_backend *backend; /**< backend */
  int current_width;                   /**< current_width */
  int current_height;                  /**< current_height */
};

#ifdef UI_TEST_MOCK_ALLOC
extern int g_mock_gles2_create_fail;
#endif

/**
 * @brief gles_fallback_begin_frame.
 * @param ctx Parameter ctx.
 * @param width Parameter width.
 * @param height Parameter height.
 * @return Return value.
 */
static ui_error_t gles_fallback_begin_frame(void *ctx, int width, int height) {
  struct gles_fallback_context *gctx = (struct gles_fallback_context *)ctx;
  struct ui_color clear_color = {0.0f, 0.0f, 0.0f, 0.0f};
  if (!gctx || !gctx->backend)
    return UI_ERROR_INVALID_ARGUMENT;

  gctx->current_width = width;
  gctx->current_height = height;

  if (gctx->backend->set_viewport) {
    {
      ui_error_t sv_rc =
          gctx->backend->set_viewport(gctx->backend, 0, 0, width, height);
      if (sv_rc != UI_ERROR_NONE)
        return sv_rc;
    }
  }
  if (gctx->backend->clear) {
    {
      ui_error_t c_rc = gctx->backend->clear(gctx->backend, clear_color);
      if (c_rc != UI_ERROR_NONE)
        return c_rc;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief gles_fallback_end_frame.
 * @param ctx Parameter ctx.
 * @return Return value.
 */
static ui_error_t gles_fallback_end_frame(void *ctx) {
  struct gles_fallback_context *gctx = (struct gles_fallback_context *)ctx;
  if (gctx && gctx->backend && gctx->backend->flush) {
    {
      ui_error_t fl_rc = gctx->backend->flush(gctx->backend);
      if (fl_rc != UI_ERROR_NONE)
        return fl_rc;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief gles_fallback_draw_rect.
 * @param ctx Parameter ctx.
 * @param r Parameter r.
 * @param c Parameter c.
 * @return Return value.
 */
static ui_error_t gles_fallback_draw_rect(void *ctx, const struct ui_rect *r,
                                          const struct ui_color *c) {
  struct gles_fallback_context *gctx = (struct gles_fallback_context *)ctx;
  if (!gctx || !gctx->backend || !gctx->backend->draw_rect || !r || !c)
    return UI_ERROR_INVALID_ARGUMENT;
  return (ui_error_t)gctx->backend->draw_rect(gctx->backend, r->x, r->y,
                                              r->width, r->height, *c);
}

/**
 * @brief gles_fallback_draw_text.
 * @param ctx Parameter ctx.
 * @param text Parameter text.
 * @param f Parameter f.
 * @param r Parameter r.
 * @return Return value.
 */
static ui_error_t gles_fallback_draw_text(void *ctx, const char *text,
                                          const struct ui_font *f,
                                          const struct ui_rect *r) {
  (void)ctx;
  (void)text;
  (void)f;
  (void)r;
  /* Stub for now until stb_truetype is wired into GLES */
  return UI_ERROR_NONE;
}

/**
 * @brief gles_fallback_draw_image.
 * @param ctx Parameter ctx.
 * @param img Parameter img.
 * @param r Parameter r.
 * @return Return value.
 */
static ui_error_t gles_fallback_draw_image(void *ctx,
                                           const struct ui_image *img,
                                           const struct ui_rect *r) {
  (void)ctx;
  (void)img;
  (void)r;
  /* Stub for now */
  return UI_ERROR_NONE;
}

/**
 * @brief gles_fallback_draw_gradient.
 * @param ctx Parameter ctx.
 * @param r Parameter r.
 * @param gradient Parameter gradient.
 * @return Return value.
 */
static ui_error_t
gles_fallback_draw_gradient(void *ctx, const struct ui_rect *r,
                            const struct ui_css_image *gradient) {
  (void)ctx;
  (void)r;
  (void)gradient;
  /* Stub for now */
  return UI_ERROR_NONE;
}

/**
 * @brief gles_fallback_draw_path.
 * @param ctx Parameter ctx.
 * @param p Parameter p.
 * @param c Parameter c.
 * @return Return value.
 */
static ui_error_t gles_fallback_draw_path(void *ctx, const struct ui_path *p,
                                          const struct ui_color *c) {
  (void)ctx;
  (void)p;
  (void)c;
  /* Stub for now */
  return UI_ERROR_NONE;
}

/**
 * @brief gles_fallback_push_clip.
 * @param ctx Parameter ctx.
 * @param r Parameter r.
 * @return Return value.
 */
static ui_error_t gles_fallback_push_clip(void *ctx, const struct ui_rect *r) {
  (void)ctx;
  (void)r;
  /* Stub for now */
  return UI_ERROR_NONE;
}

/**
 * @brief gles_fallback_pop_clip.
 * @param ctx Parameter ctx.
 * @return Return value.
 */
static ui_error_t gles_fallback_pop_clip(void *ctx) {
  (void)ctx;
  /* Stub for now */
  return UI_ERROR_NONE;
}

/**
 * @brief gles_fallback_set_blend_mode.
 * @param ctx Parameter ctx.
 * @param mode Parameter mode.
 * @return Return value.
 */
static ui_error_t gles_fallback_set_blend_mode(void *ctx,
                                               enum ui_css_blend_mode mode) {
  (void)ctx;
  (void)mode;
  /* Stub for now */
  return UI_ERROR_NONE;
}

/**
 * @brief gles_fallback_set_shadow.
 * @param ctx Parameter ctx.
 * @param shadow Parameter shadow.
 * @return Return value.
 */
static ui_error_t gles_fallback_set_shadow(void *ctx,
                                           const struct ui_css_shadow *shadow) {
  (void)ctx;
  (void)shadow;
  /* Stub for now */
  return UI_ERROR_NONE;
}

/**
 * @brief gles_fallback_read_pixels.
 * @param ctx Parameter ctx.
 * @param out_rgba_buffer Parameter out_rgba_buffer.
 * @return Return value.
 */
static ui_error_t gles_fallback_read_pixels(void *ctx,
                                            unsigned char *out_rgba_buffer) {
  struct gles_fallback_context *gctx = (struct gles_fallback_context *)ctx;
  if (!gctx || !gctx->backend || !gctx->backend->read_pixels ||
      !out_rgba_buffer)
    return UI_ERROR_INVALID_ARGUMENT;

  return (ui_error_t)gctx->backend->read_pixels(
      gctx->backend, gctx->current_width, gctx->current_height,
      out_rgba_buffer);
}

/**
 * @brief gles_fallback_destroy.
 * @param ctx Parameter ctx.
 * @return Return value.
 */
static ui_error_t gles_fallback_destroy(void *ctx) {
  struct gles_fallback_context *gctx = (struct gles_fallback_context *)ctx;
  if (gctx) {
    if (gctx->backend) {
      /* We can safely call the backend's own destroy function pointer since it
       * was initialized properly */
      {
        ui_error_t d_rc = gctx->backend->destroy(gctx->backend);
        if (d_rc != UI_ERROR_NONE)
          return d_rc;
      }
      C_MULTIPLATFORM_FREE(gctx->backend);
    }
    C_MULTIPLATFORM_FREE(gctx);
  }
  return UI_ERROR_NONE;
}

/** @brief Fallback renderer vtable */
static const struct ui_renderer_vtable gles_fallback_vtable = {
    gles_fallback_begin_frame, gles_fallback_end_frame,
    gles_fallback_draw_rect,   gles_fallback_draw_text,
    gles_fallback_draw_image,  gles_fallback_draw_gradient,
    gles_fallback_draw_path,   gles_fallback_push_clip,
    gles_fallback_pop_clip,    gles_fallback_set_blend_mode,
    gles_fallback_set_shadow,  gles_fallback_read_pixels,
    gles_fallback_destroy};

/**
 * @brief ui_renderer_gles_fallback_init.
 * @param renderer Parameter renderer.
 * @return Return value.
 */
ui_error_t ui_renderer_gles_fallback_init(struct ui_renderer *renderer) {
  struct ui_renderer_backend *backend = NULL;
  struct gles_fallback_context *gctx = NULL;
  ui_error_t err;

  if (!renderer)
    return UI_ERROR_INVALID_ARGUMENT;

#ifdef UI_TEST_MOCK_ALLOC
  if (g_mock_gles2_create_fail) {
    err = UI_ERROR_UNSUPPORTED;
  } else {
#endif
    err = ui_renderer_gles2_create(&backend);
#ifdef UI_TEST_MOCK_ALLOC
  }
#endif
  if (err == UI_ERROR_NONE) {
    err = backend->init(backend, NULL, NULL);
    if (err != UI_ERROR_NONE) {
      ui_renderer_gles2_destroy(backend);
      backend = NULL;
    }
  }

  if (!backend) {
    /* Fallback to GL 1.1 if GLES2 is unavailable */
    err = ui_renderer_gl1_create(&backend);
    if (err == UI_ERROR_NONE) {
      err = backend->init(backend, NULL, NULL);
      if (err != UI_ERROR_NONE) {
        ui_renderer_gl1_destroy(backend);
        return (ui_error_t)err;
      }
    } else {
      return (ui_error_t)err;
    }
  }

  gctx = (struct gles_fallback_context *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct gles_fallback_context));
  if (!gctx) {
    {
      ui_error_t d_rc = backend->destroy(backend);
      if (d_rc != UI_ERROR_NONE)
        return d_rc;
    }
    return UI_ERROR_OUT_OF_MEMORY;
  }

  gctx->backend = backend;
  gctx->current_width = 0;
  gctx->current_height = 0;

  renderer->vtable = &gles_fallback_vtable;
  renderer->ctx = gctx;

  return UI_ERROR_NONE;
}
