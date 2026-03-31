/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_dnd {
  char *payload_text;
  cmp_dnd_op_t operation;
};

int cmp_dnd_create(cmp_dnd_t **out_dnd) {
  cmp_dnd_t *dnd;
  if (!out_dnd) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_dnd_t), (void **)&dnd) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
  memset(dnd, 0, sizeof(cmp_dnd_t));
  dnd->operation = CMP_DND_OP_NONE;
  *out_dnd = dnd;
  return CMP_SUCCESS;
}

int cmp_dnd_destroy(cmp_dnd_t *dnd) {
  if (!dnd) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (dnd->payload_text) {
    CMP_FREE(dnd->payload_text);
  }
  CMP_FREE(dnd);
  return CMP_SUCCESS;
}

int cmp_dnd_set_payload_text(cmp_dnd_t *dnd, const char *text) {
  size_t len;
  if (!dnd || !text) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (dnd->payload_text) {
    CMP_FREE(dnd->payload_text);
    dnd->payload_text = NULL;
  }
  len = strlen(text);
  if (CMP_MALLOC(len + 1, (void **)&dnd->payload_text) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(dnd->payload_text, len + 1, text);
#else
  strcpy(dnd->payload_text, text);
#endif
  return CMP_SUCCESS;
}

int cmp_dnd_get_payload_text(const cmp_dnd_t *dnd, char **out_text) {
  size_t len;
  if (!dnd || !out_text) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (!dnd->payload_text) {
    *out_text = NULL;
    return CMP_SUCCESS;
  }
  len = strlen(dnd->payload_text);
  if (CMP_MALLOC(len + 1, (void **)out_text) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(*out_text, len + 1, dnd->payload_text);
#else
  strcpy(*out_text, dnd->payload_text);
#endif
  return CMP_SUCCESS;
}

int cmp_dnd_set_operation(cmp_dnd_t *dnd, cmp_dnd_op_t op) {
  if (!dnd) {
    return CMP_ERROR_INVALID_ARG;
  }
  dnd->operation = op;
  return CMP_SUCCESS;
}

int cmp_dnd_get_operation(const cmp_dnd_t *dnd, cmp_dnd_op_t *out_op) {
  if (!dnd || !out_op) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_op = dnd->operation;
  return CMP_SUCCESS;
}
