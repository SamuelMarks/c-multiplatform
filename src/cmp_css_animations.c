/* clang-format off */
#include "cmp_css_animations.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int cmp_strdup_safe(const char *src, char **dst) {
  size_t len;
  if (!dst)
    return -1;
  if (!src) {
    *dst = NULL;
    return 0;
  }
  len = strlen(src);
  *dst = (char *)malloc(len + 1);
  if (!*dst)
    return -1;
#if defined(_MSC_VER)
  strcpy_s(*dst, len + 1, src);
#else
  strcpy(*dst, src);
#endif
  return 0;
}

int cmp_prop_transition_group_init(cmp_prop_transition_group_t *group,
                                   const char *property, const char *duration,
                                   const char *timing_function,
                                   const char *delay,
                                   cmp_transition_behavior_t behavior) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_transition_group_t));

  group->behavior = behavior;

  if (property) {
    rc = cmp_strdup_safe(property, &group->property);
    if (rc != 0)
      goto error;
  }
  if (duration) {
    rc = cmp_strdup_safe(duration, &group->duration);
    if (rc != 0)
      goto error;
  }
  if (timing_function) {
    rc = cmp_strdup_safe(timing_function, &group->timing_function);
    if (rc != 0)
      goto error;
  }
  if (delay) {
    rc = cmp_strdup_safe(delay, &group->delay);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_transition_group_free(group);
  return -1;
}

int cmp_prop_transition_group_free(cmp_prop_transition_group_t *group) {
  if (!group)
    return -1;
  if (group->property)
    free(group->property);
  if (group->duration)
    free(group->duration);
  if (group->timing_function)
    free(group->timing_function);
  if (group->delay)
    free(group->delay);

  memset(group, 0, sizeof(cmp_prop_transition_group_t));
  return 0;
}

int cmp_prop_animation_group_init(
    cmp_prop_animation_group_t *group, const char *name, const char *duration,
    const char *timing_function, const char *iteration_count,
    cmp_animation_direction_t direction, cmp_animation_play_state_t play_state,
    const char *delay, cmp_animation_fill_mode_t fill_mode,
    cmp_animation_composition_t composition) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_animation_group_t));

  group->direction = direction;
  group->play_state = play_state;
  group->fill_mode = fill_mode;
  group->composition = composition;

  if (name) {
    rc = cmp_strdup_safe(name, &group->name);
    if (rc != 0)
      goto error;
  }
  if (duration) {
    rc = cmp_strdup_safe(duration, &group->duration);
    if (rc != 0)
      goto error;
  }
  if (timing_function) {
    rc = cmp_strdup_safe(timing_function, &group->timing_function);
    if (rc != 0)
      goto error;
  }
  if (iteration_count) {
    rc = cmp_strdup_safe(iteration_count, &group->iteration_count);
    if (rc != 0)
      goto error;
  }
  if (delay) {
    rc = cmp_strdup_safe(delay, &group->delay);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_animation_group_free(group);
  return -1;
}

int cmp_prop_animation_group_free(cmp_prop_animation_group_t *group) {
  if (!group)
    return -1;
  if (group->name)
    free(group->name);
  if (group->duration)
    free(group->duration);
  if (group->timing_function)
    free(group->timing_function);
  if (group->iteration_count)
    free(group->iteration_count);
  if (group->delay)
    free(group->delay);

  memset(group, 0, sizeof(cmp_prop_animation_group_t));
  return 0;
}

int cmp_at_rule_keyframes_init(cmp_at_rule_keyframes_t *rule, const char *name,
                               const char *content) {
  int rc;
  if (!rule)
    return -1;
  memset(rule, 0, sizeof(cmp_at_rule_keyframes_t));

  if (name) {
    rc = cmp_strdup_safe(name, &rule->name);
    if (rc != 0)
      goto error;
  }
  if (content) {
    rc = cmp_strdup_safe(content, &rule->content);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  cmp_at_rule_keyframes_free(rule);
  return -1;
}

int cmp_at_rule_keyframes_free(cmp_at_rule_keyframes_t *rule) {
  if (!rule)
    return -1;
  if (rule->name)
    free(rule->name);
  if (rule->content)
    free(rule->content);
  memset(rule, 0, sizeof(cmp_at_rule_keyframes_t));
  return 0;
}

int cmp_at_rule_starting_style_init(cmp_at_rule_starting_style_t *rule,
                                    const char *content) {
  int rc;
  if (!rule)
    return -1;
  memset(rule, 0, sizeof(cmp_at_rule_starting_style_t));

  if (content) {
    rc = cmp_strdup_safe(content, &rule->content);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  cmp_at_rule_starting_style_free(rule);
  return -1;
}

int cmp_at_rule_starting_style_free(cmp_at_rule_starting_style_t *rule) {
  if (!rule)
    return -1;
  if (rule->content)
    free(rule->content);
  memset(rule, 0, sizeof(cmp_at_rule_starting_style_t));
  return 0;
}