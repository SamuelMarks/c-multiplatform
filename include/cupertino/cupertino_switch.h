#ifndef CUPERTINO_SWITCH_H
#define CUPERTINO_SWITCH_H

/**
 * @file cupertino_switch.h
 * @brief Apple Cupertino style Switch component.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_anim.h"
#include "cupertino/cupertino_color.h"

/** @brief Cupertino Switch Widget */
typedef struct CupertinoSwitch {
    CMPWidget widget;             /**< Base widget interface. */
    CMPColor on_tint_color;       /**< Tint color when ON. */
    CMPColor thumb_tint_color;    /**< Thumb tint color. */
    CMPRect bounds;               /**< Layout bounds. */
    CMPAnimController anim;       /**< Animation controller for knob position. */
    CMPBool is_on;                /**< Current state. */
    CMPBool is_disabled;          /**< Disabled state. */
    CMPBool is_dark_mode;         /**< Dark mode styling. */
    CMPBool is_pressed;           /**< Is being interacted with. */
} CupertinoSwitch;

/**
 * @brief Initializes a Cupertino Switch.
 * @param switch_inst Pointer to the switch instance to initialize.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_switch_init(CupertinoSwitch *switch_inst);

/**
 * @brief Sets the on/off state of the switch.
 * @param switch_inst Pointer to the switch instance.
 * @param is_on CMP_TRUE for on, CMP_FALSE for off.
 * @param animated Whether to animate the transition.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_switch_set_on(CupertinoSwitch *switch_inst, CMPBool is_on, CMPBool animated);

/**
 * @brief Updates the internal state and animations of the switch.
 * @param switch_inst Pointer to the switch instance.
 * @param delta_time The elapsed time since the last update, in seconds.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_switch_update(CupertinoSwitch *switch_inst, double delta_time);

/**
 * @brief Renders the switch.
 * @param switch_inst Pointer to the switch instance.
 * @param ctx Pointer to the renderer used for drawing.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_switch_paint(const CupertinoSwitch *switch_inst, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_SWITCH_H */
