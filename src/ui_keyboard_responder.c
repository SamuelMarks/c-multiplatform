/* clang-format off */
#include "ui_keyboard_responder.h"
#include "ui_bidi_manager.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_keyboard_binding {
  char *role_or_tag;
  enum ui_key_code key_code;
  ui_error_t (*callback)(struct ui_dom_node *node, void *user_data);
  void *user_data;
};

/** \brief ui_keyboard_responder */
struct ui_keyboard_responder {
  struct ui_keyboard_binding *bindings;
  size_t bindings_count;
  size_t bindings_capacity;
};

/** \brief ui_error */
ui_error_t
ui_keyboard_responder_create(struct ui_keyboard_responder **out_responder) {
  struct ui_keyboard_responder *responder;

  if (!out_responder) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  responder = (struct ui_keyboard_responder *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_keyboard_responder));
  if (!responder) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  responder->bindings = NULL;
  responder->bindings_count = 0;
  responder->bindings_capacity = 0;

  *out_responder = responder;
  return UI_ERROR_NONE;
}

ui_error_t
ui_keyboard_responder_destroy(struct ui_keyboard_responder *responder) {
  size_t i;
  if (!responder) {
    return UI_ERROR_NONE;
  }

  if (responder->bindings) {
    for (i = 0; i < responder->bindings_count; ++i) {
      C_MULTIPLATFORM_FREE(responder->bindings[i].role_or_tag);
    }
    C_MULTIPLATFORM_FREE(responder->bindings);
  }
  C_MULTIPLATFORM_FREE(responder);
  return UI_ERROR_NONE;
}

/** \brief ui_keyboard_responder_bind_key */
ui_error_t ui_keyboard_responder_bind_key(
    struct ui_keyboard_responder *responder, const char *role_or_tag,
    enum ui_key_code key_code,
    ui_error_t (*callback)(struct ui_dom_node *node, void *user_data),
    void *user_data) {
  char *role_copy;

  if (!responder || !role_or_tag || !callback) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (responder->bindings_count >= responder->bindings_capacity) {
    size_t new_capacity = responder->bindings_capacity == 0
                              ? 8
                              : responder->bindings_capacity * 2;
    struct ui_keyboard_binding *new_bindings =
        (struct ui_keyboard_binding *)C_MULTIPLATFORM_REALLOC(
            responder->bindings,
            new_capacity * sizeof(struct ui_keyboard_binding));
    if (!new_bindings) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    responder->bindings = new_bindings;
    responder->bindings_capacity = new_capacity;
  }

  {
    size_t len = strlen(role_or_tag);
    role_copy = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
    if (role_copy) {
#if defined(_MSC_VER)
      strcpy_s(role_copy, len + 1, role_or_tag);
#else
      strcpy(role_copy, role_or_tag);
#endif
    }
  }
  if (!role_copy) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  responder->bindings[responder->bindings_count].role_or_tag = role_copy;
  responder->bindings[responder->bindings_count].key_code = key_code;
  responder->bindings[responder->bindings_count].callback = callback;
  responder->bindings[responder->bindings_count].user_data = user_data;
  responder->bindings_count++;

  return UI_ERROR_NONE;
}

/** \brief ui_keyboard_responder_handle_event */
ui_error_t ui_keyboard_responder_handle_event(
    struct ui_keyboard_responder *responder, struct ui_dom_node *focused_node,
    const struct ui_event *event, int *out_handled) {
  size_t i;
  const char *role_val = NULL;
  int is_match = 0;
  enum ui_key_code key;

  if (!out_handled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_handled = 0;

  if (!responder || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!focused_node || focused_node->type != UI_DOM_NODE_TYPE_ELEMENT) {
    return UI_ERROR_NONE;
  }

  /* We primarily bind to KEY_DOWN for actions like clicking buttons. */
  if (event->type != UI_EVENT_KEY_DOWN) {
    return UI_ERROR_NONE;
  }

  key = (enum ui_key_code)event->event_data.keyboard.key_code;
  (void)ui_bidi_normalize_horizontal_key(key, &key);

  (void)ui_dom_node_get_attribute(focused_node, "role", &role_val);

  for (i = 0; i < responder->bindings_count; ++i) {
    struct ui_keyboard_binding *binding = &responder->bindings[i];

    if (binding->key_code == key) {
      /* Match against tag_name or role */
      is_match = 0;
      if (focused_node->tag_name &&
          strcmp(focused_node->tag_name, binding->role_or_tag) == 0) {
        is_match = 1;
      } else if (role_val && strcmp(role_val, binding->role_or_tag) == 0) {
        is_match = 1;
      }

      if (is_match) {
        binding->callback(focused_node, binding->user_data);
        *out_handled = 1;
        /* If multiple bindings match, we just trigger the first one and break,
           or continue? Normally we trigger the first one that handles it. */
        break;
      }
    }
  }

  return UI_ERROR_NONE;
}
