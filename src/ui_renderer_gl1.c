/**
 * @file ui_renderer_gl1.c
 * @brief ui_renderer_gl1.c implementation.
 */
/*
 * \file ui_renderer_gl1.c
 * \brief Implementation of the UI Renderer GL1 backend.
 */

/** @brief internal */
#define GL_SILENCE_DEPRECATION
/* clang-format off */
#include <stddef.h>
#include <string.h>
#ifndef __EMSCRIPTEN__
#if defined(_WIN32) || defined(__CYGWIN__)
#include <winsock2.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#elif (defined(__linux__) || defined(__unix__)) && defined(HAVE_OPENGL)
#include <GL/gl.h>
#endif
#endif /* !__EMSCRIPTEN__ */
#include "ui_renderer_gl1.h"
#include "ui_window_backend.h"
#include "ui_font_manager.h"
#include "ui_internal_mem.h"
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_mock_gl1_glyph_box = 0;
int g_mock_gl1_glyph_metrics_fail = 0;

static ui_error_t
mock_ui_font_get_glyph_metrics(struct ui_font *font, int codepoint,
                               float font_size,
                               struct ui_glyph_metrics *out_metrics) {
  if (g_mock_gl1_glyph_metrics_fail) {
    return UI_ERROR_NOT_FOUND;
  }
  if (g_mock_gl1_glyph_box) {
    out_metrics->width = 10;
    out_metrics->height = (codepoint == 'H') ? 0 : 10;
    out_metrics->bearing_x = 0;
    out_metrics->bearing_y = 10;
    out_metrics->advance = 12;
    return UI_ERROR_NONE;
  }
  return ui_font_get_glyph_metrics(font, codepoint, font_size, out_metrics);
}
#undef ui_font_get_glyph_metrics
/** @cond */
#define ui_font_get_glyph_metrics mock_ui_font_get_glyph_metrics
/** @endcond */
#endif

#if !defined(__EMSCRIPTEN__) &&                                                \
    (defined(UI_TEST_MOCK_ALLOC) ||                                            \
     (!defined(_WIN32) && !defined(__APPLE__) && !defined(HAVE_OPENGL)))
static void mock_glViewport(int x, int y, int w, int h) {
  int ux = x, uy = y, uw = w, uh = h;
  x = ux;
  y = uy;
  w = uw;
  h = uh;
}
static void mock_glMatrixMode(int m) {
  int um = m;
  m = um;
}
static void mock_glLoadIdentity(void) {}
static void mock_glOrtho(double l, double r, double b, double t, double n,
                         double f) {
  double ul = l, ur = r, ub = b, ut = t, un = n, uf = f;
  l = ul;
  r = ur;
  b = ub;
  t = ut;
  n = un;
  f = uf;
}
static void mock_glClearColor(float r, float g, float b, float a) {
  float ur = r, ug = g, ub = b, ua = a;
  r = ur;
  g = ug;
  b = ub;
  a = ua;
}
static void mock_glClear(unsigned int m) {
  unsigned int um = m;
  m = um;
}
static void mock_glBegin(int m) {
  int um = m;
  m = um;
}
static void mock_glColor4f(float r, float g, float b, float a) {
  float ur = r, ug = g, ub = b, ua = a;
  r = ur;
  g = ug;
  b = ub;
  a = ua;
}
static void mock_glVertex2f(float x, float y) {
  float ux = x, uy = y;
  x = ux;
  y = uy;
}
static void mock_glEnd(void) {}
static void mock_glReadPixels(int x, int y, int w, int h, unsigned int f,
                              unsigned int t, void *d) {
  int ux = x, uy = y, uw = w, uh = h;
  unsigned int uf = f, ut = t;
  void *ud = d;
  x = ux;
  y = uy;
  w = uw;
  h = uh;
  f = uf;
  t = ut;
  d = ud;
}
#undef glViewport
#define glViewport mock_glViewport
#undef glMatrixMode
#define glMatrixMode mock_glMatrixMode
#undef glLoadIdentity
#define glLoadIdentity mock_glLoadIdentity
#undef glOrtho
#define glOrtho mock_glOrtho
#undef glClearColor
#define glClearColor mock_glClearColor
#undef glClear
#define glClear mock_glClear
#undef glBegin
#define glBegin mock_glBegin
#undef glColor4f
#define glColor4f mock_glColor4f
#undef glVertex2f
#define glVertex2f mock_glVertex2f
#undef glEnd
#define glEnd mock_glEnd
#undef glReadPixels
#define glReadPixels mock_glReadPixels
#endif

