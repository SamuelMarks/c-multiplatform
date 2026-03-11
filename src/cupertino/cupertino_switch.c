/* clang-format off */
#include "cupertino/cupertino_switch.h"
#include "cupertino/cupertino_color.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL cupertino_switch_init(CupertinoSwitch *switch_inst) {
  if (switch_inst == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(switch_inst, 0, sizeof(*switch_inst));

  switch_inst->bounds.x = 0;
  switch_inst->bounds.y = 0;
  switch_inst->bounds.width = 51;
  switch_inst->bounds.height = 31;

  cupertino_color_get_system(CUPERTINO_COLOR_GREEN, CMP_FALSE,
                             &switch_inst->on_tint_color);

  switch_inst->thumb_tint_color.r = 1.0f;
  switch_inst->thumb_tint_color.g = 1.0f;
  switch_inst->thumb_tint_color.b = 1.0f;
  switch_inst->thumb_tint_color.a = 1.0f;

  switch_inst->is_on = CMP_FALSE;
  switch_inst->is_disabled = CMP_FALSE;
  switch_inst->is_dark_mode = CMP_FALSE;
  switch_inst->is_pressed = CMP_FALSE;

  cmp_anim_controller_init(&switch_inst->anim);
  /* Set initial knob position */
  switch_inst->anim.value = 0.0f; /* 0 for OFF, 1 for ON */

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_switch_set_on(CupertinoSwitch *switch_inst,
                                             CMPBool is_on, CMPBool animated) {
  CMPScalar target;

  if (switch_inst == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  switch_inst->is_on = is_on;
  target = is_on ? 1.0f : 0.0f;

  if (animated) {
    /* iOS spring defaults. Standard iOS switch spring is responsive. */
    cmp_anim_controller_start_spring(&switch_inst->anim,
                                     switch_inst->anim.value, target, 300.0f,
                                     25.0f, 1.0f);
  } else {
    cmp_anim_controller_stop(&switch_inst->anim);
    switch_inst->anim.value = target;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_switch_update(CupertinoSwitch *switch_inst,
                                             double delta_time) {
  CMPBool finished;
  CMPScalar out_val;

  if (switch_inst == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  cmp_anim_controller_step(&switch_inst->anim, (CMPScalar)delta_time, &out_val,
                           &finished);

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_switch_paint(const CupertinoSwitch *switch_inst,
                                            CMPPaintContext *ctx) {
  CMPRect track_rect;
  CMPRect knob_rect;
  CMPColor track_color;
  CMPColor shadow_color;
  CMPColor border_color;
  CMPScalar progress;
  CMPScalar track_radius;
  CMPScalar knob_radius;
  CMPScalar knob_padding;
  CMPScalar knob_offset_x;

  if (switch_inst == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  progress = switch_inst->anim.value;

  track_rect = switch_inst->bounds;
  track_radius = track_rect.height / 2.0f;

  /* Colors */
  if (switch_inst->is_dark_mode) {
    cupertino_color_get_system(CUPERTINO_COLOR_GRAY4, CMP_TRUE, &track_color);
  } else {
    cupertino_color_get_system(CUPERTINO_COLOR_GRAY5, CMP_FALSE, &track_color);
  }

  /* Interpolate track color */
  track_color.r =
      track_color.r + (switch_inst->on_tint_color.r - track_color.r) * progress;
  track_color.g =
      track_color.g + (switch_inst->on_tint_color.g - track_color.g) * progress;
  track_color.b =
      track_color.b + (switch_inst->on_tint_color.b - track_color.b) * progress;
  track_color.a =
      track_color.a + (switch_inst->on_tint_color.a - track_color.a) * progress;

  if (switch_inst->is_disabled) {
    track_color.a *= 0.5f;
  }

  /* Draw track */
  if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &track_rect, track_color,
                                track_radius);
  } else if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &track_rect, track_color, 0.0f);
  }

  /* Track inner border if OFF */
  if (progress < 1.0f && !switch_inst->is_dark_mode) {
    border_color.r = 0.9f;
    border_color.g = 0.9f;
    border_color.b = 0.9f;
    border_color.a = 1.0f * (1.0f - progress);
    if (ctx->gfx->vtable->draw_rect) {
      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &track_rect, border_color,
                                  track_radius);
    }
  }

  /* Knob */
  knob_padding = 2.0f;
  knob_radius = track_radius - knob_padding;

  /* Calculate knob X taking press state into account (knob expands slightly
   * when pressed) */
  knob_rect.y = track_rect.y + knob_padding;
  knob_rect.height = knob_radius * 2.0f;

  if (switch_inst->is_pressed) {
    knob_rect.width = knob_rect.height + 6.0f;
  } else {
    knob_rect.width = knob_rect.height;
  }

  knob_offset_x =
      progress * (track_rect.width - knob_rect.width - (knob_padding * 2.0f));
  knob_rect.x = track_rect.x + knob_padding + knob_offset_x;

  /* Shadow */
  shadow_color.r = 0.0f;
  shadow_color.g = 0.0f;
  shadow_color.b = 0.0f;
  shadow_color.a = 0.2f;

  if (ctx->gfx->vtable->draw_rect) {
    /* Simple shadow offset */
    CMPRect shadow_rect = knob_rect;
    shadow_rect.y += 2.0f;
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &shadow_rect, shadow_color,
                                knob_radius);
    /* Draw knob */
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &knob_rect,
                                switch_inst->thumb_tint_color, knob_radius);
  } else if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &knob_rect,
                                switch_inst->thumb_tint_color, 0.0f);
  }

  return CMP_OK;
}
