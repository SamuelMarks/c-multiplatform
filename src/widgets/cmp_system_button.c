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

int cmp_button_create(cmp_button_t **out_button) {
  struct cmp_button *ctx;
  if (!out_button)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_button), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->style = CMP_BUTTON_STYLE_PLAIN;
  ctx->is_pressed = 0;

  *out_button = (cmp_button_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_button_destroy(cmp_button_t *button) {
  if (button)
    CMP_FREE(button);
  return CMP_SUCCESS;
}

int cmp_button_set_style(cmp_button_t *button, cmp_button_style_t style) {
  struct cmp_button *ctx = (struct cmp_button *)button;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->style = style;
  return CMP_SUCCESS;
}

int cmp_button_handle_event(cmp_button_t *button, const cmp_event_t *event) {
  struct cmp_button *ctx = (struct cmp_button *)button;
  if (!ctx || !event)
    return CMP_ERROR_INVALID_ARG;

  if (event->action == CMP_ACTION_DOWN) {
    ctx->is_pressed = 1;
  } else if (event->action == CMP_ACTION_UP ||
             event->action == CMP_ACTION_CANCEL) {
    ctx->is_pressed = 0;
  }

  return CMP_SUCCESS;
}

int cmp_button_update(cmp_button_t *button, float dt_ms) {
  struct cmp_button *ctx = (struct cmp_button *)button;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  /* State machine updates (e.g. hover intent timeouts, long-press) would go
   * here */
  (void)dt_ms;
  return CMP_SUCCESS;
}

int cmp_button_get_state(const cmp_button_t *button,
                         cmp_button_style_t *out_style, int *out_is_pressed) {
  struct cmp_button *ctx = (struct cmp_button *)button;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  if (out_style)
    *out_style = ctx->style;
  if (out_is_pressed)
    *out_is_pressed = ctx->is_pressed;
  return CMP_SUCCESS;
}

int cmp_toggle_create(cmp_toggle_t **out_toggle) {
  struct cmp_toggle *ctx;
  if (!out_toggle)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_toggle), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->is_on = 0;

  *out_toggle = (cmp_toggle_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_toggle_destroy(cmp_toggle_t *toggle) {
  if (toggle)
    CMP_FREE(toggle);
  return CMP_SUCCESS;
}

int cmp_toggle_set_state(cmp_toggle_t *toggle, int is_on) {
  struct cmp_toggle *ctx = (struct cmp_toggle *)toggle;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->is_on = is_on;
  return CMP_SUCCESS;
}

int cmp_toggle_handle_event(cmp_toggle_t *toggle, const cmp_event_t *event) {
  struct cmp_toggle *ctx = (struct cmp_toggle *)toggle;
  if (!ctx || !event)
    return CMP_ERROR_INVALID_ARG;

  if (event->action == CMP_ACTION_UP) {
    ctx->is_on = !ctx->is_on;
  }

  return CMP_SUCCESS;
}

int cmp_toggle_update(cmp_toggle_t *toggle, float dt_ms) {
  struct cmp_toggle *ctx = (struct cmp_toggle *)toggle;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  /* Animation states (e.g. spring progress) could be ticked here */
  (void)dt_ms;
  return CMP_SUCCESS;
}

int cmp_toggle_get_state(const cmp_toggle_t *toggle, int *out_is_on) {
  struct cmp_toggle *ctx = (struct cmp_toggle *)toggle;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  if (out_is_on)
    *out_is_on = ctx->is_on;
  return CMP_SUCCESS;
}
