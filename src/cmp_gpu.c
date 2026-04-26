/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief dummy_begin_frame
 *
 * @param gpu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int dummy_begin_frame(cmp_gpu_t *gpu) {
  (void)gpu;
  return CMP_SUCCESS;
}

/**
 * @brief dummy_end_frame
 *
 * @param gpu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int dummy_end_frame(cmp_gpu_t *gpu) {
  (void)gpu;
  return CMP_SUCCESS;
}

/**
 * @brief dummy_destroy
 *
 * @param gpu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int dummy_destroy(cmp_gpu_t *gpu) {
  (void)gpu;
  return CMP_SUCCESS;
}

static const cmp_gpu_vtable_t dummy_vtable = {dummy_begin_frame,
                                              dummy_end_frame, dummy_destroy};

/**
 * @brief cmp_gpu_create
 *
 * @param preferred_backend Parameter description.
 * @param out_gpu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gpu_create(cmp_gpu_backend_type_t preferred_backend,
                   cmp_gpu_t **out_gpu) {
  int rc = CMP_SUCCESS;
  cmp_gpu_t *gpu = NULL;

  if (!out_gpu) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gpu_create: Invalid argument (out_gpu=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_gpu_t), (void **)&gpu);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_gpu_create: Out of memory\n");
    return rc;
  }

  memset(gpu, 0, sizeof(cmp_gpu_t));
  gpu->backend = preferred_backend;
  gpu->vtable = &dummy_vtable;
  *out_gpu = gpu;
  return rc;
}

/**
 * @brief cmp_gpu_destroy
 *
 * @param gpu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gpu_destroy(cmp_gpu_t *gpu) {
  int rc = CMP_SUCCESS;

  if (!gpu) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gpu_destroy: Invalid argument (gpu=NULL)\n");
    return rc;
  }

  if (gpu->vtable && gpu->vtable->destroy) {
    rc = gpu->vtable->destroy(gpu);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_gpu_destroy: vtable->destroy failed\n");
    }
  }
  CMP_FREE(gpu);
  return rc;
}

/**
 * @brief cmp_gpu_begin_frame
 *
 * @param gpu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gpu_begin_frame(cmp_gpu_t *gpu) {
  int rc = CMP_SUCCESS;

  if (!gpu || !gpu->vtable || !gpu->vtable->begin_frame) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gpu_begin_frame: Invalid argument\n");
    return rc;
  }

  rc = gpu->vtable->begin_frame(gpu);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_gpu_begin_frame: vtable->begin_frame failed\n");
  }
  return rc;
}

/**
 * @brief cmp_gpu_end_frame
 *
 * @param gpu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gpu_end_frame(cmp_gpu_t *gpu) {
  int rc = CMP_SUCCESS;

  if (!gpu || !gpu->vtable || !gpu->vtable->end_frame) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gpu_end_frame: Invalid argument\n");
    return rc;
  }

  rc = gpu->vtable->end_frame(gpu);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_gpu_end_frame: vtable->end_frame failed\n");
  }
  return rc;
}

/**
 * @brief cmp_vbo_create
 *
 * @param out_vbo Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vbo_create(cmp_vbo_t **out_vbo) {
  int rc = CMP_SUCCESS;
  cmp_vbo_t *vbo = NULL;

  if (!out_vbo) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_vbo_create: Invalid argument (out_vbo=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_vbo_t), (void **)&vbo);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_vbo_create: Out of memory\n");
    return rc;
  }

  memset(vbo, 0, sizeof(cmp_vbo_t));
  *out_vbo = vbo;
  return rc;
}

/**
 * @brief cmp_vbo_append
 *
 * @param vbo Parameter description.
 * @param vertices Parameter description.
 * @param count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vbo_append(cmp_vbo_t *vbo, const float *vertices, size_t count) {
  int rc = CMP_SUCCESS;
  size_t new_cap;
  float *new_data = NULL;

  if (!vbo || !vertices || count == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_vbo_append: Invalid argument\n");
    return rc;
  }

  if (vbo->count + count > vbo->capacity) {
    new_cap = vbo->capacity == 0 ? 1024 : vbo->capacity * 2;
    while (new_cap < vbo->count + count)
      new_cap *= 2;

    rc = CMP_MALLOC(new_cap * sizeof(float), (void **)&new_data);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_vbo_append: Out of memory\n");
      return rc;
    }

    if (vbo->data) {
      memcpy(new_data, vbo->data, vbo->count * sizeof(float));
      CMP_FREE(vbo->data);
    }
    vbo->data = new_data;
    vbo->capacity = new_cap;
  }

  memcpy(vbo->data + vbo->count, vertices, count * sizeof(float));
  vbo->count += count;
  return rc;
}

/**
 * @brief cmp_vbo_destroy
 *
 * @param vbo Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vbo_destroy(cmp_vbo_t *vbo) {
  int rc = CMP_SUCCESS;

  if (!vbo) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_vbo_destroy: Invalid argument (vbo=NULL)\n");
    return rc;
  }

  if (vbo->data) {
    CMP_FREE(vbo->data);
  }
  CMP_FREE(vbo);
  return rc;
}

/**
 * @brief cmp_ubo_create
 *
 * @param size Parameter description.
 * @param out_ubo Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ubo_create(size_t size, cmp_ubo_t **out_ubo) {
  int rc = CMP_SUCCESS;
  cmp_ubo_t *ubo = NULL;

  if (!out_ubo || size == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ubo_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ubo_t), (void **)&ubo);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ubo_create: Out of memory for ubo\n");
    return rc;
  }

  memset(ubo, 0, sizeof(cmp_ubo_t));

  rc = CMP_MALLOC(size, &ubo->data);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(ubo);
    LOG_DEBUG("Error in cmp_ubo_create: Out of memory for data\n");
    return rc;
  }

  memset(ubo->data, 0, size);
  ubo->size = size;
  *out_ubo = ubo;
  return rc;
}

/**
 * @brief cmp_ubo_update
 *
 * @param ubo Parameter description.
 * @param data Parameter description.
 * @param size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ubo_update(cmp_ubo_t *ubo, const void *data, size_t size) {
  int rc = CMP_SUCCESS;

  if (!ubo || !data || size == 0 || size > ubo->size) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ubo_update: Invalid argument\n");
    return rc;
  }

  memcpy(ubo->data, data, size);
  return rc;
}

/**
 * @brief cmp_ubo_destroy
 *
 * @param ubo Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ubo_destroy(cmp_ubo_t *ubo) {
  int rc = CMP_SUCCESS;

  if (!ubo) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ubo_destroy: Invalid argument (ubo=NULL)\n");
    return rc;
  }

  if (ubo->data) {
    CMP_FREE(ubo->data);
  }
  CMP_FREE(ubo);
  return rc;
}
/**
 * @brief cmp_draw_call_optimizer_create
 *
 * @param out_opt Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_draw_call_optimizer_create(cmp_draw_call_optimizer_t **out_opt) {
  int rc = CMP_SUCCESS;
  cmp_draw_call_optimizer_t *opt = NULL;

  if (!out_opt) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_draw_call_optimizer_create: Invalid argument "
              "(out_opt=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_draw_call_optimizer_t), (void **)&opt);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_draw_call_optimizer_create: Out of memory\n");
    return rc;
  }

  memset(opt, 0, sizeof(cmp_draw_call_optimizer_t));
  *out_opt = opt;
  return rc;
}

/**
 * @brief cmp_draw_call_optimizer_add
 *
 * @param opt Parameter description.
 * @param call Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_draw_call_optimizer_add(cmp_draw_call_optimizer_t *opt,
                                const cmp_draw_call_t *call) {
  int rc = CMP_SUCCESS;
  size_t new_cap;
  cmp_draw_call_t *new_calls = NULL;

  if (!opt || !call) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_draw_call_optimizer_add: Invalid argument\n");
    return rc;
  }

  if (opt->count >= opt->capacity) {
    new_cap = opt->capacity == 0 ? 32 : opt->capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_draw_call_t), (void **)&new_calls);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_draw_call_optimizer_add: Out of memory\n");
      return rc;
    }
    if (opt->calls) {
      memcpy(new_calls, opt->calls, opt->count * sizeof(cmp_draw_call_t));
      CMP_FREE(opt->calls);
    }
    opt->calls = new_calls;
    opt->capacity = new_cap;
  }
  memcpy(&opt->calls[opt->count], call, sizeof(cmp_draw_call_t));
  opt->count++;
  return rc;
}

/**
 * @brief cmp_draw_call_optimizer_optimize
 *
 * @param opt Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_draw_call_optimizer_optimize(cmp_draw_call_optimizer_t *opt) {
  int rc = CMP_SUCCESS;
  cmp_draw_call_t *optimized = NULL;
  size_t optimized_count = 0, i;

  if (!opt) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_draw_call_optimizer_optimize: Invalid argument\n");
    return rc;
  }

  if (opt->count <= 1)
    return rc;

  rc = CMP_MALLOC(opt->count * sizeof(cmp_draw_call_t), (void **)&optimized);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_draw_call_optimizer_optimize: Out of memory\n");
    return rc;
  }

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
  return rc;
}

/**
 * @brief cmp_draw_call_optimizer_destroy
 *
 * @param opt Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_draw_call_optimizer_destroy(cmp_draw_call_optimizer_t *opt) {
  int rc = CMP_SUCCESS;

  if (!opt) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_draw_call_optimizer_destroy: Invalid argument\n");
    return rc;
  }

  if (opt->calls)
    CMP_FREE(opt->calls);
  CMP_FREE(opt);
  return rc;
}

struct cmp_command_buffer {
  cmp_gpu_t *gpu;
  int is_secondary;
  cmp_draw_call_t *commands;
  int command_count;
  int command_capacity;
  int is_recording;
};

/**
 * @brief cmp_command_buffer_create
 *
 * @param gpu Parameter description.
 * @param is_secondary Parameter description.
 * @param out_cb Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_buffer_create(cmp_gpu_t *gpu, int is_secondary,
                              cmp_command_buffer_t **out_cb) {
  int rc = CMP_SUCCESS;
  struct cmp_command_buffer *cb = NULL;

  if (!gpu || !out_cb) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_command_buffer_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_command_buffer), (void **)&cb);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_command_buffer_create: Out of memory for cb\n");
    return rc;
  }

  memset(cb, 0, sizeof(struct cmp_command_buffer));
  cb->gpu = gpu;
  cb->is_secondary = is_secondary;
  cb->command_capacity = 256;

  rc = CMP_MALLOC(sizeof(cmp_draw_call_t) * cb->command_capacity,
                  (void **)&cb->commands);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(cb);
    LOG_DEBUG(
        "Error in cmp_command_buffer_create: Out of memory for commands\n");
    return rc;
  }

  memset(cb->commands, 0, sizeof(cmp_draw_call_t) * cb->command_capacity);
  *out_cb = (cmp_command_buffer_t *)cb;
  return rc;
}

/**
 * @brief cmp_command_buffer_destroy
 *
 * @param cb Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_buffer_destroy(cmp_command_buffer_t *cb) {
  int rc = CMP_SUCCESS;
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_command_buffer_destroy: Invalid argument\n");
    return rc;
  }

  if (ctx->commands)
    CMP_FREE(ctx->commands);
  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_command_buffer_begin
 *
 * @param cb Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_buffer_begin(cmp_command_buffer_t *cb) {
  int rc = CMP_SUCCESS;
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_command_buffer_begin: Invalid argument\n");
    return rc;
  }

  ctx->command_count = 0;
  ctx->is_recording = 1;
  return rc;
}

/**
 * @brief cmp_command_buffer_end
 *
 * @param cb Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_buffer_end(cmp_command_buffer_t *cb) {
  int rc = CMP_SUCCESS;
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_command_buffer_end: Invalid argument\n");
    return rc;
  }

  ctx->is_recording = 0;
  return rc;
}

/**
 * @brief cmp_command_buffer_draw
 *
 * @param cb Parameter description.
 * @param call Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_buffer_draw(cmp_command_buffer_t *cb,
                            const cmp_draw_call_t *call) {
  int rc = CMP_SUCCESS;
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;

  if (!ctx || !call) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_command_buffer_draw: Invalid argument\n");
    return rc;
  }

  if (!ctx->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    LOG_DEBUG("Error in cmp_command_buffer_draw: Not recording\n");
    return rc;
  }

  if (ctx->command_count >= ctx->command_capacity) {
    cmp_draw_call_t *new_cmds = NULL;
    int new_cap = ctx->command_capacity * 2;
    rc = CMP_MALLOC(sizeof(cmp_draw_call_t) * new_cap, (void **)&new_cmds);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_command_buffer_draw: Out of memory\n");
      return rc;
    }
    memcpy(new_cmds, ctx->commands,
           sizeof(cmp_draw_call_t) * ctx->command_count);
    CMP_FREE(ctx->commands);
    ctx->commands = new_cmds;
    ctx->command_capacity = new_cap;
  }
  ctx->commands[ctx->command_count++] = *call;
  return rc;
}

/**
 * @brief cmp_command_buffer_execute_commands
 *
 * @param primary Parameter description.
 * @param secondaries Parameter description.
 * @param count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_buffer_execute_commands(cmp_command_buffer_t *primary,
                                        cmp_command_buffer_t **secondaries,
                                        int count) {
  int rc = CMP_SUCCESS;
  struct cmp_command_buffer *p_ctx = (struct cmp_command_buffer *)primary;
  int i;

  if (!p_ctx || !secondaries) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_command_buffer_execute_commands: Invalid argument\n");
    return rc;
  }

  if (p_ctx->is_secondary) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_command_buffer_execute_commands: Cannot execute "
              "secondaries in a secondary\n");
    return rc;
  }

  if (!p_ctx->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    LOG_DEBUG("Error in cmp_command_buffer_execute_commands: Primary is not "
              "recording\n");
    return rc;
  }

  for (i = 0; i < count; i++) {
    struct cmp_command_buffer *s_ctx =
        (struct cmp_command_buffer *)secondaries[i];
    int j;

    if (!s_ctx || !s_ctx->is_secondary) {
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("Error in cmp_command_buffer_execute_commands: Invalid "
                "secondary buffer\n");
      return rc;
    }

    if (s_ctx->is_recording) {
      rc = CMP_ERROR_INVALID_STATE;
      LOG_DEBUG("Error in cmp_command_buffer_execute_commands: Secondary is "
                "still recording\n");
      return rc;
    }

    for (j = 0; j < s_ctx->command_count; j++) {
      rc = cmp_command_buffer_draw((cmp_command_buffer_t *)p_ctx,
                                   &s_ctx->commands[j]);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Error in cmp_command_buffer_execute_commands: "
                  "cmp_command_buffer_draw failed\n");
        return rc;
      }
    }
  }
  return rc;
}

struct cmp_render_graph {
  cmp_render_pass_config_t passes[64];
  int pass_count;
};

/**
 * @brief cmp_render_graph_create
 *
 * @param out_graph Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_render_graph_create(cmp_render_graph_t **out_graph) {
  int rc = CMP_SUCCESS;
  struct cmp_render_graph *graph = NULL;

  if (!out_graph) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_render_graph_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_render_graph), (void **)&graph);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_render_graph_create: Out of memory\n");
    return rc;
  }

  memset(graph, 0, sizeof(struct cmp_render_graph));
  *out_graph = (cmp_render_graph_t *)graph;
  return rc;
}

/**
 * @brief cmp_render_graph_destroy
 *
 * @param graph Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_render_graph_destroy(cmp_render_graph_t *graph) {
  int rc = CMP_SUCCESS;
  struct cmp_render_graph *ctx = (struct cmp_render_graph *)graph;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_render_graph_destroy: Invalid argument\n");
    return rc;
  }

  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_render_graph_add_pass
 *
 * @param graph Parameter description.
 * @param config Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_render_graph_add_pass(cmp_render_graph_t *graph,
                              const cmp_render_pass_config_t *config) {
  int rc = CMP_SUCCESS;
  struct cmp_render_graph *ctx = (struct cmp_render_graph *)graph;

  if (!ctx || !config) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_render_graph_add_pass: Invalid argument\n");
    return rc;
  }

  if (ctx->pass_count >= 64) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_render_graph_add_pass: Max passes reached\n");
    return rc;
  }

  ctx->passes[ctx->pass_count++] = *config;
  return rc;
}

/**
 * @brief _cmp_render_graph_find_pass
 *
 * @param ctx Parameter description.
 * @param id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int _cmp_render_graph_find_pass(struct cmp_render_graph *ctx, int id) {
  int i;
  for (i = 0; i < ctx->pass_count; ++i) {
    if (ctx->passes[i].id == id)
      return i;
  }
  return -1;
}

/**
 * @brief cmp_render_graph_execute
 *
 * @param graph Parameter description.
 * @param cb Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_render_graph_execute(cmp_render_graph_t *graph,
                             cmp_command_buffer_t *cb) {
  int rc = CMP_SUCCESS;
  struct cmp_render_graph *ctx = (struct cmp_render_graph *)graph;
  int executed[64];
  int executed_count = 0;
  int progress;
  int i, j;
  int all_deps_met;

  if (!ctx || !cb) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_render_graph_execute: Invalid argument\n");
    return rc;
  }

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
    rc = CMP_ERROR_INVALID_STATE; /* Circular dependency or missing pass */
    LOG_DEBUG("Error in cmp_render_graph_execute: Circular dependency or "
              "missing pass\n");
    return rc;
  }

  return rc;
}

