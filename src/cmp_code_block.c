/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct cmp_code_ref {
  size_t offset;
  size_t length;
  char id[64];
} cmp_code_ref_t;

struct cmp_code_block {
  int is_expanded;
  cmp_code_ref_t *refs;
  size_t ref_count;
  size_t ref_capacity;
};

int cmp_code_block_create(cmp_code_block_t **out_block) {
  cmp_code_block_t *block;
  if (!out_block)
    return CMP_ERROR_INVALID_ARG;

  block = (cmp_code_block_t *)malloc(sizeof(cmp_code_block_t));
  if (!block)
    return CMP_ERROR_OOM;

  block->is_expanded = 1;
  block->ref_count = 0;
  block->ref_capacity = 0;
  block->refs = NULL;

  *out_block = block;
  return CMP_SUCCESS;
}

int cmp_code_block_destroy(cmp_code_block_t *block) {
  if (!block)
    return CMP_ERROR_INVALID_ARG;
  if (block->refs)
    free(block->refs);
  free(block);
  return CMP_SUCCESS;
}

int cmp_code_block_toggle_fold(cmp_code_block_t *block) {
  if (!block)
    return CMP_ERROR_INVALID_ARG;
  block->is_expanded = block->is_expanded ? 0 : 1;
  return CMP_SUCCESS;
}

int cmp_code_block_is_expanded(const cmp_code_block_t *block,
                               int *out_is_expanded) {
  if (!block || !out_is_expanded)
    return CMP_ERROR_INVALID_ARG;
  *out_is_expanded = block->is_expanded;
  return CMP_SUCCESS;
}

int cmp_code_block_add_reference_highlight(cmp_code_block_t *block,
                                           size_t offset, size_t length,
                                           const char *reference_id) {
  cmp_code_ref_t *new_array;
  if (!block || !reference_id)
    return CMP_ERROR_INVALID_ARG;

  if (block->ref_count == block->ref_capacity) {
    size_t new_cap = block->ref_capacity == 0 ? 4 : block->ref_capacity * 2;
    new_array = (cmp_code_ref_t *)realloc(block->refs,
                                          new_cap * sizeof(cmp_code_ref_t));
    if (!new_array)
      return CMP_ERROR_OOM;
    block->refs = new_array;
    block->ref_capacity = new_cap;
  }

  block->refs[block->ref_count].offset = offset;
  block->refs[block->ref_count].length = length;
  strncpy(block->refs[block->ref_count].id, reference_id, 63);
  block->refs[block->ref_count].id[63] = '\0';
  block->ref_count++;

  return CMP_SUCCESS;
}
