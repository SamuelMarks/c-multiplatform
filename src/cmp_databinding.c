/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <string.h>
/* clang-format on */

struct cmp_databinding_s {
  cmp_data_type_t type;
  union {
    char *str_val;
    int int_val;
    float float_val;
    int bool_val;
  } data;

  cmp_databinding_cb_t *listeners;
  void **listener_ctxs;
  size_t listener_count;
  size_t listener_capacity;
};

/**
 * @brief Creates a new databinding context.
 *
 * @param out_binding Pointer to store the newly created databinding context.
 * @param type The type of data this binding will hold.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_databinding_create(cmp_databinding_t **out_binding,
                           cmp_data_type_t type) {
  int rc = CMP_SUCCESS;
  cmp_databinding_t *binding = NULL;

  if (out_binding == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_databinding_create: Invalid argument: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_databinding_t), (void **)&binding);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_databinding_create: Out of memory: %d\n", rc);

    return rc;
  }

  memset(binding, 0, sizeof(cmp_databinding_t));
  binding->type = type;

  *out_binding = binding;
  cmp_log_debug("cmp_databinding_create: Created databinding context\n");
  return rc;
}

/**
 * @brief Destroys a databinding context and frees its resources.
 *
 * @param binding Pointer to the databinding context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_databinding_destroy(cmp_databinding_t *binding) {
  int rc = CMP_SUCCESS;

  if (binding == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_databinding_destroy: Invalid argument: %d\n", rc);

    return rc;
  }

  if (binding->type == CMP_DATA_TYPE_STRING && binding->data.str_val != NULL) {
    rc = CMP_FREE(binding->data.str_val);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_databinding_destroy: CMP_FREE str_val failed\n: %d\n", rc);
      return rc;
    }
  }

  if (binding->listeners != NULL) {
    rc = CMP_FREE(binding->listeners);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_databinding_destroy: CMP_FREE listeners failed\n: %d\n",
                rc);
      return rc;
    }
  }
  if (binding->listener_ctxs != NULL) {
    rc = CMP_FREE(binding->listener_ctxs);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "cmp_databinding_destroy: CMP_FREE listener_ctxs failed\n: %d\n", rc);
      return rc;
    }
  }

  rc = CMP_FREE(binding);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("CMP_FREE: %d\n", rc);
    return rc;
  }
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_databinding_destroy: CMP_FREE context failed\n: %d\n", rc);
    return rc;
  }

  cmp_log_debug(
      "cmp_databinding_destroy: Successfully destroyed databinding context\n");
  return rc;
}

/**
 * @brief Internal helper to notify all registered listeners of a value change.
 *
 * @param binding Pointer to the databinding context.
 */
static void notify_listeners(cmp_databinding_t *binding) {
  size_t i;
  for (i = 0; i < binding->listener_count; ++i) {
    if (binding->listeners[i]) {
      binding->listeners[i](binding, binding->listener_ctxs[i]);
    }
  }
}

