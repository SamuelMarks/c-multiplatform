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

int cmp_button_handle_press(cmp_button_t *button, int is_pressed,
                            float *out_opacity) {
  struct cmp_button *ctx = (struct cmp_button *)button;
  if (!ctx || !out_opacity)
    return CMP_ERROR_INVALID_ARG;

  ctx->is_pressed = is_pressed;

  if (is_pressed) {
    if (ctx->style == CMP_BUTTON_STYLE_PLAIN ||
        ctx->style == CMP_BUTTON_STYLE_CLOSE) {
      /* Instant dim to 30% without artificial delay (HIG) */
      *out_opacity = 0.3f;
    } else {
      /* Filled, Tinted, Gray buttons typically get a dark overlay or reduce
         background opacity slightly, not a full view opacity drop */
      *out_opacity = 0.8f; /* Approximate highlight state for solid buttons */
    }
  } else {
    *out_opacity = 1.0f;
  }

  return CMP_SUCCESS;
}

int cmp_button_resolve_background(cmp_button_t *button, float *out_bg_r,
                                  float *out_bg_g, float *out_bg_b,
                                  float *out_bg_a) {
  struct cmp_button *ctx = (struct cmp_button *)button;
  if (!ctx || !out_bg_r || !out_bg_g || !out_bg_b || !out_bg_a)
    return CMP_ERROR_INVALID_ARG;

  switch (ctx->style) {
  case CMP_BUTTON_STYLE_PLAIN:
    *out_bg_r = 0.0f;
    *out_bg_g = 0.0f;
    *out_bg_b = 0.0f;
    *out_bg_a = 0.0f; /* Transparent */
    break;
  case CMP_BUTTON_STYLE_FILLED:
    /* Assume standard System Blue accent color for test */
    *out_bg_r = 0.0f;
    *out_bg_g = 0.478f;
    *out_bg_b = 1.0f;
    *out_bg_a = 1.0f;
    break;
  case CMP_BUTTON_STYLE_TINTED:
    /* 15-20% opacity of tint color */
    *out_bg_r = 0.0f;
    *out_bg_g = 0.478f;
    *out_bg_b = 1.0f;
    *out_bg_a = 0.15f;
    break;
  case CMP_BUTTON_STYLE_GRAY:
    /* Translucent system gray */
    *out_bg_r = 0.5f;
    *out_bg_g = 0.5f;
    *out_bg_b = 0.5f;
    *out_bg_a = 0.2f;
    break;
  case CMP_BUTTON_STYLE_CLOSE:
    /* Small blurred gray background */
    *out_bg_r = 0.4f;
    *out_bg_g = 0.4f;
    *out_bg_b = 0.4f;
    *out_bg_a = 0.3f;
    break;
  }

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

  /* In a real engine, this would fire an impact haptic and trigger a spring
   * animation */
  /* cmp_haptics_trigger(CMP_HAPTIC_IMPACT_LIGHT); */

  return CMP_SUCCESS;
}

int cmp_toggle_resolve_visuals(cmp_toggle_t *toggle, float *out_track_r,
                               float *out_track_g, float *out_track_b,
                               float *out_track_a, float *out_thumb_x_offset) {
  struct cmp_toggle *ctx = (struct cmp_toggle *)toggle;
  if (!ctx || !out_track_r || !out_track_g || !out_track_b || !out_track_a ||
      !out_thumb_x_offset)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->is_on) {
    /* System Green when ON */
    *out_track_r = 0.2f;
    *out_track_g = 0.8f;
    *out_track_b = 0.2f;
    *out_track_a = 1.0f;
    *out_thumb_x_offset = 20.0f; /* Slide to the right */
  } else {
    /* Translucent gray when OFF */
    *out_track_r = 0.5f;
    *out_track_g = 0.5f;
    *out_track_b = 0.5f;
    *out_track_a = 0.3f;
    *out_thumb_x_offset = 2.0f; /* Slight padding from left edge */
  }

  return CMP_SUCCESS;
}
