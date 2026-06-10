#ifndef CMP_CSS_LOGICAL_H
#define CMP_CSS_LOGICAL_H

/* clang-format off */
#include <stddef.h>
#include "cmp_css_color.h"
#include "cmp_css_backgrounds_borders.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_logical.h
 * @brief CSS Logical Properties implementation.
 */

/**
 * @brief Represents logical size group properties.
 */
typedef struct cmp_prop_logical_size_group {
  char *block_size;
  char *inline_size;
  char *min_block_size;
  char *min_inline_size;
  char *max_block_size;
  char *max_inline_size;
} cmp_prop_logical_size_group_t;

/**
 * @brief Represents logical margin properties.
 */
typedef struct cmp_prop_logical_margin_group {
  char *margin_block_start;
  char *margin_block_end;
  char *margin_inline_start;
  char *margin_inline_end;
} cmp_prop_logical_margin_group_t;

/**
 * @brief Represents logical padding properties.
 */
typedef struct cmp_prop_logical_padding_group {
  char *padding_block_start;
  char *padding_block_end;
  char *padding_inline_start;
  char *padding_inline_end;
} cmp_prop_logical_padding_group_t;

/**
 * @brief Represents logical border part (width, style, color).
 */
typedef struct cmp_logical_border_part {
  char *width;
  cmp_border_style_t style;
  cmp_prop_color_t color;
} cmp_logical_border_part_t;

/**
 * @brief Represents logical border group properties.
 */
typedef struct cmp_prop_logical_border_group {
  cmp_logical_border_part_t block_start;
  cmp_logical_border_part_t block_end;
  cmp_logical_border_part_t inline_start;
  cmp_logical_border_part_t inline_end;
} cmp_prop_logical_border_group_t;

/**
 * @brief Represents logical inset properties.
 */
typedef struct cmp_prop_logical_inset_group {
  char *inset_block_start;
  char *inset_block_end;
  char *inset_inline_start;
  char *inset_inline_end;
} cmp_prop_logical_inset_group_t;

/**
 * @brief Represents logical border-radius properties.
 */
typedef struct cmp_prop_logical_radius_group {
  char *start_start;
  char *start_end;
  char *end_start;
  char *end_end;
} cmp_prop_logical_radius_group_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes logical size group.
 * @param group The group to initialize.
 * @param b_size block-size.
 * @param i_size inline-size.
 * @param min_b min-block-size.
 * @param min_i min-inline-size.
 * @param max_b max-block-size.
 * @param max_i max-inline-size.
 * @return 0 on success.
 */
int cmp_prop_logical_size_group_init(cmp_prop_logical_size_group_t *group,
                                     const char *b_size, const char *i_size,
                                     const char *min_b, const char *min_i,
                                     const char *max_b, const char *max_i);

/**
 * @brief Frees logical size group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_logical_size_group_free(cmp_prop_logical_size_group_t *group);

/**
 * @brief Initializes logical margin group.
 * @param group The group to initialize.
 * @param bs margin-block-start.
 * @param be margin-block-end.
 * @param is margin-inline-start.
 * @param ie margin-inline-end.
 * @return 0 on success.
 */
int cmp_prop_logical_margin_group_init(cmp_prop_logical_margin_group_t *group,
                                       const char *bs, const char *be,
                                       const char *is, const char *ie);

/**
 * @brief Frees logical margin group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_logical_margin_group_free(cmp_prop_logical_margin_group_t *group);

/**
 * @brief Initializes logical padding group.
 * @param group The group to initialize.
 * @param bs padding-block-start.
 * @param be padding-block-end.
 * @param is padding-inline-start.
 * @param ie padding-inline-end.
 * @return 0 on success.
 */
int cmp_prop_logical_padding_group_init(cmp_prop_logical_padding_group_t *group,
                                        const char *bs, const char *be,
                                        const char *is, const char *ie);

/**
 * @brief Frees logical padding group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_logical_padding_group_free(
    cmp_prop_logical_padding_group_t *group);

/**
 * @brief Initializes a single logical border part.
 * @param part The part to initialize.
 * @param width border width string.
 * @param style border style.
 * @param color border color.
 * @return 0 on success.
 */
int cmp_logical_border_part_init(cmp_logical_border_part_t *part,
                                 const char *width, cmp_border_style_t style,
                                 const cmp_prop_color_t *color);

/**
 * @brief Frees a single logical border part.
 * @param part The part to free.
 * @return 0 on success.
 */
int cmp_logical_border_part_free(cmp_logical_border_part_t *part);

/**
 * @brief Initializes logical border group.
 * @param group The group to initialize.
 * @return 0 on success. Note: Parts must be initialized individually.
 */
int cmp_prop_logical_border_group_init(cmp_prop_logical_border_group_t *group);

/**
 * @brief Frees logical border group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_logical_border_group_free(cmp_prop_logical_border_group_t *group);

/**
 * @brief Initializes logical inset group.
 * @param group The group to initialize.
 * @param bs inset-block-start.
 * @param be inset-block-end.
 * @param is inset-inline-start.
 * @param ie inset-inline-end.
 * @return 0 on success.
 */
int cmp_prop_logical_inset_group_init(cmp_prop_logical_inset_group_t *group,
                                      const char *bs, const char *be,
                                      const char *is, const char *ie);

/**
 * @brief Frees logical inset group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_logical_inset_group_free(cmp_prop_logical_inset_group_t *group);

/**
 * @brief Initializes logical radius group.
 * @param group The group to initialize.
 * @param ss border-start-start-radius.
 * @param se border-start-end-radius.
 * @param es border-end-start-radius.
 * @param ee border-end-end-radius.
 * @return 0 on success.
 */
int cmp_prop_logical_radius_group_init(cmp_prop_logical_radius_group_t *group,
                                       const char *ss, const char *se,
                                       const char *es, const char *ee);

/**
 * @brief Frees logical radius group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_logical_radius_group_free(cmp_prop_logical_radius_group_t *group);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_LOGICAL_H */
