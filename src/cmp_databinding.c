/* clang-format off */
#include "cmp.h"
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

int cmp_databinding_create(cmp_databinding_t **out_binding,
                           cmp_data_type_t type) {
  int rc;
  cmp_databinding_t *binding;

  if (out_binding == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_databinding_t), (void **)&binding);
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  memset(binding, 0, sizeof(cmp_databinding_t));
  binding->type = type;

  *out_binding = binding;
  return CMP_SUCCESS;
}

int cmp_databinding_destroy(cmp_databinding_t *binding) {
  if (binding == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (binding->type == CMP_DATA_TYPE_STRING && binding->data.str_val) {
    CMP_FREE(binding->data.str_val);
  }

  if (binding->listeners) {
    CMP_FREE(binding->listeners);
  }
  if (binding->listener_ctxs) {
    CMP_FREE(binding->listener_ctxs);
  }

  CMP_FREE(binding);
  return CMP_SUCCESS;
}

static void notify_listeners(cmp_databinding_t *binding) {
  size_t i;
  for (i = 0; i < binding->listener_count; ++i) {
    if (binding->listeners[i]) {
      binding->listeners[i](binding, binding->listener_ctxs[i]);
    }
  }
}

int cmp_databinding_set_string(cmp_databinding_t *binding, const char *val) {
  size_t len;
  char *new_str;
  int rc;

  if (binding == NULL || binding->type != CMP_DATA_TYPE_STRING) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (val == NULL) {
    if (binding->data.str_val) {
      CMP_FREE(binding->data.str_val);
      binding->data.str_val = NULL;
      notify_listeners(binding);
    }
    return CMP_SUCCESS;
  }

  len = strlen(val);
  rc = CMP_MALLOC(len + 1, (void **)&new_str);
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  memcpy(new_str, val, len + 1);

  if (binding->data.str_val) {
    CMP_FREE(binding->data.str_val);
  }
  binding->data.str_val = new_str;

  notify_listeners(binding);

  return CMP_SUCCESS;
}

int cmp_databinding_get_string(cmp_databinding_t *binding,
                               const char **out_val) {
  if (binding == NULL || binding->type != CMP_DATA_TYPE_STRING ||
      out_val == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_val = binding->data.str_val;
  return CMP_SUCCESS;
}

int cmp_databinding_set_int(cmp_databinding_t *binding, int val) {
  if (binding == NULL || binding->type != CMP_DATA_TYPE_INT) {
    return CMP_ERROR_INVALID_ARG;
  }
  binding->data.int_val = val;
  notify_listeners(binding);
  return CMP_SUCCESS;
}

int cmp_databinding_get_int(cmp_databinding_t *binding, int *out_val) {
  if (binding == NULL || binding->type != CMP_DATA_TYPE_INT ||
      out_val == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_val = binding->data.int_val;
  return CMP_SUCCESS;
}

int cmp_databinding_add_listener(cmp_databinding_t *binding,
                                 cmp_databinding_cb_t cb, void *user_data) {
  size_t new_cap;
  cmp_databinding_cb_t *new_listeners;
  void **new_ctxs;
  int rc;

  if (binding == NULL || cb == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (binding->listener_count >= binding->listener_capacity) {
    new_cap =
        binding->listener_capacity == 0 ? 4 : binding->listener_capacity * 2;

    rc = CMP_MALLOC(new_cap * sizeof(cmp_databinding_cb_t),
                    (void **)&new_listeners);
    if (rc != CMP_SUCCESS)
      return rc;

    rc = CMP_MALLOC(new_cap * sizeof(void *), (void **)&new_ctxs);
    if (rc != CMP_SUCCESS) {
      CMP_FREE(new_listeners);
      return rc;
    }

    if (binding->listener_count > 0) {
      memcpy(new_listeners, binding->listeners,
             binding->listener_count * sizeof(cmp_databinding_cb_t));
      memcpy(new_ctxs, binding->listener_ctxs,
             binding->listener_count * sizeof(void *));
      CMP_FREE(binding->listeners);
      CMP_FREE(binding->listener_ctxs);
    }

    binding->listeners = new_listeners;
    binding->listener_ctxs = new_ctxs;
    binding->listener_capacity = new_cap;
  }

  binding->listeners[binding->listener_count] = cb;
  binding->listener_ctxs[binding->listener_count] = user_data;
  binding->listener_count++;

  return CMP_SUCCESS;
}

typedef struct {
  cmp_ui_node_t *node;
  char property_name[32];
} cmp_node_binding_ctx_t;

static void node_binding_cb(cmp_databinding_t *binding, void *user_data) {
  cmp_node_binding_ctx_t *ctx = (cmp_node_binding_ctx_t *)user_data;
  if (ctx && ctx->node && ctx->property_name) {
    if (strcmp(ctx->property_name, "text") == 0 &&
        binding->type == CMP_DATA_TYPE_STRING) {
      const char *val;
      if (cmp_databinding_get_string(binding, &val) == CMP_SUCCESS) {
        if (ctx->node->type == 2) { /* Text */
          if (ctx->node->properties) {
            CMP_FREE(ctx->node->properties);
            ctx->node->properties = NULL;
          }
          if (val) {
            size_t len = strlen(val);
            char *copy;
            if (CMP_MALLOC(len + 1, (void **)&copy) == CMP_SUCCESS) {
              memcpy(copy, val, len + 1);
              ctx->node->properties = copy;
            }
          }
        }
      }
    }
  }
}

int cmp_ui_node_bind_generic(cmp_ui_node_t *node, cmp_databinding_t *binding,
                             const char *property_name) {
  cmp_node_binding_ctx_t *ctx;
  int rc;

  if (node == NULL || binding == NULL || property_name == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_node_binding_ctx_t), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  memset(ctx, 0, sizeof(cmp_node_binding_ctx_t));
  ctx->node = node;
#if defined(_MSC_VER)
  strcpy_s(ctx->property_name, sizeof(ctx->property_name), property_name);
#else
  strncpy(ctx->property_name, property_name, sizeof(ctx->property_name) - 1);
#endif

  rc = cmp_databinding_add_listener(binding, node_binding_cb, ctx);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(ctx);
    return rc;
  }

  /* Initial sync */
  node_binding_cb(binding, ctx);

  return CMP_SUCCESS;
}
