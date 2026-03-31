/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_selection {
  int start_index;
  int end_index;
};

int cmp_selection_create(cmp_selection_t **out_selection) {
  struct cmp_selection *selection;

  if (!out_selection)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_selection), (void **)&selection) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(selection, 0, sizeof(struct cmp_selection));

  *out_selection = (cmp_selection_t *)selection;
  return CMP_SUCCESS;
}

int cmp_selection_destroy(cmp_selection_t *selection) {
  struct cmp_selection *internal_selection = (struct cmp_selection *)selection;
  if (!internal_selection)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_selection);
  return CMP_SUCCESS;
}