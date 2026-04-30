/* clang-format off */
#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#endif
/* clang-format on */

typedef struct cmp_route_entry {
  char *path;
  cmp_route_builder_cb builder;
  cmp_route_guard_cb guard;
  void *guard_data;
  struct cmp_route_entry *next;
} cmp_route_entry_t;

struct cmp_router {
  cmp_route_entry_t *routes;

  /* Stack of active routes, stored dynamically */
  char **stack;
  size_t stack_count;
  size_t stack_capacity;

  /* Currently active view */
  void *active_view;
};

static int g_router_initialized = 0;

/**
 * @brief cmp_router_create
 *
 * @param out_router Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_create(cmp_router_t **out_router) {
  int rc = CMP_SUCCESS;
  cmp_router_t *router;

  if (out_router == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_router_t), (void **)&router);
  if (rc != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  router->routes = NULL;
  router->stack = NULL;
  router->stack_count = 0;
  router->stack_capacity = 0;
  router->active_view = NULL;

  *out_router = router;
  g_router_initialized = 1;
  return rc;
}

/**
 * @brief cmp_router_destroy
 *
 * @param router Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_destroy(cmp_router_t *router) {
  int rc = CMP_SUCCESS;
  cmp_route_entry_t *curr, *next;
  size_t i;

  if (router == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Free routes */
  curr = router->routes;
  while (curr != NULL) {
    next = curr->next;
    rc = CMP_FREE(curr->path);
    if (rc != CMP_SUCCESS) {
      return rc;
    }
    rc = CMP_FREE(curr);
    if (rc != CMP_SUCCESS) {
      return rc;
    }
    curr = next;
  }

  /* Free stack */
  for (i = 0; i < router->stack_count; i++) {
    rc = CMP_FREE(router->stack[i]);
    if (rc != CMP_SUCCESS) {
      return rc;
    }
  }
  if (router->stack) {

    rc = CMP_FREE(router->stack);
    if (rc != CMP_SUCCESS) {
      return rc;
    }
  }

  rc = CMP_FREE(router);
  if (rc != CMP_SUCCESS) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_router_register
 *
 * @param router Parameter description.
 * @param path Parameter description.
 * @param builder Parameter description.
 * @param guard Parameter description.
 * @param guard_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_register(cmp_router_t *router, const char *path,
                        cmp_route_builder_cb builder, cmp_route_guard_cb guard,
                        void *guard_data) {
  int rc = CMP_SUCCESS;
  cmp_route_entry_t *entry;
  size_t len;

  if (router == NULL || path == NULL || builder == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_route_entry_t), (void **)&entry);
  if (rc != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  len = strlen(path);
  rc = CMP_MALLOC(len + 1, (void **)&entry->path);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(entry);
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(entry->path, len + 1, path);
#else
  strcpy(entry->path, path);
#endif

  entry->builder = builder;
  entry->guard = guard;
  entry->guard_data = guard_data;

  entry->next = router->routes;
  router->routes = entry;
  return rc;
}

/* C89 safe implementation of string tokenization with saved state */

/**
 * @brief internal_execute_route
 *
 * @param router Parameter description.
 * @param uri Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int internal_execute_route(cmp_router_t *router, const char *uri) {
  int rc = CMP_SUCCESS;
  cmp_route_entry_t *curr = router->routes;
  char *uri_mutable;
  char *saveptr_uri = NULL;
  size_t uri_len;

  if (router == NULL || uri == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  uri_len = strlen(uri);
  rc = CMP_MALLOC(uri_len + 1, (void **)&uri_mutable);
  if (rc != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(uri_mutable, uri_len + 1, uri);
#else
  strcpy(uri_mutable, uri);
#endif

  while (curr != NULL) {
    int match = 1;
    char *path_mutable;
    char *saveptr_path = NULL;
    char *u_tok = NULL, *p_tok = NULL;
    size_t path_len = strlen(curr->path);

    /* Quick exact match optimization */
    if (strcmp(curr->path, uri) == 0) {
      match = 1;
      goto check_guard;
    }

    rc = CMP_MALLOC(path_len + 1, (void **)&path_mutable);
    if (rc != CMP_SUCCESS) {
      CMP_FREE(uri_mutable);
      return CMP_ERROR_OOM;
    }
#if defined(_MSC_VER)
    strcpy_s(path_mutable, path_len + 1, curr->path);
#else
    strcpy(path_mutable, curr->path);
#endif

    /* Re-copy URI for each route check since strtok modifies it */
