#ifndef CUPERTINO_DETENT_SHEET_H
#define CUPERTINO_DETENT_SHEET_H

/**
 * @file cupertino_detent_sheet.h
 * @brief Apple iOS style Detent Bottom Sheet.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_anim.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cupertino/cupertino_color.h"
/* clang-format on */

/** @brief Cupertino Detent Sheet Widget */
typedef struct CupertinoDetentSheet {
  CMPWidget widget; /**< Base widget interface. */

  CMPRect screen_bounds; /**< Full screen/window bounds. */

  CMPScalar detent_hidden; /**< Y offset when hidden (usually screen height). */
  CMPScalar detent_medium; /**< Y offset for medium detent (half screen). */
  CMPScalar detent_large;  /**< Y offset for large detent (near top). */

  cmp_i32 current_detent;       /**< 0: hidden, 1: medium, 2: large */
  CMPAnimController slide_anim; /**< Animation for sheet sliding. */

  CMPBool is_dark_mode; /**< Dark mode styling. */
} CupertinoDetentSheet;

/**
 * @brief Initializes a Cupertino Detent Sheet.
 * @param sheet Pointer to the sheet instance.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_detent_sheet_init(CupertinoDetentSheet *sheet);

/**
 * @brief Sets the layout bounds of the screen containing the sheet.
 * @param sheet Pointer to the sheet.
 * @param bounds Full screen bounds.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL
cupertino_detent_sheet_set_bounds(CupertinoDetentSheet *sheet, CMPRect bounds);

/**
 * @brief Sets the active detent of the sheet.
 * @param sheet Pointer to the sheet.
 * @param detent 0 for hidden, 1 for medium, 2 for large.
 * @param animated Whether to animate the transition.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_detent_sheet_set_detent(
    CupertinoDetentSheet *sheet, cmp_i32 detent, CMPBool animated);

/**
 * @brief Updates the sheet animations.
 * @param sheet Pointer to the sheet.
 * @param delta_time Elapsed time in seconds.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_detent_sheet_update(CupertinoDetentSheet *sheet,
                                                   double delta_time);

/**
 * @brief Renders the sheet background and grabber.
 * Note: Content inside the sheet should be rendered separately relative to the
 * animated Y offset.
 * @param sheet Pointer to the sheet.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_detent_sheet_paint(
    const CupertinoDetentSheet *sheet, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_DETENT_SHEET_H */
