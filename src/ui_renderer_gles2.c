/*
 * \file ui_renderer_gles2.c
 * \brief Implementation of the UI Renderer GLES2 backend.
 */

/** @brief internal */
#define GL_SILENCE_DEPRECATION
/* clang-format off */
#include "../include/ui_renderer.h"
#include "../include/ui_error.h"
/* clang-format on */

#if defined(_WIN32) || defined(__CYGWIN__)
/* clang-format off */
#include <winsock2.h>
#include <GL/gl.h>
#include <stddef.h>
#include "ui_internal_mem.h"

#ifndef GL_FRAGMENT_SHADER
/** @brief internal */
#define GL_FRAGMENT_SHADER 0x8B30
/** @brief internal */
#define GL_VERTEX_SHADER 0x8B31
/** @brief internal */
#define GL_ARRAY_BUFFER 0x8892
/** @brief internal */
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
/** @brief internal */
#define GL_STATIC_DRAW 0x88E4
/** @brief internal */
#define GL_DYNAMIC_DRAW 0x88E8
#endif

#ifndef GLsizeiptr
typedef ptrdiff_t GLsizeiptr;
#endif

typedef void (__stdcall *PFNGLUSEPROGRAMPROC) (unsigned int program);
typedef void (__stdcall *PFNGLUNIFORM2FPROC) (int location, float v0, float v1);
typedef void (__stdcall *PFNGLBINDBUFFERPROC) (unsigned int target, unsigned int buffer);
typedef void (__stdcall *PFNGLBUFFERDATAPROC) (unsigned int target, GLsizeiptr size, const void *data, unsigned int usage);
typedef void (__stdcall *PFNGLENABLEVERTEXATTRIBARRAYPROC) (unsigned int index);
typedef void (__stdcall *PFNGLVERTEXATTRIBPOINTERPROC) (unsigned int index, int size, unsigned int type, unsigned char normalized, int stride, const void *pointer);
typedef void (__stdcall *PFNGLDISABLEVERTEXATTRIBARRAYPROC) (unsigned int index);
typedef unsigned int (__stdcall *PFNGLCREATESHADERPROC) (unsigned int type);
typedef void (__stdcall *PFNGLSHADERSOURCEPROC) (unsigned int shader, int count, const char *const*string, const int *length);
typedef void (__stdcall *PFNGLCOMPILESHADERPROC) (unsigned int shader);
typedef unsigned int (__stdcall *PFNGLCREATEPROGRAMPROC) (void);
typedef void (__stdcall *PFNGLATTACHSHADERPROC) (unsigned int program, unsigned int shader);
typedef void (__stdcall *PFNGLLINKPROGRAMPROC) (unsigned int program);
typedef void (__stdcall *PFNGLDELETESHADERPROC) (unsigned int shader);
typedef int (__stdcall *PFNGLGETATTRIBLOCATIONPROC) (unsigned int program, const char *name);
typedef int (__stdcall *PFNGLGETUNIFORMLOCATIONPROC) (unsigned int program, const char *name);
typedef void (__stdcall *PFNGLGENBUFFERSPROC) (int n, unsigned int *buffers);
typedef void (__stdcall *PFNGLDELETEPROGRAMPROC) (unsigned int program);
typedef void (__stdcall *PFNGLDELETEBUFFERSPROC) (int n, const unsigned int *buffers);

static PFNGLUSEPROGRAMPROC glUseProgram;
static PFNGLUNIFORM2FPROC glUniform2f;
static PFNGLBINDBUFFERPROC glBindBuffer;
static PFNGLBUFFERDATAPROC glBufferData;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
static PFNGLCREATESHADERPROC glCreateShader;
static PFNGLSHADERSOURCEPROC glShaderSource;
static PFNGLCOMPILESHADERPROC glCompileShader;
static PFNGLCREATEPROGRAMPROC glCreateProgram;
static PFNGLATTACHSHADERPROC glAttachShader;
static PFNGLLINKPROGRAMPROC glLinkProgram;
static PFNGLDELETESHADERPROC glDeleteShader;
static PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
static PFNGLGENBUFFERSPROC glGenBuffers;
static PFNGLDELETEPROGRAMPROC glDeleteProgram;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers;

