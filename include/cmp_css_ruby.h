#ifndef CMP_CSS_RUBY_H
#define CMP_CSS_RUBY_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_ruby.h
 * @brief CSS Ruby Layout implementation.
 */

/**
 * @brief Represents the logical elements of a Ruby layout.
 */
typedef enum cmp_layout_ruby_tag {
  CMP_LAYOUT_RUBY_RUBY = 0,
  CMP_LAYOUT_RUBY_RT,
  CMP_LAYOUT_RUBY_RTC,
  CMP_LAYOUT_RUBY_RB
} cmp_layout_ruby_tag_t;

/**
 * @brief Represents ruby layout primitive.
 */
typedef struct cmp_layout_ruby {
  cmp_layout_ruby_tag_t tag_type;
} cmp_layout_ruby_t;

/**
 * @brief Represents ruby-position property.
 */
typedef enum cmp_ruby_position {
  CMP_RUBY_POSITION_OVER = 0,
  CMP_RUBY_POSITION_UNDER,
  CMP_RUBY_POSITION_INTER_CHARACTER
} cmp_ruby_position_t;

/**
 * @brief Represents ruby-align property.
 */
typedef enum cmp_ruby_align {
  CMP_RUBY_ALIGN_START = 0,
  CMP_RUBY_ALIGN_CENTER,
  CMP_RUBY_ALIGN_SPACE_BETWEEN,
  CMP_RUBY_ALIGN_SPACE_AROUND
} cmp_ruby_align_t;

/**
 * @brief Represents ruby-position and ruby-align group.
 */
typedef struct cmp_prop_ruby_align_group {
  cmp_ruby_position_t position;
  cmp_ruby_align_t align;
} cmp_prop_ruby_align_group_t;

/**
 * @brief Represents ruby-overhang property.
 */
typedef enum cmp_ruby_overhang {
  CMP_RUBY_OVERHANG_AUTO = 0,
  CMP_RUBY_OVERHANG_NONE
} cmp_ruby_overhang_t;

/**
 * @brief Represents ruby-overhang.
 */
typedef struct cmp_prop_ruby_overhang {
  cmp_ruby_overhang_t overhang;
} cmp_prop_ruby_overhang_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a ruby layout tag type.
 * @param layout The layout struct to initialize.
 * @param tag The tag type.
 * @return 0 on success.
 */
int cmp_layout_ruby_init(cmp_layout_ruby_t *layout, cmp_layout_ruby_tag_t tag);

/**
 * @brief Initializes ruby-align group.
 * @param group The group to initialize.
 * @param position position value.
 * @param align align value.
 * @return 0 on success.
 */
int cmp_prop_ruby_align_group_init(cmp_prop_ruby_align_group_t *group,
                                   cmp_ruby_position_t position,
                                   cmp_ruby_align_t align);

/**
 * @brief Initializes ruby-overhang property.
 * @param prop The property to initialize.
 * @param overhang overhang value.
 * @return 0 on success.
 */
int cmp_prop_ruby_overhang_init(cmp_prop_ruby_overhang_t *prop,
                                cmp_ruby_overhang_t overhang);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_RUBY_H */
