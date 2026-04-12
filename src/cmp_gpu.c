/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int dummy_begin_frame(cmp_gpu_t *gpu) {
  (void)gpu;
  return CMP_SUCCESS;
}

static int dummy_end_frame(cmp_gpu_t *gpu) {
  (void)gpu;
  return CMP_SUCCESS;
}

static int dummy_destroy(cmp_gpu_t *gpu) {
  (void)gpu;
  return CMP_SUCCESS;
}

static const cmp_gpu_vtable_t dummy_vtable = {dummy_begin_frame,
                                              dummy_end_frame, dummy_destroy};

int cmp_gpu_create(cmp_gpu_backend_type_t preferred_backend,
                   cmp_gpu_t **out_gpu) {
  cmp_gpu_t *gpu;
  if (!out_gpu)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_gpu_t), (void **)&gpu) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(gpu, 0, sizeof(cmp_gpu_t));
  gpu->backend = preferred_backend;
  gpu->vtable = &dummy_vtable;
  *out_gpu = gpu;
  return CMP_SUCCESS;
}
int cmp_gpu_destroy(cmp_gpu_t *gpu) {
  if (!gpu)
    return CMP_ERROR_INVALID_ARG;
  if (gpu->vtable && gpu->vtable->destroy) {
    gpu->vtable->destroy(gpu);
  }
  CMP_FREE(gpu);
  return CMP_SUCCESS;
}

int cmp_gpu_begin_frame(cmp_gpu_t *gpu) {
  if (!gpu || !gpu->vtable || !gpu->vtable->begin_frame)
    return CMP_ERROR_INVALID_ARG;
  return gpu->vtable->begin_frame(gpu);
}

