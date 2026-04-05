/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

#define SPLITTER_THICKNESS 8.0f
#define MIN_PANE_WIDTH 100.0f

struct cmp_workspace_layout {
  float pane_widths[CMP_PANE_COUNT];
  int sidebar_glass_enabled;
};

int cmp_workspace_layout_create(cmp_workspace_layout_t **out_layout) {
  cmp_workspace_layout_t *layout;

  if (!out_layout) {
    return CMP_ERROR_INVALID_ARG;
  }

  layout = (cmp_workspace_layout_t *)malloc(sizeof(cmp_workspace_layout_t));
  if (!layout) {
    return CMP_ERROR_OOM;
  }

  layout->pane_widths[CMP_PANE_SIDEBAR] = 250.0f;
  layout->pane_widths[CMP_PANE_CHAT] = 350.0f;
  layout->pane_widths[CMP_PANE_EDITOR] =
      800.0f; /* Takes remaining space typically */

  layout->sidebar_glass_enabled = 1;

  *out_layout = layout;
  return CMP_SUCCESS;
}

int cmp_workspace_layout_destroy(cmp_workspace_layout_t *layout) {
  if (!layout) {
    return CMP_ERROR_INVALID_ARG;
  }

  free(layout);
  return CMP_SUCCESS;
}

int cmp_workspace_layout_set_pane_width(cmp_workspace_layout_t *layout,
                                        cmp_pane_type_t pane, float width) {
  if (!layout || pane >= CMP_PANE_COUNT) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (width < MIN_PANE_WIDTH) {
    width = MIN_PANE_WIDTH;
  }

  layout->pane_widths[pane] = width;
  return CMP_SUCCESS;
}

int cmp_workspace_layout_get_pane_width(const cmp_workspace_layout_t *layout,
                                        cmp_pane_type_t pane,
                                        float *out_width) {
  if (!layout || pane >= CMP_PANE_COUNT || !out_width) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_width = layout->pane_widths[pane];
  return CMP_SUCCESS;
}

int cmp_workspace_layout_set_sidebar_glass(cmp_workspace_layout_t *layout,
                                           int enable_glass) {
  if (!layout) {
    return CMP_ERROR_INVALID_ARG;
  }

  layout->sidebar_glass_enabled = enable_glass ? 1 : 0;
  return CMP_SUCCESS;
}

int cmp_workspace_layout_hit_test_splitters(
    const cmp_workspace_layout_t *layout, float x, float y,
    int *out_is_over_splitter) {
  float s1_x, s2_x;

  if (!layout || !out_is_over_splitter) {
    return CMP_ERROR_INVALID_ARG;
  }

  (void)y; /* Currently y doesn't matter for full-height vertical splitters */

  s1_x = layout->pane_widths[CMP_PANE_SIDEBAR];
  s2_x = s1_x + SPLITTER_THICKNESS + layout->pane_widths[CMP_PANE_CHAT];

  if ((x >= s1_x && x <= s1_x + SPLITTER_THICKNESS) ||
      (x >= s2_x && x <= s2_x + SPLITTER_THICKNESS)) {
    *out_is_over_splitter = 1;
  } else {
    *out_is_over_splitter = 0;
  }

  return CMP_SUCCESS;
}