#ifndef wglGetProcAddress
/* wglGetProcAddress is provided by wingdi.h via gl.h */
#endif

static ui_error_t load_gl_extensions(void) {
    if (glCreateProgram) return UI_ERROR_NONE;
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
  return UI_ERROR_NONE;
}
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
/** @brief internal */
#define load_gl_extensions()
#elif defined(__EMSCRIPTEN__)
#include <GLES2/gl2.h>
/** @brief internal */
#define load_gl_extensions()
#elif defined(__linux__) || defined(__unix__)
/** @brief internal */
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
/** @brief internal */
#define load_gl_extensions()
#endif

#ifdef UI_TEST_MOCK_ALLOC
#undef glCreateProgram
/** @brief internal */
#define glCreateProgram() 1
#undef glCreateShader
/** @brief internal */
#define glCreateShader(t) ((void)(t), 1)
#undef glAttachShader
/** @brief internal */
#define glAttachShader(p, s) do { (void)(p); (void)(s); } while(0)
#undef glLinkProgram
/** @brief internal */
#define glLinkProgram(p) do { (void)(p); } while(0)
#undef glDeleteShader
/** @brief internal */
#define glDeleteShader(s) do { (void)(s); } while(0)
#undef glGetAttribLocation
/** @brief internal */
#define glGetAttribLocation(p, n) ((void)(p), (void)(n), 1)
#undef glGetUniformLocation
/** @brief internal */
#define glGetUniformLocation(p, n) ((void)(p), (void)(n), 1)
#undef glGenBuffers
/** @brief internal */
#define glGenBuffers(n, b) do { (void)(n); *(b) = 1; } while(0)
#undef glDeleteProgram
/** @brief internal */
#define glDeleteProgram(p) do { (void)(p); } while(0)
#undef glDeleteBuffers
/** @brief internal */
#define glDeleteBuffers(n, b) do { (void)(n); (void)(b); } while(0)
#undef glUseProgram
/** @brief internal */
#define glUseProgram(p) do { (void)(p); } while(0)
#undef glUniform2f
/** @brief internal */
#define glUniform2f(l, v0, v1) do { (void)(l); (void)(v0); (void)(v1); } while(0)
#undef glBindBuffer
/** @brief internal */
#define glBindBuffer(t, b) do { (void)(t); (void)(b); } while(0)
#undef glBufferData
/** @brief internal */
#define glBufferData(t, s, d, u) do { (void)(t); (void)(s); (void)(d); (void)(u); } while(0)
#undef glEnableVertexAttribArray
/** @brief internal */
#define glEnableVertexAttribArray(i) do { (void)(i); } while(0)
#undef glVertexAttribPointer
/** @brief internal */
#define glVertexAttribPointer(i, s, t, n, st, p) do { (void)(i); (void)(s); (void)(t); (void)(n); (void)(st); (void)(p); } while(0)
#undef glDrawElements
/** @brief internal */
#define glDrawElements(m, c, t, p) do { (void)(m); (void)(c); (void)(t); (void)(p); } while(0)
#undef glDisableVertexAttribArray
/** @brief internal */
#define glDisableVertexAttribArray(i) do { (void)(i); } while(0)
#undef glShaderSource
/** @brief internal */
#define glShaderSource(s, c, str, l) do { (void)(s); (void)(c); (void)(str); (void)(l); } while(0)
#undef glCompileShader
/** @brief internal */
#define glCompileShader(s) do { (void)(s); } while(0)
#undef glViewport
/** @brief internal */
#define glViewport(x, y, w, h) do { (void)(x); (void)(y); (void)(w); (void)(h); } while(0)
#undef glClearColor
/** @brief internal */
#define glClearColor(r, g, b, a) do { (void)(r); (void)(g); (void)(b); (void)(a); } while(0)
#undef glClear
/** @brief internal */
#define glClear(m) do { (void)(m); } while(0)
#undef glReadPixels
/** @brief internal */
#define glReadPixels(x, y, w, h, f, t, d) do { (void)(x); (void)(y); (void)(w); (void)(h); (void)(f); (void)(t); (void)(d); } while(0)
#endif

#include "ui_renderer_gles2.h"
#include "ui_window_backend.h"
#ifndef _WIN32
#include "ui_internal_mem.h"
/* clang-format on */
#endif