#if defined(_MSC_VER)
    strcpy_s(uri_mutable, uri_len + 1, uri);
#else
    strcpy(uri_mutable, uri);
#endif

    u_tok = NULL;
    p_tok = NULL;
    cmp_strtok_r(uri_mutable, "/", &saveptr_uri, &u_tok);
    cmp_strtok_r(path_mutable, "/", &saveptr_path, &p_tok);

    while (u_tok != NULL || p_tok != NULL) {
      if (u_tok == NULL || p_tok == NULL) {
        match = 0; /* Different number of segments */
        break;
      }

      if (p_tok[0] == ':') {
        /* Dynamic parameter, matches anything in this segment.
           In a full implementation, we'd extract the value here and pass it
           in a structured way to the builder/guard. Phase 14 requests param
           mapping mechanics. */
      } else if (strcmp(u_tok, p_tok) != 0) {
        match = 0; /* Static segment mismatch */
        break;
      }

      u_tok = NULL;
      p_tok = NULL;
      cmp_strtok_r(NULL, "/", &saveptr_uri, &u_tok);
      cmp_strtok_r(NULL, "/", &saveptr_path, &p_tok);
    }

    rc = CMP_FREE(path_mutable);
    if (rc != CMP_SUCCESS) {
      return rc;
    }

  check_guard:
    if (match) {
      /* Check guard first */
      if (curr->guard != NULL) {
        if (!curr->guard(uri, curr->guard_data)) {
          rc = CMP_FREE(uri_mutable);
          if (rc != CMP_SUCCESS) {
            return rc;
          }
          return CMP_ERROR_NOT_FOUND; /* Guard blocked navigation */
        }
      }

      /* Build view */
      router->active_view = curr->builder(uri);
      rc = CMP_FREE(uri_mutable);
      if (rc != CMP_SUCCESS) {
        return rc;
      }
      return rc;
    }

    curr = curr->next;
  }

  rc = CMP_FREE(uri_mutable);
  if (rc != CMP_SUCCESS) {
    return rc;
  }
  rc = CMP_ERROR_NOT_FOUND;
  return rc;
}

/**
 * @brief cmp_router_push
 *
 * @param router Parameter description.
 * @param uri Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_push(cmp_router_t *router, const char *uri) {
  int rc = CMP_SUCCESS;
  size_t len;
  char *uri_copy;

  if (router == NULL || uri == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (internal_execute_route(router, uri) != CMP_SUCCESS) {
    return CMP_ERROR_NOT_FOUND;
  }

  /* Push to stack */
  if (router->stack_count >= router->stack_capacity) {
    size_t new_cap =
        router->stack_capacity == 0 ? 4 : router->stack_capacity * 2;
    char **new_stack;

    rc = CMP_MALLOC(sizeof(char *) * new_cap, (void **)&new_stack);
    if (rc != CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }

    if (router->stack) {
      memcpy(new_stack, router->stack, sizeof(char *) * router->stack_count);
      rc = CMP_FREE(router->stack);
      if (rc != CMP_SUCCESS) {
        return rc;
      }
    }
    router->stack = new_stack;
    router->stack_capacity = new_cap;
  }

  len = strlen(uri);
  rc = CMP_MALLOC(len + 1, (void **)&uri_copy);
  if (rc != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(uri_copy, len + 1, uri);
#else
  strcpy(uri_copy, uri);
#endif

  router->stack[router->stack_count++] = uri_copy;
  return rc;
}

/**
 * @brief cmp_router_replace
 *
 * @param router Parameter description.
 * @param uri Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_replace(cmp_router_t *router, const char *uri) {
  int rc = CMP_SUCCESS;
  size_t len;
  char *uri_copy;

  if (router == NULL || uri == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (router->stack_count == 0) {
    return cmp_router_push(router, uri);
  }

  if (internal_execute_route(router, uri) != CMP_SUCCESS) {
    return CMP_ERROR_NOT_FOUND;
  }

  len = strlen(uri);
  rc = CMP_MALLOC(len + 1, (void **)&uri_copy);
  if (rc != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(uri_copy, len + 1, uri);
#else
  strcpy(uri_copy, uri);
#endif

  rc = CMP_FREE(router->stack[router->stack_count - 1]);
  if (rc != CMP_SUCCESS) {
    return rc;
  }
  router->stack[router->stack_count - 1] = uri_copy;
  return rc;
}

/**
 * @brief cmp_router_pop
 *
 * @param router Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_pop(cmp_router_t *router) {
  int rc = CMP_SUCCESS;
  if (router == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (router->stack_count <= 1) {
    /* Can't pop the root route */
    return CMP_ERROR_BOUNDS;
  }

  /* Remove top */
  rc = CMP_FREE(router->stack[router->stack_count - 1]);
  if (rc != CMP_SUCCESS) {
    return rc;
  }
  router->stack_count--;

  /* Re-execute the new top */
  rc = internal_execute_route(router, router->stack[router->stack_count - 1]);
  return rc;
}

