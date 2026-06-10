#ifndef CMP_CSS_ANIMATIONS_H
#define CMP_CSS_ANIMATIONS_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_animations.h
 * @brief CSS Transitions & Animations (Level 1) implementation.
 */

/**
 * @brief Represents transition-behavior values.
 */
typedef enum cmp_transition_behavior {
  CMP_TRANSITION_BEHAVIOR_NORMAL = 0,
  CMP_TRANSITION_BEHAVIOR_ALLOW_DISCRETE
} cmp_transition_behavior_t;

/**
 * @brief Represents transition-property, transition-duration,
 * transition-timing-function, transition-delay, transition-behavior, and
 * transition.
 */
typedef struct cmp_prop_transition_group {
  char *property;        /**< transition-property (e.g., "all", "width") */
  char *duration;        /**< transition-duration (e.g., "2s", "500ms") */
  char *timing_function; /**< transition-timing-function (e.g., "ease",
                            "cubic-bezier(0.1, 0.7, 1.0, 0.1)") */
  char *delay;           /**< transition-delay (e.g., "1s", "0s") */
  cmp_transition_behavior_t behavior; /**< transition-behavior */
} cmp_prop_transition_group_t;

/**
 * @brief Represents animation-direction values.
 */
typedef enum cmp_animation_direction {
  CMP_ANIMATION_DIRECTION_NORMAL = 0,
  CMP_ANIMATION_DIRECTION_REVERSE,
  CMP_ANIMATION_DIRECTION_ALTERNATE,
  CMP_ANIMATION_DIRECTION_ALTERNATE_REVERSE
} cmp_animation_direction_t;

/**
 * @brief Represents animation-play-state values.
 */
typedef enum cmp_animation_play_state {
  CMP_ANIMATION_PLAY_STATE_RUNNING = 0,
  CMP_ANIMATION_PLAY_STATE_PAUSED
} cmp_animation_play_state_t;

/**
 * @brief Represents animation-fill-mode values.
 */
typedef enum cmp_animation_fill_mode {
  CMP_ANIMATION_FILL_MODE_NONE = 0,
  CMP_ANIMATION_FILL_MODE_FORWARDS,
  CMP_ANIMATION_FILL_MODE_BACKWARDS,
  CMP_ANIMATION_FILL_MODE_BOTH
} cmp_animation_fill_mode_t;

/**
 * @brief Represents animation-composition values.
 */
typedef enum cmp_animation_composition {
  CMP_ANIMATION_COMPOSITION_REPLACE = 0,
  CMP_ANIMATION_COMPOSITION_ADD,
  CMP_ANIMATION_COMPOSITION_ACCUMULATE
} cmp_animation_composition_t;

/**
 * @brief Represents animation-name, animation-duration,
 * animation-timing-function, animation-iteration-count, animation-direction,
 * animation-play-state, animation-delay, animation-fill-mode,
 * animation-composition, and animation.
 */
typedef struct cmp_prop_animation_group {
  char *name;            /**< animation-name (e.g., "slidein", "none") */
  char *duration;        /**< animation-duration (e.g., "3s") */
  char *timing_function; /**< animation-timing-function (e.g., "linear") */
  char *
      iteration_count; /**< animation-iteration-count (e.g., "infinite", "3") */
  cmp_animation_direction_t direction;     /**< animation-direction */
  cmp_animation_play_state_t play_state;   /**< animation-play-state */
  char *delay;                             /**< animation-delay (e.g., "2s") */
  cmp_animation_fill_mode_t fill_mode;     /**< animation-fill-mode */
  cmp_animation_composition_t composition; /**< animation-composition */
} cmp_prop_animation_group_t;

/**
 * @brief Represents an `@keyframes` rule.
 */
typedef struct cmp_at_rule_keyframes {
  char *name;    /**< Name of the keyframes rule */
  char *content; /**< The raw body of the keyframes block */
} cmp_at_rule_keyframes_t;

/**
 * @brief Represents an `@starting-style` rule.
 */
typedef struct cmp_at_rule_starting_style {
  char *content; /**< The raw body of the starting-style block */
} cmp_at_rule_starting_style_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a transition property group.
 * @param group The transition group to initialize.
 * @param property transition-property string (can be NULL).
 * @param duration transition-duration string (can be NULL).
 * @param timing_function transition-timing-function string (can be NULL).
 * @param delay transition-delay string (can be NULL).
 * @param behavior transition-behavior enum.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_transition_group_init(cmp_prop_transition_group_t *group,
                                   const char *property, const char *duration,
                                   const char *timing_function,
                                   const char *delay,
                                   cmp_transition_behavior_t behavior);

/**
 * @brief Frees resources in a transition property group.
 * @param group The group to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_transition_group_free(cmp_prop_transition_group_t *group);

/**
 * @brief Initializes an animation property group.
 * @param group The animation group to initialize.
 * @param name animation-name string (can be NULL).
 * @param duration animation-duration string (can be NULL).
 * @param timing_function animation-timing-function string (can be NULL).
 * @param iteration_count animation-iteration-count string (can be NULL).
 * @param direction animation-direction enum.
 * @param play_state animation-play-state enum.
 * @param delay animation-delay string (can be NULL).
 * @param fill_mode animation-fill-mode enum.
 * @param composition animation-composition enum.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_animation_group_init(
    cmp_prop_animation_group_t *group, const char *name, const char *duration,
    const char *timing_function, const char *iteration_count,
    cmp_animation_direction_t direction, cmp_animation_play_state_t play_state,
    const char *delay, cmp_animation_fill_mode_t fill_mode,
    cmp_animation_composition_t composition);

/**
 * @brief Frees resources in an animation property group.
 * @param group The group to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_animation_group_free(cmp_prop_animation_group_t *group);

/**
 * @brief Initializes a keyframes at-rule.
 * @param rule The keyframes rule to initialize.
 * @param name The name identifier.
 * @param content The block content.
 * @return 0 on success, non-zero on error.
 */
int cmp_at_rule_keyframes_init(cmp_at_rule_keyframes_t *rule, const char *name,
                               const char *content);

/**
 * @brief Frees resources in a keyframes at-rule.
 * @param rule The rule to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_at_rule_keyframes_free(cmp_at_rule_keyframes_t *rule);

/**
 * @brief Initializes a starting-style at-rule.
 * @param rule The starting-style rule to initialize.
 * @param content The block content.
 * @return 0 on success, non-zero on error.
 */
int cmp_at_rule_starting_style_init(cmp_at_rule_starting_style_t *rule,
                                    const char *content);

/**
 * @brief Frees resources in a starting-style at-rule.
 * @param rule The rule to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_at_rule_starting_style_free(cmp_at_rule_starting_style_t *rule);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_ANIMATIONS_H */