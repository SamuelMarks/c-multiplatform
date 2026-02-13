#ifndef CMP_TEXT_FIELD_H
#define CMP_TEXT_FIELD_H

/**
 * @file cmp_text_field.h
 * @brief Text field widget and editing helpers for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_anim.h"
#include "cmpc/cmp_text.h"

/** @brief Default horizontal padding for text fields. */
#define CMP_TEXT_FIELD_DEFAULT_PADDING_X 12.0f
/** @brief Default vertical padding for text fields. */
#define CMP_TEXT_FIELD_DEFAULT_PADDING_Y 8.0f
/** @brief Default minimum text field height. */
#define CMP_TEXT_FIELD_DEFAULT_MIN_HEIGHT 48.0f
/** @brief Default corner radius for text fields. */
#define CMP_TEXT_FIELD_DEFAULT_CORNER_RADIUS 4.0f
/** @brief Default outline width for text fields. */
#define CMP_TEXT_FIELD_DEFAULT_OUTLINE_WIDTH 1.0f
/** @brief Default floating label offset from the top edge. */
#define CMP_TEXT_FIELD_DEFAULT_LABEL_FLOAT_OFFSET 6.0f
/** @brief Default label animation duration in seconds. */
#define CMP_TEXT_FIELD_DEFAULT_LABEL_ANIM_DURATION 0.15f
/** @brief Default cursor width in pixels. */
#define CMP_TEXT_FIELD_DEFAULT_CURSOR_WIDTH 1.0f
/** @brief Default cursor blink period in seconds. */
#define CMP_TEXT_FIELD_DEFAULT_CURSOR_BLINK 1.0f
/** @brief Default selection handle radius in pixels. */
#define CMP_TEXT_FIELD_DEFAULT_HANDLE_RADIUS 4.0f
/** @brief Default selection handle height in pixels. */
#define CMP_TEXT_FIELD_DEFAULT_HANDLE_HEIGHT 12.0f

struct CMPTextField;

/**
 * @brief Text field change callback signature.
 * @param ctx User callback context pointer.
 * @param field Text field instance that changed.
 * @param utf8 UTF-8 text buffer (may be NULL when empty).
 * @param utf8_len Length of the text buffer in bytes.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPTextFieldOnChange)(void *ctx, struct CMPTextField *field,
                                          const char *utf8, cmp_usize utf8_len);

/**
 * @brief Text field style descriptor.
 */
typedef struct CMPTextFieldStyle {
  CMPTextStyle text_style;             /**< Text style for input content. */
  CMPTextStyle label_style;            /**< Text style for the floating label. */
  CMPColor container_color;            /**< Container fill color. */
  CMPColor outline_color;              /**< Outline color (idle). */
  CMPColor focused_outline_color;      /**< Outline color when focused. */
  CMPColor disabled_container_color;   /**< Container color when disabled. */
  CMPColor disabled_outline_color;     /**< Outline color when disabled. */
  CMPColor disabled_text_color;        /**< Text color when disabled. */
  CMPColor disabled_label_color;       /**< Label color when disabled. */
  CMPColor placeholder_color;          /**< Placeholder text color. */
  CMPColor disabled_placeholder_color; /**< Placeholder color when disabled. */
  CMPColor cursor_color;               /**< Cursor color. */
  CMPColor selection_color;            /**< Selection highlight color. */
  CMPColor handle_color;               /**< Selection handle color. */
  CMPScalar outline_width;             /**< Outline width in pixels (>= 0). */
  CMPScalar corner_radius;             /**< Corner radius in pixels (>= 0). */
  CMPScalar padding_x;          /**< Horizontal padding in pixels (>= 0). */
  CMPScalar padding_y;          /**< Vertical padding in pixels (>= 0). */
  CMPScalar min_height;         /**< Minimum height in pixels (>= 0). */
  CMPScalar label_float_offset; /**< Label floating offset from top (>= 0). */
  CMPScalar
      label_anim_duration; /**< Label animation duration in seconds (>= 0). */
  CMPScalar cursor_width;   /**< Cursor width in pixels (>= 0). */
  CMPScalar cursor_blink_period; /**< Cursor blink period in seconds (>= 0). */
  CMPScalar handle_radius; /**< Selection handle radius in pixels (>= 0). */
  CMPScalar handle_height; /**< Selection handle height in pixels (>= 0). */
} CMPTextFieldStyle;