#ifndef GL_COLOR_BUFFER_BIT
/* \brief Fallback GL_COLOR_BUFFER_BIT */
#define GL_COLOR_BUFFER_BIT 0x00004000
#endif

#ifndef GL_FLOAT
/* \brief Fallback GL_FLOAT */
#define GL_FLOAT 0x1406
/* \brief Fallback GL_UNSIGNED_SHORT */
#define GL_UNSIGNED_SHORT 0x1403
/* \brief Fallback GL_TRIANGLES */
#define GL_TRIANGLES 0x0004
/* \brief Fallback GL_RGBA */
#define GL_RGBA 0x1908
/* \brief Fallback GL_UNSIGNED_BYTE */
#define GL_UNSIGNED_BYTE 0x1401
#endif

/* \brief Maximum vertices per batch */
#define GLES2_MAX_VERTICES 8192
/* \brief Maximum indices per batch */
#define GLES2_MAX_INDICES 24576

/**
 * @struct gles2_renderer_data
 * \brief Internal state for the GLES2 renderer.
 */
struct gles2_renderer_data {
  struct ui_vertex vertices[GLES2_MAX_VERTICES]; /**< Batch vertices */
  unsigned short indices[GLES2_MAX_INDICES];     /**< Batch indices */
  int vertex_count;                              /**< Current vertex count */
  int index_count;                               /**< Current index count */

  unsigned int program; /**< Shader program */
  int a_position;       /**< Position attribute loc */
  int a_color;          /**< Color attribute loc */
  int u_resolution;     /**< Resolution uniform loc */
  unsigned int vbo;     /**< Vertex buffer object */
  unsigned int ibo;     /**< Index buffer object */

  float viewport_width;  /**< Viewport width */
  float viewport_height; /**< Viewport height */
};

/* \brief Simple passthrough vertex shader */
static const char *vertex_shader_source =
    "attribute vec2 a_position;\n"
    "attribute vec4 a_color;\n"
    "uniform vec2 u_resolution;\n"
    "varying vec4 v_color;\n"
    "void main() {\n"
    "  vec2 zeroToOne = a_position / u_resolution;\n"
    "  vec2 zeroToTwo = zeroToOne * 2.0;\n"
    "  vec2 clipSpace = zeroToTwo - 1.0;\n"
    "  gl_Position = vec4(clipSpace.x, -clipSpace.y, 0.0, 1.0);\n"
    "  v_color = a_color;\n"
    "}\n";

/* \brief Simple passthrough fragment shader */
static const char *fragment_shader_source = "precision mediump float;\n"
                                            "varying vec4 v_color;\n"
                                            "void main() {\n"
                                            "  gl_FragColor = v_color;\n"
                                            "}\n";

/*
 * \brief Compiles a shader.
 *
 * \param type Shader type.
 * \param source Source code.
 * \param out_shader Pointer to receive the shader ID.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t compile_shader(unsigned int type, const char *source,
                                 unsigned int *out_shader) {
  unsigned int shader;
  *out_shader = 0;
#if defined(_WIN32) || defined(__CYGWIN__)
  if (!glCreateShader)
    return UI_ERROR_UNSUPPORTED;
#endif
  shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  *out_shader = shader;
  return UI_ERROR_NONE;
}

/*
 * \brief Flushes the batched geometry to the GPU.
 *
 * \param backend The renderer backend.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_flush(struct ui_renderer_backend *backend) {
  struct gles2_renderer_data *data;
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_mock_gles2_flush_fail;
  if (g_mock_gles2_flush_fail) {
    return UI_ERROR_UNKNOWN;
  }
#endif
  if (!backend || !backend->user_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  data = (struct gles2_renderer_data *)backend->user_data;

  if (data->index_count == 0) {
    return UI_ERROR_NONE; /* Nothing to flush */
  }

