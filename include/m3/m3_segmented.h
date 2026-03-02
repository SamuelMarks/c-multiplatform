#ifndef M3_SEGMENTED_H
#define M3_SEGMENTED_H

/**
 * @file m3_segmented.h
 * @brief Material 3 Segmented Button widgets.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"
#include "m3_button.h"

/**
 * @brief Material 3 Segmented Button Group style descriptor.
 */
typedef struct M3SegmentedStyle {
  M3ButtonStyle segment_style; /**< Style applied to each segment. */
  CMPColor border_color;       /**< Group border color. */
  CMPScalar corner_radius;     /**< Group corner radius. */
  CMPScalar border_width;      /**< Border width. */
} M3SegmentedStyle;

struct M3SegmentedGroup;

/**
 * @brief Selection change callback.
 * @param ctx User context.
 * @param group The segmented group instance.
 * @param index The selected segment index.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *M3SegmentedOnChange)(void *ctx,
                                           struct M3SegmentedGroup *group,
                                           cmp_usize index);

/**
 * @brief Material 3 Segmented Button Group widget.
 */
typedef struct M3SegmentedGroup {
  CMPWidget widget;            /**< Widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing text. */
  M3SegmentedStyle style;      /**< Group styling. */
  M3Button **segments;         /**< Array of button segments. */
  cmp_usize segment_count;     /**< Number of segments. */
  cmp_usize capacity;          /**< Capacity of segment array. */
  cmp_usize selected_index;    /**< The currently selected segment index. */
  CMPBool multi_select;        /**< If CMP_TRUE, allows multiple selections. */
  CMPRect bounds;              /**< Layout bounds. */
  M3SegmentedOnChange on_change; /**< Selection change callback. */
  void *on_change_ctx;           /**< Callback context. */
} M3SegmentedGroup;

/**
 * @brief Initialize a segmented button style with defaults.
 * @param style Style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_style_init(M3SegmentedStyle *style);

/**
 * @brief Initialize a Material 3 segmented group.
 * @param group Group instance.
 * @param style Style descriptor.
 * @param text_backend Text backend.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_init(M3SegmentedGroup *group,
                                       const M3SegmentedStyle *style,
                                       CMPTextBackend text_backend);

/**
 * @brief Add a segment button to the group.
 * @param group Group instance.
 * @param utf8_label Button label text.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_add(M3SegmentedGroup *group,
                                      const char *utf8_label);

/**
 * @brief Set the selection callback.
 * @param group Group instance.
 * @param on_change Callback function.
 * @param ctx Callback context.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_set_on_change(M3SegmentedGroup *group,
                                                M3SegmentedOnChange on_change,
                                                void *ctx);

/**
 * @brief Set the selected index (single select mode).
 * @param group Group instance.
 * @param index Segment index to select.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_segmented_set_selected(M3SegmentedGroup *group,
                                               cmp_usize index);

/**
 * @brief Test wrapper helper.
 */
CMP_API int CMP_CALL m3_segmented_test_helper(void);

#ifdef __cplusplus
}
#endif

#endif /* M3_SEGMENTED_H */
