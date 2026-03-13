#ifndef CUPERTINO_TYPOGRAPHY_H
#define CUPERTINO_TYPOGRAPHY_H

/**
 * @file cupertino_typography.h
 * @brief Apple Cupertino Typography system and Dynamic Type text styles.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_text.h"
/* clang-format on */

/** @brief Cupertino/HIG typography text styles. */
typedef enum CupertinoTypographyStyle {
  CUPERTINO_TYPOGRAPHY_LARGE_TITLE = 0,
  CUPERTINO_TYPOGRAPHY_TITLE_1,
  CUPERTINO_TYPOGRAPHY_TITLE_2,
  CUPERTINO_TYPOGRAPHY_TITLE_3,
  CUPERTINO_TYPOGRAPHY_HEADLINE,
  CUPERTINO_TYPOGRAPHY_BODY,
  CUPERTINO_TYPOGRAPHY_CALLOUT,
  CUPERTINO_TYPOGRAPHY_SUBHEADLINE,
  CUPERTINO_TYPOGRAPHY_FOOTNOTE,
  CUPERTINO_TYPOGRAPHY_CAPTION_1,
  CUPERTINO_TYPOGRAPHY_CAPTION_2,
  CUPERTINO_TYPOGRAPHY_STYLE_COUNT
} CupertinoTypographyStyle;

/**
 * @brief Cupertino typography scale holding styles for all text sizes.
 */
typedef struct CupertinoTypographyScale {
  CMPTextStyle styles[CUPERTINO_TYPOGRAPHY_STYLE_COUNT]; /**< Text styles by
                                                            hierarchy. */
} CupertinoTypographyScale;

/**
 * @brief Initialize a default Cupertino typography scale (San Francisco system
 * font).
 * @param scale Typography scale to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cupertino_typography_scale_init(CupertinoTypographyScale *scale);

/**
 * @brief Retrieve a specific text style from a typography scale.
 * @param scale The initialized typography scale.
 * @param style_id The style to retrieve.
 * @param out_style Pointer to receive the text style.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cupertino_typography_get_style(
    const CupertinoTypographyScale *scale, CupertinoTypographyStyle style_id,
    CMPTextStyle *out_style);

/**
 * @brief Apply a uniform color to all styles in a scale.
 * @param scale Typography scale to update.
 * @param color The text color to apply.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cupertino_typography_scale_set_color(
    CupertinoTypographyScale *scale, CMPColor color);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_TYPOGRAPHY_H */
