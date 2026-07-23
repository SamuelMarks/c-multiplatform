#ifndef __EMSCRIPTEN__
#if defined(_WIN32) || defined(__CYGWIN__)
/* clang-format off */
#include <winsock2.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#elif defined(__linux__) || defined(__unix__)
#include <GL/gl.h>
#endif
#endif /* !__EMSCRIPTEN__ */

#ifdef UI_TEST_MOCK_ALLOC
#undef glViewport
#define glViewport(x, y, w, h) do { (void)(x); (void)(y); (void)(w); (void)(h); } while(0)
#undef glMatrixMode
#define glMatrixMode(m) do { (void)(m); } while(0)
#undef glLoadIdentity
#define glLoadIdentity() do {} while(0)
#undef glOrtho
#define glOrtho(l, r, b, t, n, f) do { (void)(l); (void)(r); (void)(b); (void)(t); (void)(n); (void)(f); } while(0)
#undef glClearColor
#define glClearColor(r, g, b, a) do { (void)(r); (void)(g); (void)(b); (void)(a); } while(0)
#undef glClear
#define glClear(m) do { (void)(m); } while(0)
#undef glBegin
#define glBegin(m) do { (void)(m); } while(0)
#undef glColor4f
#define glColor4f(r, g, b, a) do { (void)(r); (void)(g); (void)(b); (void)(a); } while(0)
#undef glVertex2f
#define glVertex2f(x, y) do { (void)(x); (void)(y); } while(0)
#undef glEnd
#define glEnd() do {} while(0)
#undef glReadPixels
#define glReadPixels(x, y, w, h, f, t, d) do { (void)(x); (void)(y); (void)(w); (void)(h); (void)(f); (void)(t); (void)(d); } while(0)
#endif

#include "../include/ui_renderer_gl1.h"
#include "../include/ui_window_backend.h"
#include "ui_internal_mem.h"
/* clang-format on */

#ifdef __EMSCRIPTEN__

enum ui_error ui_renderer_gl1_create(struct ui_renderer_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNSUPPORTED; /* Or whatever error code */
}

enum ui_error ui_renderer_gl1_destroy(struct ui_renderer_backend *backend) {
  (void)backend;
  return UI_ERROR_NONE;
}

#else

#ifndef GL_COLOR_BUFFER_BIT
#define GL_COLOR_BUFFER_BIT 0x00004000
#endif

#define GL1_MAX_VERTICES 8192
#define GL1_MAX_INDICES 24576

/** \brief gl1_renderer_data */
struct gl1_renderer_data {
  struct ui_vertex vertices[GL1_MAX_VERTICES];
  unsigned short indices[GL1_MAX_INDICES];
  int vertex_count;
  int index_count;
  /* In a real implementation, we would hold VBO, IBO, and Shader IDs here */
};

/* Fallback stubs for missing OpenGL headers on some generic Linux environments
   that might be tested without GLES2 dev packages installed. But standard build
   assumes they are available if linking to them.
   Wait, let's assume glViewport, glClearColor, glClear are present in whatever
   GL header is included. */

static enum ui_error gl1_flush(struct ui_renderer_backend *backend) {
  struct gl1_renderer_data *data;
  int i;
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

static enum ui_error gl1_draw_triangles(struct ui_renderer_backend *backend,
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
    (void)gl1_flush(backend);
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

static enum ui_error gl1_draw_rect(struct ui_renderer_backend *backend, float x,
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

static enum ui_error gl1_draw_border(struct ui_renderer_backend *backend,
                                     float x, float y, float width,
                                     float height, float thickness,
                                     struct ui_color color) {
  enum ui_error rc = UI_ERROR_NONE;

  rc = (enum ui_error)(
      (int)rc | (int)gl1_draw_rect(backend, x, y, width, thickness, color));
  rc = (enum ui_error)((int)rc |
                       (int)gl1_draw_rect(backend, x, y + height - thickness,
                                          width, thickness, color));
  rc = (enum ui_error)((int)rc |
                       (int)gl1_draw_rect(backend, x, y + thickness, thickness,
                                          height - (2.0f * thickness), color));
  rc = (enum ui_error)((int)rc |
                       (int)gl1_draw_rect(backend, x + width - thickness,
                                          y + thickness, thickness,
                                          height - (2.0f * thickness), color));

  return rc;
}

static enum ui_error gl1_init(struct ui_renderer_backend *backend,
                              struct ui_window_backend *window_backend,
                              struct ui_window *window) {
  struct gl1_renderer_data *data;
  (void)window_backend;
  (void)window;

  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  data =
      (struct gl1_renderer_data *)UI_MALLOC(sizeof(struct gl1_renderer_data));
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

static enum ui_error gl1_destroy(struct ui_renderer_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (backend->user_data) {
    UI_FREE(backend->user_data);
    backend->user_data = NULL;
  }
  return UI_ERROR_NONE;
}

static enum ui_error gl1_set_viewport(struct ui_renderer_backend *backend,
                                      int x, int y, int width, int height) {
  (void)backend;
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

static enum ui_error gl1_clear(struct ui_renderer_backend *backend,
                               struct ui_color color) {
  (void)backend;
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
  return UI_ERROR_NONE;
}

/** \brief gl1_texture */
struct gl1_texture {
  unsigned int fbo_id;
  unsigned int tex_id;
  int width;
  int height;
};

static enum ui_error gl1_create_texture(struct ui_renderer_backend *backend,
                                        int width, int height,
                                        void **out_texture_handle) {
  struct gl1_texture *tex;
  if (!backend || !out_texture_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tex = (struct gl1_texture *)UI_MALLOC(sizeof(struct gl1_texture));
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

static enum ui_error gl1_destroy_texture(struct ui_renderer_backend *backend,
                                         void *texture_handle) {
  if (!backend || !texture_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  UI_FREE(texture_handle);
  return UI_ERROR_NONE;
}

static enum ui_error gl1_set_render_target(struct ui_renderer_backend *backend,
                                           void *texture_handle) {
  (void)texture_handle;
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

static enum ui_error gl1_draw_texture(struct ui_renderer_backend *backend,
                                      void *texture_handle, float x, float y,
                                      float width, float height,
                                      float opacity) {
  struct ui_color color;
  if (!backend || !texture_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* In a real implementation, we would flush the current batch, bind this
   * texture, emit a quad, and flush again. */
  /* For the stub, we just push a quad with a mock color, factoring in opacity.
   */
  (void)gl1_flush(backend);

  color.r = 1.0f;
  color.g = 1.0f;
  color.b = 1.0f;
  color.a = opacity;
  return gl1_draw_rect(backend, x, y, width, height, color);
}

static enum ui_error gl1_read_pixels(struct ui_renderer_backend *backend,
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

enum ui_error ui_renderer_gl1_create(struct ui_renderer_backend **out_backend) {
  struct ui_renderer_backend *backend;

  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  backend = (struct ui_renderer_backend *)UI_MALLOC(
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
  backend->read_pixels = gl1_read_pixels;
  backend->user_data = NULL;

  *out_backend = backend;
  return UI_ERROR_NONE;
}

enum ui_error ui_renderer_gl1_destroy(struct ui_renderer_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  gl1_destroy(backend); /* free user_data if present */
  UI_FREE(backend);
  return UI_ERROR_NONE;
}
#endif /* __EMSCRIPTEN__ */
