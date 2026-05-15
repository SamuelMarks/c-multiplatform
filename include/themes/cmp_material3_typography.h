/* clang-format off */
#ifndef CMP_MATERIAL3_TYPOGRAPHY_H
#define CMP_MATERIAL3_TYPOGRAPHY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Material 3 Typography scale categories.
 */
typedef enum {
  CMP_M3_TYPESCALE_DISPLAY_LARGE,   /**< Display Large */
  CMP_M3_TYPESCALE_DISPLAY_MEDIUM,  /**< Display Medium */
  CMP_M3_TYPESCALE_DISPLAY_SMALL,   /**< Display Small */
  CMP_M3_TYPESCALE_HEADLINE_LARGE,  /**< Headline Large */
  CMP_M3_TYPESCALE_HEADLINE_MEDIUM, /**< Headline Medium */
  CMP_M3_TYPESCALE_HEADLINE_SMALL,  /**< Headline Small */
  CMP_M3_TYPESCALE_TITLE_LARGE,     /**< Title Large */
  CMP_M3_TYPESCALE_TITLE_MEDIUM,    /**< Title Medium */
  CMP_M3_TYPESCALE_TITLE_SMALL,     /**< Title Small */
  CMP_M3_TYPESCALE_BODY_LARGE,      /**< Body Large */
  CMP_M3_TYPESCALE_BODY_MEDIUM,     /**< Body Medium */
  CMP_M3_TYPESCALE_BODY_SMALL,      /**< Body Small */
  CMP_M3_TYPESCALE_LABEL_LARGE,     /**< Label Large */
  CMP_M3_TYPESCALE_LABEL_MEDIUM,    /**< Label Medium */
  CMP_M3_TYPESCALE_LABEL_SMALL      /**< Label Small */
} cmp_m3_typescale_category_t;

/**
 * @brief Material 3 Font weight mappings.
 */
typedef enum {
  CMP_M3_FONT_WEIGHT_REGULAR, /**< Regular weight */
  CMP_M3_FONT_WEIGHT_MEDIUM,  /**< Medium weight */
  CMP_M3_FONT_WEIGHT_BOLD     /**< Bold weight */
} cmp_m3_font_weight_t;

/**
 * @brief Material 3 type metrics for a given category.
 */
typedef struct {
  float font_size;             /**< Font size in points */
  float line_height;           /**< Line height in points */
  float tracking;              /**< Letter tracking/spacing */
  cmp_m3_font_weight_t weight; /**< Typography weight */
} cmp_m3_type_metrics_t;

/**
 * @brief Resolves type metrics for a given Material 3 typography category.
 *
 * @param category The Material 3 typescale category.
 * @param out_metrics A pointer to the metrics struct to populate.
 * @return 0 on success, or an error code.
 */
int cmp_m3_typography_resolve(cmp_m3_typescale_category_t category, cmp_m3_type_metrics_t *out_metrics);

/**
 * @brief Gets the font family string for Material 3.
 *
 * @param is_brand Whether to return the brand font family or plain text family.
 * @param out_font_family A pointer to receive the font family string pointer.
 * @return 0 on success, or an error code.
 */
int cmp_m3_typography_get_family(int is_brand, const char **out_font_family);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_TYPOGRAPHY_H */
/* clang-format on */