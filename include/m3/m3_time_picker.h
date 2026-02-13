#ifndef M3_TIME_PICKER_H
#define M3_TIME_PICKER_H

/**
 * @file m3_time_picker.h
 * @brief Time picker widget for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_datetime.h"
#include "cmpc/cmp_layout.h"

/** @brief Minimum supported hour value. */
#define M3_TIME_PICKER_MIN_HOUR 0u
/** @brief Maximum supported hour value. */
#define M3_TIME_PICKER_MAX_HOUR 23u
/** @brief Minimum supported minute value. */
#define M3_TIME_PICKER_MIN_MINUTE 0u
/** @brief Maximum supported minute value. */
#define M3_TIME_PICKER_MAX_MINUTE 59u

/** @brief 12-hour time format. */
#define M3_TIME_PICKER_FORMAT_12H 0u
/** @brief 24-hour time format. */
#define M3_TIME_PICKER_FORMAT_24H 1u

/** @brief Active field is the hour hand. */
#define M3_TIME_PICKER_FIELD_HOUR 0u
/** @brief Active field is the minute hand. */
#define M3_TIME_PICKER_FIELD_MINUTE 1u

/** @brief AM period. */
#define M3_TIME_PICKER_PERIOD_AM 0u
/** @brief PM period. */
#define M3_TIME_PICKER_PERIOD_PM 1u

/** @brief Default dial diameter in pixels. */
#define M3_TIME_PICKER_DEFAULT_DIAMETER 240.0f
/** @brief Default dial ring thickness in pixels. */
#define M3_TIME_PICKER_DEFAULT_RING_THICKNESS 4.0f
/** @brief Default inner ring radius ratio for 24h mode. */
#define M3_TIME_PICKER_DEFAULT_INNER_RING_RATIO 0.65f
/** @brief Default hand thickness in pixels. */
#define M3_TIME_PICKER_DEFAULT_HAND_THICKNESS 2.0f
/** @brief Default hand center radius in pixels. */
#define M3_TIME_PICKER_DEFAULT_HAND_CENTER_RADIUS 4.0f
/** @brief Default padding around the dial in pixels. */
#define M3_TIME_PICKER_DEFAULT_PADDING 8.0f

/**
 * @brief Time picker style descriptor.
 */
typedef struct M3TimePickerStyle {
  CMPLayoutEdges padding;       /**< Padding around the dial. */
  CMPScalar diameter;           /**< Dial diameter in pixels (> 0). */
  CMPScalar ring_thickness;     /**< Dial ring thickness in pixels (> 0). */
  CMPScalar inner_ring_ratio;   /**< Inner ring radius ratio (0..1). */
  CMPScalar hand_thickness;     /**< Hand line thickness in pixels (> 0). */
  CMPScalar hand_center_radius; /**< Center knob radius in pixels (>= 0). */
  CMPColor background_color;    /**< Dial background color. */
  CMPColor ring_color;          /**< Dial ring color. */
  CMPColor hand_color;          /**< Hand color. */
  CMPColor selection_color;     /**< Selection accent color. */
  CMPColor disabled_color;      /**< Color for disabled state. */
} M3TimePickerStyle;

/**
 * @brief Time picker layout metrics.
 */
typedef struct M3TimePickerMetrics {
  CMPRect dial_bounds;    /**< Dial bounds in pixels. */
  CMPScalar center_x;     /**< Dial center X coordinate. */
  CMPScalar center_y;     /**< Dial center Y coordinate. */
  CMPScalar outer_radius; /**< Outer ring radius in pixels. */
  CMPScalar inner_radius; /**< Inner ring radius in pixels. */
} M3TimePickerMetrics;

struct M3TimePicker;

/**
 * @brief Time picker change callback signature.
 * @param ctx User callback context pointer.
 * @param picker Time picker instance that changed.
 * @param time Current time selection.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPTimePickerOnChange)(void *ctx,
                                           struct M3TimePicker *picker,
                                           const CMPTime *time);

/**
 * @brief Time picker widget instance.
 */
typedef struct M3TimePicker {
  CMPWidget widget;         /**< Widget interface (points to this instance). */
  M3TimePickerStyle style; /**< Current style. */
  CMPRect bounds;           /**< Layout bounds. */
  M3TimePickerMetrics metrics;    /**< Cached dial metrics. */
  cmp_u32 format;                  /**< Time format (CMP_TIME_PICKER_FORMAT_*). */
  cmp_u32 active_field;            /**< Active field (CMP_TIME_PICKER_FIELD_*). */
  CMPTime time;                    /**< Current time selection. */
  CMPBool pressed;                 /**< CMP_TRUE when pointer is pressed. */
  CMPTimePickerOnChange on_change; /**< Change callback (may be NULL). */
  void *on_change_ctx;            /**< Change callback context pointer. */
} M3TimePicker;

