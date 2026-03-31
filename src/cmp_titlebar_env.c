/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_titlebar_env {
  float x;
  float y;
  float width;
  float height;
};

int cmp_titlebar_env_create(cmp_titlebar_env_t **out_env) {
  cmp_titlebar_env_t *env;
  if (!out_env) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_titlebar_env_t), (void **)&env) != 0) {
    return CMP_ERROR_OOM;
  }
  memset(env, 0, sizeof(cmp_titlebar_env_t));
  *out_env = env;
  return CMP_SUCCESS;
}

int cmp_titlebar_env_destroy(cmp_titlebar_env_t *env) {
  if (!env) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(env);
  return CMP_SUCCESS;
}

int cmp_titlebar_env_set_area(cmp_titlebar_env_t *env, float x, float y,
                              float width, float height) {
  if (!env) {
    return CMP_ERROR_INVALID_ARG;
  }
  env->x = x;
  env->y = y;
  env->width = width;
  env->height = height;
  return CMP_SUCCESS;
}

int cmp_titlebar_env_get_area(const cmp_titlebar_env_t *env, float *out_x,
                              float *out_y, float *out_width,
                              float *out_height) {
  if (!env || !out_x || !out_y || !out_width || !out_height) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_x = env->x;
  *out_y = env->y;
  *out_width = env->width;
  *out_height = env->height;
  return CMP_SUCCESS;
}
