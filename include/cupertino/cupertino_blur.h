#ifndef CUPERTINO_BLUR_H
#define CUPERTINO_BLUR_H

/**
 * @file cupertino_blur.h
 * @brief Apple Cupertino Vibrancy and Blur (UIBlurEffectStyle) mappings.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_visuals.h"

/** @brief iOS UIBlurEffectStyle equivalents */
typedef enum CupertinoBlurStyle {
  CUPERTINO_BLUR_STYLE_ULTRA_THIN = 0,
  CUPERTINO_BLUR_STYLE_THIN,
  CUPERTINO_BLUR_STYLE_NORMAL,
  CUPERTINO_BLUR_STYLE_THICK,
  CUPERTINO_BLUR_STYLE_CHROME
} CupertinoBlurStyle;

/**
 * @brief Represents a multi-layered background blur configuration.
 */
typedef struct CupertinoBlurEffect {
  CMPScalar blur_radius; /**< Base gaussian blur radius in pixels. */
  CMPColor tint_color;   /**< Overlay tint color applied above the blur. */
  CMPColor base_color; /**< Base solid color underneath the blur (for fallbacks
                          or reduced transparency). */
} CupertinoBlurEffect;

/**
 * @brief Retrieves the blur configuration for a specific style, adapting to
 * light/dark mode.
 * @param style_id The blur style to map.
 * @param is_dark CMP_TRUE if dark mode is active, CMP_FALSE otherwise.
 * @param out_effect Pointer to receive the blur configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cupertino_blur_get_effect(CupertinoBlurStyle style_id,
                                               CMPBool is_dark,
                                               CupertinoBlurEffect *out_effect);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_BLUR_H */
