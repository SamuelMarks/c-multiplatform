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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_PROGRESS_H */
