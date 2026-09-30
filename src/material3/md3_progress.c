/**
 * @file md3_progress.c
 * @brief Material 3 Linear, Circular, and Expressive Progress Indicators
 * implementation.
 */

/* clang-format off */
#include "material3/md3_progress.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t md3_progress_create(struct ui_engine *engine,
                               enum md3_progress_type type,
                               struct md3_progress **out_progress) {
  struct md3_progress *p;
  ui_error_t rc;

  if (!engine || !out_progress || (unsigned)type >= MD3_PROGRESS_TYPE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  p = (struct md3_progress *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_progress));
  if (!p) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(p, 0, sizeof(struct md3_progress));
  p->type = type;
  p->min = 0.0f;
  p->max = 100.0f;
  p->value = 0.0f;
  p->is_indeterminate = 0;

  rc = ui_progress_base_create(&p->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(p);
    return rc;
  }

  *out_progress = p;
  return UI_ERROR_NONE;
}

ui_error_t md3_progress_destroy(struct md3_progress *progress) {
  ui_error_t rc;

  if (!progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_progress_base_destroy(progress->base);
  C_MULTIPLATFORM_FREE(progress);
  return rc;
}

ui_error_t md3_progress_set_determinate(struct md3_progress *progress,
                                        float value, float min, float max) {
  if (!progress || min >= max) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  progress->value = value;
  progress->min = min;
  progress->max = max;
  progress->is_indeterminate = 0;
  return ui_progress_base_set_determinate(progress->base, value, min, max);
}

ui_error_t md3_progress_set_indeterminate(struct md3_progress *progress) {
  if (!progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  progress->is_indeterminate = 1;
  return ui_progress_base_set_indeterminate(progress->base);
}

ui_error_t md3_progress_get_percentage(const struct md3_progress *progress,
                                       float *out_percentage) {
  if (!progress || !out_percentage) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_progress_base_get_normalized_percentage(progress->base,
                                                    out_percentage);
}

ui_error_t md3_progress_is_indeterminate(const struct md3_progress *progress,
                                         int *out_is_indeterminate) {
  if (!progress || !out_is_indeterminate) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_progress_base_is_indeterminate(progress->base,
                                           out_is_indeterminate);
}

ui_error_t md3_progress_get_base(struct md3_progress *progress,
                                 struct ui_progress_base **out_base) {
  if (!progress || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = progress->base;
  return UI_ERROR_NONE;
}
