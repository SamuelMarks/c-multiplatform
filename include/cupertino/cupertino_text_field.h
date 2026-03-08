#ifndef CUPERTINO_TEXT_FIELD_H
#define CUPERTINO_TEXT_FIELD_H

/**
 * @file cupertino_text_field.h
 * @brief Apple Cupertino style Text Field component.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_text.h"
#include "cupertino/cupertino_color.h"
#include "cupertino/cupertino_typography.h"

#define CUPERTINO_TEXT_FIELD_MAX_LENGTH 256

/** @brief Text field styles matching iOS UIKit */
typedef enum CupertinoTextFieldStyle {
    CUPERTINO_TEXT_FIELD_STYLE_ROUNDED_RECT = 0,
    CUPERTINO_TEXT_FIELD_STYLE_LINE,
    CUPERTINO_TEXT_FIELD_STYLE_BEZEL,
    CUPERTINO_TEXT_FIELD_STYLE_PLAIN
} CupertinoTextFieldStyle;

/** @brief Cupertino Text Field Widget */
typedef struct CupertinoTextField {
    CMPWidget widget;                             /**< Base widget interface. */
    CMPTextBackend text_backend;                  /**< Text backend for drawing. */
    
    char text[CUPERTINO_TEXT_FIELD_MAX_LENGTH];   /**< Text buffer. */
    cmp_usize text_len;                           /**< Current text length. */
    
    char placeholder[CUPERTINO_TEXT_FIELD_MAX_LENGTH]; /**< Placeholder buffer. */
    cmp_usize placeholder_len;                    /**< Placeholder length. */
    
    CupertinoTextFieldStyle style;                /**< Visual style. */
    CMPRect bounds;                               /**< Layout bounds. */
    
    CMPBool is_dark_mode;                         /**< Dark mode styling. */
    CMPBool is_focused;                           /**< Focus state (showing cursor/focus ring). */
    CMPBool is_disabled;                          /**< Disabled state. */
    
    double cursor_blink_time;                     /**< Timer for cursor blinking. */
} CupertinoTextField;

/**
 * @brief Initializes a Cupertino Text Field.
 * @param field Pointer to the text field instance.
 * @param text_backend Pointer to the text backend interface.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_text_field_init(CupertinoTextField *field, const CMPTextBackend *text_backend);

/**
 * @brief Sets the text of the field.
 * @param field Pointer to the text field instance.
 * @param text_utf8 UTF-8 encoded string.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_text_field_set_text(CupertinoTextField *field, const char *text_utf8);

/**
 * @brief Sets the placeholder text.
 * @param field Pointer to the text field instance.
 * @param placeholder_utf8 UTF-8 encoded string.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_text_field_set_placeholder(CupertinoTextField *field, const char *placeholder_utf8);

/**
 * @brief Updates the text field (cursor blink).
 * @param field Pointer to the text field instance.
 * @param delta_time Elapsed time in seconds.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_text_field_update(CupertinoTextField *field, double delta_time);

/**
 * @brief Renders the text field.
 * @param field Pointer to the text field instance.
 * @param ctx Pointer to the paint context.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_text_field_paint(const CupertinoTextField *field, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_TEXT_FIELD_H */
