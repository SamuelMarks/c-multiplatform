#ifndef CMP_CSS_CONTAINMENT_H
#define CMP_CSS_CONTAINMENT_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_containment.h
 * @brief CSS Containment & Optimization implementation.
 */

/**
 * @brief Represents contain property.
 */
typedef enum cmp_contain_flag {
  CMP_CONTAIN_NONE = 0,
  CMP_CONTAIN_STRICT = 1,
  CMP_CONTAIN_CONTENT = 2,
  CMP_CONTAIN_SIZE = 4,
  CMP_CONTAIN_INLINE_SIZE = 8,
  CMP_CONTAIN_LAYOUT = 16,
  CMP_CONTAIN_STYLE = 32,
  CMP_CONTAIN_PAINT = 64
} cmp_contain_flag_t;

/**
 * @brief Represents contain property.
 */
typedef struct cmp_prop_contain {
  int flags; /**< Bitmask of cmp_contain_flag_t */
} cmp_prop_contain_t;

/**
 * @brief Represents content-visibility property.
 */
typedef enum cmp_content_visibility {
  CMP_CONTENT_VISIBILITY_VISIBLE = 0,
  CMP_CONTENT_VISIBILITY_HIDDEN,
  CMP_CONTENT_VISIBILITY_AUTO
} cmp_content_visibility_t;

/**
 * @brief Represents content-visibility.
 */
typedef struct cmp_prop_content_visibility {
  cmp_content_visibility_t visibility;
} cmp_prop_content_visibility_t;

/**
 * @brief Represents contain-intrinsic-size group.
 */
typedef struct cmp_prop_contain_intrinsic_t {
  char *size;
  char *block_size;
  char *inline_size;
} cmp_prop_contain_intrinsic_t;

/**
 * @brief Represents will-change property.
 */
typedef struct cmp_prop_will_change {
  char *will_change; /**< String representation, e.g., "auto", "transform,
                        opacity" */
} cmp_prop_will_change_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes contain property.
 * @param prop The property to initialize.
 * @param flags contain flags.
 * @return 0 on success.
 */
int cmp_prop_contain_init(cmp_prop_contain_t *prop, int flags);

/**
 * @brief Initializes content-visibility property.
 * @param prop The property to initialize.
 * @param visibility visibility value.
 * @return 0 on success.
 */
int cmp_prop_content_visibility_init(cmp_prop_content_visibility_t *prop,
                                     cmp_content_visibility_t visibility);

/**
 * @brief Initializes contain-intrinsic-size group.
 * @param group The group to initialize.
 * @param size size string.
 * @param block block size string.
 * @param inln inline size string.
 * @return 0 on success.
 */
int cmp_prop_contain_intrinsic_init(cmp_prop_contain_intrinsic_t *group,
                                    const char *size, const char *block,
                                    const char *inln);

/**
 * @brief Frees contain-intrinsic-size group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_contain_intrinsic_free(cmp_prop_contain_intrinsic_t *group);

/**
 * @brief Initializes will-change property.
 * @param prop The property to initialize.
 * @param will_change string representation.
 * @return 0 on success.
 */
int cmp_prop_will_change_init(cmp_prop_will_change_t *prop,
                              const char *will_change);

/**
 * @brief Frees will-change property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_will_change_free(cmp_prop_will_change_t *prop);

/**
 * @brief Represents container-type property.
 */
typedef enum cmp_prop_container_type {
  CMP_CONTAINER_TYPE_NORMAL = 0,
  CMP_CONTAINER_TYPE_SIZE,
  CMP_CONTAINER_TYPE_INLINE_SIZE
} cmp_prop_container_type_t;

/**
 * @brief Represents container property group.
 */
typedef struct cmp_prop_container_group {
  cmp_prop_container_type_t type;
  char *name; /**< container-name (dynamically allocated string) */
} cmp_prop_container_group_t;

/**
 * @brief Initializes container group.
 * @param group The group to initialize.
 * @param type Container type.
 * @param name Container name.
 * @return 0 on success.
 */
int cmp_prop_container_group_init(cmp_prop_container_group_t *group,
                                  cmp_prop_container_type_t type,
                                  const char *name);

/**
 * @brief Frees container group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_container_group_free(cmp_prop_container_group_t *group);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_CONTAINMENT_H */
