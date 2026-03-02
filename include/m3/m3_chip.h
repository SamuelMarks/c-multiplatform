#ifndef M3_CHIP_H
#define M3_CHIP_H

/**
 * @file m3_chip.h
 * \image html chip_assist_linux_material.svg "Assist Chip"
 * @brief Chip widgets for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_text.h"
#include "cmpc/cmp_visuals.h"

/** @brief Assist chip variant. */
#define M3_CHIP_VARIANT_ASSIST 1
/** @brief Filter chip variant. */
#define M3_CHIP_VARIANT_FILTER 2
/** @brief Input chip variant. */
#define M3_CHIP_VARIANT_INPUT 3
/** @brief Suggestion chip variant. */
#define M3_CHIP_VARIANT_SUGGESTION 4

/** @brief Default horizontal padding for chips. */
#define M3_CHIP_DEFAULT_PADDING_X 12.0f
/** @brief Default vertical padding for chips. */
#define M3_CHIP_DEFAULT_PADDING_Y 6.0f
/** @brief Default minimum chip width. */
#define M3_CHIP_DEFAULT_MIN_WIDTH 0.0f
/** @brief Default minimum chip height. */
#define M3_CHIP_DEFAULT_MIN_HEIGHT 32.0f
/** @brief Default delete icon size. */
#define M3_CHIP_DEFAULT_ICON_SIZE 18.0f
/** @brief Default gap between text and delete icon. */
#define M3_CHIP_DEFAULT_ICON_GAP 8.0f
/** @brief Default delete icon stroke thickness. */
#define M3_CHIP_DEFAULT_DELETE_THICKNESS 1.5f

/** @brief Dense horizontal padding for chips. */
#define M3_CHIP_DENSE_PADDING_X 8.0f
/** @brief Dense vertical padding for chips. */
#define M3_CHIP_DENSE_PADDING_Y 4.0f
/** @brief Dense minimum chip width. */
#define M3_CHIP_DENSE_MIN_WIDTH 0.0f
/** @brief Dense minimum chip height. */
#define M3_CHIP_DENSE_MIN_HEIGHT 24.0f
/** @brief Dense delete icon size. */
#define M3_CHIP_DENSE_ICON_SIZE 16.0f
/** @brief Dense gap between text and delete icon. */
#define M3_CHIP_DENSE_ICON_GAP 6.0f
/** @brief Dense delete icon stroke thickness. */
#define M3_CHIP_DENSE_DELETE_THICKNESS 1.0f

/** @brief Default chip corner radius. */
#define M3_CHIP_DEFAULT_CORNER_RADIUS 8.0f
/** @brief Default chip outline width. */
#define M3_CHIP_DEFAULT_OUTLINE_WIDTH 1.0f
/** @brief Default ripple expansion duration in seconds. */
#define M3_CHIP_DEFAULT_RIPPLE_EXPAND 0.2f
/** @brief Default ripple fade duration in seconds. */
#define M3_CHIP_DEFAULT_RIPPLE_FADE 0.15f

struct M3Chip;

/**
 * @brief Chip click callback signature.
 * @param ctx User callback context pointer.
 * @param chip Chip instance that was activated.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPChipOnClick)(void *ctx, struct M3Chip *chip);

/**
 * @brief Chip delete callback signature.
 * @param ctx User callback context pointer.
 * @param chip Chip instance whose delete icon was activated.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPChipOnDelete)(void *ctx, struct M3Chip *chip);

/**
 * @brief Chip layout metrics.
 */
typedef struct M3ChipLayout {
  CMPScalar padding_x;             /**< Horizontal padding in pixels (>= 0). */
  CMPScalar padding_y;             /**< Vertical padding in pixels (>= 0). */
  CMPScalar min_width;             /**< Minimum chip width in pixels (>= 0). */
  CMPScalar min_height;            /**< Minimum chip height in pixels (>= 0). */
  CMPScalar icon_size;             /**< Trailing delete icon size in pixels
                                     (>= 0). */
  CMPScalar icon_gap;              /**< Gap between text and icon in pixels
                                     (>= 0). */
  CMPScalar delete_icon_thickness; /**< Delete icon stroke thickness in pixels
                                     (>= 0). */
} M3ChipLayout;

/**
 * @brief Chip style descriptor.
 */
typedef struct M3ChipStyle {
  cmp_u32 variant;                    /**< Chip variant (CMP_CHIP_VARIANT_*). */
  CMPTextStyle text_style;            /**< Label text style. */
  CMPColor background_color;          /**< Background fill color. */
  CMPColor outline_color;             /**< Outline color. */
  CMPColor selected_background_color; /**< Background color when selected. */
  CMPColor selected_outline_color;    /**< Outline color when selected. */
  CMPColor selected_text_color;       /**< Text color when selected. */
  CMPColor disabled_background_color; /**< Background color when disabled. */
  CMPColor disabled_outline_color;    /**< Outline color when disabled. */
  CMPColor disabled_text_color;       /**< Text color when disabled. */
  CMPColor ripple_color;              /**< Ripple overlay color. */
  CMPScalar outline_width;            /**< Outline width in pixels (>= 0). */
  CMPScalar corner_radius;            /**< Corner radius in pixels (>= 0). */
  CMPScalar ripple_expand_duration;   /**< Ripple expansion duration in seconds
                                        (>= 0). */
  CMPScalar ripple_fade_duration;     /**< Ripple fade duration in seconds
                                        (>= 0). */
  M3ChipLayout layout;                /**< Default layout metrics. */
  M3ChipLayout dense_layout;          /**< Dense layout metrics. */
} M3ChipStyle;

