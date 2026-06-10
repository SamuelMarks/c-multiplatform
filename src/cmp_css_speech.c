/* clang-format off */
#include "cmp_css_speech.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_prop_voice_group_init(cmp_prop_voice_group_t *group, const char *vol,
                              const char **family, const char *rate,
                              const char *pitch, const char *range,
                              const char *stress, const char *duration) {
  size_t count = 0;
  size_t i;
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));

  if (vol) {
    size_t len = strlen(vol);
    group->voice_volume = (char *)malloc(len + 1);
    if (!group->voice_volume)
      goto error;
    memcpy(group->voice_volume, vol, len + 1);
  }

  if (family) {
    while (family[count]) {
      count++;
    }
    group->voice_family = (char **)malloc((count + 1) * sizeof(char *));
    if (!group->voice_family)
      goto error;
    for (i = 0; i < count; i++) {
      size_t len = strlen(family[i]);
      group->voice_family[i] = (char *)malloc(len + 1);
      if (!group->voice_family[i])
        goto error;
      memcpy(group->voice_family[i], family[i], len + 1);
    }
    group->voice_family[count] = NULL;
  }

  if (rate) {
    size_t len = strlen(rate);
    group->voice_rate = (char *)malloc(len + 1);
    if (!group->voice_rate)
      goto error;
    memcpy(group->voice_rate, rate, len + 1);
  }

  if (pitch) {
    size_t len = strlen(pitch);
    group->voice_pitch = (char *)malloc(len + 1);
    if (!group->voice_pitch)
      goto error;
    memcpy(group->voice_pitch, pitch, len + 1);
  }

  if (range) {
    size_t len = strlen(range);
    group->voice_range = (char *)malloc(len + 1);
    if (!group->voice_range)
      goto error;
    memcpy(group->voice_range, range, len + 1);
  }

  if (stress) {
    size_t len = strlen(stress);
    group->voice_stress = (char *)malloc(len + 1);
    if (!group->voice_stress)
      goto error;
    memcpy(group->voice_stress, stress, len + 1);
  }

  if (duration) {
    size_t len = strlen(duration);
    group->voice_duration = (char *)malloc(len + 1);
    if (!group->voice_duration)
      goto error;
    memcpy(group->voice_duration, duration, len + 1);
  }

  return 0;

error:
  (void)cmp_prop_voice_group_free(group);
  return -1;
}

int cmp_prop_voice_group_free(cmp_prop_voice_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->voice_volume) {
    free(group->voice_volume);
    group->voice_volume = NULL;
  }
  if (group->voice_family) {
    size_t i = 0;
    while (group->voice_family[i]) {
      free(group->voice_family[i]);
      i++;
    }
    free(group->voice_family);
    group->voice_family = NULL;
  }
  if (group->voice_rate) {
    free(group->voice_rate);
    group->voice_rate = NULL;
  }
  if (group->voice_pitch) {
    free(group->voice_pitch);
    group->voice_pitch = NULL;
  }
  if (group->voice_range) {
    free(group->voice_range);
    group->voice_range = NULL;
  }
  if (group->voice_stress) {
    free(group->voice_stress);
    group->voice_stress = NULL;
  }
  if (group->voice_duration) {
    free(group->voice_duration);
    group->voice_duration = NULL;
  }
  return 0;
}

int cmp_prop_speak_init(cmp_prop_speak_t *prop, cmp_speak_value_t speak,
                        const char *speak_as) {
  if (!prop) {
    return -1;
  }
  memset(prop, 0, sizeof(*prop));
  prop->speak = speak;
  if (speak_as) {
    size_t len = strlen(speak_as);
    prop->speak_as = (char *)malloc(len + 1);
    if (!prop->speak_as) {
      return -1;
    }
    memcpy(prop->speak_as, speak_as, len + 1);
  }
  return 0;
}

int cmp_prop_speak_free(cmp_prop_speak_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->speak_as) {
    free(prop->speak_as);
    prop->speak_as = NULL;
  }
  return 0;
}

