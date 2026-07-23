/* clang-format off */
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include "ui_mutation_observer.h"
#include "ui_web_bridge.h"
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* Use MSVC Safe CRT internally, string.h is already included. */
#endif

static enum ui_error internal_strdup(const char *src, char **out_str) {
  size_t len;
  char *copy;

  len = strlen(src);
  copy = (char *)UI_MALLOC(len + 1);
  if (!copy) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  strcpy_s(copy, len + 1, src);
#else
  strcpy(copy, src);
#endif

  *out_str = copy;
  return UI_ERROR_NONE;
}

enum ui_error ui_dom_node_create(enum ui_dom_node_type type,
                                 struct ui_dom_node **out_node) {
  struct ui_dom_node *node;

  if (!out_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  node = (struct ui_dom_node *)UI_MALLOC(sizeof(struct ui_dom_node));
  if (!node) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  node->type = type;
  node->tag_name = NULL;
  node->text_content = NULL;
  node->attributes = NULL;
  node->listeners = NULL;
  node->state_flags = 0;
  node->parent = NULL;
  node->first_child = NULL;
  node->last_child = NULL;
  node->previous_sibling = NULL;
  node->next_sibling = NULL;

  *out_node = node;
  /* Phase 3 Web Bridge Hook */
#if defined(__EMSCRIPTEN__)
  {
    const char *tag = "div";
    if (type == UI_DOM_NODE_TYPE_TEXT)
      tag = "span";
    ui_web_bridge_create_node((uint32_t)(uintptr_t)node, tag);
  }
#endif
  return UI_ERROR_NONE;
}

void ui_dom_node_destroy(struct ui_dom_node *node) {
  struct ui_dom_node *child;
  struct ui_dom_node *next_child;
  struct ui_dom_attribute *attr;
  struct ui_dom_attribute *next_attr;
  struct ui_dom_event_listener *listener, *next_listener;

  if (!node) {
    return;
  }

  /* Recursively destroy children */
  child = node->first_child;
  while (child) {
    next_child = child->next_sibling;
    ui_dom_node_destroy(child);
    child = next_child;
  }

  /* Free attributes */
  attr = node->attributes;

  listener = node->listeners;
  while (listener) {
    next_listener = listener->next;
    UI_FREE(listener);
    listener = next_listener;
  }
  while (attr) {
    next_attr = attr->next;
    UI_FREE(attr->name);
    UI_FREE(attr->value);
    UI_FREE(attr);
    attr = next_attr;
  }

  /* Free strings */
  UI_FREE(node->tag_name);
  UI_FREE(node->text_content);

  /* Phase 3 Web Bridge Hook */
#if defined(__EMSCRIPTEN__)
  ui_web_bridge_destroy_node((uint32_t)(uintptr_t)node);
#endif

  UI_FREE(node);
}

enum ui_error ui_dom_node_append_child(struct ui_dom_node *parent,
                                       struct ui_dom_node *child) {
  if (!parent || !child) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* If child already has a parent, standard DOM would remove it first.
     For this minimal implementation, we enforce it must be detached. */
  if (child->parent) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  child->parent = parent;

  if (!parent->last_child) {
    /* Parent has no children */
    parent->first_child = child;
    parent->last_child = child;
    child->previous_sibling = NULL;
    child->next_sibling = NULL;
  } else {
    /* Append to the end */
    parent->last_child->next_sibling = child;
    child->previous_sibling = parent->last_child;
    child->next_sibling = NULL;
    parent->last_child = child;
  }

  ui_mutation_observer_notify_child_list(parent, child, NULL);

  return UI_ERROR_NONE;
}

enum ui_error ui_dom_node_remove_child(struct ui_dom_node *parent,
                                       struct ui_dom_node *child) {
  if (!parent || !child || child->parent != parent) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (child->previous_sibling) {
    child->previous_sibling->next_sibling = child->next_sibling;
  } else {
    parent->first_child = child->next_sibling;
  }

  if (child->next_sibling) {
    child->next_sibling->previous_sibling = child->previous_sibling;
  } else {
    parent->last_child = child->previous_sibling;
  }

  child->parent = NULL;
  child->previous_sibling = NULL;
  child->next_sibling = NULL;

  ui_mutation_observer_notify_child_list(parent, NULL, child);

  return UI_ERROR_NONE;
}

enum ui_error ui_dom_node_set_attribute(struct ui_dom_node *node,
                                        const char *name, const char *value) {
  struct ui_dom_attribute *attr;
  struct ui_dom_attribute *new_attr;
  char *name_copy;
  char *value_copy;
  enum ui_error err;

  if (!node || !name || !value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (node->type != UI_DOM_NODE_TYPE_ELEMENT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  name_copy = NULL;
  value_copy = NULL;
  new_attr = NULL;

  /* Check if attribute already exists */
  attr = node->attributes;
  while (attr) {
    if (strcmp(attr->name, name) == 0) {
      char *old_val_ptr = attr->value;
      /* Update existing attribute */
      err = internal_strdup(value, &value_copy);
      if (err != UI_ERROR_NONE) {
        goto cleanup;
      }
      attr->value = value_copy;

#if defined(__EMSCRIPTEN__)
      ui_web_bridge_set_attribute((uint32_t)(uintptr_t)node, name, value);
#endif

      ui_mutation_observer_notify_attribute(node, name, old_val_ptr);
      UI_FREE(old_val_ptr);
      return UI_ERROR_NONE;
    }
    attr = attr->next;
  }

  /* Create new attribute */
  err = internal_strdup(name, &name_copy);
  if (err != UI_ERROR_NONE) {
    goto cleanup;
  }

  err = internal_strdup(value, &value_copy);
  if (err != UI_ERROR_NONE) {
    goto cleanup;
  }

  new_attr =
      (struct ui_dom_attribute *)UI_MALLOC(sizeof(struct ui_dom_attribute));
  if (!new_attr) {
    err = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  new_attr->name = name_copy;
  new_attr->value = value_copy;
  new_attr->next = node->attributes;
  node->attributes = new_attr;

#if defined(__EMSCRIPTEN__)
  ui_web_bridge_set_attribute((uint32_t)(uintptr_t)node, name, value);
#endif

  ui_mutation_observer_notify_attribute(node, name, NULL);

  return UI_ERROR_NONE;

cleanup:
  UI_FREE(name_copy);
  UI_FREE(value_copy);
  UI_FREE(new_attr);
  return err;
}

enum ui_error ui_dom_node_get_attribute(const struct ui_dom_node *node,
                                        const char *name,
                                        const char **out_value) {
  struct ui_dom_attribute *attr;

  if (!node || !name || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (node->type != UI_DOM_NODE_TYPE_ELEMENT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  attr = node->attributes;
  while (attr) {
    if (strcmp(attr->name, name) == 0) {
      *out_value = attr->value;
      return UI_ERROR_NONE;
    }
    attr = attr->next;
  }

  return UI_ERROR_NOT_FOUND;
}

enum ui_error ui_dom_node_remove_attribute(struct ui_dom_node *node,
                                           const char *name) {
  struct ui_dom_attribute *attr;
  struct ui_dom_attribute *prev = NULL;

  if (!node || !name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (node->type != UI_DOM_NODE_TYPE_ELEMENT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  attr = node->attributes;
  while (attr) {
    if (strcmp(attr->name, name) == 0) {
      char *old_val_ptr = attr->value;
      if (prev) {
        prev->next = attr->next;
      } else {
        node->attributes = attr->next;
      }

#if defined(__EMSCRIPTEN__)
      ui_web_bridge_set_attribute((uint32_t)(uintptr_t)node, name, NULL);
#endif

      ui_mutation_observer_notify_attribute(node, name, old_val_ptr);
      UI_FREE(attr->name);
      UI_FREE(old_val_ptr);
      UI_FREE(attr);
      return UI_ERROR_NONE;
    }
    prev = attr;
    attr = attr->next;
  }

  return UI_ERROR_NONE; /* Not found, but not an error to remove it */
}

enum ui_error ui_dom_node_set_tag_name(struct ui_dom_node *node,
                                       const char *tag_name) {
  char *tag_copy;
  enum ui_error err;

  if (!node || !tag_name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (node->type != UI_DOM_NODE_TYPE_ELEMENT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = internal_strdup(tag_name, &tag_copy);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  UI_FREE(node->tag_name);

  node->tag_name = tag_copy;
  return UI_ERROR_NONE;
}

enum ui_error ui_dom_node_set_text_content(struct ui_dom_node *node,
                                           const char *text) {
  char *text_copy;
  char *old_val_ptr = NULL;
  enum ui_error err;

  if (!node || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (node->type != UI_DOM_NODE_TYPE_TEXT &&
      node->type != UI_DOM_NODE_TYPE_COMMENT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = internal_strdup(text, &text_copy);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  old_val_ptr = node->text_content;

  node->text_content = text_copy;

  ui_mutation_observer_notify_character_data(node, old_val_ptr);

  if (old_val_ptr) {
    UI_FREE(old_val_ptr);
  }

#if defined(__EMSCRIPTEN__)
  ui_web_bridge_set_text((uint32_t)(uintptr_t)node, node->text_content);
#endif

  return UI_ERROR_NONE;
}

enum ui_error ui_dom_node_add_event_listener(struct ui_dom_node *node,
                                             enum ui_event_type type,
                                             ui_event_handler_t handler,
                                             void *user_data) {
  struct ui_dom_event_listener *listener;
  if (!node || !handler)
    return UI_ERROR_INVALID_ARGUMENT;
  listener = UI_MALLOC(sizeof(struct ui_dom_event_listener));
  if (!listener)
    return UI_ERROR_OUT_OF_MEMORY;
  listener->type = type;
  listener->handler = handler;
  listener->user_data = user_data;
  listener->next = node->listeners;
  node->listeners = listener;
  return UI_ERROR_NONE;
}

enum ui_error ui_dom_node_remove_event_listener(struct ui_dom_node *node,
                                                enum ui_event_type type,
                                                ui_event_handler_t handler) {
  struct ui_dom_event_listener *curr, *prev = NULL;
  if (!node || !handler)
    return UI_ERROR_INVALID_ARGUMENT;
  curr = node->listeners;
  while (curr) {
    if (curr->type == type && curr->handler == handler) {
      if (prev)
        prev->next = curr->next;
      else
        node->listeners = curr->next;
      UI_FREE(curr);
      return UI_ERROR_NONE;
    }
    prev = curr;
    curr = curr->next;
  }
  return UI_ERROR_NOT_FOUND;
}