int cmp_gpu_end_frame(cmp_gpu_t *gpu) {
  if (!gpu || !gpu->vtable || !gpu->vtable->end_frame)
    return CMP_ERROR_INVALID_ARG;
  return gpu->vtable->end_frame(gpu);
}
int cmp_vbo_create(cmp_vbo_t **out_vbo) {
  cmp_vbo_t *vbo;
  if (!out_vbo)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_vbo_t), (void **)&vbo) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(vbo, 0, sizeof(cmp_vbo_t));
  *out_vbo = vbo;
  return CMP_SUCCESS;
}
int cmp_vbo_append(cmp_vbo_t *vbo, const float *vertices, size_t count) {
  size_t new_cap;
  float *new_data;
  if (!vbo || !vertices || count == 0)
    return CMP_ERROR_INVALID_ARG;
  if (vbo->count + count > vbo->capacity) {
    new_cap = vbo->capacity == 0 ? 1024 : vbo->capacity * 2;
    while (new_cap < vbo->count + count)
      new_cap *= 2;
    if (CMP_MALLOC(new_cap * sizeof(float), (void **)&new_data) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
    if (vbo->data) {
      memcpy(new_data, vbo->data, vbo->count * sizeof(float));
      CMP_FREE(vbo->data);
    }
    vbo->data = new_data;
    vbo->capacity = new_cap;
  }
  memcpy(vbo->data + vbo->count, vertices, count * sizeof(float));
  vbo->count += count;
  return CMP_SUCCESS;
}
int cmp_vbo_destroy(cmp_vbo_t *vbo) {
  if (!vbo)
    return CMP_ERROR_INVALID_ARG;
  if (vbo->data)
    CMP_FREE(vbo->data);
  CMP_FREE(vbo);
  return CMP_SUCCESS;
}
int cmp_ubo_create(size_t size, cmp_ubo_t **out_ubo) {
  cmp_ubo_t *ubo;
  if (!out_ubo || size == 0)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_ubo_t), (void **)&ubo) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(ubo, 0, sizeof(cmp_ubo_t));
  if (CMP_MALLOC(size, &ubo->data) != CMP_SUCCESS) {
    CMP_FREE(ubo);
    return CMP_ERROR_OOM;
  }
  memset(ubo->data, 0, size);
  ubo->size = size;
  *out_ubo = ubo;
  return CMP_SUCCESS;
}
int cmp_ubo_update(cmp_ubo_t *ubo, const void *data, size_t size) {
  if (!ubo || !data || size == 0 || size > ubo->size)
    return CMP_ERROR_INVALID_ARG;
  memcpy(ubo->data, data, size);
  return CMP_SUCCESS;
}
int cmp_ubo_destroy(cmp_ubo_t *ubo) {
  if (!ubo)
    return CMP_ERROR_INVALID_ARG;
  if (ubo->data)
    CMP_FREE(ubo->data);
  CMP_FREE(ubo);
  return CMP_SUCCESS;
}
int cmp_draw_call_optimizer_create(cmp_draw_call_optimizer_t **out_opt) {
  cmp_draw_call_optimizer_t *opt;
  if (!out_opt)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_draw_call_optimizer_t), (void **)&opt) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(opt, 0, sizeof(cmp_draw_call_optimizer_t));
  *out_opt = opt;
  return CMP_SUCCESS;
}
int cmp_draw_call_optimizer_add(cmp_draw_call_optimizer_t *opt,
                                const cmp_draw_call_t *call) {
  size_t new_cap;
  cmp_draw_call_t *new_calls;
  if (!opt || !call)
    return CMP_ERROR_INVALID_ARG;
  if (opt->count >= opt->capacity) {
    new_cap = opt->capacity == 0 ? 32 : opt->capacity * 2;
    if (CMP_MALLOC(new_cap * sizeof(cmp_draw_call_t), (void **)&new_calls) !=
        CMP_SUCCESS)
      return CMP_ERROR_OOM;
    if (opt->calls) {
      memcpy(new_calls, opt->calls, opt->count * sizeof(cmp_draw_call_t));
      CMP_FREE(opt->calls);
    }
    opt->calls = new_calls;
    opt->capacity = new_cap;
  }
  memcpy(&opt->calls[opt->count], call, sizeof(cmp_draw_call_t));
  opt->count++;
  return CMP_SUCCESS;
}
int cmp_draw_call_optimizer_optimize(cmp_draw_call_optimizer_t *opt) {
  cmp_draw_call_t *optimized;
  size_t optimized_count = 0, i;
  if (!opt)
    return CMP_ERROR_INVALID_ARG;
  if (opt->count <= 1)
    return CMP_SUCCESS;
  if (CMP_MALLOC(opt->count * sizeof(cmp_draw_call_t), (void **)&optimized) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memcpy(&optimized[0], &opt->calls[0], sizeof(cmp_draw_call_t));
  optimized_count++;
  for (i = 1; i < opt->count; i++) {
    cmp_draw_call_t *last = &optimized[optimized_count - 1];
    cmp_draw_call_t *curr = &opt->calls[i];
    if (last->texture_id == curr->texture_id &&
        last->shader_id == curr->shader_id &&
        last->blend_mode == curr->blend_mode &&
        last->scissor_enable == curr->scissor_enable &&
        (!last->scissor_enable ||
         (last->scissor_rect.x == curr->scissor_rect.x &&
          last->scissor_rect.y == curr->scissor_rect.y &&
          last->scissor_rect.width == curr->scissor_rect.width &&
          last->scissor_rect.height == curr->scissor_rect.height)) &&
        last->vertex_offset + last->vertex_count == curr->vertex_offset) {
      last->vertex_count += curr->vertex_count;
    } else {
      memcpy(&optimized[optimized_count], curr, sizeof(cmp_draw_call_t));
      optimized_count++;
    }
  }
  CMP_FREE(opt->calls);
  opt->calls = optimized;
  opt->count = optimized_count;
  opt->capacity = opt->count;
  return CMP_SUCCESS;
}
int cmp_draw_call_optimizer_destroy(cmp_draw_call_optimizer_t *opt) {
  if (!opt)
    return CMP_ERROR_INVALID_ARG;
  if (opt->calls)
    CMP_FREE(opt->calls);
  CMP_FREE(opt);
  return CMP_SUCCESS;
}

struct cmp_command_buffer {
  cmp_gpu_t *gpu;
  int is_secondary;
  cmp_draw_call_t *commands;
  int command_count;
  int command_capacity;
  int is_recording;
};

int cmp_command_buffer_create(cmp_gpu_t *gpu, int is_secondary,
                              cmp_command_buffer_t **out_cb) {
  struct cmp_command_buffer *cb;
  if (!gpu || !out_cb)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_command_buffer), (void **)&cb) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(cb, 0, sizeof(struct cmp_command_buffer));
  cb->gpu = gpu;
  cb->is_secondary = is_secondary;
  cb->command_capacity = 256;
  if (CMP_MALLOC(sizeof(cmp_draw_call_t) * cb->command_capacity,
                 (void **)&cb->commands) != CMP_SUCCESS) {
    CMP_FREE(cb);
    return CMP_ERROR_OOM;
  }
  memset(cb->commands, 0, sizeof(cmp_draw_call_t) * cb->command_capacity);
  *out_cb = (cmp_command_buffer_t *)cb;
  return CMP_SUCCESS;
}

