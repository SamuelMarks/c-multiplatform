/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_select_ui {
  int is_open;
};

int cmp_select_ui_create(cmp_select_ui_t **out_select) {
  struct cmp_select_ui *select_ui;

  if (!out_select)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_select_ui), (void **)&select_ui) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(select_ui, 0, sizeof(struct cmp_select_ui));

  *out_select = (cmp_select_ui_t *)select_ui;
  return CMP_SUCCESS;
}

int cmp_select_ui_destroy(cmp_select_ui_t *select) {
  struct cmp_select_ui *internal_select = (struct cmp_select_ui *)select;
  if (!internal_select)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_select);
  return CMP_SUCCESS;
}

int cmp_select_ui_open(cmp_select_ui_t *select) {
  struct cmp_select_ui *internal_select = (struct cmp_select_ui *)select;

  if (!internal_select)
    return CMP_ERROR_INVALID_ARG;

  internal_select->is_open = 1;

  return CMP_SUCCESS;
}
