#ifndef CMP_CSS_BOX_MODEL_H
#define CMP_CSS_BOX_MODEL_H

/* clang-format off */
#include "cmp_css_values.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents CSS intrinsic size keywords.
 */
typedef enum cmp_intrinsic_size {
  CMP_INTRINSIC_SIZE_NONE = 0,
  CMP_INTRINSIC_SIZE_MAX_CONTENT,
  CMP_INTRINSIC_SIZE_MIN_CONTENT,
  CMP_INTRINSIC_SIZE_FIT_CONTENT
} cmp_intrinsic_size_t;

/**
 * @brief Type of a CSS size property (width, height, min-width, etc.).
 */
typedef enum cmp_prop_size_type {
  CMP_PROP_SIZE_AUTO = 0,
  CMP_PROP_SIZE_LENGTH_PHYS,
  CMP_PROP_SIZE_LENGTH_REL,
  CMP_PROP_SIZE_PERCENT,
  CMP_PROP_SIZE_INTRINSIC
} cmp_prop_size_type_t;

/**
 * @brief Represents a CSS size property (e.g., width, height).
 */
typedef struct cmp_prop_size {
  cmp_prop_size_type_t type; /**< The type of the size */
  union {
    cmp_length_phys_t phys;         /**< Physical length */
    cmp_length_rel_t rel;           /**< Relative length */
    cmp_val_percent_t percent;      /**< Percentage */
    cmp_intrinsic_size_t intrinsic; /**< Intrinsic size keyword */
  } value;                          /**< The value of the size */
} cmp_prop_size_t;

/**
 * @brief Represents min/max size limits.
 */
typedef struct cmp_prop_size_limits {
  cmp_prop_size_t min_width;  /**< min-width */
  cmp_prop_size_t min_height; /**< min-height */
  cmp_prop_size_t max_width;  /**< max-width */
  cmp_prop_size_t max_height; /**< max-height */
} cmp_prop_size_limits_t;

/**
 * @brief Type of a margin value.
 */
typedef enum cmp_prop_margin_type {
  CMP_PROP_MARGIN_AUTO = 0,
  CMP_PROP_MARGIN_LENGTH_PHYS,
  CMP_PROP_MARGIN_LENGTH_REL,
  CMP_PROP_MARGIN_PERCENT
} cmp_prop_margin_type_t;

/**
 * @brief Represents a margin property (margin-top, margin-right, etc.).
 */
typedef struct cmp_prop_margin {
  cmp_prop_margin_type_t type; /**< The type of the margin */
  union {
    cmp_length_phys_t phys;    /**< Physical length */
    cmp_length_rel_t rel;      /**< Relative length */
    cmp_val_percent_t percent; /**< Percentage */
  } value;                     /**< The value of the margin */
} cmp_prop_margin_t;

/**
 * @brief Type of a padding value.
 */
typedef enum cmp_prop_padding_type {
  CMP_PROP_PADDING_LENGTH_PHYS = 0,
  CMP_PROP_PADDING_LENGTH_REL,
  CMP_PROP_PADDING_PERCENT
} cmp_prop_padding_type_t;

/**
 * @brief Represents a padding property (padding-top, etc.).
 */
typedef struct cmp_prop_padding {
  cmp_prop_padding_type_t type; /**< The type of the padding */
  union {
    cmp_length_phys_t phys;    /**< Physical length */
    cmp_length_rel_t rel;      /**< Relative length */
    cmp_val_percent_t percent; /**< Percentage */
  } value;                     /**< The value of the padding */
} cmp_prop_padding_t;

/**
 * @brief Represents the box-sizing property.
 */
typedef enum cmp_prop_box_sizing {
  CMP_BOX_SIZING_CONTENT_BOX = 0,
  CMP_BOX_SIZING_BORDER_BOX
} cmp_prop_box_sizing_t;

/**
 * @brief Represents the margin-trim property.
 */
typedef enum cmp_prop_margin_trim {
  CMP_MARGIN_TRIM_NONE = 0,
  CMP_MARGIN_TRIM_BLOCK,
  CMP_MARGIN_TRIM_BLOCK_START,
  CMP_MARGIN_TRIM_BLOCK_END,
  CMP_MARGIN_TRIM_INLINE,
  CMP_MARGIN_TRIM_INLINE_START,
  CMP_MARGIN_TRIM_INLINE_END,
  CMP_MARGIN_TRIM_ALL
} cmp_prop_margin_trim_t;

/**
 * @brief Type of the aspect-ratio property.
 */
typedef enum cmp_prop_aspect_ratio_type {
  CMP_ASPECT_RATIO_AUTO = 0,
  CMP_ASPECT_RATIO_RATIO,
  CMP_ASPECT_RATIO_AUTO_AND_RATIO
} cmp_prop_aspect_ratio_type_t;

/**
 * @brief Represents the aspect-ratio property.
 */
typedef struct cmp_prop_aspect_ratio {
  cmp_prop_aspect_ratio_type_t type; /**< The type of the aspect-ratio */
  cmp_val_ratio_t ratio;             /**< The ratio value if applicable */
} cmp_prop_aspect_ratio_t;

/**
 * @brief Initializes a size property to 'auto'.
 * @param size The size property to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_size_init_auto(cmp_prop_size_t *size);

/**
 * @brief Initializes a size property to a physical length.
 * @param size The size property to initialize.
 * @param val The value.
 * @param unit The unit.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_size_init_phys(cmp_prop_size_t *size, float val,
                            cmp_length_phys_unit_t unit);

/**
 * @brief Initializes a size property to a percentage.
 * @param size The size property to initialize.
 * @param val The percentage value.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_size_init_percent(cmp_prop_size_t *size, float val);

/**
 * @brief Initializes a size property to an intrinsic keyword.
 * @param size The size property to initialize.
 * @param intrinsic The keyword.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_size_init_intrinsic(cmp_prop_size_t *size,
                                 cmp_intrinsic_size_t intrinsic);

/**
 * @brief Initializes a margin property to 'auto'.
 * @param margin The margin property to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_margin_init_auto(cmp_prop_margin_t *margin);

/**
 * @brief Initializes a margin property to a physical length.
 * @param margin The margin property to initialize.
 * @param val The value.
 * @param unit The unit.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_margin_init_phys(cmp_prop_margin_t *margin, float val,
                              cmp_length_phys_unit_t unit);

/**
 * @brief Initializes a padding property to a physical length.
 * @param padding The padding property to initialize.
 * @param val The value.
 * @param unit The unit.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_padding_init_phys(cmp_prop_padding_t *padding, float val,
                               cmp_length_phys_unit_t unit);

/**
 * @brief Initializes an aspect-ratio property to 'auto'.
 * @param ar The aspect ratio property to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_aspect_ratio_init_auto(cmp_prop_aspect_ratio_t *ar);

/**
 * @brief Initializes an aspect-ratio property to a given ratio.
 * @param ar The aspect ratio property to initialize.
 * @param num The numerator.
 * @param den The denominator.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_aspect_ratio_init_ratio(cmp_prop_aspect_ratio_t *ar, float num,
                                     float den);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_BOX_MODEL_H */