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
 * @brief Creates a new code block context.
 *
 * @param out_block Pointer to store the newly allocated code block context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_code_block_create(cmp_code_block_t **out_block) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_code_block_t *block = NULL;

  if (out_block == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_code_block_create: Invalid argument (out_block=NULL): %s\n",
        err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_code_block_t), (void **)&block);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_code_block_create: Out of memory: %s\n", err_str);

    return rc;
  }

  block->is_expanded = 1;
  block->ref_count = 0;
  block->ref_capacity = 0;
  block->refs = NULL;

  *out_block = block;
  cmp_log_debug(
      "cmp_code_block_create: Successfully created code block context\n");

  return rc;
}

/**
 * @brief Destroys a code block context.
 *
 * @param block Pointer to the code block context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_code_block_destroy(cmp_code_block_t *block) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (block == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_code_block_destroy: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (block->refs != NULL) {
    rc = CMP_FREE(block->refs);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_code_block_destroy: CMP_FREE refs failed\n");
    }
  }

  rc = CMP_FREE(block);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_code_block_destroy: CMP_FREE block failed\n");

    return rc;
  }

  cmp_log_debug(
      "cmp_code_block_destroy: Successfully destroyed code block context\n");
  return rc;
}

/**
 * @brief Toggles the folded state of a code block.
 *
 * @param block Pointer to the code block context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_code_block_toggle_fold(cmp_code_block_t *block) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (block == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_code_block_toggle_fold: Invalid argument: %s\n",
                  err_str);

    return rc;
  }
  block->is_expanded = block->is_expanded ? 0 : 1;
  cmp_log_debug("cmp_code_block_toggle_fold: Toggled fold state to %d\n",
                block->is_expanded);

  return rc;
}

/**
 * @brief Retrieves the expanded state of a code block.
 *
 * @param block Pointer to the code block context.
 * @param out_is_expanded Pointer to store the expansion state (1 if expanded, 0
 * if folded).
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_code_block_is_expanded(const cmp_code_block_t *block,
                               int *out_is_expanded) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (block == NULL || out_is_expanded == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_code_block_is_expanded: Invalid argument: %s\n",
                  err_str);

    return rc;
  }
  *out_is_expanded = block->is_expanded;
  cmp_log_debug("cmp_code_block_is_expanded: Is expanded=%d\n",
                *out_is_expanded);

  return rc;
}

/**
 * @brief Adds a highlight reference to a specific portion of the code block.
 *
 * @param block Pointer to the code block context.
 * @param offset Character offset where the highlight begins.
 * @param length Length of the highlight in characters.
 * @param reference_id The string identifier of the reference.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_code_block_add_reference_highlight(cmp_code_block_t *block,
                                           size_t offset, size_t length,
                                           const char *reference_id) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_code_ref_t *new_array = NULL;
  size_t new_cap;

  if (block == NULL || reference_id == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_code_block_add_reference_highlight: Invalid argument: %s\n",
        err_str);

    return rc;
  }

  if (block->ref_count == block->ref_capacity) {
    new_cap = block->ref_capacity == 0 ? 4 : block->ref_capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_code_ref_t), (void **)&new_array);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug(
          "cmp_code_block_add_reference_highlight: Out of memory: %s\n",
          err_str);

      return rc;
    }
    if (block->refs != NULL) {
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
    cmp_log_debug("cmp_code_block_add_reference_highlight: strncpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strncpy(block->refs[block->ref_count].id, reference_id, 63);
  block->refs[block->ref_count].id[63] = '\0';
#endif
  block->ref_count++;

  cmp_log_debug(
      "cmp_code_block_add_reference_highlight: Added highlight ref\n");

  return rc;
}
