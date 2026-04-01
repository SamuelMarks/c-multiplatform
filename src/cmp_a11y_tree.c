/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

typedef struct cmp_a11y_tree_node {
  int id;
  char *role;
  char *name;
  char *label;
  char *hint;
  char *value;
  uint32_t traits;

  int is_grouped;

  char **actions;
  size_t action_count;
  size_t action_capacity;

  char **rotors;
  size_t rotor_count;
  size_t rotor_capacity;

  int focus_order;

  int braille_input;
  char *audio_description_url;
  char *pronunciation;
  int direct_touch;

  int ignores_invert;
  int differentiate_without_color;
  char *hover_text_string;

  int is_switch_anchor;
  int voice_numeric_id;
  char *voice_phonetic_label;
  int is_guided_access_disabled;
  float cognitive_time_extension_ms;
  char *sound_caption;
} cmp_a11y_tree_node_t;

struct cmp_a11y_tree {
  cmp_a11y_tree_node_t *nodes;
  size_t count;
  size_t capacity;
};

int cmp_a11y_tree_create(cmp_a11y_tree_t **out_tree) {
  struct cmp_a11y_tree *tree;

  if (!out_tree)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_a11y_tree), (void **)&tree) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  tree->nodes = NULL;
  tree->count = 0;
  tree->capacity = 0;

  *out_tree = (cmp_a11y_tree_t *)tree;
  return CMP_SUCCESS;
}

int cmp_a11y_tree_destroy(cmp_a11y_tree_t *tree) {
  size_t i;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  if (t->nodes) {
    for (i = 0; i < t->count; ++i) {
      size_t j;
      if (t->nodes[i].role) {
        CMP_FREE(t->nodes[i].role);
      }
      if (t->nodes[i].name) {
        CMP_FREE(t->nodes[i].name);
      }
      if (t->nodes[i].label) {
        CMP_FREE(t->nodes[i].label);
      }
      if (t->nodes[i].hint) {
        CMP_FREE(t->nodes[i].hint);
      }
      if (t->nodes[i].value) {
        CMP_FREE(t->nodes[i].value);
      }
      if (t->nodes[i].audio_description_url) {
        CMP_FREE(t->nodes[i].audio_description_url);
      }
      if (t->nodes[i].pronunciation) {
        CMP_FREE(t->nodes[i].pronunciation);
      }
      if (t->nodes[i].hover_text_string) {
        CMP_FREE(t->nodes[i].hover_text_string);
      }
      if (t->nodes[i].voice_phonetic_label) {
        CMP_FREE(t->nodes[i].voice_phonetic_label);
      }
      if (t->nodes[i].sound_caption) {
        CMP_FREE(t->nodes[i].sound_caption);
      }
      for (j = 0; j < t->nodes[i].action_count; ++j) {
        if (t->nodes[i].actions[j])
          CMP_FREE(t->nodes[i].actions[j]);
      }
      if (t->nodes[i].actions)
        CMP_FREE(t->nodes[i].actions);

      for (j = 0; j < t->nodes[i].rotor_count; ++j) {
        if (t->nodes[i].rotors[j])
          CMP_FREE(t->nodes[i].rotors[j]);
      }
      if (t->nodes[i].rotors)
        CMP_FREE(t->nodes[i].rotors);
    }
    CMP_FREE(t->nodes);
  }

  CMP_FREE(t);
  return CMP_SUCCESS;
}

