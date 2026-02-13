#ifndef CMP_TEXT_H
#define CMP_TEXT_H

/**
 * @file cmp_text.h
 * @brief Text primitives and widget stub for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"

/** @brief Text measurement output. */
typedef struct CMPTextMetrics {
  CMPScalar width;    /**< Measured width in pixels. */
  CMPScalar height;   /**< Measured height in pixels. */
  CMPScalar baseline; /**< Baseline offset in pixels. */
} CMPTextMetrics;

/**
 * @brief Font style descriptor.
 */
typedef struct CMPTextStyle {
  const char *utf8_family; /**< Font family name (UTF-8). */
  cmp_i32 size_px;         /**< Font size in pixels. */
  cmp_i32 weight;          /**< Font weight (100..900). */
  CMPBool italic;          /**< CMP_TRUE for italic. */
  CMPColor color;          /**< Text color. */
} CMPTextStyle;

/**
 * @brief Text backend descriptor.
 */
typedef struct CMPTextBackend {
  void *ctx;                   /**< Backend context pointer. */
  const CMPTextVTable *vtable; /**< Backend text virtual table. */
} CMPTextBackend;

/**
 * @brief Text widget stub.
 */
typedef struct CMPTextWidget {
  CMPWidget widget;       /**< Widget interface (points to this instance). */
  CMPTextBackend backend; /**< Text backend instance. */
  CMPTextStyle style;     /**< Text style. */
  CMPHandle font;         /**< Font handle. */
  CMPTextMetrics metrics; /**< Cached metrics. */
  const char *utf8;       /**< UTF-8 text pointer. */
  cmp_usize utf8_len;     /**< UTF-8 length in bytes. */
  CMPRect bounds;         /**< Layout bounds. */
  CMPBool owns_font;      /**< CMP_TRUE when widget owns the font. */
  CMPBool metrics_valid;  /**< CMP_TRUE when cached metrics are valid. */
} CMPTextWidget;

/**
 * @brief Initialize a text style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_style_init(CMPTextStyle *style);

/**
 * @brief Populate a text backend from a graphics interface.
 * @param gfx Graphics interface instance.
 * @param out_backend Receives the backend descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_backend_from_gfx(const CMPGfx *gfx,
                                               CMPTextBackend *out_backend);

/**
 * @brief Create a font using a text backend.
 * @param backend Text backend instance.
 * @param style Font style descriptor.
 * @param out_font Receives the font handle.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_font_create(const CMPTextBackend *backend,
                                          const CMPTextStyle *style,
                                          CMPHandle *out_font);

/**
 * @brief Destroy a font using a text backend.
 * @param backend Text backend instance.
 * @param font Font handle to destroy.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_font_destroy(const CMPTextBackend *backend,
                                           CMPHandle font);

/**
 * @brief Measure a UTF-8 buffer.
 * @param backend Text backend instance.
 * @param font Font handle to measure with.
 * @param utf8 UTF-8 byte buffer (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the UTF-8 buffer in bytes.
 * @param out_metrics Receives the text metrics.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_measure_utf8(const CMPTextBackend *backend,
                                           CMPHandle font, const char *utf8,
                                           cmp_usize utf8_len,
                                           CMPTextMetrics *out_metrics);

/**
 * @brief Measure a null-terminated UTF-8 string.
 * @param backend Text backend instance.
 * @param font Font handle to measure with.
 * @param utf8 UTF-8 null-terminated string.
 * @param out_metrics Receives the text metrics.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_measure_cstr(const CMPTextBackend *backend,
                                           CMPHandle font, const char *utf8,
                                           CMPTextMetrics *out_metrics);

/**
 * @brief Retrieve font metrics by measuring an empty string.
 * @param backend Text backend instance.
 * @param font Font handle to measure with.
 * @param out_metrics Receives the font metrics.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_font_metrics(const CMPTextBackend *backend,
                                           CMPHandle font,
                                           CMPTextMetrics *out_metrics);

/**
 * @brief Initialize a text widget.
 * @param widget Text widget instance.
 * @param backend Text backend instance.
 * @param style Text style (must include a valid family name).
 * @param utf8 UTF-8 text pointer (may be NULL when utf8_len is 0).
 * @param utf8_len Length of UTF-8 text in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_widget_init(CMPTextWidget *widget,
                                          const CMPTextBackend *backend,
                                          const CMPTextStyle *style,
                                          const char *utf8, cmp_usize utf8_len);

/**
 * @brief Update the text widget content.
 * @param widget Text widget instance.
 * @param utf8 UTF-8 text pointer (may be NULL when utf8_len is 0).
 * @param utf8_len Length of UTF-8 text in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_widget_set_text(CMPTextWidget *widget,
                                              const char *utf8,
                                              cmp_usize utf8_len);

/**
 * @brief Update the text widget style.
 * @param widget Text widget instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_widget_set_style(CMPTextWidget *widget,
                                               const CMPTextStyle *style);

/**
 * @brief Retrieve cached text metrics.
 * @param widget Text widget instance.
 * @param out_metrics Receives the text metrics.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_widget_get_metrics(CMPTextWidget *widget,
                                                 CMPTextMetrics *out_metrics);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for color validation.
 * @param color Color to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_test_validate_color(const CMPColor *color);

/**
 * @brief Test wrapper for style validation.
 * @param style Style to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_test_validate_style(const CMPTextStyle *style);

/**
 * @brief Test wrapper for backend validation.
 * @param backend Backend to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_text_test_validate_backend(const CMPTextBackend *backend);

/**
 * @brief Test wrapper for measure spec validation.
 * @param spec Measure spec to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_test_validate_measure_spec(CMPMeasureSpec spec);

/**
 * @brief Test wrapper for rectangle validation.
 * @param rect Rectangle to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_test_validate_rect(const CMPRect *rect);

/**
 * @brief Test wrapper to update widget metrics.
 * @param widget Text widget instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_test_metrics_update(CMPTextWidget *widget);

/**
 * @brief Test wrapper for C-string length helper.
 * @param cstr Null-terminated string.
 * @param out_len Receives string length.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_test_cstrlen(const char *cstr,
                                           cmp_usize *out_len);

/**
 * @brief Override the maximum C-string length used by text helpers.
 * @param max_len Maximum allowed length (0 disables override).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_test_set_cstr_limit(cmp_usize max_len);

/**
 * @brief Force cmp_text_style_init to fail after N calls (0 disables).
 * @param call_count Call count to fail on.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_text_test_set_style_init_fail_after(cmp_u32 call_count);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_TEXT_H */
