/**
 * @file ui_dynamic_context.c
 * @brief Dynamic scoped data context implementation for runtime widgets.
 */

/* clang-format off */
#include "ui_dynamic_context.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @struct ui_named_signal
 * @brief Linked list entry for a named scoped signal.
 */
struct ui_named_signal {
  char *name;                   /**< Signal name */
  struct ui_signal *signal;     /**< Signal pointer */
  struct ui_named_signal *next; /**< Next entry */
};

/**
 * @struct ui_named_group
 * @brief Linked list entry for a named form group.
 */
struct ui_named_group {
  char *name;                  /**< Form group name */
  ui_form_group_t *group;      /**< Form group pointer */
  struct ui_named_group *next; /**< Next entry */
};

/**
 * @struct ui_named_computed
 * @brief Linked list entry for a named computed value.
 */
struct ui_named_computed {
  char *name;                     /**< Identifier name */
  struct ui_computed *comp;       /**< Computed value pointer */
  struct ui_named_computed *next; /**< Next entry */
};

/**
 * @struct ui_dynamic_context
 * @brief Dynamic context container storing scoped reactive entities.
 */
struct ui_dynamic_context {
  struct ui_arena *arena;              /**< Memory arena used for allocations */
  struct ui_named_signal *signals;     /**< Head of signals list */
  struct ui_named_group *groups;       /**< Head of form groups list */
  struct ui_named_computed *computeds; /**< Head of computed values list */
};

/**
 * @brief Duplicates a string using the context's arena or heap fallback.
 */
static ui_error_t ctx_strdup(struct ui_dynamic_context *ctx, const char *src,
                             char **out_str) {
  size_t len;
  char *dest = NULL;
  ui_error_t rc;

  len = strlen(src);
  if (ctx->arena) {
    rc = ui_arena_alloc(ctx->arena, len + 1, 1, (void **)&dest);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    dest = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
    if (!dest) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  if (UI_STRCPY(dest, len + 1, src) != 0) {
    if (!ctx->arena) {
      C_MULTIPLATFORM_FREE(dest);
    }
    return UI_ERROR_UNKNOWN;
  }

  *out_str = dest;
  return UI_ERROR_NONE;
}

/**
 * @brief Creates a new dynamic data context.
 *
 * @param arena The memory arena to allocate from.
 * @param out_ctx Pointer to receive the allocated context.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_dynamic_context_create(struct ui_arena *arena,
                                     struct ui_dynamic_context **out_ctx) {
  struct ui_dynamic_context *ctx = NULL;
  ui_error_t rc;

  if (!out_ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (arena) {
    rc = ui_arena_alloc(arena, sizeof(struct ui_dynamic_context), 8,
                        (void **)&ctx);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    ctx = (struct ui_dynamic_context *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_dynamic_context));
    if (!ctx) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  ctx->arena = arena;
  ctx->signals = NULL;
  ctx->groups = NULL;
  ctx->computeds = NULL;
  *out_ctx = ctx;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a dynamic context.
 *
 * @param ctx The context to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_dynamic_context_destroy(struct ui_dynamic_context *ctx) {
  if (!ctx) {
    return UI_ERROR_NONE;
  }

  /* If arena-allocated, arena teardown frees memory */
  if (ctx->arena) {
    return UI_ERROR_NONE;
  }

  while (ctx->signals) {
    struct ui_named_signal *next = ctx->signals->next;
    C_MULTIPLATFORM_FREE(ctx->signals->name);
    C_MULTIPLATFORM_FREE(ctx->signals);
    ctx->signals = next;
  }

  while (ctx->groups) {
    struct ui_named_group *next = ctx->groups->next;
    C_MULTIPLATFORM_FREE(ctx->groups->name);
    C_MULTIPLATFORM_FREE(ctx->groups);
    ctx->groups = next;
  }

  while (ctx->computeds) {
    struct ui_named_computed *next = ctx->computeds->next;
    C_MULTIPLATFORM_FREE(ctx->computeds->name);
    C_MULTIPLATFORM_FREE(ctx->computeds);
    ctx->computeds = next;
  }

  C_MULTIPLATFORM_FREE(ctx);
  return UI_ERROR_NONE;
}

