#ifndef CUPERTINO_SEGMENTED_CONTROL_H
#define CUPERTINO_SEGMENTED_CONTROL_H

/**
 * @file cupertino_segmented_control.h
 * @brief Apple Cupertino style Segmented Control.
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
#include "cmpc/cmp_text.h"
#include "cupertino/cupertino_color.h"
#include "cupertino/cupertino_typography.h"
/* clang-format on */

/** @brief Maximum number of segments in a segmented control. */
#define CUPERTINO_SEGMENTED_CONTROL_MAX_SEGMENTS 6

/** @brief Cupertino Segmented Control Widget */
typedef struct CupertinoSegmentedControl {
  CMPWidget widget;            /**< Base widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing labels. */

  const char
      *segments[CUPERTINO_SEGMENTED_CONTROL_MAX_SEGMENTS]; /**< Segment labels
                                                              in UTF-8. */
  cmp_usize
      segment_lengths[CUPERTINO_SEGMENTED_CONTROL_MAX_SEGMENTS]; /**< Lengths of
                                                                    the segment
                                                                    strings. */
  cmp_usize segment_count; /**< Number of segments. */

  cmp_i32 selected_index; /**< Currently selected segment index. */
  CMPAnimController
      selection_anim; /**< Animation for the selection background. */

  CMPRect bounds;       /**< Layout bounds. */
  CMPBool is_disabled;  /**< Disabled state. */
  CMPBool is_dark_mode; /**< Dark mode styling. */
  cmp_i32
      pressed_index; /**< Segment index currently being pressed (-1 if none). */
} CupertinoSegmentedControl;

/**
 * @brief Initializes a Cupertino Segmented Control.
 * @param control Pointer to the control instance.
 * @param text_backend Pointer to the text backend interface.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_segmented_control_init(
    CupertinoSegmentedControl *control, const CMPTextBackend *text_backend);

/**
 * @brief Adds a segment to the control.
 * @param control Pointer to the control instance.
 * @param label_utf8 Label of the segment.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_segmented_control_add_segment(
    CupertinoSegmentedControl *control, const char *label_utf8);

/**
 * @brief Sets the selected segment index.
 * @param control Pointer to the control instance.
 * @param index The index to select.
 * @param animated Whether to animate the transition.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_segmented_control_set_selected(
    CupertinoSegmentedControl *control, cmp_i32 index, CMPBool animated);

/**
 * @brief Updates the animations of the control.
 * @param control Pointer to the control instance.
 * @param delta_time Elapsed time in seconds.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_segmented_control_update(
    CupertinoSegmentedControl *control, double delta_time);

/**
 * @brief Renders the segmented control.
 * @param control Pointer to the control instance.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_segmented_control_paint(
    const CupertinoSegmentedControl *control, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_SEGMENTED_CONTROL_H */
