/* clang-format off */
#include "cupertino/cupertino_slider.h"
#include "cupertino/cupertino_color.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL cupertino_slider_init(CupertinoSlider *slider) {
  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(slider, 0, sizeof(*slider));

  slider->bounds.x = 0;
  slider->bounds.y = 0;
  slider->bounds.width = 150;
  slider->bounds.height =
      30; /* Height is the touch area; visual track is thinner */

  slider->min_value = 0.0f;
  slider->max_value = 1.0f;
  slider->value = 0.5f;

  cupertino_color_get_system(CUPERTINO_COLOR_BLUE, CMP_FALSE,
                             &slider->min_track_color);
  cupertino_color_get_system(CUPERTINO_COLOR_GRAY5, CMP_FALSE,
                             &slider->max_track_color);

  slider->thumb_color.r = 1.0f;
  slider->thumb_color.g = 1.0f;
  slider->thumb_color.b = 1.0f;
  slider->thumb_color.a = 1.0f;

  slider->is_disabled = CMP_FALSE;
  slider->is_dark_mode = CMP_FALSE;
  slider->is_pressed = CMP_FALSE;

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_slider_set_value(CupertinoSlider *slider,
                                                CMPScalar value) {
  if (slider == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (value < slider->min_value) {
    value = slider->min_value;
  }
  if (value > slider->max_value) {
    value = slider->max_value;
  }

  slider->value = value;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_slider_paint(const CupertinoSlider *slider,
                                            CMPPaintContext *ctx) {
  CMPRect track_rect;
  CMPRect min_track_rect;
  CMPRect max_track_rect;
  CMPRect thumb_rect;
  CMPColor thumb_shadow_color;
  CMPColor current_max_track_color;
  CMPScalar progress;
  CMPScalar track_height;
  CMPScalar track_radius;
  CMPScalar thumb_radius;
  CMPScalar thumb_x;

  if (slider == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  track_height = 4.0f; /* iOS slider track is about 4pt thick */
  track_radius = track_height / 2.0f;

  if (slider->max_value > slider->min_value) {
    progress = (slider->value - slider->min_value) /
               (slider->max_value - slider->min_value);
  } else {
    progress = 0.0f;
  }

  /* Track Layout */
  track_rect.x = slider->bounds.x;
  track_rect.y =
      slider->bounds.y + (slider->bounds.height / 2.0f) - (track_height / 2.0f);
  track_rect.width = slider->bounds.width;
  track_rect.height = track_height;

  /* Thumb */
  thumb_radius = 14.0f; /* 28x28 thumb */
  if (slider->is_pressed) {
    thumb_radius += 2.0f; /* Thumb expands slightly when pressed */
  }

  /* Keep thumb inside bounds */
  thumb_x = track_rect.x + thumb_radius +
            progress * (track_rect.width - thumb_radius * 2.0f);

  min_track_rect = track_rect;
  min_track_rect.width = thumb_x - track_rect.x;

  max_track_rect = track_rect;
  max_track_rect.x = thumb_x;
  max_track_rect.width = (track_rect.x + track_rect.width) - thumb_x;

  thumb_rect.x = thumb_x - thumb_radius;
  thumb_rect.y =
      slider->bounds.y + (slider->bounds.height / 2.0f) - thumb_radius;
  thumb_rect.width = thumb_radius * 2.0f;
  thumb_rect.height = thumb_radius * 2.0f;

  /* Dark mode track color adaptation */
  current_max_track_color = slider->max_track_color;
  if (slider->is_dark_mode) {
    cupertino_color_get_system(CUPERTINO_COLOR_GRAY4, CMP_TRUE,
                               &current_max_track_color);
  }

  /* Draw Min Track */
  if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &min_track_rect,
                                slider->min_track_color, track_radius);
  } else if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &min_track_rect,
                                slider->min_track_color, 0.0f);
  }

  /* Draw Max Track */
  if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &max_track_rect,
                                current_max_track_color, track_radius);
  } else if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &max_track_rect,
                                current_max_track_color, 0.0f);
  }

  /* Thumb Shadow */
  thumb_shadow_color.r = 0.0f;
  thumb_shadow_color.g = 0.0f;
  thumb_shadow_color.b = 0.0f;
  thumb_shadow_color.a = 0.25f;

  /* Draw Thumb */
  if (ctx->gfx->vtable->draw_rect) {
    CMPRect shadow_rect = thumb_rect;
    shadow_rect.y += 2.0f;

    /* Shadow */
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &shadow_rect, thumb_shadow_color,
                                thumb_radius);
    /* Knob */
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &thumb_rect, slider->thumb_color,
                                thumb_radius);

    /* Thin border for light mode thumb */
    if (!slider->is_dark_mode) {
      CMPColor border_color = {0.8f, 0.8f, 0.8f, 1.0f};
      if (ctx->gfx->vtable->draw_rect) {
        ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &thumb_rect, border_color,
                                    thumb_radius);
      }
    }
  } else if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &thumb_rect, slider->thumb_color,
                                0.0f);
  }

  return CMP_OK;
}