/**
 * @brief Registers a scoped signal into the dynamic context.
 *
 * @param ctx The dynamic context.
 * @param name Signal identifier name.
 * @param signal The signal instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_dynamic_context_register_signal(struct ui_dynamic_context *ctx,
                                              const char *name,
                                              struct ui_signal *signal) {
  struct ui_named_signal *curr;
  struct ui_named_signal *entry = NULL;
  ui_error_t rc;

  if (!ctx || !name || !signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = ctx->signals;
  while (curr) {
    if (strcmp(curr->name, name) == 0) {
      curr->signal = signal;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  if (ctx->arena) {
    rc = ui_arena_alloc(ctx->arena, sizeof(struct ui_named_signal), 8,
                        (void **)&entry);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    entry = (struct ui_named_signal *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_named_signal));
    if (!entry) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  rc = ctx_strdup(ctx, name, &entry->name);
  if (rc != UI_ERROR_NONE) {
    if (!ctx->arena) {
      C_MULTIPLATFORM_FREE(entry);
    }
    return rc;
  }

  entry->signal = signal;
  entry->next = ctx->signals;
  ctx->signals = entry;

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves a scoped signal from the context by name.
 *
 * @param ctx The dynamic context.
 * @param name Signal identifier name.
 * @param out_signal Pointer to receive the found signal.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if not present.
 */
