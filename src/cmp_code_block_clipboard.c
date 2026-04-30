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
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_clipboard_overlay_t *overlay = NULL;

  if (out_overlay == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_overlay_create: Invalid argument "
                  "(out_overlay=NULL): %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_clipboard_overlay_t), (void **)&overlay);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_overlay_create: Out of memory: %s\n", err_str);

    return rc;
  }

  overlay->state = CMP_CLIPBOARD_STATE_IDLE;

  *out_overlay = overlay;
  cmp_log_debug("cmp_clipboard_overlay_create: Successfully created clipboard "
                "overlay context\n");

  return rc;
}

/**
 * @brief cmp_clipboard_overlay_destroy
 *
 * @param overlay Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_clipboard_overlay_destroy(cmp_clipboard_overlay_t *overlay) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (overlay == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_overlay_destroy: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_FREE(overlay);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_clipboard_overlay_destroy: CMP_FREE failed\n");

    return rc;
  }

  cmp_log_debug("cmp_clipboard_overlay_destroy: Successfully destroyed "
                "clipboard overlay context\n");
  return rc;
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
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (overlay == NULL || block == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_overlay_render: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  overlay->state = state;
  /* Visual rendering logic goes here. E.g. update button opacity, icon, etc. */
  cmp_log_debug(
      "cmp_clipboard_overlay_render: Rendered clipboard overlay in state %d\n",
      (int)state);

  return rc;
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
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (overlay == NULL || window == NULL || raw_code == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_overlay_copy: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  /* Actually copy to the OS clipboard via cmp */
  rc = cmp_window_set_clipboard_text(window, CMP_CLIPBOARD_PRIMARY, raw_code);
  if (rc == CMP_SUCCESS) {
    overlay->state = CMP_CLIPBOARD_STATE_COPIED;
    cmp_log_debug(
        "cmp_clipboard_overlay_copy: Successfully copied to clipboard\n");
  } else {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_overlay_copy: cmp_window_set_clipboard_text "
                  "failed: %s\n",
                  err_str);
  }

  return rc;
}
