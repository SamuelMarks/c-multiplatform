/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */
int cmp_gpu_create(cmp_gpu_backend_type_t preferred_backend,
                   cmp_gpu_t **out_gpu) {
  cmp_gpu_t *gpu;
  if (!out_gpu)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_gpu_t), (void **)&gpu) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(gpu, 0, sizeof(cmp_gpu_t));
  gpu->backend = preferred_backend;
  *out_gpu = gpu;
  return CMP_SUCCESS;
}
int cmp_gpu_destroy(cmp_gpu_t *gpu) {
  if (!gpu)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(gpu);
  return CMP_SUCCESS;
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
