#ifndef M3_TEXT_FIELD_H
#define M3_TEXT_FIELD_H

/**
 * @file m3_text_field.h
 * @brief Material 3 Text Field widgets (Filled and Outlined variants).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_text_field.h"
#include "cmpc/cmp_visuals.h"

/** @brief Filled text field variant. */
#define M3_TEXT_FIELD_VARIANT_FILLED 1
/** @brief Outlined text field variant. */
#define M3_TEXT_FIELD_VARIANT_OUTLINED 2

/** @brief Default horizontal padding for M3 text fields. */
#define M3_TEXT_FIELD_DEFAULT_PADDING_X 16.0f
/** @brief Default vertical padding for M3 text fields. */
#define M3_TEXT_FIELD_DEFAULT_PADDING_Y 16.0f
/** @brief Default text field minimum height. */
#define M3_TEXT_FIELD_DEFAULT_MIN_HEIGHT 56.0f
/** @brief Default filled corner radius. */
#define M3_TEXT_FIELD_DEFAULT_FILLED_RADIUS 4.0f
/** @brief Default outlined corner radius. */
#define M3_TEXT_FIELD_DEFAULT_OUTLINED_RADIUS 4.0f

/**
 * @brief Material 3 Text Field style descriptor.
 */
typedef struct M3TextFieldStyle {
  cmp_u32 variant;                    /**< Variant (M3_TEXT_FIELD_VARIANT_*). */
  CMPTextFieldStyle core;             /**< Core text field styling. */
  CMPColor error_color;               /**< Color for error state. */
  CMPColor supporting_color;          /**< Color for supporting text. */
  CMPColor icon_color;                /**< Default icon color. */
  CMPTextStyle supporting_text_style; /**< Text style for supporting text. */
  CMPScalar icon_size;                /**< Icon size in pixels. */
} M3TextFieldStyle;

struct M3TextField;

/**
 * @brief Text field change callback.
 * @param ctx User context.
 * @param text_field The text field instance.
 * @param utf8_text The current text.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *M3TextFieldOnChange)(void *ctx,
                                           struct M3TextField *text_field,
                                           const char *utf8_text);

/**
 * @brief Material 3 Text Field widget.
 */
typedef struct M3TextField {
  CMPWidget widget;              /**< Widget interface. */
  CMPTextField core_field;       /**< Embedded core text field. */
  M3TextFieldStyle style;        /**< M3 specific styling. */
  CMPBool error;                 /**< Error state. */
  CMPBool disabled;              /**< Disabled state. */
  const char *utf8_supporting;   /**< Supporting text (may be NULL). */
  const char *utf8_error;        /**< Error text (may be NULL). */
  M3TextFieldOnChange on_change; /**< Change callback. */
  void *on_change_ctx;           /**< Callback context. */
} M3TextField;

/**
 * @brief Initialize an M3 text field style with defaults.
 * @param style Style descriptor to initialize.
 * @param variant M3_TEXT_FIELD_VARIANT_FILLED or _OUTLINED.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_text_field_style_init(M3TextFieldStyle *style,
                                              cmp_u32 variant);

/**
 * @brief Initialize a Material 3 text field.
 * @param text_field The text field instance.
 * @param style M3 text field style descriptor.
 * @param allocator Allocator for internal text storage.
 * @param text_backend Text backend for measurements.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_text_field_init(M3TextField *text_field,
                                        const M3TextFieldStyle *style,
                                        CMPAllocator allocator,
                                        CMPTextBackend text_backend);

/**
 * @brief Update the text field's style.
 * @param text_field Text field instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_text_field_set_style(M3TextField *text_field,
                                             const M3TextFieldStyle *style);

/**
 * @brief Set the floating label text.
 * @param text_field Text field instance.
 * @param utf8_label Label text (UTF-8).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_text_field_set_label(M3TextField *text_field,
                                             const char *utf8_label);

/**
 * @brief Set the placeholder text.
 * @param text_field Text field instance.
 * @param utf8_placeholder Placeholder text (UTF-8).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_text_field_set_placeholder(
    M3TextField *text_field, const char *utf8_placeholder);

/**
 * @brief Set the supporting text.
 * @param text_field Text field instance.
 * @param utf8_text Supporting text (UTF-8).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_text_field_set_supporting_text(M3TextField *text_field,
                                                       const char *utf8_text);

/**
 * @brief Set the error text and error state.
 * @param text_field Text field instance.
 * @param utf8_error Error text (UTF-8). Set NULL to clear error state.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_text_field_set_error(M3TextField *text_field,
                                             const char *utf8_error);

/**
 * @brief Enable or disable the text field.
 * @param text_field Text field instance.
 * @param disabled CMP_TRUE to disable, CMP_FALSE to enable.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_text_field_set_disabled(M3TextField *text_field,
                                                CMPBool disabled);

/**
 * @brief Set the change callback.
 * @param text_field Text field instance.
 * @param on_change Callback function.
 * @param ctx Callback context.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_text_field_set_on_change(M3TextField *text_field,
                                                 M3TextFieldOnChange on_change,
                                                 void *ctx);

/**
 * @brief Set text directly.
 * @param text_field Text field instance.
 * @param utf8_text Null-terminated text string.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_text_field_set_text(M3TextField *text_field,
                                            const char *utf8_text);

/**
 * @brief Get the current text length in bytes.
 * @param text_field Text field instance.
 * @param out_len Receives the length.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_text_field_get_text_len(const M3TextField *text_field,
                                                cmp_usize *out_len);

/**
 * @brief Helper for test coverage.
 */
CMP_API int CMP_CALL m3_text_field_test_helper(void);

#ifdef __cplusplus
}
#endif

#endif /* M3_TEXT_FIELD_H */
