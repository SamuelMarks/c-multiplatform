#ifndef CMP_CSS_BOX_ALIGNMENT_H
#define CMP_CSS_BOX_ALIGNMENT_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_box_alignment.h
 * @brief CSS Box Alignment Layout (Level 3) implementation.
 */

/**
 * @brief Represents alignment values (start, end, center, space-between, etc).
 */
typedef enum cmp_align_value {
  CMP_ALIGN_NORMAL = 0,
  CMP_ALIGN_STRETCH,
  CMP_ALIGN_CENTER,
  CMP_ALIGN_START,
  CMP_ALIGN_END,
  CMP_ALIGN_FLEX_START,
  CMP_ALIGN_FLEX_END,
  CMP_ALIGN_BASELINE,
  CMP_ALIGN_FIRST_BASELINE,
  CMP_ALIGN_LAST_BASELINE,
  CMP_ALIGN_SPACE_BETWEEN,
  CMP_ALIGN_SPACE_AROUND,
  CMP_ALIGN_SPACE_EVENLY,
  CMP_ALIGN_SAFE_CENTER,
  CMP_ALIGN_UNSAFE_CENTER
} cmp_align_value_t;

/**
 * @brief Represents justify-content, align-content, and place-content.
 */
typedef struct cmp_prop_align_content_group {
  cmp_align_value_t justify_content; /**< justify-content */
  cmp_align_value_t align_content;   /**< align-content */
} cmp_prop_align_content_group_t;

/**
 * @brief Represents justify-items, align-items, and place-items.
 */
typedef struct cmp_prop_align_items_group {
  cmp_align_value_t justify_items; /**< justify-items */
  cmp_align_value_t align_items;   /**< align-items */
} cmp_prop_align_items_group_t;

/**
 * @brief Represents justify-self, align-self, and place-self.
 */
typedef struct cmp_prop_align_self_group {
  cmp_align_value_t justify_self; /**< justify-self */
  cmp_align_value_t align_self;   /**< align-self */
} cmp_prop_align_self_group_t;

/**
 * @brief Represents gap, row-gap, and column-gap.
 */
typedef struct cmp_prop_gap {
  char *row_gap;    /**< row-gap string representation (e.g., "10px") */
  char *column_gap; /**< column-gap string representation (e.g., "1rem") */
} cmp_prop_gap_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes an align content group.
 * @param group The content group to initialize.
 * @param jc justify-content value.
 * @param ac align-content value.
 * @return 0 on success.
 */
int cmp_prop_align_content_group_init(cmp_prop_align_content_group_t *group,
                                      cmp_align_value_t jc,
                                      cmp_align_value_t ac);

/**
 * @brief Initializes an align items group.
 * @param group The items group to initialize.
 * @param ji justify-items value.
 * @param ai align-items value.
 * @return 0 on success.
 */
int cmp_prop_align_items_group_init(cmp_prop_align_items_group_t *group,
                                    cmp_align_value_t ji, cmp_align_value_t ai);

/**
 * @brief Initializes an align self group.
 * @param group The self group to initialize.
 * @param js justify-self value.
 * @param as align-self value.
 * @return 0 on success.
 */
int cmp_prop_align_self_group_init(cmp_prop_align_self_group_t *group,
                                   cmp_align_value_t js, cmp_align_value_t as);

/**
 * @brief Initializes a gap property.
 * @param gap The gap property to initialize.
 * @param row_gap The row gap string value (can be NULL).
 * @param col_gap The column gap string value (can be NULL).
 * @return 0 on success.
 */
int cmp_prop_gap_init(cmp_prop_gap_t *gap, const char *row_gap,
                      const char *col_gap);

/**
 * @brief Frees resources in a gap property.
 * @param gap The gap property to free.
 * @return 0 on success.
 */
int cmp_prop_gap_free(cmp_prop_gap_t *gap);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_BOX_ALIGNMENT_H */