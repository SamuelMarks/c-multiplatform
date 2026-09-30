/**
 * @file md3_divider.c
 * @brief Material 3 Divider component implementation wrapping ui_divider_base.
 */

/* clang-format off */
#include "material3/md3_divider.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t md3_divider_create(struct ui_engine *engine,
                              enum ui_divider_orientation orientation,
                              int is_inset, struct md3_divider **out_divider) {
  struct md3_divider *div;
  ui_error_t rc;

  if (!engine || !out_divider ||
      (orientation != UI_DIVIDER_ORIENTATION_HORIZONTAL &&
       orientation != UI_DIVIDER_ORIENTATION_VERTICAL)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  div =
      (struct md3_divider *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_divider));
  if (!div) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(div, 0, sizeof(struct md3_divider));
  div->orientation = orientation;
  div->is_inset = is_inset ? 1 : 0;

  rc = ui_divider_base_create(&div->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(div);
    return rc;
  }

  if (orientation != UI_DIVIDER_ORIENTATION_HORIZONTAL) {
    rc = ui_divider_base_set_orientation(div->base, orientation);
    if (rc != UI_ERROR_NONE) {
      ui_divider_base_destroy(div->base);
      C_MULTIPLATFORM_FREE(div);
      return rc;
    }
  }

  if (div->is_inset) {
    rc = ui_divider_base_set_inset(div->base, div->is_inset);
    if (rc != UI_ERROR_NONE) {
      ui_divider_base_destroy(div->base);
      C_MULTIPLATFORM_FREE(div);
      return rc;
    }
  }

  *out_divider = div;
  return UI_ERROR_NONE;
}

ui_error_t md3_divider_destroy(struct md3_divider *divider) {
  ui_error_t rc;

  if (!divider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_divider_base_destroy(divider->base);
  C_MULTIPLATFORM_FREE(divider);
  return rc;
}

ui_error_t
md3_divider_set_orientation(struct md3_divider *divider,
                            enum ui_divider_orientation orientation) {
  if (!divider || (orientation != UI_DIVIDER_ORIENTATION_HORIZONTAL &&
                   orientation != UI_DIVIDER_ORIENTATION_VERTICAL)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  divider->orientation = orientation;
  return ui_divider_base_set_orientation(divider->base, orientation);
}

ui_error_t md3_divider_set_inset(struct md3_divider *divider, int is_inset) {
  if (!divider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  divider->is_inset = is_inset ? 1 : 0;
  return ui_divider_base_set_inset(divider->base, divider->is_inset);
}

ui_error_t md3_divider_get_base(struct md3_divider *divider,
                                struct ui_divider_base **out_base) {
  if (!divider || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = divider->base;
  return UI_ERROR_NONE;
}
