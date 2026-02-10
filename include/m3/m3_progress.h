#ifndef M3_PROGRESS_H
#define M3_PROGRESS_H

/**
 * @file m3_progress.h
 * @brief Progress indicators and slider widgets for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_ui.h"

/** @brief Default linear progress height in pixels. */
#define M3_LINEAR_PROGRESS_DEFAULT_HEIGHT 4.0f
/** @brief Default linear progress minimum width in pixels. */
#define M3_LINEAR_PROGRESS_DEFAULT_MIN_WIDTH 120.0f
/** @brief Default linear progress corner radius in pixels. */
#define M3_LINEAR_PROGRESS_DEFAULT_CORNER_RADIUS 2.0f

/** @brief Default circular progress diameter in pixels. */
#define M3_CIRCULAR_PROGRESS_DEFAULT_DIAMETER 48.0f
/** @brief Default circular progress stroke thickness in pixels. */
#define M3_CIRCULAR_PROGRESS_DEFAULT_THICKNESS 4.0f
/** @brief Default circular progress segment count. */
#define M3_CIRCULAR_PROGRESS_DEFAULT_SEGMENTS 48u
/** @brief Default circular progress start angle in radians (top). */
#define M3_CIRCULAR_PROGRESS_DEFAULT_START_ANGLE (-1.57079632679f)

/** @brief Default slider track length in pixels. */
#define M3_SLIDER_DEFAULT_LENGTH 200.0f
/** @brief Default slider track height in pixels. */
#define M3_SLIDER_DEFAULT_TRACK_HEIGHT 4.0f
/** @brief Default slider track corner radius in pixels. */
#define M3_SLIDER_DEFAULT_TRACK_CORNER_RADIUS 2.0f
/** @brief Default slider thumb radius in pixels. */
#define M3_SLIDER_DEFAULT_THUMB_RADIUS 10.0f
/** @brief Default slider step (0 for continuous). */
#define M3_SLIDER_DEFAULT_STEP 0.0f

/**
 * @brief Linear progress style descriptor.
 */
typedef struct M3LinearProgressStyle {
    M3Scalar height; /**< Track height in pixels (> 0). */
    M3Scalar min_width; /**< Minimum width in pixels (>= 0). */
    M3Scalar corner_radius; /**< Track corner radius in pixels (>= 0). */
    M3Color track_color; /**< Track background color. */
    M3Color indicator_color; /**< Filled indicator color. */
    M3Color disabled_track_color; /**< Track color when disabled. */
    M3Color disabled_indicator_color; /**< Indicator color when disabled. */
} M3LinearProgressStyle;

/**
 * @brief Linear progress indicator instance.
 */
typedef struct M3LinearProgress {
    M3Widget widget; /**< Widget interface (points to this instance). */
    M3LinearProgressStyle style; /**< Current style descriptor. */
    M3Rect bounds; /**< Layout bounds. */
    M3Scalar value; /**< Progress value (0..1). */
    const char *utf8_label; /**< UTF-8 label for semantics (optional). */
    m3_usize utf8_len; /**< UTF-8 label length in bytes. */
} M3LinearProgress;

/**
 * @brief Initialize a linear progress style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_linear_progress_style_init(M3LinearProgressStyle *style);

/**
 * @brief Initialize a linear progress indicator.
 * @param progress Linear progress instance.
 * @param style Style descriptor.
 * @param value Initial progress value (0..1).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_linear_progress_init(M3LinearProgress *progress, const M3LinearProgressStyle *style, M3Scalar value);

/**
 * @brief Update the linear progress value.
 * @param progress Linear progress instance.
 * @param value New progress value (0..1).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_linear_progress_set_value(M3LinearProgress *progress, M3Scalar value);

/**
 * @brief Retrieve the linear progress value.
 * @param progress Linear progress instance.
 * @param out_value Receives the progress value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_linear_progress_get_value(const M3LinearProgress *progress, M3Scalar *out_value);

/**
 * @brief Update the linear progress style.
 * @param progress Linear progress instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_linear_progress_set_style(M3LinearProgress *progress, const M3LinearProgressStyle *style);

/**
 * @brief Update the linear progress semantics label.
 * @param progress Linear progress instance.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Label length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_linear_progress_set_label(M3LinearProgress *progress, const char *utf8_label, m3_usize utf8_len);

/**
 * @brief Circular progress style descriptor.
 */