/**
 * @brief Initialize a time picker style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_style_init(M3TimePickerStyle *style);

/**
 * @brief Initialize a time picker widget.
 * @param picker Time picker instance.
 * @param style Style descriptor.
 * @param hour Initial hour (0..23).
 * @param minute Initial minute (0..59).
 * @param format Time format (CMP_TIME_PICKER_FORMAT_*).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_init(M3TimePicker *picker,
                                       const M3TimePickerStyle *style,
                                       cmp_u32 hour, cmp_u32 minute,
                                       cmp_u32 format);

/**
 * @brief Update the time picker style.
 * @param picker Time picker instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_set_style(M3TimePicker *picker,
                                            const M3TimePickerStyle *style);

/**
 * @brief Update the time format.
 * @param picker Time picker instance.
 * @param format Time format (CMP_TIME_PICKER_FORMAT_*).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_set_format(M3TimePicker *picker,
                                             cmp_u32 format);

/**
 * @brief Retrieve the time format.
 * @param picker Time picker instance.
 * @param out_format Receives the time format.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_get_format(const M3TimePicker *picker,
                                             cmp_u32 *out_format);

/**
 * @brief Update the active field.
 * @param picker Time picker instance.
 * @param field Active field (CMP_TIME_PICKER_FIELD_*).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_set_active_field(M3TimePicker *picker,
                                                   cmp_u32 field);

/**
 * @brief Retrieve the active field.
 * @param picker Time picker instance.
 * @param out_field Receives the active field.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_get_active_field(const M3TimePicker *picker,
                                                   cmp_u32 *out_field);

/**
 * @brief Update the selected time.
 * @param picker Time picker instance.
 * @param time New time selection.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_set_time(M3TimePicker *picker,
                                           const CMPTime *time);

/**
 * @brief Retrieve the selected time.
 * @param picker Time picker instance.
 * @param out_time Receives the time selection.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_get_time(const M3TimePicker *picker,
                                           CMPTime *out_time);

/**
 * @brief Update the AM/PM period (12-hour format only).
 * @param picker Time picker instance.
 * @param period Period value (CMP_TIME_PICKER_PERIOD_*).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_set_period(M3TimePicker *picker,
                                             cmp_u32 period);

/**
 * @brief Retrieve the AM/PM period.
 * @param picker Time picker instance.
 * @param out_period Receives the period (CMP_TIME_PICKER_PERIOD_*).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_get_period(const M3TimePicker *picker,
                                             cmp_u32 *out_period);

/**
 * @brief Assign a change callback.
 * @param picker Time picker instance.
 * @param on_change Change callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_set_on_change(M3TimePicker *picker,
                                                CMPTimePickerOnChange on_change,
                                                void *ctx);

/**
 * @brief Recompute cached dial metrics.
 * @param picker Time picker instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_update(M3TimePicker *picker);

#ifdef CMP_TESTING
/**
 * @brief Test hook to force dial-size zeroing in metrics computation.
 * @param enable CMP_TRUE to force the next dial computation to zero out.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_set_force_dial_size_zero(CMPBool enable);
/**
 * @brief Test hook to force angle-from-point failure.
 * @param enable CMP_TRUE to force the next angle computation to fail.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_set_force_angle_error(CMPBool enable);
/**
 * @brief Test hook to force angle-to-index failure.
 * @param enable CMP_TRUE to force the next index computation to fail.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_time_picker_test_set_force_angle_index_error(CMPBool enable);
/**
 * @brief Test hook to force invalid hour mapping in index conversion.
 * @param enable CMP_TRUE to force the next hour mapping to fail validation.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_set_force_hour_invalid(CMPBool enable);
/**
 * @brief Test hook to force hour-to-index conversion failure.
 * @param enable CMP_TRUE to force the next hour-to-index conversion to fail.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_time_picker_test_set_force_hour_to_index_error(CMPBool enable);
/**
 * @brief Test hook to force draw-circle rectangle validation failure.
 * @param enable CMP_TRUE to force the next circle draw to fail validation.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_set_force_rect_error(CMPBool enable);
/**
 * @brief Test hook to force resolve-colors failure.
 * @param enable CMP_TRUE to force the next resolve-colors call to fail.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_time_picker_test_set_force_resolve_colors_error(CMPBool enable);
/**
 * @brief Test hook to force compute-hand minute validation failure.
 * @param enable CMP_TRUE to force the next compute-hand minute check to fail.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_time_picker_test_set_force_compute_hand_minute_error(CMPBool enable);

/**
 * @brief Test hook for validating colors.
 * @param color Color to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_validate_color(const CMPColor *color);
/**
 * @brief Test hook for validating layout edges.
 * @param edges Edges to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_time_picker_test_validate_edges(const CMPLayoutEdges *edges);
/**
 * @brief Test hook for validating styles.
 * @param style Style to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_time_picker_test_validate_style(const M3TimePickerStyle *style);
/**
 * @brief Test hook for validating measure specs.
 * @param spec Measure spec to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_time_picker_test_validate_measure_spec(CMPMeasureSpec spec);
/**
 * @brief Test hook for validating rectangles.
 * @param rect Rectangle to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_validate_rect(const CMPRect *rect);
/**
 * @brief Test hook for validating times.
 * @param time Time to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_validate_time(const CMPTime *time);
/**
 * @brief Test hook for converting a point to an angle.
 * @param cx Dial center X coordinate.
 * @param cy Dial center Y coordinate.
 * @param x Point X coordinate.
 * @param y Point Y coordinate.
 * @param out_angle Receives the angle in radians.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_angle_from_point(CMPScalar cx,
                                                        CMPScalar cy, CMPScalar x,
                                                        CMPScalar y,
                                                        CMPScalar *out_angle);
/**
 * @brief Test hook for mapping an angle to a dial index.
 * @param angle Angle in radians.
 * @param count Dial slot count.
 * @param out_index Receives the index.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_angle_to_index(CMPScalar angle,
                                                      cmp_u32 count,
                                                      cmp_u32 *out_index);
/**
 * @brief Test hook for mapping a dial index to an hour.
 * @param index Dial index (0..11).
 * @param format Time format (CMP_TIME_PICKER_FORMAT_*).
 * @param inner_ring CMP_TRUE when using the inner ring in 24h mode.
 * @param current_hour Current hour used for 12h period mapping.
 * @param out_hour Receives the mapped hour.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_hour_from_index(cmp_u32 index,
                                                       cmp_u32 format,
                                                       CMPBool inner_ring,
                                                       cmp_u32 current_hour,
                                                       cmp_u32 *out_hour);
/**
 * @brief Test hook for mapping an hour to a dial index.
 * @param hour Hour to map (0..23).
 * @param format Time format (CMP_TIME_PICKER_FORMAT_*).
 * @param out_index Receives the dial index.
 * @param out_inner Receives CMP_TRUE when the inner ring is used.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_hour_to_index(cmp_u32 hour, cmp_u32 format,
                                                     cmp_u32 *out_index,
                                                     CMPBool *out_inner);
/**
 * @brief Test hook for computing dial metrics.
 * @param picker Time picker instance.
 * @param out_metrics Receives the computed metrics.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_compute_metrics(
    const M3TimePicker *picker, M3TimePickerMetrics *out_metrics);
/**
 * @brief Test hook for hit-testing the dial.
 * @param picker Time picker instance.
 * @param x X coordinate in pixels.
 * @param y Y coordinate in pixels.
 * @param field Active field (CMP_TIME_PICKER_FIELD_*).
 * @param out_time Receives the candidate time.
 * @param out_valid Receives CMP_TRUE when a valid selection is hit.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_pick_time(const M3TimePicker *picker,
                                                 CMPScalar x, CMPScalar y,
                                                 cmp_u32 field, CMPTime *out_time,
                                                 CMPBool *out_valid);
/**
 * @brief Test hook for resolving dial colors.
 * @param picker Time picker instance.
 * @param out_background Receives background color.
 * @param out_ring Receives ring color.
 * @param out_hand Receives hand color.
 * @param out_selection Receives selection color.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_resolve_colors(
    const M3TimePicker *picker, CMPColor *out_background, CMPColor *out_ring,
    CMPColor *out_hand, CMPColor *out_selection);
/**
 * @brief Test hook for computing hand angle and radius.
 * @param picker Time picker instance.
 * @param metrics Dial metrics to use.
 * @param out_angle Receives the hand angle in radians.
 * @param out_radius Receives the hand radius in pixels.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_compute_hand(
    const M3TimePicker *picker, const M3TimePickerMetrics *metrics,
    CMPScalar *out_angle, CMPScalar *out_radius);
/**
 * @brief Test hook for drawing a dial circle.
 * @param gfx Graphics backend instance.
 * @param cx Center X coordinate.
 * @param cy Center Y coordinate.
 * @param radius Circle radius in pixels.
 * @param color Fill color.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_draw_circle(CMPGfx *gfx, CMPScalar cx,
                                                   CMPScalar cy, CMPScalar radius,
                                                   CMPColor color);
/**
 * @brief Test hook for drawing a dial ring.
 * @param gfx Graphics backend instance.
 * @param cx Center X coordinate.
 * @param cy Center Y coordinate.
 * @param radius Ring radius in pixels.
 * @param thickness Ring thickness in pixels.
 * @param ring_color Ring color.
 * @param fill_color Inner fill color.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_draw_ring(CMPGfx *gfx, CMPScalar cx,
                                                 CMPScalar cy, CMPScalar radius,
                                                 CMPScalar thickness,
                                                 CMPColor ring_color,
                                                 CMPColor fill_color);
/**
 * @brief Test hook for updating cached metrics.
 * @param picker Time picker instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_time_picker_test_update_metrics(M3TimePicker *picker);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_TIME_PICKER_H */
