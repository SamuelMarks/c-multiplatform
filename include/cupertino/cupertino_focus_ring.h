#ifndef CUPERTINO_FOCUS_RING_H
#define CUPERTINO_FOCUS_RING_H

/**
 * @file cupertino_focus_ring.h
 * @brief Apple macOS style Focus Rings.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ui.h"

/**
 * @brief Paints a macOS-style focus ring around a given bound.
 * @param ctx Paint context.
 * @param bounds Target element bounds.
 * @param corner_radius Target element corner radius.
 * @param is_dark_mode True if dark mode styling is required.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_focus_ring_paint(CMPPaintContext *ctx, CMPRect bounds, CMPScalar corner_radius, CMPBool is_dark_mode);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_FOCUS_RING_H */
