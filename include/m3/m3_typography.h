#ifndef M3_TYPOGRAPHY_H
#define M3_TYPOGRAPHY_H

/**
 * @file m3_typography.h
 * @brief Material 3 Typography system and standard text styles.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_text.h"

/** @brief Material 3 typography roles. */
typedef enum M3TypographyRole {
  M3_TYPOGRAPHY_DISPLAY_LARGE = 0,
  M3_TYPOGRAPHY_DISPLAY_MEDIUM,
  M3_TYPOGRAPHY_DISPLAY_SMALL,
  M3_TYPOGRAPHY_HEADLINE_LARGE,
  M3_TYPOGRAPHY_HEADLINE_MEDIUM,
  M3_TYPOGRAPHY_HEADLINE_SMALL,
  M3_TYPOGRAPHY_TITLE_LARGE,
  M3_TYPOGRAPHY_TITLE_MEDIUM,
  M3_TYPOGRAPHY_TITLE_SMALL,
  M3_TYPOGRAPHY_LABEL_LARGE,
  M3_TYPOGRAPHY_LABEL_MEDIUM,
  M3_TYPOGRAPHY_LABEL_SMALL,
  M3_TYPOGRAPHY_BODY_LARGE,
  M3_TYPOGRAPHY_BODY_MEDIUM,
  M3_TYPOGRAPHY_BODY_SMALL,
  M3_TYPOGRAPHY_ROLE_COUNT
} M3TypographyRole;

/**
 * @brief Material 3 typography scale holding styles for all roles.
 */
typedef struct M3TypographyScale {
  CMPTextStyle roles[M3_TYPOGRAPHY_ROLE_COUNT]; /**< Text styles by role. */
} M3TypographyScale;

/**
 * @brief Initialize a default Material 3 typography scale (Roboto/system).
 * @param scale Typography scale to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_typography_scale_init(M3TypographyScale *scale);

/**
 * @brief Retrieve a specific text style from a typography scale.
 * @param scale The initialized typography scale.
 * @param role The role to retrieve.
 * @param out_style Pointer to receive the text style.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_typography_get_style(const M3TypographyScale *scale,
                                             M3TypographyRole role,
                                             CMPTextStyle *out_style);

/**
 * @brief Apply a uniform color to all styles in a scale.
 * @param scale Typography scale to update.
 * @param color The text color to apply.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_typography_scale_set_color(M3TypographyScale *scale,
                                                   CMPColor color);

/**
 * @brief Apply a specific font family to all styles in a scale.
 * @param scale Typography scale to update.
 * @param utf8_family The font family name in UTF-8.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_typography_scale_set_family(M3TypographyScale *scale,
                                                    const char *utf8_family);

#ifdef __cplusplus
}
#endif

#endif /* M3_TYPOGRAPHY_H */