/**
 * @brief Text field widget instance.
 */
typedef struct CMPTextField {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend
      text_backend;       /**< Text backend for measurements and rendering. */
  CMPTextFieldStyle style; /**< Current text field style. */
  CMPAllocator allocator;  /**< Allocator for internal text buffers. */
  CMPHandle text_font;     /**< Font handle for input text. */
  CMPHandle label_font;    /**< Font handle for labels. */
  CMPTextMetrics text_metrics;        /**< Cached text metrics. */
  CMPTextMetrics label_metrics;       /**< Cached label metrics. */
  CMPTextMetrics placeholder_metrics; /**< Cached placeholder metrics. */
  CMPTextMetrics text_font_metrics;   /**< Cached font metrics for text. */
  CMPTextMetrics label_font_metrics;  /**< Cached font metrics for label. */
  CMPRect bounds;                     /**< Layout bounds. */
  char *utf8;               /**< Owned UTF-8 text buffer (null-terminated). */
  cmp_usize utf8_len;        /**< UTF-8 length in bytes. */
  cmp_usize utf8_capacity;   /**< UTF-8 buffer capacity in bytes. */
  cmp_usize cursor;          /**< Cursor byte offset in UTF-8 buffer. */
  cmp_usize selection_start; /**< Selection start byte offset. */
  cmp_usize selection_end;   /**< Selection end byte offset. */
  CMPBool selecting; /**< CMP_TRUE when pointer selection drag is active. */
  CMPBool text_metrics_valid; /**< CMP_TRUE when cached text metrics are valid. */
  CMPBool
      label_metrics_valid; /**< CMP_TRUE when cached label metrics are valid. */
  CMPBool placeholder_metrics_valid; /**< CMP_TRUE when cached placeholder metrics
                                       are valid. */
  CMPBool font_metrics_valid; /**< CMP_TRUE when cached font metrics are valid. */
  CMPBool owns_fonts;         /**< CMP_TRUE when widget owns font handles. */
  CMPBool focused;            /**< CMP_TRUE when field is focused. */
  CMPBool cursor_visible;     /**< CMP_TRUE when cursor is visible. */
  CMPScalar cursor_timer;     /**< Cursor blink timer in seconds. */
  CMPAnimController label_anim;   /**< Label animation controller. */
  CMPScalar label_value;          /**< Current label animation value (0..1). */
  const char *utf8_label;        /**< UTF-8 label text (may be NULL). */
  cmp_usize label_len;            /**< Label text length in bytes. */
  const char *utf8_placeholder;  /**< UTF-8 placeholder text (may be NULL). */
  cmp_usize placeholder_len;      /**< Placeholder text length in bytes. */
  CMPTextFieldOnChange on_change; /**< Change callback (may be NULL). */
  void *on_change_ctx;           /**< Change callback context pointer. */
} CMPTextField;

