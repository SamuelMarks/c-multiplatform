/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
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

/**
 * @brief cmp_code_block_create
 *
 * @param out_block Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_code_block_create(cmp_code_block_t **out_block) {
  cmp_code_block_t *block = NULL;
  int rc;

  if (!out_block) {
    LOG_DEBUG("cmp_code_block_create: out_block is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_code_block_t), (void **)&block);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_code_block_create: OOM\n");
    return rc;
  }

  block->is_expanded = 1;
  block->ref_count = 0;
  block->ref_capacity = 0;
  block->refs = NULL;

  *out_block = block;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_code_block_destroy
 *
 * @param block Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_code_block_destroy(cmp_code_block_t *block) {
  int rc;

  if (!block) {
    LOG_DEBUG("cmp_code_block_destroy: block is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (block->refs) {
    rc = CMP_FREE(block->refs);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_code_block_destroy: CMP_FREE refs failed\n");
    }
  }

  rc = CMP_FREE(block);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_code_block_destroy: CMP_FREE block failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_code_block_toggle_fold
 *
 * @param block Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_code_block_toggle_fold(cmp_code_block_t *block) {
  if (!block) {
    LOG_DEBUG("cmp_code_block_toggle_fold: block is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  block->is_expanded = block->is_expanded ? 0 : 1;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_code_block_is_expanded
 *
 * @param block Parameter description.
 * @param out_is_expanded Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_code_block_is_expanded(const cmp_code_block_t *block,
                               int *out_is_expanded) {
  if (!block || !out_is_expanded) {
    LOG_DEBUG("cmp_code_block_is_expanded: Invalid args\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_is_expanded = block->is_expanded;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_code_block_add_reference_highlight
 *
 * @param block Parameter description.
 * @param offset Parameter description.
 * @param length Parameter description.
 * @param reference_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_code_block_add_reference_highlight(cmp_code_block_t *block,
                                           size_t offset, size_t length,
                                           const char *reference_id) {
  cmp_code_ref_t *new_array = NULL;
  int rc;

  if (!block || !reference_id) {
    LOG_DEBUG("cmp_code_block_add_reference_highlight: Invalid args\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (block->ref_count == block->ref_capacity) {
    size_t new_cap = block->ref_capacity == 0 ? 4 : block->ref_capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_code_ref_t), (void **)&new_array);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_code_block_add_reference_highlight: OOM\n");
      return rc;
    }
    if (block->refs) {
      memcpy(new_array, block->refs, block->ref_count * sizeof(cmp_code_ref_t));
      CMP_FREE(block->refs);
    }
    block->refs = new_array;
    block->ref_capacity = new_cap;
  }

  block->refs[block->ref_count].offset = offset;
  block->refs[block->ref_count].length = length;
#if defined(_MSC_VER)
  if (strncpy_s(block->refs[block->ref_count].id, 64, reference_id,
                _TRUNCATE) != 0) {
    LOG_DEBUG("cmp_code_block_add_reference_highlight: strncpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strncpy(block->refs[block->ref_count].id, reference_id, 63);
  block->refs[block->ref_count].id[63] = '\0';
#endif
  block->ref_count++;

  return CMP_SUCCESS;
}