typedef struct M3CircularProgressStyle {
    M3Scalar diameter; /**< Indicator diameter in pixels (> 0). */
    M3Scalar thickness; /**< Stroke thickness in pixels (> 0). */
    M3Scalar start_angle; /**< Start angle in radians. */
    m3_u32 segments; /**< Segment count for approximation (>= 3). */
    M3Color track_color; /**< Track background color. */
    M3Color indicator_color; /**< Indicator arc color. */
    M3Color disabled_track_color; /**< Track color when disabled. */
    M3Color disabled_indicator_color; /**< Indicator color when disabled. */
} M3CircularProgressStyle;

/**
 * @brief Circular progress indicator instance.
 */
typedef struct M3CircularProgress {
    M3Widget widget; /**< Widget interface (points to this instance). */
    M3CircularProgressStyle style; /**< Current style descriptor. */
    M3Rect bounds; /**< Layout bounds. */
    M3Scalar value; /**< Progress value (0..1). */
    const char *utf8_label; /**< UTF-8 label for semantics (optional). */
    m3_usize utf8_len; /**< UTF-8 label length in bytes. */
} M3CircularProgress;

/**
 * @brief Initialize a circular progress style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_circular_progress_style_init(M3CircularProgressStyle *style);

/**
 * @brief Initialize a circular progress indicator.
 * @param progress Circular progress instance.
 * @param style Style descriptor.
 * @param value Initial progress value (0..1).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_circular_progress_init(M3CircularProgress *progress, const M3CircularProgressStyle *style, M3Scalar value);

/**
 * @brief Update the circular progress value.
 * @param progress Circular progress instance.
 * @param value New progress value (0..1).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_circular_progress_set_value(M3CircularProgress *progress, M3Scalar value);

/**
 * @brief Retrieve the circular progress value.
 * @param progress Circular progress instance.
 * @param out_value Receives the progress value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_circular_progress_get_value(const M3CircularProgress *progress, M3Scalar *out_value);

/**
 * @brief Update the circular progress style.
 * @param progress Circular progress instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_circular_progress_set_style(M3CircularProgress *progress, const M3CircularProgressStyle *style);

/**
 * @brief Update the circular progress semantics label.
 * @param progress Circular progress instance.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Label length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_circular_progress_set_label(M3CircularProgress *progress, const char *utf8_label, m3_usize utf8_len);

struct M3Slider;

/**
 * @brief Slider value change callback signature.
 * @param ctx User callback context pointer.
 * @param slider Slider instance that changed.
 * @param value New slider value.
 * @return M3_OK on success or a failure code.
 */
typedef int (M3_CALL *M3SliderOnChange)(void *ctx, struct M3Slider *slider, M3Scalar value);

/**
 * @brief Slider style descriptor.
 */
typedef struct M3SliderStyle {
    M3Scalar track_length; /**< Track length in pixels (> 0). */
    M3Scalar track_height; /**< Track height in pixels (> 0). */
    M3Scalar track_corner_radius; /**< Track corner radius in pixels (>= 0). */
    M3Scalar thumb_radius; /**< Thumb radius in pixels (> 0). */
    M3Color track_color; /**< Inactive track color. */
    M3Color active_track_color; /**< Active track color. */
    M3Color thumb_color; /**< Thumb fill color. */
    M3Color disabled_track_color; /**< Inactive track color when disabled. */
    M3Color disabled_active_track_color; /**< Active track color when disabled. */
    M3Color disabled_thumb_color; /**< Thumb color when disabled. */
} M3SliderStyle;

/**
 * @brief Slider widget instance.
 */
