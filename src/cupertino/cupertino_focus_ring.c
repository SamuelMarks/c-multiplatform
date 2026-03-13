/* clang-format off */
#include "cupertino/cupertino_focus_ring.h"
#include "cupertino/cupertino_color.h"
/* clang-format on */

CMP_API int CMP_CALL cupertino_focus_ring_paint(CMPPaintContext *ctx,
                                                CMPRect bounds,
                                                CMPScalar corner_radius,
                                                CMPBool is_dark_mode) {
  CMPColor focus_color;
  CMPRect ring_bounds;
  CMPScalar thickness = 3.0f; /* Standard macOS focus ring expansion */

  if (ctx == NULL || ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* macOS accent/focus ring blue (slightly transparent) */
  cupertino_color_get_system(CUPERTINO_COLOR_BLUE, is_dark_mode, &focus_color);
  focus_color.a = 0.5f;

  ring_bounds = bounds;
  ring_bounds.x -= thickness;
  ring_bounds.y -= thickness;
  ring_bounds.width += thickness * 2.0f;
  ring_bounds.height += thickness * 2.0f;

  if (ctx->gfx->vtable->draw_rect) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &ring_bounds, focus_color,
                                corner_radius + thickness);
  }

  return CMP_OK;
}