int cmp_command_buffer_destroy(cmp_command_buffer_t *cb) {
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  if (ctx->commands)
    CMP_FREE(ctx->commands);
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_command_buffer_begin(cmp_command_buffer_t *cb) {
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->command_count = 0;
  ctx->is_recording = 1;
  return CMP_SUCCESS;
}

int cmp_command_buffer_end(cmp_command_buffer_t *cb) {
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->is_recording = 0;
  return CMP_SUCCESS;
}

int cmp_command_buffer_draw(cmp_command_buffer_t *cb,
                            const cmp_draw_call_t *call) {
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;
  if (!ctx || !call)
    return CMP_ERROR_INVALID_ARG;
  if (!ctx->is_recording)
    return CMP_ERROR_INVALID_STATE;

  if (ctx->command_count >= ctx->command_capacity) {
    cmp_draw_call_t *new_cmds;
    int new_cap = ctx->command_capacity * 2;
    if (CMP_MALLOC(sizeof(cmp_draw_call_t) * new_cap, (void **)&new_cmds) !=
        CMP_SUCCESS)
      return CMP_ERROR_OOM;
    memcpy(new_cmds, ctx->commands,
           sizeof(cmp_draw_call_t) * ctx->command_count);
    CMP_FREE(ctx->commands);
    ctx->commands = new_cmds;
    ctx->command_capacity = new_cap;
  }
  ctx->commands[ctx->command_count++] = *call;
  return CMP_SUCCESS;
}

int cmp_command_buffer_execute_commands(cmp_command_buffer_t *primary,
                                        cmp_command_buffer_t **secondaries,
                                        int count) {
  struct cmp_command_buffer *p_ctx = (struct cmp_command_buffer *)primary;
  int i;
  if (!p_ctx || !secondaries)
    return CMP_ERROR_INVALID_ARG;
  if (p_ctx->is_secondary)
    return CMP_ERROR_INVALID_ARG; /* Cannot execute secondaries in a secondary
                                   */
  if (!p_ctx->is_recording)
    return CMP_ERROR_INVALID_STATE;

  for (i = 0; i < count; i++) {
    struct cmp_command_buffer *s_ctx =
        (struct cmp_command_buffer *)secondaries[i];
    int j;
    if (!s_ctx || !s_ctx->is_secondary)
      return CMP_ERROR_INVALID_ARG;
    if (s_ctx->is_recording)
      return CMP_ERROR_INVALID_STATE;

    for (j = 0; j < s_ctx->command_count; j++) {
      int res = cmp_command_buffer_draw((cmp_command_buffer_t *)p_ctx,
                                        &s_ctx->commands[j]);
      if (res != CMP_SUCCESS)
        return res;
    }
  }
  return CMP_SUCCESS;
}

struct cmp_render_graph {
  cmp_render_pass_config_t passes[64];
  int pass_count;
};

int cmp_render_graph_create(cmp_render_graph_t **out_graph) {
  struct cmp_render_graph *graph;
  if (!out_graph)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_render_graph), (void **)&graph) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(graph, 0, sizeof(struct cmp_render_graph));
  *out_graph = (cmp_render_graph_t *)graph;
  return CMP_SUCCESS;
}

