#ifndef F2_TYPOGRAPHY_H
#define F2_TYPOGRAPHY_H

/**
 * @file f2_typography.h
 * @brief Microsoft Fluent 2 typography system.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_text.h"
/* clang-format on */

/** @brief Fluent 2 typography roles. */
typedef enum F2TypographyRole {
  F2_TYPOGRAPHY_CAPTION = 0,
  F2_TYPOGRAPHY_BODY,
  F2_TYPOGRAPHY_BODY_STRONG,
  F2_TYPOGRAPHY_SUBTITLE,
  F2_TYPOGRAPHY_TITLE,
  F2_TYPOGRAPHY_TITLE_LARGE,
  F2_TYPOGRAPHY_DISPLAY,
  F2_TYPOGRAPHY_DISPLAY_LARGE,
  F2_TYPOGRAPHY_DISPLAY_EXTRA_LARGE,
  F2_TYPOGRAPHY_ROLE_COUNT
} F2TypographyRole;

/**
 * @brief Fluent 2 typography scale holding styles for all roles.
 */
typedef struct F2TypographyScale {
  CMPTextStyle roles[F2_TYPOGRAPHY_ROLE_COUNT]; /**< Text styles by role. */
} F2TypographyScale;

/**
 * @brief Initialize a default Fluent 2 typography scale.
 * @param scale Typography scale to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_typography_scale_init(F2TypographyScale *scale);

/**
 * @brief Retrieve a specific text style from a typography scale.
 * @param scale The initialized typography scale.
 * @param role The role to retrieve.
 * @param out_style Pointer to receive the text style.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_typography_get_style(const F2TypographyScale *scale,
                                             F2TypographyRole role,
                                             CMPTextStyle *out_style);

/**
 * @brief Apply a uniform color to all styles in a scale.
 * @param scale Typography scale to update.
 * @param color The text color to apply.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_typography_scale_set_color(F2TypographyScale *scale,
                                                   CMPColor color);

/**
 * @brief Apply a specific font family to all styles in a scale.
 * @param scale Typography scale to update.
 * @param utf8_family The font family name in UTF-8.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_typography_scale_set_family(F2TypographyScale *scale,
                                                    const char *utf8_family);

#ifdef __cplusplus
}
#endif

#endif /* F2_TYPOGRAPHY_H */