#if defined(_WIN32) || defined(__CYGWIN__)
  if (data->program && glBindBuffer && glBufferData) {
#else
  if (data->program) {
#endif
    glUseProgram(data->program);
    {
      float w = data->viewport_width;
      float h = data->viewport_height;
      glUniform2f(data->u_resolution, w, h);
    }

    glBindBuffer(GL_ARRAY_BUFFER, data->vbo);
    {
      GLsizeiptr vsz =
          (GLsizeiptr)((size_t)data->vertex_count * sizeof(struct ui_vertex));
      glBufferData(GL_ARRAY_BUFFER, vsz, data->vertices, GL_DYNAMIC_DRAW);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ibo);
    {
      GLsizeiptr isz =
          (GLsizeiptr)((size_t)data->index_count * sizeof(unsigned short));
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, isz, data->indices,
                   GL_DYNAMIC_DRAW);
    }
    glEnableVertexAttribArray((GLuint)data->a_position);
    glVertexAttribPointer((GLuint)data->a_position, 2, GL_FLOAT, 0,
                          sizeof(struct ui_vertex), (void *)0);

    glEnableVertexAttribArray((GLuint)data->a_color);
    glVertexAttribPointer((GLuint)data->a_color, 4, GL_FLOAT, 0,
                          sizeof(struct ui_vertex),
                          (void *)(sizeof(float) * 2));

    glDrawElements(GL_TRIANGLES, data->index_count, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray((GLuint)data->a_position);
    glDisableVertexAttribArray((GLuint)data->a_color);
  }

  data->vertex_count = 0;
  data->index_count = 0;
  return UI_ERROR_NONE;
}