int cmp_render_graph_destroy(cmp_render_graph_t *graph) {
  struct cmp_render_graph *ctx = (struct cmp_render_graph *)graph;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_render_graph_add_pass(cmp_render_graph_t *graph,
                              const cmp_render_pass_config_t *config) {
  struct cmp_render_graph *ctx = (struct cmp_render_graph *)graph;
  if (!ctx || !config)
    return CMP_ERROR_INVALID_ARG;
  if (ctx->pass_count >= 64)
    return CMP_ERROR_OOM;
  ctx->passes[ctx->pass_count++] = *config;
  return CMP_SUCCESS;
}

static int _cmp_render_graph_find_pass(struct cmp_render_graph *ctx, int id) {
  int i;
  for (i = 0; i < ctx->pass_count; ++i) {
    if (ctx->passes[i].id == id)
      return i;
  }
  return -1;
}

int cmp_render_graph_execute(cmp_render_graph_t *graph,
                             cmp_command_buffer_t *cb) {
  struct cmp_render_graph *ctx = (struct cmp_render_graph *)graph;
  int executed[64];
  int executed_count = 0;
  int progress;
  int i, j;
  int all_deps_met;

  if (!ctx || !cb)
    return CMP_ERROR_INVALID_ARG;

  memset(executed, 0, sizeof(executed));

  do {
    progress = 0;
    for (i = 0; i < ctx->pass_count; i++) {
      if (executed[i])
        continue;

      all_deps_met = 1;
      for (j = 0; j < ctx->passes[i].dependency_count; j++) {
        int dep_id = ctx->passes[i].dependencies[j];
        int dep_idx = _cmp_render_graph_find_pass(ctx, dep_id);
        if (dep_idx == -1 || !executed[dep_idx]) {
          all_deps_met = 0;
          break;
        }
      }

      if (all_deps_met) {
        if (ctx->passes[i].execute_cb) {
          ctx->passes[i].execute_cb(cb, ctx->passes[i].user_data);
        }
        executed[i] = 1;
        executed_count++;
        progress = 1;
      }
    }
  } while (progress && executed_count < ctx->pass_count);

  if (executed_count < ctx->pass_count) {
    return CMP_ERROR_INVALID_STATE; /* Circular dependency or missing pass */
  }

  return CMP_SUCCESS;
}

struct cmp_pso {
  cmp_pipeline_state_t state;
  void *internal_handle;
};

struct cmp_pso_cache {
  cmp_pso_t *psos[256];
  int count;
};

int cmp_pso_cache_create(cmp_pso_cache_t **out_cache) {
  struct cmp_pso_cache *cache;
  if (!out_cache)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_pso_cache), (void **)&cache) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(cache, 0, sizeof(struct cmp_pso_cache));
  *out_cache = (cmp_pso_cache_t *)cache;
  return CMP_SUCCESS;
}

