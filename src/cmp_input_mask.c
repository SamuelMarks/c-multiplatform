/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_input_mask {
  char pattern[64];
};

/**
 * @brief cmp_input_mask_create
 *
 * @param mask_pattern Parameter description.
 * @param out_mask Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_input_mask_create(const char *mask_pattern,
                          cmp_input_mask_t **out_mask) {
  int rc = CMP_SUCCESS;
  struct cmp_input_mask *mask = NULL;

  if (!mask_pattern || !out_mask) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_input_mask_create: Invalid argument\n");

    return rc;
  }

  if (strlen(mask_pattern) >= sizeof(mask->pattern)) {
    rc = CMP_ERROR_BOUNDS;
    LOG_DEBUG("Error in cmp_input_mask_create: Mask pattern too long\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_input_mask), (void **)&mask);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_input_mask_create: Out of memory\n");

    return rc;
  }

  memset(mask, 0, sizeof(struct cmp_input_mask));
#if defined(_MSC_VER)
  strcpy_s(mask->pattern, sizeof(mask->pattern), mask_pattern);
#else
  strcpy(mask->pattern, mask_pattern);
#endif

  *out_mask = (cmp_input_mask_t *)mask;

  return rc;
}

/**
 * @brief cmp_input_mask_destroy
 *
 * @param mask Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_input_mask_destroy(cmp_input_mask_t *mask) {
  int rc = CMP_SUCCESS;
  struct cmp_input_mask *internal_mask = (struct cmp_input_mask *)mask;

  if (!internal_mask) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_input_mask_destroy: Invalid argument\n");

    return rc;
  }

  rc = CMP_FREE(internal_mask);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_input_mask_destroy: CMP_FREE failed\n");
  }

  return rc;
}

/**
 * @brief cmp_input_mask_apply
 *
 * @param mask Parameter description.
 * @param raw_input Parameter description.
 * @param out_buffer Parameter description.
 * @param out_capacity Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_input_mask_apply(cmp_input_mask_t *mask, const char *raw_input,
                         char *out_buffer, size_t out_capacity) {
  int rc = CMP_SUCCESS;
  struct cmp_input_mask *internal_mask = (struct cmp_input_mask *)mask;
  size_t pattern_len, input_len, p, i, o;

  if (!internal_mask || !raw_input || !out_buffer || out_capacity == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_input_mask_apply: Invalid argument\n");

    return rc;
  }

  pattern_len = strlen(internal_mask->pattern);
  input_len = strlen(raw_input);

  p = 0;
  i = 0;
  o = 0;

  while (p < pattern_len && i < input_len && o < out_capacity - 1) {
    if (internal_mask->pattern[p] == 'X') {
      out_buffer[o++] = raw_input[i++];
    } else if (internal_mask->pattern[p] == '9') {
      if (raw_input[i] >= '0' && raw_input[i] <= '9') {
        out_buffer[o++] = raw_input[i++];
      } else {
        i++;
        continue;
      }
    } else if (internal_mask->pattern[p] == 'A') {
      if ((raw_input[i] >= 'a' && raw_input[i] <= 'z') ||
          (raw_input[i] >= 'A' && raw_input[i] <= 'Z')) {
        out_buffer[o++] = raw_input[i++];
      } else {
        i++;
        continue;
      }
    } else {
      /* Static char in mask */
      out_buffer[o++] = internal_mask->pattern[p];
      if (raw_input[i] == internal_mask->pattern[p]) {
        i++; /* User typed the static char, skip it */
      }
    }
    p++;
  }

  /* Add remaining static chars if input ends exactly before them */
  while (p < pattern_len && internal_mask->pattern[p] != 'X' &&
         internal_mask->pattern[p] != '9' && internal_mask->pattern[p] != 'A' &&
         o < out_capacity - 1) {
    out_buffer[o++] = internal_mask->pattern[p];
    p++;
  }

  out_buffer[o] = '\0';

  return rc;
}
