/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_clipboard_overlay {
  cmp_clipboard_state_t state;
};

/**
 * @brief cmp_clipboard_overlay_create
 *
 * @param out_overlay Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_clipboard_overlay_create(cmp_clipboard_overlay_t **out_overlay) {
  cmp_clipboard_overlay_t *overlay;
  int rc;

  if (!out_overlay) {
    LOG_DEBUG("cmp_clipboard_overlay_create: out_overlay is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_clipboard_overlay_t), (void **)&overlay);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_clipboard_overlay_create: OOM\n");
    return rc;
  }

  overlay->state = CMP_CLIPBOARD_STATE_IDLE;

  *out_overlay = overlay;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_clipboard_overlay_destroy
 *
 * @param overlay Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_clipboard_overlay_destroy(cmp_clipboard_overlay_t *overlay) {
  int rc;

  if (!overlay) {
    LOG_DEBUG("cmp_clipboard_overlay_destroy: overlay is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(overlay);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_clipboard_overlay_destroy: CMP_FREE failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_clipboard_overlay_render
 *
 * @param overlay Parameter description.
 * @param block Parameter description.
 * @param state Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_clipboard_overlay_render(cmp_clipboard_overlay_t *overlay,
                                 cmp_code_block_t *block,
                                 cmp_clipboard_state_t state) {
  if (!overlay || !block) {
    LOG_DEBUG("cmp_clipboard_overlay_render: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  overlay->state = state;
  /* Visual rendering logic goes here. E.g. update button opacity, icon, etc. */
  return CMP_SUCCESS;
}

/**
 * @brief cmp_clipboard_overlay_copy
 *
 * @param overlay Parameter description.
 * @param window Parameter description.
 * @param raw_code Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_clipboard_overlay_copy(cmp_clipboard_overlay_t *overlay,
                               cmp_window_t *window, const char *raw_code) {
  int result;

  if (!overlay || !window || !raw_code) {
    LOG_DEBUG("cmp_clipboard_overlay_copy: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Actually copy to the OS clipboard via cmp */
  result =
      cmp_window_set_clipboard_text(window, CMP_CLIPBOARD_PRIMARY, raw_code);
  if (result == CMP_SUCCESS) {
    overlay->state = CMP_CLIPBOARD_STATE_COPIED;
  } else {
    LOG_DEBUG(
        "cmp_clipboard_overlay_copy: cmp_window_set_clipboard_text failed\n");
  }

  return result;
}