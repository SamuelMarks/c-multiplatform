#include "cmpc/cmp_backend_web.h"

#include "cmpc/cmp_log.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_object.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#if defined(CMP_WEB_AVAILABLE)
#include <GLES2/gl2.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#if defined(CMP_WEBGPU_AVAILABLE)
#include <emscripten/html5_webgpu.h>
#include <webgpu/webgpu.h>
#endif
#endif

#define CMP_WEB_RETURN_IF_ERROR(rc)                                             \
  do {                                                                         \
    if ((rc) != CMP_OK) {                                                       \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define CMP_WEB_RETURN_IF_ERROR_CLEANUP(rc, cleanup)                            \
  do {                                                                         \
    if ((rc) != CMP_OK) {                                                       \
      cleanup;                                                                 \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define CMP_WEB_DEFAULT_HANDLE_CAPACITY 64u
#define CMP_WEB_DEFAULT_CANVAS_ID "#canvas"

static int cmp_web_backend_validate_config(const CMPWebBackendConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->handle_capacity == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_canvas_id == NULL || config->utf8_canvas_id[0] == '\0') {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->allocator != NULL) {
    if (config->allocator->alloc == NULL ||
        config->allocator->realloc == NULL || config->allocator->free == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL
cmp_web_backend_test_validate_config(const CMPWebBackendConfig *config) {
  return cmp_web_backend_validate_config(config);
}
#endif

int CMP_CALL cmp_web_backend_is_available(CMPBool *out_available) {
  if (out_available == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_WEB_AVAILABLE)
  *out_available = CMP_TRUE;
#else
  *out_available = CMP_FALSE;
#endif
  return CMP_OK;
}

int CMP_CALL cmp_web_backend_config_init(CMPWebBackendConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->handle_capacity = CMP_WEB_DEFAULT_HANDLE_CAPACITY;
  config->clipboard_limit = (cmp_usize) ~(cmp_usize)0;
  config->utf8_canvas_id = CMP_WEB_DEFAULT_CANVAS_ID;
  config->enable_logging = CMP_TRUE;
  config->inline_tasks = CMP_TRUE;
  return CMP_OK;
}

/* GCOVR_EXCL_START */
#if defined(CMP_WEB_AVAILABLE)

#define CMP_WEB_TYPE_WINDOW 1
#define CMP_WEB_TYPE_TEXTURE 2
#define CMP_WEB_TYPE_FONT 3

#define CMP_WEB_EVENT_CAPACITY 256u
#define CMP_WEB_CLIP_STACK_CAPACITY 32u

typedef struct CMPWebWindow {
  CMPObjectHeader header;
  struct CMPWebBackend *backend;
  cmp_i32 width;
  cmp_i32 height;
  CMPScalar dpi_scale;
  cmp_u32 flags;
  CMPBool visible;
} CMPWebWindow;

typedef struct CMPWebTexture {
  CMPObjectHeader header;
  struct CMPWebBackend *backend;
  cmp_i32 width;
  cmp_i32 height;
  cmp_u32 format;
  cmp_u32 bytes_per_pixel;
  GLuint gl_texture;
#if defined(CMP_WEBGPU_AVAILABLE)
  WGPUTexture wgpu_texture;
  WGPUTextureView wgpu_view;
  WGPUBindGroup wgpu_bind_group;
  WGPUTextureFormat wgpu_format;
  CMPBool wgpu_alpha_only;
#endif
} CMPWebTexture;

typedef struct CMPWebFont {
  CMPObjectHeader header;
  struct CMPWebBackend *backend;
  cmp_i32 size_px;
  cmp_i32 weight;
  CMPBool italic;
} CMPWebFont;

typedef struct CMPWebEventQueue {
  CMPInputEvent events[CMP_WEB_EVENT_CAPACITY];
  cmp_usize head;
  cmp_usize count;
} CMPWebEventQueue;

typedef struct CMPWebGLState {
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
  CMPBool initialized;
} CMPWebGLState;

#if defined(CMP_WEBGPU_AVAILABLE)
typedef struct CMPWebGPUUniforms {
  float transform[16];
  float viewport[4];
  float rect[4];
  float color[4];
  float params[4];
} CMPWebGPUUniforms;

typedef struct CMPWebGPUState {
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
  cmp_u32 scissor_x;
  cmp_u32 scissor_y;
  cmp_u32 scissor_w;
  cmp_u32 scissor_h;
  cmp_i32 config_width;
  cmp_i32 config_height;
  CMPBool scissor_dirty;
  CMPBool configured;
  CMPBool pass_active;
  CMPBool initialized;
} CMPWebGPUState;
#endif

typedef struct CMPWebVertex {
  float x;
  float y;
  float u;
  float v;
} CMPWebVertex;

struct CMPWebBackend {
  CMPAllocator allocator;
  CMPHandleSystem handles;
  CMPWS ws;
  CMPGfx gfx;
  CMPEnv env;
  CMPIO io;
  CMPSensors sensors;
  CMPCamera camera;
  CMPNetwork network;
  CMPTasks tasks;
  CMPBool initialized;
  CMPBool log_enabled;
  CMPBool log_owner;
  CMPBool inline_tasks;
  cmp_u32 time_ms_fallback;
  char *clipboard;
  cmp_usize clipboard_capacity;
  cmp_usize clipboard_length;
  cmp_usize clipboard_limit;
  char *canvas_id;
  CMPWebWindow *active_window;
  CMPHandle event_window;
  CMPWebEventQueue events;
  CMPRect clip_stack[CMP_WEB_CLIP_STACK_CAPACITY];
  cmp_usize clip_depth;
  CMPMat3 transform;
  CMPBool has_transform;
  cmp_i32 viewport_width;
  cmp_i32 viewport_height;
  cmp_i32 device_width;
  cmp_i32 device_height;
  CMPBool callbacks_registered;
  CMPBool in_frame;
  CMPWebGLState gl;
#if defined(CMP_WEBGPU_AVAILABLE)
  CMPBool use_webgpu;
  CMPWebGPUState wgpu;
#endif
};

static int cmp_web_backend_log(struct CMPWebBackend *backend, CMPLogLevel level,
                              const char *message) {
  if (backend == NULL || message == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->log_enabled) {
    return CMP_OK;
  }
  return cmp_log_write(level, "m3.web", message);
}

static int cmp_web_backend_resolve(struct CMPWebBackend *backend, CMPHandle handle,
                                  cmp_u32 type_id, void **out_obj) {
  void *resolved;
  cmp_u32 actual_type;
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (out_obj != NULL) {
    *out_obj = NULL;
  }

  rc =
      backend->handles.vtable->resolve(backend->handles.ctx, handle, &resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_object_get_type_id((const CMPObjectHeader *)resolved, &actual_type);
  CMP_WEB_RETURN_IF_ERROR(rc);
  if (actual_type != type_id) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (out_obj != NULL) {
    *out_obj = resolved;
  }
  return CMP_OK;
}

static int cmp_web_backend_copy_string(const CMPAllocator *allocator,
                                      const char *src, char **out_dst) {
  cmp_usize length;
  cmp_usize max_value;
  char *mem;
  int rc;

  if (allocator == NULL || src == NULL || out_dst == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  length = 0;
  while (src[length] != '\0') {
    length += 1;
  }

  max_value = (cmp_usize) ~(cmp_usize)0;
  if (length >= max_value) {
    return CMP_ERR_OVERFLOW;
  }

  rc = allocator->alloc(allocator->ctx, length + 1, (void **)&mem);
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (length > 0) {
    memcpy(mem, src, length);
  }
  mem[length] = '\0';
  *out_dst = mem;
  return CMP_OK;
}

static int cmp_web_backend_map_result(int result) {
  switch (result) {
  case EMSCRIPTEN_RESULT_SUCCESS:
    return CMP_OK;
  case EMSCRIPTEN_RESULT_NOT_SUPPORTED:
    return CMP_ERR_UNSUPPORTED;
  case EMSCRIPTEN_RESULT_INVALID_PARAM:
    return CMP_ERR_INVALID_ARGUMENT;
  case EMSCRIPTEN_RESULT_INVALID_TARGET:
  case EMSCRIPTEN_RESULT_UNKNOWN_TARGET:
    return CMP_ERR_NOT_FOUND;
  case EMSCRIPTEN_RESULT_FAILED:
  case EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED:
  default:
    return CMP_ERR_UNKNOWN;
  }
}

static int cmp_web_backend_time_now_ms(cmp_u32 *out_time_ms) {
  double now;
  double max_value;

  if (out_time_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  now = emscripten_get_now();
  if (now < 0.0) {
    return CMP_ERR_UNKNOWN;
  }

  max_value = (double)((cmp_u32) ~(cmp_u32)0);
  if (now > max_value) {
    return CMP_ERR_OVERFLOW;
  }

  *out_time_ms = (cmp_u32)now;
  return CMP_OK;
}

static int cmp_web_double_to_i32(double value, cmp_i32 *out_value) {
  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (value > (double)INT_MAX) {
    return CMP_ERR_RANGE;
  }
  if (value < (double)INT_MIN) {
    return CMP_ERR_RANGE;
  }
  *out_value = (cmp_i32)value;
  return CMP_OK;
}

static int cmp_web_event_queue_push(CMPWebEventQueue *queue,
                                   const CMPInputEvent *event) {
  cmp_usize index;

  if (queue == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (queue->count >= CMP_WEB_EVENT_CAPACITY) {
    return CMP_ERR_BUSY;
  }

  index = (queue->head + queue->count) % CMP_WEB_EVENT_CAPACITY;
  queue->events[index] = *event;
  queue->count += 1;
  return CMP_OK;
}

static int cmp_web_event_queue_pop(CMPWebEventQueue *queue,
                                  CMPInputEvent *out_event,
                                  CMPBool *out_has_event) {
  cmp_usize index;

  if (queue == NULL || out_event == NULL || out_has_event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (queue->count == 0) {
    *out_has_event = CMP_FALSE;
    memset(out_event, 0, sizeof(*out_event));
    return CMP_OK;
  }

  index = queue->head;
  *out_event = queue->events[index];
  queue->head = (queue->head + 1) % CMP_WEB_EVENT_CAPACITY;
  queue->count -= 1;
  *out_has_event = CMP_TRUE;
  return CMP_OK;
}

static int cmp_web_build_modifiers(cmp_u32 *out_modifiers, int shift, int ctrl,
                                  int alt, int meta) {
  cmp_u32 mods;

  if (out_modifiers == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  mods = 0u;
  if (shift) {
    mods |= CMP_MOD_SHIFT;
  }
  if (ctrl) {
    mods |= CMP_MOD_CTRL;
  }
  if (alt) {
    mods |= CMP_MOD_ALT;
  }
  if (meta) {
    mods |= CMP_MOD_META;
  }

  *out_modifiers = mods;
  return CMP_OK;
}

static int cmp_web_event_stamp(struct CMPWebBackend *backend,
                              CMPInputEvent *event) {
  int rc;

  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_web_backend_time_now_ms(&event->time_ms);
  if (rc != CMP_OK) {
    backend->time_ms_fallback += 16u;
    event->time_ms = backend->time_ms_fallback;
    cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "events.time_fallback");
  }
  return rc;
}

static int cmp_web_gl_check_error(struct CMPWebBackend *backend,
                                 const char *label) {
  GLenum err;
  int rc;

  if (backend == NULL || label == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  err = glGetError();
  if (err == GL_NO_ERROR) {
    return CMP_OK;
  }

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_ERROR, label);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_ERR_UNKNOWN;
}

static int cmp_web_gl_compile_shader(struct CMPWebBackend *backend, GLenum type,
                                    const char *source, GLuint *out_shader) {
  GLuint shader;
  GLint status;
  char info[256];
  GLsizei info_len;
  int rc;

  if (backend == NULL || source == NULL || out_shader == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  shader = glCreateShader(type);
  if (shader == 0) {
    return CMP_ERR_UNKNOWN;
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
      cmp_web_backend_log(backend, CMP_LOG_LEVEL_ERROR, info);
    }
    glDeleteShader(shader);
    return CMP_ERR_UNKNOWN;
  }

  rc = cmp_web_gl_check_error(backend, "gl.compile_shader");
  if (rc != CMP_OK) {
    glDeleteShader(shader);
    return rc;
  }

  *out_shader = shader;
  return CMP_OK;
}

static int cmp_web_gl_link_program(struct CMPWebBackend *backend, GLuint vert,
                                  GLuint frag, GLuint *out_program) {
  GLuint program;
  GLint status;
  char info[256];
  GLsizei info_len;
  int rc;

  if (backend == NULL || out_program == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  program = glCreateProgram();
  if (program == 0) {
    return CMP_ERR_UNKNOWN;
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
      cmp_web_backend_log(backend, CMP_LOG_LEVEL_ERROR, info);
    }
    glDeleteProgram(program);
    return CMP_ERR_UNKNOWN;
  }

  rc = cmp_web_gl_check_error(backend, "gl.link_program");
  if (rc != CMP_OK) {
    glDeleteProgram(program);
    return rc;
  }

  *out_program = program;
  return CMP_OK;
}

static int cmp_web_gl_init(struct CMPWebBackend *backend) {
  EmscriptenWebGLContextAttributes attrs;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
  const char *vert_src;
  const char *frag_src;
  GLuint vert_shader;
  GLuint frag_shader;
  GLuint program;
  CMPWebGLState *gl;
  int result;
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  gl = &backend->gl;
  if (gl->initialized) {
    return CMP_OK;
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
    return CMP_ERR_UNKNOWN;
  }

  result = emscripten_webgl_make_context_current(ctx);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    emscripten_webgl_destroy_context(ctx);
    return cmp_web_backend_map_result(result);
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

  rc = cmp_web_gl_compile_shader(backend, GL_VERTEX_SHADER, vert_src,
                                &vert_shader);
  if (rc != CMP_OK) {
    emscripten_webgl_destroy_context(ctx);
    return rc;
  }

  rc = cmp_web_gl_compile_shader(backend, GL_FRAGMENT_SHADER, frag_src,
                                &frag_shader);
  if (rc != CMP_OK) {
    glDeleteShader(vert_shader);
    emscripten_webgl_destroy_context(ctx);
    return rc;
  }

  rc = cmp_web_gl_link_program(backend, vert_shader, frag_shader, &program);
  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);
  if (rc != CMP_OK) {
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
    return CMP_ERR_UNKNOWN;
  }

  glGenBuffers(1, &gl->vbo);
  if (gl->vbo == 0) {
    glDeleteProgram(program);
    emscripten_webgl_destroy_context(ctx);
    memset(gl, 0, sizeof(*gl));
    return CMP_ERR_UNKNOWN;
  }

  glBindBuffer(GL_ARRAY_BUFFER, gl->vbo);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(CMPWebVertex) * 4u), NULL,
               GL_DYNAMIC_DRAW);

  rc = cmp_web_gl_check_error(backend, "gl.init");
  if (rc != CMP_OK) {
    glDeleteBuffers(1, &gl->vbo);
    glDeleteProgram(program);
    emscripten_webgl_destroy_context(ctx);
    memset(gl, 0, sizeof(*gl));
    return rc;
  }

  gl->initialized = CMP_TRUE;
  return CMP_OK;
}

static int cmp_web_gl_require_context(struct CMPWebBackend *backend) {
  int result;
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_web_gl_init(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  result = emscripten_webgl_make_context_current(backend->gl.context);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }

  return CMP_OK;
}

static int cmp_web_gl_prepare(struct CMPWebBackend *backend) {
  CMPMat3 transform;
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->in_frame) {
    return CMP_ERR_STATE;
  }

#if defined(CMP_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return cmp_web_wgpu_clear(backend, color);
  }
#endif

  rc = cmp_web_gl_require_context(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  glUseProgram(backend->gl.program);
  glBindBuffer(GL_ARRAY_BUFFER, backend->gl.vbo);
  glEnableVertexAttribArray((GLuint)backend->gl.attr_pos);
  glEnableVertexAttribArray((GLuint)backend->gl.attr_uv);
  glVertexAttribPointer((GLuint)backend->gl.attr_pos, 2, GL_FLOAT, GL_FALSE,
                        (GLsizei)sizeof(CMPWebVertex), (void *)0);
  glVertexAttribPointer((GLuint)backend->gl.attr_uv, 2, GL_FLOAT, GL_FALSE,
                        (GLsizei)sizeof(CMPWebVertex),
                        (void *)(sizeof(float) * 2u));

  if (backend->has_transform) {
    transform = backend->transform;
  } else {
    rc = cmp_mat3_identity(&transform);
    CMP_WEB_RETURN_IF_ERROR(rc);
  }

  glUniformMatrix3fv(backend->gl.uni_transform, 1, GL_FALSE, transform.m);
  glUniform2f(backend->gl.uni_viewport, (float)backend->viewport_width,
              (float)backend->viewport_height);

  return cmp_web_gl_check_error(backend, "gl.prepare");
}

#if defined(CMP_WEBGPU_AVAILABLE)
#define CMP_WEB_WGPU_ROW_ALIGN 256u

static const char *g_cmp_web_wgpu_shader =
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

static int cmp_web_wgpu_align_size(cmp_usize value, cmp_usize alignment,
                                  cmp_usize *out_value) {
  cmp_usize remainder;
  cmp_usize add;
  cmp_usize max_value;

  if (out_value == NULL || alignment == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  remainder = value % alignment;
  if (remainder == 0u) {
    *out_value = value;
    return CMP_OK;
  }

  add = alignment - remainder;
  max_value = (cmp_usize) ~(cmp_usize)0;
  if (value > max_value - add) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = value + add;
  return CMP_OK;
}

static int cmp_web_wgpu_fill_transform(const CMPMat3 *transform, float *out_mat) {
  if (transform == NULL || out_mat == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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
  return CMP_OK;
}

static int cmp_web_wgpu_shutdown(struct CMPWebBackend *backend) {
  CMPWebGPUState *wgpu;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  wgpu = &backend->wgpu;

  if (wgpu->pass != NULL) {
    if (wgpu->pass_active) {
      wgpuRenderPassEncoderEnd(wgpu->pass);
    }
    wgpuRenderPassEncoderRelease(wgpu->pass);
    wgpu->pass = NULL;
  }
  wgpu->pass_active = CMP_FALSE;

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
  wgpu->configured = CMP_FALSE;
  wgpu->initialized = CMP_FALSE;
  wgpu->scissor_dirty = CMP_FALSE;
  wgpu->config_width = 0;
  wgpu->config_height = 0;
  memset(&wgpu->clear_color, 0, sizeof(wgpu->clear_color));
  wgpu->scissor_x = 0u;
  wgpu->scissor_y = 0u;
  wgpu->scissor_w = 0u;
  wgpu->scissor_h = 0u;

  return CMP_OK;
}

static int cmp_web_wgpu_init(struct CMPWebBackend *backend) {
  CMPWebGPUState *wgpu;
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
  unsigned char white_row[CMP_WEB_WGPU_ROW_ALIGN];
  WGPUDevice device;
  WGPUQueue queue;
  WGPUInstance instance;
  WGPUSurface surface;
  WGPUTextureFormat format;
  WGPUShaderModule shader;
  cmp_usize i;
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  wgpu = &backend->wgpu;
  if (wgpu->initialized) {
    return CMP_OK;
  }

  rc = cmp_web_wgpu_shutdown(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  device = emscripten_webgpu_get_device();
  if (device == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }
  queue = wgpuDeviceGetQueue(device);
  if (queue == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  memset(&instance_desc, 0, sizeof(instance_desc));
  instance = wgpuCreateInstance(&instance_desc);
  if (instance == NULL) {
    return CMP_ERR_UNSUPPORTED;
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
    return CMP_ERR_UNSUPPORTED;
  }

  format = emscripten_webgpu_get_swap_chain_preferred_format(device);
  if (format == WGPUTextureFormat_Undefined) {
    wgpuSurfaceRelease(surface);
    wgpuInstanceRelease(instance);
    return CMP_ERR_UNSUPPORTED;
  }

  wgpu->instance = instance;
  wgpu->device = device;
  wgpu->queue = queue;
  wgpu->surface = surface;
  wgpu->surface_format = format;

  memset(&wgsl_desc, 0, sizeof(wgsl_desc));
  wgsl_desc.chain.sType = WGPUSType_ShaderSourceWGSL;
  wgsl_desc.chain.next = NULL;
  wgsl_desc.code.data = g_cmp_web_wgpu_shader;
  wgsl_desc.code.length = WGPU_STRLEN;

  memset(&shader_desc, 0, sizeof(shader_desc));
  shader_desc.nextInChain = (const WGPUChainedStruct *)&wgsl_desc;

  shader = wgpuDeviceCreateShaderModule(device, &shader_desc);
  if (shader == NULL) {
    cmp_web_wgpu_shutdown(backend);
    return CMP_ERR_UNKNOWN;
  }

  memset(layout_entries, 0, sizeof(layout_entries));
  layout_entries[0].binding = 0;
  layout_entries[0].visibility =
      WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
  layout_entries[0].buffer.type = WGPUBufferBindingType_Uniform;
  layout_entries[0].buffer.minBindingSize = sizeof(CMPWebGPUUniforms);

  layout_entries[1].binding = 1;
  layout_entries[1].visibility = WGPUShaderStage_Fragment;
  layout_entries[1].sampler.type = WGPUSamplerBindingType_Filtering;

  layout_entries[2].binding = 2;
  layout_entries[2].visibility = WGPUShaderStage_Fragment;
  layout_entries[2].texture.sampleType = WGPUTextureSampleType_Float;
  layout_entries[2].texture.viewDimension = WGPUTextureViewDimension_2D;
  layout_entries[2].texture.multisampled = 0;

  memset(&layout_desc, 0, sizeof(layout_desc));
  layout_desc.entryCount = (cmp_u32)CMP_COUNTOF(layout_entries);
  layout_desc.entries = layout_entries;

  wgpu->bind_group_layout =
      wgpuDeviceCreateBindGroupLayout(device, &layout_desc);
  if (wgpu->bind_group_layout == NULL) {
    wgpuShaderModuleRelease(shader);
    cmp_web_wgpu_shutdown(backend);
    return CMP_ERR_UNKNOWN;
  }

  memset(&pipeline_layout_desc, 0, sizeof(pipeline_layout_desc));
  pipeline_layout_desc.bindGroupLayoutCount = 1u;
  pipeline_layout_desc.bindGroupLayouts = &wgpu->bind_group_layout;

  pipeline_layout =
      wgpuDeviceCreatePipelineLayout(device, &pipeline_layout_desc);
  if (pipeline_layout == NULL) {
    wgpuShaderModuleRelease(shader);
    cmp_web_wgpu_shutdown(backend);
    return CMP_ERR_UNKNOWN;
  }

  memset(attrs, 0, sizeof(attrs));
  attrs[0].shaderLocation = 0;
  attrs[0].format = WGPUVertexFormat_Float32x2;
  attrs[0].offset = 0;
  attrs[1].shaderLocation = 1;
  attrs[1].format = WGPUVertexFormat_Float32x2;
  attrs[1].offset = (cmp_usize)(sizeof(float) * 2u);

  memset(&vb_layout, 0, sizeof(vb_layout));
  vb_layout.arrayStride = (cmp_usize)sizeof(CMPWebVertex);
  vb_layout.stepMode = WGPUVertexStepMode_Vertex;
  vb_layout.attributeCount = (cmp_u32)CMP_COUNTOF(attrs);
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
    cmp_web_wgpu_shutdown(backend);
    return CMP_ERR_UNKNOWN;
  }

  memset(&buffer_desc, 0, sizeof(buffer_desc));
  buffer_desc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
  buffer_desc.size = (cmp_usize)sizeof(CMPWebGPUUniforms);

  wgpu->uniform_buffer = wgpuDeviceCreateBuffer(device, &buffer_desc);
  if (wgpu->uniform_buffer == NULL) {
    cmp_web_wgpu_shutdown(backend);
    return CMP_ERR_UNKNOWN;
  }

  memset(&buffer_desc, 0, sizeof(buffer_desc));
  buffer_desc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
  buffer_desc.size = (cmp_usize)(sizeof(CMPWebVertex) * 4u);

  wgpu->vertex_buffer = wgpuDeviceCreateBuffer(device, &buffer_desc);
  if (wgpu->vertex_buffer == NULL) {
    cmp_web_wgpu_shutdown(backend);
    return CMP_ERR_UNKNOWN;
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
    cmp_web_wgpu_shutdown(backend);
    return CMP_ERR_UNKNOWN;
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
    cmp_web_wgpu_shutdown(backend);
    return CMP_ERR_UNKNOWN;
  }

  memset(&view_desc, 0, sizeof(view_desc));
  wgpu->default_view = wgpuTextureCreateView(wgpu->default_texture, &view_desc);
  if (wgpu->default_view == NULL) {
    cmp_web_wgpu_shutdown(backend);
    return CMP_ERR_UNKNOWN;
  }

  for (i = 0; i < (cmp_usize)CMP_WEB_WGPU_ROW_ALIGN; ++i) {
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
  data_layout.bytesPerRow = (cmp_u32)CMP_WEB_WGPU_ROW_ALIGN;
  data_layout.rowsPerImage = 1u;

  memset(&extent, 0, sizeof(extent));
  extent.width = 1u;
  extent.height = 1u;
  extent.depthOrArrayLayers = 1u;

  wgpuQueueWriteTexture(queue, &copy_dst, white_row,
                        (cmp_usize)CMP_WEB_WGPU_ROW_ALIGN, &data_layout, &extent);

  memset(bind_entries, 0, sizeof(bind_entries));
  bind_entries[0].binding = 0u;
  bind_entries[0].buffer = wgpu->uniform_buffer;
  bind_entries[0].offset = 0u;
  bind_entries[0].size = (cmp_usize)sizeof(CMPWebGPUUniforms);
  bind_entries[1].binding = 1u;
  bind_entries[1].sampler = wgpu->sampler;
  bind_entries[2].binding = 2u;
  bind_entries[2].textureView = wgpu->default_view;

  memset(&bind_desc, 0, sizeof(bind_desc));
  bind_desc.layout = wgpu->bind_group_layout;
  bind_desc.entryCount = (cmp_u32)CMP_COUNTOF(bind_entries);
  bind_desc.entries = bind_entries;

  wgpu->default_bind_group = wgpuDeviceCreateBindGroup(device, &bind_desc);
  if (wgpu->default_bind_group == NULL) {
    cmp_web_wgpu_shutdown(backend);
    return CMP_ERR_UNKNOWN;
  }

  wgpu->initialized = CMP_TRUE;
  wgpu->configured = CMP_FALSE;
  wgpu->pass_active = CMP_FALSE;
  wgpu->scissor_dirty = CMP_TRUE;
  wgpu->config_width = 0;
  wgpu->config_height = 0;
  memset(&wgpu->clear_color, 0, sizeof(wgpu->clear_color));

  return CMP_OK;
}

static int cmp_web_wgpu_configure_surface(struct CMPWebBackend *backend,
                                         cmp_i32 width, cmp_i32 height) {
  CMPWebGPUState *wgpu;
  WGPUSurfaceConfiguration config;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }

  wgpu = &backend->wgpu;
  if (!wgpu->initialized || wgpu->surface == NULL || wgpu->device == NULL) {
    return CMP_ERR_STATE;
  }

  if (wgpu->configured && wgpu->config_width == width &&
      wgpu->config_height == height) {
    return CMP_OK;
  }

  memset(&config, 0, sizeof(config));
  config.device = wgpu->device;
  config.format = wgpu->surface_format;
  config.usage = WGPUTextureUsage_RenderAttachment;
  config.width = (cmp_u32)width;
  config.height = (cmp_u32)height;
  config.presentMode = WGPUPresentMode_Fifo;
  config.alphaMode = WGPUCompositeAlphaMode_Auto;

  wgpuSurfaceConfigure(wgpu->surface, &config);
  wgpu->configured = CMP_TRUE;
  wgpu->config_width = width;
  wgpu->config_height = height;
  return CMP_OK;
}

static int cmp_web_wgpu_acquire_frame(struct CMPWebBackend *backend) {
  CMPWebGPUState *wgpu;
  WGPUCommandEncoderDescriptor encoder_desc;
  WGPUSurfaceTexture surface_texture;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  wgpu = &backend->wgpu;
  if (!wgpu->configured || wgpu->surface == NULL || wgpu->device == NULL) {
    return CMP_ERR_STATE;
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
    return CMP_ERR_UNKNOWN;
  }
  if (surface_texture.status !=
          WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
      surface_texture.status !=
          WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
    wgpuTextureRelease(surface_texture.texture);
    return CMP_ERR_UNKNOWN;
  }

  wgpu->surface_texture = surface_texture.texture;

  wgpu->surface_view = wgpuTextureCreateView(wgpu->surface_texture, NULL);
  if (wgpu->surface_view == NULL) {
    wgpuTextureRelease(wgpu->surface_texture);
    wgpu->surface_texture = NULL;
    return CMP_ERR_UNKNOWN;
  }

  memset(&encoder_desc, 0, sizeof(encoder_desc));
  wgpu->encoder = wgpuDeviceCreateCommandEncoder(wgpu->device, &encoder_desc);
  if (wgpu->encoder == NULL) {
    wgpuTextureViewRelease(wgpu->surface_view);
    wgpuTextureRelease(wgpu->surface_texture);
    wgpu->surface_view = NULL;
    wgpu->surface_texture = NULL;
    return CMP_ERR_UNKNOWN;
  }

  wgpu->pass_active = CMP_FALSE;
  wgpu->scissor_dirty = CMP_TRUE;
  return CMP_OK;
}

static int cmp_web_wgpu_begin_pass(struct CMPWebBackend *backend, CMPBool clear) {
  CMPWebGPUState *wgpu;
  WGPURenderPassColorAttachment color_attachment;
  WGPURenderPassDescriptor pass_desc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->in_frame) {
    return CMP_ERR_STATE;
  }

  wgpu = &backend->wgpu;
  if (wgpu->encoder == NULL || wgpu->surface_view == NULL) {
    return CMP_ERR_STATE;
  }

  if (wgpu->pass_active) {
    if (!clear) {
      return CMP_OK;
    }
    wgpuRenderPassEncoderEnd(wgpu->pass);
    wgpuRenderPassEncoderRelease(wgpu->pass);
    wgpu->pass = NULL;
    wgpu->pass_active = CMP_FALSE;
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
    return CMP_ERR_UNKNOWN;
  }

  wgpu->pass_active = CMP_TRUE;
  wgpu->scissor_dirty = CMP_TRUE;
  return CMP_OK;
}

static int cmp_web_wgpu_apply_clip(struct CMPWebBackend *backend) {
  CMPRect rect;
  CMPScalar scale;
  cmp_i32 x;
  cmp_i32 y;
  cmp_i32 w;
  cmp_i32 h;
  cmp_i32 max_w;
  cmp_i32 max_h;
  CMPWebGPUState *wgpu;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->in_frame) {
    return CMP_ERR_STATE;
  }

  wgpu = &backend->wgpu;

  max_w = backend->device_width;
  max_h = backend->device_height;
  if (max_w <= 0 || max_h <= 0) {
    return CMP_ERR_STATE;
  }

  if (backend->clip_depth == 0) {
    x = 0;
    y = 0;
    w = max_w;
    h = max_h;
  } else {
    if (backend->active_window == NULL) {
      return CMP_ERR_STATE;
    }
    rect = backend->clip_stack[backend->clip_depth - 1u];
    if (rect.width < 0.0f || rect.height < 0.0f) {
      return CMP_ERR_RANGE;
    }

    scale = backend->active_window->dpi_scale;
    if (scale <= 0.0f) {
      scale = 1.0f;
    }

    x = (cmp_i32)(rect.x * scale);
    y = (cmp_i32)(rect.y * scale);
    w = (cmp_i32)(rect.width * scale);
    h = (cmp_i32)(rect.height * scale);

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

  wgpu->scissor_x = (cmp_u32)x;
  wgpu->scissor_y = (cmp_u32)y;
  wgpu->scissor_w = (cmp_u32)w;
  wgpu->scissor_h = (cmp_u32)h;
  wgpu->scissor_dirty = CMP_TRUE;

  if (wgpu->pass_active && wgpu->pass != NULL) {
    wgpuRenderPassEncoderSetScissorRect(wgpu->pass, wgpu->scissor_x,
                                        wgpu->scissor_y, wgpu->scissor_w,
                                        wgpu->scissor_h);
    wgpu->scissor_dirty = CMP_FALSE;
  }

  return CMP_OK;
}

static int cmp_web_wgpu_prepare_uniforms(struct CMPWebBackend *backend,
                                        const CMPRect *rect, CMPColor color,
                                        CMPScalar opacity, CMPScalar radius,
                                        CMPBool use_texture, CMPBool alpha_only,
                                        CMPWebGPUUniforms *out_uniforms) {
  CMPMat3 transform;
  int rc;

  if (backend == NULL || rect == NULL || out_uniforms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (backend->viewport_width <= 0 || backend->viewport_height <= 0) {
    return CMP_ERR_STATE;
  }

  if (backend->has_transform) {
    transform = backend->transform;
  } else {
    rc = cmp_mat3_identity(&transform);
    CMP_WEB_RETURN_IF_ERROR(rc);
  }

  rc = cmp_web_wgpu_fill_transform(&transform, out_uniforms->transform);
  CMP_WEB_RETURN_IF_ERROR(rc);

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

  return CMP_OK;
}

static int cmp_web_wgpu_draw_quad(struct CMPWebBackend *backend,
                                 const CMPWebVertex *verts, CMPColor color,
                                 CMPBool use_texture,
                                 const CMPWebTexture *texture, CMPScalar opacity,
                                 const CMPRect *rect, CMPScalar radius) {
  CMPWebGPUState *wgpu;
  CMPWebGPUUniforms uniforms;
  WGPUBindGroup bind_group;
  int rc;

  if (backend == NULL || verts == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (use_texture && texture == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->in_frame) {
    return CMP_ERR_STATE;
  }

  wgpu = &backend->wgpu;
  if (!wgpu->initialized || wgpu->queue == NULL ||
      wgpu->uniform_buffer == NULL || wgpu->vertex_buffer == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_web_wgpu_begin_pass(backend, CMP_FALSE);
  CMP_WEB_RETURN_IF_ERROR(rc);
  if (wgpu->pass == NULL || wgpu->pipeline == NULL) {
    return CMP_ERR_STATE;
  }

  bind_group =
      use_texture ? texture->wgpu_bind_group : wgpu->default_bind_group;
  if (bind_group == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_web_wgpu_prepare_uniforms(
      backend, rect, color, opacity, radius, use_texture,
      use_texture ? texture->wgpu_alpha_only : CMP_FALSE, &uniforms);
  CMP_WEB_RETURN_IF_ERROR(rc);

  wgpuQueueWriteBuffer(wgpu->queue, wgpu->uniform_buffer, 0u, &uniforms,
                       (cmp_usize)sizeof(uniforms));
  wgpuQueueWriteBuffer(wgpu->queue, wgpu->vertex_buffer, 0u, verts,
                       (cmp_usize)(sizeof(CMPWebVertex) * 4u));

  if (wgpu->scissor_dirty) {
    rc = cmp_web_wgpu_apply_clip(backend);
    CMP_WEB_RETURN_IF_ERROR(rc);
  }

  wgpuRenderPassEncoderSetPipeline(wgpu->pass, wgpu->pipeline);
  wgpuRenderPassEncoderSetBindGroup(wgpu->pass, 0u, bind_group, 0u, NULL);
  wgpuRenderPassEncoderSetVertexBuffer(wgpu->pass, 0u, wgpu->vertex_buffer, 0u,
                                       (cmp_usize)(sizeof(CMPWebVertex) * 4u));
  wgpuRenderPassEncoderDraw(wgpu->pass, 4u, 1u, 0u, 0u);

  return CMP_OK;
}

static int cmp_web_wgpu_begin_frame(struct CMPWebBackend *backend,
                                   CMPWebWindow *resolved, cmp_i32 width,
                                   cmp_i32 height, CMPScalar dpi_scale) {
  cmp_i32 device_width;
  cmp_i32 device_height;
  int rc;

  if (backend == NULL || resolved == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (dpi_scale <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_web_wgpu_init(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (resolved->width != width || resolved->height != height ||
      resolved->dpi_scale != dpi_scale) {
    resolved->width = width;
    resolved->height = height;
    resolved->dpi_scale = dpi_scale;
    rc = cmp_web_backend_set_canvas_size(backend, width, height, dpi_scale);
    CMP_WEB_RETURN_IF_ERROR(rc);
  }

  device_width = (cmp_i32)((CMPScalar)width * dpi_scale + 0.5f);
  device_height = (cmp_i32)((CMPScalar)height * dpi_scale + 0.5f);
  if (device_width <= 0 || device_height <= 0) {
    return CMP_ERR_RANGE;
  }

  backend->viewport_width = width;
  backend->viewport_height = height;
  backend->device_width = device_width;
  backend->device_height = device_height;
  backend->active_window = resolved;
  backend->in_frame = CMP_TRUE;

  rc = cmp_web_wgpu_configure_surface(backend, device_width, device_height);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_wgpu_acquire_frame(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_wgpu_apply_clip(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static int cmp_web_wgpu_end_frame(struct CMPWebBackend *backend) {
  CMPWebGPUState *wgpu;
  WGPUCommandBufferDescriptor cmd_desc;
  WGPUCommandBuffer command_buffer;
  WGPUStatus present_status;
  int result;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->in_frame) {
    return CMP_ERR_STATE;
  }

  wgpu = &backend->wgpu;
  if (wgpu->pass != NULL) {
    if (wgpu->pass_active) {
      wgpuRenderPassEncoderEnd(wgpu->pass);
    }
    wgpuRenderPassEncoderRelease(wgpu->pass);
    wgpu->pass = NULL;
  }
  wgpu->pass_active = CMP_FALSE;

  if (wgpu->encoder == NULL || wgpu->queue == NULL) {
    return CMP_ERR_STATE;
  }

  memset(&cmd_desc, 0, sizeof(cmd_desc));
  command_buffer = wgpuCommandEncoderFinish(wgpu->encoder, &cmd_desc);
  if (command_buffer == NULL) {
    wgpuCommandEncoderRelease(wgpu->encoder);
    wgpu->encoder = NULL;
    result = CMP_ERR_UNKNOWN;
  } else {
    wgpuQueueSubmit(wgpu->queue, 1u, &command_buffer);
    wgpuCommandBufferRelease(command_buffer);
    wgpuCommandEncoderRelease(wgpu->encoder);
    wgpu->encoder = NULL;
    result = CMP_OK;
  }

  if (result == CMP_OK) {
    if (wgpu->surface != NULL && wgpu->surface_texture != NULL) {
      present_status = wgpuSurfacePresent(wgpu->surface);
      if (present_status != WGPUStatus_Success) {
        result = CMP_ERR_UNKNOWN;
      }
    } else {
      result = CMP_ERR_STATE;
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
  backend->in_frame = CMP_FALSE;
  return result;
}

static int cmp_web_wgpu_clear(struct CMPWebBackend *backend, CMPColor color) {
  CMPWebGPUState *wgpu;
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->in_frame) {
    return CMP_ERR_STATE;
  }

  wgpu = &backend->wgpu;
  wgpu->clear_color.r = color.r;
  wgpu->clear_color.g = color.g;
  wgpu->clear_color.b = color.b;
  wgpu->clear_color.a = color.a;

  rc = cmp_web_wgpu_begin_pass(backend, CMP_TRUE);
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_OK;
}
#endif

static int cmp_web_gfx_apply_clip(struct CMPWebBackend *backend) {
  CMPRect rect;
  CMPScalar scale;
  cmp_i32 x;
  cmp_i32 y;
  cmp_i32 w;
  cmp_i32 h;
  cmp_i32 max_w;
  cmp_i32 max_h;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(CMP_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return cmp_web_wgpu_apply_clip(backend);
  }
#endif

  if (backend->clip_depth == 0) {
    glDisable(GL_SCISSOR_TEST);
    return cmp_web_gl_check_error(backend, "gfx.clip.disable");
  }

  if (backend->active_window == NULL) {
    return CMP_ERR_STATE;
  }

  rect = backend->clip_stack[backend->clip_depth - 1u];
  if (rect.width < 0.0f || rect.height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  scale = backend->active_window->dpi_scale;
  if (scale <= 0.0f) {
    scale = 1.0f;
  }

  x = (cmp_i32)(rect.x * scale);
  y = (cmp_i32)(rect.y * scale);
  w = (cmp_i32)(rect.width * scale);
  h = (cmp_i32)(rect.height * scale);

  if (w < 0) {
    w = 0;
  }
  if (h < 0) {
    h = 0;
  }

  max_w = backend->device_width;
  max_h = backend->device_height;
  if (max_w <= 0 || max_h <= 0) {
    return CMP_ERR_STATE;
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

  return cmp_web_gl_check_error(backend, "gfx.clip");
}

static int cmp_web_gfx_draw_quad(struct CMPWebBackend *backend,
                                const CMPWebVertex *verts, CMPColor color,
                                CMPBool use_texture, GLuint texture,
                                CMPScalar opacity, const CMPRect *rect,
                                CMPScalar radius) {
  int rc;

  if (backend == NULL || verts == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (use_texture && texture == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_web_gl_prepare(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

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

  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(CMPWebVertex) * 4u), verts,
               GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  return cmp_web_gl_check_error(backend, "gfx.draw_quad");
}

static int cmp_web_backend_set_canvas_size(struct CMPWebBackend *backend,
                                          cmp_i32 width, cmp_i32 height,
                                          CMPScalar dpi_scale) {
  cmp_i32 device_width;
  cmp_i32 device_height;
  int result;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (backend->canvas_id == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (dpi_scale <= 0.0f) {
    dpi_scale = 1.0f;
  }

  device_width = (cmp_i32)((CMPScalar)width * dpi_scale + 0.5f);
  device_height = (cmp_i32)((CMPScalar)height * dpi_scale + 0.5f);
  if (device_width <= 0 || device_height <= 0) {
    return CMP_ERR_RANGE;
  }

  result = emscripten_set_canvas_element_size(backend->canvas_id, device_width,
                                              device_height);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }

  result = emscripten_set_element_css_size(backend->canvas_id, (double)width,
                                           (double)height);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }

  return CMP_OK;
}

static EM_BOOL cmp_web_on_mouse_down(int event_type,
                                    const EmscriptenMouseEvent *event,
                                    void *user_data) {
  struct CMPWebBackend *backend;
  CMPInputEvent input;
  cmp_u32 mods;
  int rc;

  CMP_UNUSED(event_type);

  backend = (struct CMPWebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc = cmp_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != CMP_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = CMP_INPUT_POINTER_DOWN;
  input.modifiers = mods;
  input.window = backend->event_window;
  cmp_web_event_stamp(backend, &input);

  input.data.pointer.pointer_id = 0;
  input.data.pointer.x = (cmp_i32)event->canvasX;
  input.data.pointer.y = (cmp_i32)event->canvasY;
  input.data.pointer.buttons = (cmp_i32)event->buttons;
  input.data.pointer.scroll_x = 0;
  input.data.pointer.scroll_y = 0;

  rc = cmp_web_event_queue_push(&backend->events, &input);
  if (rc != CMP_OK) {
    cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL cmp_web_on_mouse_up(int event_type,
                                  const EmscriptenMouseEvent *event,
                                  void *user_data) {
  struct CMPWebBackend *backend;
  CMPInputEvent input;
  cmp_u32 mods;
  int rc;

  CMP_UNUSED(event_type);

  backend = (struct CMPWebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc = cmp_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != CMP_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = CMP_INPUT_POINTER_UP;
  input.modifiers = mods;
  input.window = backend->event_window;
  cmp_web_event_stamp(backend, &input);

  input.data.pointer.pointer_id = 0;
  input.data.pointer.x = (cmp_i32)event->canvasX;
  input.data.pointer.y = (cmp_i32)event->canvasY;
  input.data.pointer.buttons = (cmp_i32)event->buttons;
  input.data.pointer.scroll_x = 0;
  input.data.pointer.scroll_y = 0;

  rc = cmp_web_event_queue_push(&backend->events, &input);
  if (rc != CMP_OK) {
    cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL cmp_web_on_mouse_move(int event_type,
                                    const EmscriptenMouseEvent *event,
                                    void *user_data) {
  struct CMPWebBackend *backend;
  CMPInputEvent input;
  cmp_u32 mods;
  int rc;

  CMP_UNUSED(event_type);

  backend = (struct CMPWebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc = cmp_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != CMP_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = CMP_INPUT_POINTER_MOVE;
  input.modifiers = mods;
  input.window = backend->event_window;
  cmp_web_event_stamp(backend, &input);

  input.data.pointer.pointer_id = 0;
  input.data.pointer.x = (cmp_i32)event->canvasX;
  input.data.pointer.y = (cmp_i32)event->canvasY;
  input.data.pointer.buttons = (cmp_i32)event->buttons;
  input.data.pointer.scroll_x = 0;
  input.data.pointer.scroll_y = 0;

  rc = cmp_web_event_queue_push(&backend->events, &input);
  if (rc != CMP_OK) {
    cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL cmp_web_on_wheel(int event_type,
                               const EmscriptenWheelEvent *event,
                               void *user_data) {
  struct CMPWebBackend *backend;
  CMPInputEvent input;
  cmp_u32 mods;
  cmp_i32 dx;
  cmp_i32 dy;
  int rc;

  CMP_UNUSED(event_type);

  backend = (struct CMPWebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc =
      cmp_web_build_modifiers(&mods, event->mouse.shiftKey, event->mouse.ctrlKey,
                             event->mouse.altKey, event->mouse.metaKey);
  if (rc != CMP_OK) {
    return EM_FALSE;
  }

  dx = 0;
  dy = 0;
  rc = cmp_web_double_to_i32(event->deltaX, &dx);
  if (rc != CMP_OK) {
    dx = 0;
  }
  rc = cmp_web_double_to_i32(event->deltaY, &dy);
  if (rc != CMP_OK) {
    dy = 0;
  }

  memset(&input, 0, sizeof(input));
  input.type = CMP_INPUT_POINTER_SCROLL;
  input.modifiers = mods;
  input.window = backend->event_window;
  cmp_web_event_stamp(backend, &input);

  input.data.pointer.pointer_id = 0;
  input.data.pointer.x = (cmp_i32)event->mouse.canvasX;
  input.data.pointer.y = (cmp_i32)event->mouse.canvasY;
  input.data.pointer.buttons = (cmp_i32)event->mouse.buttons;
  input.data.pointer.scroll_x = dx;
  input.data.pointer.scroll_y = dy;

  rc = cmp_web_event_queue_push(&backend->events, &input);
  if (rc != CMP_OK) {
    cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL cmp_web_on_touch(int event_type,
                               const EmscriptenTouchEvent *event,
                               void *user_data) {
  struct CMPWebBackend *backend;
  cmp_u32 mods;
  int rc;
  int i;

  backend = (struct CMPWebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc = cmp_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != CMP_OK) {
    return EM_FALSE;
  }

  for (i = 0; i < event->numTouches; i += 1) {
    CMPInputEvent input;
    cmp_i32 buttons;

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
      input.type = CMP_INPUT_POINTER_DOWN;
    } else if (event_type == EMSCRIPTEN_EVENT_TOUCHEND ||
               event_type == EMSCRIPTEN_EVENT_TOUCHCANCEL) {
      input.type = CMP_INPUT_POINTER_UP;
    } else {
      input.type = CMP_INPUT_POINTER_MOVE;
    }
    input.modifiers = mods;
    input.window = backend->event_window;
    cmp_web_event_stamp(backend, &input);

    input.data.pointer.pointer_id = (cmp_i32)event->touches[i].identifier;
    input.data.pointer.x = (cmp_i32)event->touches[i].canvasX;
    input.data.pointer.y = (cmp_i32)event->touches[i].canvasY;
    input.data.pointer.buttons = buttons;
    input.data.pointer.scroll_x = 0;
    input.data.pointer.scroll_y = 0;

    rc = cmp_web_event_queue_push(&backend->events, &input);
    if (rc != CMP_OK) {
      cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "events.full");
      break;
    }
  }

  return EM_TRUE;
}

static EM_BOOL cmp_web_on_key_down(int event_type,
                                  const EmscriptenKeyboardEvent *event,
                                  void *user_data) {
  struct CMPWebBackend *backend;
  CMPInputEvent input;
  cmp_u32 mods;
  int rc;

  CMP_UNUSED(event_type);

  backend = (struct CMPWebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc = cmp_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != CMP_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = CMP_INPUT_KEY_DOWN;
  input.modifiers = mods;
  input.window = backend->event_window;
  cmp_web_event_stamp(backend, &input);

  input.data.key.key_code = (cmp_u32)event->keyCode;
  input.data.key.native_code = (cmp_u32)event->which;
  input.data.key.is_repeat = event->repeat ? CMP_TRUE : CMP_FALSE;

  rc = cmp_web_event_queue_push(&backend->events, &input);
  if (rc != CMP_OK) {
    cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL cmp_web_on_key_up(int event_type,
                                const EmscriptenKeyboardEvent *event,
                                void *user_data) {
  struct CMPWebBackend *backend;
  CMPInputEvent input;
  cmp_u32 mods;
  int rc;

  CMP_UNUSED(event_type);

  backend = (struct CMPWebBackend *)user_data;
  if (backend == NULL || event == NULL) {
    return EM_FALSE;
  }

  rc = cmp_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != CMP_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = CMP_INPUT_KEY_UP;
  input.modifiers = mods;
  input.window = backend->event_window;
  cmp_web_event_stamp(backend, &input);

  input.data.key.key_code = (cmp_u32)event->keyCode;
  input.data.key.native_code = (cmp_u32)event->which;
  input.data.key.is_repeat = event->repeat ? CMP_TRUE : CMP_FALSE;

  rc = cmp_web_event_queue_push(&backend->events, &input);
  if (rc != CMP_OK) {
    cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL cmp_web_on_key_press(int event_type,
                                   const EmscriptenKeyboardEvent *event,
                                   void *user_data) {
  struct CMPWebBackend *backend;
  CMPInputEvent input;
  cmp_u32 mods;
  cmp_usize length;
  int rc;

  CMP_UNUSED(event_type);

  backend = (struct CMPWebBackend *)user_data;
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

  rc = cmp_web_build_modifiers(&mods, event->shiftKey, event->ctrlKey,
                              event->altKey, event->metaKey);
  if (rc != CMP_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = CMP_INPUT_TEXT;
  input.modifiers = mods;
  input.window = backend->event_window;
  cmp_web_event_stamp(backend, &input);

  memcpy(input.data.text.utf8, event->key, length);
  input.data.text.utf8[length] = '\0';
  input.data.text.length = (cmp_u32)length;

  rc = cmp_web_event_queue_push(&backend->events, &input);
  if (rc != CMP_OK) {
    cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL cmp_web_on_focus(int event_type,
                               const EmscriptenFocusEvent *event,
                               void *user_data) {
  struct CMPWebBackend *backend;
  CMPInputEvent input;
  int rc;

  CMP_UNUSED(event_type);
  CMP_UNUSED(event);

  backend = (struct CMPWebBackend *)user_data;
  if (backend == NULL) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = CMP_INPUT_WINDOW_FOCUS;
  input.modifiers = 0u;
  input.window = backend->event_window;
  cmp_web_event_stamp(backend, &input);

  rc = cmp_web_event_queue_push(&backend->events, &input);
  if (rc != CMP_OK) {
    cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL cmp_web_on_blur(int event_type, const EmscriptenFocusEvent *event,
                              void *user_data) {
  struct CMPWebBackend *backend;
  CMPInputEvent input;
  int rc;

  CMP_UNUSED(event_type);
  CMP_UNUSED(event);

  backend = (struct CMPWebBackend *)user_data;
  if (backend == NULL) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = CMP_INPUT_WINDOW_BLUR;
  input.modifiers = 0u;
  input.window = backend->event_window;
  cmp_web_event_stamp(backend, &input);

  rc = cmp_web_event_queue_push(&backend->events, &input);
  if (rc != CMP_OK) {
    cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static EM_BOOL cmp_web_on_resize(int event_type, const EmscriptenUiEvent *event,
                                void *user_data) {
  struct CMPWebBackend *backend;
  CMPWebWindow *window;
  CMPInputEvent input;
  double css_width;
  double css_height;
  cmp_i32 width;
  cmp_i32 height;
  int result;
  int rc;

  CMP_UNUSED(event_type);
  CMP_UNUSED(event);

  backend = (struct CMPWebBackend *)user_data;
  if (backend == NULL) {
    return EM_FALSE;
  }

  if (backend->event_window.id == 0u &&
      backend->event_window.generation == 0u) {
    return EM_FALSE;
  }

  window = NULL;
  rc = cmp_web_backend_resolve(backend, backend->event_window,
                              CMP_WEB_TYPE_WINDOW, (void **)&window);
  if (rc != CMP_OK || window == NULL) {
    return EM_FALSE;
  }

  css_width = 0.0;
  css_height = 0.0;
  result = emscripten_get_element_css_size(backend->canvas_id, &css_width,
                                           &css_height);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return EM_FALSE;
  }

  rc = cmp_web_double_to_i32(css_width, &width);
  if (rc != CMP_OK) {
    return EM_FALSE;
  }
  rc = cmp_web_double_to_i32(css_height, &height);
  if (rc != CMP_OK) {
    return EM_FALSE;
  }

  if (width <= 0 || height <= 0) {
    return EM_FALSE;
  }

  window->width = width;
  window->height = height;
  rc =
      cmp_web_backend_set_canvas_size(backend, width, height, window->dpi_scale);
  if (rc != CMP_OK) {
    return EM_FALSE;
  }

  memset(&input, 0, sizeof(input));
  input.type = CMP_INPUT_WINDOW_RESIZE;
  input.modifiers = 0u;
  input.window = backend->event_window;
  cmp_web_event_stamp(backend, &input);

  input.data.window.width = width;
  input.data.window.height = height;

  rc = cmp_web_event_queue_push(&backend->events, &input);
  if (rc != CMP_OK) {
    cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "events.full");
  }
  return EM_TRUE;
}

static int cmp_web_register_callbacks(struct CMPWebBackend *backend) {
  int result;
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (backend->callbacks_registered) {
    return CMP_OK;
  }

  result = emscripten_set_mousedown_callback(backend->canvas_id, backend, 1,
                                             cmp_web_on_mouse_down);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }
  result = emscripten_set_mouseup_callback(backend->canvas_id, backend, 1,
                                           cmp_web_on_mouse_up);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }
  result = emscripten_set_mousemove_callback(backend->canvas_id, backend, 1,
                                             cmp_web_on_mouse_move);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }
  result = emscripten_set_wheel_callback(backend->canvas_id, backend, 1,
                                         cmp_web_on_wheel);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }

  result = emscripten_set_touchstart_callback(backend->canvas_id, backend, 1,
                                              cmp_web_on_touch);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }
  result = emscripten_set_touchend_callback(backend->canvas_id, backend, 1,
                                            cmp_web_on_touch);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }
  result = emscripten_set_touchmove_callback(backend->canvas_id, backend, 1,
                                             cmp_web_on_touch);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }
  result = emscripten_set_touchcancel_callback(backend->canvas_id, backend, 1,
                                               cmp_web_on_touch);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }

  result = emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                           backend, 1, cmp_web_on_key_down);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }
  result = emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                         backend, 1, cmp_web_on_key_up);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }
  result = emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                            backend, 1, cmp_web_on_key_press);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }

  result = emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                         backend, 1, cmp_web_on_focus);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }
  result = emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, backend,
                                        1, cmp_web_on_blur);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }
  result = emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                          backend, 1, cmp_web_on_resize);
  if (result != EMSCRIPTEN_RESULT_SUCCESS) {
    return cmp_web_backend_map_result(result);
  }

  backend->callbacks_registered = CMP_TRUE;
  rc =
      cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.callbacks_registered");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_web_object_retain(void *obj) {
  return cmp_object_retain((CMPObjectHeader *)obj);
}

static int cmp_web_object_release(void *obj) {
  return cmp_object_release((CMPObjectHeader *)obj);
}

static int cmp_web_object_get_type_id(void *obj, cmp_u32 *out_type_id) {
  return cmp_object_get_type_id((const CMPObjectHeader *)obj, out_type_id);
}

static int cmp_web_window_destroy(void *obj) {
  CMPWebWindow *window;
  struct CMPWebBackend *backend;
  int rc;

  window = (CMPWebWindow *)obj;
  backend = window->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  window->header.handle);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = backend->allocator.free(backend->allocator.ctx, window);
  CMP_WEB_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static int cmp_web_texture_destroy(void *obj) {
  CMPWebTexture *texture;
  struct CMPWebBackend *backend;
  int rc;

  texture = (CMPWebTexture *)obj;
  backend = texture->backend;

  if (texture->gl_texture != 0) {
    rc = cmp_web_gl_require_context(backend);
    CMP_WEB_RETURN_IF_ERROR(rc);
    glDeleteTextures(1, &texture->gl_texture);
    texture->gl_texture = 0;
  }
#if defined(CMP_WEBGPU_AVAILABLE)
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
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = backend->allocator.free(backend->allocator.ctx, texture);
  CMP_WEB_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static int cmp_web_font_destroy(void *obj) {
  CMPWebFont *font;
  struct CMPWebBackend *backend;
  int rc;

  font = (CMPWebFont *)obj;
  backend = font->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  font->header.handle);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = backend->allocator.free(backend->allocator.ctx, font);
  CMP_WEB_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static const CMPObjectVTable g_cmp_web_window_vtable = {
    cmp_web_object_retain, cmp_web_object_release, cmp_web_window_destroy,
    cmp_web_object_get_type_id};

static const CMPObjectVTable g_cmp_web_texture_vtable = {
    cmp_web_object_retain, cmp_web_object_release, cmp_web_texture_destroy,
    cmp_web_object_get_type_id};

static const CMPObjectVTable g_cmp_web_font_vtable = {
    cmp_web_object_retain, cmp_web_object_release, cmp_web_font_destroy,
    cmp_web_object_get_type_id};

static int cmp_web_ws_init(void *ws, const CMPWSConfig *config) {
  struct CMPWebBackend *backend;
  int rc;

  if (ws == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_app_name == NULL || config->utf8_app_id == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.init");
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_register_callbacks(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_web_ws_shutdown(void *ws) {
  struct CMPWebBackend *backend;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.shutdown");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_web_ws_create_window(void *ws, const CMPWSWindowConfig *config,
                                   CMPHandle *out_window) {
  struct CMPWebBackend *backend;
  CMPWebWindow *window;
  double ratio;
  int rc;

  if (ws == NULL || config == NULL || out_window == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_title == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->width <= 0 || config->height <= 0) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWebBackend *)ws;
  out_window->id = 0u;
  out_window->generation = 0u;

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.create_window");
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPWebWindow),
                                (void **)&window);
  CMP_WEB_RETURN_IF_ERROR(rc);

  memset(window, 0, sizeof(*window));
  window->backend = backend;
  window->width = config->width;
  window->height = config->height;
  window->flags = config->flags;
  window->visible = CMP_FALSE;

  ratio = emscripten_get_device_pixel_ratio();
  if (ratio <= 0.0) {
    ratio = 1.0;
  }
  window->dpi_scale = (CMPScalar)ratio;

  rc = cmp_object_header_init(&window->header, CMP_WEB_TYPE_WINDOW, 0,
                             &g_cmp_web_window_vtable);
  CMP_WEB_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, window));

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &window->header);
  CMP_WEB_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, window));

  rc = cmp_web_backend_set_canvas_size(backend, window->width, window->height,
                                      window->dpi_scale);
  CMP_WEB_RETURN_IF_ERROR_CLEANUP(rc, cmp_object_release(&window->header));

  if (backend->event_window.id == 0u &&
      backend->event_window.generation == 0u) {
    backend->event_window = window->header.handle;
  }

#if defined(CMP_WEBGPU_AVAILABLE)
  rc = cmp_web_wgpu_init(backend);
  if (rc == CMP_OK) {
    backend->use_webgpu = CMP_TRUE;
  } else {
    backend->use_webgpu = CMP_FALSE;
    rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_WARN, "webgpu.init_failed");
    CMP_WEB_RETURN_IF_ERROR_CLEANUP(rc, cmp_object_release(&window->header));
    rc = cmp_web_gl_init(backend);
    CMP_WEB_RETURN_IF_ERROR_CLEANUP(rc, cmp_object_release(&window->header));
  }
#else
  rc = cmp_web_gl_init(backend);
  CMP_WEB_RETURN_IF_ERROR_CLEANUP(rc, cmp_object_release(&window->header));
#endif

  *out_window = window->header.handle;
  return CMP_OK;
}

static int cmp_web_ws_destroy_window(void *ws, CMPHandle window) {
  struct CMPWebBackend *backend;
  CMPWebWindow *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_resolve(backend, window, CMP_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.destroy_window");
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (backend->event_window.id == window.id &&
      backend->event_window.generation == window.generation) {
    backend->event_window.id = 0u;
    backend->event_window.generation = 0u;
  }

  return cmp_object_release(&resolved->header);
}

static int cmp_web_ws_show_window(void *ws, CMPHandle window) {
  struct CMPWebBackend *backend;
  CMPWebWindow *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_resolve(backend, window, CMP_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.show_window");
  CMP_WEB_RETURN_IF_ERROR(rc);

  resolved->visible = CMP_TRUE;
  return CMP_OK;
}

static int cmp_web_ws_hide_window(void *ws, CMPHandle window) {
  struct CMPWebBackend *backend;
  CMPWebWindow *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_resolve(backend, window, CMP_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.hide_window");
  CMP_WEB_RETURN_IF_ERROR(rc);

  resolved->visible = CMP_FALSE;
  return CMP_OK;
}

static int cmp_web_ws_set_window_title(void *ws, CMPHandle window,
                                      const char *utf8_title) {
  struct CMPWebBackend *backend;
  int rc;

  if (ws == NULL || utf8_title == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_resolve(backend, window, CMP_WEB_TYPE_WINDOW, NULL);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_window_title");
  CMP_WEB_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static int cmp_web_ws_set_window_size(void *ws, CMPHandle window, cmp_i32 width,
                                     cmp_i32 height) {
  struct CMPWebBackend *backend;
  CMPWebWindow *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_resolve(backend, window, CMP_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_window_size");
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_set_canvas_size(backend, width, height,
                                      resolved->dpi_scale);
  CMP_WEB_RETURN_IF_ERROR(rc);

  resolved->width = width;
  resolved->height = height;
  return CMP_OK;
}

static int cmp_web_ws_get_window_size(void *ws, CMPHandle window,
                                     cmp_i32 *out_width, cmp_i32 *out_height) {
  struct CMPWebBackend *backend;
  CMPWebWindow *resolved;
  int rc;

  if (ws == NULL || out_width == NULL || out_height == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_resolve(backend, window, CMP_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.get_window_size");
  CMP_WEB_RETURN_IF_ERROR(rc);

  *out_width = resolved->width;
  *out_height = resolved->height;
  return CMP_OK;
}

static int cmp_web_ws_set_window_dpi_scale(void *ws, CMPHandle window,
                                          CMPScalar scale) {
  struct CMPWebBackend *backend;
  CMPWebWindow *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (scale <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_resolve(backend, window, CMP_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc =
      cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_window_dpi_scale");
  CMP_WEB_RETURN_IF_ERROR(rc);

  resolved->dpi_scale = scale;
  rc = cmp_web_backend_set_canvas_size(backend, resolved->width,
                                      resolved->height, resolved->dpi_scale);
  CMP_WEB_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static int cmp_web_ws_get_window_dpi_scale(void *ws, CMPHandle window,
                                          CMPScalar *out_scale) {
  struct CMPWebBackend *backend;
  CMPWebWindow *resolved;
  int rc;

  if (ws == NULL || out_scale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_resolve(backend, window, CMP_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc =
      cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.get_window_dpi_scale");
  CMP_WEB_RETURN_IF_ERROR(rc);

  *out_scale = resolved->dpi_scale;
  return CMP_OK;
}

static int cmp_web_ws_set_clipboard_text(void *ws, const char *utf8_text) {
  struct CMPWebBackend *backend;
  cmp_usize length;
  cmp_usize required;
  int rc;

  if (ws == NULL || utf8_text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_clipboard_text");
  CMP_WEB_RETURN_IF_ERROR(rc);

  length = 0;
  while (utf8_text[length] != '\0') {
    length += 1;
  }
  if (length > backend->clipboard_limit) {
    return CMP_ERR_RANGE;
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
    CMP_WEB_RETURN_IF_ERROR(rc);
    backend->clipboard = (char *)mem;
    backend->clipboard_capacity = required;
  }

  memcpy(backend->clipboard, utf8_text, required);
  backend->clipboard_length = length;
  return CMP_OK;
}

static int cmp_web_ws_get_clipboard_text(void *ws, char *buffer,
                                        cmp_usize buffer_size,
                                        cmp_usize *out_length) {
  struct CMPWebBackend *backend;
  cmp_usize required;
  int rc;

  if (ws == NULL || buffer == NULL || out_length == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.get_clipboard_text");
  CMP_WEB_RETURN_IF_ERROR(rc);

  required = backend->clipboard_length + 1;
  if (buffer_size < required) {
    *out_length = backend->clipboard_length;
    return CMP_ERR_RANGE;
  }

  if (backend->clipboard_length == 0) {
    buffer[0] = '\0';
  } else {
    memcpy(buffer, backend->clipboard, required);
  }
  *out_length = backend->clipboard_length;
  return CMP_OK;
}

static int cmp_web_ws_poll_event(void *ws, CMPInputEvent *out_event,
                                CMPBool *out_has_event) {
  struct CMPWebBackend *backend;
  int rc;

  if (ws == NULL || out_event == NULL || out_has_event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.poll_event");
  CMP_WEB_RETURN_IF_ERROR(rc);

  return cmp_web_event_queue_pop(&backend->events, out_event, out_has_event);
}

static int cmp_web_ws_pump_events(void *ws) {
  struct CMPWebBackend *backend;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.pump_events");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_web_ws_get_time_ms(void *ws, cmp_u32 *out_time_ms) {
  struct CMPWebBackend *backend;
  int rc;

  if (ws == NULL || out_time_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)ws;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.get_time_ms");
  CMP_WEB_RETURN_IF_ERROR(rc);

  return cmp_web_backend_time_now_ms(out_time_ms);
}

static const CMPWSVTable g_cmp_web_ws_vtable = {cmp_web_ws_init,
                                              cmp_web_ws_shutdown,
                                              cmp_web_ws_create_window,
                                              cmp_web_ws_destroy_window,
                                              cmp_web_ws_show_window,
                                              cmp_web_ws_hide_window,
                                              cmp_web_ws_set_window_title,
                                              cmp_web_ws_set_window_size,
                                              cmp_web_ws_get_window_size,
                                              cmp_web_ws_set_window_dpi_scale,
                                              cmp_web_ws_get_window_dpi_scale,
                                              cmp_web_ws_set_clipboard_text,
                                              cmp_web_ws_get_clipboard_text,
                                              cmp_web_ws_poll_event,
                                              cmp_web_ws_pump_events,
                                              cmp_web_ws_get_time_ms};

static int cmp_web_gfx_begin_frame(void *gfx, CMPHandle window, cmp_i32 width,
                                  cmp_i32 height, CMPScalar dpi_scale) {
  struct CMPWebBackend *backend;
  CMPWebWindow *resolved;
  cmp_i32 device_width;
  cmp_i32 device_height;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (dpi_scale <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWebBackend *)gfx;
  rc = cmp_web_backend_resolve(backend, window, CMP_WEB_TYPE_WINDOW,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.begin_frame");
  CMP_WEB_RETURN_IF_ERROR(rc);

#if defined(CMP_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return cmp_web_wgpu_begin_frame(backend, resolved, width, height, dpi_scale);
  }
#endif

  rc = cmp_web_gl_require_context(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (resolved->width != width || resolved->height != height ||
      resolved->dpi_scale != dpi_scale) {
    resolved->width = width;
    resolved->height = height;
    resolved->dpi_scale = dpi_scale;
    rc = cmp_web_backend_set_canvas_size(backend, width, height, dpi_scale);
    CMP_WEB_RETURN_IF_ERROR(rc);
  }

  device_width = (cmp_i32)((CMPScalar)width * dpi_scale + 0.5f);
  device_height = (cmp_i32)((CMPScalar)height * dpi_scale + 0.5f);
  if (device_width <= 0 || device_height <= 0) {
    return CMP_ERR_RANGE;
  }

  backend->viewport_width = width;
  backend->viewport_height = height;
  backend->device_width = device_width;
  backend->device_height = device_height;
  backend->active_window = resolved;
  backend->in_frame = CMP_TRUE;

  glViewport(0, 0, device_width, device_height);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  rc = cmp_web_gfx_apply_clip(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  return cmp_web_gl_check_error(backend, "gfx.begin_frame");
}

static int cmp_web_gfx_end_frame(void *gfx, CMPHandle window) {
  struct CMPWebBackend *backend;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)gfx;
  rc = cmp_web_backend_resolve(backend, window, CMP_WEB_TYPE_WINDOW, NULL);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.end_frame");
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (!backend->in_frame) {
    return CMP_ERR_STATE;
  }

#if defined(CMP_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return cmp_web_wgpu_end_frame(backend);
  }
#endif

  glFlush();

  backend->active_window = NULL;
  backend->in_frame = CMP_FALSE;
  return CMP_OK;
}

static int cmp_web_gfx_clear(void *gfx, CMPColor color) {
  struct CMPWebBackend *backend;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)gfx;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.clear");
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (!backend->in_frame) {
    return CMP_ERR_STATE;
  }

  rc = cmp_web_gl_require_context(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);

  return cmp_web_gl_check_error(backend, "gfx.clear");
}

static int cmp_web_gfx_draw_rect(void *gfx, const CMPRect *rect, CMPColor color,
                                CMPScalar corner_radius) {
  struct CMPWebBackend *backend;
  CMPWebVertex verts[4];
  CMPRect rect_copy;
  CMPScalar radius;
  CMPScalar max_radius;
  int rc;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWebBackend *)gfx;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_rect");
  CMP_WEB_RETURN_IF_ERROR(rc);

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

#if defined(CMP_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return cmp_web_wgpu_draw_quad(backend, verts, color, CMP_FALSE, NULL, 1.0f,
                                 &rect_copy, radius);
  }
#endif

  return cmp_web_gfx_draw_quad(backend, verts, color, CMP_FALSE, 0, 1.0f,
                              &rect_copy, radius);
}

static int cmp_web_gfx_draw_line(void *gfx, CMPScalar x0, CMPScalar y0,
                                CMPScalar x1, CMPScalar y1, CMPColor color,
                                CMPScalar thickness) {
  struct CMPWebBackend *backend;
  CMPWebVertex verts[4];
  CMPRect rect;
  CMPScalar dx;
  CMPScalar dy;
  CMPScalar len;
  CMPScalar nx;
  CMPScalar ny;
  CMPScalar half;
  CMPScalar min_x;
  CMPScalar max_x;
  CMPScalar min_y;
  CMPScalar max_y;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (thickness < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (thickness == 0.0f) {
    return CMP_OK;
  }

  backend = (struct CMPWebBackend *)gfx;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_line");
  CMP_WEB_RETURN_IF_ERROR(rc);

  dx = x1 - x0;
  dy = y1 - y0;
  len = (CMPScalar)sqrt((double)(dx * dx + dy * dy));
  if (len <= 0.0f) {
    return CMP_OK;
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

#if defined(CMP_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return cmp_web_wgpu_draw_quad(backend, verts, color, CMP_FALSE, NULL, 1.0f,
                                 &rect, 0.0f);
  }
#endif

  return cmp_web_gfx_draw_quad(backend, verts, color, CMP_FALSE, 0, 1.0f, &rect,
                              0.0f);
}

static int cmp_web_gfx_draw_path(void *gfx, const CMPPath *path, CMPColor color) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(color);

  if (gfx == NULL || path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (path->commands == NULL) {
    return CMP_ERR_STATE;
  }
  if (path->count > path->capacity) {
    return CMP_ERR_STATE;
  }
  if (path->count == 0) {
    return CMP_OK;
  }

  backend = (struct CMPWebBackend *)gfx;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_path");
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (!backend->in_frame) {
    return CMP_ERR_STATE;
  }

  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_gfx_push_clip(void *gfx, const CMPRect *rect) {
  struct CMPWebBackend *backend;
  CMPRect clipped;
  CMPBool has_intersection;
  int rc;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWebBackend *)gfx;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.push_clip");
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (backend->clip_depth >= CMP_WEB_CLIP_STACK_CAPACITY) {
    return CMP_ERR_OVERFLOW;
  }

  if (backend->clip_depth == 0) {
    clipped = *rect;
  } else {
    rc = cmp_rect_intersect(&backend->clip_stack[backend->clip_depth - 1u], rect,
                           &clipped, &has_intersection);
    CMP_WEB_RETURN_IF_ERROR(rc);
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
    rc = cmp_web_gfx_apply_clip(backend);
    CMP_WEB_RETURN_IF_ERROR(rc);
  }

  return CMP_OK;
}

static int cmp_web_gfx_pop_clip(void *gfx) {
  struct CMPWebBackend *backend;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)gfx;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.pop_clip");
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (backend->clip_depth == 0) {
    return CMP_ERR_STATE;
  }

  backend->clip_depth -= 1u;

  if (backend->in_frame) {
    rc = cmp_web_gfx_apply_clip(backend);
    CMP_WEB_RETURN_IF_ERROR(rc);
  }

  return CMP_OK;
}

static int cmp_web_gfx_set_transform(void *gfx, const CMPMat3 *transform) {
  struct CMPWebBackend *backend;
  int rc;

  if (gfx == NULL || transform == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)gfx;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.set_transform");
  CMP_WEB_RETURN_IF_ERROR(rc);

  backend->transform = *transform;
  backend->has_transform = CMP_TRUE;
  return CMP_OK;
}

static int cmp_web_mul_size(cmp_usize a, cmp_usize b, cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = (cmp_usize) ~(cmp_usize)0;
  if (a != 0 && b > max_value / a) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return CMP_OK;
}

static int cmp_web_convert_bgra(const CMPAllocator *allocator, const void *pixels,
                               cmp_usize size, void **out_pixels) {
  unsigned char *dst;
  const unsigned char *src;
  cmp_usize i;
  int rc;

  if (allocator == NULL || out_pixels == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size == 0 || pixels == NULL) {
    *out_pixels = NULL;
    return CMP_OK;
  }

  rc = allocator->alloc(allocator->ctx, size, (void **)&dst);
  CMP_WEB_RETURN_IF_ERROR(rc);

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
  return CMP_OK;
}

#if defined(CMP_WEBGPU_AVAILABLE)
static int cmp_web_wgpu_write_texture(struct CMPWebBackend *backend,
                                     CMPWebTexture *texture, cmp_i32 x, cmp_i32 y,
                                     cmp_i32 width, cmp_i32 height,
                                     const void *pixels, cmp_usize size) {
  cmp_usize row_bytes;
  cmp_usize expected_size;
  cmp_usize padded_row_bytes;
  cmp_usize padded_size;
  unsigned char *staging;
  const unsigned char *src;
  cmp_i32 row;
  WGPUTexelCopyTextureInfo dst;
  WGPUTexelCopyBufferLayout layout;
  WGPUExtent3D extent;
  int rc;

  if (backend == NULL || texture == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (pixels == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (pixels == NULL && size == 0) {
    return CMP_OK;
  }
  if (backend->wgpu.queue == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_web_mul_size((cmp_usize)width, (cmp_usize)texture->bytes_per_pixel,
                       &row_bytes);
  CMP_WEB_RETURN_IF_ERROR(rc);
  rc = cmp_web_mul_size(row_bytes, (cmp_usize)height, &expected_size);
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (size != 0 && size < expected_size) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_web_wgpu_align_size(row_bytes, (cmp_usize)CMP_WEB_WGPU_ROW_ALIGN,
                              &padded_row_bytes);
  CMP_WEB_RETURN_IF_ERROR(rc);
  rc = cmp_web_mul_size(padded_row_bytes, (cmp_usize)height, &padded_size);
  CMP_WEB_RETURN_IF_ERROR(rc);

  staging = (unsigned char *)pixels;
  if (padded_row_bytes != row_bytes) {
    rc = backend->allocator.alloc(backend->allocator.ctx, padded_size,
                                  (void **)&staging);
    CMP_WEB_RETURN_IF_ERROR(rc);

    src = (const unsigned char *)pixels;
    for (row = 0; row < height; ++row) {
      memcpy(staging + (cmp_usize)row * padded_row_bytes,
             src + (cmp_usize)row * row_bytes, row_bytes);
      if (padded_row_bytes > row_bytes) {
        memset(staging + (cmp_usize)row * padded_row_bytes + row_bytes, 0,
               padded_row_bytes - row_bytes);
      }
    }
  }

  memset(&dst, 0, sizeof(dst));
  dst.texture = texture->wgpu_texture;
  dst.mipLevel = 0u;
  dst.origin.x = (cmp_u32)x;
  dst.origin.y = (cmp_u32)y;
  dst.origin.z = 0u;
  dst.aspect = WGPUTextureAspect_All;

  memset(&layout, 0, sizeof(layout));
  layout.offset = 0u;
  layout.bytesPerRow = (cmp_u32)padded_row_bytes;
  layout.rowsPerImage = (cmp_u32)height;

  memset(&extent, 0, sizeof(extent));
  extent.width = (cmp_u32)width;
  extent.height = (cmp_u32)height;
  extent.depthOrArrayLayers = 1u;

  wgpuQueueWriteTexture(backend->wgpu.queue, &dst, staging, padded_size,
                        &layout, &extent);

  if (staging != pixels) {
    rc = backend->allocator.free(backend->allocator.ctx, staging);
    CMP_WEB_RETURN_IF_ERROR(rc);
  }

  return CMP_OK;
}

static int cmp_web_wgpu_create_texture(struct CMPWebBackend *backend,
                                      cmp_i32 width, cmp_i32 height,
                                      cmp_u32 format, const void *pixels,
                                      cmp_usize size, CMPHandle *out_texture) {
  CMPWebTexture *texture;
  WGPUTextureDescriptor texture_desc;
  WGPUTextureViewDescriptor view_desc;
  WGPUBindGroupEntry bind_entries[3];
  WGPUBindGroupDescriptor bind_desc;
  cmp_usize expected_size;
  cmp_usize row_size;
  cmp_u32 bytes_per_pixel;
  WGPUTextureFormat wgpu_format;
  CMPBool alpha_only;
  int rc;

  if (backend == NULL || out_texture == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (format != CMP_TEX_FORMAT_RGBA8 && format != CMP_TEX_FORMAT_BGRA8 &&
      format != CMP_TEX_FORMAT_A8) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (pixels == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  out_texture->id = 0u;
  out_texture->generation = 0u;

  rc = cmp_web_wgpu_init(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (backend->wgpu.device == NULL || backend->wgpu.bind_group_layout == NULL ||
      backend->wgpu.uniform_buffer == NULL || backend->wgpu.sampler == NULL) {
    return CMP_ERR_STATE;
  }

  bytes_per_pixel = 4u;
  wgpu_format = WGPUTextureFormat_RGBA8Unorm;
  alpha_only = CMP_FALSE;
  if (format == CMP_TEX_FORMAT_BGRA8) {
    wgpu_format = WGPUTextureFormat_BGRA8Unorm;
  } else if (format == CMP_TEX_FORMAT_A8) {
    bytes_per_pixel = 1u;
    wgpu_format = WGPUTextureFormat_R8Unorm;
    alpha_only = CMP_TRUE;
  }

  rc = cmp_web_mul_size((cmp_usize)width, (cmp_usize)height, &row_size);
  CMP_WEB_RETURN_IF_ERROR(rc);
  rc = cmp_web_mul_size(row_size, (cmp_usize)bytes_per_pixel, &expected_size);
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (size != 0 && size < expected_size) {
    return CMP_ERR_RANGE;
  }

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPWebTexture),
                                (void **)&texture);
  CMP_WEB_RETURN_IF_ERROR(rc);

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
  texture_desc.size.width = (cmp_u32)width;
  texture_desc.size.height = (cmp_u32)height;
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
    return CMP_ERR_UNKNOWN;
  }

  memset(&view_desc, 0, sizeof(view_desc));
  texture->wgpu_view = wgpuTextureCreateView(texture->wgpu_texture, &view_desc);
  if (texture->wgpu_view == NULL) {
    wgpuTextureRelease(texture->wgpu_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return CMP_ERR_UNKNOWN;
  }

  memset(bind_entries, 0, sizeof(bind_entries));
  bind_entries[0].binding = 0u;
  bind_entries[0].buffer = backend->wgpu.uniform_buffer;
  bind_entries[0].offset = 0u;
  bind_entries[0].size = (cmp_usize)sizeof(CMPWebGPUUniforms);
  bind_entries[1].binding = 1u;
  bind_entries[1].sampler = backend->wgpu.sampler;
  bind_entries[2].binding = 2u;
  bind_entries[2].textureView = texture->wgpu_view;

  memset(&bind_desc, 0, sizeof(bind_desc));
  bind_desc.layout = backend->wgpu.bind_group_layout;
  bind_desc.entryCount = (cmp_u32)CMP_COUNTOF(bind_entries);
  bind_desc.entries = bind_entries;

  texture->wgpu_bind_group =
      wgpuDeviceCreateBindGroup(backend->wgpu.device, &bind_desc);
  if (texture->wgpu_bind_group == NULL) {
    wgpuTextureViewRelease(texture->wgpu_view);
    wgpuTextureRelease(texture->wgpu_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return CMP_ERR_UNKNOWN;
  }

  if (pixels != NULL && size != 0) {
    rc = cmp_web_wgpu_write_texture(backend, texture, 0, 0, width, height,
                                   pixels, size);
    if (rc != CMP_OK) {
      wgpuBindGroupRelease(texture->wgpu_bind_group);
      wgpuTextureViewRelease(texture->wgpu_view);
      wgpuTextureRelease(texture->wgpu_texture);
      backend->allocator.free(backend->allocator.ctx, texture);
      return rc;
    }
  }

  rc = cmp_object_header_init(&texture->header, CMP_WEB_TYPE_TEXTURE, 0,
                             &g_cmp_web_texture_vtable);
  if (rc != CMP_OK) {
    wgpuBindGroupRelease(texture->wgpu_bind_group);
    wgpuTextureViewRelease(texture->wgpu_view);
    wgpuTextureRelease(texture->wgpu_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &texture->header);
  if (rc != CMP_OK) {
    wgpuBindGroupRelease(texture->wgpu_bind_group);
    wgpuTextureViewRelease(texture->wgpu_view);
    wgpuTextureRelease(texture->wgpu_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  *out_texture = texture->header.handle;
  return CMP_OK;
}

static int cmp_web_wgpu_update_texture(struct CMPWebBackend *backend,
                                      CMPWebTexture *resolved, cmp_i32 x,
                                      cmp_i32 y, cmp_i32 width, cmp_i32 height,
                                      const void *pixels, cmp_usize size) {
  int rc;

  if (backend == NULL || resolved == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (x < 0 || y < 0 || width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (pixels == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (x + width > resolved->width || y + height > resolved->height) {
    return CMP_ERR_RANGE;
  }
  if (pixels == NULL && size == 0) {
    return CMP_OK;
  }

  rc = cmp_web_wgpu_write_texture(backend, resolved, x, y, width, height, pixels,
                                 size);
  CMP_WEB_RETURN_IF_ERROR(rc);

  return CMP_OK;
}
#endif

static int cmp_web_gfx_create_texture(void *gfx, cmp_i32 width, cmp_i32 height,
                                     cmp_u32 format, const void *pixels,
                                     cmp_usize size, CMPHandle *out_texture) {
  struct CMPWebBackend *backend;
  CMPWebTexture *texture;
  GLenum gl_format;
  GLuint gl_texture;
  cmp_usize expected_size;
  cmp_usize row_size;
  cmp_u32 bytes_per_pixel;
  void *upload_pixels;
  int rc;

  if (gfx == NULL || out_texture == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (format != CMP_TEX_FORMAT_RGBA8 && format != CMP_TEX_FORMAT_BGRA8 &&
      format != CMP_TEX_FORMAT_A8) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (pixels == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)gfx;
  out_texture->id = 0u;
  out_texture->generation = 0u;

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.create_texture");
  CMP_WEB_RETURN_IF_ERROR(rc);

#if defined(CMP_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return cmp_web_wgpu_create_texture(backend, width, height, format, pixels,
                                      size, out_texture);
  }
#endif

  bytes_per_pixel = 4u;
  gl_format = GL_RGBA;
  if (format == CMP_TEX_FORMAT_A8) {
    bytes_per_pixel = 1u;
    gl_format = GL_ALPHA;
  }

  rc = cmp_web_mul_size((cmp_usize)width, (cmp_usize)height, &row_size);
  CMP_WEB_RETURN_IF_ERROR(rc);
  rc = cmp_web_mul_size(row_size, (cmp_usize)bytes_per_pixel, &expected_size);
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (size != 0 && size < expected_size) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_web_gl_require_context(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPWebTexture),
                                (void **)&texture);
  CMP_WEB_RETURN_IF_ERROR(rc);

  memset(texture, 0, sizeof(*texture));
  texture->backend = backend;
  texture->width = width;
  texture->height = height;
  texture->format = format;
  texture->bytes_per_pixel = bytes_per_pixel;

  glGenTextures(1, &gl_texture);
  if (gl_texture == 0) {
    backend->allocator.free(backend->allocator.ctx, texture);
    return CMP_ERR_UNKNOWN;
  }

  glBindTexture(GL_TEXTURE_2D, gl_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  upload_pixels = (void *)pixels;
  if (format == CMP_TEX_FORMAT_BGRA8 && pixels != NULL && size != 0) {
    rc = cmp_web_convert_bgra(&backend->allocator, pixels, expected_size,
                             &upload_pixels);
    if (rc != CMP_OK) {
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

  rc = cmp_web_gl_check_error(backend, "gfx.create_texture");
  if (rc != CMP_OK) {
    glDeleteTextures(1, &gl_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  texture->gl_texture = gl_texture;

  rc = cmp_object_header_init(&texture->header, CMP_WEB_TYPE_TEXTURE, 0,
                             &g_cmp_web_texture_vtable);
  if (rc != CMP_OK) {
    glDeleteTextures(1, &gl_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &texture->header);
  if (rc != CMP_OK) {
    glDeleteTextures(1, &gl_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  *out_texture = texture->header.handle;
  return CMP_OK;
}

static int cmp_web_gfx_update_texture(void *gfx, CMPHandle texture, cmp_i32 x,
                                     cmp_i32 y, cmp_i32 width, cmp_i32 height,
                                     const void *pixels, cmp_usize size) {
  struct CMPWebBackend *backend;
  CMPWebTexture *resolved;
  cmp_usize expected_size;
  cmp_usize row_size;
  void *upload_pixels;
  GLenum gl_format;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (x < 0 || y < 0 || width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (pixels == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)gfx;
  rc = cmp_web_backend_resolve(backend, texture, CMP_WEB_TYPE_TEXTURE,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (x + width > resolved->width || y + height > resolved->height) {
    return CMP_ERR_RANGE;
  }

  if (pixels == NULL && size == 0) {
    return CMP_OK;
  }

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.update_texture");
  CMP_WEB_RETURN_IF_ERROR(rc);

#if defined(CMP_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return cmp_web_wgpu_update_texture(backend, resolved, x, y, width, height,
                                      pixels, size);
  }
#endif

  if (resolved->gl_texture == 0) {
    return CMP_ERR_STATE;
  }

  rc = cmp_web_mul_size((cmp_usize)width, (cmp_usize)height, &row_size);
  CMP_WEB_RETURN_IF_ERROR(rc);
  rc = cmp_web_mul_size(row_size, (cmp_usize)resolved->bytes_per_pixel,
                       &expected_size);
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (size != 0 && size < expected_size) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_web_gl_require_context(backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  gl_format = GL_RGBA;
  if (resolved->format == CMP_TEX_FORMAT_A8) {
    gl_format = GL_ALPHA;
  }

  upload_pixels = (void *)pixels;
  if (resolved->format == CMP_TEX_FORMAT_BGRA8 && pixels != NULL) {
    rc = cmp_web_convert_bgra(&backend->allocator, pixels, expected_size,
                             &upload_pixels);
    CMP_WEB_RETURN_IF_ERROR(rc);
  }

  glBindTexture(GL_TEXTURE_2D, resolved->gl_texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, gl_format,
                  GL_UNSIGNED_BYTE, upload_pixels);

  if (upload_pixels != pixels && upload_pixels != NULL) {
    backend->allocator.free(backend->allocator.ctx, upload_pixels);
  }

  return cmp_web_gl_check_error(backend, "gfx.update_texture");
}

static int cmp_web_gfx_destroy_texture(void *gfx, CMPHandle texture) {
  struct CMPWebBackend *backend;
  CMPWebTexture *resolved;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)gfx;
  rc = cmp_web_backend_resolve(backend, texture, CMP_WEB_TYPE_TEXTURE,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.destroy_texture");
  CMP_WEB_RETURN_IF_ERROR(rc);

  return cmp_object_release(&resolved->header);
}

static int cmp_web_gfx_draw_texture(void *gfx, CMPHandle texture,
                                   const CMPRect *src, const CMPRect *dst,
                                   CMPScalar opacity) {
  struct CMPWebBackend *backend;
  CMPWebTexture *resolved;
  CMPWebVertex verts[4];
  CMPRect rect_copy;
  CMPColor color;
  CMPScalar inv_w;
  CMPScalar inv_h;
  CMPScalar u0;
  CMPScalar v0;
  CMPScalar u1;
  CMPScalar v1;
  int rc;

  if (gfx == NULL || src == NULL || dst == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (opacity < 0.0f || opacity > 1.0f) {
    return CMP_ERR_RANGE;
  }
  if (dst->width < 0.0f || dst->height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWebBackend *)gfx;
  rc = cmp_web_backend_resolve(backend, texture, CMP_WEB_TYPE_TEXTURE,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (src->x < 0.0f || src->y < 0.0f || src->width < 0.0f ||
      src->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (src->x + src->width > (CMPScalar)resolved->width ||
      src->y + src->height > (CMPScalar)resolved->height) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_texture");
  CMP_WEB_RETURN_IF_ERROR(rc);

  inv_w = 1.0f / (CMPScalar)resolved->width;
  inv_h = 1.0f / (CMPScalar)resolved->height;
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

#if defined(CMP_WEBGPU_AVAILABLE)
  if (backend->use_webgpu) {
    return cmp_web_wgpu_draw_quad(backend, verts, color, CMP_TRUE, resolved,
                                 opacity, &rect_copy, 0.0f);
  }
#endif

  return cmp_web_gfx_draw_quad(backend, verts, color, CMP_TRUE,
                              resolved->gl_texture, opacity, &rect_copy, 0.0f);
}

static const CMPGfxVTable g_cmp_web_gfx_vtable = {
    cmp_web_gfx_begin_frame,    cmp_web_gfx_end_frame,
    cmp_web_gfx_clear,          cmp_web_gfx_draw_rect,
    cmp_web_gfx_draw_line,      cmp_web_gfx_draw_path,
    cmp_web_gfx_push_clip,      cmp_web_gfx_pop_clip,
    cmp_web_gfx_set_transform,  cmp_web_gfx_create_texture,
    cmp_web_gfx_update_texture, cmp_web_gfx_destroy_texture,
    cmp_web_gfx_draw_texture};

static int cmp_web_text_create_font(void *text, const char *utf8_family,
                                   cmp_i32 size_px, cmp_i32 weight, CMPBool italic,
                                   CMPHandle *out_font) {
  struct CMPWebBackend *backend;
  CMPWebFont *font;
  int rc;

  if (text == NULL || out_font == NULL || utf8_family == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (weight < 100 || weight > 900) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWebBackend *)text;
  out_font->id = 0u;
  out_font->generation = 0u;

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.create_font");
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPWebFont),
                                (void **)&font);
  CMP_WEB_RETURN_IF_ERROR(rc);

  memset(font, 0, sizeof(*font));
  font->backend = backend;
  font->size_px = size_px;
  font->weight = weight;
  font->italic = italic ? CMP_TRUE : CMP_FALSE;

  rc = cmp_object_header_init(&font->header, CMP_WEB_TYPE_FONT, 0,
                             &g_cmp_web_font_vtable);
  CMP_WEB_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, font));

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &font->header);
  CMP_WEB_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, font));

  *out_font = font->header.handle;
  return CMP_OK;
}

static int cmp_web_text_destroy_font(void *text, CMPHandle font) {
  struct CMPWebBackend *backend;
  CMPWebFont *resolved;
  int rc;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)text;
  rc = cmp_web_backend_resolve(backend, font, CMP_WEB_TYPE_FONT,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.destroy_font");
  CMP_WEB_RETURN_IF_ERROR(rc);

  return cmp_object_release(&resolved->header);
}

static int cmp_web_text_measure_text(void *text, CMPHandle font, const char *utf8,
                                    cmp_usize utf8_len, CMPScalar *out_width,
                                    CMPScalar *out_height,
                                    CMPScalar *out_baseline) {
  struct CMPWebBackend *backend;
  CMPWebFont *resolved;
  CMPScalar size;
  int rc;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)text;
  rc = cmp_web_backend_resolve(backend, font, CMP_WEB_TYPE_FONT,
                              (void **)&resolved);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.measure_text");
  CMP_WEB_RETURN_IF_ERROR(rc);

  size = (CMPScalar)resolved->size_px;
  *out_width = size * (CMPScalar)utf8_len * 0.5f;
  *out_height = size;
  *out_baseline = size * 0.8f;
  return CMP_OK;
}

static int cmp_web_text_draw_text(void *text, CMPHandle font, const char *utf8,
                                 cmp_usize utf8_len, CMPScalar x, CMPScalar y,
                                 CMPColor color) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(x);
  CMP_UNUSED(y);
  CMP_UNUSED(color);

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)text;
  rc = cmp_web_backend_resolve(backend, font, CMP_WEB_TYPE_FONT, NULL);
  CMP_WEB_RETURN_IF_ERROR(rc);

  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.draw_text");
  CMP_WEB_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static const CMPTextVTable g_cmp_web_text_vtable = {
    cmp_web_text_create_font, cmp_web_text_destroy_font, cmp_web_text_measure_text,
    cmp_web_text_draw_text};

static int cmp_web_io_read_file(void *io, const char *utf8_path, void *buffer,
                               cmp_usize buffer_size, cmp_usize *out_read) {
  struct CMPWebBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_read == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (buffer == NULL && buffer_size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)io;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.read_file");
  CMP_WEB_RETURN_IF_ERROR(rc);

  *out_read = 0;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_io_read_file_alloc(void *io, const char *utf8_path,
                                     const CMPAllocator *allocator,
                                     void **out_data, cmp_usize *out_size) {
  struct CMPWebBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || allocator == NULL ||
      out_data == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)io;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.read_file_alloc");
  CMP_WEB_RETURN_IF_ERROR(rc);

  *out_data = NULL;
  *out_size = 0;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_io_write_file(void *io, const char *utf8_path,
                                const void *data, cmp_usize size,
                                CMPBool overwrite) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(overwrite);

  if (io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (data == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)io;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.write_file");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_io_file_exists(void *io, const char *utf8_path,
                                 CMPBool *out_exists) {
  struct CMPWebBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_exists == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)io;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.file_exists");
  CMP_WEB_RETURN_IF_ERROR(rc);

  *out_exists = CMP_FALSE;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_io_delete_file(void *io, const char *utf8_path) {
  struct CMPWebBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)io;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.delete_file");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_io_stat_file(void *io, const char *utf8_path,
                               CMPFileInfo *out_info) {
  struct CMPWebBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_info == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)io;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.stat_file");
  CMP_WEB_RETURN_IF_ERROR(rc);

  memset(out_info, 0, sizeof(*out_info));
  return CMP_ERR_UNSUPPORTED;
}

static const CMPIOVTable g_cmp_web_io_vtable = {
    cmp_web_io_read_file,   cmp_web_io_read_file_alloc, cmp_web_io_write_file,
    cmp_web_io_file_exists, cmp_web_io_delete_file,     cmp_web_io_stat_file};

static int cmp_web_sensors_is_available(void *sensors, cmp_u32 type,
                                       CMPBool *out_available) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL || out_available == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)sensors;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.is_available");
  CMP_WEB_RETURN_IF_ERROR(rc);

  *out_available = CMP_FALSE;
  return CMP_OK;
}

static int cmp_web_sensors_start(void *sensors, cmp_u32 type) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)sensors;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.start");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_sensors_stop(void *sensors, cmp_u32 type) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)sensors;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.stop");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_sensors_read(void *sensors, cmp_u32 type,
                               CMPSensorReading *out_reading,
                               CMPBool *out_has_reading) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL || out_reading == NULL || out_has_reading == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)sensors;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.read");
  CMP_WEB_RETURN_IF_ERROR(rc);

  memset(out_reading, 0, sizeof(*out_reading));
  *out_has_reading = CMP_FALSE;
  return CMP_OK;
}

static const CMPSensorsVTable g_cmp_web_sensors_vtable = {
    cmp_web_sensors_is_available, cmp_web_sensors_start, cmp_web_sensors_stop,
    cmp_web_sensors_read};

static int cmp_web_camera_open(void *camera, cmp_u32 camera_id) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(camera_id);

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)camera;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.open");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_camera_open_with_config(void *camera,
                                          const CMPCameraConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return cmp_web_camera_open(camera, config->camera_id);
}

static int cmp_web_camera_close(void *camera) {
  struct CMPWebBackend *backend;
  int rc;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)camera;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.close");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_camera_start(void *camera) {
  struct CMPWebBackend *backend;
  int rc;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)camera;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.start");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_camera_stop(void *camera) {
  struct CMPWebBackend *backend;
  int rc;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)camera;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.stop");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_camera_read_frame(void *camera, CMPCameraFrame *out_frame,
                                    CMPBool *out_has_frame) {
  struct CMPWebBackend *backend;
  int rc;

  if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)camera;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.read_frame");
  CMP_WEB_RETURN_IF_ERROR(rc);

  memset(out_frame, 0, sizeof(*out_frame));
  *out_has_frame = CMP_FALSE;
  return CMP_OK;
}

static const CMPCameraVTable g_cmp_web_camera_vtable = {
    cmp_web_camera_open,  cmp_web_camera_open_with_config,
    cmp_web_camera_close, cmp_web_camera_start,
    cmp_web_camera_stop,  cmp_web_camera_read_frame};

static int cmp_web_network_request(void *net, const CMPNetworkRequest *request,
                                  const CMPAllocator *allocator,
                                  CMPNetworkResponse *out_response) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(request);
  CMP_UNUSED(allocator);

  if (net == NULL || out_response == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)net;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "network.request");
  CMP_WEB_RETURN_IF_ERROR(rc);

  memset(out_response, 0, sizeof(*out_response));
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_network_free_response(void *net, const CMPAllocator *allocator,
                                        CMPNetworkResponse *response) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(allocator);
  CMP_UNUSED(response);

  if (net == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)net;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "network.free_response");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static const CMPNetworkVTable g_cmp_web_network_vtable = {
    cmp_web_network_request, cmp_web_network_free_response};

static int cmp_web_tasks_thread_create(void *tasks, CMPThreadFn entry, void *user,
                                      CMPHandle *out_thread) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(entry);
  CMP_UNUSED(user);

  if (tasks == NULL || out_thread == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)tasks;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.thread_create");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_tasks_thread_join(void *tasks, CMPHandle thread) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(thread);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)tasks;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.thread_join");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_tasks_mutex_create(void *tasks, CMPHandle *out_mutex) {
  struct CMPWebBackend *backend;
  int rc;

  if (tasks == NULL || out_mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)tasks;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_create");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_tasks_mutex_destroy(void *tasks, CMPHandle mutex) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(mutex);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)tasks;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_destroy");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_tasks_mutex_lock(void *tasks, CMPHandle mutex) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(mutex);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)tasks;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_lock");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_tasks_mutex_unlock(void *tasks, CMPHandle mutex) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(mutex);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)tasks;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_unlock");
  CMP_WEB_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_tasks_sleep_ms(void *tasks, cmp_u32 ms) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(ms);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)tasks;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.sleep_ms");
  CMP_WEB_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return CMP_ERR_UNSUPPORTED;
  }
  return CMP_OK;
}

static int cmp_web_tasks_post(void *tasks, CMPTaskFn fn, void *user) {
  struct CMPWebBackend *backend;
  int rc;

  if (tasks == NULL || fn == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)tasks;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.post");
  CMP_WEB_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return CMP_ERR_UNSUPPORTED;
  }
  return fn(user);
}

static int cmp_web_tasks_post_delayed(void *tasks, CMPTaskFn fn, void *user,
                                     cmp_u32 delay_ms) {
  struct CMPWebBackend *backend;
  int rc;

  CMP_UNUSED(delay_ms);

  if (tasks == NULL || fn == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)tasks;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.post_delayed");
  CMP_WEB_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return CMP_ERR_UNSUPPORTED;
  }
  return fn(user);
}

static const CMPTasksVTable g_cmp_web_tasks_vtable = {
    cmp_web_tasks_thread_create, cmp_web_tasks_thread_join,
    cmp_web_tasks_mutex_create,  cmp_web_tasks_mutex_destroy,
    cmp_web_tasks_mutex_lock,    cmp_web_tasks_mutex_unlock,
    cmp_web_tasks_sleep_ms,      cmp_web_tasks_post,
    cmp_web_tasks_post_delayed};

static int cmp_web_env_get_io(void *env, CMPIO *out_io) {
  struct CMPWebBackend *backend;
  int rc;

  if (env == NULL || out_io == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)env;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_io");
  CMP_WEB_RETURN_IF_ERROR(rc);

  *out_io = backend->io;
  return CMP_OK;
}

static int cmp_web_env_get_sensors(void *env, CMPSensors *out_sensors) {
  struct CMPWebBackend *backend;
  int rc;

  if (env == NULL || out_sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)env;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_sensors");
  CMP_WEB_RETURN_IF_ERROR(rc);

  *out_sensors = backend->sensors;
  return CMP_OK;
}

static int cmp_web_env_get_camera(void *env, CMPCamera *out_camera) {
  struct CMPWebBackend *backend;
  int rc;

  if (env == NULL || out_camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)env;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_camera");
  CMP_WEB_RETURN_IF_ERROR(rc);

  *out_camera = backend->camera;
  return CMP_OK;
}

static int cmp_web_env_get_image(void *env, CMPImage *out_image) {
  struct CMPWebBackend *backend;
  int rc;

  if (env == NULL || out_image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)env;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_image");
  CMP_WEB_RETURN_IF_ERROR(rc);

  out_image->ctx = NULL;
  out_image->vtable = NULL;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_env_get_video(void *env, CMPVideo *out_video) {
  struct CMPWebBackend *backend;
  int rc;

  if (env == NULL || out_video == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)env;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_video");
  CMP_WEB_RETURN_IF_ERROR(rc);

  out_video->ctx = NULL;
  out_video->vtable = NULL;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_env_get_audio(void *env, CMPAudio *out_audio) {
  struct CMPWebBackend *backend;
  int rc;

  if (env == NULL || out_audio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)env;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_audio");
  CMP_WEB_RETURN_IF_ERROR(rc);

  out_audio->ctx = NULL;
  out_audio->vtable = NULL;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_web_env_get_network(void *env, CMPNetwork *out_network) {
  struct CMPWebBackend *backend;
  int rc;

  if (env == NULL || out_network == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)env;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_network");
  CMP_WEB_RETURN_IF_ERROR(rc);

  *out_network = backend->network;
  return CMP_OK;
}

static int cmp_web_env_get_tasks(void *env, CMPTasks *out_tasks) {
  struct CMPWebBackend *backend;
  int rc;

  if (env == NULL || out_tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)env;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_tasks");
  CMP_WEB_RETURN_IF_ERROR(rc);

  *out_tasks = backend->tasks;
  return CMP_OK;
}

static int cmp_web_env_get_time_ms(void *env, cmp_u32 *out_time_ms) {
  struct CMPWebBackend *backend;
  int rc;

  if (env == NULL || out_time_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWebBackend *)env;
  rc = cmp_web_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_time_ms");
  CMP_WEB_RETURN_IF_ERROR(rc);

  return cmp_web_backend_time_now_ms(out_time_ms);
}

static const CMPEnvVTable g_cmp_web_env_vtable = {
    cmp_web_env_get_io,     cmp_web_env_get_sensors, cmp_web_env_get_camera,
    cmp_web_env_get_image,  cmp_web_env_get_video,   cmp_web_env_get_audio,
    cmp_web_env_get_network, cmp_web_env_get_tasks,  cmp_web_env_get_time_ms};

int CMP_CALL cmp_web_backend_create(const CMPWebBackendConfig *config,
                                  CMPWebBackend **out_backend) {
  CMPWebBackendConfig local_config;
  CMPAllocator allocator;
  struct CMPWebBackend *backend;
  int rc;

  if (out_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = cmp_web_backend_config_init(&local_config);
    CMP_WEB_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = cmp_web_backend_validate_config(config);
  CMP_WEB_RETURN_IF_ERROR(rc);

  if (config->allocator == NULL) {
    rc = cmp_get_default_allocator(&allocator);
    CMP_WEB_RETURN_IF_ERROR(rc);
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = allocator.alloc(allocator.ctx, sizeof(CMPWebBackend), (void **)&backend);
  CMP_WEB_RETURN_IF_ERROR(rc);

  memset(backend, 0, sizeof(*backend));
  backend->allocator = allocator;
  backend->log_enabled = config->enable_logging ? CMP_TRUE : CMP_FALSE;
  backend->inline_tasks = config->inline_tasks ? CMP_TRUE : CMP_FALSE;
  backend->clipboard_limit = config->clipboard_limit;

  rc = cmp_web_backend_copy_string(&allocator, config->utf8_canvas_id,
                                  &backend->canvas_id);
  if (rc != CMP_OK) {
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  if (backend->log_enabled) {
    rc = cmp_log_init(&allocator);
    if (rc != CMP_OK && rc != CMP_ERR_STATE) {
      allocator.free(allocator.ctx, backend->canvas_id);
      allocator.free(allocator.ctx, backend);
      return rc;
    }
    backend->log_owner = (rc == CMP_OK) ? CMP_TRUE : CMP_FALSE;
  }

  rc = cmp_handle_system_default_create(config->handle_capacity, &allocator,
                                       &backend->handles);
  if (rc != CMP_OK) {
    if (backend->log_owner) {
      cmp_log_shutdown();
    }
    allocator.free(allocator.ctx, backend->canvas_id);
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  backend->ws.ctx = backend;
  backend->ws.vtable = &g_cmp_web_ws_vtable;
  backend->gfx.ctx = backend;
  backend->gfx.vtable = &g_cmp_web_gfx_vtable;
  backend->gfx.text_vtable = &g_cmp_web_text_vtable;
  backend->env.ctx = backend;
  backend->env.vtable = &g_cmp_web_env_vtable;
  backend->io.ctx = backend;
  backend->io.vtable = &g_cmp_web_io_vtable;
  backend->sensors.ctx = backend;
  backend->sensors.vtable = &g_cmp_web_sensors_vtable;
  backend->camera.ctx = backend;
  backend->camera.vtable = &g_cmp_web_camera_vtable;
  backend->network.ctx = backend;
  backend->network.vtable = &g_cmp_web_network_vtable;
  backend->tasks.ctx = backend;
  backend->tasks.vtable = &g_cmp_web_tasks_vtable;
  backend->initialized = CMP_TRUE;

  *out_backend = backend;
  return CMP_OK;
}

int CMP_CALL cmp_web_backend_destroy(CMPWebBackend *backend) {
  int rc;
  int first_error;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }

  rc = cmp_handle_system_default_destroy(&backend->handles);
  CMP_WEB_RETURN_IF_ERROR(rc);

  first_error = CMP_OK;

  if (backend->clipboard != NULL) {
    rc = backend->allocator.free(backend->allocator.ctx, backend->clipboard);
    if (rc != CMP_OK && first_error == CMP_OK) {
      first_error = rc;
    }
    backend->clipboard = NULL;
    backend->clipboard_capacity = 0;
    backend->clipboard_length = 0;
  }

#if defined(CMP_WEBGPU_AVAILABLE)
  rc = cmp_web_wgpu_shutdown(backend);
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }
  backend->use_webgpu = CMP_FALSE;
#endif

  if (backend->gl.initialized) {
    rc = cmp_web_gl_require_context(backend);
    if (rc != CMP_OK && first_error == CMP_OK) {
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
    backend->gl.initialized = CMP_FALSE;
  }

  if (backend->canvas_id != NULL) {
    rc = backend->allocator.free(backend->allocator.ctx, backend->canvas_id);
    if (rc != CMP_OK && first_error == CMP_OK) {
      first_error = rc;
    }
    backend->canvas_id = NULL;
  }

  if (backend->log_owner) {
    rc = cmp_log_shutdown();
    if (rc != CMP_OK && first_error == CMP_OK) {
      first_error = rc;
    }
  }

  backend->initialized = CMP_FALSE;
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
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }

  return first_error;
}

int CMP_CALL cmp_web_backend_get_ws(CMPWebBackend *backend, CMPWS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  *out_ws = backend->ws;
  return CMP_OK;
}

int CMP_CALL cmp_web_backend_get_gfx(CMPWebBackend *backend, CMPGfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  *out_gfx = backend->gfx;
  return CMP_OK;
}

int CMP_CALL cmp_web_backend_get_env(CMPWebBackend *backend, CMPEnv *out_env) {
  if (backend == NULL || out_env == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  *out_env = backend->env;
  return CMP_OK;
}

/* GCOVR_EXCL_STOP */
#else

int CMP_CALL
cmp_web_backend_create(const CMPWebBackendConfig *config, /* GCOVR_EXCL_LINE */
                      CMPWebBackend **out_backend) {     /* GCOVR_EXCL_LINE */
  CMPWebBackendConfig local_config;                      /* GCOVR_EXCL_LINE */
  int rc;

  if (out_backend == NULL) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = cmp_web_backend_config_init(&local_config);
    CMP_WEB_RETURN_IF_ERROR(rc);
    config = &local_config; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_web_backend_validate_config(config); /* GCOVR_EXCL_LINE */
  CMP_WEB_RETURN_IF_ERROR(rc);

  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_web_backend_destroy(CMPWebBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_web_backend_get_ws(CMPWebBackend *backend, CMPWS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_ws, 0, sizeof(*out_ws));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_web_backend_get_gfx(CMPWebBackend *backend, CMPGfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_gfx, 0, sizeof(*out_gfx));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_web_backend_get_env(CMPWebBackend *backend, CMPEnv *out_env) {
  if (backend == NULL || out_env == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_env, 0, sizeof(*out_env));
  return CMP_ERR_UNSUPPORTED;
}

#endif
