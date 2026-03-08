#ifndef M3_BUTTON_H
#define M3_BUTTON_H

/**
 * @file m3_button.h
 * \image html button_filled_linux_material.svg "Filled Button"
 * \image html button_outlined_linux_material.svg "Outlined Button"
 * @brief Button widgets for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_icon.h"
#include "cmpc/cmp_text.h"
#include "cmpc/cmp_visuals.h"

/** @brief Filled button variant. */
#define M3_BUTTON_VARIANT_FILLED 1
/** @brief Tonal button variant. */
#define M3_BUTTON_VARIANT_TONAL 2
/** @brief Outlined button variant. */
#define M3_BUTTON_VARIANT_OUTLINED 3
/** @brief Text button variant. */
#define M3_BUTTON_VARIANT_TEXT 4
/** @brief Elevated button variant. */
#define M3_BUTTON_VARIANT_ELEVATED 5
/** @brief Floating action button (FAB) variant. */
#define M3_BUTTON_VARIANT_FAB 6
/** @brief Icon button variant (standard). */
#define M3_BUTTON_VARIANT_ICON_STANDARD 7
/** @brief Icon button variant (filled). */
#define M3_BUTTON_VARIANT_ICON_FILLED 8
/** @brief Icon button variant (tonal). */
#define M3_BUTTON_VARIANT_ICON_TONAL 9
/** @brief Icon button variant (outlined). */
#define M3_BUTTON_VARIANT_ICON_OUTLINED 10
/** @brief Extended FAB variant. */
#define M3_BUTTON_VARIANT_EXTENDED_FAB 11

/** @brief Default horizontal padding for buttons. */
#define M3_BUTTON_DEFAULT_PADDING_X 12.0f
/** @brief Default vertical padding for buttons. */
#define M3_BUTTON_DEFAULT_PADDING_Y 8.0f
/** @brief Default minimum button width. */
#define M3_BUTTON_DEFAULT_MIN_WIDTH 64.0f
/** @brief Default minimum button height. */
#define M3_BUTTON_DEFAULT_MIN_HEIGHT 36.0f
/** @brief Default button corner radius. */
#define M3_BUTTON_DEFAULT_CORNER_RADIUS 4.0f
/** @brief Default outline width for outlined buttons. */
#define M3_BUTTON_DEFAULT_OUTLINE_WIDTH 1.0f
/** @brief Default icon-to-text spacing. */
#define M3_BUTTON_DEFAULT_ICON_SPACING 8.0f
/** @brief Default icon button diameter. */
#define M3_BUTTON_DEFAULT_ICON_DIAMETER 40.0f
/** @brief Default Extended FAB corner radius. */
#define M3_BUTTON_DEFAULT_EXTENDED_FAB_RADIUS 16.0f
/** @brief Default FAB diameter. */
#define M3_BUTTON_DEFAULT_FAB_DIAMETER 56.0f
/** @brief Default ripple expansion duration in seconds. */
#define M3_BUTTON_DEFAULT_RIPPLE_EXPAND 0.2f
/** @brief Default ripple fade duration in seconds. */
#define M3_BUTTON_DEFAULT_RIPPLE_FADE 0.15f

struct M3Button;

/**
 * @brief Button click callback signature.
 * @param ctx User callback context pointer.
 * @param button Button instance that was activated.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPButtonOnClick)(void *ctx, struct M3Button *button);

/**
 * @brief Button style descriptor.
 */
typedef struct M3ButtonStyle {
  cmp_u32 variant; /**< Button visual variant. */
  CMPBool is_rtl;            /**< CMP_TRUE if layout should be right-to-left. */           /**< Button variant (CMP_BUTTON_VARIANT_*). */
  CMPTextStyle text_style;   /**< Label text style. */
  CMPColor background_color; /**< Fill color. */
  CMPColor outline_color;    /**< Outline color. */
  CMPColor ripple_color;     /**< Ripple overlay color. */
  CMPColor disabled_background_color; /**< Fill color when disabled. */
  CMPColor disabled_text_color;       /**< Text color when disabled. */
  CMPColor disabled_outline_color;    /**< Outline color when disabled. */
  CMPScalar outline_width;            /**< Outline width in pixels. */
  CMPScalar corner_radius;            /**< Corner radius in pixels. */
  CMPScalar padding_x;                /**< Horizontal padding in pixels. */
  CMPScalar padding_y;                /**< Vertical padding in pixels. */
  CMPScalar min_width;                /**< Minimum width in pixels. */
  CMPScalar min_height;               /**< Minimum height in pixels. */
  CMPScalar fab_diameter; /**< FAB diameter in pixels (variant FAB). */
  CMPScalar
      ripple_expand_duration;     /**< Ripple expansion duration in seconds. */
  CMPScalar ripple_fade_duration; /**< Ripple fade-out duration in seconds. */
  CMPShadow shadow;        /**< Shadow descriptor for elevated buttons. */
  CMPBool shadow_enabled;  /**< CMP_TRUE when shadow is enabled. */
  CMPIconStyle icon_style; /**< Optional icon style. */
  CMPScalar icon_spacing;  /**< Spacing between icon and text. */
  CMPScalar icon_diameter; /**< Icon button diameter in pixels. */
} M3ButtonStyle;

/**
 * @brief Button widget instance.
 */
