#ifndef F2_BUTTON_H
#define F2_BUTTON_H

/**
 * @file f2_button.h
 * @brief Microsoft Fluent 2 button widgets.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_icon.h"
#include "cmpc/cmp_text.h"
#include "cmpc/cmp_visuals.h"

/** @brief Primary button variant. */
#define F2_BUTTON_VARIANT_PRIMARY 1
/** @brief Standard button variant. */
#define F2_BUTTON_VARIANT_STANDARD 2
/** @brief Subtle button variant. */
#define F2_BUTTON_VARIANT_SUBTLE 3
/** @brief Outline button variant. */
#define F2_BUTTON_VARIANT_OUTLINE 4
/** @brief Transparent button variant. */
#define F2_BUTTON_VARIANT_TRANSPARENT 5

/** @brief Default corner radii. */
#define F2_BUTTON_DEFAULT_CORNER_RADIUS 4.0f
/** @brief Minimum button width. */
#define F2_BUTTON_DEFAULT_MIN_WIDTH 64.0f
/** @brief Minimum button height. */
#define F2_BUTTON_DEFAULT_MIN_HEIGHT 32.0f
/** @brief Horizontal padding. */
#define F2_BUTTON_DEFAULT_PADDING_X 12.0f
/** @brief Vertical padding. */
#define F2_BUTTON_DEFAULT_PADDING_Y 5.0f

struct F2Button;

/**
 * @brief Button click callback signature.
 * @param ctx User callback context pointer.
 * @param button Button instance that was activated.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *F2ButtonOnClick)(void *ctx, struct F2Button *button);

/**
 * @brief Fluent 2 Button style descriptor.
 */
typedef struct F2ButtonStyle {
  cmp_u32 variant; /**< Button visual variant. */
  CMPBool is_rtl;  /**< CMP_TRUE if layout should be right-to-left. */
  CMPTextStyle text_style;   /**< Label text style. */
  CMPColor background_color; /**< Fill color. */
  CMPColor background_hover_color; /**< Hover fill color. */
  CMPColor background_pressed_color; /**< Pressed fill color. */
  CMPColor outline_color;    /**< Outline color. */
  CMPColor disabled_background_color; /**< Fill color when disabled. */
  CMPColor disabled_text_color;       /**< Text color when disabled. */
  CMPColor disabled_outline_color;    /**< Outline color when disabled. */
  CMPScalar outline_width;            /**< Outline width in pixels. */
  CMPScalar corner_radius;            /**< Corner radius in pixels. */
  CMPScalar padding_x;                /**< Horizontal padding in pixels. */
  CMPScalar padding_y;                /**< Vertical padding in pixels. */
  CMPScalar min_width;                /**< Minimum width in pixels. */
  CMPScalar min_height;               /**< Minimum height in pixels. */
  CMPShadow shadow;        /**< Shadow descriptor for buttons. */
  CMPBool shadow_enabled;  /**< CMP_TRUE when shadow is enabled. */
  CMPIconStyle icon_style; /**< Optional icon style. */
  CMPScalar icon_spacing;  /**< Spacing between icon and text. */
  CMPScalar icon_diameter; /**< Icon button diameter in pixels. */
} F2ButtonStyle;

/**
 * @brief Fluent 2 Button widget instance.
 */
typedef struct F2Button {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend text_backend; /**< Text backend for measurements. */
  F2ButtonStyle style;         /**< Current button style. */
  CMPHandle font;              /**< Font handle for label text. */
  CMPTextMetrics metrics;      /**< Cached text metrics. */
  const char *utf8_icon;       /**< UTF-8 icon name (may be NULL). */
  cmp_usize icon_len;          /**< UTF-8 icon name length in bytes. */
  CMPIconMetrics icon_metrics; /**< Cached icon metrics. */
  const char *utf8_label;    /**< UTF-8 label text (may be NULL when empty). */
  cmp_usize utf8_len;        /**< UTF-8 label length in bytes. */
  CMPRect bounds;            /**< Layout bounds. */
  CMPBool hovered;           /**< CMP_TRUE when hovered. */
  CMPBool pressed;           /**< CMP_TRUE when pressed. */
  CMPBool metrics_valid;     /**< CMP_TRUE when text metrics are valid. */
  CMPBool owns_font;         /**< CMP_TRUE when widget owns the font. */
  F2ButtonOnClick on_click;  /**< Click callback (may be NULL). */
  void *on_click_ctx;        /**< Click callback context pointer. */
} F2Button;