#ifdef __EMSCRIPTEN__

/**
 * \brief Creates a legacy OpenGL 1.1 renderer backend. Emscripten stub.
 *
 * \param out_backend Pointer to receive the allocated backend.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_renderer_gl1_create(struct ui_renderer_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNSUPPORTED; /* Or whatever error code */
}

/**
 * \brief Destroys a legacy OpenGL 1.1 renderer backend. Emscripten stub.
 *
 * \param backend The backend to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_renderer_gl1_destroy(struct ui_renderer_backend *backend) {
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_mock_gles2_destroy_fail;
  if (g_mock_gles2_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
#endif
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

#else

/** @def GL_COLOR_BUFFER_BIT
 * @brief OpenGL color buffer bit
 */
#ifndef GL_COLOR_BUFFER_BIT
/* \brief Fallback GL_COLOR_BUFFER_BIT */
#define GL_COLOR_BUFFER_BIT 0x00004000
#endif

#ifndef GL_RGBA
/* \brief Fallback GL_RGBA */
#define GL_RGBA 0x1908
#endif

#ifndef GL_UNSIGNED_BYTE
/* \brief Fallback GL_UNSIGNED_BYTE */
#define GL_UNSIGNED_BYTE 0x1401
#endif

#ifndef GL_PROJECTION
/* \brief Fallback GL_PROJECTION */
#define GL_PROJECTION 0x1701
#endif

#ifndef GL_MODELVIEW
/* \brief Fallback GL_MODELVIEW */
#define GL_MODELVIEW 0x1700
#endif

#ifndef GL_TRIANGLES
/* \brief Fallback GL_TRIANGLES */
#define GL_TRIANGLES 0x0004
#endif

/** @def GL1_MAX_VERTICES
 * @brief Maximum vertices per batch
 */
#define GL1_MAX_VERTICES 8192
/** @def GL1_MAX_INDICES
 * @brief Maximum indices per batch
 */
#define GL1_MAX_INDICES 24576

/**
 * @struct gl1_renderer_data
 * \brief Internal state for the GL1 renderer.
 */
struct gl1_renderer_data {
  struct ui_vertex vertices[GL1_MAX_VERTICES]; /**< Batch vertices */
  unsigned short indices[GL1_MAX_INDICES];     /**< Batch indices */
  int vertex_count;                            /**< Current vertex count */
  int index_count;                             /**< Current index count */
  /* In a real implementation, we would hold VBO, IBO, and Shader IDs here */
};

/* Fallback stubs for missing OpenGL headers on some generic Linux environments
   that might be tested without GLES2 dev packages installed. But standard build
   assumes they are available if linking to them.
   Wait, let's assume glViewport, glClearColor, glClear are present in whatever
   GL header is included. */

/**
 * \brief Flushes the batched geometry to the GPU.
 *
 * \param backend The renderer backend.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_flush.
 * @param backend Parameter backend.
 * @return Return value.
 */
