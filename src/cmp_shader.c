/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_shader_init_rounded_rect(cmp_shader_t *shader) {
  if (!shader)
    return CMP_ERROR_INVALID_ARG;
  /* Allocate internal handle representation */
  if (CMP_MALLOC(sizeof(int), &shader->internal_handle) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  *((int *)shader->internal_handle) = 1; /* Type 1: Rounded Rect */
  return CMP_SUCCESS;
}

int cmp_shader_init_gradient(cmp_shader_t *shader) {
  if (!shader)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(int), &shader->internal_handle) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  *((int *)shader->internal_handle) = 2; /* Type 2: Gradient */
  return CMP_SUCCESS;
}

int cmp_shader_init_sdf_text(cmp_shader_t *shader) {
  if (!shader)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(int), &shader->internal_handle) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  *((int *)shader->internal_handle) = 3; /* Type 3: SDF Text */
  return CMP_SUCCESS;
}

int cmp_shader_destroy(cmp_shader_t *shader) {
  if (!shader)
    return CMP_ERROR_INVALID_ARG;
  if (shader->internal_handle) {
    CMP_FREE(shader->internal_handle);
    shader->internal_handle = NULL;
  }
  return CMP_SUCCESS;
}
