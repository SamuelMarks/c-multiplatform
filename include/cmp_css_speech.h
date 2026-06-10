#ifndef CMP_CSS_SPEECH_H
#define CMP_CSS_SPEECH_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_speech.h
 * @brief CSS Speech Module (Aural CSS) implementation.
 */

/**
 * @brief Represents voice property group.
 */
typedef struct cmp_prop_voice_group {
  char *voice_volume;
  char **voice_family; /**< NULL-terminated array of family strings */
  char *voice_rate;
  char *voice_pitch;
  char *voice_range;
  char *voice_stress;
  char *voice_duration;
} cmp_prop_voice_group_t;

/**
 * @brief Represents speak property.
 */
typedef enum cmp_speak_value {
  CMP_SPEAK_AUTO = 0,
  CMP_SPEAK_NEVER,
  CMP_SPEAK_ALWAYS
} cmp_speak_value_t;

/**
 * @brief Represents speak property.
 */
typedef struct cmp_prop_speak {
  cmp_speak_value_t speak;
  char *speak_as; /**< String representation */
} cmp_prop_speak_t;

/**
 * @brief Represents pause property.
 */
typedef struct cmp_prop_pause {
  char *pause_before;
  char *pause_after;
} cmp_prop_pause_t;

/**
 * @brief Represents rest property.
 */
typedef struct cmp_prop_rest {
  char *rest_before;
  char *rest_after;
} cmp_prop_rest_t;

/**
 * @brief Represents cue property.
 */
typedef struct cmp_prop_cue {
  char *cue_before;
  char *cue_after;
} cmp_prop_cue_t;

/**
 * @brief Represents play-during property.
 */
typedef struct cmp_prop_play_during {
  char *play_during; /**< String representation */
} cmp_prop_play_during_t;

/**
 * @brief Represents @media speech rule block properties.
 */
typedef struct cmp_at_rule_media_speech {
  cmp_prop_voice_group_t voice;
  cmp_prop_speak_t speak;
  cmp_prop_pause_t pause;
  cmp_prop_rest_t rest;
  cmp_prop_cue_t cue;
  cmp_prop_play_during_t play_during;
} cmp_at_rule_media_speech_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes voice group.
 * @param group The group to initialize.
 * @param vol volume.
 * @param family NULL-terminated array of voice family strings.
 * @param rate rate.
 * @param pitch pitch.
 * @param range range.
 * @param stress stress.
 * @param duration duration.
 * @return 0 on success.
 */
int cmp_prop_voice_group_init(cmp_prop_voice_group_t *group, const char *vol,
                              const char **family, const char *rate,
                              const char *pitch, const char *range,
                              const char *stress, const char *duration);

/**
 * @brief Frees voice group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_voice_group_free(cmp_prop_voice_group_t *group);

/**
 * @brief Initializes speak property.
 * @param prop The property to initialize.
 * @param speak speak value.
 * @param speak_as speak_as string.
 * @return 0 on success.
 */
int cmp_prop_speak_init(cmp_prop_speak_t *prop, cmp_speak_value_t speak,
                        const char *speak_as);

/**
 * @brief Frees speak property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_speak_free(cmp_prop_speak_t *prop);

/**
 * @brief Initializes pause property.
 * @param prop The property to initialize.
 * @param before pause-before string.
 * @param after pause-after string.
 * @return 0 on success.
 */
int cmp_prop_pause_init(cmp_prop_pause_t *prop, const char *before,
                        const char *after);

/**
 * @brief Frees pause property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_pause_free(cmp_prop_pause_t *prop);

/**
 * @brief Initializes rest property.
 * @param prop The property to initialize.
 * @param before rest-before string.
 * @param after rest-after string.
 * @return 0 on success.
 */
int cmp_prop_rest_init(cmp_prop_rest_t *prop, const char *before,
                       const char *after);

/**
 * @brief Frees rest property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_rest_free(cmp_prop_rest_t *prop);

/**
 * @brief Initializes cue property.
 * @param prop The property to initialize.
 * @param before cue-before string.
 * @param after cue-after string.
 * @return 0 on success.
 */
int cmp_prop_cue_init(cmp_prop_cue_t *prop, const char *before,
                      const char *after);

/**
 * @brief Frees cue property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_cue_free(cmp_prop_cue_t *prop);

/**
 * @brief Initializes play-during property.
 * @param prop The property to initialize.
 * @param play_during play_during string.
 * @return 0 on success.
 */
int cmp_prop_play_during_init(cmp_prop_play_during_t *prop,
                              const char *play_during);

/**
 * @brief Frees play-during property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_play_during_free(cmp_prop_play_during_t *prop);

/**
 * @brief Initializes @media speech rule.
 * @param rule The rule to initialize.
 * @return 0 on success.
 */
int cmp_at_rule_media_speech_init(cmp_at_rule_media_speech_t *rule);

/**
 * @brief Frees @media speech rule.
 * @param rule The rule to free.
 * @return 0 on success.
 */
int cmp_at_rule_media_speech_free(cmp_at_rule_media_speech_t *rule);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_SPEECH_H */