typedef struct M3Button {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend text_backend; /**< Text backend for measurements. */
  M3ButtonStyle style;         /**< Current button style. */
  CMPHandle font;              /**< Font handle for label text. */
  CMPTextMetrics metrics;      /**< Cached text metrics. */
  const char *utf8_icon;       /**< UTF-8 icon name (may be NULL). */
  cmp_usize icon_len;          /**< UTF-8 icon name length in bytes. */
  CMPIconMetrics icon_metrics; /**< Cached icon metrics. */
  const char *utf8_label;    /**< UTF-8 label text (may be NULL when empty). */
  cmp_usize utf8_len;        /**< UTF-8 label length in bytes. */
  CMPRect bounds;            /**< Layout bounds. */
  CMPRipple ripple;          /**< Ripple state. */
  CMPBool pressed;           /**< CMP_TRUE when pressed. */
  CMPBool metrics_valid;     /**< CMP_TRUE when text metrics are valid. */
  CMPBool owns_font;         /**< CMP_TRUE when widget owns the font. */
  CMPButtonOnClick on_click; /**< Click callback (may be NULL). */
  void *on_click_ctx;        /**< Click callback context pointer. */
} M3Button;

/**
 * @brief Initialize a filled button style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_button_style_init_filled(M3ButtonStyle *style);

/**
 * @brief Initialize a tonal button style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_button_style_init_tonal(M3ButtonStyle *style);

/**
 * @brief Initialize an outlined button style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_button_style_init_outlined(M3ButtonStyle *style);

/**
 * @brief Initialize a text button style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_button_style_init_text(M3ButtonStyle *style);

/**
 * @brief Initialize an elevated button style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_button_style_init_elevated(M3ButtonStyle *style);

/**
 * @brief Initialize a floating action button (FAB) style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_button_style_init_fab(M3ButtonStyle *style);

/**
 * @brief Initialize an icon button style.
 * @param style Style structure to initialize.
 * @param variant Button variant.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_button_style_init_icon(M3ButtonStyle *style,
                                               cmp_u32 variant);

/**
 * @brief Initialize an extended FAB button style.
 * @param style Style structure to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_button_style_init_extended_fab(M3ButtonStyle *style);

/**
 * @brief Set the icon for a button.
 * @param button The button widget.
 * @param utf8_icon UTF-8 encoded icon string.
 * @param icon_len Length of the icon string.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_button_set_icon(M3Button *button, const char *utf8_icon,
                                        cmp_usize icon_len);

/**
 * @brief Initialize a button widget.
 * @param button Button instance.
 * @param backend Text backend instance.
 * @param style Button style descriptor.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the label in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_button_init(M3Button *button,
                                    const CMPTextBackend *backend,
                                    const M3ButtonStyle *style,
                                    const char *utf8_label, cmp_usize utf8_len);

/**
 * @brief Update the button label.
 * @param button Button instance.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the label in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_button_set_label(M3Button *button,
                                         const char *utf8_label,
                                         cmp_usize utf8_len);

/**
 * @brief Update the button style.
 * @param button Button instance.
 * @param style New button style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_button_set_style(M3Button *button,
                                         const M3ButtonStyle *style);

/**
 * @brief Assign a click callback to the button.
 * @param button Button instance.
 * @param on_click Click callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_button_set_on_click(M3Button *button,
                                            CMPButtonOnClick on_click,
                                            void *ctx);

/**
 * @brief Helper for test coverage.
 * @param fail_point Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_set_fail_point(cmp_u32 fail_point);

/**
 * @brief Helper for test coverage.
 * @param call_count Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_set_color_fail_after(cmp_u32 call_count);

/**
 * @brief Helper for test coverage.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_clear_fail_points(void);

/**
 * @brief Helper for test coverage.
 * @param color Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_validate_color(const CMPColor *color);

/**
 * @brief Helper for test coverage.
 * @param color Parameter for testing.
 * @param r Parameter for testing.
 * @param g Parameter for testing.
 * @param b Parameter for testing.
 * @param a Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_color_set(CMPColor *color, CMPScalar r,
                                              CMPScalar g, CMPScalar b,
                                              CMPScalar a);

/**
 * @brief Helper for test coverage.
 * @param base Parameter for testing.
 * @param alpha Parameter for testing.
 * @param out_color Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_color_with_alpha(const CMPColor *base,
                                                     CMPScalar alpha,
                                                     CMPColor *out_color);

/**
 * @brief Helper for test coverage.
 * @param style Parameter for testing.
 * @param require_family Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_validate_text_style(
    const CMPTextStyle *style, CMPBool require_family);

/**
 * @brief Helper for test coverage.
 * @param style Parameter for testing.
 * @param require_family Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_validate_style(const M3ButtonStyle *style,
                                                   CMPBool require_family);

/**
 * @brief Helper for test coverage.
 * @param spec Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_validate_measure_spec(CMPMeasureSpec spec);

/**
 * @brief Helper for test coverage.
 * @param rect Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_validate_rect(const CMPRect *rect);

CMP_API int CMP_CALL
/**
 * @brief Helper for test coverage.
 * @param backend Parameter for testing.
 * @return Test result.
 */
m3_button_test_validate_backend(const CMPTextBackend *backend);

/**
 * @brief Helper for test coverage.
 * @param button Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_metrics_update(M3Button *button);

/**
 * @brief Helper for test coverage.
 * @param button Parameter for testing.
 * @param out_background Parameter for testing.
 * @param out_text Parameter for testing.
 * @param out_outline Parameter for testing.
 * @param out_ripple Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_resolve_colors(const M3Button *button,
                                                   CMPColor *out_background,
                                                   CMPColor *out_text,
                                                   CMPColor *out_outline,
                                                   CMPColor *out_ripple);

/**
 * @brief Helper for test coverage.
 * @param button Parameter for testing.
 * @param out_corner Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL m3_button_test_resolve_corner(const M3Button *button,
                                                   CMPScalar *out_corner);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_BUTTON_H */
