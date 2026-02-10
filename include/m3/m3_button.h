#ifndef M3_BUTTON_H
#define M3_BUTTON_H

/**
 * @file m3_button.h
 * @brief Button widgets for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_text.h"
#include "m3_visuals.h"

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
 * @return M3_OK on success or a failure code.
 */
typedef int (M3_CALL *M3ButtonOnClick)(void *ctx, struct M3Button *button);

/**
 * @brief Button style descriptor.
 */
typedef struct M3ButtonStyle {
    m3_u32 variant; /**< Button variant (M3_BUTTON_VARIANT_*). */
    M3TextStyle text_style; /**< Label text style. */
    M3Color background_color; /**< Fill color. */
    M3Color outline_color; /**< Outline color. */
    M3Color ripple_color; /**< Ripple overlay color. */
    M3Color disabled_background_color; /**< Fill color when disabled. */
    M3Color disabled_text_color; /**< Text color when disabled. */
    M3Color disabled_outline_color; /**< Outline color when disabled. */
    M3Scalar outline_width; /**< Outline width in pixels. */
    M3Scalar corner_radius; /**< Corner radius in pixels. */
    M3Scalar padding_x; /**< Horizontal padding in pixels. */
    M3Scalar padding_y; /**< Vertical padding in pixels. */
    M3Scalar min_width; /**< Minimum width in pixels. */
    M3Scalar min_height; /**< Minimum height in pixels. */
    M3Scalar fab_diameter; /**< FAB diameter in pixels (variant FAB). */
    M3Scalar ripple_expand_duration; /**< Ripple expansion duration in seconds. */
    M3Scalar ripple_fade_duration; /**< Ripple fade-out duration in seconds. */
    M3Shadow shadow; /**< Shadow descriptor for elevated buttons. */
    M3Bool shadow_enabled; /**< M3_TRUE when shadow is enabled. */
} M3ButtonStyle;

/**
 * @brief Button widget instance.
 */
typedef struct M3Button {
    M3Widget widget; /**< Widget interface (points to this instance). */
    M3TextBackend text_backend; /**< Text backend for measurements. */
    M3ButtonStyle style; /**< Current button style. */
    M3Handle font; /**< Font handle for label text. */
    M3TextMetrics metrics; /**< Cached text metrics. */
    const char *utf8_label; /**< UTF-8 label text (may be NULL when empty). */
    m3_usize utf8_len; /**< UTF-8 label length in bytes. */
    M3Rect bounds; /**< Layout bounds. */
    M3Ripple ripple; /**< Ripple state. */
    M3Bool pressed; /**< M3_TRUE when pressed. */
    M3Bool metrics_valid; /**< M3_TRUE when text metrics are valid. */
    M3Bool owns_font; /**< M3_TRUE when widget owns the font. */
    M3ButtonOnClick on_click; /**< Click callback (may be NULL). */
    void *on_click_ctx; /**< Click callback context pointer. */
} M3Button;

/**
 * @brief Initialize a filled button style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_button_style_init_filled(M3ButtonStyle *style);

/**
 * @brief Initialize a tonal button style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_button_style_init_tonal(M3ButtonStyle *style);

/**
 * @brief Initialize an outlined button style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_button_style_init_outlined(M3ButtonStyle *style);

/**
 * @brief Initialize a text button style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_button_style_init_text(M3ButtonStyle *style);

/**
 * @brief Initialize an elevated button style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_button_style_init_elevated(M3ButtonStyle *style);

/**
 * @brief Initialize a floating action button (FAB) style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_button_style_init_fab(M3ButtonStyle *style);

/**
 * @brief Initialize a button widget.
 * @param button Button instance.
 * @param backend Text backend instance.
 * @param style Button style descriptor.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the label in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_button_init(M3Button *button, const M3TextBackend *backend, const M3ButtonStyle *style,
    const char *utf8_label, m3_usize utf8_len);

/**
 * @brief Update the button label.
 * @param button Button instance.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the label in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_button_set_label(M3Button *button, const char *utf8_label, m3_usize utf8_len);

/**
 * @brief Update the button style.
 * @param button Button instance.
 * @param style New button style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_button_set_style(M3Button *button, const M3ButtonStyle *style);

/**
 * @brief Assign a click callback to the button.
 * @param button Button instance.
 * @param on_click Click callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_button_set_on_click(M3Button *button, M3ButtonOnClick on_click, void *ctx);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_BUTTON_H */
