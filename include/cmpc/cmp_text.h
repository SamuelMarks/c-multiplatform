#ifndef CMP_TEXT_H
#define CMP_TEXT_H

/**
 * @file cmp_text.h
 * @brief Text primitives and widget for LibCMPC.
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
 * @brief Shaped text glyph descriptor.
 */
typedef struct CMPTextGlyph {
  cmp_u32 codepoint;   /**< Unicode codepoint or font glyph index. */
  CMPScalar x_advance; /**< X advance in pixels. */
  CMPScalar y_advance; /**< Y advance in pixels. */
  CMPScalar x_offset;  /**< X offset in pixels. */
  CMPScalar y_offset;  /**< Y offset in pixels. */
} CMPTextGlyph;

/**
 * @brief Complete shaped text layout representation.
 */
typedef struct CMPTextLayout {
  CMPTextGlyph *glyphs;   /**< Array of shaped glyphs. */
  cmp_usize glyph_count;  /**< Number of glyphs in the array. */
  CMPTextMetrics metrics; /**< Complete dimensions and baseline. */
} CMPTextLayout;

/**
 * @brief Font style descriptor.
 */
typedef struct CMPTextStyle {
  const char *
      utf8_family; /**< Font family name or fallback chain (comma-separated). */
  cmp_i32 size_px; /**< Font size in pixels. */
  cmp_i32 weight;  /**< Font weight (100..900) mapped to 'wght' axis. */
  CMPBool italic;  /**< CMP_TRUE for italic (legacy support). */
  CMPScalar
      width_axis; /**< 'wdth' variable font axis (e.g. 100.0 for normal). */
  CMPScalar optical_size; /**< 'opsz' variable font axis (e.g. 14.0). */
  CMPScalar slant;        /**< 'slnt' variable font axis (e.g. 0.0 to -90.0). */
  CMPScalar grade;        /**< 'GRAD' variable font axis (e.g. 0.0). */
  CMPScalar letter_spacing; /**< Letter spacing (tracking) in pixels. */
  CMPScalar line_height_px; /**< Line height in pixels (0.0 for auto). */
  CMPColor color;           /**< Text color. */
} CMPTextStyle;

/**
 * @brief Text backend descriptor.
 */
typedef struct CMPTextBackend {
  void *ctx;                   /**< Backend context pointer. */
  const CMPTextVTable *vtable; /**< Backend text virtual table. */
} CMPTextBackend;

/**
 * @brief Text widget.
 */
typedef struct CMPTextWidget {
  CMPWidget widget;       /**< Widget interface (points to this instance). */
  CMPTextBackend backend; /**< Text backend instance. */
  CMPTextStyle style;     /**< Text style. */
  CMPHandle font;         /**< Font handle. */
  CMPTextMetrics metrics; /**< Cached metrics. */
  const char *utf8;       /**< UTF-8 text pointer. */
  cmp_usize utf8_len;     /**< UTF-8 length in bytes. */
  cmp_u32 base_direction; /**< Base text direction. */
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
 * @param base_direction Base direction.
 * @param out_metrics Receives the text metrics.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_measure_utf8(const CMPTextBackend *backend,
                                           CMPHandle font, const char *utf8,
                                           cmp_usize utf8_len,
                                           cmp_u32 base_direction,
                                           CMPTextMetrics *out_metrics);

/**
 * @brief Measure a null-terminated UTF-8 string.
 * @param backend Text backend instance.
 * @param font Font handle to measure with.
 * @param utf8 UTF-8 null-terminated string.
 * @param base_direction Base direction.
 * @param out_metrics Receives the text metrics.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_measure_cstr(const CMPTextBackend *backend,
                                           CMPHandle font, const char *utf8,
                                           cmp_u32 base_direction,
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
 * @brief Shape UTF-8 text into a glyph layout (for BiDi/complex typography).
 * @param backend Text backend instance.
 * @param font Font handle.
 * @param utf8 UTF-8 string to shape.
 * @param utf8_len Length of string in bytes.
 * @param base_direction Base text direction.
 * @param out_layout Receives the shaped layout (caller must call free_layout).
 * @return CMP_OK on success, CMP_ERR_UNSUPPORTED if backend lacks shaping, or a
 * failure code.
 */
CMP_API int CMP_CALL cmp_text_shape_utf8(const CMPTextBackend *backend,
                                         CMPHandle font, const char *utf8,
                                         cmp_usize utf8_len,
                                         cmp_u32 base_direction,
                                         CMPTextLayout *out_layout);

/**
 * @brief Free a text layout created by cmp_text_shape_utf8.
 * @param backend Text backend instance.
 * @param layout The layout to free.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_free_layout(const CMPTextBackend *backend,
                                          CMPTextLayout *layout);

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

#if defined(CMP_TESTING) && !defined(DOXYGEN_SHOULD_SKIP_THIS)
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
