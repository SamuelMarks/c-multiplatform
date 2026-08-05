#ifndef UI_COLOR_SPACE_H
#define UI_COLOR_SPACE_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a 32-bit ARGB color.
 */
typedef ui_uint32 ui_color_t;

/**
 * @brief Constructs an ARGB color from its components.
 */
#define UI_COLOR_ARGB(a, r, g, b)                                              \
  (((ui_color_t)(a) << 24) | ((ui_color_t)(r) << 16) |                         \
   ((ui_color_t)(g) << 8) | (ui_color_t)(b))

/**
 * @brief Extracts the alpha component from an ARGB color.
 */
#define UI_COLOR_ALPHA(c) ((ui_uint8)(((c) >> 24) & 0xFF))

/**
 * @brief Extracts the red component from an ARGB color.
 */
#define UI_COLOR_RED(c) ((ui_uint8)(((c) >> 16) & 0xFF))

/**
 * @brief Extracts the green component from an ARGB color.
 */
#define UI_COLOR_GREEN(c) ((ui_uint8)(((c) >> 8) & 0xFF))

/**
 * @brief Extracts the blue component from an ARGB color.
 */
#define UI_COLOR_BLUE(c) ((ui_uint8)((c) & 0xFF))

/**
 * @brief Represents a color in the CAM16 color space.
 */
struct ui_color_cam16 {
  float hue;    /**< 0.0 to 360.0 */
  float chroma; /**< 0.0 to approx 130.0 */
  float j;      /**< Lightness, 0.0 to 100.0 */
  float q;      /**< Brightness */
  float m;      /**< Colorfulness */
  float s;      /**< Saturation */
};

/**
 * @brief Represents a color in the HCT (Hue, Chroma, Tone) color space.
 */
struct ui_color_hct {
  float hue;    /**< 0.0 to 360.0 */
  float chroma; /**< 0.0 to approx 130.0 */
  float tone;   /**< 0.0 to 100.0 */
};

/**
 * @brief Converts an ARGB color to CAM16.
 *
 * @param argb The input color.
 * @param out_cam16 Pointer to the output CAM16 structure.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
ui_error_t ui_color_argb_to_cam16(ui_color_t argb,
                                  struct ui_color_cam16 *out_cam16);

/**
 * @brief Converts a CAM16 color to ARGB.
 *
 * @param cam16 The input CAM16 color.
 * @param out_argb Pointer to the output ARGB color.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
ui_error_t ui_color_cam16_to_argb(const struct ui_color_cam16 *cam16,
                                  ui_color_t *out_argb);

/**
 * @brief Converts an ARGB color to HCT.
 *
 * @param argb The input color.
 * @param out_hct Pointer to the output HCT structure.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
ui_error_t ui_color_argb_to_hct(ui_color_t argb, struct ui_color_hct *out_hct);

/**
 * @brief Converts an HCT color to ARGB.
 *
 * @param hct The input HCT color.
 * @param out_argb Pointer to the output ARGB color.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
ui_error_t ui_color_hct_to_argb(const struct ui_color_hct *hct,
                                ui_color_t *out_argb);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_COLOR_SPACE_H */
