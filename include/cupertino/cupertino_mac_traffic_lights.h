#ifndef CUPERTINO_MAC_TRAFFIC_LIGHTS_H
#define CUPERTINO_MAC_TRAFFIC_LIGHTS_H

/**
 * @file cupertino_mac_traffic_lights.h
 * @brief Apple macOS style Window Control Buttons (Traffic Lights).
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cupertino/cupertino_color.h"
/* clang-format on */

/** @brief Cupertino macOS Traffic Lights Widget */
typedef struct CupertinoMacTrafficLights {
  CMPWidget widget;         /**< Base widget interface. */
  CMPRect bounds;           /**< Layout bounds. */
  CMPBool is_dark_mode;     /**< Dark mode styling. */
  CMPBool is_window_active; /**< CMP_TRUE if the window is currently focused. */
  CMPBool is_hovered; /**< CMP_TRUE if mouse is hovering over the controls. */
  int pressed_button; /**< -1: none, 0: close, 1: minimize, 2: zoom */
} CupertinoMacTrafficLights;

/**
 * @brief Initializes macOS traffic light controls.
 * @param lights Pointer to the traffic lights instance.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL
cupertino_mac_traffic_lights_init(CupertinoMacTrafficLights *lights);

/**
 * @brief Renders the traffic lights.
 * @param lights Pointer to the traffic lights instance.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_mac_traffic_lights_paint(
    const CupertinoMacTrafficLights *lights, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_MAC_TRAFFIC_LIGHTS_H */
