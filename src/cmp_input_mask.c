/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_input_mask {
  char pattern[64];
};

int cmp_input_mask_create(const char *mask_pattern,
                          cmp_input_mask_t **out_mask) {
  struct cmp_input_mask *mask;

  if (!mask_pattern || !out_mask ||
      strlen(mask_pattern) >= sizeof(mask->pattern))
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_input_mask), (void **)&mask) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(mask, 0, sizeof(struct cmp_input_mask));
  strcpy(mask->pattern, mask_pattern);

  *out_mask = (cmp_input_mask_t *)mask;
  return CMP_SUCCESS;
}

int cmp_input_mask_destroy(cmp_input_mask_t *mask) {
  struct cmp_input_mask *internal_mask = (struct cmp_input_mask *)mask;
  if (!internal_mask)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_mask);
  return CMP_SUCCESS;
}

int cmp_input_mask_apply(cmp_input_mask_t *mask, const char *raw_input,
                         char *out_buffer, size_t out_capacity) {
  struct cmp_input_mask *internal_mask = (struct cmp_input_mask *)mask;
  size_t pattern_len, input_len, p, i, o;

  if (!internal_mask || !raw_input || !out_buffer || out_capacity == 0)
    return CMP_ERROR_INVALID_ARG;

  pattern_len = strlen(internal_mask->pattern);
  input_len = strlen(raw_input);

  p = 0;
  i = 0;
  o = 0;

  while (p < pattern_len && i < input_len && o < out_capacity - 1) {
    if (internal_mask->pattern[p] == 'X') {
      /* Assume 'X' means any char in this simple stub */
      out_buffer[o++] = raw_input[i++];
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
         o < out_capacity - 1) {
    out_buffer[o++] = internal_mask->pattern[p];
    p++;
  }

  out_buffer[o] = '\0';

  return CMP_SUCCESS;
}