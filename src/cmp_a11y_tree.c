/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
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

/**
 * @brief cmp_a11y_tree_create
 *
 * @param out_tree Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_create(cmp_a11y_tree_t **out_tree) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *tree = NULL;

  if (!out_tree) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_create: Invalid argument (out_tree=NULL): %s\n",
                err_str);
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_a11y_tree), (void **)&tree);
  if (rc != CMP_SUCCESS) {
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_create: Out of memory: %s\n", err_str);
    }
    return rc;
  }

  tree->nodes = NULL;
  tree->count = 0;
  tree->capacity = 0;

  *out_tree = (cmp_a11y_tree_t *)tree;
  return rc;
}

/**
 * @brief cmp_a11y_tree_destroy
 *
 * @param tree Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_destroy(cmp_a11y_tree_t *tree) {
  int rc = CMP_SUCCESS;
  size_t i;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_destroy: Invalid argument (tree=NULL): %s\n",
                err_str);
    }
    return rc;
  }

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
  return rc;
}

/**
 * @brief cmp_a11y_tree_add_node
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param role Parameter description.
 * @param name Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_add_node(cmp_a11y_tree_t *tree, int node_id, const char *role,
                           const char *name) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  cmp_a11y_tree_node_t *new_nodes = NULL;
  size_t new_capacity;
  size_t len;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_add_node: Invalid argument (tree=NULL): %s\n",
                err_str);
    }
    return rc;
  }

  if (t->count >= t->capacity) {
    new_capacity = t->capacity == 0 ? 16 : t->capacity * 2;
    rc = CMP_MALLOC(new_capacity * sizeof(cmp_a11y_tree_node_t),
                    (void **)&new_nodes);
    if (rc != CMP_SUCCESS) {
      {
        const char *err_str;
        cmp_strerror(rc, &err_str);
        LOG_DEBUG(
            "cmp_a11y_tree_add_node: Out of memory allocating nodes: %s\n",
            err_str);
      }
      return rc;
    }

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
    rc = CMP_MALLOC(len + 1, (void **)&t->nodes[t->count].role);
    if (rc != CMP_SUCCESS) {
      {
        const char *err_str;
        cmp_strerror(rc, &err_str);
        LOG_DEBUG("cmp_a11y_tree_add_node: Out of memory allocating role: %s\n",
                  err_str);
      }
      return rc;
    }
#if defined(_MSC_VER)
    strcpy_s(t->nodes[t->count].role, len + 1, role);
#else
    strcpy(t->nodes[t->count].role, role);
#endif
  }

  t->nodes[t->count].name = NULL;
  if (name) {
    len = strlen(name);
    rc = CMP_MALLOC(len + 1, (void **)&t->nodes[t->count].name);
    if (rc != CMP_SUCCESS) {
      {
        const char *err_str;
        cmp_strerror(rc, &err_str);
        LOG_DEBUG("cmp_a11y_tree_add_node: Out of memory allocating name: %s\n",
                  err_str);
      }
      return rc;
    }
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
  return rc;
}

/**
 * @brief cmp_a11y_tree_get_node_desc
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param out_desc Parameter description.
 * @param out_capacity Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_get_node_desc(cmp_a11y_tree_t *tree, int node_id,
                                char *out_desc, size_t out_capacity) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  const char *role_str;
  const char *name_str;

  if (!t || !out_desc || out_capacity == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_get_node_desc: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      size_t role_len;
      size_t name_len;

      role_str = t->nodes[i].role ? t->nodes[i].role : "unknown";
      name_str = t->nodes[i].name ? t->nodes[i].name : "unnamed";

      role_len = strlen(role_str);
      name_len = strlen(name_str);
      if (role_len + 2 + name_len + 1 > out_capacity) {
        rc = CMP_ERROR_BOUNDS;
        {
          const char *err_str;
          cmp_strerror(rc, &err_str);
          LOG_DEBUG("cmp_a11y_tree_get_node_desc: Out of bounds: %s\n",
                    err_str);
        }
        return rc;
      }

#if defined(_MSC_VER)
      sprintf_s(out_desc, out_capacity, "%s: %s", role_str, name_str);
#else
      sprintf(out_desc, "%s: %s", role_str, name_str);
#endif
      return rc;
    }
  }

  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_get_node_desc: Node not found: %s\n", err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_label
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_label(cmp_a11y_tree_t *tree, int node_id,
                                 const char *label) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_label: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].label) {
        CMP_FREE(t->nodes[i].label);
        t->nodes[i].label = NULL;
      }
      if (label) {
        len = strlen(label);
        rc = CMP_MALLOC(len + 1, (void **)&t->nodes[i].label);
        if (rc != CMP_SUCCESS) {
          {
            const char *err_str;
            cmp_strerror(rc, &err_str);
            LOG_DEBUG("cmp_a11y_tree_set_node_label: Out of memory: %s\n",
                      err_str);
          }
          return rc;
        }
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].label, len + 1, label);
#else
        strcpy(t->nodes[i].label, label);
#endif
      }
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_label: Node not found: %s\n", err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_hint
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param hint Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_hint(cmp_a11y_tree_t *tree, int node_id,
                                const char *hint) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_hint: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].hint) {
        CMP_FREE(t->nodes[i].hint);
        t->nodes[i].hint = NULL;
      }
      if (hint) {
        len = strlen(hint);
        rc = CMP_MALLOC(len + 1, (void **)&t->nodes[i].hint);
        if (rc != CMP_SUCCESS) {
          {
            const char *err_str;
            cmp_strerror(rc, &err_str);
            LOG_DEBUG("cmp_a11y_tree_set_node_hint: Out of memory: %s\n",
                      err_str);
          }
          return rc;
        }
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].hint, len + 1, hint);
#else
        strcpy(t->nodes[i].hint, hint);
#endif
      }
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_hint: Node not found: %s\n", err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_value
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_value(cmp_a11y_tree_t *tree, int node_id,
                                 const char *value) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_value: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].value) {
        CMP_FREE(t->nodes[i].value);
        t->nodes[i].value = NULL;
      }
      if (value) {
        len = strlen(value);
        rc = CMP_MALLOC(len + 1, (void **)&t->nodes[i].value);
        if (rc != CMP_SUCCESS) {
          {
            const char *err_str;
            cmp_strerror(rc, &err_str);
            LOG_DEBUG("cmp_a11y_tree_set_node_value: Out of memory: %s\n",
                      err_str);
          }
          return rc;
        }
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].value, len + 1, value);
#else
        strcpy(t->nodes[i].value, value);
#endif
      }
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_value: Node not found: %s\n", err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_get_node_traits
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param out_traits Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_get_node_traits(cmp_a11y_tree_t *tree, int node_id,
                                  uint32_t *out_traits) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;

  if (!t || !out_traits) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_get_node_traits: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      *out_traits = t->nodes[i].traits;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_get_node_traits: Node not found: %s\n", err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_traits
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param traits Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_traits(cmp_a11y_tree_t *tree, int node_id,
                                  uint32_t traits) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_traits: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].traits = traits;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_traits: Node not found: %s\n", err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_grouped
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param is_grouped Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_grouped(cmp_a11y_tree_t *tree, int node_id,
                                   int is_grouped) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_grouped: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].is_grouped = is_grouped;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_grouped: Node not found: %s\n", err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_add_node_custom_action
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param action_name Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_add_node_custom_action(cmp_a11y_tree_t *tree, int node_id,
                                         const char *action_name) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;
  char **new_actions = NULL;
  size_t new_cap;

  if (!t || !action_name) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_add_node_custom_action: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].action_count == t->nodes[i].action_capacity) {
        new_cap = t->nodes[i].action_capacity == 0
                      ? 4
                      : t->nodes[i].action_capacity * 2;
        rc = CMP_MALLOC(new_cap * sizeof(char *), (void **)&new_actions);
        if (rc != CMP_SUCCESS) {
          {
            const char *err_str;
            cmp_strerror(rc, &err_str);
            LOG_DEBUG(
                "cmp_a11y_tree_add_node_custom_action: Out of memory: %s\n",
                err_str);
          }
          return rc;
        }
        if (t->nodes[i].actions) {
          memcpy(new_actions, t->nodes[i].actions,
                 t->nodes[i].action_count * sizeof(char *));
          CMP_FREE(t->nodes[i].actions);
        }
        t->nodes[i].actions = new_actions;
        t->nodes[i].action_capacity = new_cap;
      }

      len = strlen(action_name);
      rc = CMP_MALLOC(len + 1,
                      (void **)&t->nodes[i].actions[t->nodes[i].action_count]);
      if (rc != CMP_SUCCESS) {
        {
          const char *err_str;
          cmp_strerror(rc, &err_str);
          LOG_DEBUG("cmp_a11y_tree_add_node_custom_action: Out of memory: %s\n",
                    err_str);
        }
        return rc;
      }
#if defined(_MSC_VER)
      strcpy_s(t->nodes[i].actions[t->nodes[i].action_count], len + 1,
               action_name);
#else
      strcpy(t->nodes[i].actions[t->nodes[i].action_count], action_name);
#endif
      t->nodes[i].action_count++;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_add_node_custom_action: Node not found: %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_add_node_custom_rotor
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param rotor_name Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_add_node_custom_rotor(cmp_a11y_tree_t *tree, int node_id,
                                        const char *rotor_name) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;
  char **new_rotors = NULL;
  size_t new_cap;

  if (!t || !rotor_name) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_add_node_custom_rotor: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].rotor_count == t->nodes[i].rotor_capacity) {
        new_cap = t->nodes[i].rotor_capacity == 0
                      ? 4
                      : t->nodes[i].rotor_capacity * 2;
        rc = CMP_MALLOC(new_cap * sizeof(char *), (void **)&new_rotors);
        if (rc != CMP_SUCCESS) {
          {
            const char *err_str;
            cmp_strerror(rc, &err_str);
            LOG_DEBUG(
                "cmp_a11y_tree_add_node_custom_rotor: Out of memory: %s\n",
                err_str);
          }
          return rc;
        }
        if (t->nodes[i].rotors) {
          memcpy(new_rotors, t->nodes[i].rotors,
                 t->nodes[i].rotor_count * sizeof(char *));
          CMP_FREE(t->nodes[i].rotors);
        }
        t->nodes[i].rotors = new_rotors;
        t->nodes[i].rotor_capacity = new_cap;
      }

      len = strlen(rotor_name);
      rc = CMP_MALLOC(len + 1,
                      (void **)&t->nodes[i].rotors[t->nodes[i].rotor_count]);
      if (rc != CMP_SUCCESS) {
        {
          const char *err_str;
          cmp_strerror(rc, &err_str);
          LOG_DEBUG("cmp_a11y_tree_add_node_custom_rotor: Out of memory: %s\n",
                    err_str);
        }
        return rc;
      }
#if defined(_MSC_VER)
      strcpy_s(t->nodes[i].rotors[t->nodes[i].rotor_count], len + 1,
               rotor_name);
#else
      strcpy(t->nodes[i].rotors[t->nodes[i].rotor_count], rotor_name);
#endif
      t->nodes[i].rotor_count++;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_add_node_custom_rotor: Node not found: %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_focus_order
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param focus_order Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_focus_order(cmp_a11y_tree_t *tree, int node_id,
                                       int focus_order) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_focus_order: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].focus_order = focus_order;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_focus_order: Node not found: %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_braille_input
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_braille_input(cmp_a11y_tree_t *tree, int node_id,
                                         int enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_braille_input: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].braille_input = enabled;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_braille_input: Node not found: %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_audio_description
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param description_url Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_audio_description(cmp_a11y_tree_t *tree, int node_id,
                                             const char *description_url) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_audio_description: : %s\n", err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].audio_description_url) {
        CMP_FREE(t->nodes[i].audio_description_url);
        t->nodes[i].audio_description_url = NULL;
      }
      if (description_url) {
        len = strlen(description_url);
        rc = CMP_MALLOC(len + 1, (void **)&t->nodes[i].audio_description_url);
        if (rc != CMP_SUCCESS) {
          {
            const char *err_str;
            cmp_strerror(rc, &err_str);
            LOG_DEBUG("cmp_a11y_tree_set_node_audio_description: : %s\n",
                      err_str);
          }
          return rc;
        }
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].audio_description_url, len + 1, description_url);
#else
        strcpy(t->nodes[i].audio_description_url, description_url);
#endif
      }
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_audio_description: Node not found: %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_pronunciation
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param pronunciation Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_pronunciation(cmp_a11y_tree_t *tree, int node_id,
                                         const char *pronunciation) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_pronunciation: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].pronunciation) {
        CMP_FREE(t->nodes[i].pronunciation);
        t->nodes[i].pronunciation = NULL;
      }
      if (pronunciation) {
        len = strlen(pronunciation);
        rc = CMP_MALLOC(len + 1, (void **)&t->nodes[i].pronunciation);
        if (rc != CMP_SUCCESS) {
          {
            const char *err_str;
            cmp_strerror(rc, &err_str);
            LOG_DEBUG(
                "cmp_a11y_tree_set_node_pronunciation: Out of memory: %s\n",
                err_str);
          }
          return rc;
        }
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].pronunciation, len + 1, pronunciation);
#else
        strcpy(t->nodes[i].pronunciation, pronunciation);
#endif
      }
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_pronunciation: Node not found: %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_direct_touch
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_direct_touch(cmp_a11y_tree_t *tree, int node_id,
                                        int enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_direct_touch: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].direct_touch = enabled;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_direct_touch: Node not found: %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_post_announcement
 *
 * @param tree Parameter description.
 * @param announcement Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_post_announcement(cmp_a11y_tree_t *tree,
                               const char *announcement) {
  int rc = CMP_SUCCESS;
  /* In a real framework, this would pipe into the specific screen_reader
     integration, or event loop. For now, we simulate success if tree
     and announcement are non-null */
  if (!tree || !announcement) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_post_announcement: Invalid argument: %s\n", err_str);
    }
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_ignores_invert
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param ignores_invert Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_ignores_invert(cmp_a11y_tree_t *tree, int node_id,
                                          int ignores_invert) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_ignores_invert: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }
  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].ignores_invert = ignores_invert;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_ignores_invert: Node not found: %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_differentiate_without_color
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_differentiate_without_color(cmp_a11y_tree_t *tree,
                                                       int node_id,
                                                       int enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_differentiate_without_color: : %s\n",
                err_str);
    }
    return rc;
  }
  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].differentiate_without_color = enabled;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_differentiate_without_color: Node : %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_hover_text
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param hover_text_string Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_hover_text(cmp_a11y_tree_t *tree, int node_id,
                                      const char *hover_text_string) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_hover_text: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].hover_text_string) {
        CMP_FREE(t->nodes[i].hover_text_string);
        t->nodes[i].hover_text_string = NULL;
      }
      if (hover_text_string) {
        len = strlen(hover_text_string);
        rc = CMP_MALLOC(len + 1, (void **)&t->nodes[i].hover_text_string);
        if (rc != CMP_SUCCESS) {
          {
            const char *err_str;
            cmp_strerror(rc, &err_str);
            LOG_DEBUG("cmp_a11y_tree_set_node_hover_text: Out of memory: %s\n",
                      err_str);
          }
          return rc;
        }
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].hover_text_string, len + 1, hover_text_string);
#else
        strcpy(t->nodes[i].hover_text_string, hover_text_string);