int cmp_pso_cache_destroy(cmp_pso_cache_t *cache) {
  struct cmp_pso_cache *ctx = (struct cmp_pso_cache *)cache;
  int i;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  for (i = 0; i < ctx->count; i++) {
    CMP_FREE(ctx->psos[i]);
  }
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_pso_cache_get_or_create(cmp_pso_cache_t *cache,
                                const cmp_pipeline_state_t *state,
                                cmp_pso_t **out_pso) {
  struct cmp_pso_cache *ctx = (struct cmp_pso_cache *)cache;
  int i;
  struct cmp_pso *new_pso;

  if (!ctx || !state || !out_pso)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < ctx->count; i++) {
    if (memcmp(&ctx->psos[i]->state, state, sizeof(cmp_pipeline_state_t)) ==
        0) {
      *out_pso = (cmp_pso_t *)ctx->psos[i];
      return CMP_SUCCESS;
    }
  }

  if (ctx->count >= 256)
    return CMP_ERROR_OOM;

  if (CMP_MALLOC(sizeof(struct cmp_pso), (void **)&new_pso) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(new_pso, 0, sizeof(struct cmp_pso));
  new_pso->state = *state;
  new_pso->internal_handle = (void *)(size_t)(0xCAFEBABE + ctx->count);

  ctx->psos[ctx->count++] = new_pso;
  *out_pso = (cmp_pso_t *)new_pso;
  return CMP_SUCCESS;
}

int cmp_command_buffer_bind_pso(cmp_command_buffer_t *cb, cmp_pso_t *pso) {
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;
  if (!ctx || !pso)
    return CMP_ERROR_INVALID_ARG;
  if (!ctx->is_recording)
    return CMP_ERROR_INVALID_STATE;
  return CMP_SUCCESS;
}

int cmp_shader_compile_spirv(const char *source, size_t size,
                             cmp_shader_t **out_shader) {
  struct cmp_shader *shader;
  (void)source;
  (void)size;
  if (!out_shader)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_shader), (void **)&shader) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(shader, 0, sizeof(struct cmp_shader));
  shader->internal_handle = (void *)0x5B1210;
  *out_shader = shader;
  return CMP_SUCCESS;
}

int cmp_shader_compile_msl(const char *source, size_t size,
                           cmp_shader_t **out_shader) {
  return cmp_shader_compile_spirv(source, size, out_shader);
}

struct cmp_gpu_allocator {
  cmp_gpu_t *gpu;
  size_t block_size;
  void *base_mem;
  size_t used;
};

int cmp_gpu_allocator_create(cmp_gpu_t *gpu, size_t block_size,
                             cmp_gpu_allocator_t **out_allocator) {
  struct cmp_gpu_allocator *alloc;
  if (!gpu || !out_allocator)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_gpu_allocator), (void **)&alloc) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(alloc, 0, sizeof(struct cmp_gpu_allocator));
  alloc->gpu = gpu;
  alloc->block_size = block_size;
  alloc->used = 0;
  if (CMP_MALLOC(block_size, &alloc->base_mem) != CMP_SUCCESS) {
    CMP_FREE(alloc);
    return CMP_ERROR_OOM;
  }
  memset(alloc->base_mem, 0, block_size);
  *out_allocator = (cmp_gpu_allocator_t *)alloc;
  return CMP_SUCCESS;
}

int cmp_gpu_allocator_destroy(cmp_gpu_allocator_t *allocator) {
  struct cmp_gpu_allocator *alloc = (struct cmp_gpu_allocator *)allocator;
  if (!alloc)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(alloc->base_mem);
  CMP_FREE(alloc);
  return CMP_SUCCESS;
}

int cmp_gpu_allocator_alloc(cmp_gpu_allocator_t *allocator, size_t size,
                            size_t alignment, void **out_mem,
                            size_t *out_offset) {
  struct cmp_gpu_allocator *alloc = (struct cmp_gpu_allocator *)allocator;
  size_t padding;
  if (!alloc || !out_mem || !out_offset)
    return CMP_ERROR_INVALID_ARG;

  padding = (alignment - (alloc->used % alignment)) % alignment;
  if (alloc->used + padding + size > alloc->block_size)
    return CMP_ERROR_OOM;

  alloc->used += padding;
  *out_offset = alloc->used;
  *out_mem = (void *)((char *)alloc->base_mem + alloc->used);
  alloc->used += size;
  return CMP_SUCCESS;
}

