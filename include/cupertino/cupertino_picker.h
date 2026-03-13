#ifndef CUPERTINO_PICKER_H
#define CUPERTINO_PICKER_H

/**
 * @file cupertino_picker.h
 * @brief Apple Cupertino style Picker (Wheel).
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

/** @brief Maximum number of items in a picker. */
#define CUPERTINO_PICKER_MAX_ITEMS 32

/** @brief Cupertino Picker Widget */
typedef struct CupertinoPicker {
  CMPWidget widget;            /**< Base widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing labels. */

  const char *items[CUPERTINO_PICKER_MAX_ITEMS];      /**< Items in UTF-8. */
  cmp_usize item_lengths[CUPERTINO_PICKER_MAX_ITEMS]; /**< Lengths of the item
                                                         strings. */
  cmp_usize item_count;                               /**< Number of items. */

  cmp_i32 selected_index;  /**< Currently selected item index. */
  CMPScalar scroll_offset; /**< Current scroll offset in items (e.g., 1.5 means
                              halfway between 1 and 2). */
  CMPAnimController scroll_anim; /**< Animation for snapping/scrolling. */

  CMPRect bounds;       /**< Layout bounds. */
  CMPBool is_dark_mode; /**< Dark mode styling. */
  CMPBool is_disabled;  /**< Disabled state. */
} CupertinoPicker;

/**
 * @brief Initializes a Cupertino Picker.
 * @param picker Pointer to the picker instance.
 * @param text_backend Pointer to the text backend interface.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_picker_init(CupertinoPicker *picker,
                                           const CMPTextBackend *text_backend);

/**
 * @brief Adds an item to the picker.
 * @param picker Pointer to the picker.
 * @param item_utf8 UTF-8 item text.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_picker_add_item(CupertinoPicker *picker,
                                               const char *item_utf8);

/**
 * @brief Sets the selected item index.
 * @param picker Pointer to the picker.
 * @param index The index to select.
 * @param animated Whether to animate the transition.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_picker_set_selected(CupertinoPicker *picker,
                                                   cmp_i32 index,
                                                   CMPBool animated);

/**
 * @brief Updates the picker animations.
 * @param picker Pointer to the picker.
 * @param delta_time Elapsed time in seconds.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_picker_update(CupertinoPicker *picker,
                                             double delta_time);

/**
 * @brief Renders the picker.
 * @param picker Pointer to the picker.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_picker_paint(const CupertinoPicker *picker,
                                            CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_PICKER_H */