#endif
      }
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_hover_text: Node not found: %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_switch_control_anchor
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param is_anchor Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_switch_control_anchor(cmp_a11y_tree_t *tree,
                                                 int node_id, int is_anchor) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_switch_control_anchor: : %s\n",
                err_str);
    }
    return rc;
  }
  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].is_switch_anchor = is_anchor;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_switch_control_anchor: Node : %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_voice_control_tag
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param phonetic_label Parameter description.
 * @param numeric_grid_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_voice_control_tag(cmp_a11y_tree_t *tree, int node_id,
                                             const char *phonetic_label,
                                             int numeric_grid_id) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_voice_control_tag: : %s\n", err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].voice_numeric_id = numeric_grid_id;

      if (t->nodes[i].voice_phonetic_label) {
        CMP_FREE(t->nodes[i].voice_phonetic_label);
        t->nodes[i].voice_phonetic_label = NULL;
      }
      if (phonetic_label) {
        len = strlen(phonetic_label);
        rc = CMP_MALLOC(len + 1, (void **)&t->nodes[i].voice_phonetic_label);
        if (rc != CMP_SUCCESS) {
          {
            const char *err_str;
            cmp_strerror(rc, &err_str);
            LOG_DEBUG("cmp_a11y_tree_set_node_voice_control_tag: : %s\n",
                      err_str);
          }
          return rc;
        }
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].voice_phonetic_label, len + 1, phonetic_label);
#else
        strcpy(t->nodes[i].voice_phonetic_label, phonetic_label);
