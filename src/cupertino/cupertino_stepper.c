/* clang-format off */
#include "cupertino/cupertino_stepper.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL cupertino_stepper_init(CupertinoStepper *stepper) {
  if (stepper == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(stepper, 0, sizeof(*stepper));

  stepper->bounds.width = 94;
  stepper->bounds.height = 32; /* Standard iOS height */

  stepper->value = 0.0;
  stepper->min_value = 0.0;
  stepper->max_value = 100.0;
  stepper->step_value = 1.0;

  stepper->pressed_button = -1;
  stepper->is_dark_mode = CMP_FALSE;
  stepper->is_disabled = CMP_FALSE;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_stepper_set_value(CupertinoStepper *stepper,
                                                 double value) {
  if (stepper == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (value < stepper->min_value)
    value = stepper->min_value;
  if (value > stepper->max_value)
    value = stepper->max_value;

  stepper->value = value;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_stepper_paint(const CupertinoStepper *stepper,
                                             CMPPaintContext *ctx) {
  CMPColor bg_color;
  CMPColor pressed_bg_color;
  CMPColor icon_color;
  CMPColor disabled_icon_color;
  CMPColor divider_color;
  CMPScalar corner_radius = 8.0f;
  CMPScalar half_width;
  CMPRect minus_rect;
  CMPRect plus_rect;

  CMPBool minus_disabled;
  CMPBool plus_disabled;

  if (stepper == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  half_width = stepper->bounds.width / 2.0f;

  minus_rect = stepper->bounds;
  minus_rect.width = half_width;

  plus_rect = stepper->bounds;
  plus_rect.x += half_width;
  plus_rect.width = half_width;

  /* Colors */
  if (stepper->is_dark_mode) {
    bg_color.r = 0.11f;
    bg_color.g = 0.11f;
    bg_color.b = 0.118f;
    bg_color.a = 1.0f;
    pressed_bg_color.r = 0.2f;
    pressed_bg_color.g = 0.2f;
    pressed_bg_color.b = 0.2f;
    pressed_bg_color.a = 1.0f;
    icon_color.r = 1.0f;
    icon_color.g = 1.0f;
    icon_color.b = 1.0f;
    icon_color.a = 1.0f;
    disabled_icon_color.r = 0.3f;
    disabled_icon_color.g = 0.3f;
    disabled_icon_color.b = 0.3f;
    disabled_icon_color.a = 1.0f;
    divider_color.r = 0.3f;
    divider_color.g = 0.3f;
    divider_color.b = 0.3f;
    divider_color.a = 1.0f;
  } else {
    bg_color.r = 0.93f;
    bg_color.g = 0.93f;
    bg_color.b = 0.937f;
    bg_color.a = 1.0f;
    pressed_bg_color.r = 0.8f;
    pressed_bg_color.g = 0.8f;
    pressed_bg_color.b = 0.8f;
    pressed_bg_color.a = 1.0f;
    icon_color.r = 0.0f;
    icon_color.g = 0.0f;
    icon_color.b = 0.0f;
    icon_color.a = 1.0f;
    disabled_icon_color.r = 0.7f;
    disabled_icon_color.g = 0.7f;
    disabled_icon_color.b = 0.7f;
    disabled_icon_color.a = 1.0f;
    divider_color.r = 0.7f;
    divider_color.g = 0.7f;
    divider_color.b = 0.7f;
    divider_color.a = 1.0f;
  }

  minus_disabled =
      stepper->is_disabled || (stepper->value <= stepper->min_value);
  plus_disabled =
      stepper->is_disabled || (stepper->value >= stepper->max_value);

  /* Background (Draw single rounded rect) */
  if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &stepper->bounds, bg_color,
                                corner_radius);

    /* Highlight pressed buttons */
    if (stepper->pressed_button == 0 && !minus_disabled) {
      /* Draw left side pressed */
      CMPRect pr = minus_rect;
      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &pr, pressed_bg_color,
                                  corner_radius);
      /* Hack to un-round the inner edge if we used draw_rect directly:
       * Not ideal, but simple for now without complex clipping/paths */
    } else if (stepper->pressed_button == 1 && !plus_disabled) {
      /* Draw right side pressed */
      CMPRect pr = plus_rect;
      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &pr, pressed_bg_color,
                                  corner_radius);
    }
  }

  /* Central Divider */
  if (ctx->gfx->vtable->draw_line) {
    ctx->gfx->vtable->draw_line(
        ctx->gfx->ctx, plus_rect.x, stepper->bounds.y + 4.0f, plus_rect.x,
        stepper->bounds.y + stepper->bounds.height - 4.0f, divider_color, 1.0f);
  }

  /* Minus Icon */
  if (ctx->gfx->vtable->draw_line) {
    CMPScalar cx = minus_rect.x + (minus_rect.width / 2.0f);
    CMPScalar cy = minus_rect.y + (minus_rect.height / 2.0f);
    CMPScalar size = 6.0f;
    CMPColor c = minus_disabled ? disabled_icon_color : icon_color;

    ctx->gfx->vtable->draw_line(ctx->gfx->ctx, cx - size, cy, cx + size, cy, c,
                                2.0f);
  }

  /* Plus Icon */
  if (ctx->gfx->vtable->draw_line) {
    CMPScalar cx = plus_rect.x + (plus_rect.width / 2.0f);
    CMPScalar cy = plus_rect.y + (plus_rect.height / 2.0f);
    CMPScalar size = 6.0f;
    CMPColor c = plus_disabled ? disabled_icon_color : icon_color;

    ctx->gfx->vtable->draw_line(ctx->gfx->ctx, cx - size, cy, cx + size, cy, c,
                                2.0f);
    ctx->gfx->vtable->draw_line(ctx->gfx->ctx, cx, cy - size, cx, cy + size, c,
                                2.0f);
  }

  return CMP_OK;
}