int cmp_gpu_allocator_free(cmp_gpu_allocator_t *allocator, void *mem) {
  (void)allocator;
  (void)mem;
  return CMP_SUCCESS;
}

struct cmp_atlas {
  cmp_gpu_t *gpu;
  int width, height;
  void *pixels;
  int current_x, current_y;
  int current_row_height;
};

int cmp_atlas_create(cmp_gpu_t *gpu, int width, int height,
                     cmp_atlas_t **out_atlas) {
  struct cmp_atlas *atlas;
  if (!gpu || !out_atlas || width <= 0 || height <= 0)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_atlas), (void **)&atlas) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(atlas, 0, sizeof(struct cmp_atlas));
  atlas->gpu = gpu;
  atlas->width = width;
  atlas->height = height;
  if (CMP_MALLOC(width * height * 4, &atlas->pixels) != CMP_SUCCESS) {
    CMP_FREE(atlas);
    return CMP_ERROR_OOM;
  }
  memset(atlas->pixels, 0, width * height * 4);
  *out_atlas = (cmp_atlas_t *)atlas;
  return CMP_SUCCESS;
}

int cmp_atlas_destroy(cmp_atlas_t *atlas) {
  struct cmp_atlas *ctx = (struct cmp_atlas *)atlas;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(ctx->pixels);
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_atlas_insert(cmp_atlas_t *atlas, int width, int height,
                     const void *pixels, int *out_x, int *out_y) {
  struct cmp_atlas *ctx = (struct cmp_atlas *)atlas;
  if (!ctx || !pixels || !out_x || !out_y || width <= 0 || height <= 0)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->current_x + width > ctx->width) {
    ctx->current_x = 0;
    ctx->current_y += ctx->current_row_height;
    ctx->current_row_height = 0;
  }

  if (ctx->current_y + height > ctx->height)
    return CMP_ERROR_OOM;

  *out_x = ctx->current_x;
  *out_y = ctx->current_y;

  ctx->current_x += width;
  if (height > ctx->current_row_height)
    ctx->current_row_height = height;

  return CMP_SUCCESS;
}

int cmp_atlas_evict(cmp_atlas_t *atlas) {
  struct cmp_atlas *ctx = (struct cmp_atlas *)atlas;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->current_x = 0;
  ctx->current_y = 0;
  ctx->current_row_height = 0;
  memset(ctx->pixels, 0, ctx->width * ctx->height * 4);
  return CMP_SUCCESS;
}

int cmp_tex_compression_decode_astc(const void *data, size_t size,
                                    void **out_rgba, int *out_width,
                                    int *out_height) {
  (void)data;
  (void)size;
  if (!out_rgba || !out_width || !out_height)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(64 * 64 * 4, out_rgba) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(*out_rgba, 0, 64 * 64 * 4);
  *out_width = 64;
  *out_height = 64;
  return CMP_SUCCESS;
}

int cmp_tex_compression_decode_bc7(const void *data, size_t size,
                                   void **out_rgba, int *out_width,
                                   int *out_height) {
  return cmp_tex_compression_decode_astc(data, size, out_rgba, out_width,
                                         out_height);
}

int cmp_frustum_culling_test(const cmp_rect_t *node_rect,
                             const cmp_rect_t *viewport_rect,
                             int *out_is_visible) {
  if (!node_rect || !viewport_rect || !out_is_visible)
    return CMP_ERROR_INVALID_ARG;

  if (node_rect->x + node_rect->width < viewport_rect->x ||
      node_rect->x > viewport_rect->x + viewport_rect->width ||
      node_rect->y + node_rect->height < viewport_rect->y ||
      node_rect->y > viewport_rect->y + viewport_rect->height) {
    *out_is_visible = 0;
  } else {
    *out_is_visible = 1;
  }
  return CMP_SUCCESS;
}
