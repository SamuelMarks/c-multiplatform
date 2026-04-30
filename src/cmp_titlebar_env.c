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

/**
 * @brief cmp_titlebar_env_create
 *
 * @param out_env Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_titlebar_env_create(cmp_titlebar_env_t **out_env) {
  int rc = CMP_SUCCESS;
  cmp_titlebar_env_t *env;
  if (!out_env) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_titlebar_env_t), (void **)&env) != 0) {
    return CMP_ERROR_OOM;
  }
  memset(env, 0, sizeof(cmp_titlebar_env_t));
  *out_env = env;

  return rc;
}

/**
 * @brief cmp_titlebar_env_destroy
 *
 * @param env Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_titlebar_env_destroy(cmp_titlebar_env_t *env) {
  int rc = CMP_SUCCESS;
  if (!env) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(env);

  return rc;
}

/**
 * @brief cmp_titlebar_env_set_area
 *
 * @param env Parameter description.
 * @param x Parameter description.
 * @param y Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_titlebar_env_set_area(cmp_titlebar_env_t *env, float x, float y,
                              float width, float height) {
  int rc = CMP_SUCCESS;
  if (!env) {
    return CMP_ERROR_INVALID_ARG;
  }
  env->x = x;
  env->y = y;
  env->width = width;
  env->height = height;

  return rc;
}

/**
 * @brief cmp_titlebar_env_get_area
 *
 * @param env Parameter description.
 * @param out_x Parameter description.
 * @param out_y Parameter description.
 * @param out_width Parameter description.
 * @param out_height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_titlebar_env_get_area(const cmp_titlebar_env_t *env, float *out_x,
                              float *out_y, float *out_width,
                              float *out_height) {
  int rc = CMP_SUCCESS;
  if (!env || !out_x || !out_y || !out_width || !out_height) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_x = env->x;
  *out_y = env->y;
  *out_width = env->width;
  *out_height = env->height;

  return rc;
}