int cmp_a11y_tree_add_node(cmp_a11y_tree_t *tree, int node_id, const char *role,
                           const char *name) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  cmp_a11y_tree_node_t *new_nodes;
  size_t new_capacity;
  size_t len;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  if (t->count >= t->capacity) {
    new_capacity = t->capacity == 0 ? 16 : t->capacity * 2;
    if (CMP_MALLOC(new_capacity * sizeof(cmp_a11y_tree_node_t),
                   (void **)&new_nodes) != CMP_SUCCESS)
      return CMP_ERROR_OOM;

    if (t->nodes) {
      memcpy(new_nodes, t->nodes, t->count * sizeof(cmp_a11y_tree_node_t));
      CMP_FREE(t->nodes);
    }
    t->nodes = new_nodes;
    t->capacity = new_capacity;
  }

  t->nodes[t->count].id = node_id;

  t->nodes[t->count].role = NULL;
  if (role) {
    len = strlen(role);
    if (CMP_MALLOC(len + 1, (void **)&t->nodes[t->count].role) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
#if defined(_MSC_VER)
    strcpy_s(t->nodes[t->count].role, len + 1, role);
#else
    strcpy(t->nodes[t->count].role, role);
#endif
  }

  t->nodes[t->count].name = NULL;
  if (name) {
    len = strlen(name);
    if (CMP_MALLOC(len + 1, (void **)&t->nodes[t->count].name) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
#if defined(_MSC_VER)
    strcpy_s(t->nodes[t->count].name, len + 1, name);
#else
    strcpy(t->nodes[t->count].name, name);
#endif
  }

  t->nodes[t->count].label = NULL;
  t->nodes[t->count].hint = NULL;
  t->nodes[t->count].value = NULL;
  t->nodes[t->count].traits = 0;

  t->nodes[t->count].is_grouped = 0;
  t->nodes[t->count].actions = NULL;
  t->nodes[t->count].action_count = 0;
  t->nodes[t->count].action_capacity = 0;

  t->nodes[t->count].rotors = NULL;
  t->nodes[t->count].rotor_count = 0;
  t->nodes[t->count].rotor_capacity = 0;

  t->nodes[t->count].focus_order = 0;
  t->nodes[t->count].braille_input = 0;
  t->nodes[t->count].audio_description_url = NULL;
  t->nodes[t->count].pronunciation = NULL;
  t->nodes[t->count].direct_touch = 0;

  t->nodes[t->count].ignores_invert = 0;
  t->nodes[t->count].differentiate_without_color = 0;
  t->nodes[t->count].hover_text_string = NULL;

  t->nodes[t->count].is_switch_anchor = 0;
  t->nodes[t->count].voice_numeric_id = 0;
  t->nodes[t->count].voice_phonetic_label = NULL;
  t->nodes[t->count].is_guided_access_disabled = 0;
  t->nodes[t->count].cognitive_time_extension_ms = 0.0f;
  t->nodes[t->count].sound_caption = NULL;

  t->count++;
  return CMP_SUCCESS;
}

int cmp_a11y_tree_get_node_desc(cmp_a11y_tree_t *tree, int node_id,
                                char *out_desc, size_t out_capacity) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  const char *role_str;
  const char *name_str;

  if (!t || !out_desc || out_capacity == 0)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      size_t role_len;
      size_t name_len;

      role_str = t->nodes[i].role ? t->nodes[i].role : "unknown";
      name_str = t->nodes[i].name ? t->nodes[i].name : "unnamed";

      role_len = strlen(role_str);
      name_len = strlen(name_str);
      if (role_len + 2 + name_len + 1 > out_capacity) {
        return CMP_ERROR_BOUNDS;
      }

#if defined(_MSC_VER)
      sprintf_s(out_desc, out_capacity, "%s: %s", role_str, name_str);
#else
      sprintf(out_desc, "%s: %s", role_str, name_str);
#endif
      return CMP_SUCCESS;
    }
  }

  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_label(cmp_a11y_tree_t *tree, int node_id,
                                 const char *label) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].label) {
        CMP_FREE(t->nodes[i].label);
        t->nodes[i].label = NULL;
      }
      if (label) {
        len = strlen(label);
        if (CMP_MALLOC(len + 1, (void **)&t->nodes[i].label) != CMP_SUCCESS)
          return CMP_ERROR_OOM;
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].label, len + 1, label);
#else
        strcpy(t->nodes[i].label, label);
#endif
      }
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_hint(cmp_a11y_tree_t *tree, int node_id,
                                const char *hint) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].hint) {
        CMP_FREE(t->nodes[i].hint);
        t->nodes[i].hint = NULL;
      }
      if (hint) {
        len = strlen(hint);
        if (CMP_MALLOC(len + 1, (void **)&t->nodes[i].hint) != CMP_SUCCESS)
          return CMP_ERROR_OOM;
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].hint, len + 1, hint);
#else
        strcpy(t->nodes[i].hint, hint);
#endif
      }
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_value(cmp_a11y_tree_t *tree, int node_id,
                                 const char *value) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].value) {
        CMP_FREE(t->nodes[i].value);
        t->nodes[i].value = NULL;
      }
      if (value) {
        len = strlen(value);
        if (CMP_MALLOC(len + 1, (void **)&t->nodes[i].value) != CMP_SUCCESS)
          return CMP_ERROR_OOM;
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].value, len + 1, value);
#else
        strcpy(t->nodes[i].value, value);
