#ifndef CMP_CSS_WRITING_MODES_H
#define CMP_CSS_WRITING_MODES_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_writing_modes.h
 * @brief CSS Writing Modes and Direction implementation.
 */

/**
 * @brief Represents direction property.
 */
typedef enum cmp_direction {
  CMP_DIRECTION_LTR = 0,
  CMP_DIRECTION_RTL
} cmp_direction_t;

/**
 * @brief Represents unicode-bidi property.
 */
typedef enum cmp_unicode_bidi {
  CMP_UNICODE_BIDI_NORMAL = 0,
  CMP_UNICODE_BIDI_EMBED,
  CMP_UNICODE_BIDI_ISOLATE,
  CMP_UNICODE_BIDI_BIDI_OVERRIDE,
  CMP_UNICODE_BIDI_ISOLATE_OVERRIDE,
  CMP_UNICODE_BIDI_PLAINTEXT
} cmp_unicode_bidi_t;

/**
 * @brief Represents direction and unicode-bidi properties.
 */
typedef struct cmp_prop_bidi_group {
  cmp_direction_t direction;
  cmp_unicode_bidi_t unicode_bidi;
} cmp_prop_bidi_group_t;

/**
 * @brief Represents writing-mode property.
 */
typedef enum cmp_writing_mode {
  CMP_WRITING_MODE_HORIZONTAL_TB = 0,
  CMP_WRITING_MODE_VERTICAL_RL,
  CMP_WRITING_MODE_VERTICAL_LR,
  CMP_WRITING_MODE_SIDEWAYS_RL,
  CMP_WRITING_MODE_SIDEWAYS_LR
} cmp_writing_mode_t;

/**
 * @brief Represents text-orientation property.
 */
typedef enum cmp_text_orientation {
  CMP_TEXT_ORIENTATION_MIXED = 0,
  CMP_TEXT_ORIENTATION_UPRIGHT,
  CMP_TEXT_ORIENTATION_SIDEWAYS
} cmp_text_orientation_t;

/**
 * @brief Represents text-combine-upright property.
 */
typedef enum cmp_text_combine_upright_type {
  CMP_TEXT_COMBINE_UPRIGHT_NONE = 0,
  CMP_TEXT_COMBINE_UPRIGHT_ALL,
  CMP_TEXT_COMBINE_UPRIGHT_DIGITS
} cmp_text_combine_upright_type_t;

/**
 * @brief Represents writing-mode, text-orientation, text-combine-upright
 * properties.
 */
typedef struct cmp_prop_writing_mode_group {
  cmp_writing_mode_t writing_mode;
  cmp_text_orientation_t text_orientation;
  cmp_text_combine_upright_type_t text_combine_upright_type;
  int text_combine_upright_digits; /**< Integer value for digits variant
                                      (usually 2, 3, or 4) */
} cmp_prop_writing_mode_group_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a bidi group.
 * @param group The group to initialize.
 * @param dir direction value.
 * @param bidi unicode-bidi value.
 * @return 0 on success.
 */
int cmp_prop_bidi_group_init(cmp_prop_bidi_group_t *group, cmp_direction_t dir,
                             cmp_unicode_bidi_t bidi);

/**
 * @brief Initializes a writing mode group.
 * @param group The group to initialize.
 * @param mode writing-mode value.
 * @param orient text-orientation value.
 * @param combine_type text-combine-upright type.
 * @param combine_digits valid if combine_type is DIGITS.
 * @return 0 on success.
 */
int cmp_prop_writing_mode_group_init(
    cmp_prop_writing_mode_group_t *group, cmp_writing_mode_t mode,
    cmp_text_orientation_t orient, cmp_text_combine_upright_type_t combine_type,
    int combine_digits);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_WRITING_MODES_H */