/**
 * @brief Sets the string value of a databinding and notifies listeners.
 *
 * @param binding Pointer to the databinding context.
 * @param val The new string value to set, or NULL to clear.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_databinding_set_string(cmp_databinding_t *binding, const char *val) {
  size_t len;
  char *new_str = NULL;
  int rc = CMP_SUCCESS;

  if (binding == NULL || binding->type != CMP_DATA_TYPE_STRING) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_databinding_set_string: Invalid argument: %d\n", rc);

    return rc;
  }

  if (val == NULL) {
    if (binding->data.str_val != NULL) {
      rc = CMP_FREE(binding->data.str_val);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
      if (rc != CMP_SUCCESS) {
        cmp_log_debug(
            "cmp_databinding_set_string: CMP_FREE old str_val failed\n");
      }
      binding->data.str_val = NULL;
      notify_listeners(binding);
    }
    return rc;
  }

  len = strlen(val);
  rc = CMP_MALLOC(len + 1, (void **)&new_str);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_databinding_set_string: Out of memory: %d\n", rc);

    return rc;
  }
#if defined(_MSC_VER)
  if (strcpy_s(new_str, len + 1, val) != 0) {
    rc = CMP_FREE(new_str);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    cmp_log_debug("cmp_databinding_set_string: strcpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(new_str, val);
#endif

  if (binding->data.str_val != NULL) {
    rc = CMP_FREE(binding->data.str_val);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_databinding_set_string: CMP_FREE old str_val failed\n");
    }
  }
  binding->data.str_val = new_str;

  notify_listeners(binding);

  cmp_log_debug("cmp_databinding_set_string: String binding updated\n");
  return rc;
}

/**
 * @brief Gets the current string value of a databinding.
 *
 * @param binding Pointer to the databinding context.
 * @param out_val Pointer to store the returned string pointer.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_databinding_get_string(cmp_databinding_t *binding,
                               const char **out_val) {
  int rc = CMP_SUCCESS;

  if (binding == NULL || binding->type != CMP_DATA_TYPE_STRING ||
      out_val == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_databinding_get_string: Invalid argument: %d\n", rc);

    return rc;
  }
  *out_val = binding->data.str_val;

  return rc;
}

/**
 * @brief Sets the integer value of a databinding and notifies listeners.
 *
 * @param binding Pointer to the databinding context.
 * @param val The new integer value.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_databinding_set_int(cmp_databinding_t *binding, int val) {
  int rc = CMP_SUCCESS;

  if (binding == NULL || binding->type != CMP_DATA_TYPE_INT) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_databinding_set_int: Invalid argument: %d\n", rc);

    return rc;
  }
  binding->data.int_val = val;
  notify_listeners(binding);
  cmp_log_debug("cmp_databinding_set_int: Int binding updated\n");

  return rc;
}

/**
 * @brief Gets the current integer value of a databinding.
 *
 * @param binding Pointer to the databinding context.
 * @param out_val Pointer to store the returned integer value.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_databinding_get_int(cmp_databinding_t *binding, int *out_val) {
  int rc = CMP_SUCCESS;

  if (binding == NULL || binding->type != CMP_DATA_TYPE_INT ||
      out_val == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_databinding_get_int: Invalid argument: %d\n", rc);

    return rc;
  }
  *out_val = binding->data.int_val;

  return rc;
}

/**
 * @brief Registers a callback listener to be invoked on databinding value
 * changes.
 *
 * @param binding Pointer to the databinding context.
 * @param cb The callback function to invoke.
 * @param user_data Optional user context to pass to the callback.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_databinding_add_listener(cmp_databinding_t *binding,
                                 cmp_databinding_cb_t cb, void *user_data) {
  size_t new_cap;
  cmp_databinding_cb_t *new_listeners = NULL;
  void **new_ctxs = NULL;
  int rc = CMP_SUCCESS;

  if (binding == NULL || cb == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_databinding_add_listener: Invalid argument: %d\n", rc);

    return rc;
  }

  if (binding->listener_count >= binding->listener_capacity) {
    new_cap =
        binding->listener_capacity == 0 ? 4 : binding->listener_capacity * 2;

    rc = CMP_MALLOC(new_cap * sizeof(cmp_databinding_cb_t),
                    (void **)&new_listeners);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_databinding_add_listener CMP_MALLOC: %d\n", rc);

      return rc;
    }

    rc = CMP_MALLOC(new_cap * sizeof(void *), (void **)&new_ctxs);
    if (rc != CMP_SUCCESS) {
      rc = CMP_FREE(new_listeners);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
      cmp_log_debug("cmp_databinding_add_listener: Out of memory (ctxs)\n");

      return rc;
    }

    if (binding->listener_count > 0) {
      memcpy(new_listeners, binding->listeners,
             binding->listener_count * sizeof(cmp_databinding_cb_t));
      memcpy(new_ctxs, binding->listener_ctxs,
             binding->listener_count * sizeof(void *));
      rc = CMP_FREE(binding->listeners);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
      rc = CMP_FREE(binding->listener_ctxs);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }

    binding->listeners = new_listeners;
    binding->listener_ctxs = new_ctxs;
    binding->listener_capacity = new_cap;
  }

  binding->listeners[binding->listener_count] = cb;
  binding->listener_ctxs[binding->listener_count] = user_data;
  binding->listener_count++;

  cmp_log_debug("cmp_databinding_add_listener: Registered listener\n");
  return rc;
}

typedef struct {
  cmp_ui_node_t *node;
  char property_name[32];
} cmp_node_binding_ctx_t;

/**
 * @brief Internal callback to handle updating a UI node property when a binding
 * changes.
 *
 * @param binding Pointer to the databinding context that triggered the event.
 * @param user_data The context containing the node and property mappings.
 */
