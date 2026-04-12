/* clang-format off */
#include "cmp_ui_progress_bar.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ui_progress_bar {
  cmp_ui_node_t *node_track;
  cmp_ui_node_t *node_fill;
  float progress;
};

int cmp_ui_progress_bar_create(cmp_ui_progress_bar_t **out_bar,
                               uint32_t track_color, uint32_t fill_color) {
  cmp_ui_progress_bar_t *bar;
  int err;

  if (!out_bar) {
    return CMP_ERROR_INVALID_ARG;
  }

  bar = (cmp_ui_progress_bar_t *)malloc(sizeof(cmp_ui_progress_bar_t));
  if (!bar) {
    return CMP_ERROR_OOM;
  }

  bar->progress = 0.0f;

  err = cmp_ui_box_create(&bar->node_track);
  if (err != 0) {
    free(bar);
    return err;
  }

  bar->node_track->bg_color = track_color;

  err = cmp_ui_box_create(&bar->node_fill);
  if (err != 0) {
    free(bar);
    return err;
  }

  bar->node_fill->bg_color = fill_color;

  cmp_ui_node_add_child(bar->node_track, bar->node_fill);

  *out_bar = bar;
  return 0;
}

int cmp_ui_progress_bar_destroy(cmp_ui_progress_bar_t *bar) {
  if (!bar) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(bar);
  return 0;
}

int cmp_ui_progress_bar_get_node(cmp_ui_progress_bar_t *bar,
                                 cmp_ui_node_t **out_node) {
  if (!bar || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = bar->node_track;
  return 0;
}

int cmp_ui_progress_bar_set_progress(cmp_ui_progress_bar_t *bar,
                                     float progress) {
  if (!bar) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (progress < 0.0f) {
    progress = 0.0f;
  } else if (progress > 1.0f) {
    progress = 1.0f;
  }

  bar->progress = progress;
  return 0;
}