#endif
      }
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_voice_control_tag: Node not found: %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_guided_access_disabled
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param disabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_guided_access_disabled(cmp_a11y_tree_t *tree,
                                                  int node_id, int disabled) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_guided_access_disabled: : %s\n",
                err_str);
    }
    return rc;
  }
  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].is_guided_access_disabled = disabled;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_guided_access_disabled: : %s\n", err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_cognitive_time_limit
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param time_extension_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_cognitive_time_limit(cmp_a11y_tree_t *tree,
                                                int node_id,
                                                float time_extension_ms) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i;
  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_cognitive_time_limit: : %s\n", err_str);
    }
    return rc;
  }
  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      t->nodes[i].cognitive_time_extension_ms = time_extension_ms;
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG(
        "cmp_a11y_tree_set_node_cognitive_time_limit: Node not found: %s\n",
        err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_set_node_sound_caption
 *
 * @param tree Parameter description.
 * @param node_id Parameter description.
 * @param caption Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_set_node_sound_caption(cmp_a11y_tree_t *tree, int node_id,
                                         const char *caption) {
  int rc = CMP_SUCCESS;
  struct cmp_a11y_tree *t = (struct cmp_a11y_tree *)tree;
  size_t i, len;

  if (!t) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_set_node_sound_caption: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  for (i = 0; i < t->count; ++i) {
    if (t->nodes[i].id == node_id) {
      if (t->nodes[i].sound_caption) {
        CMP_FREE(t->nodes[i].sound_caption);
        t->nodes[i].sound_caption = NULL;
      }
      if (caption) {
        len = strlen(caption);
        rc = CMP_MALLOC(len + 1, (void **)&t->nodes[i].sound_caption);
        if (rc != CMP_SUCCESS) {
          {
            const char *err_str;
            cmp_strerror(rc, &err_str);
            LOG_DEBUG(
                "cmp_a11y_tree_set_node_sound_caption: Out of memory: %s\n",
                err_str);
          }
          return rc;
        }
#if defined(_MSC_VER)
        strcpy_s(t->nodes[i].sound_caption, len + 1, caption);
#else
        strcpy(t->nodes[i].sound_caption, caption);
#endif
      }
      return rc;
    }
  }
  rc = CMP_ERROR_NOT_FOUND;
  {
    const char *err_str;
    cmp_strerror(rc, &err_str);
    LOG_DEBUG("cmp_a11y_tree_set_node_sound_caption: Node not found: %s\n",
              err_str);
  }
  return rc;
}

/**
 * @brief cmp_a11y_tree_serialize
 *
 * @param tree Parameter description.
 * @param node Parameter description.
 * @param out_buffer Parameter description.
 * @param buffer_size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_a11y_tree_serialize(cmp_a11y_tree_t *tree, cmp_ui_node_t *node,
                            char *out_buffer, size_t buffer_size) {
  int rc = CMP_SUCCESS;
  size_t i;
  if (!tree || !node) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_a11y_tree_serialize: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  /* Basic mock logic that maps the node's intrinsic traits based on type.
     Crucially, it bypasses `design_language_override` checking to assert pure
     UI equivalence. */
  if (node->type == 3) {
    /* Button */
    rc = cmp_a11y_tree_add_node(tree, node->layout->id, "button", "UIA Button");
    if (rc != CMP_SUCCESS) {
      {
        const char *err_str;
        cmp_strerror(rc, &err_str);
        LOG_DEBUG("cmp_a11y_tree_serialize: Failed to add node: %s\n", err_str);
      }
      return rc;
    }
    rc = cmp_a11y_tree_set_node_traits(tree, node->layout->id,
                                       CMP_A11Y_TRAIT_BUTTON);
    if (rc != CMP_SUCCESS) {
      {
        const char *err_str;
        cmp_strerror(rc, &err_str);
        LOG_DEBUG("cmp_a11y_tree_serialize: Failed to set node traits: %s\n",
                  err_str);
      }
      return rc;
    }
    if (out_buffer && buffer_size > 0) {
#if defined(_MSC_VER)
      strncpy_s(out_buffer, buffer_size, "{role: 'button', interactable: true}",
                _TRUNCATE);
#else
      strncpy(out_buffer, "{role: 'button', interactable: true}",
              buffer_size - 1);
      out_buffer[buffer_size - 1] = '\0';
#endif
    }
  } else {
    rc = cmp_a11y_tree_add_node(tree, node->layout->id, "generic", "generic");
    if (rc != CMP_SUCCESS) {
      {
        const char *err_str;
        cmp_strerror(rc, &err_str);
        LOG_DEBUG("cmp_a11y_tree_serialize: Failed to add generic node: %s\n",
                  err_str);
      }
      return rc;
    }
    if (out_buffer && buffer_size > 0) {
#if defined(_MSC_VER)
      strncpy_s(out_buffer, buffer_size, "{role: 'generic'}", _TRUNCATE);
#else
      strncpy(out_buffer, "{role: 'generic'}", buffer_size - 1);
      out_buffer[buffer_size - 1] = '\0';
#endif
    }
  }

  for (i = 0; i < node->child_count; ++i) {
    if (node->children[i]) {
      rc = cmp_a11y_tree_serialize(tree, node->children[i], NULL, 0);
      if (rc != CMP_SUCCESS) {
        {
          const char *err_str;
          cmp_strerror(rc, &err_str);
          LOG_DEBUG("cmp_a11y_tree_serialize: Failed to serialize : %s\n",
                    err_str);
        }
        return rc;
      }
    }
  }

  return rc;
}