static void node_binding_cb(cmp_databinding_t *binding, void *user_data) {
  int rc = CMP_SUCCESS;
  cmp_node_binding_ctx_t *ctx = (cmp_node_binding_ctx_t *)user_data;
  if (ctx && ctx->node && ctx->property_name) {
    if (strcmp(ctx->property_name, "text") == 0 &&
        binding->type == CMP_DATA_TYPE_STRING) {
      const char *val;
      if (cmp_databinding_get_string(binding, &val) == CMP_SUCCESS) {
        if (ctx->node->type == 2) { /* Text */
          if (ctx->node->properties) {
            rc = CMP_FREE(ctx->node->properties);
            if (rc != CMP_SUCCESS) {
              LOG_DEBUG("CMP_FREE: %d\n", rc);
              return;
            }
            ctx->node->properties = NULL;
          }
          if (val) {
            size_t len = strlen(val);
            char *copy = NULL;
            if (CMP_MALLOC(len + 1, (void **)&copy) == CMP_SUCCESS) {
#if defined(_MSC_VER)
              if (strcpy_s(copy, len + 1, val) == 0) {
                ctx->node->properties = copy;
              } else {
                rc = CMP_FREE(copy);
                if (rc != CMP_SUCCESS) {
                  LOG_DEBUG("CMP_FREE: %d\n", rc);
                  return;
                }
              }
#else
              strcpy(copy, val);
              ctx->node->properties = copy;
#endif
            }
          }
        }
      }
    }
  }
}

/**
 * @brief Binds a specific property of a generic UI node to a databinding
 * context.
 *
 * @param node Pointer to the target UI node.
 * @param binding Pointer to the databinding context to observe.
 * @param property_name The name of the property being bound.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_node_bind_generic(cmp_ui_node_t *node, cmp_databinding_t *binding,
                             const char *property_name) {
  cmp_node_binding_ctx_t *ctx = NULL;
  int rc = CMP_SUCCESS;

  if (node == NULL || binding == NULL || property_name == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_node_bind_generic: Invalid argument: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_node_binding_ctx_t), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_node_bind_generic CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(ctx, 0, sizeof(cmp_node_binding_ctx_t));
  ctx->node = node;
#if defined(_MSC_VER)
  if (strncpy_s(ctx->property_name, sizeof(ctx->property_name), property_name,
                _TRUNCATE) != 0) {
    rc = CMP_FREE(ctx);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    return CMP_ERROR_GENERAL;
  }
#else
  strncpy(ctx->property_name, property_name, sizeof(ctx->property_name) - 1);
  ctx->property_name[sizeof(ctx->property_name) - 1] = '\0';
#endif

  rc = cmp_databinding_add_listener(binding, node_binding_cb, ctx);
  if (rc != CMP_SUCCESS) {
    rc = CMP_FREE(ctx);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
    LOG_DEBUG("cmp_ui_node_bind_generic cmp_databinding_add_listener: %d\n",
              rc);

    return rc;
  }

  /* Initial sync */
  node_binding_cb(binding, ctx);

  cmp_log_debug("cmp_ui_node_bind_generic: Successfully bound generic node\n");
  return rc;
}