static ui_error_t gl1_flush(struct ui_renderer_backend *backend) {
  struct gl1_renderer_data *data;
  int i;
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_mock_gles2_flush_fail;
  if (g_mock_gles2_flush_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_mock_gles2_flush_fail > 1) {
    g_mock_gles2_flush_fail--;
  }
#endif
  if (!backend || !backend->user_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  data = (struct gl1_renderer_data *)backend->user_data;

  if (data->index_count == 0) {
    return UI_ERROR_NONE; /* Nothing to flush */
  }

#if defined(GL_PROJECTION)
  glBegin(GL_TRIANGLES);
  for (i = 0; i < data->index_count; ++i) {
    unsigned short idx = data->indices[i];
    struct ui_vertex *v = &data->vertices[idx];
    glColor4f(v->color.r, v->color.g, v->color.b, v->color.a);
    glVertex2f(v->x, v->y);
  }
  glEnd();
#endif

  data->vertex_count = 0;
  data->index_count = 0;
  return UI_ERROR_NONE;
}

/**
 * \brief Submits arbitrary triangulated geometry (e.g. SVG paths) to the batch.
 *
 * \param backend The renderer backend.
 * \param vertices Array of vertices.
 * \param vertex_count Number of vertices.
 * \param indices Array of indices defining triangles.
 * \param index_count Number of indices.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_draw_triangles.
 * @param backend Parameter backend.
 * @param vertices Parameter vertices.
 * @param vertex_count Parameter vertex_count.
 * @param indices Parameter indices.
 * @param index_count Parameter index_count.
 * @return Return value.
 */
static ui_error_t gl1_draw_triangles(struct ui_renderer_backend *backend,
                                     const struct ui_vertex *vertices,
                                     int vertex_count,
                                     const unsigned short *indices,
                                     int index_count) {
  struct gl1_renderer_data *data;
  int i;
  int start_vertex;

  if (!backend || !backend->user_data || !vertices || !indices) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  data = (struct gl1_renderer_data *)backend->user_data;

  /* If batch is full, flush it */
  if (data->vertex_count + vertex_count > GL1_MAX_VERTICES ||
      data->index_count + index_count > GL1_MAX_INDICES) {
    {
      ui_error_t rc = gl1_flush(backend);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    /* If a single submission is larger than our batch limit, we must split it
       or fail. For now, return OUT_OF_MEMORY if it strictly exceeds our static
       buffer. */
    if (vertex_count > GL1_MAX_VERTICES || index_count > GL1_MAX_INDICES) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  start_vertex = data->vertex_count;

  /* Copy vertices */
  for (i = 0; i < vertex_count; ++i) {
    data->vertices[data->vertex_count++] = vertices[i];
  }

  /* Copy indices with offset */
  for (i = 0; i < index_count; ++i) {
    data->indices[data->index_count++] =
        (unsigned short)(start_vertex + indices[i]);
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Submits a UI rectangle to the geometry batch.
 *
 * \param backend The renderer backend.
 * \param x X origin.
 * \param y Y origin.
 * \param width Rectangle width.
 * \param height Rectangle height.
 * \param color Fill color.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_draw_rect.
 * @param backend Parameter backend.
 * @param x Parameter x.
 * @param y Parameter y.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param color Parameter color.
 * @return Return value.
 */
static ui_error_t gl1_draw_rect(struct ui_renderer_backend *backend, float x,
                                float y, float width, float height,
                                struct ui_color color) {
  struct ui_vertex vertices[4];
  unsigned short indices[6];

  vertices[0].x = x;
  vertices[0].y = y;
  vertices[0].u = 0.0f;
  vertices[0].v = 0.0f;
  vertices[0].color = color;
  vertices[1].x = x + width;
  vertices[1].y = y;
  vertices[1].u = 1.0f;
  vertices[1].v = 0.0f;
  vertices[1].color = color;
  vertices[2].x = x + width;
  vertices[2].y = y + height;
  vertices[2].u = 1.0f;
  vertices[2].v = 1.0f;
  vertices[2].color = color;
  vertices[3].x = x;
  vertices[3].y = y + height;
  vertices[3].u = 0.0f;
  vertices[3].v = 1.0f;
  vertices[3].color = color;

  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 2;
  indices[3] = 2;
  indices[4] = 3;
  indices[5] = 0;

  return gl1_draw_triangles(backend, vertices, 4, indices, 6);
}

/**
 * \brief Submits a UI border (hollow rectangle) to the geometry batch.
 *
 * \param backend The renderer backend.
 * \param x X origin.
 * \param y Y origin.
 * \param width Rectangle width.
 * \param height Rectangle height.
 * \param thickness Border thickness.
 * \param color Border color.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_draw_border.
 * @param backend Parameter backend.
 * @param x Parameter x.
 * @param y Parameter y.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param thickness Parameter thickness.
 * @param color Parameter color.
 * @return Return value.
 */
static ui_error_t gl1_draw_border(struct ui_renderer_backend *backend, float x,
                                  float y, float width, float height,
                                  float thickness, struct ui_color color) {
  ui_error_t rc = UI_ERROR_NONE;

  rc = gl1_draw_rect(backend, x, y, width, thickness, color);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = gl1_draw_rect(backend, x, y + height - thickness, width, thickness,
                     color);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = gl1_draw_rect(backend, x, y + thickness, thickness,
                     height - (2.0f * thickness), color);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = gl1_draw_rect(backend, x + width - thickness, y + thickness, thickness,
                     height - (2.0f * thickness), color);
  if (rc != UI_ERROR_NONE)
    return rc;

  return rc;
}

/**
 * \brief Initializes the renderer for the given window.
 *
 * \param backend The renderer backend.
 * \param window_backend The window backend.
 * \param window The window instance.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_init.
 * @param backend Parameter backend.
 * @param window_backend Parameter window_backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t gl1_init(struct ui_renderer_backend *backend,
                           struct ui_window_backend *window_backend,
                           struct ui_window *window) {
  struct gl1_renderer_data *data;
  struct ui_window_backend *unused_wb = window_backend;
  struct ui_window *unused_win = window;
  window_backend = unused_wb;
  window = unused_win;

  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  data = (struct gl1_renderer_data *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct gl1_renderer_data));
  if (!data) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  data->vertex_count = 0;
  data->index_count = 0;

  backend->user_data = data;

  /* In a real implementation, we might load GL extensions here, but basic GL
   * functions are already available. */
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys the renderer backend internal state.
 *
 * \param backend The renderer backend.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_destroy.
 * @param backend Parameter backend.
 * @return Return value.
 */
static ui_error_t gl1_destroy(struct ui_renderer_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (backend->user_data) {
    C_MULTIPLATFORM_FREE(backend->user_data);
    backend->user_data = NULL;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the rendering viewport.
 *
 * \param backend The renderer backend.
 * \param x X origin.
 * \param y Y origin.
 * \param width Viewport width.
 * \param height Viewport height.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_set_viewport.
 * @param backend Parameter backend.
 * @param x Parameter x.
 * @param y Parameter y.
 * @param width Parameter width.
 * @param height Parameter height.
 * @return Return value.
 */
static ui_error_t gl1_set_viewport(struct ui_renderer_backend *backend, int x,
                                   int y, int width, int height) {
  struct ui_renderer_backend *unused_b = backend;
  backend = unused_b;

  glViewport(x, y, width, height);

#if defined(GL_PROJECTION)
  /* Setup orthographic projection for UI rendering mapping (0,0) to top-left */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  /* glOrtho(left, right, bottom, top, near, far) */
  /* Wait, usually y goes down in UI, so top=0, bottom=height */
  /* Desktop OpenGL has glOrtho */
  glOrtho(0.0, (double)width, (double)height, 0.0, -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
#endif

  return UI_ERROR_NONE;
}

/**
 * \brief Clears the screen with the specified color.
 *
 * \param backend The renderer backend.
 * \param color The clear color.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_clear.
 * @param backend Parameter backend.
 * @param color Parameter color.
 * @return Return value.
 */
static ui_error_t gl1_clear(struct ui_renderer_backend *backend,
                            struct ui_color color) {
  struct ui_renderer_backend *unused_b = backend;
  backend = unused_b;

  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
  return UI_ERROR_NONE;
}

/**
 * @struct gl1_texture
 * \brief Internal structure representing a GL1 texture.
 */
struct gl1_texture {
  unsigned int fbo_id; /**< FBO ID */
  unsigned int tex_id; /**< Texture ID */
  int width;           /**< Texture width */
  int height;          /**< Texture height */
};

/**
 * \brief Creates an offscreen texture (FBO) for rendering.
 *
 * \param backend The renderer backend.
 * \param width Texture width.
 * \param height Texture height.
 * \param out_texture_handle Pointer to receive the texture handle.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_create_texture.
 * @param backend Parameter backend.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param out_texture_handle Parameter out_texture_handle.
 * @return Return value.
 */
static ui_error_t gl1_create_texture(struct ui_renderer_backend *backend,
                                     int width, int height,
                                     void **out_texture_handle) {
  struct gl1_texture *tex;
  if (!backend || !out_texture_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tex =
      (struct gl1_texture *)C_MULTIPLATFORM_MALLOC(sizeof(struct gl1_texture));
  if (!tex) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  tex->fbo_id = 1; /* Stub */
  tex->tex_id = 1; /* Stub */
  tex->width = width;
  tex->height = height;

  *out_texture_handle = tex;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys an offscreen texture (FBO).
 *
 * \param backend The renderer backend.
 * \param texture_handle The texture handle to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_destroy_texture.
 * @param backend Parameter backend.
 * @param texture_handle Parameter texture_handle.
 * @return Return value.
 */
static ui_error_t gl1_destroy_texture(struct ui_renderer_backend *backend,
                                      void *texture_handle) {
  if (!backend || !texture_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(texture_handle);
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the render target to a specific texture (FBO).
 *
 * \param backend The renderer backend.
 * \param texture_handle The texture handle, or NULL for screen.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_set_render_target.
 * @param backend Parameter backend.
 * @param texture_handle Parameter texture_handle.
 * @return Return value.
 */
static ui_error_t gl1_set_render_target(struct ui_renderer_backend *backend,
                                        void *texture_handle) {
  if (texture_handle) {
  }
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* In a real implementation:
   * if (texture_handle) glBindFramebuffer(GL_FRAMEBUFFER, ((struct
   * gl1_texture*)texture_handle)->fbo_id); else
   * glBindFramebuffer(GL_FRAMEBUFFER, 0);
   */
  return UI_ERROR_NONE;
}

/**
 * \brief Draws a previously rendered offscreen texture.
 *
 * \param backend The renderer backend.
 * \param texture_handle The texture to draw.
 * \param x Dest X origin.
 * \param y Dest Y origin.
 * \param width Dest width.
 * \param height Dest height.
 * \param opacity Alpha opacity modifier.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_draw_texture.
 * @param backend Parameter backend.
 * @param texture_handle Parameter texture_handle.
 * @param x Parameter x.
 * @param y Parameter y.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param opacity Parameter opacity.
 * @return Return value.
 */
static ui_error_t gl1_draw_texture(struct ui_renderer_backend *backend,
                                   void *texture_handle, float x, float y,
                                   float width, float height, float opacity) {
  struct ui_color color;
  if (!backend || !texture_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* In a real implementation, we would flush the current batch, bind this
   * texture, emit a quad, and flush again. */
  /* For the stub, we just push a quad with a mock color, factoring in opacity.
   */
  {
    ui_error_t rc = gl1_flush(backend);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  color.r = 1.0f;
  color.g = 1.0f;
  color.b = 1.0f;
  color.a = opacity;
  return gl1_draw_rect(backend, x, y, width, height, color);
}

static ui_error_t gl1_draw_text(struct ui_renderer_backend *backend,
                                const char *text, const struct ui_font *f,
                                float x, float y, float font_size,
                                struct ui_color color) {
  float cur_x = x;
  const char *p;
  ui_error_t rc;

  if (!backend || !text || !f || font_size <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = gl1_flush(backend);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  for (p = text; *p; ++p) {
    int codepoint = (int)(unsigned char)*p;
    struct ui_glyph_metrics metrics;

    rc = ui_font_get_glyph_metrics((struct ui_font *)f, codepoint, font_size,
                                   &metrics);
    if (rc != UI_ERROR_NONE) {
      continue;
    }

    if (metrics.width > 0 && metrics.height > 0) {
      float gx = cur_x + (float)metrics.bearing_x;
      float gy = y - (float)metrics.bearing_y;
      rc = gl1_draw_rect(backend, gx, gy, (float)metrics.width,
                         (float)metrics.height, color);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
    cur_x += (float)metrics.advance;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Reads pixels from the current render target.
 *
 * \param backend The renderer backend.
 * \param width Width of region to read.
 * \param height Height of region to read.
 * \param out_rgba_buffer Pre-allocated buffer to receive RGBA pixels.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
/**
 * @brief gl1_read_pixels.
 * @param backend Parameter backend.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param out_rgba_buffer Parameter out_rgba_buffer.
 * @return Return value.
 */
static ui_error_t gl1_read_pixels(struct ui_renderer_backend *backend,
                                  int width, int height,
                                  unsigned char *out_rgba_buffer) {
  if (!backend || !out_rgba_buffer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, out_rgba_buffer);
  /* Note: OpenGL reads from bottom-left to top-right. We might need to flip it
   * vertically later in the test harness. */

  return UI_ERROR_NONE;
}

/**
 * \brief Creates a legacy OpenGL 1.1 renderer backend.
 *
 * \param out_backend Pointer to receive the allocated backend.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_renderer_gl1_create(struct ui_renderer_backend **out_backend) {
  struct ui_renderer_backend *backend;

  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  backend = (struct ui_renderer_backend *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_renderer_backend));
  if (!backend) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  backend->init = gl1_init;
  backend->destroy = gl1_destroy;
  backend->set_viewport = gl1_set_viewport;
  backend->clear = gl1_clear;
  backend->draw_rect = gl1_draw_rect;
  backend->draw_border = gl1_draw_border;
  backend->draw_triangles = gl1_draw_triangles;
  backend->flush = gl1_flush;
  backend->create_texture = gl1_create_texture;
  backend->destroy_texture = gl1_destroy_texture;
  backend->set_render_target = gl1_set_render_target;
  backend->draw_texture = gl1_draw_texture;
  backend->draw_text = gl1_draw_text;
  backend->read_pixels = gl1_read_pixels;
  backend->user_data = NULL;

  *out_backend = backend;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a legacy OpenGL 1.1 renderer backend.
 *
 * \param backend The backend to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_renderer_gl1_destroy(struct ui_renderer_backend *backend) {
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_mock_gles2_destroy_fail;
  if (g_mock_gles2_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
#endif
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  gl1_destroy(backend); /* free user_data if present */
  C_MULTIPLATFORM_FREE(backend);
  return UI_ERROR_NONE;
}
#endif /* __EMSCRIPTEN__ */