#endif
      }
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_traits(cmp_a11y_tree_t *tree, int node_id,
                                  uint32_t traits) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].traits = traits;
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_grouped(cmp_a11y_tree_t *tree, int node_id,
                                   int is_grouped) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].is_grouped = is_grouped;
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_add_node_custom_action(cmp_a11y_tree_t *tree, int node_id,
                                         const char *action_name) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;
  char **new_actions;
  size_t new_cap;

  if (!t || !action_name)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].action_count == t->nodes[i].action_capacity) {
        new_cap = t->nodes[i].action_capacity == 0
                      ? 4
                      : t->nodes[i].action_capacity * 2;
        if (CMP_MALLOC(new_cap * sizeof(char *), (void **)&new_actions) !=
            CMP_SUCCESS)
          return CMP_ERROR_OOM;
        if (t->nodes[i].actions) {
          memcpy(new_actions, t->nodes[i].actions,
                 t->nodes[i].action_count * sizeof(char *));
          CMP_FREE(t->nodes[i].actions);
        }
        t->nodes[i].actions = new_actions;
        t->nodes[i].action_capacity = new_cap;
      }

      len = strlen(action_name);
      if (CMP_MALLOC(len + 1,
                     (void **)&t->nodes[i].actions[t->nodes[i].action_count]) !=
          CMP_SUCCESS)
        return CMP_ERROR_OOM;
#if defined(_MSC_VER)
      strcpy_s(t->nodes[i].actions[t->nodes[i].action_count], len + 1,
               action_name);
#else
      strcpy(t->nodes[i].actions[t->nodes[i].action_count], action_name);
#endif
      t->nodes[i].action_count++;
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_add_node_custom_rotor(cmp_a11y_tree_t *tree, int node_id,
                                        const char *rotor_name) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;
  char **new_rotors;
  size_t new_cap;

  if (!t || !rotor_name)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].rotor_count == t->nodes[i].rotor_capacity) {
        new_cap = t->nodes[i].rotor_capacity == 0
                      ? 4
                      : t->nodes[i].rotor_capacity * 2;
        if (CMP_MALLOC(new_cap * sizeof(char *), (void **)&new_rotors) !=
            CMP_SUCCESS)
          return CMP_ERROR_OOM;
        if (t->nodes[i].rotors) {
          memcpy(new_rotors, t->nodes[i].rotors,
                 t->nodes[i].rotor_count * sizeof(char *));
          CMP_FREE(t->nodes[i].rotors);
        }
        t->nodes[i].rotors = new_rotors;
        t->nodes[i].rotor_capacity = new_cap;
      }

      len = strlen(rotor_name);
      if (CMP_MALLOC(len + 1,
                     (void **)&t->nodes[i].rotors[t->nodes[i].rotor_count]) !=
          CMP_SUCCESS)
        return CMP_ERROR_OOM;
#if defined(_MSC_VER)
      strcpy_s(t->nodes[i].rotors[t->nodes[i].rotor_count], len + 1,
               rotor_name);
#else
      strcpy(t->nodes[i].rotors[t->nodes[i].rotor_count], rotor_name);
#endif
      t->nodes[i].rotor_count++;
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_focus_order(cmp_a11y_tree_t *tree, int node_id,
                                       int focus_order) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].focus_order = focus_order;
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_braille_input(cmp_a11y_tree_t *tree, int node_id,
                                         int enabled) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].braille_input = enabled;
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_audio_description(cmp_a11y_tree_t *tree, int node_id,
                                             const char *description_url) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].audio_description_url) {
        CMP_FREE(t->nodes[i].audio_description_url);
        t->nodes[i].audio_description_url = NULL;
      }
      if (description_url) {
        len = strlen(description_url);
        if (CMP_MALLOC(len + 1, (void **)&t->nodes[i].audio_description_url) !=
            CMP_SUCCESS)
          return CMP_ERROR_OOM;
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].audio_description_url, len + 1, description_url);
#else
        strcpy(t->nodes[i].audio_description_url, description_url);