/**
 * @brief Initialize a text field style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_style_init(CMPTextFieldStyle *style);

/**
 * @brief Initialize a text field widget.
 * @param field Text field instance.
 * @param backend Text backend instance.
 * @param style Style descriptor.
 * @param allocator Allocator for internal buffers (NULL uses default
 * allocator).
 * @param utf8_text Initial UTF-8 text (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the initial text in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_init(CMPTextField *field,
                                      const CMPTextBackend *backend,
                                      const CMPTextFieldStyle *style,
                                      const CMPAllocator *allocator,
                                      const char *utf8_text, cmp_usize utf8_len);

/**
 * @brief Update the text field content.
 * @param field Text field instance.
 * @param utf8_text UTF-8 text (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the text in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_set_text(CMPTextField *field,
                                          const char *utf8_text,
                                          cmp_usize utf8_len);

/**
 * @brief Retrieve the current text buffer.
 * @param field Text field instance.
 * @param out_utf8 Receives the UTF-8 buffer pointer (may be NULL when empty).
 * @param out_len Receives the text length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_get_text(const CMPTextField *field,
                                          const char **out_utf8,
                                          cmp_usize *out_len);

/**
 * @brief Insert UTF-8 text at the cursor (replacing any selection).
 * @param field Text field instance.
 * @param utf8_text UTF-8 text to insert (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the text in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_insert_utf8(CMPTextField *field,
                                             const char *utf8_text,
                                             cmp_usize utf8_len);

/**
 * @brief Delete the current selection, if any.
 * @param field Text field instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_delete_selection(CMPTextField *field);

/**
 * @brief Delete the previous UTF-8 codepoint (backspace).
 * @param field Text field instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_backspace(CMPTextField *field);

/**
 * @brief Delete the next UTF-8 codepoint (delete forward).
 * @param field Text field instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_delete_forward(CMPTextField *field);

/**
 * @brief Update the text field label.
 * @param field Text field instance.
 * @param utf8_label UTF-8 label text (may be NULL when label_len is 0).
 * @param label_len Label length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_set_label(CMPTextField *field,
                                           const char *utf8_label,
                                           cmp_usize label_len);

/**
 * @brief Update the text field placeholder.
 * @param field Text field instance.
 * @param utf8_placeholder UTF-8 placeholder text (may be NULL when
 * placeholder_len is 0).
 * @param placeholder_len Placeholder length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_set_placeholder(CMPTextField *field,
                                                 const char *utf8_placeholder,
                                                 cmp_usize placeholder_len);

/**
 * @brief Update the text field style.
 * @param field Text field instance.
 * @param style New style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_set_style(CMPTextField *field,
                                           const CMPTextFieldStyle *style);

/**
 * @brief Update the text selection range.
 * @param field Text field instance.
 * @param start Selection start byte offset.
 * @param end Selection end byte offset.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_set_selection(CMPTextField *field,
                                               cmp_usize start, cmp_usize end);

/**
 * @brief Retrieve the current text selection range.
 * @param field Text field instance.
 * @param out_start Receives selection start byte offset.
 * @param out_end Receives selection end byte offset.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_get_selection(const CMPTextField *field,
                                               cmp_usize *out_start,
                                               cmp_usize *out_end);

/**
 * @brief Update the cursor position.
 * @param field Text field instance.
 * @param cursor Cursor byte offset.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_set_cursor(CMPTextField *field,
                                            cmp_usize cursor);

/**
 * @brief Retrieve the cursor position.
 * @param field Text field instance.
 * @param out_cursor Receives cursor byte offset.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_get_cursor(const CMPTextField *field,
                                            cmp_usize *out_cursor);

/**
 * @brief Set the text field focus state.
 * @param field Text field instance.
 * @param focused CMP_TRUE to focus, CMP_FALSE to unfocus.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_set_focus(CMPTextField *field, CMPBool focused);

/**
 * @brief Step text field animations (label and cursor).
 * @param field Text field instance.
 * @param dt Delta time in seconds (>= 0).
 * @param out_changed Receives CMP_TRUE when visual state changed.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_step(CMPTextField *field, CMPScalar dt,
                                      CMPBool *out_changed);

/**
 * @brief Assign a change callback to the text field.
 * @param field Text field instance.
 * @param on_change Callback function (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_text_field_set_on_change(CMPTextField *field,
                                               CMPTextFieldOnChange on_change,
                                               void *ctx);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_TEXT_FIELD_H */