static int alloc_pair(char **dst1, const char *src1, char **dst2,
                      const char *src2) {
  if (src1) {
    size_t len = strlen(src1);
    *dst1 = (char *)malloc(len + 1);
    if (!*dst1)
      return -1;
    memcpy(*dst1, src1, len + 1);
  }
  if (src2) {
    size_t len = strlen(src2);
    *dst2 = (char *)malloc(len + 1);
    if (!*dst2) {
      if (*dst1) {
        free(*dst1);
        *dst1 = NULL;
      }
      return -1;
    }
    memcpy(*dst2, src2, len + 1);
  }
  return 0;
}

int cmp_prop_pause_init(cmp_prop_pause_t *prop, const char *before,
                        const char *after) {
  if (!prop)
    return -1;
  memset(prop, 0, sizeof(*prop));
  return alloc_pair(&prop->pause_before, before, &prop->pause_after, after);
}

int cmp_prop_pause_free(cmp_prop_pause_t *prop) {
  if (!prop)
    return -1;
  if (prop->pause_before) {
    free(prop->pause_before);
    prop->pause_before = NULL;
  }
  if (prop->pause_after) {
    free(prop->pause_after);
    prop->pause_after = NULL;
  }
  return 0;
}

int cmp_prop_rest_init(cmp_prop_rest_t *prop, const char *before,
                       const char *after) {
  if (!prop)
    return -1;
  memset(prop, 0, sizeof(*prop));
  return alloc_pair(&prop->rest_before, before, &prop->rest_after, after);
}

int cmp_prop_rest_free(cmp_prop_rest_t *prop) {
  if (!prop)
    return -1;
  if (prop->rest_before) {
    free(prop->rest_before);
    prop->rest_before = NULL;
  }
  if (prop->rest_after) {
    free(prop->rest_after);
    prop->rest_after = NULL;
  }
  return 0;
}

int cmp_prop_cue_init(cmp_prop_cue_t *prop, const char *before,
                      const char *after) {
  if (!prop)
    return -1;
  memset(prop, 0, sizeof(*prop));
  return alloc_pair(&prop->cue_before, before, &prop->cue_after, after);
}

int cmp_prop_cue_free(cmp_prop_cue_t *prop) {
  if (!prop)
    return -1;
  if (prop->cue_before) {
    free(prop->cue_before);
    prop->cue_before = NULL;
  }
  if (prop->cue_after) {
    free(prop->cue_after);
    prop->cue_after = NULL;
  }
  return 0;
}

int cmp_prop_play_during_init(cmp_prop_play_during_t *prop,
                              const char *play_during) {
  if (!prop)
    return -1;
  memset(prop, 0, sizeof(*prop));
  if (play_during) {
    size_t len = strlen(play_during);
    prop->play_during = (char *)malloc(len + 1);
    if (!prop->play_during)
      return -1;
    memcpy(prop->play_during, play_during, len + 1);
  }
  return 0;
}

int cmp_prop_play_during_free(cmp_prop_play_during_t *prop) {
  if (!prop)
    return -1;
  if (prop->play_during) {
    free(prop->play_during);
    prop->play_during = NULL;
  }
  return 0;
}

int cmp_at_rule_media_speech_init(cmp_at_rule_media_speech_t *rule) {
  if (!rule)
    return -1;
  memset(rule, 0, sizeof(*rule));
  return 0;
}

int cmp_at_rule_media_speech_free(cmp_at_rule_media_speech_t *rule) {
  if (!rule)
    return -1;
  (void)cmp_prop_voice_group_free(&rule->voice);
  (void)cmp_prop_speak_free(&rule->speak);
  (void)cmp_prop_pause_free(&rule->pause);
  (void)cmp_prop_rest_free(&rule->rest);
  (void)cmp_prop_cue_free(&rule->cue);
  (void)cmp_prop_play_during_free(&rule->play_during);
  return 0;
}