/**
 * @brief Initialize a Primary button style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_button_style_init_primary(F2ButtonStyle *style);

/**
 * @brief Initialize a Standard button style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_button_style_init_standard(F2ButtonStyle *style);

/**
 * @brief Initialize an Outline button style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_button_style_init_outline(F2ButtonStyle *style);

/**
 * @brief Initialize a Subtle button style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_button_style_init_subtle(F2ButtonStyle *style);

/**
 * @brief Initialize a Transparent button style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_button_style_init_transparent(F2ButtonStyle *style);

/**
 * @brief Initialize a button widget.
 * @param button Button instance.
 * @param backend Text backend instance.
 * @param style Button style descriptor.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the label in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_button_init(F2Button *button,
                                    const CMPTextBackend *backend,
                                    const F2ButtonStyle *style,
                                    const char *utf8_label, cmp_usize utf8_len);

/**
 * @brief Set the icon for a button.
 * @param button The button widget.
 * @param utf8_icon UTF-8 encoded icon string.
 * @param icon_len Length of the icon string.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_button_set_icon(F2Button *button, const char *utf8_icon,
                                        cmp_usize icon_len);

/**
 * @brief Update the button label.
 * @param button Button instance.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the label in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_button_set_label(F2Button *button,
                                         const char *utf8_label,
                                         cmp_usize utf8_len);

/**
 * @brief Assign a click callback to the button.
 * @param button Button instance.
 * @param on_click Click callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_button_set_on_click(F2Button *button,
                                            F2ButtonOnClick on_click,
                                            void *ctx);

/**
 * @brief Fluent 2 Toggle Button widget instance.
 */
typedef struct F2ToggleButton {
  CMPWidget widget;         /**< Widget interface. */
  F2Button core;            /**< Core button structure. */
  CMPBool is_toggled;       /**< CMP_TRUE when toggled on. */
  CMPColor toggled_bg_color;/**< Background color when toggled. */
  CMPColor toggled_text_color; /**< Text color when toggled. */
} F2ToggleButton;

/**
 * @brief Initialize a Fluent 2 Toggle Button.
 * @param toggle_button Toggle Button instance.
 * @param backend Text backend instance.
 * @param style Button style descriptor.
 * @param utf8_label UTF-8 label text.
 * @param utf8_len Length of the label.
 * @param is_toggled Initial toggle state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_toggle_button_init(F2ToggleButton *toggle_button,
                                           const CMPTextBackend *backend,
                                           const F2ButtonStyle *style,
                                           const char *utf8_label,
                                           cmp_usize utf8_len,
                                           CMPBool is_toggled);

/**
 * @brief Set the toggle state.
 * @param toggle_button Toggle Button instance.
 * @param is_toggled New toggle state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_toggle_button_set_toggled(F2ToggleButton *toggle_button, CMPBool is_toggled);

/**
 * @brief Fluent 2 Split Button widget instance.
 */
typedef struct F2SplitButton {
  CMPWidget widget;         /**< Widget interface. */
  F2Button primary_action;  /**< Main action button half. */
  F2Button dropdown_action; /**< Chevron dropdown button half. */
  CMPBool dropdown_open;    /**< True when dropdown flyout is active. */
} F2SplitButton;

/**
 * @brief Initialize a Fluent 2 Split Button.
 * @param split_button Split Button instance.
 * @param backend Text backend instance.
 * @param style Button style descriptor.
 * @param utf8_label UTF-8 label text.
 * @param utf8_len Length of the label.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_split_button_init(F2SplitButton *split_button,
                                          const CMPTextBackend *backend,
                                          const F2ButtonStyle *style,
                                          const char *utf8_label,
                                          cmp_usize utf8_len);

/**
 * @brief Set the dropdown state of the split button.
 * @param split_button Split Button instance.
 * @param is_open True to mark dropdown as open.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_split_button_set_dropdown_open(F2SplitButton *split_button, CMPBool is_open);

#ifdef __cplusplus
}
#endif

#endif /* F2_BUTTON_H */
