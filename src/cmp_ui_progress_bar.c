/* clang-format off */
#include "cmp_ui_progress_bar.h"
#include <stdlib.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_ui_progress_bar {
  cmp_ui_node_t *node_track;
  cmp_ui_node_t *node_fill;
  float progress;
};

/**
 * @brief cmp_ui_progress_bar_create
 *
 * @param out_bar Parameter description.
 * @param track_color Parameter description.
 * @param fill_color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_progress_bar_create(cmp_ui_progress_bar_t **out_bar,
                               uint32_t track_color, uint32_t fill_color) {
  int rc = CMP_SUCCESS;
  cmp_ui_progress_bar_t *bar;
  int err;

  if (!out_bar) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_progress_bar_t), (void **)&(bar));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }
  memset(bar, 0, sizeof(cmp_ui_progress_bar_t));

  bar->progress = CMP_MATH_ZERO;

  err = cmp_ui_box_create(&bar->node_track);
  if (err != CMP_SUCCESS) {
    CMP_FREE(bar);
    return err;
  }

  bar->node_track->bg_color = track_color;
  bar->node_track->type = CMP_UI_NODE_TYPE_PROGRESS; /* Progress */
  bar->node_track->layout->measure_ctx = bar->node_track;
  bar->node_track->layout->measure_cb = cmp_ui_layout_measure_dispatch;

  err = cmp_ui_box_create(&bar->node_fill);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(bar->node_track);
    CMP_FREE(bar);
    return err;
  }

  bar->node_fill->bg_color = fill_color;

  err = cmp_ui_node_add_child(bar->node_track, bar->node_fill);
  if (err != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_progress_bar_create: cmp_ui_node_add_child failed\n");
    cmp_ui_node_destroy(bar->node_fill);
    cmp_ui_node_destroy(bar->node_track);
    CMP_FREE(bar);
    return err;
  }

  *out_bar = bar;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_progress_bar_destroy
 *
 * @param bar Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_progress_bar_destroy(cmp_ui_progress_bar_t *bar) {
  int rc = CMP_SUCCESS;
  if (!bar) {
    return CMP_ERROR_INVALID_ARG;
  }
  rc = CMP_FREE(bar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return rc;
}

/**
 * @brief cmp_ui_progress_bar_get_node
 *
 * @param bar Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_progress_bar_get_node(cmp_ui_progress_bar_t *bar,
                                 cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!bar || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = bar->node_track;

  return rc;
}

/**
 * @brief cmp_ui_progress_bar_set_progress
 *
 * @param bar Parameter description.
 * @param progress Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_progress_bar_set_progress(cmp_ui_progress_bar_t *bar,
                                     float progress) {
  int rc = CMP_SUCCESS;
  if (!bar) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (progress < CMP_MATH_ZERO) {
    progress = CMP_MATH_ZERO;
  } else if (progress > CMP_MATH_ONE) {
    progress = CMP_MATH_ONE;
  }

  bar->progress = progress;

  return rc;
}
