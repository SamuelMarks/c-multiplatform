/* clang-format off */
#include "cmp_css_lists_content.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_prop_list_style_group_init(cmp_prop_list_style_group_t *group,
                                   cmp_list_style_type_t type,
                                   const char *custom_string,
                                   cmp_list_style_position_t position,
                                   const char *image) {
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));

  group->type = type;
  if (custom_string) {
    size_t len = strlen(custom_string);
    group->custom_string = (char *)malloc(len + 1);
    if (!group->custom_string)
      goto error;
    memcpy(group->custom_string, custom_string, len + 1);
  }

  group->position = position;

  if (image) {
    size_t len = strlen(image);
    group->image = (char *)malloc(len + 1);
    if (!group->image)
      goto error;
    memcpy(group->image, image, len + 1);
  }

  return 0;

error:
  (void)cmp_prop_list_style_group_free(group);
  return -1;
}

int cmp_prop_list_style_group_free(cmp_prop_list_style_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->custom_string) {
    free(group->custom_string);
    group->custom_string = NULL;
  }
  if (group->image) {
    free(group->image);
    group->image = NULL;
  }
  return 0;
}

int cmp_prop_counter_group_init(cmp_prop_counter_group_t *group) {
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));
  return 0;
}

static int cmp_counter_action_add(cmp_counter_action_t **actions,
                                  size_t *num_actions, const char *name,
                                  int value) {
  cmp_counter_action_t *new_actions;
  cmp_counter_action_t *action;
  size_t len;

  if (!actions || !num_actions || !name) {
    return -1;
  }

  new_actions = (cmp_counter_action_t *)realloc(
      *actions, (*num_actions + 1) * sizeof(cmp_counter_action_t));
  if (!new_actions) {
    return -1;
  }
  *actions = new_actions;

  action = &(*actions)[*num_actions];
  len = strlen(name);
  action->counter_name = (char *)malloc(len + 1);
  if (!action->counter_name) {
    return -1;
  }
  memcpy(action->counter_name, name, len + 1);
  action->value = value;

  (*num_actions)++;
  return 0;
}

int cmp_prop_counter_group_add_reset(cmp_prop_counter_group_t *group,
                                     const char *name, int value) {
  if (!group)
    return -1;
  return cmp_counter_action_add(&group->resets, &group->num_resets, name,
                                value);
}

int cmp_prop_counter_group_add_increment(cmp_prop_counter_group_t *group,
                                         const char *name, int value) {
  if (!group)
    return -1;
  return cmp_counter_action_add(&group->increments, &group->num_increments,
                                name, value);
}

int cmp_prop_counter_group_add_set(cmp_prop_counter_group_t *group,
                                   const char *name, int value) {
  if (!group)
    return -1;
  return cmp_counter_action_add(&group->sets, &group->num_sets, name, value);
}

int cmp_prop_counter_group_free(cmp_prop_counter_group_t *group) {
  size_t i;
  if (!group) {
    return -1;
  }
  if (group->resets) {
    for (i = 0; i < group->num_resets; i++) {
      free(group->resets[i].counter_name);
    }
    free(group->resets);
    group->resets = NULL;
    group->num_resets = 0;
  }
  if (group->increments) {
    for (i = 0; i < group->num_increments; i++) {
      free(group->increments[i].counter_name);
    }
    free(group->increments);
    group->increments = NULL;
    group->num_increments = 0;
  }
  if (group->sets) {
    for (i = 0; i < group->num_sets; i++) {
      free(group->sets[i].counter_name);
    }
    free(group->sets);
    group->sets = NULL;
    group->num_sets = 0;
  }
  return 0;
}

int cmp_prop_content_group_init(cmp_prop_content_group_t *group,
                                const char *content, const char *quotes) {
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));

  if (content) {
    size_t len = strlen(content);
    group->content_prop.content = (char *)malloc(len + 1);
    if (!group->content_prop.content)
      goto error;
    memcpy(group->content_prop.content, content, len + 1);
  }

  if (quotes) {
    size_t len = strlen(quotes);
    group->quotes_prop.quotes = (char *)malloc(len + 1);
    if (!group->quotes_prop.quotes)
      goto error;
    memcpy(group->quotes_prop.quotes, quotes, len + 1);
  }

  return 0;

error:
  (void)cmp_prop_content_group_free(group);
  return -1;
}

int cmp_prop_content_group_free(cmp_prop_content_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->content_prop.content) {
    free(group->content_prop.content);
    group->content_prop.content = NULL;
  }
  if (group->quotes_prop.quotes) {
    free(group->quotes_prop.quotes);
    group->quotes_prop.quotes = NULL;
  }
  return 0;
}
