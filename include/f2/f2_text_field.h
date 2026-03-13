#ifndef F2_TEXT_FIELD_H
#define F2_TEXT_FIELD_H

/**
 * @file f2_text_field.h
 * @brief Microsoft Fluent 2 Text Field widgets.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_text_field.h"
#include "f2/f2_color.h"
/* clang-format on */

/** @brief Default horizontal padding for F2 text fields. */
#define F2_TEXT_FIELD_DEFAULT_PADDING_X 12.0f
/** @brief Default vertical padding for F2 text fields. */
#define F2_TEXT_FIELD_DEFAULT_PADDING_Y 5.0f
/** @brief Default text field minimum height. */
#define F2_TEXT_FIELD_DEFAULT_MIN_HEIGHT 32.0f
/** @brief Default corner radius. */
#define F2_TEXT_FIELD_DEFAULT_CORNER_RADIUS 4.0f

/**
 * @brief Fluent 2 Text Field style descriptor.
 */
typedef struct F2TextFieldStyle {
  CMPBool is_rtl;         /**< CMP_TRUE if layout should be right-to-left. */
  CMPTextFieldStyle core; /**< Core text field styling. */
  CMPColor error_color;   /**< Color for error state. */
  CMPColor bottom_line_color; /**< Color for the rest state bottom stroke. */
  CMPColor bottom_line_focus_color; /**< Color for the focused bottom stroke. */
} F2TextFieldStyle;

struct F2TextField;

/**
 * @brief Text field change callback.
 * @param ctx User context.
 * @param text_field The text field instance.
 * @param utf8_text The current text.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *F2TextFieldOnChange)(void *ctx,
                                           struct F2TextField *text_field,
                                           const char *utf8_text);

/**
 * @brief Fluent 2 Text Field widget.
 */
typedef struct F2TextField {
  CMPWidget widget;              /**< Widget interface. */
  CMPTextField core;             /**< Core text field widget instance. */
  F2TextFieldStyle style;        /**< Current style. */
  CMPBool is_error;              /**< CMP_TRUE if in error state. */
  CMPBool is_disabled;           /**< CMP_TRUE if disabled. */
  F2TextFieldOnChange on_change; /**< Change callback. */
  void *on_change_ctx;           /**< Change callback context. */
} F2TextField;

/**
 * @brief Initialize a default Fluent 2 Text Field style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_text_field_style_init(F2TextFieldStyle *style);

/**
 * @brief Initialize a Fluent 2 Text Field.
 * @param text_field Text field instance.
 * @param backend Text backend.
 * @param style Text field style.
 * @param allocator Allocator for internal text buffer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_text_field_init(F2TextField *text_field,
                                        const CMPTextBackend *backend,
                                        const F2TextFieldStyle *style,
                                        CMPAllocator *allocator);

/**
 * @brief Update the text field text.
 * @param text_field Text field instance.
 * @param utf8_text New UTF-8 text.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_text_field_set_text(F2TextField *text_field,
                                            const char *utf8_text);

/**
 * @brief Update the error state.
 * @param text_field Text field instance.
 * @param is_error New error state.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_text_field_set_error(F2TextField *text_field,
                                             CMPBool is_error);

/**
 * @brief Assign a text change callback.
 * @param text_field Text field instance.
 * @param on_change Change callback.
 * @param ctx Context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_text_field_set_on_change(F2TextField *text_field,
                                                 F2TextFieldOnChange on_change,
                                                 void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* F2_TEXT_FIELD_H */
