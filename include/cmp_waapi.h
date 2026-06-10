/* clang-format off */
#ifndef CMP_WAAPI_H
#define CMP_WAAPI_H

#include "cmp_css_animations.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents WAAPI AnimationTimeline binding.
 */
typedef struct cmp_waapi_timeline {
  float current_time; /**< The current time of the timeline */
} cmp_waapi_timeline_t;

/**
 * @brief Represents WAAPI DocumentTimeline binding.
 */
typedef struct cmp_waapi_doc_timeline {
  cmp_waapi_timeline_t base; /**< Inherits from AnimationTimeline */
  float origin_time;         /**< Time offset */
} cmp_waapi_doc_timeline_t;

/**
 * @brief Represents WAAPI KeyframeEffect binding.
 */
typedef struct cmp_waapi_effect {
  void *target; /**< Opaque pointer to target DOM element */
  float iteration_duration;
  float active_duration;
  float end_time;
} cmp_waapi_effect_t;

/**
 * @brief Represents WAAPI Animation binding.
 */
typedef struct cmp_waapi_animation {
  char *id;                       /**< Animation ID */
  cmp_waapi_effect_t *effect;     /**< Associated effect */
  cmp_waapi_timeline_t *timeline; /**< Associated timeline */
  float start_time;               /**< Scheduled start time */
  float current_time;             /**< Current time */
  float playback_rate;            /**< Playback speed */
} cmp_waapi_animation_t;

/**
 * @brief Initialize a DocumentTimeline.
 * @param tl Timeline to initialize.
 * @return 0 on success.
 */
int cmp_waapi_doc_timeline_init(cmp_waapi_doc_timeline_t *tl);

/**
 * @brief Initialize a KeyframeEffect.
 * @param effect Effect to initialize.
 * @param target Opaque target element.
 * @return 0 on success.
 */
int cmp_waapi_effect_init(cmp_waapi_effect_t *effect, void *target);

/**
 * @brief Initialize an Animation.
 * @param anim Animation to initialize.
 * @param id Animation ID (can be NULL).
 * @return 0 on success.
 */
int cmp_waapi_animation_init(cmp_waapi_animation_t *anim, const char *id);

/**
 * @brief Free an Animation.
 * @param anim Animation to free.
 * @return 0 on success.
 */
int cmp_waapi_animation_free(cmp_waapi_animation_t *anim);

#ifdef __cplusplus
}
#endif

#endif /* CMP_WAAPI_H */
/* clang-format on */
