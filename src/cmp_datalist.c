/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_datalist {
  int item_count;
};

int cmp_datalist_create(cmp_datalist_t **out_datalist) {
  struct cmp_datalist *datalist;

  if (!out_datalist)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_datalist), (void **)&datalist) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(datalist, 0, sizeof(struct cmp_datalist));

  *out_datalist = (cmp_datalist_t *)datalist;
  return CMP_SUCCESS;
}

int cmp_datalist_destroy(cmp_datalist_t *datalist) {
  struct cmp_datalist *internal_datalist = (struct cmp_datalist *)datalist;
  if (!internal_datalist)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_datalist);
  return CMP_SUCCESS;
}

int cmp_datalist_filter(cmp_datalist_t *datalist, const char *input_string) {
  struct cmp_datalist *internal_datalist = (struct cmp_datalist *)datalist;

  if (!internal_datalist || !input_string)
    return CMP_ERROR_INVALID_ARG;

  /* Filter logic placeholder */
  return CMP_SUCCESS;
}