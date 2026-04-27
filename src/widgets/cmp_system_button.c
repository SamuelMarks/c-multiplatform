/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_button {
  cmp_button_style_t style;
  int is_pressed;
};

struct cmp_toggle {
  int is_on;
};

/**
 * @brief cmp_button_create
 *
 * @param out_button Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_button_create(cmp_button_t **out_button) {
  int rc;
  rc = 0;
  struct cmp_button *ctx;
  if (!out_button)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_button), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->style = CMP_BUTTON_STYLE_PLAIN;
  ctx->is_pressed = 0;

  *out_button = (cmp_button_t *)ctx;
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
 * @brief cmp_button_destroy
 *
 * @param button Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_button_destroy(cmp_button_t *button) {
  int rc;
  rc = 0;
  if (button)
    CMP_FREE(button);
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
 * @brief cmp_button_set_style
 *
 * @param button Parameter description.
 * @param style Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_button_set_style(cmp_button_t *button, cmp_button_style_t style) {
  int rc;
  rc = 0;
  struct cmp_button *ctx = (struct cmp_button *)button;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->style = style;
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
 * @brief cmp_button_handle_event
 *
 * @param button Parameter description.
 * @param event Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_button_handle_event(cmp_button_t *button, const cmp_event_t *event) {
  int rc;
  rc = 0;
  struct cmp_button *ctx = (struct cmp_button *)button;
  if (!ctx || !event)
    return CMP_ERROR_INVALID_ARG;

  if (event->action == CMP_ACTION_DOWN) {
    ctx->is_pressed = 1;
  } else if (event->action == CMP_ACTION_UP ||
             event->action == CMP_ACTION_CANCEL) {
    ctx->is_pressed = 0;
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

/**
 * @brief cmp_button_update
 *
 * @param button Parameter description.
 * @param dt_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_button_update(cmp_button_t *button, float dt_ms) {
  int rc;
  rc = 0;
  struct cmp_button *ctx = (struct cmp_button *)button;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  /* State machine updates (e.g. hover intent timeouts, long-press) would go
   * here */
  (void)dt_ms;
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
 * @brief cmp_button_get_state
 *
 * @param button Parameter description.
 * @param out_style Parameter description.
 * @param out_is_pressed Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_button_get_state(const cmp_button_t *button,
                         cmp_button_style_t *out_style, int *out_is_pressed) {
  int rc;
  rc = 0;
  struct cmp_button *ctx = (struct cmp_button *)button;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  if (out_style)
    *out_style = ctx->style;
  if (out_is_pressed)
    *out_is_pressed = ctx->is_pressed;
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
 * @brief cmp_toggle_create
 *
 * @param out_toggle Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_toggle_create(cmp_toggle_t **out_toggle) {
  int rc;
  rc = 0;
  struct cmp_toggle *ctx;
  if (!out_toggle)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_toggle), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->is_on = 0;

  *out_toggle = (cmp_toggle_t *)ctx;
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
 * @brief cmp_toggle_destroy
 *
 * @param toggle Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_toggle_destroy(cmp_toggle_t *toggle) {
  int rc;
  rc = 0;
  if (toggle)
    CMP_FREE(toggle);
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
 * @brief cmp_toggle_set_state
 *
 * @param toggle Parameter description.
 * @param is_on Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_toggle_set_state(cmp_toggle_t *toggle, int is_on) {
  int rc;
  rc = 0;
  struct cmp_toggle *ctx = (struct cmp_toggle *)toggle;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->is_on = is_on;
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
 * @brief cmp_toggle_handle_event
 *
 * @param toggle Parameter description.
 * @param event Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_toggle_handle_event(cmp_toggle_t *toggle, const cmp_event_t *event) {
  int rc;
  rc = 0;
  struct cmp_toggle *ctx = (struct cmp_toggle *)toggle;
  if (!ctx || !event)
    return CMP_ERROR_INVALID_ARG;

  if (event->action == CMP_ACTION_UP) {
    ctx->is_on = !ctx->is_on;
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

/**
 * @brief cmp_toggle_update
 *
 * @param toggle Parameter description.
 * @param dt_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_toggle_update(cmp_toggle_t *toggle, float dt_ms) {
  int rc;
  rc = 0;
  struct cmp_toggle *ctx = (struct cmp_toggle *)toggle;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  /* Animation states (e.g. spring progress) could be ticked here */
  (void)dt_ms;
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
 * @brief cmp_toggle_get_state
 *
 * @param toggle Parameter description.
 * @param out_is_on Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_toggle_get_state(const cmp_toggle_t *toggle, int *out_is_on) {
  int rc;
  rc = 0;
  struct cmp_toggle *ctx = (struct cmp_toggle *)toggle;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  if (out_is_on)
    *out_is_on = ctx->is_on;
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