/**
 * @brief Chip widget instance.
 */
typedef struct M3Chip {
  CMPWidget widget; /**< Widget interface (points to this instance). */
  CMPTextBackend text_backend; /**< Text backend for measurements. */
  M3ChipStyle style;           /**< Current chip style. */
  CMPHandle font;              /**< Font handle for label text. */
  CMPTextMetrics metrics;      /**< Cached text metrics. */
  const char *utf8_label;      /**< UTF-8 label text (may be NULL). */
  cmp_usize utf8_len;          /**< UTF-8 label length in bytes. */
  CMPRect bounds;              /**< Layout bounds. */
  CMPRipple ripple;            /**< Ripple state. */
  CMPBool pressed;             /**< CMP_TRUE when pressed. */
  CMPBool pressed_delete;      /**< CMP_TRUE when delete icon is pressed. */
  CMPBool metrics_valid;       /**< CMP_TRUE when text metrics are valid. */
  CMPBool owns_font;           /**< CMP_TRUE when widget owns the font. */
  CMPBool selected;            /**< CMP_TRUE when selected. */
  CMPBool dense;               /**< CMP_TRUE when using dense layout. */
  CMPBool show_delete;         /**< CMP_TRUE when delete icon is visible. */
  CMPChipOnClick on_click;     /**< Click callback (may be NULL). */
  void *on_click_ctx;          /**< Click callback context pointer. */
  CMPChipOnDelete on_delete;   /**< Delete callback (may be NULL). */
  void *on_delete_ctx;         /**< Delete callback context pointer. */
} M3Chip;

/**
 * @brief Initialize an assist chip style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_style_init_assist(M3ChipStyle *style);

/**
 * @brief Initialize a filter chip style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_style_init_filter(M3ChipStyle *style);

/**
 * @brief Initialize an input chip style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_style_init_input(M3ChipStyle *style);

/**
 * @brief Initialize a suggestion chip style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_style_init_suggestion(M3ChipStyle *style);

/**
 * @brief Initialize a chip widget.
 * @param chip Chip instance.
 * @param backend Text backend instance.
 * @param style Chip style descriptor.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the label in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_init(M3Chip *chip, const CMPTextBackend *backend,
                                  const M3ChipStyle *style,
                                  const char *utf8_label, cmp_usize utf8_len);

/**
 * @brief Update the chip label.
 * @param chip Chip instance.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Length of the label in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_set_label(M3Chip *chip, const char *utf8_label,
                                       cmp_usize utf8_len);

/**
 * @brief Update the chip style.
 * @param chip Chip instance.
 * @param style New chip style descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_set_style(M3Chip *chip, const M3ChipStyle *style);

/**
 * @brief Update the chip selected state.
 * @param chip Chip instance.
 * @param selected CMP_TRUE when selected.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_set_selected(M3Chip *chip, CMPBool selected);

/**
 * @brief Retrieve the chip selected state.
 * @param chip Chip instance.
 * @param out_selected Receives the selected state.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_get_selected(const M3Chip *chip,
                                          CMPBool *out_selected);

/**
 * @brief Enable or disable the dense layout.
 * @param chip Chip instance.
 * @param dense CMP_TRUE to enable dense layout.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_set_dense(M3Chip *chip, CMPBool dense);

/**
 * @brief Retrieve whether dense layout is enabled.
 * @param chip Chip instance.
 * @param out_dense Receives the dense layout state.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_get_dense(const M3Chip *chip, CMPBool *out_dense);

/**
 * @brief Show or hide the trailing delete icon.
 * @param chip Chip instance.
 * @param show_delete CMP_TRUE to show the delete icon.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_set_show_delete(M3Chip *chip, CMPBool show_delete);

/**
 * @brief Retrieve whether the trailing delete icon is visible.
 * @param chip Chip instance.
 * @param out_show_delete Receives the delete icon visibility state.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_get_show_delete(const M3Chip *chip,
                                             CMPBool *out_show_delete);

/**
 * @brief Assign a click callback to the chip.
 * @param chip Chip instance.
 * @param on_click Click callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_set_on_click(M3Chip *chip, CMPChipOnClick on_click,
                                          void *ctx);

/**
 * @brief Assign a delete callback to the chip.
 * @param chip Chip instance.
 * @param on_delete Delete callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_chip_set_on_delete(M3Chip *chip,
                                           CMPChipOnDelete on_delete,
                                           void *ctx);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_CHIP_H */
