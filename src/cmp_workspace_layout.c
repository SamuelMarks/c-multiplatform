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
 * @brief cmp_workspace_layout_create
 *
 * @param out_layout Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_workspace_layout_create(cmp_workspace_layout_t **out_layout) {
  cmp_workspace_layout_t *layout;
  int rc;

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
  return CMP_SUCCESS;
}

/**
 * @brief cmp_workspace_layout_destroy
 *
 * @param layout Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_workspace_layout_destroy(cmp_workspace_layout_t *layout) {
  int rc;

  if (!layout) {
    LOG_DEBUG("cmp_workspace_layout_destroy: layout is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(layout);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_workspace_layout_destroy: CMP_FREE failed\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_workspace_layout_set_pane_width
 *
 * @param layout Parameter description.
 * @param pane Parameter description.
 * @param width Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_workspace_layout_set_pane_width(cmp_workspace_layout_t *layout,
                                        cmp_pane_type_t pane, float width) {
  int rc;
  rc = 0;
  if (!layout || pane >= CMP_PANE_COUNT) {
    LOG_DEBUG("cmp_workspace_layout_set_pane_width: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (width < MIN_PANE_WIDTH) {
    width = MIN_PANE_WIDTH;
  }

  layout->pane_widths[pane] = width;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_workspace_layout_get_pane_width
 *
 * @param layout Parameter description.
 * @param pane Parameter description.
 * @param out_width Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_workspace_layout_get_pane_width(const cmp_workspace_layout_t *layout,
                                        cmp_pane_type_t pane,
                                        float *out_width) {
  int rc;
  rc = 0;
  if (!layout || pane >= CMP_PANE_COUNT || !out_width) {
    LOG_DEBUG("cmp_workspace_layout_get_pane_width: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_width = layout->pane_widths[pane];
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_workspace_layout_set_sidebar_glass
 *
 * @param layout Parameter description.
 * @param enable_glass Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_workspace_layout_set_sidebar_glass(cmp_workspace_layout_t *layout,
                                           int enable_glass) {
  int rc;
  rc = 0;
  if (!layout) {
    LOG_DEBUG("cmp_workspace_layout_set_sidebar_glass: layout is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  layout->sidebar_glass_enabled = enable_glass ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_workspace_layout_hit_test_splitters
 *
 * @param layout Parameter description.
 * @param x Parameter description.
 * @param y Parameter description.
 * @param out_is_over_splitter Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_workspace_layout_hit_test_splitters(
    const cmp_workspace_layout_t *layout, float x, float y,
    int *out_is_over_splitter) {
  int rc;
  rc = 0;
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

  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}