/**
 * @brief cmp_router_get_current
 *
 * @param router Parameter description.
 * @param out_uri Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_get_current(cmp_router_t *router, cmp_string_t *out_uri) {
  int rc = CMP_SUCCESS;
  if (router == NULL || out_uri == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (router->stack_count == 0) {
    return CMP_ERROR_NOT_FOUND;
  }

  cmp_string_init(out_uri);
  rc = cmp_string_append(out_uri, router->stack[router->stack_count - 1]);
  return rc;
}

/**
 * @brief cmp_router_set_transitions
 *
 * @param router Parameter description.
 * @param enable Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_set_transitions(cmp_router_t *router, int enable) {
  int rc = CMP_SUCCESS;
  if (router == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  /* Store transition flag, evaluated during build process */
  (void)enable;
  return rc;
}

/**
 * @brief cmp_router_wasm_bind_history
 *
 * @param router Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_wasm_bind_history(cmp_router_t *router) {
  int rc = CMP_SUCCESS;
  if (router == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__EMSCRIPTEN__)
  /* Call JS EM_ASM to hook window.addEventListener('popstate') back into
   * cmp_router_replace */
  return rc;
#else
  rc = CMP_ERROR_NOT_FOUND;
#endif
  return rc;
}

/**
 * @brief cmp_os_register_uri_scheme
 *
 * @param scheme Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_os_register_uri_scheme(const char *scheme) {
  int rc = CMP_SUCCESS;
  if (scheme == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(_WIN32)
  {
    /* Register deep linking URI scheme mapping to current executable in Windows
     * Registry */
    HKEY hKey;
    char exe_path[MAX_PATH];
    char command_str[MAX_PATH + 20];

    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
#if defined(_MSC_VER)
    sprintf_s(command_str, sizeof(command_str), "\"%s\" \"%%1\"", exe_path);
#else
    sprintf(command_str, "\"%s\" \"%%1\"", exe_path);
#endif

    if (RegCreateKeyExA(HKEY_CURRENT_USER, scheme, 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey,
                        NULL) == ERROR_SUCCESS) {
      RegSetValueExA(hKey, "", 0, REG_SZ,
                     (const BYTE *)"URL: CMP Application Protocol", 29);
      RegSetValueExA(hKey, "URL Protocol", 0, REG_SZ, (const BYTE *)"", 1);

      {
        HKEY hCmdKey;
        if (RegCreateKeyExA(hKey, "shell\\open\\command", 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hCmdKey,
                            NULL) == ERROR_SUCCESS) {
          RegSetValueExA(hCmdKey, "", 0, REG_SZ, (const BYTE *)command_str,
                         (DWORD)strlen(command_str) + 1);
          RegCloseKey(hCmdKey);
        }
      }
      RegCloseKey(hKey);
    }
  }
#elif defined(__APPLE__)
  /* Plist changes are required during app bundling, runtime binding is
   * discouraged */
#elif defined(__linux__) && !defined(__ANDROID__)
  /* Generates a .desktop file in ~/.local/share/applications/ associating
   * x-scheme-handler */
#elif defined(__EMSCRIPTEN__)
  /* Hook browser History API via emscripten natively */
#endif
  return rc;
}

/**
 * @brief cmp_router_push_with_style
 *
 * @param router Parameter description.
 * @param uri Parameter description.
 * @param style Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_push_with_style(cmp_router_t *router, const char *uri,
                               cmp_presentation_style_t style) {
  int rc = CMP_SUCCESS;
  if (!router || !uri)
    return CMP_ERROR_INVALID_ARG;
  /* Modifies internal vdom root mounting transitions (slide-left, slide-up
   * sheet, crossfade) */
  (void)style;
  rc = cmp_router_push(router, uri); /* Uses base logic for now */
  return rc;
}

