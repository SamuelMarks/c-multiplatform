/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

#define SPLITTER_THICKNESS 8.0f
#define MIN_PANE_WIDTH 100.0f

struct cmp_workspace_layout {
  float pane_widths[CMP_PANE_COUNT];
  int sidebar_glass_enabled;
};

/**
 * @brief Create a new workspace layout engine.
 *
 * @param out_layout Pointer to store the created layout engine handle.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_workspace_layout_create(cmp_workspace_layout_t **out_layout) {
  cmp_workspace_layout_t *layout = NULL;
  int rc = CMP_SUCCESS;

  if (!out_layout) {
    LOG_DEBUG("cmp_workspace_layout_create: out_layout is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_workspace_layout_t), (void **)&layout);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_workspace_layout_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  layout->pane_widths[CMP_PANE_SIDEBAR] = 250.0f;
  layout->pane_widths[CMP_PANE_CHAT] = 350.0f;
  layout->pane_widths[CMP_PANE_EDITOR] =
      800.0f; /* Takes remaining space typically */

  layout->sidebar_glass_enabled = 1;

  *out_layout = layout;
  return rc;
}

/**
 * @brief Destroy the layout engine.
 *
 * @param layout The layout engine to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_workspace_layout_destroy(cmp_workspace_layout_t *layout) {
  int rc = CMP_SUCCESS;

  if (!layout) {
    LOG_DEBUG("cmp_workspace_layout_destroy: layout is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(layout);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_workspace_layout_destroy: CMP_FREE failed\n");
    return rc;
  }
  return rc;
}

/**
 * @brief Set the width of a specific pane. Automatically handles splitter
 * constraints.
 *
 * @param layout The layout engine component.
 * @param pane The pane identifier.
 * @param width The target width to set for the pane.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_workspace_layout_set_pane_width(cmp_workspace_layout_t *layout,
                                        cmp_pane_type_t pane, float width) {
  int rc = CMP_SUCCESS;
  if (!layout || pane >= CMP_PANE_COUNT) {
    LOG_DEBUG("cmp_workspace_layout_set_pane_width: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (width < MIN_PANE_WIDTH) {
    width = MIN_PANE_WIDTH;
  }

  layout->pane_widths[pane] = width;

  return rc;
}

/**
 * @brief Get the width of a specific pane.
 *
 * @param layout The layout engine component.
 * @param pane The pane identifier.
 * @param out_width Pointer to store the result width.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_workspace_layout_get_pane_width(const cmp_workspace_layout_t *layout,
                                        cmp_pane_type_t pane,
                                        float *out_width) {
  int rc = CMP_SUCCESS;
  if (!layout || pane >= CMP_PANE_COUNT || !out_width) {
    LOG_DEBUG("cmp_workspace_layout_get_pane_width: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_width = layout->pane_widths[pane];

  return rc;
}

/**
 * @brief Enable or disable the glassy material effect on the sidebar.
 *
 * @param layout The layout engine component.
 * @param enable_glass 1 to enable, 0 to disable.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_workspace_layout_set_sidebar_glass(cmp_workspace_layout_t *layout,
                                           int enable_glass) {
  int rc = CMP_SUCCESS;
  if (!layout) {
    LOG_DEBUG("cmp_workspace_layout_set_sidebar_glass: layout is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  layout->sidebar_glass_enabled = enable_glass ? 1 : 0;

  return rc;
}

/**
 * @brief Perform hit-testing against the frameless splitters to allow resizing.
 *
 * @param layout The layout engine component.
 * @param x Mouse X coordinate.
 * @param y Mouse Y coordinate.
 * @param out_is_over_splitter Pointer to store boolean result (1 if over
 * splitter, 0 otherwise).
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_workspace_layout_hit_test_splitters(
    const cmp_workspace_layout_t *layout, float x, float y,
    int *out_is_over_splitter) {
  int rc = CMP_SUCCESS;
  float s1_x, s2_x;

  if (!layout || !out_is_over_splitter) {
    LOG_DEBUG("cmp_workspace_layout_hit_test_splitters: Invalid arg\n");
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

  return rc;
}