#endif
      }
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_pronunciation(cmp_a11y_tree_t *tree, int node_id,
                                         const char *pronunciation) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].pronunciation) {
        CMP_FREE(t->nodes[i].pronunciation);
        t->nodes[i].pronunciation = NULL;
      }
      if (pronunciation) {
        len = strlen(pronunciation);
        if (CMP_MALLOC(len + 1, (void **)&t->nodes[i].pronunciation) !=
            CMP_SUCCESS)
          return CMP_ERROR_OOM;
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].pronunciation, len + 1, pronunciation);
#else
        strcpy(t->nodes[i].pronunciation, pronunciation);
#endif
      }
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_direct_touch(cmp_a11y_tree_t *tree, int node_id,
                                        int enabled) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].direct_touch = enabled;
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_post_announcement(cmp_a11y_tree_t *tree,
                               const char *announcement) {
  /* In a real framework, this would pipe into the specific screen_reader
     integration, or event loop. For now, we simulate success if tree
     and announcement are non-null */
  if (!tree || !announcement)
    return CMP_ERROR_INVALID_ARG;
  return CMP_SUCCESS;
}

int cmp_a11y_tree_set_node_ignores_invert(cmp_a11y_tree_t *tree, int node_id,
                                          int ignores_invert) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  if (!t)
    return CMP_ERROR_INVALID_ARG;
  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].ignores_invert = ignores_invert;
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_differentiate_without_color(cmp_a11y_tree_t *tree,
                                                       int node_id,
                                                       int enabled) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  if (!t)
    return CMP_ERROR_INVALID_ARG;
  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].differentiate_without_color = enabled;
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_hover_text(cmp_a11y_tree_t *tree, int node_id,
                                      const char *hover_text_string) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].hover_text_string) {
        CMP_FREE(t->nodes[i].hover_text_string);
        t->nodes[i].hover_text_string = NULL;
      }
      if (hover_text_string) {
        len = strlen(hover_text_string);
        if (CMP_MALLOC(len + 1, (void **)&t->nodes[i].hover_text_string) !=
            CMP_SUCCESS)
          return CMP_ERROR_OOM;
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].hover_text_string, len + 1, hover_text_string);
#else
        strcpy(t->nodes[i].hover_text_string, hover_text_string);
#endif
      }
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_switch_control_anchor(cmp_a11y_tree_t *tree,
                                                 int node_id, int is_anchor) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  if (!t)
    return CMP_ERROR_INVALID_ARG;
  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].is_switch_anchor = is_anchor;
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_voice_control_tag(cmp_a11y_tree_t *tree, int node_id,
                                             const char *phonetic_label,
                                             int numeric_grid_id) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].voice_numeric_id = numeric_grid_id;

      if (t->nodes[i].voice_phonetic_label) {
        CMP_FREE(t->nodes[i].voice_phonetic_label);
        t->nodes[i].voice_phonetic_label = NULL;
      }
      if (phonetic_label) {
        len = strlen(phonetic_label);
        if (CMP_MALLOC(len + 1, (void **)&t->nodes[i].voice_phonetic_label) !=
            CMP_SUCCESS)
          return CMP_ERROR_OOM;
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].voice_phonetic_label, len + 1, phonetic_label);
#else
        strcpy(t->nodes[i].voice_phonetic_label, phonetic_label);
#endif
      }
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_guided_access_disabled(cmp_a11y_tree_t *tree,
                                                  int node_id, int disabled) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  if (!t)
    return CMP_ERROR_INVALID_ARG;
  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].is_guided_access_disabled = disabled;
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_cognitive_time_limit(cmp_a11y_tree_t *tree,
                                                int node_id,
                                                float time_extension_ms) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  if (!t)
    return CMP_ERROR_INVALID_ARG;
  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].cognitive_time_extension_ms = time_extension_ms;
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_a11y_tree_set_node_sound_caption(cmp_a11y_tree_t *tree, int node_id,
                                         const char *caption) {
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].sound_caption) {
        CMP_FREE(t->nodes[i].sound_caption);
        t->nodes[i].sound_caption = NULL;
      }
      if (caption) {
        len = strlen(caption);
        if (CMP_MALLOC(len + 1, (void **)&t->nodes[i].sound_caption) !=
            CMP_SUCCESS)
          return CMP_ERROR_OOM;
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].sound_caption, len + 1, caption);
#else
        strcpy(t->nodes[i].sound_caption, caption);
#endif
      }
      return CMP_SUCCESS;
    }
  }
  return CMP_ERROR_NOT_FOUND;
}