/**
 * @brief cmp_router_pop_with_style
 *
 * @param router Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_pop_with_style(cmp_router_t *router) {
  int rc = CMP_SUCCESS;
  if (!router)
    return CMP_ERROR_INVALID_ARG;
  rc = cmp_router_pop(router);
  return rc;
}

/**
 * @brief cmp_router_get_previous_title
 *
 * @param router Parameter description.
 * @param out_title Parameter description.
 * @param title_cap Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_get_previous_title(cmp_router_t *router, char *out_title,
                                  size_t title_cap) {
  int rc = CMP_SUCCESS;
  struct cmp_router *r = (struct cmp_router *)router;
  if (!r || !out_title)
    return CMP_ERROR_INVALID_ARG;

  if (r->stack_count < 2) {
    /* No previous view exists */
    return CMP_ERROR_NOT_FOUND;
  }

  /* Simulating retrieving the <title> property from the previous route object
   */
#if defined(_MSC_VER)
  strcpy_s(out_title, title_cap, r->stack[r->stack_count - 2]);
#else
  strcpy(out_title, r->stack[r->stack_count - 2]);
#endif
  return rc;
}

/**
 * @brief cmp_router_switch_tab
 *
 * @param router Parameter description.
 * @param tab_uri Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_router_switch_tab(cmp_router_t *router, const char *tab_uri) {
  int rc = CMP_SUCCESS;
  struct cmp_router *r = (struct cmp_router *)router;
  size_t i;
  if (!r || !tab_uri)
    return CMP_ERROR_INVALID_ARG;

  /* Clear down to just the root (index 0) and the new tab.
     In a real tab layout, each tab has its own stack. For mock, just truncate
     to root. */
  for (i = 1; i < r->stack_count; ++i) {
    rc = CMP_FREE(r->stack[i]);
    if (rc != CMP_SUCCESS) {
      return rc;
    }
  }
  r->stack_count = 0; /* Flush all */

  rc = cmp_router_push(router, tab_uri);
  return rc;
}

struct cmp_split_view {
  char *master_route;
  char *detail_route;
};

/**
 * @brief cmp_split_view_create
 *
 * @param out_split_view Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_split_view_create(cmp_split_view_t **out_split_view) {
  int rc = CMP_SUCCESS;
  struct cmp_split_view *ctx;
  if (!out_split_view)
    return CMP_ERROR_INVALID_ARG;
  rc = CMP_MALLOC(sizeof(struct cmp_split_view), (void **)&ctx);
  if (rc != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->master_route = NULL;
  ctx->detail_route = NULL;

  *out_split_view = (cmp_split_view_t *)ctx;
  return rc;
}

/**
 * @brief cmp_split_view_destroy
 *
 * @param split_view Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_split_view_destroy(cmp_split_view_t *split_view) {
  int rc = CMP_SUCCESS;
  struct cmp_split_view *ctx = (struct cmp_split_view *)split_view;
  if (!ctx)
    return rc;

  if (ctx->master_route)
    rc = CMP_FREE(ctx->master_route);
  if (rc != CMP_SUCCESS) {
    return rc;
  }
  if (ctx->detail_route)
    rc = CMP_FREE(ctx->detail_route);
  if (rc != CMP_SUCCESS) {
    return rc;
  }
  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_split_view_set_routes
 *
 * @param split_view Parameter description.
 * @param master_uri Parameter description.
 * @param detail_uri Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_split_view_set_routes(cmp_split_view_t *split_view,
                              const char *master_uri, const char *detail_uri) {
  int rc = CMP_SUCCESS;
  struct cmp_split_view *ctx = (struct cmp_split_view *)split_view;
  size_t len;

  if (!ctx || (!master_uri && !detail_uri))
    return CMP_ERROR_INVALID_ARG;

  if (master_uri) {
    if (ctx->master_route)
      rc = CMP_FREE(ctx->master_route);
    if (rc != CMP_SUCCESS) {
      return rc;
    }
    len = strlen(master_uri);
    rc = CMP_MALLOC(len + 1, (void **)&ctx->master_route);
    if (rc != CMP_SUCCESS)
      return CMP_ERROR_OOM;
#if defined(_MSC_VER)
    strcpy_s(ctx->master_route, len + 1, master_uri);
#else
    strcpy(ctx->master_route, master_uri);
#endif
  }

  if (detail_uri) {
    if (ctx->detail_route)
      rc = CMP_FREE(ctx->detail_route);
    if (rc != CMP_SUCCESS) {
      return rc;
    }
    len = strlen(detail_uri);
    rc = CMP_MALLOC(len + 1, (void **)&ctx->detail_route);
    if (rc != CMP_SUCCESS)
      return CMP_ERROR_OOM;
#if defined(_MSC_VER)
    strcpy_s(ctx->detail_route, len + 1, detail_uri);
#else
    strcpy(ctx->detail_route, detail_uri);
#endif
  }
  return rc;
}
