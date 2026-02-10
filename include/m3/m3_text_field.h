#ifndef M3_TEXT_FIELD_H
#define M3_TEXT_FIELD_H

/**
 * @file m3_text_field.h
 * @brief Text field widget and editing helpers for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_anim.h"
#include "m3_text.h"

/** @brief Default horizontal padding for text fields. */
#define M3_TEXT_FIELD_DEFAULT_PADDING_X 12.0f
/** @brief Default vertical padding for text fields. */
#define M3_TEXT_FIELD_DEFAULT_PADDING_Y 8.0f
/** @brief Default minimum text field height. */
#define M3_TEXT_FIELD_DEFAULT_MIN_HEIGHT 48.0f
/** @brief Default corner radius for text fields. */
#define M3_TEXT_FIELD_DEFAULT_CORNER_RADIUS 4.0f
/** @brief Default outline width for text fields. */
#define M3_TEXT_FIELD_DEFAULT_OUTLINE_WIDTH 1.0f
/** @brief Default floating label offset from the top edge. */
#define M3_TEXT_FIELD_DEFAULT_LABEL_FLOAT_OFFSET 6.0f
/** @brief Default label animation duration in seconds. */
#define M3_TEXT_FIELD_DEFAULT_LABEL_ANIM_DURATION 0.15f
/** @brief Default cursor width in pixels. */
#define M3_TEXT_FIELD_DEFAULT_CURSOR_WIDTH 1.0f
/** @brief Default cursor blink period in seconds. */
#define M3_TEXT_FIELD_DEFAULT_CURSOR_BLINK 1.0f
/** @brief Default selection handle radius in pixels. */
#define M3_TEXT_FIELD_DEFAULT_HANDLE_RADIUS 4.0f
/** @brief Default selection handle height in pixels. */
#define M3_TEXT_FIELD_DEFAULT_HANDLE_HEIGHT 12.0f

struct M3TextField;

/**
 * @brief Text field change callback signature.
 * @param ctx User callback context pointer.
 * @param field Text field instance that changed.
 * @param utf8 UTF-8 text buffer (may be NULL when empty).
 * @param utf8_len Length of the text buffer in bytes.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3TextFieldOnChange)(void *ctx, struct M3TextField *field,
                                          const char *utf8, m3_usize utf8_len);

/**
 * @brief Text field style descriptor.
 */
typedef struct M3TextFieldStyle {
  M3TextStyle text_style;             /**< Text style for input content. */
  M3TextStyle label_style;            /**< Text style for the floating label. */
  M3Color container_color;            /**< Container fill color. */
  M3Color outline_color;              /**< Outline color (idle). */
  M3Color focused_outline_color;      /**< Outline color when focused. */
  M3Color disabled_container_color;   /**< Container color when disabled. */
  M3Color disabled_outline_color;     /**< Outline color when disabled. */
  M3Color disabled_text_color;        /**< Text color when disabled. */
  M3Color disabled_label_color;       /**< Label color when disabled. */
  M3Color placeholder_color;          /**< Placeholder text color. */
  M3Color disabled_placeholder_color; /**< Placeholder color when disabled. */
  M3Color cursor_color;               /**< Cursor color. */
  M3Color selection_color;            /**< Selection highlight color. */
  M3Color handle_color;               /**< Selection handle color. */
  M3Scalar outline_width;             /**< Outline width in pixels (>= 0). */
  M3Scalar corner_radius;             /**< Corner radius in pixels (>= 0). */
  M3Scalar padding_x;          /**< Horizontal padding in pixels (>= 0). */
  M3Scalar padding_y;          /**< Vertical padding in pixels (>= 0). */
  M3Scalar min_height;         /**< Minimum height in pixels (>= 0). */
  M3Scalar label_float_offset; /**< Label floating offset from top (>= 0). */
  M3Scalar
      label_anim_duration; /**< Label animation duration in seconds (>= 0). */
  M3Scalar cursor_width;   /**< Cursor width in pixels (>= 0). */
  M3Scalar cursor_blink_period; /**< Cursor blink period in seconds (>= 0). */
  M3Scalar handle_radius; /**< Selection handle radius in pixels (>= 0). */
  M3Scalar handle_height; /**< Selection handle height in pixels (>= 0). */
} M3TextFieldStyle;

/**
 * @brief Text field widget instance.
 */
typedef struct M3TextField {
  M3Widget widget; /**< Widget interface (points to this instance). */
  M3TextBackend
      text_backend;       /**< Text backend for measurements and rendering. */
  M3TextFieldStyle style; /**< Current text field style. */
  M3Allocator allocator;  /**< Allocator for internal text buffers. */
  M3Handle text_font;     /**< Font handle for input text. */
  M3Handle label_font;    /**< Font handle for labels. */
  M3TextMetrics text_metrics;        /**< Cached text metrics. */
  M3TextMetrics label_metrics;       /**< Cached label metrics. */
  M3TextMetrics placeholder_metrics; /**< Cached placeholder metrics. */
  M3TextMetrics text_font_metrics;   /**< Cached font metrics for text. */
  M3TextMetrics label_font_metrics;  /**< Cached font metrics for label. */
  M3Rect bounds;                     /**< Layout bounds. */
  char *utf8;               /**< Owned UTF-8 text buffer (null-terminated). */
  m3_usize utf8_len;        /**< UTF-8 length in bytes. */
  m3_usize utf8_capacity;   /**< UTF-8 buffer capacity in bytes. */
  m3_usize cursor;          /**< Cursor byte offset in UTF-8 buffer. */
  m3_usize selection_start; /**< Selection start byte offset. */
  m3_usize selection_end;   /**< Selection end byte offset. */
  M3Bool selecting; /**< M3_TRUE when pointer selection drag is active. */
  M3Bool text_metrics_valid; /**< M3_TRUE when cached text metrics are valid. */
  M3Bool
      label_metrics_valid; /**< M3_TRUE when cached label metrics are valid. */
  M3Bool placeholder_metrics_valid; /**< M3_TRUE when cached placeholder metrics
                                       are valid. */
  M3Bool font_metrics_valid; /**< M3_TRUE when cached font metrics are valid. */
  M3Bool owns_fonts;         /**< M3_TRUE when widget owns font handles. */
  M3Bool focused;            /**< M3_TRUE when field is focused. */
  M3Bool cursor_visible;     /**< M3_TRUE when cursor is visible. */
  M3Scalar cursor_timer;     /**< Cursor blink timer in seconds. */
  M3AnimController label_anim;   /**< Label animation controller. */
  M3Scalar label_value;          /**< Current label animation value (0..1). */
  const char *utf8_label;        /**< UTF-8 label text (may be NULL). */
  m3_usize label_len;            /**< Label text length in bytes. */
  const char *utf8_placeholder;  /**< UTF-8 placeholder text (may be NULL). */
  m3_usize placeholder_len;      /**< Placeholder text length in bytes. */
  M3TextFieldOnChange on_change; /**< Change callback (may be NULL). */
  void *on_change_ctx;           /**< Change callback context pointer. */
} M3TextField;

