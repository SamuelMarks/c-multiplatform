#ifndef M3_TEXT_H
#define M3_TEXT_H

/**
 * @file m3_text.h
 * @brief Text primitives and widget stub for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_ui.h"

/** @brief Text measurement output. */
typedef struct M3TextMetrics {
    M3Scalar width; /**< Measured width in pixels. */
    M3Scalar height; /**< Measured height in pixels. */
    M3Scalar baseline; /**< Baseline offset in pixels. */
} M3TextMetrics;

/**
 * @brief Font style descriptor.
 */
typedef struct M3TextStyle {
    const char *utf8_family; /**< Font family name (UTF-8). */
    m3_i32 size_px; /**< Font size in pixels. */
    m3_i32 weight; /**< Font weight (100..900). */
    M3Bool italic; /**< M3_TRUE for italic. */
    M3Color color; /**< Text color. */
} M3TextStyle;

/**
 * @brief Text backend descriptor.
 */
typedef struct M3TextBackend {
    void *ctx; /**< Backend context pointer. */
    const M3TextVTable *vtable; /**< Backend text virtual table. */
} M3TextBackend;

/**
 * @brief Text widget stub.
 */
typedef struct M3TextWidget {
    M3Widget widget; /**< Widget interface (points to this instance). */
    M3TextBackend backend; /**< Text backend instance. */
    M3TextStyle style; /**< Text style. */
    M3Handle font; /**< Font handle. */
    M3TextMetrics metrics; /**< Cached metrics. */
    const char *utf8; /**< UTF-8 text pointer. */
    m3_usize utf8_len; /**< UTF-8 length in bytes. */
    M3Rect bounds; /**< Layout bounds. */
    M3Bool owns_font; /**< M3_TRUE when widget owns the font. */
    M3Bool metrics_valid; /**< M3_TRUE when cached metrics are valid. */
} M3TextWidget;

/**
 * @brief Initialize a text style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_style_init(M3TextStyle *style);

/**
 * @brief Populate a text backend from a graphics interface.
 * @param gfx Graphics interface instance.
 * @param out_backend Receives the backend descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_backend_from_gfx(const M3Gfx *gfx, M3TextBackend *out_backend);

/**
 * @brief Create a font using a text backend.
 * @param backend Text backend instance.
 * @param style Font style descriptor.
 * @param out_font Receives the font handle.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_font_create(const M3TextBackend *backend, const M3TextStyle *style, M3Handle *out_font);

/**
 * @brief Destroy a font using a text backend.
 * @param backend Text backend instance.
 * @param font Font handle to destroy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_font_destroy(const M3TextBackend *backend, M3Handle font);

/**
 * @brief Measure a UTF-8 buffer.
 * @param backend Text backend instance.
 * @param font Font handle to measure with.
 * @param utf8 UTF-8 byte buffer (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the UTF-8 buffer in bytes.
 * @param out_metrics Receives the text metrics.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_measure_utf8(const M3TextBackend *backend, M3Handle font, const char *utf8, m3_usize utf8_len,
    M3TextMetrics *out_metrics);

/**
 * @brief Measure a null-terminated UTF-8 string.
 * @param backend Text backend instance.
 * @param font Font handle to measure with.
 * @param utf8 UTF-8 null-terminated string.
 * @param out_metrics Receives the text metrics.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_measure_cstr(const M3TextBackend *backend, M3Handle font, const char *utf8, M3TextMetrics *out_metrics);

/**
 * @brief Retrieve font metrics by measuring an empty string.
 * @param backend Text backend instance.
 * @param font Font handle to measure with.
 * @param out_metrics Receives the font metrics.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_font_metrics(const M3TextBackend *backend, M3Handle font, M3TextMetrics *out_metrics);

/**
 * @brief Initialize a text widget.
 * @param widget Text widget instance.
 * @param backend Text backend instance.
 * @param style Text style (must include a valid family name).
 * @param utf8 UTF-8 text pointer (may be NULL when utf8_len is 0).
 * @param utf8_len Length of UTF-8 text in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_widget_init(M3TextWidget *widget, const M3TextBackend *backend, const M3TextStyle *style,
    const char *utf8, m3_usize utf8_len);

/**
 * @brief Update the text widget content.
 * @param widget Text widget instance.
 * @param utf8 UTF-8 text pointer (may be NULL when utf8_len is 0).
 * @param utf8_len Length of UTF-8 text in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_widget_set_text(M3TextWidget *widget, const char *utf8, m3_usize utf8_len);

/**
 * @brief Update the text widget style.
 * @param widget Text widget instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_widget_set_style(M3TextWidget *widget, const M3TextStyle *style);

/**
 * @brief Retrieve cached text metrics.
 * @param widget Text widget instance.
 * @param out_metrics Receives the text metrics.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_widget_get_metrics(M3TextWidget *widget, M3TextMetrics *out_metrics);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_TEXT_H */