typedef struct M3Slider {
    M3Widget widget; /**< Widget interface (points to this instance). */
    M3SliderStyle style; /**< Current slider style. */
    M3Rect bounds; /**< Layout bounds. */
    M3Scalar min_value; /**< Minimum value (must be < max_value). */
    M3Scalar max_value; /**< Maximum value (must be > min_value). */
    M3Scalar value; /**< Current value. */
    M3Scalar step; /**< Step size (0 for continuous). */
    const char *utf8_label; /**< UTF-8 label for semantics (optional). */
    m3_usize utf8_len; /**< UTF-8 label length in bytes. */
    M3Bool pressed; /**< M3_TRUE when pointer is pressed. */
    M3SliderOnChange on_change; /**< Change callback (may be NULL). */
    void *on_change_ctx; /**< Change callback context pointer. */
} M3Slider;

/**
 * @brief Initialize a slider style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_slider_style_init(M3SliderStyle *style);

/**
 * @brief Initialize a slider widget.
 * @param slider Slider instance.
 * @param style Slider style descriptor.
 * @param min_value Minimum slider value.
 * @param max_value Maximum slider value.
 * @param value Initial slider value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_slider_init(M3Slider *slider, const M3SliderStyle *style, M3Scalar min_value, M3Scalar max_value,
    M3Scalar value);

/**
 * @brief Update the slider value.
 * @param slider Slider instance.
 * @param value New slider value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_slider_set_value(M3Slider *slider, M3Scalar value);

/**
 * @brief Retrieve the slider value.
 * @param slider Slider instance.
 * @param out_value Receives the current value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_slider_get_value(const M3Slider *slider, M3Scalar *out_value);

/**
 * @brief Update the slider value range.
 * @param slider Slider instance.
 * @param min_value New minimum value.
 * @param max_value New maximum value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_slider_set_range(M3Slider *slider, M3Scalar min_value, M3Scalar max_value);

/**
 * @brief Update the slider step size.
 * @param slider Slider instance.
 * @param step Step size (0 for continuous).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_slider_set_step(M3Slider *slider, M3Scalar step);

/**
 * @brief Update the slider style.
 * @param slider Slider instance.
 * @param style New slider style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_slider_set_style(M3Slider *slider, const M3SliderStyle *style);

/**
 * @brief Update the slider semantics label.
 * @param slider Slider instance.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Label length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_slider_set_label(M3Slider *slider, const char *utf8_label, m3_usize utf8_len);

/**
 * @brief Assign a slider change callback.
 * @param slider Slider instance.
 * @param on_change Change callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_slider_set_on_change(M3Slider *slider, M3SliderOnChange on_change, void *ctx);

#ifdef M3_TESTING
/**
 * @brief Set a progress test fail point.
 * @param fail_point Fail point identifier.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_set_fail_point(m3_u32 fail_point);

/**
 * @brief Clear progress test fail points.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_clear_fail_points(void);

/**
 * @brief Set the call index that forces color_set to fail.
 * @param call_count Call index at or after which color_set fails (0 disables).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_set_color_set_fail_after(m3_u32 call_count);

/**
 * @brief Set the call index that forces color_with_alpha to fail.
 * @param call_count Call index at or after which color_with_alpha fails (0 disables).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_set_color_alpha_fail_after(m3_u32 call_count);

/**
 * @brief Test wrapper for progress color validation.
 * @param color Color to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_validate_color(const M3Color *color);

/**
 * @brief Test wrapper for progress color assignment.
 * @param color Output color pointer.
 * @param r Red channel (0..1).
 * @param g Green channel (0..1).
 * @param b Blue channel (0..1).
 * @param a Alpha channel (0..1).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_color_set(M3Color *color, M3Scalar r, M3Scalar g, M3Scalar b, M3Scalar a);

/**
 * @brief Test wrapper for alpha modulation helper.
 * @param base Base color.
 * @param alpha Alpha multiplier (0..1).
 * @param out_color Receives the modulated color.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_color_with_alpha(const M3Color *base, M3Scalar alpha, M3Color *out_color);

/**
 * @brief Test wrapper for measure spec validation.
 * @param spec Measure spec to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_validate_measure_spec(M3MeasureSpec spec);

/**
 * @brief Test wrapper for measure application helper.
 * @param desired Desired size in pixels.
 * @param spec Measure spec to apply.
 * @param out_size Receives the resolved size.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_apply_measure(M3Scalar desired, M3MeasureSpec spec, M3Scalar *out_size);

/**
 * @brief Test wrapper for rectangle validation.
 * @param rect Rectangle to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_validate_rect(const M3Rect *rect);

/**
 * @brief Test wrapper for value range validation (0..1).
 * @param value Value to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_validate_value01(M3Scalar value);

/**
 * @brief Test wrapper for linear progress style validation.
 * @param style Style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_linear_validate_style(const M3LinearProgressStyle *style);

/**
 * @brief Test wrapper for linear progress color resolution.
 * @param progress Linear progress instance.
 * @param out_track Receives resolved track color.
 * @param out_indicator Receives resolved indicator color.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_linear_resolve_colors(const M3LinearProgress *progress, M3Color *out_track,
    M3Color *out_indicator);

/**
 * @brief Test wrapper for circular progress style validation.
 * @param style Style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_circular_validate_style(const M3CircularProgressStyle *style);

/**
 * @brief Test wrapper for circular progress color resolution.
 * @param progress Circular progress instance.
 * @param out_track Receives resolved track color.
 * @param out_indicator Receives resolved indicator color.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_circular_resolve_colors(const M3CircularProgress *progress, M3Color *out_track,
    M3Color *out_indicator);

/**
 * @brief Test wrapper for circular arc drawing helper.
 * @param gfx Graphics backend.
 * @param cx Center X coordinate.
 * @param cy Center Y coordinate.
 * @param radius Arc radius.
 * @param start_angle Start angle in radians.
 * @param end_angle End angle in radians.
 * @param color Arc color.
 * @param thickness Stroke thickness.
 * @param segments Segment count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_circular_draw_arc(M3Gfx *gfx, M3Scalar cx, M3Scalar cy, M3Scalar radius,
    M3Scalar start_angle, M3Scalar end_angle, M3Color color, M3Scalar thickness, m3_u32 segments);

/**
 * @brief Test wrapper for slider style validation.
 * @param style Style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_slider_validate_style(const M3SliderStyle *style);

/**
 * @brief Test wrapper for slider value-to-fraction conversion.
 * @param slider Slider instance.
 * @param out_fraction Receives the fraction (0..1).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_slider_value_to_fraction(const M3Slider *slider, M3Scalar *out_fraction);

/**
 * @brief Test wrapper for slider X-coordinate to value conversion.
 * @param slider Slider instance.
 * @param x X coordinate.
 * @param out_value Receives the slider value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_slider_value_from_x(const M3Slider *slider, M3Scalar x, M3Scalar *out_value);

/**
 * @brief Test wrapper for slider snapping helper.
 * @param value Raw value.
 * @param min_value Minimum value.
 * @param max_value Maximum value.
 * @param step Step size.
 * @param out_value Receives the snapped value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_slider_snap_value(M3Scalar value, M3Scalar min_value, M3Scalar max_value, M3Scalar step,
    M3Scalar *out_value);

/**
 * @brief Test wrapper for slider color resolution.
 * @param slider Slider instance.
 * @param out_track Receives track color.
 * @param out_active Receives active track color.
 * @param out_thumb Receives thumb color.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_slider_resolve_colors(const M3Slider *slider, M3Color *out_track, M3Color *out_active,
    M3Color *out_thumb);

/**
 * @brief Test wrapper for slider value update helper.
 * @param slider Slider instance.
 * @param next_value Next value to apply.
 * @param notify M3_TRUE to invoke the change callback.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_progress_test_slider_update_value(M3Slider *slider, M3Scalar next_value, M3Bool notify);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_PROGRESS_H */