/**
 * @brief Initialize a text field style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_style_init(M3TextFieldStyle *style);

/**
 * @brief Initialize a text field widget.
 * @param field Text field instance.
 * @param backend Text backend instance.
 * @param style Style descriptor.
 * @param allocator Allocator for internal buffers (NULL uses default
 * allocator).
 * @param utf8_text Initial UTF-8 text (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the initial text in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_init(M3TextField *field,
                                      const M3TextBackend *backend,
                                      const M3TextFieldStyle *style,
                                      const M3Allocator *allocator,
                                      const char *utf8_text, m3_usize utf8_len);

/**
 * @brief Update the text field content.
 * @param field Text field instance.
 * @param utf8_text UTF-8 text (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the text in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_set_text(M3TextField *field,
                                          const char *utf8_text,
                                          m3_usize utf8_len);

/**
 * @brief Retrieve the current text buffer.
 * @param field Text field instance.
 * @param out_utf8 Receives the UTF-8 buffer pointer (may be NULL when empty).
 * @param out_len Receives the text length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_get_text(const M3TextField *field,
                                          const char **out_utf8,
                                          m3_usize *out_len);

/**
 * @brief Insert UTF-8 text at the cursor (replacing any selection).
 * @param field Text field instance.
 * @param utf8_text UTF-8 text to insert (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the text in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_insert_utf8(M3TextField *field,
                                             const char *utf8_text,
                                             m3_usize utf8_len);

/**
 * @brief Delete the current selection, if any.
 * @param field Text field instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_delete_selection(M3TextField *field);

/**
 * @brief Delete the previous UTF-8 codepoint (backspace).
 * @param field Text field instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_backspace(M3TextField *field);

/**
 * @brief Delete the next UTF-8 codepoint (delete forward).
 * @param field Text field instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_delete_forward(M3TextField *field);

/**
 * @brief Update the text field label.
 * @param field Text field instance.
 * @param utf8_label UTF-8 label text (may be NULL when label_len is 0).
 * @param label_len Label length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_set_label(M3TextField *field,
                                           const char *utf8_label,
                                           m3_usize label_len);

/**
 * @brief Update the text field placeholder.
 * @param field Text field instance.
 * @param utf8_placeholder UTF-8 placeholder text (may be NULL when
 * placeholder_len is 0).
 * @param placeholder_len Placeholder length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_set_placeholder(M3TextField *field,
                                                 const char *utf8_placeholder,
                                                 m3_usize placeholder_len);

/**
 * @brief Update the text field style.
 * @param field Text field instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_set_style(M3TextField *field,
                                           const M3TextFieldStyle *style);

/**
 * @brief Update the text selection range.
 * @param field Text field instance.
 * @param start Selection start byte offset.
 * @param end Selection end byte offset.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_set_selection(M3TextField *field,
                                               m3_usize start, m3_usize end);

/**
 * @brief Retrieve the current text selection range.
 * @param field Text field instance.
 * @param out_start Receives selection start byte offset.
 * @param out_end Receives selection end byte offset.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_get_selection(const M3TextField *field,
                                               m3_usize *out_start,
                                               m3_usize *out_end);

/**
 * @brief Update the cursor position.
 * @param field Text field instance.
 * @param cursor Cursor byte offset.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_set_cursor(M3TextField *field,
                                            m3_usize cursor);

/**
 * @brief Retrieve the cursor position.
 * @param field Text field instance.
 * @param out_cursor Receives cursor byte offset.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_get_cursor(const M3TextField *field,
                                            m3_usize *out_cursor);

/**
 * @brief Set the text field focus state.
 * @param field Text field instance.
 * @param focused M3_TRUE to focus, M3_FALSE to unfocus.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_set_focus(M3TextField *field, M3Bool focused);

/**
 * @brief Step text field animations (label and cursor).
 * @param field Text field instance.
 * @param dt Delta time in seconds (>= 0).
 * @param out_changed Receives M3_TRUE when visual state changed.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_step(M3TextField *field, M3Scalar dt,
                                      M3Bool *out_changed);

/**
 * @brief Assign a change callback to the text field.
 * @param field Text field instance.
 * @param on_change Callback function (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_text_field_set_on_change(M3TextField *field,
                                               M3TextFieldOnChange on_change,
                                               void *ctx);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_TEXT_FIELD_H */
