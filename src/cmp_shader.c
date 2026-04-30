/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>

#include "cmp_log.h"
/* clang-format on */

/**
 * @brief Initialize a rounded rect shader.
 *
 * @param shader Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_init_rounded_rect(cmp_shader_t *shader) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (shader == NULL) {
    LOG_DEBUG("Invalid argument: shader is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Allocate internal handle representation */
  rc = CMP_MALLOC(sizeof(int), &shader->internal_handle);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  *((int *)shader->internal_handle) = 1; /* Type 1: Rounded Rect */

  return rc;
}

/**
 * @brief Initialize a gradient shader.
 *
 * @param shader Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_init_gradient(cmp_shader_t *shader) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (shader == NULL) {
    LOG_DEBUG("Invalid argument: shader is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(int), &shader->internal_handle);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  *((int *)shader->internal_handle) = 2; /* Type 2: Gradient */

  return rc;
}

/**
 * @brief Initialize an SDF text shader.
 *
 * @param shader Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_init_sdf_text(cmp_shader_t *shader) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (shader == NULL) {
    LOG_DEBUG("Invalid argument: shader is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(int), &shader->internal_handle);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  *((int *)shader->internal_handle) = 3; /* Type 3: SDF Text */

  return rc;
}

/**
 * @brief Destroy a shader.
 *
 * @param shader Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_destroy(cmp_shader_t *shader) {
  int rc = CMP_SUCCESS;

  rc = CMP_SUCCESS;

  if (shader == NULL) {
    LOG_DEBUG("Invalid argument: shader is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (shader->internal_handle != NULL) {
    rc = CMP_FREE(shader->internal_handle);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
      return rc;
    }
    shader->internal_handle = NULL;
  }

  return rc;
}

/**
 * @brief Get rounded rect SDF GLSL source.
 *
 * @param out_source Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_get_rounded_rect_sdf_glsl(const char **out_source) {
  int rc = CMP_SUCCESS;
  if (out_source == NULL) {
    LOG_DEBUG("Invalid argument: out_source is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_source =
      "void main() { float d = length(max(abs(pos) - size + radius, 0.0)) - "
      "radius; gl_FragColor = vec4(1.0, 1.0, 1.0, step(d, 0.0)); }";

  return rc;
}

/**
 * @brief Get squircle SDF GLSL source.
 *
 * @param out_source Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_get_squircle_sdf_glsl(const char **out_source) {
  int rc = CMP_SUCCESS;
  if (out_source == NULL) {
    LOG_DEBUG("Invalid argument: out_source is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_source =
      "void main() { float d = pow(abs(pos.x), 3.0) + pow(abs(pos.y), 3.0) - "
      "pow(radius, 3.0); gl_FragColor = vec4(1.0, 1.0, 1.0, step(d, 0.0)); }";

  return rc;
}
