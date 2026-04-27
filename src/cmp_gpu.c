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
  int rc;
  rc = 0;
  (void)gpu;
  cmp_log_debug("dummy_begin_frame: Executed mock begin_frame\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief dummy_end_frame
 *
 * @param gpu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int dummy_end_frame(cmp_gpu_t *gpu) {
  int rc;
  rc = 0;
  (void)gpu;
  cmp_log_debug("dummy_end_frame: Executed mock end_frame\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief dummy_destroy
 *
 * @param gpu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int dummy_destroy(cmp_gpu_t *gpu) {
  int rc;
  rc = 0;
  (void)gpu;
  cmp_log_debug("dummy_destroy: Executed mock destroy\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_gpu_t *gpu = NULL;

  if (out_gpu == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gpu_create: Invalid argument (out_gpu=NULL): %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_gpu_t), (void **)&gpu);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gpu_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(gpu, 0, sizeof(cmp_gpu_t));
  gpu->backend = preferred_backend;
  gpu->vtable = &dummy_vtable;
  *out_gpu = gpu;
  cmp_log_debug("cmp_gpu_create: Successfully created GPU context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_gpu_destroy
 *
 * @param gpu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gpu_destroy(cmp_gpu_t *gpu) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (gpu == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gpu_destroy: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (gpu->vtable != NULL && gpu->vtable->destroy != NULL) {
    rc = gpu->vtable->destroy(gpu);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_gpu_destroy: vtable->destroy failed\n");
    }
  }

  rc = CMP_FREE(gpu);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_gpu_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug("cmp_gpu_destroy: Successfully destroyed GPU context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_gpu_begin_frame
 *
 * @param gpu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gpu_begin_frame(cmp_gpu_t *gpu) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (gpu == NULL || gpu->vtable == NULL || gpu->vtable->begin_frame == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gpu_begin_frame: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = gpu->vtable->begin_frame(gpu);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_gpu_begin_frame: vtable->begin_frame failed\n");
  }
  if (rc != 0) {
    return rc;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (gpu == NULL || gpu->vtable == NULL || gpu->vtable->end_frame == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gpu_end_frame: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = gpu->vtable->end_frame(gpu);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_gpu_end_frame: vtable->end_frame failed\n");
  }
  if (rc != 0) {
    return rc;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_vbo_t *vbo = NULL;

  if (out_vbo == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_vbo_create: Invalid argument (out_vbo=NULL): %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_vbo_t), (void **)&vbo);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_vbo_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(vbo, 0, sizeof(cmp_vbo_t));
  *out_vbo = vbo;
  cmp_log_debug("cmp_vbo_create: Successfully created VBO\n");
  return CMP_SUCCESS;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t new_cap;
  float *new_data = NULL;

  if (vbo == NULL || vertices == NULL || count == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_vbo_append: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (vbo->count + count > vbo->capacity) {
    new_cap = vbo->capacity == 0 ? 1024 : vbo->capacity * 2;
    while (new_cap < vbo->count + count) {
      new_cap *= 2;
    }

    rc = CMP_MALLOC(new_cap * sizeof(float), (void **)&new_data);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_vbo_append: Out of memory: %s\n", err_str);
      if (rc != 0) {
        return rc;
      }
      return rc;
    }

    if (vbo->data != NULL) {
      memcpy(new_data, vbo->data, vbo->count * sizeof(float));
      rc = CMP_FREE(vbo->data);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug("cmp_vbo_append: CMP_FREE old buffer failed\n");
      }
    }
    vbo->data = new_data;
    vbo->capacity = new_cap;
  }

  memcpy(vbo->data + vbo->count, vertices, count * sizeof(float));
  vbo->count += count;
  cmp_log_debug("cmp_vbo_append: Appended to VBO\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_vbo_destroy
 *
 * @param vbo Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vbo_destroy(cmp_vbo_t *vbo) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (vbo == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_vbo_destroy: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (vbo->data != NULL) {
    rc = CMP_FREE(vbo->data);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_vbo_destroy: CMP_FREE data failed\n");
    }
  }
  rc = CMP_FREE(vbo);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_vbo_destroy: CMP_FREE vbo failed\n");
  }

  cmp_log_debug("cmp_vbo_destroy: Successfully destroyed VBO\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ubo_create
 *
 * @param size Parameter description.
 * @param out_ubo Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ubo_create(size_t size, cmp_ubo_t **out_ubo) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_ubo_t *ubo = NULL;

  if (out_ubo == NULL || size == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_ubo_create: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ubo_t), (void **)&ubo);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_ubo_create: Out of memory for ubo: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(ubo, 0, sizeof(cmp_ubo_t));

  rc = CMP_MALLOC(size, &ubo->data);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    rc = CMP_FREE(ubo);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_ubo_create: CMP_FREE ubo recovery failed\n");
    }
    cmp_log_debug("cmp_ubo_create: Out of memory for data: %s\n", err_str);
    return CMP_ERROR_OOM;
  }

  memset(ubo->data, 0, size);
  ubo->size = size;
  *out_ubo = ubo;
  cmp_log_debug("cmp_ubo_create: Successfully created UBO\n");
  return CMP_SUCCESS;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ubo == NULL || data == NULL || size == 0 || size > ubo->size) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_ubo_update: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memcpy(ubo->data, data, size);
  cmp_log_debug("cmp_ubo_update: Updated UBO\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ubo_destroy
 *
 * @param ubo Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ubo_destroy(cmp_ubo_t *ubo) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ubo == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_ubo_destroy: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ubo->data != NULL) {
    rc = CMP_FREE(ubo->data);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_ubo_destroy: CMP_FREE data failed\n");
    }
  }

  rc = CMP_FREE(ubo);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_ubo_destroy: CMP_FREE ubo failed\n");
  }

  cmp_log_debug("cmp_ubo_destroy: Successfully destroyed UBO\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_draw_call_optimizer_create
 *
 * @param out_opt Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_draw_call_optimizer_create(cmp_draw_call_optimizer_t **out_opt) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_draw_call_optimizer_t *opt = NULL;

  if (out_opt == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_draw_call_optimizer_create: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_draw_call_optimizer_t), (void **)&opt);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_draw_call_optimizer_create: Out of memory: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(opt, 0, sizeof(cmp_draw_call_optimizer_t));
  *out_opt = opt;
  cmp_log_debug(
      "cmp_draw_call_optimizer_create: Successfully created optimizer\n");
  return CMP_SUCCESS;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t new_cap;
  cmp_draw_call_t *new_calls = NULL;

  if (opt == NULL || call == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_draw_call_optimizer_add: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (opt->count >= opt->capacity) {
    new_cap = opt->capacity == 0 ? 32 : opt->capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_draw_call_t), (void **)&new_calls);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_draw_call_optimizer_add: Out of memory: %s\n",
                    err_str);
      if (rc != 0) {
        return rc;
      }
      return rc;
    }
    if (opt->calls != NULL) {
      memcpy(new_calls, opt->calls, opt->count * sizeof(cmp_draw_call_t));
      rc = CMP_FREE(opt->calls);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug(
            "cmp_draw_call_optimizer_add: CMP_FREE old array failed\n");
      }
    }
    opt->calls = new_calls;
    opt->capacity = new_cap;
  }
  memcpy(&opt->calls[opt->count], call, sizeof(cmp_draw_call_t));
  opt->count++;
  cmp_log_debug("cmp_draw_call_optimizer_add: Added draw call to optimizer\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_draw_call_optimizer_optimize
 *
 * @param opt Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_draw_call_optimizer_optimize(cmp_draw_call_optimizer_t *opt) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_draw_call_t *optimized = NULL;
  size_t optimized_count = 0, i;
  cmp_draw_call_t *last;
  cmp_draw_call_t *curr;

  if (opt == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_draw_call_optimizer_optimize: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (opt->count <= 1) {
    return CMP_SUCCESS;
  }

  rc = CMP_MALLOC(opt->count * sizeof(cmp_draw_call_t), (void **)&optimized);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_draw_call_optimizer_optimize: Out of memory: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memcpy(&optimized[0], &opt->calls[0], sizeof(cmp_draw_call_t));
  optimized_count++;
  for (i = 1; i < opt->count; i++) {
    last = &optimized[optimized_count - 1];
    curr = &opt->calls[i];
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

  rc = CMP_FREE(opt->calls);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_draw_call_optimizer_optimize: CMP_FREE old unoptimized "
                  "array failed\n");
  }

  opt->calls = optimized;
  opt->count = optimized_count;
  opt->capacity = opt->count;
  cmp_log_debug(
      "cmp_draw_call_optimizer_optimize: Optimization loop complete\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_draw_call_optimizer_destroy
 *
 * @param opt Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_draw_call_optimizer_destroy(cmp_draw_call_optimizer_t *opt) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (opt == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_draw_call_optimizer_destroy: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (opt->calls != NULL) {
    rc = CMP_FREE(opt->calls);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_draw_call_optimizer_destroy: CMP_FREE array failed\n");
    }
  }

  rc = CMP_FREE(opt);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_draw_call_optimizer_destroy: CMP_FREE opt failed\n");
  }

  cmp_log_debug(
      "cmp_draw_call_optimizer_destroy: Successfully destroyed optimizer\n");
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_command_buffer *cb = NULL;

  if (gpu == NULL || out_cb == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_buffer_create: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_command_buffer), (void **)&cb);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_buffer_create: Out of memory for cb: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(cb, 0, sizeof(struct cmp_command_buffer));
  cb->gpu = gpu;
  cb->is_secondary = is_secondary;
  cb->command_capacity = 256;

  rc = CMP_MALLOC(sizeof(cmp_draw_call_t) * cb->command_capacity,
                  (void **)&cb->commands);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    rc = CMP_FREE(cb);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_command_buffer_create: CMP_FREE recovery failed\n");
    }
    cmp_log_debug("cmp_command_buffer_create: Out of memory for commands: %s\n",
                  err_str);
    return CMP_ERROR_OOM;
  }

  memset(cb->commands, 0, sizeof(cmp_draw_call_t) * cb->command_capacity);
  *out_cb = (cmp_command_buffer_t *)cb;
  cmp_log_debug(
      "cmp_command_buffer_create: Successfully created command buffer\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_command_buffer_destroy
 *
 * @param cb Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_buffer_destroy(cmp_command_buffer_t *cb) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_buffer_destroy: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ctx->commands != NULL) {
    rc = CMP_FREE(ctx->commands);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_command_buffer_destroy: CMP_FREE commands failed\n");
    }
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_command_buffer_destroy: CMP_FREE ctx failed\n");
  }

  cmp_log_debug(
      "cmp_command_buffer_destroy: Successfully destroyed command buffer\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_command_buffer_begin
 *
 * @param cb Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_buffer_begin(cmp_command_buffer_t *cb) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_buffer_begin: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->command_count = 0;
  ctx->is_recording = 1;
  cmp_log_debug("cmp_command_buffer_begin: Command buffer begin recording\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_command_buffer_end
 *
 * @param cb Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_buffer_end(cmp_command_buffer_t *cb) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_buffer_end: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->is_recording = 0;
  cmp_log_debug("cmp_command_buffer_end: Command buffer end recording\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;
  cmp_draw_call_t *new_cmds = NULL;
  int new_cap;

  if (ctx == NULL || call == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_buffer_draw: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ctx->is_recording == 0) {
    rc = CMP_ERROR_INVALID_STATE;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_buffer_draw: Not recording: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ctx->command_count >= ctx->command_capacity) {
    new_cap = ctx->command_capacity * 2;
    rc = CMP_MALLOC(sizeof(cmp_draw_call_t) * new_cap, (void **)&new_cmds);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_command_buffer_draw: Out of memory: %s\n", err_str);
      if (rc != 0) {
        return rc;
      }
      return rc;
    }
    if (ctx->commands != NULL) {
      memcpy(new_cmds, ctx->commands,
             sizeof(cmp_draw_call_t) * ctx->command_count);
      rc = CMP_FREE(ctx->commands);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug(
            "cmp_command_buffer_draw: CMP_FREE old commands failed\n");
      }
    }
    ctx->commands = new_cmds;
    ctx->command_capacity = new_cap;
  }
  ctx->commands[ctx->command_count++] = *call;
  cmp_log_debug("cmp_command_buffer_draw: Tracked draw call\n");
  return CMP_SUCCESS;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_command_buffer *p_ctx = (struct cmp_command_buffer *)primary;
  struct cmp_command_buffer *s_ctx;
  int i;
  int j;

  if (p_ctx == NULL || secondaries == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_buffer_execute_commands: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (p_ctx->is_secondary != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_buffer_execute_commands: Cannot execute "
                  "secondaries in a secondary: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (p_ctx->is_recording == 0) {
    rc = CMP_ERROR_INVALID_STATE;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_buffer_execute_commands: Primary is not "
                  "recording: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  for (i = 0; i < count; i++) {
    s_ctx = (struct cmp_command_buffer *)secondaries[i];

    if (s_ctx == NULL || s_ctx->is_secondary == 0) {
      rc = CMP_ERROR_INVALID_ARG;
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_command_buffer_execute_commands: Invalid "
                    "secondary buffer: %s\n",
                    err_str);
      if (rc != 0) {
        return rc;
      }
      return rc;
    }

    if (s_ctx->is_recording != 0) {
      rc = CMP_ERROR_INVALID_STATE;
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_command_buffer_execute_commands: Secondary is "
                    "still recording: %s\n",
                    err_str);
      if (rc != 0) {
        return rc;
      }
      return rc;
    }

    for (j = 0; j < s_ctx->command_count; j++) {
      rc = cmp_command_buffer_draw((cmp_command_buffer_t *)p_ctx,
                                   &s_ctx->commands[j]);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug("cmp_command_buffer_execute_commands: "
                      "cmp_command_buffer_draw failed\n");
        if (rc != 0) {
          return rc;
        }
        return rc;
      }
    }
  }

  cmp_log_debug(
      "cmp_command_buffer_execute_commands: Executed %d secondary buffers\n",
      count);
  return CMP_SUCCESS;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_render_graph *graph = NULL;

  if (out_graph == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_render_graph_create: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_render_graph), (void **)&graph);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_render_graph_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(graph, 0, sizeof(struct cmp_render_graph));
  *out_graph = (cmp_render_graph_t *)graph;
  cmp_log_debug("cmp_render_graph_create: Successfully created render graph\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_render_graph_destroy
 *
 * @param graph Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_render_graph_destroy(cmp_render_graph_t *graph) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_render_graph *ctx = (struct cmp_render_graph *)graph;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_render_graph_destroy: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_render_graph_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug(
      "cmp_render_graph_destroy: Successfully destroyed render graph\n");
  return CMP_SUCCESS;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_render_graph *ctx = (struct cmp_render_graph *)graph;

  if (ctx == NULL || config == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_render_graph_add_pass: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ctx->pass_count >= 64) {
    rc = CMP_ERROR_OOM;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_render_graph_add_pass: Max passes reached: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->passes[ctx->pass_count++] = *config;
  cmp_log_debug("cmp_render_graph_add_pass: Added render pass node\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
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
  int rc;
  rc = 0;
  int i;
  for (i = 0; i < ctx->pass_count; ++i) {
    if (ctx->passes[i].id == id) {
      return i;
    }
  }
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_render_graph *ctx = (struct cmp_render_graph *)graph;
  int executed[64];
  int executed_count = 0;
  int progress;
  int i, j;
  int all_deps_met;
  int dep_id;
  int dep_idx;

  if (ctx == NULL || cb == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_render_graph_execute: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(executed, 0, sizeof(executed));

  do {
    progress = 0;
    for (i = 0; i < ctx->pass_count; i++) {
      if (executed[i] != 0) {
        continue;
      }

      all_deps_met = 1;
      for (j = 0; j < ctx->passes[i].dependency_count; j++) {
        dep_id = ctx->passes[i].dependencies[j];
        dep_idx = _cmp_render_graph_find_pass(ctx, dep_id);
        if (dep_idx == -1 || executed[dep_idx] == 0) {
          all_deps_met = 0;
          break;
        }
      }

      if (all_deps_met != 0) {
        if (ctx->passes[i].execute_cb != NULL) {
          ctx->passes[i].execute_cb(cb, ctx->passes[i].user_data);
        }
        executed[i] = 1;
        executed_count++;
        progress = 1;
      }
    }
  } while (progress != 0 && executed_count < ctx->pass_count);

  if (executed_count < ctx->pass_count) {
    rc = CMP_ERROR_INVALID_STATE; /* Circular dependency or missing pass */
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_render_graph_execute: Circular dependency or "
                  "missing pass: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  cmp_log_debug(
      "cmp_render_graph_execute: Executed rendering graph topology\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_pso_cache *cache = NULL;

  if (out_cache == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_pso_cache_create: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_pso_cache), (void **)&cache);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_pso_cache_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(cache, 0, sizeof(struct cmp_pso_cache));
  *out_cache = (cmp_pso_cache_t *)cache;
  cmp_log_debug("cmp_pso_cache_create: Successfully created PSO cache\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_pso_cache_destroy
 *
 * @param cache Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pso_cache_destroy(cmp_pso_cache_t *cache) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_pso_cache *ctx = (struct cmp_pso_cache *)cache;
  int i;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_pso_cache_destroy: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  for (i = 0; i < ctx->count; i++) {
    rc = CMP_FREE(ctx->psos[i]);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_pso_cache_destroy: CMP_FREE cache item failed\n");
    }
  }
  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_pso_cache_destroy: CMP_FREE context failed\n");
  }

  cmp_log_debug("cmp_pso_cache_destroy: Successfully destroyed PSO cache\n");
  return CMP_SUCCESS;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_pso_cache *ctx = (struct cmp_pso_cache *)cache;
  int i;
  struct cmp_pso *new_pso = NULL;

  if (ctx == NULL || state == NULL || out_pso == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_pso_cache_get_or_create: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  for (i = 0; i < ctx->count; i++) {
    if (memcmp(&ctx->psos[i]->state, state, sizeof(cmp_pipeline_state_t)) ==
        0) {
      *out_pso = (cmp_pso_t *)ctx->psos[i];
      cmp_log_debug("cmp_pso_cache_get_or_create: Found PSO in cache\n");
      return CMP_SUCCESS;
    }
  }

  if (ctx->count >= 256) {
    rc = CMP_ERROR_OOM;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_pso_cache_get_or_create: Cache full: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_pso), (void **)&new_pso);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_pso_cache_get_or_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(new_pso, 0, sizeof(struct cmp_pso));
  new_pso->state = *state;
  new_pso->internal_handle = (void *)(size_t)(0xCAFEBABE + ctx->count);

  ctx->psos[ctx->count++] = new_pso;
  *out_pso = (cmp_pso_t *)new_pso;
  cmp_log_debug("cmp_pso_cache_get_or_create: Cached new PSO\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_command_buffer_bind_pso
 *
 * @param cb Parameter description.
 * @param pso Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_buffer_bind_pso(cmp_command_buffer_t *cb, cmp_pso_t *pso) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_command_buffer *ctx = (struct cmp_command_buffer *)cb;

  if (ctx == NULL || pso == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_buffer_bind_pso: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ctx->is_recording == 0) {
    rc = CMP_ERROR_INVALID_STATE;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_buffer_bind_pso: Not recording: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  cmp_log_debug(
      "cmp_command_buffer_bind_pso: Bound PSO state to command buffer\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_shader *shader = NULL;

  (void)source;
  (void)size;

  if (out_shader == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_shader_compile_spirv: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_shader), (void **)&shader);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_shader_compile_spirv: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(shader, 0, sizeof(struct cmp_shader));
  shader->internal_handle = (void *)0x5B1210;
  *out_shader = shader;
  cmp_log_debug("cmp_shader_compile_spirv: Mapped mocked SPIR-V generation\n");
  return CMP_SUCCESS;
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
  int rc;
  rc = cmp_shader_compile_spirv(source, size, out_shader);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_shader_compile_msl: Fallback to spirv failed\n");
  }
  if (rc != 0) {
    return rc;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_gpu_allocator *alloc = NULL;

  if (gpu == NULL || out_allocator == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gpu_allocator_create: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_gpu_allocator), (void **)&alloc);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gpu_allocator_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(alloc, 0, sizeof(struct cmp_gpu_allocator));
  alloc->gpu = gpu;
  alloc->block_size = block_size;
  alloc->used = 0;

  rc = CMP_MALLOC(block_size, &alloc->base_mem);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    rc = CMP_FREE(alloc);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_gpu_allocator_create: CMP_FREE recovery failed\n");
    }
    cmp_log_debug("cmp_gpu_allocator_create: Out of memory allocating "
                  "base_mem: %s\n",
                  err_str);
    return CMP_ERROR_OOM;
  }

  memset(alloc->base_mem, 0, block_size);
  *out_allocator = (cmp_gpu_allocator_t *)alloc;
  cmp_log_debug("cmp_gpu_allocator_create: Configured global memory buffer "
                "allocator map\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_gpu_allocator_destroy
 *
 * @param allocator Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_gpu_allocator_destroy(cmp_gpu_allocator_t *allocator) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_gpu_allocator *alloc = (struct cmp_gpu_allocator *)allocator;

  if (alloc == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gpu_allocator_destroy: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (alloc->base_mem != NULL) {
    rc = CMP_FREE(alloc->base_mem);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_gpu_allocator_destroy: CMP_FREE base_mem failed\n");
    }
  }
  rc = CMP_FREE(alloc);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_gpu_allocator_destroy: CMP_FREE alloc context failed\n");
  }

  cmp_log_debug("cmp_gpu_allocator_destroy: Successfully dismantled global "
                "memory buffer\n");
  return CMP_SUCCESS;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_gpu_allocator *alloc = (struct cmp_gpu_allocator *)allocator;
  size_t padding;

  if (alloc == NULL || out_mem == NULL || out_offset == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gpu_allocator_alloc: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  padding = (alignment - (alloc->used % alignment)) % alignment;
  if (alloc->used + padding + size > alloc->block_size) {
    rc = CMP_ERROR_OOM;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_gpu_allocator_alloc: Block full: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  alloc->used += padding;
  *out_offset = alloc->used;
  *out_mem = (void *)((char *)alloc->base_mem + alloc->used);
  alloc->used += size;

  cmp_log_debug(
      "cmp_gpu_allocator_alloc: Provided managed suballocation map pointer\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
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
  int rc;
  rc = CMP_SUCCESS;
  (void)allocator;
  (void)mem;
  cmp_log_debug(
      "cmp_gpu_allocator_free: Evaluated linear memory chunk freeing rules\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_atlas *atlas = NULL;

  if (gpu == NULL || out_atlas == NULL || width <= 0 || height <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_atlas_create: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_atlas), (void **)&atlas);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_atlas_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(atlas, 0, sizeof(struct cmp_atlas));
  atlas->gpu = gpu;
  atlas->width = width;
  atlas->height = height;

  rc = CMP_MALLOC((size_t)(width * height * 4), &atlas->pixels);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    rc = CMP_FREE(atlas);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_atlas_create: CMP_FREE atlas recovery failed\n");
    }
    cmp_log_debug("cmp_atlas_create: Out of memory for pixels: %s\n", err_str);
    return CMP_ERROR_OOM;
  }

  memset(atlas->pixels, 0, (size_t)(width * height * 4));
  *out_atlas = (cmp_atlas_t *)atlas;
  cmp_log_debug(
      "cmp_atlas_create: Instantiated linear pixel atlas dimensions\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_atlas_destroy
 *
 * @param atlas Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_atlas_destroy(cmp_atlas_t *atlas) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_atlas *ctx = (struct cmp_atlas *)atlas;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_atlas_destroy: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ctx->pixels != NULL) {
    rc = CMP_FREE(ctx->pixels);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_atlas_destroy: CMP_FREE pixels failed\n");
    }
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_atlas_destroy: CMP_FREE ctx failed\n");
  }

  cmp_log_debug(
      "cmp_atlas_destroy: Flushed localized atlas memory constraints\n");
  return CMP_SUCCESS;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_atlas *ctx = (struct cmp_atlas *)atlas;

  if (ctx == NULL || pixels == NULL || out_x == NULL || out_y == NULL ||
      width <= 0 || height <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_atlas_insert: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ctx->current_x + width > ctx->width) {
    ctx->current_x = 0;
    ctx->current_y += ctx->current_row_height;
    ctx->current_row_height = 0;
  }

  if (ctx->current_y + height > ctx->height) {
    rc = CMP_ERROR_OOM;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_atlas_insert: Atlas full: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  *out_x = ctx->current_x;
  *out_y = ctx->current_y;

  ctx->current_x += width;
  if (height > ctx->current_row_height) {
    ctx->current_row_height = height;
  }

  cmp_log_debug("cmp_atlas_insert: Configured row boundary displacement\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_atlas_evict
 *
 * @param atlas Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_atlas_evict(cmp_atlas_t *atlas) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_atlas *ctx = (struct cmp_atlas *)atlas;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_atlas_evict: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->current_x = 0;
  ctx->current_y = 0;
  ctx->current_row_height = 0;
  memset(ctx->pixels, 0, (size_t)(ctx->width * ctx->height * 4));
  cmp_log_debug("cmp_atlas_evict: Cleared internal GPU texture allocations\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  (void)data;
  (void)size;

  if (out_rgba == NULL || out_width == NULL || out_height == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_tex_compression_decode_astc: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC((size_t)(64 * 64 * 4), out_rgba);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_tex_compression_decode_astc: Out of memory: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(*out_rgba, 0, (size_t)(64 * 64 * 4));
  *out_width = 64;
  *out_height = 64;
  cmp_log_debug(
      "cmp_tex_compression_decode_astc: Emulated block decoding ASTC\n");
  return CMP_SUCCESS;
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
  int rc;
  rc = cmp_tex_compression_decode_astc(data, size, out_rgba, out_width,
                                       out_height);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_tex_compression_decode_bc7: Fallback to astc failed\n");
  }
  if (rc != 0) {
    return rc;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (node_rect == NULL || viewport_rect == NULL || out_is_visible == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_frustum_culling_test: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
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

  cmp_log_debug("cmp_frustum_culling_test: Validated AABB boundaries %d\n",
                *out_is_visible);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
