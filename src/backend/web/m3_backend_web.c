#include "m3/m3_backend_web.h"

#include "m3/m3_log.h"
#include "m3/m3_math.h"
#include "m3/m3_object.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#if defined(M3_WEB_AVAILABLE)
#include <GLES2/gl2.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#if defined(M3_WEBGPU_AVAILABLE)
#include <emscripten/html5_webgpu.h>
#include <webgpu/webgpu.h>
#endif
#endif

#define M3_WEB_RETURN_IF_ERROR(rc)                                             \
  do {                                                                         \
    if ((rc) != M3_OK) {                                                       \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define M3_WEB_RETURN_IF_ERROR_CLEANUP(rc, cleanup)                            \
  do {                                                                         \
    if ((rc) != M3_OK) {                                                       \
      cleanup;                                                                 \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define M3_WEB_DEFAULT_HANDLE_CAPACITY 64u
#define M3_WEB_DEFAULT_CANVAS_ID "#canvas"

static int m3_web_backend_validate_config(const M3WebBackendConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->handle_capacity == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_canvas_id == NULL || config->utf8_canvas_id[0] == '\0') {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->allocator != NULL) {
    if (config->allocator->alloc == NULL ||
        config->allocator->realloc == NULL || config->allocator->free == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
  }
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL
m3_web_backend_test_validate_config(const M3WebBackendConfig *config) {
  return m3_web_backend_validate_config(config);
}
#endif

int M3_CALL m3_web_backend_is_available(M3Bool *out_available) {
  if (out_available == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#if defined(M3_WEB_AVAILABLE)
  *out_available = M3_TRUE;
#else
  *out_available = M3_FALSE;
#endif
  return M3_OK;
}

int M3_CALL m3_web_backend_config_init(M3WebBackendConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->handle_capacity = M3_WEB_DEFAULT_HANDLE_CAPACITY;
  config->clipboard_limit = (m3_usize) ~(m3_usize)0;
  config->utf8_canvas_id = M3_WEB_DEFAULT_CANVAS_ID;
  config->enable_logging = M3_TRUE;
  config->inline_tasks = M3_TRUE;
  return M3_OK;
}

#if defined(M3_WEB_AVAILABLE)

#define M3_WEB_TYPE_WINDOW 1
#define M3_WEB_TYPE_TEXTURE 2
#define M3_WEB_TYPE_FONT 3

#define M3_WEB_EVENT_CAPACITY 256u
#define M3_WEB_CLIP_STACK_CAPACITY 32u

typedef struct M3WebWindow {
  M3ObjectHeader header;
  struct M3WebBackend *backend;
  m3_i32 width;
  m3_i32 height;
  M3Scalar dpi_scale;
  m3_u32 flags;
  M3Bool visible;
} M3WebWindow;

typedef struct M3WebTexture {
  M3ObjectHeader header;
  struct M3WebBackend *backend;
  m3_i32 width;
  m3_i32 height;
  m3_u32 format;
  m3_u32 bytes_per_pixel;
  GLuint gl_texture;
#if defined(M3_WEBGPU_AVAILABLE)
  WGPUTexture wgpu_texture;
  WGPUTextureView wgpu_view;
  WGPUBindGroup wgpu_bind_group;
  WGPUTextureFormat wgpu_format;
  M3Bool wgpu_alpha_only;
#endif
} M3WebTexture;

typedef struct M3WebFont {
  M3ObjectHeader header;
  struct M3WebBackend *backend;
  m3_i32 size_px;
  m3_i32 weight;
  M3Bool italic;
} M3WebFont;

typedef struct M3WebEventQueue {
  M3InputEvent events[M3_WEB_EVENT_CAPACITY];
  m3_usize head;
  m3_usize count;
} M3WebEventQueue;

typedef struct M3WebGLState {
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
  GLuint program;
  GLint attr_pos;
  GLint attr_uv;
  GLint uni_color;
  GLint uni_use_tex;
  GLint uni_tex;
  GLint uni_opacity;
  GLint uni_transform;
  GLint uni_viewport;
  GLint uni_rect;
  GLint uni_radius;
  GLuint vbo;
  M3Bool initialized;
} M3WebGLState;

#if defined(M3_WEBGPU_AVAILABLE)
typedef struct M3WebGPUUniforms {
  float transform[16];
  float viewport[4];
  float rect[4];
  float color[4];
  float params[4];
} M3WebGPUUniforms;

typedef struct M3WebGPUState {
  WGPUInstance instance;
  WGPUDevice device;
  WGPUQueue queue;
  WGPUSurface surface;
  WGPUTextureFormat surface_format;
  WGPUTexture surface_texture;
  WGPUTextureView surface_view;
  WGPURenderPipeline pipeline;
  WGPUBindGroupLayout bind_group_layout;
  WGPUBuffer uniform_buffer;
  WGPUBuffer vertex_buffer;
  WGPUSampler sampler;
  WGPUTexture default_texture;
  WGPUTextureView default_view;
  WGPUBindGroup default_bind_group;
  WGPUCommandEncoder encoder;
  WGPURenderPassEncoder pass;
  WGPUColor clear_color;
  m3_u32 scissor_x;
  m3_u32 scissor_y;
  m3_u32 scissor_w;
  m3_u32 scissor_h;
  m3_i32 config_width;
  m3_i32 config_height;
  M3Bool scissor_dirty;
  M3Bool configured;
  M3Bool pass_active;
  M3Bool initialized;
} M3WebGPUState;
#endif

typedef struct M3WebVertex {
  float x;
  float y;
  float u;
  float v;
} M3WebVertex;

struct M3WebBackend {
  M3Allocator allocator;
  M3HandleSystem handles;
  M3WS ws;
  M3Gfx gfx;
  M3Env env;
  M3IO io;
  M3Sensors sensors;
  M3Camera camera;
  M3Network network;
  M3Tasks tasks;
  M3Bool initialized;
  M3Bool log_enabled;
  M3Bool log_owner;
  M3Bool inline_tasks;
  m3_u32 time_ms_fallback;
  char *clipboard;
  m3_usize clipboard_capacity;
  m3_usize clipboard_length;
  m3_usize clipboard_limit;
  char *canvas_id;
  M3WebWindow *active_window;
  M3Handle event_window;
  M3WebEventQueue events;
  M3Rect clip_stack[M3_WEB_CLIP_STACK_CAPACITY];
  m3_usize clip_depth;
  M3Mat3 transform;
  M3Bool has_transform;
  m3_i32 viewport_width;
  m3_i32 viewport_height;
  m3_i32 device_width;
  m3_i32 device_height;
  M3Bool callbacks_registered;
  M3Bool in_frame;
  M3WebGLState gl;
#if defined(M3_WEBGPU_AVAILABLE)
  M3Bool use_webgpu;
  M3WebGPUState wgpu;
#endif
};

static int m3_web_backend_log(struct M3WebBackend *backend, M3LogLevel level,
                              const char *message) {
  if (backend == NULL || message == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->log_enabled) {
    return M3_OK;
  }
  return m3_log_write(level, "m3.web", message);
}

static int m3_web_backend_resolve(struct M3WebBackend *backend, M3Handle handle,
                                  m3_u32 type_id, void **out_obj) {
  void *resolved;
  m3_u32 actual_type;
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (out_obj != NULL) {
    *out_obj = NULL;
  }

  rc =
      backend->handles.vtable->resolve(backend->handles.ctx, handle, &resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_object_get_type_id((const M3ObjectHeader *)resolved, &actual_type);
  M3_WEB_RETURN_IF_ERROR(rc);
  if (actual_type != type_id) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (out_obj != NULL) {
    *out_obj = resolved;
  }
  return M3_OK;
}

static int m3_web_backend_copy_string(const M3Allocator *allocator,
                                      const char *src, char **out_dst) {
  m3_usize length;
  m3_usize max_value;
  char *mem;
  int rc;

  if (allocator == NULL || src == NULL || out_dst == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  length = 0;
  while (src[length] != '\0') {
    length += 1;
  }

  max_value = (m3_usize) ~(m3_usize)0;
  if (length >= max_value) {
    return M3_ERR_OVERFLOW;
  }

  rc = allocator->alloc(allocator->ctx, length + 1, (void **)&mem);
  M3_WEB_RETURN_IF_ERROR(rc);

  if (length > 0) {
    memcpy(mem, src, length);
  }
  mem[length] = '\0';
  *out_dst = mem;
  return M3_OK;
}

static int m3_web_backend_map_result(int result) {
  switch (result) {
  case EMSCRIPTEN_RESULT_SUCCESS:
    return M3_OK;
  case EMSCRIPTEN_RESULT_NOT_SUPPORTED:
    return M3_ERR_UNSUPPORTED;
  case EMSCRIPTEN_RESULT_INVALID_PARAM:
    return M3_ERR_INVALID_ARGUMENT;
  case EMSCRIPTEN_RESULT_INVALID_TARGET:
  case EMSCRIPTEN_RESULT_UNKNOWN_TARGET:
    return M3_ERR_NOT_FOUND;
  case EMSCRIPTEN_RESULT_FAILED:
  case EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED:
  default:
    return M3_ERR_UNKNOWN;
  }
}

static int m3_web_backend_time_now_ms(m3_u32 *out_time_ms) {
  double now;
  double max_value;

  if (out_time_ms == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  now = emscripten_get_now();
  if (now < 0.0) {
    return M3_ERR_UNKNOWN;
  }

  max_value = (double)((m3_u32) ~(m3_u32)0);
  if (now > max_value) {
    return M3_ERR_OVERFLOW;
  }

  *out_time_ms = (m3_u32)now;
  return M3_OK;
}

static int m3_web_double_to_i32(double value, m3_i32 *out_value) {
  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (value > (double)INT_MAX) {
    return M3_ERR_RANGE;
  }
  if (value < (double)INT_MIN) {
    return M3_ERR_RANGE;
  }
  *out_value = (m3_i32)value;
  return M3_OK;
}

static int m3_web_event_queue_push(M3WebEventQueue *queue,
                                   const M3InputEvent *event) {
  m3_usize index;

  if (queue == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (queue->count >= M3_WEB_EVENT_CAPACITY) {
    return M3_ERR_BUSY;
  }

  index = (queue->head + queue->count) % M3_WEB_EVENT_CAPACITY;
  queue->events[index] = *event;
  queue->count += 1;
  return M3_OK;
}

static int m3_web_event_queue_pop(M3WebEventQueue *queue,
                                  M3InputEvent *out_event,
                                  M3Bool *out_has_event) {
  m3_usize index;

  if (queue == NULL || out_event == NULL || out_has_event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (queue->count == 0) {
    *out_has_event = M3_FALSE;
    memset(out_event, 0, sizeof(*out_event));
    return M3_OK;
  }

  index = queue->head;
  *out_event = queue->events[index];
  queue->head = (queue->head + 1) % M3_WEB_EVENT_CAPACITY;
  queue->count -= 1;
  *out_has_event = M3_TRUE;
  return M3_OK;
}

static int m3_web_build_modifiers(m3_u32 *out_modifiers, int shift, int ctrl,
                                  int alt, int meta) {
  m3_u32 mods;

  if (out_modifiers == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  mods = 0u;
  if (shift) {
    mods |= M3_MOD_SHIFT;
  }
  if (ctrl) {
    mods |= M3_MOD_CTRL;
  }
  if (alt) {
    mods |= M3_MOD_ALT;
  }
  if (meta) {
    mods |= M3_MOD_META;
  }

  *out_modifiers = mods;
  return M3_OK;
}

static int m3_web_event_stamp(struct M3WebBackend *backend,
                              M3InputEvent *event) {
  int rc;

  if (backend == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_web_backend_time_now_ms(&event->time_ms);
  if (rc != M3_OK) {
    backend->time_ms_fallback += 16u;
    event->time_ms = backend->time_ms_fallback;
    m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "events.time_fallback");
  }
  return rc;
}

static int m3_web_gl_check_error(struct M3WebBackend *backend,
                                 const char *label) {
  GLenum err;
  int rc;

  if (backend == NULL || label == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  err = glGetError();
  if (err == GL_NO_ERROR) {
    return M3_OK;
  }

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_ERROR, label);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_ERR_UNKNOWN;
}

static int m3_web_gl_compile_shader(struct M3WebBackend *backend, GLenum type,
                                    const char *source, GLuint *out_shader) {
  GLuint shader;
  GLint status;
  char info[256];
  GLsizei info_len;
  int rc;

  if (backend == NULL || source == NULL || out_shader == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  shader = glCreateShader(type);
  if (shader == 0) {
    return M3_ERR_UNKNOWN;
  }

  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    info[0] = '\0';
    info_len = 0;
    glGetShaderInfoLog(shader, (GLsizei)(sizeof(info) - 1u), &info_len, info);
    if (info_len > 0) {
      info[info_len] = '\0';
      m3_web_backend_log(backend, M3_LOG_LEVEL_ERROR, info);
    }
    glDeleteShader(shader);
    return M3_ERR_UNKNOWN;
  }

  rc = m3_web_gl_check_error(backend, "gl.compile_shader");
  if (rc != M3_OK) {
    glDeleteShader(shader);
    return rc;
  }

  *out_shader = shader;
  return M3_OK;
}

static int m3_web_gl_link_program(struct M3WebBackend *backend, GLuint vert,
                                  GLuint frag, GLuint *out_program) {
  GLuint program;
  GLint status;
  char info[256];
  GLsizei info_len;
  int rc;

  if (backend == NULL || out_program == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  program = glCreateProgram();
  if (program == 0) {
    return M3_ERR_UNKNOWN;
  }

  glAttachShader(program, vert);
  glAttachShader(program, frag);
  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    info[0] = '\0';
    info_len = 0;
    glGetProgramInfoLog(program, (GLsizei)(sizeof(info) - 1u), &info_len, info);
    if (info_len > 0) {
      info[info_len] = '\0';
      m3_web_backend_log(backend, M3_LOG_LEVEL_ERROR, info);
    }
    glDeleteProgram(program);
    return M3_ERR_UNKNOWN;
  }

  rc = m3_web_gl_check_error(backend, "gl.link_program");
  if (rc != M3_OK) {
    glDeleteProgram(program);
    return rc;
  }

  *out_program = program;
  return M3_OK;
}

static int m3_web_gl_init(struct M3WebBackend *backend) {
  EmscriptenWebGLContextAttributes attrs;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
  const char *vert_src;
  const char *frag_src;
  GLuint vert_shader;
  GLuint frag_shader;
  GLuint program;
  M3WebGLState *gl;
  int result;
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  gl = &backend->gl;
  if (gl->initialized) {
    return M3_OK;
  }

  emscripten_webgl_init_context_attributes(&attrs);
  attrs.alpha = 1;
  attrs.depth = 0;
  attrs.stencil = 1;
  attrs.antialias = 1;
  attrs.premultipliedAlpha = 1;
  attrs.preserveDrawingBuffer = 0;
  attrs.majorVersion = 1;
  attrs.minorVersion = 0;

  ctx = emscripten_webgl_create_context(backend->canvas_id, &attrs);
  if (ctx <= 0) {
    return M3_ERR_UNKNOWN;
  }

  result = emscripten_webgl_make_context_current(ctx);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    emscripten_webgl_destroy_context(ctx);
    return m3_web_backend_map_result(result);
  }

  vert_src = "attribute vec2 a_pos;\n"
             "attribute vec2 a_uv;\n"
             "uniform mat3 u_transform;\n"
             "uniform vec2 u_viewport;\n"
             "varying vec2 v_uv;\n"
             "varying vec2 v_pos;\n"
             "void main() {\n"
             "  vec3 pos = u_transform * vec3(a_pos, 1.0);\n"
             "  v_pos = pos.xy;\n"
             "  v_uv = a_uv;\n"
             "  vec2 clip = vec2((pos.x / u_viewport.x) * 2.0 - 1.0, 1.0 - "
             "(pos.y / u_viewport.y) * 2.0);\n"
             "  gl_Position = vec4(clip, 0.0, 1.0);\n"
             "}\n";

  frag_src =
      "precision mediump float;\n"
      "uniform vec4 u_color;\n"
      "uniform float u_use_tex;\n"
      "uniform sampler2D u_tex;\n"
      "uniform float u_opacity;\n"
      "uniform vec4 u_rect;\n"
      "uniform float u_radius;\n"
      "varying vec2 v_uv;\n"
      "varying vec2 v_pos;\n"
      "void main() {\n"
      "  vec4 color = u_color;\n"
      "  if (u_use_tex > 0.5) {\n"
      "    color *= texture2D(u_tex, v_uv);\n"
      "  }\n"
      "  if (u_radius > 0.0) {\n"
      "    vec2 rel = v_pos - u_rect.xy;\n"
      "    vec2 half_size = u_rect.zw * 0.5;\n"
      "    vec2 d = abs(rel - half_size) - (half_size - vec2(u_radius));\n"
      "    vec2 d_clamped = max(d, vec2(0.0));\n"
      "    float dist = length(d_clamped) + min(max(d.x, d.y), 0.0);\n"
      "    if (dist > 0.0) {\n"
      "      discard;\n"
      "    }\n"
      "  }\n"
      "  gl_FragColor = vec4(color.rgb, color.a * u_opacity);\n"
      "}\n";

  vert_shader = 0;
  frag_shader = 0;
  program = 0;

  rc = m3_web_gl_compile_shader(backend, GL_VERTEX_SHADER, vert_src,
                                &vert_shader);
  if (rc != M3_OK) {
    emscripten_webgl_destroy_context(ctx);
    return rc;
  }

  rc = m3_web_gl_compile_shader(backend, GL_FRAGMENT_SHADER, frag_src,
                                &frag_shader);
  if (rc != M3_OK) {
    glDeleteShader(vert_shader);
    emscripten_webgl_destroy_context(ctx);
    return rc;
  }

  rc = m3_web_gl_link_program(backend, vert_shader, frag_shader, &program);
  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);
  if (rc != M3_OK) {
    emscripten_webgl_destroy_context(ctx);
    return rc;
  }

  gl->context = ctx;
  gl->program = program;
  gl->attr_pos = glGetAttribLocation(program, "a_pos");
  gl->attr_uv = glGetAttribLocation(program, "a_uv");
  gl->uni_color = glGetUniformLocation(program, "u_color");
  gl->uni_use_tex = glGetUniformLocation(program, "u_use_tex");
  gl->uni_tex = glGetUniformLocation(program, "u_tex");
  gl->uni_opacity = glGetUniformLocation(program, "u_opacity");
  gl->uni_transform = glGetUniformLocation(program, "u_transform");
  gl->uni_viewport = glGetUniformLocation(program, "u_viewport");
  gl->uni_rect = glGetUniformLocation(program, "u_rect");
  gl->uni_radius = glGetUniformLocation(program, "u_radius");

  if (gl->attr_pos < 0 || gl->attr_uv < 0 || gl->uni_color < 0 ||
      gl->uni_use_tex < 0 || gl->uni_tex < 0 || gl->uni_opacity < 0 ||
      gl->uni_transform < 0 || gl->uni_viewport < 0 || gl->uni_rect < 0 ||
      gl->uni_radius < 0) {
    glDeleteProgram(program);
    emscripten_webgl_destroy_context(ctx);
    memset(gl, 0, sizeof(*gl));
    return M3_ERR_UNKNOWN;
  }

  glGenBuffers(1, &gl->vbo);
  if (gl->vbo == 0) {
    glDeleteProgram(program);
    emscripten_webgl_destroy_context(ctx);
    memset(gl, 0, sizeof(*gl));
    return M3_ERR_UNKNOWN;
  }

  glBindBuffer(GL_ARRAY_BUFFER, gl->vbo);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(M3WebVertex) * 4u), NULL,
               GL_DYNAMIC_DRAW);

  rc = m3_web_gl_check_error(backend, "gl.init");
  if (rc != M3_OK) {
    glDeleteBuffers(1, &gl->vbo);
    glDeleteProgram(program);
    emscripten_webgl_destroy_context(ctx);
    memset(gl, 0, sizeof(*gl));
    return rc;
  }

  gl->initialized = M3_TRUE;
  return M3_OK;
}

static int m3_web_gl_require_context(struct M3WebBackend *backend) {
  int result;
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_web_gl_init(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  result = emscripten_webgl_make_context_current(backend->gl.context);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }

  return M3_OK;
}

static int m3_web_gl_prepare(struct M3WebBackend *backend) {
  M3Mat3 transform;
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->in_frame) {
    return M3_ERR_STATE;
  }

#if defined(M3_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return m3_web_wgpu_clear(backend, color);
  }
#endif

  rc = m3_web_gl_require_context(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  glUseProgram(backend->gl.program);
  glBindBuffer(GL_ARRAY_BUFFER, backend->gl.vbo);
  glEnableVertexAttribArray((GLuint)backend->gl.attr_pos);
  glEnableVertexAttribArray((GLuint)backend->gl.attr_uv);
  glVertexAttribPointer((GLuint)backend->gl.attr_pos, 2, GL_FLOAT, GL_FALSE,
                        (GLsizei)sizeof(M3WebVertex), (void *)0);
  glVertexAttribPointer((GLuint)backend->gl.attr_uv, 2, GL_FLOAT, GL_FALSE,
                        (GLsizei)sizeof(M3WebVertex),
                        (void *)(sizeof(float) * 2u));

  if (backend->has_transform) {
    transform = backend->transform;
  } else {
    rc = m3_mat3_identity(&transform);
    M3_WEB_RETURN_IF_ERROR(rc);
  }

  glUniformMatrix3fv(backend->gl.uni_transform, 1, GL_FALSE, transform.m);
  glUniform2f(backend->gl.uni_viewport, (float)backend->viewport_width,
              (float)backend->viewport_height);

  return m3_web_gl_check_error(backend, "gl.prepare");
}

#if defined(M3_WEBGPU_AVAILABLE)
#define M3_WEB_WGPU_ROW_ALIGN 256u

static const char *g_m3_web_wgpu_shader =
    "struct Uniforms {\n"
    "  transform : mat4x4<f32>,\n"
    "  viewport : vec4<f32>,\n"
    "  rect : vec4<f32>,\n"
    "  color : vec4<f32>,\n"
    "  params : vec4<f32>,\n"
    "};\n"
    "@group(0) @binding(0) var<uniform> u : Uniforms;\n"
    "@group(0) @binding(1) var u_sampler : sampler;\n"
    "@group(0) @binding(2) var u_tex : texture_2d<f32>;\n"
    "struct VertexInput {\n"
    "  @location(0) pos : vec2<f32>,\n"
    "  @location(1) uv : vec2<f32>,\n"
    "};\n"
    "struct VertexOutput {\n"
    "  @builtin(position) position : vec4<f32>,\n"
    "  @location(0) uv : vec2<f32>,\n"
    "  @location(1) pos : vec2<f32>,\n"
    "};\n"
    "@vertex\n"
    "fn vs_main(input : VertexInput) -> VertexOutput {\n"
    "  var out : VertexOutput;\n"
    "  let p = u.transform * vec4<f32>(input.pos, 0.0, 1.0);\n"
    "  out.pos = p.xy;\n"
    "  out.uv = input.uv;\n"
    "  let clip = vec2<f32>((p.x / u.viewport.x) * 2.0 - 1.0, 1.0 - (p.y / "
    "u.viewport.y) * 2.0);\n"
    "  out.position = vec4<f32>(clip, 0.0, 1.0);\n"
    "  return out;\n"
    "}\n"
    "@fragment\n"
    "fn fs_main(input : VertexOutput) -> @location(0) vec4<f32> {\n"
    "  var color = u.color;\n"
    "  if (u.params.x > 0.5) {\n"
    "    let texel = textureSample(u_tex, u_sampler, input.uv);\n"
    "    if (u.params.w > 0.5) {\n"
    "      color = vec4<f32>(color.rgb, color.a * texel.r);\n"
    "    } else {\n"
    "      color = color * texel;\n"
    "    }\n"
    "  }\n"
    "  if (u.params.z > 0.0) {\n"
    "    let rel = input.pos - u.rect.xy;\n"
    "    let half_size = u.rect.zw * 0.5;\n"
    "    let d = abs(rel - half_size) - (half_size - vec2<f32>(u.params.z));\n"
    "    let d_clamped = max(d, vec2<f32>(0.0));\n"
    "    let dist = length(d_clamped) + min(max(d.x, d.y), 0.0);\n"
    "    if (dist > 0.0) {\n"
    "      discard;\n"
    "    }\n"
    "  }\n"
    "  return vec4<f32>(color.rgb, color.a * u.params.y);\n"
    "}\n";

static int m3_web_wgpu_align_size(m3_usize value, m3_usize alignment,
                                  m3_usize *out_value) {
  m3_usize remainder;
  m3_usize add;
  m3_usize max_value;

  if (out_value == NULL || alignment == 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  remainder = value % alignment;
  if (remainder == 0u) {
    *out_value = value;
    return M3_OK;
  }

  add = alignment - remainder;
  max_value = (m3_usize) ~(m3_usize)0;
  if (value > max_value - add) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = value + add;
  return M3_OK;
}

static int m3_web_wgpu_fill_transform(const M3Mat3 *transform, float *out_mat) {
  if (transform == NULL || out_mat == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  out_mat[0] = (float)transform->m[0];
  out_mat[1] = (float)transform->m[1];
  out_mat[2] = 0.0f;
  out_mat[3] = (float)transform->m[2];
  out_mat[4] = (float)transform->m[3];
  out_mat[5] = (float)transform->m[4];
  out_mat[6] = 0.0f;
  out_mat[7] = (float)transform->m[5];
  out_mat[8] = 0.0f;
  out_mat[9] = 0.0f;
  out_mat[10] = 1.0f;
  out_mat[11] = 0.0f;
  out_mat[12] = (float)transform->m[6];
  out_mat[13] = (float)transform->m[7];
  out_mat[14] = 0.0f;
  out_mat[15] = (float)transform->m[8];
  return M3_OK;
}

static int m3_web_wgpu_shutdown(struct M3WebBackend *backend) {
  M3WebGPUState *wgpu;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  wgpu = &backend->wgpu;

  if (wgpu->pass != NULL) {
    if (wgpu->pass_active) {
      wgpuRenderPassEncoderEnd(wgpu->pass);
    }
    wgpuRenderPassEncoderRelease(wgpu->pass);
    wgpu->pass = NULL;
  }
  wgpu->pass_active = M3_FALSE;

  if (wgpu->encoder != NULL) {
    wgpuCommandEncoderRelease(wgpu->encoder);
    wgpu->encoder = NULL;
  }

  if (wgpu->surface_view != NULL) {
    wgpuTextureViewRelease(wgpu->surface_view);
    wgpu->surface_view = NULL;
  }
  if (wgpu->surface_texture != NULL) {
    wgpuTextureRelease(wgpu->surface_texture);
    wgpu->surface_texture = NULL;
  }

  if (wgpu->default_bind_group != NULL) {
    wgpuBindGroupRelease(wgpu->default_bind_group);
    wgpu->default_bind_group = NULL;
  }
  if (wgpu->default_view != NULL) {
    wgpuTextureViewRelease(wgpu->default_view);
    wgpu->default_view = NULL;
  }
  if (wgpu->default_texture != NULL) {
    wgpuTextureRelease(wgpu->default_texture);
    wgpu->default_texture = NULL;
  }
  if (wgpu->sampler != NULL) {
    wgpuSamplerRelease(wgpu->sampler);
    wgpu->sampler = NULL;
  }
  if (wgpu->vertex_buffer != NULL) {
    wgpuBufferRelease(wgpu->vertex_buffer);
    wgpu->vertex_buffer = NULL;
  }
  if (wgpu->uniform_buffer != NULL) {
    wgpuBufferRelease(wgpu->uniform_buffer);
    wgpu->uniform_buffer = NULL;
  }
  if (wgpu->pipeline != NULL) {
    wgpuRenderPipelineRelease(wgpu->pipeline);
    wgpu->pipeline = NULL;
  }
  if (wgpu->bind_group_layout != NULL) {
    wgpuBindGroupLayoutRelease(wgpu->bind_group_layout);
    wgpu->bind_group_layout = NULL;
  }
  if (wgpu->surface != NULL) {
    wgpuSurfaceRelease(wgpu->surface);
    wgpu->surface = NULL;
  }
  if (wgpu->queue != NULL) {
    wgpuQueueRelease(wgpu->queue);
    wgpu->queue = NULL;
  }
  if (wgpu->device != NULL) {
    wgpuDeviceRelease(wgpu->device);
    wgpu->device = NULL;
  }
  if (wgpu->instance != NULL) {
    wgpuInstanceRelease(wgpu->instance);
    wgpu->instance = NULL;
  }

  wgpu->surface_format = WGPUTextureFormat_Undefined;
  wgpu->configured = M3_FALSE;
  wgpu->initialized = M3_FALSE;
  wgpu->scissor_dirty = M3_FALSE;
  wgpu->config_width = 0;
  wgpu->config_height = 0;
  memset(&wgpu->clear_color, 0, sizeof(wgpu->clear_color));
  wgpu->scissor_x = 0u;
  wgpu->scissor_y = 0u;
  wgpu->scissor_w = 0u;
  wgpu->scissor_h = 0u;

  return M3_OK;
}

static int m3_web_wgpu_init(struct M3WebBackend *backend) {
  M3WebGPUState *wgpu;
  WGPUInstanceDescriptor instance_desc;
  WGPUSurfaceDescriptorFromCanvasHTMLSelector canvas_desc;
  WGPUSurfaceDescriptor surface_desc;
  WGPUShaderSourceWGSL wgsl_desc;
  WGPUShaderModuleDescriptor shader_desc;
  WGPUBindGroupLayoutEntry layout_entries[3];
  WGPUBindGroupLayoutDescriptor layout_desc;
  WGPUPipelineLayout pipeline_layout;
  WGPUPipelineLayoutDescriptor pipeline_layout_desc;
  WGPUVertexAttribute attrs[2];
  WGPUVertexBufferLayout vb_layout;
  WGPUBlendState blend;
  WGPUColorTargetState color_target;
  WGPUFragmentState fragment_state;
  WGPUVertexState vertex_state;
  WGPUPrimitiveState primitive;
  WGPUMultisampleState multisample;
  WGPURenderPipelineDescriptor pipeline_desc;
  WGPUBufferDescriptor buffer_desc;
  WGPUSamplerDescriptor sampler_desc;
  WGPUTextureDescriptor texture_desc;
  WGPUTextureViewDescriptor view_desc;
  WGPUBindGroupEntry bind_entries[3];
  WGPUBindGroupDescriptor bind_desc;
  WGPUTexelCopyTextureInfo copy_dst;
  WGPUTexelCopyBufferLayout data_layout;
  WGPUExtent3D extent;
  unsigned char white_row[M3_WEB_WGPU_ROW_ALIGN];
  WGPUDevice device;
  WGPUQueue queue;
  WGPUInstance instance;
  WGPUSurface surface;
  WGPUTextureFormat format;
  WGPUShaderModule shader;
  m3_usize i;
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  wgpu = &backend->wgpu;
  if (wgpu->initialized) {
    return M3_OK;
  }

  rc = m3_web_wgpu_shutdown(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  device = emscripten_webgpu_get_device();
  if (device == NULL) {
    return M3_ERR_UNSUPPORTED;
  }
  queue = wgpuDeviceGetQueue(device);
  if (queue == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  memset(&instance_desc, 0, sizeof(instance_desc));
  instance = wgpuCreateInstance(&instance_desc);
  if (instance == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  memset(&canvas_desc, 0, sizeof(canvas_desc));
  canvas_desc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
  canvas_desc.chain.next = NULL;
  canvas_desc.selector = backend->canvas_id;

  memset(&surface_desc, 0, sizeof(surface_desc));
  surface_desc.nextInChain = (const WGPUChainedStruct *)&canvas_desc;

  surface = wgpuInstanceCreateSurface(instance, &surface_desc);
  if (surface == NULL) {
    wgpuInstanceRelease(instance);
    return M3_ERR_UNSUPPORTED;
  }

  format = emscripten_webgpu_get_swap_chain_preferred_format(device);
  if (format == WGPUTextureFormat_Undefined) {
    wgpuSurfaceRelease(surface);
    wgpuInstanceRelease(instance);
    return M3_ERR_UNSUPPORTED;
  }

  wgpu->instance = instance;
  wgpu->device = device;
  wgpu->queue = queue;
  wgpu->surface = surface;
  wgpu->surface_format = format;

  memset(&wgsl_desc, 0, sizeof(wgsl_desc));
  wgsl_desc.chain.sType = WGPUSType_ShaderSourceWGSL;
  wgsl_desc.chain.next = NULL;
  wgsl_desc.code.data = g_m3_web_wgpu_shader;
  wgsl_desc.code.length = WGPU_STRLEN;

  memset(&shader_desc, 0, sizeof(shader_desc));
  shader_desc.nextInChain = (const WGPUChainedStruct *)&wgsl_desc;

  shader = wgpuDeviceCreateShaderModule(device, &shader_desc);
  if (shader == NULL) {
    m3_web_wgpu_shutdown(backend);
    return M3_ERR_UNKNOWN;
  }

  memset(layout_entries, 0, sizeof(layout_entries));
  layout_entries[0].binding = 0;
  layout_entries[0].visibility =
      WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
  layout_entries[0].buffer.type = WGPUBufferBindingType_Uniform;
  layout_entries[0].buffer.minBindingSize = sizeof(M3WebGPUUniforms);

  layout_entries[1].binding = 1;
  layout_entries[1].visibility = WGPUShaderStage_Fragment;
  layout_entries[1].sampler.type = WGPUSamplerBindingType_Filtering;

  layout_entries[2].binding = 2;
  layout_entries[2].visibility = WGPUShaderStage_Fragment;
  layout_entries[2].texture.sampleType = WGPUTextureSampleType_Float;
  layout_entries[2].texture.viewDimension = WGPUTextureViewDimension_2D;
  layout_entries[2].texture.multisampled = 0;

  memset(&layout_desc, 0, sizeof(layout_desc));
  layout_desc.entryCount = (m3_u32)M3_COUNTOF(layout_entries);
  layout_desc.entries = layout_entries;

  wgpu->bind_group_layout =
      wgpuDeviceCreateBindGroupLayout(device, &layout_desc);
  if (wgpu->bind_group_layout == NULL) {
    wgpuShaderModuleRelease(shader);
    m3_web_wgpu_shutdown(backend);
    return M3_ERR_UNKNOWN;
  }

  memset(&pipeline_layout_desc, 0, sizeof(pipeline_layout_desc));
  pipeline_layout_desc.bindGroupLayoutCount = 1u;
  pipeline_layout_desc.bindGroupLayouts = &wgpu->bind_group_layout;

  pipeline_layout =
      wgpuDeviceCreatePipelineLayout(device, &pipeline_layout_desc);
  if (pipeline_layout == NULL) {
    wgpuShaderModuleRelease(shader);
    m3_web_wgpu_shutdown(backend);
    return M3_ERR_UNKNOWN;
  }

  memset(attrs, 0, sizeof(attrs));
  attrs[0].shaderLocation = 0;
  attrs[0].format = WGPUVertexFormat_Float32x2;
  attrs[0].offset = 0;
  attrs[1].shaderLocation = 1;
  attrs[1].format = WGPUVertexFormat_Float32x2;
  attrs[1].offset = (m3_usize)(sizeof(float) * 2u);

  memset(&vb_layout, 0, sizeof(vb_layout));
  vb_layout.arrayStride = (m3_usize)sizeof(M3WebVertex);
  vb_layout.stepMode = WGPUVertexStepMode_Vertex;
  vb_layout.attributeCount = (m3_u32)M3_COUNTOF(attrs);
  vb_layout.attributes = attrs;

  memset(&blend, 0, sizeof(blend));
  blend.color.operation = WGPUBlendOperation_Add;
  blend.color.srcFactor = WGPUBlendFactor_SrcAlpha;
  blend.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
  blend.alpha.operation = WGPUBlendOperation_Add;
  blend.alpha.srcFactor = WGPUBlendFactor_One;
  blend.alpha.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;

  memset(&color_target, 0, sizeof(color_target));
  color_target.format = format;
  color_target.blend = &blend;
  color_target.writeMask = WGPUColorWriteMask_All;

  memset(&fragment_state, 0, sizeof(fragment_state));
  fragment_state.module = shader;
  fragment_state.entryPoint.data = "fs_main";
  fragment_state.entryPoint.length = WGPU_STRLEN;
  fragment_state.targetCount = 1u;
  fragment_state.targets = &color_target;

  memset(&vertex_state, 0, sizeof(vertex_state));
  vertex_state.module = shader;
  vertex_state.entryPoint.data = "vs_main";
  vertex_state.entryPoint.length = WGPU_STRLEN;
  vertex_state.bufferCount = 1u;
  vertex_state.buffers = &vb_layout;

  memset(&primitive, 0, sizeof(primitive));
  primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
  primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
  primitive.frontFace = WGPUFrontFace_CCW;
  primitive.cullMode = WGPUCullMode_None;

  memset(&multisample, 0, sizeof(multisample));
  multisample.count = 1u;

  memset(&pipeline_desc, 0, sizeof(pipeline_desc));
  pipeline_desc.layout = pipeline_layout;
  pipeline_desc.vertex = vertex_state;
  pipeline_desc.fragment = &fragment_state;
  pipeline_desc.primitive = primitive;
  pipeline_desc.multisample = multisample;

  wgpu->pipeline = wgpuDeviceCreateRenderPipeline(device, &pipeline_desc);

  wgpuPipelineLayoutRelease(pipeline_layout);
  wgpuShaderModuleRelease(shader);

  if (wgpu->pipeline == NULL) {
    m3_web_wgpu_shutdown(backend);
    return M3_ERR_UNKNOWN;
  }

  memset(&buffer_desc, 0, sizeof(buffer_desc));
  buffer_desc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
  buffer_desc.size = (m3_usize)sizeof(M3WebGPUUniforms);

  wgpu->uniform_buffer = wgpuDeviceCreateBuffer(device, &buffer_desc);
  if (wgpu->uniform_buffer == NULL) {
    m3_web_wgpu_shutdown(backend);
    return M3_ERR_UNKNOWN;
  }

  memset(&buffer_desc, 0, sizeof(buffer_desc));
  buffer_desc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
  buffer_desc.size = (m3_usize)(sizeof(M3WebVertex) * 4u);

  wgpu->vertex_buffer = wgpuDeviceCreateBuffer(device, &buffer_desc);
  if (wgpu->vertex_buffer == NULL) {
    m3_web_wgpu_shutdown(backend);
    return M3_ERR_UNKNOWN;
  }

  memset(&sampler_desc, 0, sizeof(sampler_desc));
  sampler_desc.addressModeU = WGPUAddressMode_ClampToEdge;
  sampler_desc.addressModeV = WGPUAddressMode_ClampToEdge;
  sampler_desc.addressModeW = WGPUAddressMode_ClampToEdge;
  sampler_desc.magFilter = WGPUFilterMode_Linear;
  sampler_desc.minFilter = WGPUFilterMode_Linear;
  sampler_desc.mipmapFilter = WGPUFilterMode_Nearest;

  wgpu->sampler = wgpuDeviceCreateSampler(device, &sampler_desc);
  if (wgpu->sampler == NULL) {
    m3_web_wgpu_shutdown(backend);
    return M3_ERR_UNKNOWN;
  }

  memset(&texture_desc, 0, sizeof(texture_desc));
  texture_desc.dimension = WGPUTextureDimension_2D;
  texture_desc.size.width = 1u;
  texture_desc.size.height = 1u;
  texture_desc.size.depthOrArrayLayers = 1u;
  texture_desc.sampleCount = 1u;
  texture_desc.mipLevelCount = 1u;
  texture_desc.format = WGPUTextureFormat_RGBA8Unorm;
  texture_desc.usage =
      WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;

  wgpu->default_texture = wgpuDeviceCreateTexture(device, &texture_desc);
  if (wgpu->default_texture == NULL) {
    m3_web_wgpu_shutdown(backend);
    return M3_ERR_UNKNOWN;
  }

  memset(&view_desc, 0, sizeof(view_desc));
  wgpu->default_view = wgpuTextureCreateView(wgpu->default_texture, &view_desc);
  if (wgpu->default_view == NULL) {
    m3_web_wgpu_shutdown(backend);
    return M3_ERR_UNKNOWN;
  }

  for (i = 0; i < (m3_usize)M3_WEB_WGPU_ROW_ALIGN; ++i) {
    white_row[i] = 0u;
  }
  white_row[0] = 255u;
  white_row[1] = 255u;
  white_row[2] = 255u;
  white_row[3] = 255u;

  memset(&copy_dst, 0, sizeof(copy_dst));
  copy_dst.texture = wgpu->default_texture;
  copy_dst.mipLevel = 0u;
  copy_dst.origin.x = 0u;
  copy_dst.origin.y = 0u;
  copy_dst.origin.z = 0u;
  copy_dst.aspect = WGPUTextureAspect_All;

  memset(&data_layout, 0, sizeof(data_layout));
  data_layout.offset = 0u;
  data_layout.bytesPerRow = (m3_u32)M3_WEB_WGPU_ROW_ALIGN;
  data_layout.rowsPerImage = 1u;

  memset(&extent, 0, sizeof(extent));
  extent.width = 1u;
  extent.height = 1u;
  extent.depthOrArrayLayers = 1u;

  wgpuQueueWriteTexture(queue, &copy_dst, white_row,
                        (m3_usize)M3_WEB_WGPU_ROW_ALIGN, &data_layout, &extent);

  memset(bind_entries, 0, sizeof(bind_entries));
  bind_entries[0].binding = 0u;
  bind_entries[0].buffer = wgpu->uniform_buffer;
  bind_entries[0].offset = 0u;
  bind_entries[0].size = (m3_usize)sizeof(M3WebGPUUniforms);
  bind_entries[1].binding = 1u;
  bind_entries[1].sampler = wgpu->sampler;
  bind_entries[2].binding = 2u;
  bind_entries[2].textureView = wgpu->default_view;

  memset(&bind_desc, 0, sizeof(bind_desc));
  bind_desc.layout = wgpu->bind_group_layout;
  bind_desc.entryCount = (m3_u32)M3_COUNTOF(bind_entries);
  bind_desc.entries = bind_entries;

  wgpu->default_bind_group = wgpuDeviceCreateBindGroup(device, &bind_desc);
  if (wgpu->default_bind_group == NULL) {
    m3_web_wgpu_shutdown(backend);
    return M3_ERR_UNKNOWN;
  }

  wgpu->initialized = M3_TRUE;
  wgpu->configured = M3_FALSE;
  wgpu->pass_active = M3_FALSE;
  wgpu->scissor_dirty = M3_TRUE;
  wgpu->config_width = 0;
  wgpu->config_height = 0;
  memset(&wgpu->clear_color, 0, sizeof(wgpu->clear_color));

  return M3_OK;
}

static int m3_web_wgpu_configure_surface(struct M3WebBackend *backend,
                                         m3_i32 width, m3_i32 height) {
  M3WebGPUState *wgpu;
  WGPUSurfaceConfiguration config;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }

  wgpu = &backend->wgpu;
  if (!wgpu->initialized || wgpu->surface == NULL || wgpu->device == NULL) {
    return M3_ERR_STATE;
  }

  if (wgpu->configured && wgpu->config_width == width &&
      wgpu->config_height == height) {
    return M3_OK;
  }

  memset(&config, 0, sizeof(config));
  config.device = wgpu->device;
  config.format = wgpu->surface_format;
  config.usage = WGPUTextureUsage_RenderAttachment;
  config.width = (m3_u32)width;
  config.height = (m3_u32)height;
  config.presentMode = WGPUPresentMode_Fifo;
  config.alphaMode = WGPUCompositeAlphaMode_Auto;

  wgpuSurfaceConfigure(wgpu->surface, &config);
  wgpu->configured = M3_TRUE;
  wgpu->config_width = width;
  wgpu->config_height = height;
  return M3_OK;
}

static int m3_web_wgpu_acquire_frame(struct M3WebBackend *backend) {
  M3WebGPUState *wgpu;
  WGPUCommandEncoderDescriptor encoder_desc;
  WGPUSurfaceTexture surface_texture;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  wgpu = &backend->wgpu;
  if (!wgpu->configured || wgpu->surface == NULL || wgpu->device == NULL) {
    return M3_ERR_STATE;
  }

  if (wgpu->encoder != NULL) {
    wgpuCommandEncoderRelease(wgpu->encoder);
    wgpu->encoder = NULL;
  }
  if (wgpu->surface_view != NULL) {
    wgpuTextureViewRelease(wgpu->surface_view);
    wgpu->surface_view = NULL;
  }
  if (wgpu->surface_texture != NULL) {
    wgpuTextureRelease(wgpu->surface_texture);
    wgpu->surface_texture = NULL;
  }

  memset(&surface_texture, 0, sizeof(surface_texture));
  wgpuSurfaceGetCurrentTexture(wgpu->surface, &surface_texture);
  if (surface_texture.texture == NULL) {
    return M3_ERR_UNKNOWN;
  }
  if (surface_texture.status !=
          WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
      surface_texture.status !=
          WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
    wgpuTextureRelease(surface_texture.texture);
    return M3_ERR_UNKNOWN;
  }

  wgpu->surface_texture = surface_texture.texture;

  wgpu->surface_view = wgpuTextureCreateView(wgpu->surface_texture, NULL);
  if (wgpu->surface_view == NULL) {
    wgpuTextureRelease(wgpu->surface_texture);
    wgpu->surface_texture = NULL;
    return M3_ERR_UNKNOWN;
  }

  memset(&encoder_desc, 0, sizeof(encoder_desc));
  wgpu->encoder = wgpuDeviceCreateCommandEncoder(wgpu->device, &encoder_desc);
  if (wgpu->encoder == NULL) {
    wgpuTextureViewRelease(wgpu->surface_view);
    wgpuTextureRelease(wgpu->surface_texture);
    wgpu->surface_view = NULL;
    wgpu->surface_texture = NULL;
    return M3_ERR_UNKNOWN;
  }

  wgpu->pass_active = M3_FALSE;
  wgpu->scissor_dirty = M3_TRUE;
  return M3_OK;
}

static int m3_web_wgpu_begin_pass(struct M3WebBackend *backend, M3Bool clear) {
  M3WebGPUState *wgpu;
  WGPURenderPassColorAttachment color_attachment;
  WGPURenderPassDescriptor pass_desc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->in_frame) {
    return M3_ERR_STATE;
  }

  wgpu = &backend->wgpu;
  if (wgpu->encoder == NULL || wgpu->surface_view == NULL) {
    return M3_ERR_STATE;
  }

  if (wgpu->pass_active) {
    if (!clear) {
      return M3_OK;
    }
    wgpuRenderPassEncoderEnd(wgpu->pass);
    wgpuRenderPassEncoderRelease(wgpu->pass);
    wgpu->pass = NULL;
    wgpu->pass_active = M3_FALSE;
  }

  memset(&color_attachment, 0, sizeof(color_attachment));
  color_attachment.view = wgpu->surface_view;
  color_attachment.loadOp = clear ? WGPULoadOp_Clear : WGPULoadOp_Load;
  color_attachment.storeOp = WGPUStoreOp_Store;
  if (clear) {
    color_attachment.clearValue = wgpu->clear_color;
  } else {
    color_attachment.clearValue.r = 0.0;
    color_attachment.clearValue.g = 0.0;
    color_attachment.clearValue.b = 0.0;
    color_attachment.clearValue.a = 0.0;
  }

  memset(&pass_desc, 0, sizeof(pass_desc));
  pass_desc.colorAttachmentCount = 1u;
  pass_desc.colorAttachments = &color_attachment;

  wgpu->pass = wgpuCommandEncoderBeginRenderPass(wgpu->encoder, &pass_desc);
  if (wgpu->pass == NULL) {
    return M3_ERR_UNKNOWN;
  }

  wgpu->pass_active = M3_TRUE;
  wgpu->scissor_dirty = M3_TRUE;
  return M3_OK;
}

static int m3_web_wgpu_apply_clip(struct M3WebBackend *backend) {
  M3Rect rect;
  M3Scalar scale;
  m3_i32 x;
  m3_i32 y;
  m3_i32 w;
  m3_i32 h;
  m3_i32 max_w;
  m3_i32 max_h;
  M3WebGPUState *wgpu;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->in_frame) {
    return M3_ERR_STATE;
  }

  wgpu = &backend->wgpu;

  max_w = backend->device_width;
  max_h = backend->device_height;
  if (max_w <= 0 || max_h <= 0) {
    return M3_ERR_STATE;
  }

  if (backend->clip_depth == 0) {
    x = 0;
    y = 0;
    w = max_w;
    h = max_h;
  } else {
    if (backend->active_window == NULL) {
      return M3_ERR_STATE;
    }
    rect = backend->clip_stack[backend->clip_depth - 1u];
    if (rect.width < 0.0f || rect.height < 0.0f) {
      return M3_ERR_RANGE;
    }

    scale = backend->active_window->dpi_scale;
    if (scale <= 0.0f) {
      scale = 1.0f;
    }

    x = (m3_i32)(rect.x * scale);
    y = (m3_i32)(rect.y * scale);
    w = (m3_i32)(rect.width * scale);
    h = (m3_i32)(rect.height * scale);

    if (w < 0) {
      w = 0;
    }
    if (h < 0) {
      h = 0;
    }

    if (x < 0) {
      w += x;
      x = 0;
    }
    if (y < 0) {
      h += y;
      y = 0;
    }
    if (x + w > max_w) {
      w = max_w - x;
    }
    if (y + h > max_h) {
      h = max_h - y;
    }
    if (w < 0) {
      w = 0;
    }
    if (h < 0) {
      h = 0;
    }
  }

  wgpu->scissor_x = (m3_u32)x;
  wgpu->scissor_y = (m3_u32)y;
  wgpu->scissor_w = (m3_u32)w;
  wgpu->scissor_h = (m3_u32)h;
  wgpu->scissor_dirty = M3_TRUE;

  if (wgpu->pass_active && wgpu->pass != NULL) {
    wgpuRenderPassEncoderSetScissorRect(wgpu->pass, wgpu->scissor_x,
                                        wgpu->scissor_y, wgpu->scissor_w,
                                        wgpu->scissor_h);
    wgpu->scissor_dirty = M3_FALSE;
  }

  return M3_OK;
}

static int m3_web_wgpu_prepare_uniforms(struct M3WebBackend *backend,
                                        const M3Rect *rect, M3Color color,
                                        M3Scalar opacity, M3Scalar radius,
                                        M3Bool use_texture, M3Bool alpha_only,
                                        M3WebGPUUniforms *out_uniforms) {
  M3Mat3 transform;
  int rc;

  if (backend == NULL || rect == NULL || out_uniforms == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (backend->viewport_width <= 0 || backend->viewport_height <= 0) {
    return M3_ERR_STATE;
  }

  if (backend->has_transform) {
    transform = backend->transform;
  } else {
    rc = m3_mat3_identity(&transform);
    M3_WEB_RETURN_IF_ERROR(rc);
  }

  rc = m3_web_wgpu_fill_transform(&transform, out_uniforms->transform);
  M3_WEB_RETURN_IF_ERROR(rc);

  out_uniforms->viewport[0] = (float)backend->viewport_width;
  out_uniforms->viewport[1] = (float)backend->viewport_height;
  out_uniforms->viewport[2] = 0.0f;
  out_uniforms->viewport[3] = 0.0f;
  out_uniforms->rect[0] = (float)rect->x;
  out_uniforms->rect[1] = (float)rect->y;
  out_uniforms->rect[2] = (float)rect->width;
  out_uniforms->rect[3] = (float)rect->height;
  out_uniforms->color[0] = (float)color.r;
  out_uniforms->color[1] = (float)color.g;
  out_uniforms->color[2] = (float)color.b;
  out_uniforms->color[3] = (float)color.a;
  out_uniforms->params[0] = use_texture ? 1.0f : 0.0f;
  out_uniforms->params[1] = (float)opacity;
  out_uniforms->params[2] = (float)radius;
  out_uniforms->params[3] = alpha_only ? 1.0f : 0.0f;

  return M3_OK;
}

static int m3_web_wgpu_draw_quad(struct M3WebBackend *backend,
                                 const M3WebVertex *verts, M3Color color,
                                 M3Bool use_texture,
                                 const M3WebTexture *texture, M3Scalar opacity,
                                 const M3Rect *rect, M3Scalar radius) {
  M3WebGPUState *wgpu;
  M3WebGPUUniforms uniforms;
  WGPUBindGroup bind_group;
  int rc;

  if (backend == NULL || verts == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (use_texture && texture == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->in_frame) {
    return M3_ERR_STATE;
  }

  wgpu = &backend->wgpu;
  if (!wgpu->initialized || wgpu->queue == NULL ||
      wgpu->uniform_buffer == NULL || wgpu->vertex_buffer == NULL) {
    return M3_ERR_STATE;
  }

  rc = m3_web_wgpu_begin_pass(backend, M3_FALSE);
  M3_WEB_RETURN_IF_ERROR(rc);
  if (wgpu->pass == NULL || wgpu->pipeline == NULL) {
    return M3_ERR_STATE;
  }

  bind_group =
      use_texture ? texture->wgpu_bind_group : wgpu->default_bind_group;
  if (bind_group == NULL) {
    return M3_ERR_STATE;
  }

  rc = m3_web_wgpu_prepare_uniforms(
      backend, rect, color, opacity, radius, use_texture,
      use_texture ? texture->wgpu_alpha_only : M3_FALSE, &uniforms);
  M3_WEB_RETURN_IF_ERROR(rc);

  wgpuQueueWriteBuffer(wgpu->queue, wgpu->uniform_buffer, 0u, &uniforms,
                       (m3_usize)sizeof(uniforms));
  wgpuQueueWriteBuffer(wgpu->queue, wgpu->vertex_buffer, 0u, verts,
                       (m3_usize)(sizeof(M3WebVertex) * 4u));

  if (wgpu->scissor_dirty) {
    rc = m3_web_wgpu_apply_clip(backend);
    M3_WEB_RETURN_IF_ERROR(rc);
  }

  wgpuRenderPassEncoderSetPipeline(wgpu->pass, wgpu->pipeline);
  wgpuRenderPassEncoderSetBindGroup(wgpu->pass, 0u, bind_group, 0u, NULL);
  wgpuRenderPassEncoderSetVertexBuffer(wgpu->pass, 0u, wgpu->vertex_buffer, 0u,
                                       (m3_usize)(sizeof(M3WebVertex) * 4u));
  wgpuRenderPassEncoderDraw(wgpu->pass, 4u, 1u, 0u, 0u);

  return M3_OK;
}

static int m3_web_wgpu_begin_frame(struct M3WebBackend *backend,
                                   M3WebWindow *resolved, m3_i32 width,
                                   m3_i32 height, M3Scalar dpi_scale) {
  m3_i32 device_width;
  m3_i32 device_height;
  int rc;

  if (backend == NULL || resolved == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (dpi_scale <= 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_web_wgpu_init(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  if (resolved->width != width || resolved->height != height ||
      resolved->dpi_scale != dpi_scale) {
    resolved->width = width;
    resolved->height = height;
    resolved->dpi_scale = dpi_scale;
    rc = m3_web_backend_set_canvas_size(backend, width, height, dpi_scale);
    M3_WEB_RETURN_IF_ERROR(rc);
  }

  device_width = (m3_i32)((M3Scalar)width * dpi_scale + 0.5f);
  device_height = (m3_i32)((M3Scalar)height * dpi_scale + 0.5f);
  if (device_width <= 0 || device_height <= 0) {
    return M3_ERR_RANGE;
  }

  backend->viewport_width = width;
  backend->viewport_height = height;
  backend->device_width = device_width;
  backend->device_height = device_height;
  backend->active_window = resolved;
  backend->in_frame = M3_TRUE;

  rc = m3_web_wgpu_configure_surface(backend, device_width, device_height);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_wgpu_acquire_frame(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_wgpu_apply_clip(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static int m3_web_wgpu_end_frame(struct M3WebBackend *backend) {
  M3WebGPUState *wgpu;
  WGPUCommandBufferDescriptor cmd_desc;
  WGPUCommandBuffer command_buffer;
  WGPUStatus present_status;
  int result;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->in_frame) {
    return M3_ERR_STATE;
  }

  wgpu = &backend->wgpu;
  if (wgpu->pass != NULL) {
    if (wgpu->pass_active) {
      wgpuRenderPassEncoderEnd(wgpu->pass);
    }
    wgpuRenderPassEncoderRelease(wgpu->pass);
    wgpu->pass = NULL;
  }
  wgpu->pass_active = M3_FALSE;

  if (wgpu->encoder == NULL || wgpu->queue == NULL) {
    return M3_ERR_STATE;
  }

  memset(&cmd_desc, 0, sizeof(cmd_desc));
  command_buffer = wgpuCommandEncoderFinish(wgpu->encoder, &cmd_desc);
  if (command_buffer == NULL) {
    wgpuCommandEncoderRelease(wgpu->encoder);
    wgpu->encoder = NULL;
    result = M3_ERR_UNKNOWN;
  } else {
    wgpuQueueSubmit(wgpu->queue, 1u, &command_buffer);
    wgpuCommandBufferRelease(command_buffer);
    wgpuCommandEncoderRelease(wgpu->encoder);
    wgpu->encoder = NULL;
    result = M3_OK;
  }

  if (result == M3_OK) {
    if (wgpu->surface != NULL && wgpu->surface_texture != NULL) {
      present_status = wgpuSurfacePresent(wgpu->surface);
      if (present_status != WGPUStatus_Success) {
        result = M3_ERR_UNKNOWN;
      }
    } else {
      result = M3_ERR_STATE;
    }
  }

  if (wgpu->surface_view != NULL) {
    wgpuTextureViewRelease(wgpu->surface_view);
    wgpu->surface_view = NULL;
  }
  if (wgpu->surface_texture != NULL) {
    wgpuTextureRelease(wgpu->surface_texture);
    wgpu->surface_texture = NULL;
  }

  backend->active_window = NULL;
  backend->in_frame = M3_FALSE;
  return result;
}

static int m3_web_wgpu_clear(struct M3WebBackend *backend, M3Color color) {
  M3WebGPUState *wgpu;
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->in_frame) {
    return M3_ERR_STATE;
  }

  wgpu = &backend->wgpu;
  wgpu->clear_color.r = color.r;
  wgpu->clear_color.g = color.g;
  wgpu->clear_color.b = color.b;
  wgpu->clear_color.a = color.a;

  rc = m3_web_wgpu_begin_pass(backend, M3_TRUE);
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_OK;
}
#endif

static int m3_web_gfx_apply_clip(struct M3WebBackend *backend) {
  M3Rect rect;
  M3Scalar scale;
  m3_i32 x;
  m3_i32 y;
  m3_i32 w;
  m3_i32 h;
  m3_i32 max_w;
  m3_i32 max_h;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

#if defined(M3_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return m3_web_wgpu_apply_clip(backend);
  }
#endif

  if (backend->clip_depth == 0) {
    glDisable(GL_SCISSOR_TEST);
    return m3_web_gl_check_error(backend, "gfx.clip.disable");
  }

  if (backend->active_window == NULL) {
    return M3_ERR_STATE;
  }

  rect = backend->clip_stack[backend->clip_depth - 1u];
  if (rect.width < 0.0f || rect.height < 0.0f) {
    return M3_ERR_RANGE;
  }

  scale = backend->active_window->dpi_scale;
  if (scale <= 0.0f) {
    scale = 1.0f;
  }

  x = (m3_i32)(rect.x * scale);
  y = (m3_i32)(rect.y * scale);
  w = (m3_i32)(rect.width * scale);
  h = (m3_i32)(rect.height * scale);

  if (w < 0) {
    w = 0;
  }
  if (h < 0) {
    h = 0;
  }

  max_w = backend->device_width;
  max_h = backend->device_height;
  if (max_w <= 0 || max_h <= 0) {
    return M3_ERR_STATE;
  }

  y = max_h - (y + h);

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > max_w) {
    w = max_w - x;
  }
  if (y + h > max_h) {
    h = max_h - y;
  }
  if (w < 0) {
    w = 0;
  }
  if (h < 0) {
    h = 0;
  }

  glEnable(GL_SCISSOR_TEST);
  glScissor(x, y, w, h);

  return m3_web_gl_check_error(backend, "gfx.clip");
}

static int m3_web_gfx_draw_quad(struct M3WebBackend *backend,
                                const M3WebVertex *verts, M3Color color,
                                M3Bool use_texture, GLuint texture,
                                M3Scalar opacity, const M3Rect *rect,
                                M3Scalar radius) {
  int rc;

  if (backend == NULL || verts == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (use_texture && texture == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_web_gl_prepare(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  glUniform4f(backend->gl.uni_color, color.r, color.g, color.b, color.a);
  glUniform1f(backend->gl.uni_use_tex, use_texture ? 1.0f : 0.0f);
  glUniform1f(backend->gl.uni_opacity, opacity);
  glUniform4f(backend->gl.uni_rect, rect->x, rect->y, rect->width,
              rect->height);
  glUniform1f(backend->gl.uni_radius, radius);

  if (use_texture) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(backend->gl.uni_tex, 0);
  } else {
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(M3WebVertex) * 4u), verts,
               GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  return m3_web_gl_check_error(backend, "gfx.draw_quad");
}

static int m3_web_backend_set_canvas_size(struct M3WebBackend *backend,
                                          m3_i32 width, m3_i32 height,
                                          M3Scalar dpi_scale) {
  m3_i32 device_width;
  m3_i32 device_height;
  int result;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (backend->canvas_id == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (dpi_scale <= 0.0f) {
    dpi_scale = 1.0f;
  }

  device_width = (m3_i32)((M3Scalar)width * dpi_scale + 0.5f);
  device_height = (m3_i32)((M3Scalar)height * dpi_scale + 0.5f);
  if (device_width <= 0 || device_height <= 0) {
    return M3_ERR_RANGE;
  }

  result = emscripten_set_canvas_element_size(backend->canvas_id, device_width,
                                              device_height);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }

  result = emscripten_set_element_css_size(backend->canvas_id, (double)width,
                                           (double)height);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }

  return M3_OK;
}

static EM_BOOL m3_web_on_mouse_down(int event_type,
                                    const EmscriptenMouseEvent *event,
                                    void *user_data) {
  struct M3WebBackend *backend;
  M3InputEvent input;
  m3_u32 mods;
  int rc;

  M3_UNUSED(event_type);

  backend = (struct M3WebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc = m3_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != M3_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = M3_INPUT_POINTER_DOWN;
  input.modifiers = mods;
  input.window = backend->event_window;
  m3_web_event_stamp(backend, &input);

  input.data.pointer.pointer_id = 0;
  input.data.pointer.x = (m3_i32)event->canvasX;
  input.data.pointer.y = (m3_i32)event->canvasY;
  input.data.pointer.buttons = (m3_i32)event->buttons;
  input.data.pointer.scroll_x = 0;
  input.data.pointer.scroll_y = 0;

  rc = m3_web_event_queue_push(&backend->events, &input);
  if (rc != M3_OK) {
    m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL m3_web_on_mouse_up(int event_type,
                                  const EmscriptenMouseEvent *event,
                                  void *user_data) {
  struct M3WebBackend *backend;
  M3InputEvent input;
  m3_u32 mods;
  int rc;

  M3_UNUSED(event_type);

  backend = (struct M3WebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc = m3_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != M3_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = M3_INPUT_POINTER_UP;
  input.modifiers = mods;
  input.window = backend->event_window;
  m3_web_event_stamp(backend, &input);

  input.data.pointer.pointer_id = 0;
  input.data.pointer.x = (m3_i32)event->canvasX;
  input.data.pointer.y = (m3_i32)event->canvasY;
  input.data.pointer.buttons = (m3_i32)event->buttons;
  input.data.pointer.scroll_x = 0;
  input.data.pointer.scroll_y = 0;

  rc = m3_web_event_queue_push(&backend->events, &input);
  if (rc != M3_OK) {
    m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL m3_web_on_mouse_move(int event_type,
                                    const EmscriptenMouseEvent *event,
                                    void *user_data) {
  struct M3WebBackend *backend;
  M3InputEvent input;
  m3_u32 mods;
  int rc;

  M3_UNUSED(event_type);

  backend = (struct M3WebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc = m3_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != M3_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = M3_INPUT_POINTER_MOVE;
  input.modifiers = mods;
  input.window = backend->event_window;
  m3_web_event_stamp(backend, &input);

  input.data.pointer.pointer_id = 0;
  input.data.pointer.x = (m3_i32)event->canvasX;
  input.data.pointer.y = (m3_i32)event->canvasY;
  input.data.pointer.buttons = (m3_i32)event->buttons;
  input.data.pointer.scroll_x = 0;
  input.data.pointer.scroll_y = 0;

  rc = m3_web_event_queue_push(&backend->events, &input);
  if (rc != M3_OK) {
    m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL m3_web_on_wheel(int event_type,
                               const EmscriptenWheelEvent *event,
                               void *user_data) {
  struct M3WebBackend *backend;
  M3InputEvent input;
  m3_u32 mods;
  m3_i32 dx;
  m3_i32 dy;
  int rc;

  M3_UNUSED(event_type);

  backend = (struct M3WebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc =
      m3_web_build_modifiers(&mods, event->mouse.shiftKey, event->mouse.ctrlKey,
                             event->mouse.altKey, event->mouse.metaKey);
  if (rc != M3_OK) {
    return EM_FALSE;
  }

  dx = 0;
  dy = 0;
  rc = m3_web_double_to_i32(event->deltaX, &dx);
  if (rc != M3_OK) {
    dx = 0;
  }
  rc = m3_web_double_to_i32(event->deltaY, &dy);
  if (rc != M3_OK) {
    dy = 0;
  }

  memset(&input, 0, sizeof(input));
  input.type = M3_INPUT_POINTER_SCROLL;
  input.modifiers = mods;
  input.window = backend->event_window;
  m3_web_event_stamp(backend, &input);

  input.data.pointer.pointer_id = 0;
  input.data.pointer.x = (m3_i32)event->mouse.canvasX;
  input.data.pointer.y = (m3_i32)event->mouse.canvasY;
  input.data.pointer.buttons = (m3_i32)event->mouse.buttons;
  input.data.pointer.scroll_x = dx;
  input.data.pointer.scroll_y = dy;

  rc = m3_web_event_queue_push(&backend->events, &input);
  if (rc != M3_OK) {
    m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL m3_web_on_touch(int event_type,
                               const EmscriptenTouchEvent *event,
                               void *user_data) {
  struct M3WebBackend *backend;
  m3_u32 mods;
  int rc;
  int i;

  backend = (struct M3WebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc = m3_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != M3_OK) {
    return EM_FALSE;
  }

  for (i = 0; i < event->numTouches; i += 1) {
    M3InputEvent input;
    m3_i32 buttons;

    if (!event->touches[i].isChanged) {
      continue;
    }

    buttons = 1;
    if (event_type == EMSCRIPTEN_EVENT_TOUCHEND ||
        event_type == EMSCRIPTEN_EVENT_TOUCHCANCEL) {
      buttons = 0;
    }

    memset(&input, 0, sizeof(input));
    if (event_type == EMSCRIPTEN_EVENT_TOUCHSTART) {
      input.type = M3_INPUT_POINTER_DOWN;
    } else if (event_type == EMSCRIPTEN_EVENT_TOUCHEND ||
               event_type == EMSCRIPTEN_EVENT_TOUCHCANCEL) {
      input.type = M3_INPUT_POINTER_UP;
    } else {
      input.type = M3_INPUT_POINTER_MOVE;
    }
    input.modifiers = mods;
    input.window = backend->event_window;
    m3_web_event_stamp(backend, &input);

    input.data.pointer.pointer_id = (m3_i32)event->touches[i].identifier;
    input.data.pointer.x = (m3_i32)event->touches[i].canvasX;
    input.data.pointer.y = (m3_i32)event->touches[i].canvasY;
    input.data.pointer.buttons = buttons;
    input.data.pointer.scroll_x = 0;
    input.data.pointer.scroll_y = 0;

    rc = m3_web_event_queue_push(&backend->events, &input);
    if (rc != M3_OK) {
      m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "events.full");
      break;
    }
  }

  return EM_TRUE;
}

static EM_BOOL m3_web_on_key_down(int event_type,
                                  const EmscriptenKeyboardEvent *event,
                                  void *user_data) {
  struct M3WebBackend *backend;
  M3InputEvent input;
  m3_u32 mods;
  int rc;

  M3_UNUSED(event_type);

  backend = (struct M3WebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc = m3_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != M3_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = M3_INPUT_KEY_DOWN;
  input.modifiers = mods;
  input.window = backend->event_window;
  m3_web_event_stamp(backend, &input);

  input.data.key.key_code = (m3_u32)event->keyCode;
  input.data.key.native_code = (m3_u32)event->which;
  input.data.key.is_repeat = event->repeat ? M3_TRUE : M3_FALSE;

  rc = m3_web_event_queue_push(&backend->events, &input);
  if (rc != M3_OK) {
    m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL m3_web_on_key_up(int event_type,
                                const EmscriptenKeyboardEvent *event,
                                void *user_data) {
  struct M3WebBackend *backend;
  M3InputEvent input;
  m3_u32 mods;
  int rc;

  M3_UNUSED(event_type);

  backend = (struct M3WebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc = m3_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != M3_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = M3_INPUT_KEY_UP;
  input.modifiers = mods;
  input.window = backend->event_window;
  m3_web_event_stamp(backend, &input);

  input.data.key.key_code = (m3_u32)event->keyCode;
  input.data.key.native_code = (m3_u32)event->which;
  input.data.key.is_repeat = event->repeat ? M3_TRUE : M3_FALSE;

  rc = m3_web_event_queue_push(&backend->events, &input);
  if (rc != M3_OK) {
    m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL m3_web_on_key_press(int event_type,
                                   const EmscriptenKeyboardEvent *event,
                                   void *user_data) {
  struct M3WebBackend *backend;
  M3InputEvent input;
  m3_u32 mods;
  m3_usize length;
  int rc;

  M3_UNUSED(event_type);

  backend = (struct M3WebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  if (event->charCode == 0 && event->key[0] == '\0') {
    return EM_FALSE;
  }

  length = 0;
  while (event->key[length] != '\0') {
    if (length >= (sizeof(input.data.text.utf8) - 1u)) {
      return EM_FALSE;
    }
    length += 1;
  }

  if (length == 0) {
    return EM_FALSE;
  }

  rc = m3_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != M3_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = M3_INPUT_TEXT;
  input.modifiers = mods;
  input.window = backend->event_window;
  m3_web_event_stamp(backend, &input);

  memcpy(input.data.text.utf8, event->key, length);
  input.data.text.utf8[length] = '\0';
  input.data.text.length = (m3_u32)length;

  rc = m3_web_event_queue_push(&backend->events, &input);
  if (rc != M3_OK) {
    m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL m3_web_on_focus(int event_type,
                               const EmscriptenFocusEvent *event,
                               void *user_data) {
  struct M3WebBackend *backend;
  M3InputEvent input;
  int rc;

  M3_UNUSED(event_type);
  M3_UNUSED(event);

  backend = (struct M3WebBackend *)user_data;
  if (backend == NULL) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = M3_INPUT_WINDOW_FOCUS;
  input.modifiers = 0u;
  input.window = backend->event_window;
  m3_web_event_stamp(backend, &input);

  rc = m3_web_event_queue_push(&backend->events, &input);
  if (rc != M3_OK) {
    m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL m3_web_on_blur(int event_type, const EmscriptenFocusEvent *event,
                              void *user_data) {
  struct M3WebBackend *backend;
  M3InputEvent input;
  int rc;

  M3_UNUSED(event_type);
  M3_UNUSED(event);

  backend = (struct M3WebBackend *)user_data;
  if (backend == NULL) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = M3_INPUT_WINDOW_BLUR;
  input.modifiers = 0u;
  input.window = backend->event_window;
  m3_web_event_stamp(backend, &input);

  rc = m3_web_event_queue_push(&backend->events, &input);
  if (rc != M3_OK) {
    m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL m3_web_on_resize(int event_type, const EmscriptenUiEvent *event,
                                void *user_data) {
  struct M3WebBackend *backend;
  M3WebWindow *window;
  M3InputEvent input;
  double css_width;
  double css_height;
  m3_i32 width;
  m3_i32 height;
  int result;
  int rc;

  M3_UNUSED(event_type);
  M3_UNUSED(event);

  backend = (struct M3WebBackend *)user_data;
  if (backend == NULL) {
    return EM_FALSE;
  }

  if (backend->event_window.id == 0u &&
      backend->event_window.generation == 0u) {
    return EM_FALSE;
  }

  window = NULL;
  rc = m3_web_backend_resolve(backend, backend->event_window,
                              M3_WEB_TYPE_WINDOW, (void **)&window);
  if (rc != M3_OK || window == NULL) {
    return EM_FALSE;
  }

  css_width = 0.0;
  css_height = 0.0;
  result = emscripten_get_element_css_size(backend->canvas_id, &css_width,
                                           &css_height);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return EM_FALSE;
  }

  rc = m3_web_double_to_i32(css_width, &width);
  if (rc != M3_OK) {
    return EM_FALSE;
  }
  rc = m3_web_double_to_i32(css_height, &height);
  if (rc != M3_OK) {
    return EM_FALSE;
  }

  if (width <= 0 || height <= 0) {
    return EM_FALSE;
  }

  window->width = width;
  window->height = height;
  rc =
      m3_web_backend_set_canvas_size(backend, width, height, window->dpi_scale);
  if (rc != M3_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = M3_INPUT_WINDOW_RESIZE;
  input.modifiers = 0u;
  input.window = backend->event_window;
  m3_web_event_stamp(backend, &input);

  input.data.window.width = width;
  input.data.window.height = height;

  rc = m3_web_event_queue_push(&backend->events, &input);
  if (rc != M3_OK) {
    m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static int m3_web_register_callbacks(struct M3WebBackend *backend) {
  int result;
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (backend->callbacks_registered) {
    return M3_OK;
  }

  result = emscripten_set_mousedown_callback(backend->canvas_id, backend, 1,
                                             m3_web_on_mouse_down);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }
  result = emscripten_set_mouseup_callback(backend->canvas_id, backend, 1,
                                           m3_web_on_mouse_up);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }
  result = emscripten_set_mousemove_callback(backend->canvas_id, backend, 1,
                                             m3_web_on_mouse_move);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }
  result = emscripten_set_wheel_callback(backend->canvas_id, backend, 1,
                                         m3_web_on_wheel);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }

  result = emscripten_set_touchstart_callback(backend->canvas_id, backend, 1,
                                              m3_web_on_touch);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }
  result = emscripten_set_touchend_callback(backend->canvas_id, backend, 1,
                                            m3_web_on_touch);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }
  result = emscripten_set_touchmove_callback(backend->canvas_id, backend, 1,
                                             m3_web_on_touch);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }
  result = emscripten_set_touchcancel_callback(backend->canvas_id, backend, 1,
                                               m3_web_on_touch);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }

  result = emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                           backend, 1, m3_web_on_key_down);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }
  result = emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                         backend, 1, m3_web_on_key_up);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }
  result = emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                            backend, 1, m3_web_on_key_press);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }

  result = emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                         backend, 1, m3_web_on_focus);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }
  result = emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, backend,
                                        1, m3_web_on_blur);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }
  result = emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                          backend, 1, m3_web_on_resize);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return m3_web_backend_map_result(result);
  }

  backend->callbacks_registered = M3_TRUE;
  rc =
      m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.callbacks_registered");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static int m3_web_object_retain(void *obj) {
  return m3_object_retain((M3ObjectHeader *)obj);
}

static int m3_web_object_release(void *obj) {
  return m3_object_release((M3ObjectHeader *)obj);
}

static int m3_web_object_get_type_id(void *obj, m3_u32 *out_type_id) {
  return m3_object_get_type_id((const M3ObjectHeader *)obj, out_type_id);
}

static int m3_web_window_destroy(void *obj) {
  M3WebWindow *window;
  struct M3WebBackend *backend;
  int rc;

  window = (M3WebWindow *)obj;
  backend = window->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  window->header.handle);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = backend->allocator.free(backend->allocator.ctx, window);
  M3_WEB_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static int m3_web_texture_destroy(void *obj) {
  M3WebTexture *texture;
  struct M3WebBackend *backend;
  int rc;

  texture = (M3WebTexture *)obj;
  backend = texture->backend;

  if (texture->gl_texture != 0) {
    rc = m3_web_gl_require_context(backend);
    M3_WEB_RETURN_IF_ERROR(rc);
    glDeleteTextures(1, &texture->gl_texture);
    texture->gl_texture = 0;
  }
#if defined(M3_WEBGPU_AVAILABLE)
  if (texture->wgpu_bind_group != NULL) {
    wgpuBindGroupRelease(texture->wgpu_bind_group);
    texture->wgpu_bind_group = NULL;
  }
  if (texture->wgpu_view != NULL) {
    wgpuTextureViewRelease(texture->wgpu_view);
    texture->wgpu_view = NULL;
  }
  if (texture->wgpu_texture != NULL) {
    wgpuTextureRelease(texture->wgpu_texture);
    texture->wgpu_texture = NULL;
  }
#endif

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  texture->header.handle);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = backend->allocator.free(backend->allocator.ctx, texture);
  M3_WEB_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static int m3_web_font_destroy(void *obj) {
  M3WebFont *font;
  struct M3WebBackend *backend;
  int rc;

  font = (M3WebFont *)obj;
  backend = font->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  font->header.handle);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = backend->allocator.free(backend->allocator.ctx, font);
  M3_WEB_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static const M3ObjectVTable g_m3_web_window_vtable = {
    m3_web_object_retain, m3_web_object_release, m3_web_window_destroy,
    m3_web_object_get_type_id};

static const M3ObjectVTable g_m3_web_texture_vtable = {
    m3_web_object_retain, m3_web_object_release, m3_web_texture_destroy,
    m3_web_object_get_type_id};

static const M3ObjectVTable g_m3_web_font_vtable = {
    m3_web_object_retain, m3_web_object_release, m3_web_font_destroy,
    m3_web_object_get_type_id};

static int m3_web_ws_init(void *ws, const M3WSConfig *config) {
  struct M3WebBackend *backend;
  int rc;

  if (ws == NULL || config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_app_name == NULL || config->utf8_app_id == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.init");
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_register_callbacks(backend);
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static int m3_web_ws_shutdown(void *ws) {
  struct M3WebBackend *backend;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.shutdown");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static int m3_web_ws_create_window(void *ws, const M3WSWindowConfig *config,
                                   M3Handle *out_window) {
  struct M3WebBackend *backend;
  M3WebWindow *window;
  double ratio;
  int rc;

  if (ws == NULL || config == NULL || out_window == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_title == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->width <= 0 || config->height <= 0) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3WebBackend *)ws;
  out_window->id = 0u;
  out_window->generation = 0u;

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.create_window");
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3WebWindow),
                                (void **)&window);
  M3_WEB_RETURN_IF_ERROR(rc);

  memset(window, 0, sizeof(*window));
  window->backend = backend;
  window->width = config->width;
  window->height = config->height;
  window->flags = config->flags;
  window->visible = M3_FALSE;

  ratio = emscripten_get_device_pixel_ratio();
  if (ratio <= 0.0) {
    ratio = 1.0;
  }
  window->dpi_scale = (M3Scalar)ratio;

  rc = m3_object_header_init(&window->header, M3_WEB_TYPE_WINDOW, 0,
                             &g_m3_web_window_vtable);
  M3_WEB_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, window));

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &window->header);
  M3_WEB_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, window));

  rc = m3_web_backend_set_canvas_size(backend, window->width, window->height,
                                      window->dpi_scale);
  M3_WEB_RETURN_IF_ERROR_CLEANUP(rc, m3_object_release(&window->header));

  if (backend->event_window.id == 0u &&
      backend->event_window.generation == 0u) {
    backend->event_window = window->header.handle;
  }

#if defined(M3_WEBGPU_AVAILABLE)
  rc = m3_web_wgpu_init(backend);
  if (rc == M3_OK) {
    backend->use_webgpu = M3_TRUE;
  } else {
    backend->use_webgpu = M3_FALSE;
    rc = m3_web_backend_log(backend, M3_LOG_LEVEL_WARN, "webgpu.init_failed");
    M3_WEB_RETURN_IF_ERROR_CLEANUP(rc, m3_object_release(&window->header));
    rc = m3_web_gl_init(backend);
    M3_WEB_RETURN_IF_ERROR_CLEANUP(rc, m3_object_release(&window->header));
  }
#else
  rc = m3_web_gl_init(backend);
  M3_WEB_RETURN_IF_ERROR_CLEANUP(rc, m3_object_release(&window->header));
#endif

  *out_window = window->header.handle;
  return M3_OK;
}

static int m3_web_ws_destroy_window(void *ws, M3Handle window) {
  struct M3WebBackend *backend;
  M3WebWindow *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_resolve(backend, window, M3_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.destroy_window");
  M3_WEB_RETURN_IF_ERROR(rc);

  if (backend->event_window.id == window.id &&
      backend->event_window.generation == window.generation) {
    backend->event_window.id = 0u;
    backend->event_window.generation = 0u;
  }

  return m3_object_release(&resolved->header);
}

static int m3_web_ws_show_window(void *ws, M3Handle window) {
  struct M3WebBackend *backend;
  M3WebWindow *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_resolve(backend, window, M3_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.show_window");
  M3_WEB_RETURN_IF_ERROR(rc);

  resolved->visible = M3_TRUE;
  return M3_OK;
}

static int m3_web_ws_hide_window(void *ws, M3Handle window) {
  struct M3WebBackend *backend;
  M3WebWindow *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_resolve(backend, window, M3_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.hide_window");
  M3_WEB_RETURN_IF_ERROR(rc);

  resolved->visible = M3_FALSE;
  return M3_OK;
}

static int m3_web_ws_set_window_title(void *ws, M3Handle window,
                                      const char *utf8_title) {
  struct M3WebBackend *backend;
  int rc;

  if (ws == NULL || utf8_title == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_resolve(backend, window, M3_WEB_TYPE_WINDOW, NULL);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_title");
  M3_WEB_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static int m3_web_ws_set_window_size(void *ws, M3Handle window, m3_i32 width,
                                     m3_i32 height) {
  struct M3WebBackend *backend;
  M3WebWindow *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_resolve(backend, window, M3_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_size");
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_set_canvas_size(backend, width, height,
                                      resolved->dpi_scale);
  M3_WEB_RETURN_IF_ERROR(rc);

  resolved->width = width;
  resolved->height = height;
  return M3_OK;
}

static int m3_web_ws_get_window_size(void *ws, M3Handle window,
                                     m3_i32 *out_width, m3_i32 *out_height) {
  struct M3WebBackend *backend;
  M3WebWindow *resolved;
  int rc;

  if (ws == NULL || out_width == NULL || out_height == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_resolve(backend, window, M3_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_window_size");
  M3_WEB_RETURN_IF_ERROR(rc);

  *out_width = resolved->width;
  *out_height = resolved->height;
  return M3_OK;
}

static int m3_web_ws_set_window_dpi_scale(void *ws, M3Handle window,
                                          M3Scalar scale) {
  struct M3WebBackend *backend;
  M3WebWindow *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (scale <= 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_resolve(backend, window, M3_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc =
      m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_dpi_scale");
  M3_WEB_RETURN_IF_ERROR(rc);

  resolved->dpi_scale = scale;
  rc = m3_web_backend_set_canvas_size(backend, resolved->width,
                                      resolved->height, resolved->dpi_scale);
  M3_WEB_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static int m3_web_ws_get_window_dpi_scale(void *ws, M3Handle window,
                                          M3Scalar *out_scale) {
  struct M3WebBackend *backend;
  M3WebWindow *resolved;
  int rc;

  if (ws == NULL || out_scale == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_resolve(backend, window, M3_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc =
      m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_window_dpi_scale");
  M3_WEB_RETURN_IF_ERROR(rc);

  *out_scale = resolved->dpi_scale;
  return M3_OK;
}

static int m3_web_ws_set_clipboard_text(void *ws, const char *utf8_text) {
  struct M3WebBackend *backend;
  m3_usize length;
  m3_usize required;
  int rc;

  if (ws == NULL || utf8_text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_clipboard_text");
  M3_WEB_RETURN_IF_ERROR(rc);

  length = 0;
  while (utf8_text[length] != '\0') {
    length += 1;
  }
  if (length > backend->clipboard_limit) {
    return M3_ERR_RANGE;
  }

  required = length + 1;
  if (required > backend->clipboard_capacity) {
    void *mem = NULL;
    if (backend->clipboard == NULL) {
      rc = backend->allocator.alloc(backend->allocator.ctx, required, &mem);
    } else {
      rc = backend->allocator.realloc(backend->allocator.ctx,
                                      backend->clipboard, required, &mem);
    }
    M3_WEB_RETURN_IF_ERROR(rc);
    backend->clipboard = (char *)mem;
    backend->clipboard_capacity = required;
  }

  memcpy(backend->clipboard, utf8_text, required);
  backend->clipboard_length = length;
  return M3_OK;
}

static int m3_web_ws_get_clipboard_text(void *ws, char *buffer,
                                        m3_usize buffer_size,
                                        m3_usize *out_length) {
  struct M3WebBackend *backend;
  m3_usize required;
  int rc;

  if (ws == NULL || buffer == NULL || out_length == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_clipboard_text");
  M3_WEB_RETURN_IF_ERROR(rc);

  required = backend->clipboard_length + 1;
  if (buffer_size < required) {
    *out_length = backend->clipboard_length;
    return M3_ERR_RANGE;
  }

  if (backend->clipboard_length == 0) {
    buffer[0] = '\0';
  } else {
    memcpy(buffer, backend->clipboard, required);
  }
  *out_length = backend->clipboard_length;
  return M3_OK;
}

static int m3_web_ws_poll_event(void *ws, M3InputEvent *out_event,
                                M3Bool *out_has_event) {
  struct M3WebBackend *backend;
  int rc;

  if (ws == NULL || out_event == NULL || out_has_event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.poll_event");
  M3_WEB_RETURN_IF_ERROR(rc);

  return m3_web_event_queue_pop(&backend->events, out_event, out_has_event);
}

static int m3_web_ws_pump_events(void *ws) {
  struct M3WebBackend *backend;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.pump_events");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static int m3_web_ws_get_time_ms(void *ws, m3_u32 *out_time_ms) {
  struct M3WebBackend *backend;
  int rc;

  if (ws == NULL || out_time_ms == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)ws;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.get_time_ms");
  M3_WEB_RETURN_IF_ERROR(rc);

  return m3_web_backend_time_now_ms(out_time_ms);
}

static const M3WSVTable g_m3_web_ws_vtable = {m3_web_ws_init,
                                              m3_web_ws_shutdown,
                                              m3_web_ws_create_window,
                                              m3_web_ws_destroy_window,
                                              m3_web_ws_show_window,
                                              m3_web_ws_hide_window,
                                              m3_web_ws_set_window_title,
                                              m3_web_ws_set_window_size,
                                              m3_web_ws_get_window_size,
                                              m3_web_ws_set_window_dpi_scale,
                                              m3_web_ws_get_window_dpi_scale,
                                              m3_web_ws_set_clipboard_text,
                                              m3_web_ws_get_clipboard_text,
                                              m3_web_ws_poll_event,
                                              m3_web_ws_pump_events,
                                              m3_web_ws_get_time_ms};

static int m3_web_gfx_begin_frame(void *gfx, M3Handle window, m3_i32 width,
                                  m3_i32 height, M3Scalar dpi_scale) {
  struct M3WebBackend *backend;
  M3WebWindow *resolved;
  m3_i32 device_width;
  m3_i32 device_height;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (dpi_scale <= 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3WebBackend *)gfx;
  rc = m3_web_backend_resolve(backend, window, M3_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.begin_frame");
  M3_WEB_RETURN_IF_ERROR(rc);

#if defined(M3_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return m3_web_wgpu_begin_frame(backend, resolved, width, height, dpi_scale);
  }
#endif

  rc = m3_web_gl_require_context(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  if (resolved->width != width || resolved->height != height ||
      resolved->dpi_scale != dpi_scale) {
    resolved->width = width;
    resolved->height = height;
    resolved->dpi_scale = dpi_scale;
    rc = m3_web_backend_set_canvas_size(backend, width, height, dpi_scale);
    M3_WEB_RETURN_IF_ERROR(rc);
  }

  device_width = (m3_i32)((M3Scalar)width * dpi_scale + 0.5f);
  device_height = (m3_i32)((M3Scalar)height * dpi_scale + 0.5f);
  if (device_width <= 0 || device_height <= 0) {
    return M3_ERR_RANGE;
  }

  backend->viewport_width = width;
  backend->viewport_height = height;
  backend->device_width = device_width;
  backend->device_height = device_height;
  backend->active_window = resolved;
  backend->in_frame = M3_TRUE;

  glViewport(0, 0, device_width, device_height);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  rc = m3_web_gfx_apply_clip(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  return m3_web_gl_check_error(backend, "gfx.begin_frame");
}

static int m3_web_gfx_end_frame(void *gfx, M3Handle window) {
  struct M3WebBackend *backend;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)gfx;
  rc = m3_web_backend_resolve(backend, window, M3_WEB_TYPE_WINDOW, NULL);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.end_frame");
  M3_WEB_RETURN_IF_ERROR(rc);

  if (!backend->in_frame) {
    return M3_ERR_STATE;
  }

#if defined(M3_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return m3_web_wgpu_end_frame(backend);
  }
#endif

  glFlush();

  backend->active_window = NULL;
  backend->in_frame = M3_FALSE;
  return M3_OK;
}

static int m3_web_gfx_clear(void *gfx, M3Color color) {
  struct M3WebBackend *backend;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)gfx;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.clear");
  M3_WEB_RETURN_IF_ERROR(rc);

  if (!backend->in_frame) {
    return M3_ERR_STATE;
  }

  rc = m3_web_gl_require_context(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);

  return m3_web_gl_check_error(backend, "gfx.clear");
}

static int m3_web_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color,
                                M3Scalar corner_radius) {
  struct M3WebBackend *backend;
  M3WebVertex verts[4];
  M3Rect rect_copy;
  M3Scalar radius;
  M3Scalar max_radius;
  int rc;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3WebBackend *)gfx;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_rect");
  M3_WEB_RETURN_IF_ERROR(rc);

  rect_copy = *rect;
  max_radius =
      rect_copy.width < rect_copy.height ? rect_copy.width : rect_copy.height;
  max_radius = max_radius * 0.5f;
  radius = corner_radius;
  if (radius > max_radius) {
    radius = max_radius;
  }

  verts[0].x = rect_copy.x;
  verts[0].y = rect_copy.y;
  verts[0].u = 0.0f;
  verts[0].v = 0.0f;

  verts[1].x = rect_copy.x + rect_copy.width;
  verts[1].y = rect_copy.y;
  verts[1].u = 1.0f;
  verts[1].v = 0.0f;

  verts[2].x = rect_copy.x;
  verts[2].y = rect_copy.y + rect_copy.height;
  verts[2].u = 0.0f;
  verts[2].v = 1.0f;

  verts[3].x = rect_copy.x + rect_copy.width;
  verts[3].y = rect_copy.y + rect_copy.height;
  verts[3].u = 1.0f;
  verts[3].v = 1.0f;

#if defined(M3_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return m3_web_wgpu_draw_quad(backend, verts, color, M3_FALSE, NULL, 1.0f,
                                 &rect_copy, radius);
  }
#endif

  return m3_web_gfx_draw_quad(backend, verts, color, M3_FALSE, 0, 1.0f,
                              &rect_copy, radius);
}

static int m3_web_gfx_draw_line(void *gfx, M3Scalar x0, M3Scalar y0,
                                M3Scalar x1, M3Scalar y1, M3Color color,
                                M3Scalar thickness) {
  struct M3WebBackend *backend;
  M3WebVertex verts[4];
  M3Rect rect;
  M3Scalar dx;
  M3Scalar dy;
  M3Scalar len;
  M3Scalar nx;
  M3Scalar ny;
  M3Scalar half;
  M3Scalar min_x;
  M3Scalar max_x;
  M3Scalar min_y;
  M3Scalar max_y;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (thickness < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (thickness == 0.0f) {
    return M3_OK;
  }

  backend = (struct M3WebBackend *)gfx;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_line");
  M3_WEB_RETURN_IF_ERROR(rc);

  dx = x1 - x0;
  dy = y1 - y0;
  len = (M3Scalar)sqrt((double)(dx * dx + dy * dy));
  if (len <= 0.0f) {
    return M3_OK;
  }

  half = thickness * 0.5f;
  nx = -dy / len * half;
  ny = dx / len * half;

  verts[0].x = x0 + nx;
  verts[0].y = y0 + ny;
  verts[0].u = 0.0f;
  verts[0].v = 0.0f;

  verts[1].x = x1 + nx;
  verts[1].y = y1 + ny;
  verts[1].u = 1.0f;
  verts[1].v = 0.0f;

  verts[2].x = x0 - nx;
  verts[2].y = y0 - ny;
  verts[2].u = 0.0f;
  verts[2].v = 1.0f;

  verts[3].x = x1 - nx;
  verts[3].y = y1 - ny;
  verts[3].u = 1.0f;
  verts[3].v = 1.0f;

  min_x = verts[0].x;
  max_x = verts[0].x;
  min_y = verts[0].y;
  max_y = verts[0].y;

  if (verts[1].x < min_x) {
    min_x = verts[1].x;
  }
  if (verts[2].x < min_x) {
    min_x = verts[2].x;
  }
  if (verts[3].x < min_x) {
    min_x = verts[3].x;
  }
  if (verts[1].x > max_x) {
    max_x = verts[1].x;
  }
  if (verts[2].x > max_x) {
    max_x = verts[2].x;
  }
  if (verts[3].x > max_x) {
    max_x = verts[3].x;
  }

  if (verts[1].y < min_y) {
    min_y = verts[1].y;
  }
  if (verts[2].y < min_y) {
    min_y = verts[2].y;
  }
  if (verts[3].y < min_y) {
    min_y = verts[3].y;
  }
  if (verts[1].y > max_y) {
    max_y = verts[1].y;
  }
  if (verts[2].y > max_y) {
    max_y = verts[2].y;
  }
  if (verts[3].y > max_y) {
    max_y = verts[3].y;
  }

  rect.x = min_x;
  rect.y = min_y;
  rect.width = max_x - min_x;
  rect.height = max_y - min_y;

#if defined(M3_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return m3_web_wgpu_draw_quad(backend, verts, color, M3_FALSE, NULL, 1.0f,
                                 &rect, 0.0f);
  }
#endif

  return m3_web_gfx_draw_quad(backend, verts, color, M3_FALSE, 0, 1.0f, &rect,
                              0.0f);
}

static int m3_web_gfx_draw_path(void *gfx, const M3Path *path, M3Color color) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(color);

  if (gfx == NULL || path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (path->commands == NULL) {
    return M3_ERR_STATE;
  }
  if (path->count > path->capacity) {
    return M3_ERR_STATE;
  }
  if (path->count == 0) {
    return M3_OK;
  }

  backend = (struct M3WebBackend *)gfx;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_path");
  M3_WEB_RETURN_IF_ERROR(rc);

  if (!backend->in_frame) {
    return M3_ERR_STATE;
  }

  return M3_ERR_UNSUPPORTED;
}

static int m3_web_gfx_push_clip(void *gfx, const M3Rect *rect) {
  struct M3WebBackend *backend;
  M3Rect clipped;
  M3Bool has_intersection;
  int rc;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3WebBackend *)gfx;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.push_clip");
  M3_WEB_RETURN_IF_ERROR(rc);

  if (backend->clip_depth >= M3_WEB_CLIP_STACK_CAPACITY) {
    return M3_ERR_OVERFLOW;
  }

  if (backend->clip_depth == 0) {
    clipped = *rect;
  } else {
    rc = m3_rect_intersect(&backend->clip_stack[backend->clip_depth - 1u], rect,
                           &clipped, &has_intersection);
    M3_WEB_RETURN_IF_ERROR(rc);
    if (!has_intersection) {
      clipped.x = rect->x;
      clipped.y = rect->y;
      clipped.width = 0.0f;
      clipped.height = 0.0f;
    }
  }

  backend->clip_stack[backend->clip_depth] = clipped;
  backend->clip_depth += 1u;

  if (backend->in_frame) {
    rc = m3_web_gfx_apply_clip(backend);
    M3_WEB_RETURN_IF_ERROR(rc);
  }

  return M3_OK;
}

static int m3_web_gfx_pop_clip(void *gfx) {
  struct M3WebBackend *backend;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)gfx;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.pop_clip");
  M3_WEB_RETURN_IF_ERROR(rc);

  if (backend->clip_depth == 0) {
    return M3_ERR_STATE;
  }

  backend->clip_depth -= 1u;

  if (backend->in_frame) {
    rc = m3_web_gfx_apply_clip(backend);
    M3_WEB_RETURN_IF_ERROR(rc);
  }

  return M3_OK;
}

static int m3_web_gfx_set_transform(void *gfx, const M3Mat3 *transform) {
  struct M3WebBackend *backend;
  int rc;

  if (gfx == NULL || transform == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)gfx;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.set_transform");
  M3_WEB_RETURN_IF_ERROR(rc);

  backend->transform = *transform;
  backend->has_transform = M3_TRUE;
  return M3_OK;
}

static int m3_web_mul_size(m3_usize a, m3_usize b, m3_usize *out_value) {
  m3_usize max_value;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_value = (m3_usize) ~(m3_usize)0;
  if (a != 0 && b > max_value / a) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return M3_OK;
}

static int m3_web_convert_bgra(const M3Allocator *allocator, const void *pixels,
                               m3_usize size, void **out_pixels) {
  unsigned char *dst;
  const unsigned char *src;
  m3_usize i;
  int rc;

  if (allocator == NULL || out_pixels == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size == 0 || pixels == NULL) {
    *out_pixels = NULL;
    return M3_OK;
  }

  rc = allocator->alloc(allocator->ctx, size, (void **)&dst);
  M3_WEB_RETURN_IF_ERROR(rc);

  src = (const unsigned char *)pixels;
  i = 0;
  while (i + 3u < size) {
    dst[i + 0u] = src[i + 2u];
    dst[i + 1u] = src[i + 1u];
    dst[i + 2u] = src[i + 0u];
    dst[i + 3u] = src[i + 3u];
    i += 4u;
  }

  *out_pixels = dst;
  return M3_OK;
}

#if defined(M3_WEBGPU_AVAILABLE)
static int m3_web_wgpu_write_texture(struct M3WebBackend *backend,
                                     M3WebTexture *texture, m3_i32 x, m3_i32 y,
                                     m3_i32 width, m3_i32 height,
                                     const void *pixels, m3_usize size) {
  m3_usize row_bytes;
  m3_usize expected_size;
  m3_usize padded_row_bytes;
  m3_usize padded_size;
  unsigned char *staging;
  const unsigned char *src;
  m3_i32 row;
  WGPUTexelCopyTextureInfo dst;
  WGPUTexelCopyBufferLayout layout;
  WGPUExtent3D extent;
  int rc;

  if (backend == NULL || texture == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (pixels == NULL && size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (pixels == NULL && size == 0) {
    return M3_OK;
  }
  if (backend->wgpu.queue == NULL) {
    return M3_ERR_STATE;
  }

  rc = m3_web_mul_size((m3_usize)width, (m3_usize)texture->bytes_per_pixel,
                       &row_bytes);
  M3_WEB_RETURN_IF_ERROR(rc);
  rc = m3_web_mul_size(row_bytes, (m3_usize)height, &expected_size);
  M3_WEB_RETURN_IF_ERROR(rc);

  if (size != 0 && size < expected_size) {
    return M3_ERR_RANGE;
  }

  rc = m3_web_wgpu_align_size(row_bytes, (m3_usize)M3_WEB_WGPU_ROW_ALIGN,
                              &padded_row_bytes);
  M3_WEB_RETURN_IF_ERROR(rc);
  rc = m3_web_mul_size(padded_row_bytes, (m3_usize)height, &padded_size);
  M3_WEB_RETURN_IF_ERROR(rc);

  staging = (unsigned char *)pixels;
  if (padded_row_bytes != row_bytes) {
    rc = backend->allocator.alloc(backend->allocator.ctx, padded_size,
                                  (void **)&staging);
    M3_WEB_RETURN_IF_ERROR(rc);

    src = (const unsigned char *)pixels;
    for (row = 0; row < height; ++row) {
      memcpy(staging + (m3_usize)row * padded_row_bytes,
             src + (m3_usize)row * row_bytes, row_bytes);
      if (padded_row_bytes > row_bytes) {
        memset(staging + (m3_usize)row * padded_row_bytes + row_bytes, 0,
               padded_row_bytes - row_bytes);
      }
    }
  }

  memset(&dst, 0, sizeof(dst));
  dst.texture = texture->wgpu_texture;
  dst.mipLevel = 0u;
  dst.origin.x = (m3_u32)x;
  dst.origin.y = (m3_u32)y;
  dst.origin.z = 0u;
  dst.aspect = WGPUTextureAspect_All;

  memset(&layout, 0, sizeof(layout));
  layout.offset = 0u;
  layout.bytesPerRow = (m3_u32)padded_row_bytes;
  layout.rowsPerImage = (m3_u32)height;

  memset(&extent, 0, sizeof(extent));
  extent.width = (m3_u32)width;
  extent.height = (m3_u32)height;
  extent.depthOrArrayLayers = 1u;

  wgpuQueueWriteTexture(backend->wgpu.queue, &dst, staging, padded_size,
                        &layout, &extent);

  if (staging != pixels) {
    rc = backend->allocator.free(backend->allocator.ctx, staging);
    M3_WEB_RETURN_IF_ERROR(rc);
  }

  return M3_OK;
}

static int m3_web_wgpu_create_texture(struct M3WebBackend *backend,
                                      m3_i32 width, m3_i32 height,
                                      m3_u32 format, const void *pixels,
                                      m3_usize size, M3Handle *out_texture) {
  M3WebTexture *texture;
  WGPUTextureDescriptor texture_desc;
  WGPUTextureViewDescriptor view_desc;
  WGPUBindGroupEntry bind_entries[3];
  WGPUBindGroupDescriptor bind_desc;
  m3_usize expected_size;
  m3_usize row_size;
  m3_u32 bytes_per_pixel;
  WGPUTextureFormat wgpu_format;
  M3Bool alpha_only;
  int rc;

  if (backend == NULL || out_texture == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (format != M3_TEX_FORMAT_RGBA8 && format != M3_TEX_FORMAT_BGRA8 &&
      format != M3_TEX_FORMAT_A8) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (pixels == NULL && size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  out_texture->id = 0u;
  out_texture->generation = 0u;

  rc = m3_web_wgpu_init(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  if (backend->wgpu.device == NULL || backend->wgpu.bind_group_layout == NULL ||
      backend->wgpu.uniform_buffer == NULL || backend->wgpu.sampler == NULL) {
    return M3_ERR_STATE;
  }

  bytes_per_pixel = 4u;
  wgpu_format = WGPUTextureFormat_RGBA8Unorm;
  alpha_only = M3_FALSE;
  if (format == M3_TEX_FORMAT_BGRA8) {
    wgpu_format = WGPUTextureFormat_BGRA8Unorm;
  } else if (format == M3_TEX_FORMAT_A8) {
    bytes_per_pixel = 1u;
    wgpu_format = WGPUTextureFormat_R8Unorm;
    alpha_only = M3_TRUE;
  }

  rc = m3_web_mul_size((m3_usize)width, (m3_usize)height, &row_size);
  M3_WEB_RETURN_IF_ERROR(rc);
  rc = m3_web_mul_size(row_size, (m3_usize)bytes_per_pixel, &expected_size);
  M3_WEB_RETURN_IF_ERROR(rc);

  if (size != 0 && size < expected_size) {
    return M3_ERR_RANGE;
  }

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3WebTexture),
                                (void **)&texture);
  M3_WEB_RETURN_IF_ERROR(rc);

  memset(texture, 0, sizeof(*texture));
  texture->backend = backend;
  texture->width = width;
  texture->height = height;
  texture->format = format;
  texture->bytes_per_pixel = bytes_per_pixel;
  texture->wgpu_format = wgpu_format;
  texture->wgpu_alpha_only = alpha_only;

  memset(&texture_desc, 0, sizeof(texture_desc));
  texture_desc.dimension = WGPUTextureDimension_2D;
  texture_desc.size.width = (m3_u32)width;
  texture_desc.size.height = (m3_u32)height;
  texture_desc.size.depthOrArrayLayers = 1u;
  texture_desc.sampleCount = 1u;
  texture_desc.mipLevelCount = 1u;
  texture_desc.format = wgpu_format;
  texture_desc.usage =
      WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;

  texture->wgpu_texture =
      wgpuDeviceCreateTexture(backend->wgpu.device, &texture_desc);
  if (texture->wgpu_texture == NULL) {
    backend->allocator.free(backend->allocator.ctx, texture);
    return M3_ERR_UNKNOWN;
  }

  memset(&view_desc, 0, sizeof(view_desc));
  texture->wgpu_view = wgpuTextureCreateView(texture->wgpu_texture, &view_desc);
  if (texture->wgpu_view == NULL) {
    wgpuTextureRelease(texture->wgpu_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return M3_ERR_UNKNOWN;
  }

  memset(bind_entries, 0, sizeof(bind_entries));
  bind_entries[0].binding = 0u;
  bind_entries[0].buffer = backend->wgpu.uniform_buffer;
  bind_entries[0].offset = 0u;
  bind_entries[0].size = (m3_usize)sizeof(M3WebGPUUniforms);
  bind_entries[1].binding = 1u;
  bind_entries[1].sampler = backend->wgpu.sampler;
  bind_entries[2].binding = 2u;
  bind_entries[2].textureView = texture->wgpu_view;

  memset(&bind_desc, 0, sizeof(bind_desc));
  bind_desc.layout = backend->wgpu.bind_group_layout;
  bind_desc.entryCount = (m3_u32)M3_COUNTOF(bind_entries);
  bind_desc.entries = bind_entries;

  texture->wgpu_bind_group =
      wgpuDeviceCreateBindGroup(backend->wgpu.device, &bind_desc);
  if (texture->wgpu_bind_group == NULL) {
    wgpuTextureViewRelease(texture->wgpu_view);
    wgpuTextureRelease(texture->wgpu_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return M3_ERR_UNKNOWN;
  }

  if (pixels != NULL && size != 0) {
    rc = m3_web_wgpu_write_texture(backend, texture, 0, 0, width, height,
                                   pixels, size);
    if (rc != M3_OK) {
      wgpuBindGroupRelease(texture->wgpu_bind_group);
      wgpuTextureViewRelease(texture->wgpu_view);
      wgpuTextureRelease(texture->wgpu_texture);
      backend->allocator.free(backend->allocator.ctx, texture);
      return rc;
    }
  }

  rc = m3_object_header_init(&texture->header, M3_WEB_TYPE_TEXTURE, 0,
                             &g_m3_web_texture_vtable);
  if (rc != M3_OK) {
    wgpuBindGroupRelease(texture->wgpu_bind_group);
    wgpuTextureViewRelease(texture->wgpu_view);
    wgpuTextureRelease(texture->wgpu_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &texture->header);
  if (rc != M3_OK) {
    wgpuBindGroupRelease(texture->wgpu_bind_group);
    wgpuTextureViewRelease(texture->wgpu_view);
    wgpuTextureRelease(texture->wgpu_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  *out_texture = texture->header.handle;
  return M3_OK;
}

static int m3_web_wgpu_update_texture(struct M3WebBackend *backend,
                                      M3WebTexture *resolved, m3_i32 x,
                                      m3_i32 y, m3_i32 width, m3_i32 height,
                                      const void *pixels, m3_usize size) {
  int rc;

  if (backend == NULL || resolved == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (x < 0 || y < 0 || width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (pixels == NULL && size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (x + width > resolved->width || y + height > resolved->height) {
    return M3_ERR_RANGE;
  }
  if (pixels == NULL && size == 0) {
    return M3_OK;
  }

  rc = m3_web_wgpu_write_texture(backend, resolved, x, y, width, height, pixels,
                                 size);
  M3_WEB_RETURN_IF_ERROR(rc);

  return M3_OK;
}
#endif

static int m3_web_gfx_create_texture(void *gfx, m3_i32 width, m3_i32 height,
                                     m3_u32 format, const void *pixels,
                                     m3_usize size, M3Handle *out_texture) {
  struct M3WebBackend *backend;
  M3WebTexture *texture;
  GLenum gl_format;
  GLuint gl_texture;
  m3_usize expected_size;
  m3_usize row_size;
  m3_u32 bytes_per_pixel;
  void *upload_pixels;
  int rc;

  if (gfx == NULL || out_texture == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (format != M3_TEX_FORMAT_RGBA8 && format != M3_TEX_FORMAT_BGRA8 &&
      format != M3_TEX_FORMAT_A8) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (pixels == NULL && size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)gfx;
  out_texture->id = 0u;
  out_texture->generation = 0u;

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.create_texture");
  M3_WEB_RETURN_IF_ERROR(rc);

#if defined(M3_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return m3_web_wgpu_create_texture(backend, width, height, format, pixels,
                                      size, out_texture);
  }
#endif

  bytes_per_pixel = 4u;
  gl_format = GL_RGBA;
  if (format == M3_TEX_FORMAT_A8) {
    bytes_per_pixel = 1u;
    gl_format = GL_ALPHA;
  }

  rc = m3_web_mul_size((m3_usize)width, (m3_usize)height, &row_size);
  M3_WEB_RETURN_IF_ERROR(rc);
  rc = m3_web_mul_size(row_size, (m3_usize)bytes_per_pixel, &expected_size);
  M3_WEB_RETURN_IF_ERROR(rc);

  if (size != 0 && size < expected_size) {
    return M3_ERR_RANGE;
  }

  rc = m3_web_gl_require_context(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3WebTexture),
                                (void **)&texture);
  M3_WEB_RETURN_IF_ERROR(rc);

  memset(texture, 0, sizeof(*texture));
  texture->backend = backend;
  texture->width = width;
  texture->height = height;
  texture->format = format;
  texture->bytes_per_pixel = bytes_per_pixel;

  glGenTextures(1, &gl_texture);
  if (gl_texture == 0) {
    backend->allocator.free(backend->allocator.ctx, texture);
    return M3_ERR_UNKNOWN;
  }

  glBindTexture(GL_TEXTURE_2D, gl_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  upload_pixels = (void *)pixels;
  if (format == M3_TEX_FORMAT_BGRA8 && pixels != NULL && size != 0) {
    rc = m3_web_convert_bgra(&backend->allocator, pixels, expected_size,
                             &upload_pixels);
    if (rc != M3_OK) {
      glDeleteTextures(1, &gl_texture);
      backend->allocator.free(backend->allocator.ctx, texture);
      return rc;
    }
  }

  glTexImage2D(GL_TEXTURE_2D, 0, gl_format, width, height, 0, gl_format,
               GL_UNSIGNED_BYTE, upload_pixels);

  if (upload_pixels != pixels && upload_pixels != NULL) {
    backend->allocator.free(backend->allocator.ctx, upload_pixels);
  }

  rc = m3_web_gl_check_error(backend, "gfx.create_texture");
  if (rc != M3_OK) {
    glDeleteTextures(1, &gl_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  texture->gl_texture = gl_texture;

  rc = m3_object_header_init(&texture->header, M3_WEB_TYPE_TEXTURE, 0,
                             &g_m3_web_texture_vtable);
  if (rc != M3_OK) {
    glDeleteTextures(1, &gl_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &texture->header);
  if (rc != M3_OK) {
    glDeleteTextures(1, &gl_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  *out_texture = texture->header.handle;
  return M3_OK;
}

static int m3_web_gfx_update_texture(void *gfx, M3Handle texture, m3_i32 x,
                                     m3_i32 y, m3_i32 width, m3_i32 height,
                                     const void *pixels, m3_usize size) {
  struct M3WebBackend *backend;
  M3WebTexture *resolved;
  m3_usize expected_size;
  m3_usize row_size;
  void *upload_pixels;
  GLenum gl_format;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (x < 0 || y < 0 || width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (pixels == NULL && size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)gfx;
  rc = m3_web_backend_resolve(backend, texture, M3_WEB_TYPE_TEXTURE,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  if (x + width > resolved->width || y + height > resolved->height) {
    return M3_ERR_RANGE;
  }

  if (pixels == NULL && size == 0) {
    return M3_OK;
  }

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.update_texture");
  M3_WEB_RETURN_IF_ERROR(rc);

#if defined(M3_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return m3_web_wgpu_update_texture(backend, resolved, x, y, width, height,
                                      pixels, size);
  }
#endif

  if (resolved->gl_texture == 0) {
    return M3_ERR_STATE;
  }

  rc = m3_web_mul_size((m3_usize)width, (m3_usize)height, &row_size);
  M3_WEB_RETURN_IF_ERROR(rc);
  rc = m3_web_mul_size(row_size, (m3_usize)resolved->bytes_per_pixel,
                       &expected_size);
  M3_WEB_RETURN_IF_ERROR(rc);

  if (size != 0 && size < expected_size) {
    return M3_ERR_RANGE;
  }

  rc = m3_web_gl_require_context(backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  gl_format = GL_RGBA;
  if (resolved->format == M3_TEX_FORMAT_A8) {
    gl_format = GL_ALPHA;
  }

  upload_pixels = (void *)pixels;
  if (resolved->format == M3_TEX_FORMAT_BGRA8 && pixels != NULL) {
    rc = m3_web_convert_bgra(&backend->allocator, pixels, expected_size,
                             &upload_pixels);
    M3_WEB_RETURN_IF_ERROR(rc);
  }

  glBindTexture(GL_TEXTURE_2D, resolved->gl_texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, gl_format,
                  GL_UNSIGNED_BYTE, upload_pixels);

  if (upload_pixels != pixels && upload_pixels != NULL) {
    backend->allocator.free(backend->allocator.ctx, upload_pixels);
  }

  return m3_web_gl_check_error(backend, "gfx.update_texture");
}

static int m3_web_gfx_destroy_texture(void *gfx, M3Handle texture) {
  struct M3WebBackend *backend;
  M3WebTexture *resolved;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)gfx;
  rc = m3_web_backend_resolve(backend, texture, M3_WEB_TYPE_TEXTURE,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.destroy_texture");
  M3_WEB_RETURN_IF_ERROR(rc);

  return m3_object_release(&resolved->header);
}

static int m3_web_gfx_draw_texture(void *gfx, M3Handle texture,
                                   const M3Rect *src, const M3Rect *dst,
                                   M3Scalar opacity) {
  struct M3WebBackend *backend;
  M3WebTexture *resolved;
  M3WebVertex verts[4];
  M3Rect rect_copy;
  M3Color color;
  M3Scalar inv_w;
  M3Scalar inv_h;
  M3Scalar u0;
  M3Scalar v0;
  M3Scalar u1;
  M3Scalar v1;
  int rc;

  if (gfx == NULL || src == NULL || dst == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (opacity < 0.0f || opacity > 1.0f) {
    return M3_ERR_RANGE;
  }
  if (dst->width < 0.0f || dst->height < 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3WebBackend *)gfx;
  rc = m3_web_backend_resolve(backend, texture, M3_WEB_TYPE_TEXTURE,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  if (src->x < 0.0f || src->y < 0.0f || src->width < 0.0f ||
      src->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (src->x + src->width > (M3Scalar)resolved->width ||
      src->y + src->height > (M3Scalar)resolved->height) {
    return M3_ERR_RANGE;
  }

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_texture");
  M3_WEB_RETURN_IF_ERROR(rc);

  inv_w = 1.0f / (M3Scalar)resolved->width;
  inv_h = 1.0f / (M3Scalar)resolved->height;
  u0 = src->x * inv_w;
  v0 = src->y * inv_h;
  u1 = (src->x + src->width) * inv_w;
  v1 = (src->y + src->height) * inv_h;

  rect_copy = *dst;

  verts[0].x = rect_copy.x;
  verts[0].y = rect_copy.y;
  verts[0].u = u0;
  verts[0].v = v0;

  verts[1].x = rect_copy.x + rect_copy.width;
  verts[1].y = rect_copy.y;
  verts[1].u = u1;
  verts[1].v = v0;

  verts[2].x = rect_copy.x;
  verts[2].y = rect_copy.y + rect_copy.height;
  verts[2].u = u0;
  verts[2].v = v1;

  verts[3].x = rect_copy.x + rect_copy.width;
  verts[3].y = rect_copy.y + rect_copy.height;
  verts[3].u = u1;
  verts[3].v = v1;

  color.r = 1.0f;
  color.g = 1.0f;
  color.b = 1.0f;
  color.a = 1.0f;

#if defined(M3_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return m3_web_wgpu_draw_quad(backend, verts, color, M3_TRUE, resolved,
                                 opacity, &rect_copy, 0.0f);
  }
#endif

  return m3_web_gfx_draw_quad(backend, verts, color, M3_TRUE,
                              resolved->gl_texture, opacity, &rect_copy, 0.0f);
}

static const M3GfxVTable g_m3_web_gfx_vtable = {
    m3_web_gfx_begin_frame,    m3_web_gfx_end_frame,
    m3_web_gfx_clear,          m3_web_gfx_draw_rect,
    m3_web_gfx_draw_line,      m3_web_gfx_draw_path,
    m3_web_gfx_push_clip,      m3_web_gfx_pop_clip,
    m3_web_gfx_set_transform,  m3_web_gfx_create_texture,
    m3_web_gfx_update_texture, m3_web_gfx_destroy_texture,
    m3_web_gfx_draw_texture};

static int m3_web_text_create_font(void *text, const char *utf8_family,
                                   m3_i32 size_px, m3_i32 weight, M3Bool italic,
                                   M3Handle *out_font) {
  struct M3WebBackend *backend;
  M3WebFont *font;
  int rc;

  if (text == NULL || out_font == NULL || utf8_family == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size_px <= 0) {
    return M3_ERR_RANGE;
  }
  if (weight < 100 || weight > 900) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3WebBackend *)text;
  out_font->id = 0u;
  out_font->generation = 0u;

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.create_font");
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3WebFont),
                                (void **)&font);
  M3_WEB_RETURN_IF_ERROR(rc);

  memset(font, 0, sizeof(*font));
  font->backend = backend;
  font->size_px = size_px;
  font->weight = weight;
  font->italic = italic ? M3_TRUE : M3_FALSE;

  rc = m3_object_header_init(&font->header, M3_WEB_TYPE_FONT, 0,
                             &g_m3_web_font_vtable);
  M3_WEB_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, font));

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &font->header);
  M3_WEB_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, font));

  *out_font = font->header.handle;
  return M3_OK;
}

static int m3_web_text_destroy_font(void *text, M3Handle font) {
  struct M3WebBackend *backend;
  M3WebFont *resolved;
  int rc;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)text;
  rc = m3_web_backend_resolve(backend, font, M3_WEB_TYPE_FONT,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.destroy_font");
  M3_WEB_RETURN_IF_ERROR(rc);

  return m3_object_release(&resolved->header);
}

static int m3_web_text_measure_text(void *text, M3Handle font, const char *utf8,
                                    m3_usize utf8_len, M3Scalar *out_width,
                                    M3Scalar *out_height,
                                    M3Scalar *out_baseline) {
  struct M3WebBackend *backend;
  M3WebFont *resolved;
  M3Scalar size;
  int rc;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)text;
  rc = m3_web_backend_resolve(backend, font, M3_WEB_TYPE_FONT,
                              (void **)&resolved);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.measure_text");
  M3_WEB_RETURN_IF_ERROR(rc);

  size = (M3Scalar)resolved->size_px;
  *out_width = size * (M3Scalar)utf8_len * 0.5f;
  *out_height = size;
  *out_baseline = size * 0.8f;
  return M3_OK;
}

static int m3_web_text_draw_text(void *text, M3Handle font, const char *utf8,
                                 m3_usize utf8_len, M3Scalar x, M3Scalar y,
                                 M3Color color) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(x);
  M3_UNUSED(y);
  M3_UNUSED(color);

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)text;
  rc = m3_web_backend_resolve(backend, font, M3_WEB_TYPE_FONT, NULL);
  M3_WEB_RETURN_IF_ERROR(rc);

  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.draw_text");
  M3_WEB_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static const M3TextVTable g_m3_web_text_vtable = {
    m3_web_text_create_font, m3_web_text_destroy_font, m3_web_text_measure_text,
    m3_web_text_draw_text};

static int m3_web_io_read_file(void *io, const char *utf8_path, void *buffer,
                               m3_usize buffer_size, m3_usize *out_read) {
  struct M3WebBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_read == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (buffer == NULL && buffer_size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)io;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.read_file");
  M3_WEB_RETURN_IF_ERROR(rc);

  *out_read = 0;
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_io_read_file_alloc(void *io, const char *utf8_path,
                                     const M3Allocator *allocator,
                                     void **out_data, m3_usize *out_size) {
  struct M3WebBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || allocator == NULL ||
      out_data == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)io;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.read_file_alloc");
  M3_WEB_RETURN_IF_ERROR(rc);

  *out_data = NULL;
  *out_size = 0;
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_io_write_file(void *io, const char *utf8_path,
                                const void *data, m3_usize size,
                                M3Bool overwrite) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(overwrite);

  if (io == NULL || utf8_path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (data == NULL && size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)io;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.write_file");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_io_file_exists(void *io, const char *utf8_path,
                                 M3Bool *out_exists) {
  struct M3WebBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_exists == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)io;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.file_exists");
  M3_WEB_RETURN_IF_ERROR(rc);

  *out_exists = M3_FALSE;
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_io_delete_file(void *io, const char *utf8_path) {
  struct M3WebBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)io;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.delete_file");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_io_stat_file(void *io, const char *utf8_path,
                               M3FileInfo *out_info) {
  struct M3WebBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_info == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)io;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.stat_file");
  M3_WEB_RETURN_IF_ERROR(rc);

  memset(out_info, 0, sizeof(*out_info));
  return M3_ERR_UNSUPPORTED;
}

static const M3IOVTable g_m3_web_io_vtable = {
    m3_web_io_read_file,   m3_web_io_read_file_alloc, m3_web_io_write_file,
    m3_web_io_file_exists, m3_web_io_delete_file,     m3_web_io_stat_file};

static int m3_web_sensors_is_available(void *sensors, m3_u32 type,
                                       M3Bool *out_available) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(type);

  if (sensors == NULL || out_available == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)sensors;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.is_available");
  M3_WEB_RETURN_IF_ERROR(rc);

  *out_available = M3_FALSE;
  return M3_OK;
}

static int m3_web_sensors_start(void *sensors, m3_u32 type) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(type);

  if (sensors == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)sensors;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.start");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_sensors_stop(void *sensors, m3_u32 type) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(type);

  if (sensors == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)sensors;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.stop");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_sensors_read(void *sensors, m3_u32 type,
                               M3SensorReading *out_reading,
                               M3Bool *out_has_reading) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(type);

  if (sensors == NULL || out_reading == NULL || out_has_reading == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)sensors;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.read");
  M3_WEB_RETURN_IF_ERROR(rc);

  memset(out_reading, 0, sizeof(*out_reading));
  *out_has_reading = M3_FALSE;
  return M3_OK;
}

static const M3SensorsVTable g_m3_web_sensors_vtable = {
    m3_web_sensors_is_available, m3_web_sensors_start, m3_web_sensors_stop,
    m3_web_sensors_read};

static int m3_web_camera_open(void *camera, m3_u32 camera_id) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(camera_id);

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)camera;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.open");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_camera_open_with_config(void *camera,
                                          const M3CameraConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return m3_web_camera_open(camera, config->camera_id);
}

static int m3_web_camera_close(void *camera) {
  struct M3WebBackend *backend;
  int rc;

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)camera;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.close");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_camera_start(void *camera) {
  struct M3WebBackend *backend;
  int rc;

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)camera;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.start");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_camera_stop(void *camera) {
  struct M3WebBackend *backend;
  int rc;

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)camera;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.stop");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_camera_read_frame(void *camera, M3CameraFrame *out_frame,
                                    M3Bool *out_has_frame) {
  struct M3WebBackend *backend;
  int rc;

  if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)camera;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.read_frame");
  M3_WEB_RETURN_IF_ERROR(rc);

  memset(out_frame, 0, sizeof(*out_frame));
  *out_has_frame = M3_FALSE;
  return M3_OK;
}

static const M3CameraVTable g_m3_web_camera_vtable = {
    m3_web_camera_open,  m3_web_camera_open_with_config,
    m3_web_camera_close, m3_web_camera_start,
    m3_web_camera_stop,  m3_web_camera_read_frame};

static int m3_web_network_request(void *net, const M3NetworkRequest *request,
                                  const M3Allocator *allocator,
                                  M3NetworkResponse *out_response) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(request);
  M3_UNUSED(allocator);

  if (net == NULL || out_response == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)net;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.request");
  M3_WEB_RETURN_IF_ERROR(rc);

  memset(out_response, 0, sizeof(*out_response));
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_network_free_response(void *net, const M3Allocator *allocator,
                                        M3NetworkResponse *response) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(allocator);
  M3_UNUSED(response);

  if (net == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)net;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.free_response");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static const M3NetworkVTable g_m3_web_network_vtable = {
    m3_web_network_request, m3_web_network_free_response};

static int m3_web_tasks_thread_create(void *tasks, M3ThreadFn entry, void *user,
                                      M3Handle *out_thread) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(entry);
  M3_UNUSED(user);

  if (tasks == NULL || out_thread == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)tasks;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.thread_create");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_tasks_thread_join(void *tasks, M3Handle thread) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(thread);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)tasks;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.thread_join");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_tasks_mutex_create(void *tasks, M3Handle *out_mutex) {
  struct M3WebBackend *backend;
  int rc;

  if (tasks == NULL || out_mutex == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)tasks;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_create");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_tasks_mutex_destroy(void *tasks, M3Handle mutex) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(mutex);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)tasks;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_destroy");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_tasks_mutex_lock(void *tasks, M3Handle mutex) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(mutex);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)tasks;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_lock");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_tasks_mutex_unlock(void *tasks, M3Handle mutex) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(mutex);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)tasks;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_unlock");
  M3_WEB_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_web_tasks_sleep_ms(void *tasks, m3_u32 ms) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(ms);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)tasks;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.sleep_ms");
  M3_WEB_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return M3_ERR_UNSUPPORTED;
  }
  return M3_OK;
}

static int m3_web_tasks_post(void *tasks, M3TaskFn fn, void *user) {
  struct M3WebBackend *backend;
  int rc;

  if (tasks == NULL || fn == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)tasks;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.post");
  M3_WEB_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return M3_ERR_UNSUPPORTED;
  }
  return fn(user);
}

static int m3_web_tasks_post_delayed(void *tasks, M3TaskFn fn, void *user,
                                     m3_u32 delay_ms) {
  struct M3WebBackend *backend;
  int rc;

  M3_UNUSED(delay_ms);

  if (tasks == NULL || fn == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)tasks;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.post_delayed");
  M3_WEB_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return M3_ERR_UNSUPPORTED;
  }
  return fn(user);
}

static const M3TasksVTable g_m3_web_tasks_vtable = {
    m3_web_tasks_thread_create, m3_web_tasks_thread_join,
    m3_web_tasks_mutex_create,  m3_web_tasks_mutex_destroy,
    m3_web_tasks_mutex_lock,    m3_web_tasks_mutex_unlock,
    m3_web_tasks_sleep_ms,      m3_web_tasks_post,
    m3_web_tasks_post_delayed};

static int m3_web_env_get_io(void *env, M3IO *out_io) {
  struct M3WebBackend *backend;
  int rc;

  if (env == NULL || out_io == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)env;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_io");
  M3_WEB_RETURN_IF_ERROR(rc);

  *out_io = backend->io;
  return M3_OK;
}

static int m3_web_env_get_sensors(void *env, M3Sensors *out_sensors) {
  struct M3WebBackend *backend;
  int rc;

  if (env == NULL || out_sensors == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)env;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_sensors");
  M3_WEB_RETURN_IF_ERROR(rc);

  *out_sensors = backend->sensors;
  return M3_OK;
}

static int m3_web_env_get_camera(void *env, M3Camera *out_camera) {
  struct M3WebBackend *backend;
  int rc;

  if (env == NULL || out_camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)env;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_camera");
  M3_WEB_RETURN_IF_ERROR(rc);

  *out_camera = backend->camera;
  return M3_OK;
}

static int m3_web_env_get_network(void *env, M3Network *out_network) {
  struct M3WebBackend *backend;
  int rc;

  if (env == NULL || out_network == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)env;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_network");
  M3_WEB_RETURN_IF_ERROR(rc);

  *out_network = backend->network;
  return M3_OK;
}

static int m3_web_env_get_tasks(void *env, M3Tasks *out_tasks) {
  struct M3WebBackend *backend;
  int rc;

  if (env == NULL || out_tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)env;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_tasks");
  M3_WEB_RETURN_IF_ERROR(rc);

  *out_tasks = backend->tasks;
  return M3_OK;
}

static int m3_web_env_get_time_ms(void *env, m3_u32 *out_time_ms) {
  struct M3WebBackend *backend;
  int rc;

  if (env == NULL || out_time_ms == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3WebBackend *)env;
  rc = m3_web_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_time_ms");
  M3_WEB_RETURN_IF_ERROR(rc);

  return m3_web_backend_time_now_ms(out_time_ms);
}

static const M3EnvVTable g_m3_web_env_vtable = {
    m3_web_env_get_io,      m3_web_env_get_sensors, m3_web_env_get_camera,
    m3_web_env_get_network, m3_web_env_get_tasks,   m3_web_env_get_time_ms};

int M3_CALL m3_web_backend_create(const M3WebBackendConfig *config,
                                  M3WebBackend **out_backend) {
  M3WebBackendConfig local_config;
  M3Allocator allocator;
  struct M3WebBackend *backend;
  int rc;

  if (out_backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = m3_web_backend_config_init(&local_config);
    M3_WEB_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = m3_web_backend_validate_config(config);
  M3_WEB_RETURN_IF_ERROR(rc);

  if (config->allocator == NULL) {
    rc = m3_get_default_allocator(&allocator);
    M3_WEB_RETURN_IF_ERROR(rc);
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = allocator.alloc(allocator.ctx, sizeof(M3WebBackend), (void **)&backend);
  M3_WEB_RETURN_IF_ERROR(rc);

  memset(backend, 0, sizeof(*backend));
  backend->allocator = allocator;
  backend->log_enabled = config->enable_logging ? M3_TRUE : M3_FALSE;
  backend->inline_tasks = config->inline_tasks ? M3_TRUE : M3_FALSE;
  backend->clipboard_limit = config->clipboard_limit;

  rc = m3_web_backend_copy_string(&allocator, config->utf8_canvas_id,
                                  &backend->canvas_id);
  if (rc != M3_OK) {
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  if (backend->log_enabled) {
    rc = m3_log_init(&allocator);
    if (rc != M3_OK && rc != M3_ERR_STATE) {
      allocator.free(allocator.ctx, backend->canvas_id);
      allocator.free(allocator.ctx, backend);
      return rc;
    }
    backend->log_owner = (rc == M3_OK) ? M3_TRUE : M3_FALSE;
  }

  rc = m3_handle_system_default_create(config->handle_capacity, &allocator,
                                       &backend->handles);
  if (rc != M3_OK) {
    if (backend->log_owner) {
      m3_log_shutdown();
    }
    allocator.free(allocator.ctx, backend->canvas_id);
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  backend->ws.ctx = backend;
  backend->ws.vtable = &g_m3_web_ws_vtable;
  backend->gfx.ctx = backend;
  backend->gfx.vtable = &g_m3_web_gfx_vtable;
  backend->gfx.text_vtable = &g_m3_web_text_vtable;
  backend->env.ctx = backend;
  backend->env.vtable = &g_m3_web_env_vtable;
  backend->io.ctx = backend;
  backend->io.vtable = &g_m3_web_io_vtable;
  backend->sensors.ctx = backend;
  backend->sensors.vtable = &g_m3_web_sensors_vtable;
  backend->camera.ctx = backend;
  backend->camera.vtable = &g_m3_web_camera_vtable;
  backend->network.ctx = backend;
  backend->network.vtable = &g_m3_web_network_vtable;
  backend->tasks.ctx = backend;
  backend->tasks.vtable = &g_m3_web_tasks_vtable;
  backend->initialized = M3_TRUE;

  *out_backend = backend;
  return M3_OK;
}

int M3_CALL m3_web_backend_destroy(M3WebBackend *backend) {
  int rc;
  int first_error;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }

  rc = m3_handle_system_default_destroy(&backend->handles);
  M3_WEB_RETURN_IF_ERROR(rc);

  first_error = M3_OK;

  if (backend->clipboard != NULL) {
    rc = backend->allocator.free(backend->allocator.ctx, backend->clipboard);
    if (rc != M3_OK && first_error == M3_OK) {
      first_error = rc;
    }
    backend->clipboard = NULL;
    backend->clipboard_capacity = 0;
    backend->clipboard_length = 0;
  }

#if defined(M3_WEBGPU_AVAILABLE)
  rc = m3_web_wgpu_shutdown(backend);
  if (rc != M3_OK && first_error == M3_OK) {
    first_error = rc;
  }
  backend->use_webgpu = M3_FALSE;
#endif

  if (backend->gl.initialized) {
    rc = m3_web_gl_require_context(backend);
    if (rc != M3_OK && first_error == M3_OK) {
      first_error = rc;
    }
    if (backend->gl.vbo != 0) {
      glDeleteBuffers(1, &backend->gl.vbo);
      backend->gl.vbo = 0;
    }
    if (backend->gl.program != 0) {
      glDeleteProgram(backend->gl.program);
      backend->gl.program = 0;
    }
    if (backend->gl.context > 0) {
      emscripten_webgl_destroy_context(backend->gl.context);
      backend->gl.context = 0;
    }
    backend->gl.initialized = M3_FALSE;
  }

  if (backend->canvas_id != NULL) {
    rc = backend->allocator.free(backend->allocator.ctx, backend->canvas_id);
    if (rc != M3_OK && first_error == M3_OK) {
      first_error = rc;
    }
    backend->canvas_id = NULL;
  }

  if (backend->log_owner) {
    rc = m3_log_shutdown();
    if (rc != M3_OK && first_error == M3_OK) {
      first_error = rc;
    }
  }

  backend->initialized = M3_FALSE;
  backend->ws.ctx = NULL;
  backend->ws.vtable = NULL;
  backend->gfx.ctx = NULL;
  backend->gfx.vtable = NULL;
  backend->gfx.text_vtable = NULL;
  backend->env.ctx = NULL;
  backend->env.vtable = NULL;
  backend->io.ctx = NULL;
  backend->io.vtable = NULL;
  backend->sensors.ctx = NULL;
  backend->sensors.vtable = NULL;
  backend->camera.ctx = NULL;
  backend->camera.vtable = NULL;
  backend->network.ctx = NULL;
  backend->network.vtable = NULL;
  backend->tasks.ctx = NULL;
  backend->tasks.vtable = NULL;

  rc = backend->allocator.free(backend->allocator.ctx, backend);
  if (rc != M3_OK && first_error == M3_OK) {
    first_error = rc;
  }

  return first_error;
}

int M3_CALL m3_web_backend_get_ws(M3WebBackend *backend, M3WS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }
  *out_ws = backend->ws;
  return M3_OK;
}

int M3_CALL m3_web_backend_get_gfx(M3WebBackend *backend, M3Gfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }
  *out_gfx = backend->gfx;
  return M3_OK;
}

int M3_CALL m3_web_backend_get_env(M3WebBackend *backend, M3Env *out_env) {
  if (backend == NULL || out_env == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }
  *out_env = backend->env;
  return M3_OK;
}

#else

int M3_CALL m3_web_backend_create(const M3WebBackendConfig *config,
                                  M3WebBackend **out_backend) {
  M3WebBackendConfig local_config;
  int rc;

  if (out_backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = m3_web_backend_config_init(&local_config);
    M3_WEB_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = m3_web_backend_validate_config(config);
  M3_WEB_RETURN_IF_ERROR(rc);

  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_web_backend_destroy(M3WebBackend *backend) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_web_backend_get_ws(M3WebBackend *backend, M3WS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(out_ws, 0, sizeof(*out_ws));
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_web_backend_get_gfx(M3WebBackend *backend, M3Gfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(out_gfx, 0, sizeof(*out_gfx));
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_web_backend_get_env(M3WebBackend *backend, M3Env *out_env) {
  if (backend == NULL || out_env == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(out_env, 0, sizeof(*out_env));
  return M3_ERR_UNSUPPORTED;
}

#endif