ui_error_t ui_dynamic_context_get_signal(const struct ui_dynamic_context *ctx,
                                         const char *name,
                                         struct ui_signal **out_signal) {
  const struct ui_named_signal *curr;

  if (!ctx || !name || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = ctx->signals;
  while (curr) {
    if (strcmp(curr->name, name) == 0) {
      *out_signal = curr->signal;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  return UI_ERROR_NOT_FOUND;
}

/**
 * @brief Registers a form group into the dynamic context.
 *
 * @param ctx The dynamic context.
 * @param name Form group identifier name.
 * @param group The form group instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_dynamic_context_register_form_group(
    struct ui_dynamic_context *ctx, const char *name, ui_form_group_t *group) {
  struct ui_named_group *curr;
  struct ui_named_group *entry = NULL;
  ui_error_t rc;

  if (!ctx || !name || !group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = ctx->groups;
  while (curr) {
    if (strcmp(curr->name, name) == 0) {
      curr->group = group;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  if (ctx->arena) {
    rc = ui_arena_alloc(ctx->arena, sizeof(struct ui_named_group), 8,
                        (void **)&entry);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    entry = (struct ui_named_group *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_named_group));
    if (!entry) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  rc = ctx_strdup(ctx, name, &entry->name);
  if (rc != UI_ERROR_NONE) {
    if (!ctx->arena) {
      C_MULTIPLATFORM_FREE(entry);
    }
    return rc;
  }

  entry->group = group;
  entry->next = ctx->groups;
  ctx->groups = entry;

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves a form group by name from the context.
 *
 * @param ctx The dynamic context.
 * @param name Form group identifier name.
 * @param out_group Pointer to receive the form group.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if not present.
 */
ui_error_t
ui_dynamic_context_get_form_group(const struct ui_dynamic_context *ctx,
                                  const char *name,
                                  ui_form_group_t **out_group) {
  const struct ui_named_group *curr;

  if (!ctx || !name || !out_group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = ctx->groups;
  while (curr) {
    if (strcmp(curr->name, name) == 0) {
      *out_group = curr->group;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  return UI_ERROR_NOT_FOUND;
}

/**
 * @brief Registers a computed signal value into the dynamic context.
 *
 * @param ctx The dynamic context.
 * @param name Identifier name.
 * @param comp The computed value instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_dynamic_context_register_computed(struct ui_dynamic_context *ctx,
                                                const char *name,
                                                struct ui_computed *comp) {
  struct ui_named_computed *curr;
  struct ui_named_computed *entry = NULL;
  ui_error_t rc;

  if (!ctx || !name || !comp) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = ctx->computeds;
  while (curr) {
    if (strcmp(curr->name, name) == 0) {
      curr->comp = comp;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  if (ctx->arena) {
    rc = ui_arena_alloc(ctx->arena, sizeof(struct ui_named_computed), 8,
                        (void **)&entry);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    entry = (struct ui_named_computed *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_named_computed));
    if (!entry) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  rc = ctx_strdup(ctx, name, &entry->name);
  if (rc != UI_ERROR_NONE) {
    if (!ctx->arena) {
      C_MULTIPLATFORM_FREE(entry);
    }
    return rc;
  }

  entry->comp = comp;
  entry->next = ctx->computeds;
  ctx->computeds = entry;

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves a computed value by name from the context.
 *
 * @param ctx The dynamic context.
 * @param name Identifier name.
 * @param out_comp Pointer to receive the computed value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if not present.
 */
ui_error_t ui_dynamic_context_get_computed(const struct ui_dynamic_context *ctx,
                                           const char *name,
                                           struct ui_computed **out_comp) {
  const struct ui_named_computed *curr;

  if (!ctx || !name || !out_comp) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = ctx->computeds;
  while (curr) {
    if (strcmp(curr->name, name) == 0) {
      *out_comp = curr->comp;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  return UI_ERROR_NOT_FOUND;
}

/**
 * @brief Resolves a form control by path (e.g. "user.email").
 *
 * @param ctx The dynamic context.
 * @param path Dotted path to form control.
 * @param out_ctrl Pointer to receive the resolved form control.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND /
 * UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t
ui_dynamic_context_resolve_form_control(const struct ui_dynamic_context *ctx,
                                        const char *path,
                                        ui_form_control_t **out_ctrl) {
  const char *dot;
  char group_name[128];
  size_t group_len;
  const char *ctrl_name;
  ui_form_group_t *grp = NULL;
  ui_error_t rc;

  if (!ctx || !path || !out_ctrl) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dot = strchr(path, '.');
  if (dot) {
    group_len = (size_t)(dot - path);
    if (group_len >= sizeof(group_name)) {
      return UI_ERROR_OUT_OF_BOUNDS;
    }
#if defined(_MSC_VER)
    strncpy_s(group_name, sizeof(group_name), path, group_len);
#else
    strncpy(group_name, path, group_len);
#endif
    group_name[group_len] = '\0';
    ctrl_name = dot + 1;

    rc = ui_dynamic_context_get_form_group(ctx, group_name, &grp);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    return ui_form_group_get_control(grp, ctrl_name, out_ctrl);
  }

  /* If no dot, try lookup across registered groups */
  {
    const struct ui_named_group *g = ctx->groups;
    while (g) {
      rc = ui_form_group_get_control(g->group, path, out_ctrl);
      if (rc == UI_ERROR_NONE) {
        return UI_ERROR_NONE;
      }
      g = g->next;
    }
  }

  return UI_ERROR_NOT_FOUND;
}

/**
 * @brief Resolves a signal by path, supporting global "app.<name>" routing.
 *
 * @param ctx The dynamic context.
 * @param app_state The optional app state registry.
 * @param path Signal identifier path.
 * @param out_signal Pointer to receive the resolved signal.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
ui_error_t
ui_dynamic_context_resolve_signal(const struct ui_dynamic_context *ctx,
                                  const struct ui_app_state_registry *app_state,
                                  const char *path,
                                  struct ui_signal **out_signal) {
  if (!ctx || !path || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strncmp(path, "app.", 4) == 0) {
    const char *key = path + 4;
    if (app_state) {
      return ui_app_state_registry_get_signal(app_state, key, out_signal);
    } else {
      struct ui_app_state_registry *g_state = NULL;
      ui_error_t rc = ui_app_state_registry_get_global(&g_state);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      return ui_app_state_registry_get_signal(g_state, key, out_signal);
    }
  }

  return ui_dynamic_context_get_signal(ctx, path, out_signal);
}