struct cmp_pso {
  cmp_pipeline_state_t state;
  void *internal_handle;
};

struct cmp_pso_cache {
  cmp_pso_t *psos[256];
  int count;
};

/**
 * @brief cmp_pso_cache_create
 *
 * @param out_cache Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pso_cache_create(cmp_pso_cache_t **out_cache) {
  int rc = CMP_SUCCESS;
  struct cmp_pso_cache *cache = NULL;

  if (!out_cache) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pso_cache_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_pso_cache), (void **)&cache);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_pso_cache_create: Out of memory\n");
    return rc;
  }

  memset(cache, 0, sizeof(struct cmp_pso_cache));
  *out_cache = (cmp_pso_cache_t *)cache;
  return rc;
}

/**
 * @brief cmp_pso_cache_destroy
 *
 * @param cache Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pso_cache_destroy(cmp_pso_cache_t *cache) {
  int rc = CMP_SUCCESS;
  struct cmp_pso_cache *ctx = (struct cmp_pso_cache *)cache;
  int i;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pso_cache_destroy: Invalid argument\n");
    return rc;
  }

  for (i = 0; i < ctx->count; i++) {
    CMP_FREE(ctx->psos[i]);
  }
  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_pso_cache_get_or_create
 *
 * @param cache Parameter description.
 * @param state Parameter description.
 * @param out_pso Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pso_cache_get_or_create(cmp_pso_cache_t *cache,
                                const cmp_pipeline_state_t *state,
                                cmp_pso_t **out_pso) {
  int rc = CMP_SUCCESS;
  struct cmp_pso_cache *ctx = (struct cmp_pso_cache *)cache;
  int i;
  struct cmp_pso *new_pso = NULL;

  if (!ctx || !state || !out_pso) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pso_cache_get_or_create: Invalid argument\n");
    return rc;
  }

  for (i = 0; i < ctx->count; i++) {
    if (memcmp(&ctx->psos[i]->state, state, sizeof(cmp_pipeline_state_t)) ==
        0) {
      *out_pso = (cmp_pso_t *)ctx->psos[i];
      return rc;
    }
  }

  if (ctx->count >= 256) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_pso_cache_get_or_create: Cache full\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_pso), (void **)&new_pso);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_pso_cache_get_or_create: Out of memory\n");
    return rc;
  }

  memset(new_pso, 0, sizeof(struct cmp_pso));
  new_pso->state = *state;
  new_pso->internal_handle = (void *)(size_t)(0xCAFEBABE + ctx->count);

  ctx->psos[ctx->count++] = new_pso;
  *out_pso = (cmp_pso_t *)new_pso;
  return rc;
}

/**
 * @brief cmp_command_buffer_bind_pso
 *
 * @param cb Parameter description.
 * @param pso Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_buffer_bind_pso(cmp_command_buffer_t *cb, cmp_pso_t *pso) {
  int rc = CMP_SUCCESS;
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;

  if (!ctx || !pso) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_command_buffer_bind_pso: Invalid argument\n");
    return rc;
  }

  if (!ctx->is_recording) {
    rc = CMP_ERROR_INVALID_STATE;
    LOG_DEBUG("Error in cmp_command_buffer_bind_pso: Not recording\n");
    return rc;
  }

  return rc;
}

/**
 * @brief cmp_shader_compile_spirv
 *
 * @param source Parameter description.
 * @param size Parameter description.
 * @param out_shader Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_compile_spirv(const char *source, size_t size,
                             cmp_shader_t **out_shader) {
  int rc = CMP_SUCCESS;
  struct cmp_shader *shader = NULL;

  (void)source;
  (void)size;

  if (!out_shader) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_shader_compile_spirv: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_shader), (void **)&shader);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_shader_compile_spirv: Out of memory\n");
    return rc;
  }

  memset(shader, 0, sizeof(struct cmp_shader));
  shader->internal_handle = (void *)0x5B1210;
  *out_shader = shader;
  return rc;
}

/**
 * @brief cmp_shader_compile_msl
 *
 * @param source Parameter description.
 * @param size Parameter description.
 * @param out_shader Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_compile_msl(const char *source, size_t size,
                           cmp_shader_t **out_shader) {
  int rc = cmp_shader_compile_spirv(source, size, out_shader);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_shader_compile_msl: Fallback to spirv failed\n");
  }
  return rc;
}

struct cmp_gpu_allocator {
  cmp_gpu_t *gpu;
  size_t block_size;
  void *base_mem;
  size_t used;
};

/**
 * @brief cmp_gpu_allocator_create
 *
 * @param gpu Parameter description.
 * @param block_size Parameter description.
 * @param out_allocator Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gpu_allocator_create(cmp_gpu_t *gpu, size_t block_size,
                             cmp_gpu_allocator_t **out_allocator) {
  int rc = CMP_SUCCESS;
  struct cmp_gpu_allocator *alloc = NULL;

  if (!gpu || !out_allocator) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gpu_allocator_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_gpu_allocator), (void **)&alloc);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_gpu_allocator_create: Out of memory\n");
    return rc;
  }

  memset(alloc, 0, sizeof(struct cmp_gpu_allocator));
  alloc->gpu = gpu;
  alloc->block_size = block_size;
  alloc->used = 0;

  rc = CMP_MALLOC(block_size, &alloc->base_mem);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(alloc);
    LOG_DEBUG("Error in cmp_gpu_allocator_create: Out of memory allocating "
              "base_mem\n");
    return rc;
  }

  memset(alloc->base_mem, 0, block_size);
  *out_allocator = (cmp_gpu_allocator_t *)alloc;
  return rc;
}

/**
 * @brief cmp_gpu_allocator_destroy
 *
 * @param allocator Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gpu_allocator_destroy(cmp_gpu_allocator_t *allocator) {
  int rc = CMP_SUCCESS;
  struct cmp_gpu_allocator *alloc = (struct cmp_gpu_allocator *)allocator;

  if (!alloc) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gpu_allocator_destroy: Invalid argument\n");
    return rc;
  }

  CMP_FREE(alloc->base_mem);
  CMP_FREE(alloc);
  return rc;
}

/**
 * @brief cmp_gpu_allocator_alloc
 *
 * @param allocator Parameter description.
 * @param size Parameter description.
 * @param alignment Parameter description.
 * @param out_mem Parameter description.
 * @param out_offset Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gpu_allocator_alloc(cmp_gpu_allocator_t *allocator, size_t size,
                            size_t alignment, void **out_mem,
                            size_t *out_offset) {
  int rc = CMP_SUCCESS;
  struct cmp_gpu_allocator *alloc = (struct cmp_gpu_allocator *)allocator;
  size_t padding;

  if (!alloc || !out_mem || !out_offset) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_gpu_allocator_alloc: Invalid argument\n");
    return rc;
  }

  padding = (alignment - (alloc->used % alignment)) % alignment;
  if (alloc->used + padding + size > alloc->block_size) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_gpu_allocator_alloc: Block full\n");
    return rc;
  }

  alloc->used += padding;
  *out_offset = alloc->used;
  *out_mem = (void *)((char *)alloc->base_mem + alloc->used);
  alloc->used += size;
  return rc;
}

/**
 * @brief cmp_gpu_allocator_free
 *
 * @param allocator Parameter description.
 * @param mem Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gpu_allocator_free(cmp_gpu_allocator_t *allocator, void *mem) {
  int rc = CMP_SUCCESS;
  (void)allocator;
  (void)mem;
  return rc;
}

struct cmp_atlas {
  cmp_gpu_t *gpu;
  int width, height;
  void *pixels;
  int current_x, current_y;
  int current_row_height;
};

/**
 * @brief cmp_atlas_create
 *
 * @param gpu Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @param out_atlas Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_atlas_create(cmp_gpu_t *gpu, int width, int height,
                     cmp_atlas_t **out_atlas) {
  int rc = CMP_SUCCESS;
  struct cmp_atlas *atlas = NULL;

  if (!gpu || !out_atlas || width <= 0 || height <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_atlas_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_atlas), (void **)&atlas);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_atlas_create: Out of memory\n");
    return rc;
  }

  memset(atlas, 0, sizeof(struct cmp_atlas));
  atlas->gpu = gpu;
  atlas->width = width;
  atlas->height = height;

  rc = CMP_MALLOC(width * height * 4, &atlas->pixels);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(atlas);
    LOG_DEBUG("Error in cmp_atlas_create: Out of memory for pixels\n");
    return rc;
  }

  memset(atlas->pixels, 0, width * height * 4);
  *out_atlas = (cmp_atlas_t *)atlas;
  return rc;
}

/**
 * @brief cmp_atlas_destroy
 *
 * @param atlas Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_atlas_destroy(cmp_atlas_t *atlas) {
  int rc = CMP_SUCCESS;
  struct cmp_atlas *ctx = (struct cmp_atlas *)atlas;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_atlas_destroy: Invalid argument\n");
    return rc;
  }

  CMP_FREE(ctx->pixels);
  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_atlas_insert
 *
 * @param atlas Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @param pixels Parameter description.
 * @param out_x Parameter description.
 * @param out_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_atlas_insert(cmp_atlas_t *atlas, int width, int height,
                     const void *pixels, int *out_x, int *out_y) {
  int rc = CMP_SUCCESS;
  struct cmp_atlas *ctx = (struct cmp_atlas *)atlas;

  if (!ctx || !pixels || !out_x || !out_y || width <= 0 || height <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_atlas_insert: Invalid argument\n");
    return rc;
  }

  if (ctx->current_x + width > ctx->width) {
    ctx->current_x = 0;
    ctx->current_y += ctx->current_row_height;
    ctx->current_row_height = 0;
  }

  if (ctx->current_y + height > ctx->height) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_atlas_insert: Atlas full\n");
    return rc;
  }

  *out_x = ctx->current_x;
  *out_y = ctx->current_y;

  ctx->current_x += width;
  if (height > ctx->current_row_height)
    ctx->current_row_height = height;

  return rc;
}

/**
 * @brief cmp_atlas_evict
 *
 * @param atlas Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_atlas_evict(cmp_atlas_t *atlas) {
  int rc = CMP_SUCCESS;
  struct cmp_atlas *ctx = (struct cmp_atlas *)atlas;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_atlas_evict: Invalid argument\n");
    return rc;
  }

  ctx->current_x = 0;
  ctx->current_y = 0;
  ctx->current_row_height = 0;
  memset(ctx->pixels, 0, ctx->width * ctx->height * 4);
  return rc;
}

/**
 * @brief cmp_tex_compression_decode_astc
 *
 * @param data Parameter description.
 * @param size Parameter description.
 * @param out_rgba Parameter description.
 * @param out_width Parameter description.
 * @param out_height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tex_compression_decode_astc(const void *data, size_t size,
                                    void **out_rgba, int *out_width,
                                    int *out_height) {
  int rc = CMP_SUCCESS;
  (void)data;
  (void)size;

  if (!out_rgba || !out_width || !out_height) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_tex_compression_decode_astc: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(64 * 64 * 4, out_rgba);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_tex_compression_decode_astc: Out of memory\n");
    return rc;
  }

  memset(*out_rgba, 0, 64 * 64 * 4);
  *out_width = 64;
  *out_height = 64;
  return rc;
}

/**
 * @brief cmp_tex_compression_decode_bc7
 *
 * @param data Parameter description.
 * @param size Parameter description.
 * @param out_rgba Parameter description.
 * @param out_width Parameter description.
 * @param out_height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tex_compression_decode_bc7(const void *data, size_t size,
                                   void **out_rgba, int *out_width,
                                   int *out_height) {
  int rc = cmp_tex_compression_decode_astc(data, size, out_rgba, out_width,
                                           out_height);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_tex_compression_decode_bc7: Fallback to astc failed\n");
  }
  return rc;
}

/**
 * @brief cmp_frustum_culling_test
 *
 * @param node_rect Parameter description.
 * @param viewport_rect Parameter description.
 * @param out_is_visible Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_frustum_culling_test(const cmp_rect_t *node_rect,
                             const cmp_rect_t *viewport_rect,
                             int *out_is_visible) {
  int rc = CMP_SUCCESS;

  if (!node_rect || !viewport_rect || !out_is_visible) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_frustum_culling_test: Invalid argument\n");
    return rc;
  }

  if (node_rect->x + node_rect->width < viewport_rect->x ||
      node_rect->x > viewport_rect->x + viewport_rect->width ||
      node_rect->y + node_rect->height < viewport_rect->y ||
      node_rect->y > viewport_rect->y + viewport_rect->height) {
    *out_is_visible = 0;
  } else {
    *out_is_visible = 1;
  }
  return rc;
}
