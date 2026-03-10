/* clang-format off */
#include "cupertino/cupertino_blur.h"
#include <stddef.h>
/* clang-format on */

CMP_API int CMP_CALL
cupertino_blur_get_effect(CupertinoBlurStyle style_id, CMPBool is_dark,
                          CupertinoBlurEffect *out_effect) {
  if (out_effect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* Base color fallback is usually a light or dark gray. */
  out_effect->base_color.r = is_dark ? 0.15f : 0.85f;
  out_effect->base_color.g = is_dark ? 0.15f : 0.85f;
  out_effect->base_color.b = is_dark ? 0.15f : 0.85f;
  out_effect->base_color.a = 1.0f;

  switch (style_id) {
  case CUPERTINO_BLUR_STYLE_ULTRA_THIN:
    out_effect->blur_radius = 10.0f;
    out_effect->tint_color.r = is_dark ? 0.15f : 0.98f;
    out_effect->tint_color.g = is_dark ? 0.15f : 0.98f;
    out_effect->tint_color.b = is_dark ? 0.15f : 0.98f;
    out_effect->tint_color.a = 0.25f;
    break;
  case CUPERTINO_BLUR_STYLE_THIN:
    out_effect->blur_radius = 20.0f;
    out_effect->tint_color.r = is_dark ? 0.15f : 0.98f;
    out_effect->tint_color.g = is_dark ? 0.15f : 0.98f;
    out_effect->tint_color.b = is_dark ? 0.15f : 0.98f;
    out_effect->tint_color.a = 0.40f;
    break;
  case CUPERTINO_BLUR_STYLE_NORMAL:
    out_effect->blur_radius = 30.0f;
    out_effect->tint_color.r = is_dark ? 0.15f : 0.98f;
    out_effect->tint_color.g = is_dark ? 0.15f : 0.98f;
    out_effect->tint_color.b = is_dark ? 0.15f : 0.98f;
    out_effect->tint_color.a = 0.55f;
    break;
  case CUPERTINO_BLUR_STYLE_THICK:
    out_effect->blur_radius = 40.0f;
    out_effect->tint_color.r = is_dark ? 0.15f : 0.98f;
    out_effect->tint_color.g = is_dark ? 0.15f : 0.98f;
    out_effect->tint_color.b = is_dark ? 0.15f : 0.98f;
    out_effect->tint_color.a = 0.70f;
    break;
  case CUPERTINO_BLUR_STYLE_CHROME:
    out_effect->blur_radius =
        30.0f; /* Usually a mix of normal blur and higher opacity tint */
    out_effect->tint_color.r = is_dark ? 0.10f : 0.95f;
    out_effect->tint_color.g = is_dark ? 0.10f : 0.95f;
    out_effect->tint_color.b = is_dark ? 0.10f : 0.95f;
    out_effect->tint_color.a = 0.85f;
    break;
  default:
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return CMP_OK;
}