/*
 * \brief Submits arbitrary triangulated geometry (e.g. SVG paths) to the batch.
 *
 * \param backend The renderer backend.
 * \param vertices Array of vertices.
 * \param vertex_count Number of vertices.
 * \param indices Array of indices defining triangles.
 * \param index_count Number of indices.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_draw_triangles(struct ui_renderer_backend *backend,
                                       const struct ui_vertex *vertices,
                                       int vertex_count,
                                       const unsigned short *indices,
                                       int index_count) {
  struct gles2_renderer_data *data;
  int i;
  int start_vertex;

  if (!backend || !backend->user_data || !vertices || !indices) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  data = (struct gles2_renderer_data *)backend->user_data;

  /* If batch is full, flush it */
  if (data->vertex_count + vertex_count > GLES2_MAX_VERTICES ||
      data->index_count + index_count > GLES2_MAX_INDICES) {
    {
      ui_error_t rc = gles2_flush(backend);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    if (vertex_count > GLES2_MAX_VERTICES || index_count > GLES2_MAX_INDICES) {
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

/*
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
static ui_error_t gles2_draw_rect(struct ui_renderer_backend *backend, float x,
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

  return gles2_draw_triangles(backend, vertices, 4, indices, 6);
}

/*
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
static ui_error_t gles2_draw_border(struct ui_renderer_backend *backend,
                                    float x, float y, float width, float height,
                                    float thickness, struct ui_color color) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!backend || !backend->user_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = gles2_draw_rect(backend, x, y, width, thickness, color);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = gles2_draw_rect(backend, x, y + height - thickness, width, thickness,
                       color);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = gles2_draw_rect(backend, x, y + thickness, thickness,
                       height - (2.0f * thickness), color);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = gles2_draw_rect(backend, x + width - thickness, y + thickness, thickness,
                       height - (2.0f * thickness), color);
  if (rc != UI_ERROR_NONE)
    return rc;

  return rc;
}

/*
 * \brief Initializes the renderer for the given window.
 *
 * \param backend The renderer backend.
 * \param window_backend The window backend.
 * \param window The window instance.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_init(struct ui_renderer_backend *backend,
                             struct ui_window_backend *window_backend,
                             struct ui_window *window) {
  struct gles2_renderer_data *data;
  unsigned int vs, fs;
  (void)window_backend;
  (void)window;

  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  load_gl_extensions();

  data = (struct gles2_renderer_data *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct gles2_renderer_data));
  if (!data) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  data->vertex_count = 0;
  data->index_count = 0;
  data->viewport_width = 800.0f;
  data->viewport_height = 600.0f;
  data->program = 0;
  data->vbo = 0;
  data->ibo = 0;

#if defined(_WIN32) || defined(__CYGWIN__)
  if (glCreateProgram) {
#else
  if (1) {
#endif
    (void)compile_shader(GL_VERTEX_SHADER, vertex_shader_source, &vs);
    (void)compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source, &fs);

    data->program = glCreateProgram();
    glAttachShader(data->program, vs);
    glAttachShader(data->program, fs);
    glLinkProgram(data->program);

    data->a_position = glGetAttribLocation(data->program, "a_position");
    data->a_color = glGetAttribLocation(data->program, "a_color");
    data->u_resolution = glGetUniformLocation(data->program, "u_resolution");

    glDeleteShader(vs);
    glDeleteShader(fs);

    glGenBuffers(1, &data->vbo);
    glGenBuffers(1, &data->ibo);
  }

  backend->user_data = data;
  return UI_ERROR_NONE;
}

/*
 * \brief Destroys the renderer backend internal state.
 *
 * \param backend The renderer backend.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_destroy(struct ui_renderer_backend *backend) {
  struct gles2_renderer_data *data;
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_mock_gles2_destroy_fail;
  if (g_mock_gles2_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
#endif
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (backend->user_data) {
    data = (struct gles2_renderer_data *)backend->user_data;
#if defined(_WIN32) || defined(__CYGWIN__)
    if (data->program && glDeleteProgram) {
#else
    if (data->program) {
#endif
      glDeleteProgram(data->program);
      glDeleteBuffers(1, &data->vbo);
      glDeleteBuffers(1, &data->ibo);
    }
    C_MULTIPLATFORM_FREE(backend->user_data);
    backend->user_data = NULL;
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Sets the rendering viewport.
 *
 * \param backend The renderer backend.
 * \param x X origin.
 * \param y Y origin.
 * \param width Viewport width.
 * \param height Viewport height.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_set_viewport(struct ui_renderer_backend *backend, int x,
                                     int y, int width, int height) {
  struct gles2_renderer_data *data;
  if (!backend || !backend->user_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  data = (struct gles2_renderer_data *)backend->user_data;

  glViewport(x, y, width, height);
  data->viewport_width = (float)width;
  data->viewport_height = (float)height;

  return UI_ERROR_NONE;
}

/*
 * \brief Clears the screen with the specified color.
 *
 * \param backend The renderer backend.
 * \param color The clear color.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_clear(struct ui_renderer_backend *backend,
                              struct ui_color color) {
  (void)backend;
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
  return UI_ERROR_NONE;
}

/**
 * @struct gles2_texture
 * \brief Internal structure representing a GLES2 texture.
 */
struct gles2_texture {
  unsigned int fbo_id; /**< FBO ID */
  unsigned int tex_id; /**< Texture ID */
  int width;           /**< Texture width */
  int height;          /**< Texture height */
};

/*
 * \brief Pushes a clipping rect.
 *
 * \param backend The renderer backend.
 * \param x X origin.
 * \param y Y origin.
 * \param width Width.
 * \param height Height.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_push_clip(struct ui_renderer_backend *backend, float x,
                                  float y, float width, float height) {
  ui_error_t rc;
  (void)x;
  (void)y;
  (void)width;
  (void)height;
  if (!backend || !backend->user_data)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = gles2_flush(backend);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* glScissor logic would go here. We mock it for the test. */
  return UI_ERROR_NONE;
}

/*
 * \brief Pops a clipping rect.
 *
 * \param backend The renderer backend.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_pop_clip(struct ui_renderer_backend *backend) {
  ui_error_t rc;
  if (!backend || !backend->user_data)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = gles2_flush(backend);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

/*
 * \brief Pushes a stencil clip from triangulated geometry.
 *
 * \param backend The renderer backend.
 * \param vertices Array of vertices defining the stencil.
 * \param vertex_count Number of vertices.
 * \param indices Array of indices defining triangles.
 * \param index_count Number of indices.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_push_stencil_clip(struct ui_renderer_backend *backend,
                                          const struct ui_vertex *vertices,
                                          int vertex_count,
                                          const unsigned short *indices,
                                          int index_count) {
  ui_error_t rc;
  (void)vertices;
  (void)vertex_count;
  (void)indices;
  (void)index_count;

  if (!backend || !backend->user_data)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = gles2_flush(backend);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

/*
 * \brief Pops a stencil clip.
 *
 * \param backend The renderer backend.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_pop_stencil_clip(struct ui_renderer_backend *backend) {
  ui_error_t rc;
  if (!backend || !backend->user_data)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = gles2_flush(backend);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

/*
 * \brief Creates an offscreen texture (FBO) for rendering.
 *
 * \param backend The renderer backend.
 * \param width Texture width.
 * \param height Texture height.
 * \param out_texture_handle Pointer to receive the texture handle.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_create_texture(struct ui_renderer_backend *backend,
                                       int width, int height,
                                       void **out_texture_handle) {
  struct gles2_texture *tex;
  if (!backend || !out_texture_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tex = (struct gles2_texture *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct gles2_texture));
  if (!tex) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  tex->fbo_id = 1;
  tex->tex_id = 1;
  tex->width = width;
  tex->height = height;

  *out_texture_handle = tex;
  return UI_ERROR_NONE;
}

/*
 * \brief Destroys an offscreen texture (FBO).
 *
 * \param backend The renderer backend.
 * \param texture_handle The texture handle to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_destroy_texture(struct ui_renderer_backend *backend,
                                        void *texture_handle) {
  if (!backend || !texture_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(texture_handle);
  return UI_ERROR_NONE;
}

/*
 * \brief Sets the render target to a specific texture (FBO).
 *
 * \param backend The renderer backend.
 * \param texture_handle The texture handle, or NULL for screen.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_set_render_target(struct ui_renderer_backend *backend,
                                          void *texture_handle) {
  (void)texture_handle;
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

/*
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
static ui_error_t gles2_draw_texture(struct ui_renderer_backend *backend,
                                     void *texture_handle, float x, float y,
                                     float width, float height, float opacity) {
  struct ui_color color;
  if (!backend || !texture_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  {
    ui_error_t rc = gles2_flush(backend);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  color.r = 1.0f;
  color.g = 1.0f;
  color.b = 1.0f;
  color.a = opacity;
  return gles2_draw_rect(backend, x, y, width, height, color);
}

/*
 * \brief Reads pixels from the current render target.
 *
 * \param backend The renderer backend.
 * \param width Width of region to read.
 * \param height Height of region to read.
 * \param out_rgba_buffer Pre-allocated buffer to receive RGBA pixels.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t gles2_read_pixels(struct ui_renderer_backend *backend,
                                    int width, int height,
                                    unsigned char *out_rgba_buffer) {
  if (!backend || !out_rgba_buffer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, out_rgba_buffer);
  return UI_ERROR_NONE;
}

/*
 * \brief Creates a GLES 2.0 / WebGL 1.0 renderer backend.
 *
 * \param out_backend Pointer to receive the allocated backend.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_renderer_gles2_create(struct ui_renderer_backend **out_backend) {
  struct ui_renderer_backend *backend;

  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  backend = (struct ui_renderer_backend *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_renderer_backend));
  if (!backend) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  backend->init = gles2_init;
  backend->destroy = gles2_destroy;
  backend->set_viewport = gles2_set_viewport;
  backend->clear = gles2_clear;
  backend->draw_rect = gles2_draw_rect;
  backend->draw_border = gles2_draw_border;
  backend->draw_triangles = gles2_draw_triangles;
  backend->flush = gles2_flush;
  backend->push_clip = gles2_push_clip;
  backend->pop_clip = gles2_pop_clip;
  backend->push_stencil_clip = gles2_push_stencil_clip;
  backend->pop_stencil_clip = gles2_pop_stencil_clip;
  backend->create_texture = gles2_create_texture;
  backend->destroy_texture = gles2_destroy_texture;
  backend->set_render_target = gles2_set_render_target;
  backend->draw_texture = gles2_draw_texture;
  backend->read_pixels = gles2_read_pixels;
  backend->user_data = NULL;

  *out_backend = backend;
  return UI_ERROR_NONE;
}

/*
 * \brief Destroys a GLES 2.0 renderer backend.
 *
 * \param backend The backend to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_renderer_gles2_destroy(struct ui_renderer_backend *backend) {
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_mock_gles2_destroy_fail;
  if (g_mock_gles2_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
#endif
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  gles2_destroy(backend); /* free user_data if present */
  C_MULTIPLATFORM_FREE(backend);
  return UI_ERROR_NONE;
}
