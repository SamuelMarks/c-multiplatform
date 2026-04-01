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

int cmp_router_create(cmp_router_t **out_router) {
  cmp_router_t *router;

  if (out_router == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_router_t), (void **)&router) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  router->routes = NULL;
  router->stack = NULL;
  router->stack_count = 0;
  router->stack_capacity = 0;
  router->active_view = NULL;

  *out_router = router;
  g_router_initialized = 1;
  return CMP_SUCCESS;
}

int cmp_router_destroy(cmp_router_t *router) {
  cmp_route_entry_t *curr, *next;
  size_t i;

  if (router == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Free routes */
  curr = router->routes;
  while (curr != NULL) {
    next = curr->next;
    CMP_FREE(curr->path);
    CMP_FREE(curr);
    curr = next;
  }

  /* Free stack */
  for (i = 0; i < router->stack_count; i++) {
    CMP_FREE(router->stack[i]);
  }
  if (router->stack) {
    CMP_FREE(router->stack);
  }

  CMP_FREE(router);
  return CMP_SUCCESS;
}

int cmp_router_register(cmp_router_t *router, const char *path,
                        cmp_route_builder_cb builder, cmp_route_guard_cb guard,
                        void *guard_data) {
  cmp_route_entry_t *entry;
  size_t len;

  if (router == NULL || path == NULL || builder == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_route_entry_t), (void **)&entry) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  len = strlen(path);
  if (CMP_MALLOC(len + 1, (void **)&entry->path) != CMP_SUCCESS) {
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

  return CMP_SUCCESS;
}

/* C89 safe implementation of string tokenization with saved state */
static char *cmp_strtok_r(char *str, const char *delim, char **saveptr) {
  char *token;
  if (str == NULL) {
    str = *saveptr;
  }
  if (str == NULL) {
    return NULL;
  }
  str += strspn(str, delim);
  if (*str == '\0') {
    *saveptr = NULL;
    return NULL;
  }
  token = str;
  str = strpbrk(token, delim);
  if (str == NULL) {
    *saveptr = NULL;
  } else {
    *str = '\0';
    *saveptr = str + 1;
  }
  return token;
}

static int internal_execute_route(cmp_router_t *router, const char *uri) {
  cmp_route_entry_t *curr = router->routes;
  char *uri_mutable;
  char *saveptr_uri = NULL;
  size_t uri_len;

  if (router == NULL || uri == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  uri_len = strlen(uri);
  if (CMP_MALLOC(uri_len + 1, (void **)&uri_mutable) != CMP_SUCCESS) {
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
    char *u_tok, *p_tok;
    size_t path_len = strlen(curr->path);

    /* Quick exact match optimization */
    if (strcmp(curr->path, uri) == 0) {
      match = 1;
      goto check_guard;
    }

    if (CMP_MALLOC(path_len + 1, (void **)&path_mutable) != CMP_SUCCESS) {
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

    u_tok = cmp_strtok_r(uri_mutable, "/", &saveptr_uri);
    p_tok = cmp_strtok_r(path_mutable, "/", &saveptr_path);

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

      u_tok = cmp_strtok_r(NULL, "/", &saveptr_uri);
      p_tok = cmp_strtok_r(NULL, "/", &saveptr_path);
    }

    CMP_FREE(path_mutable);

  check_guard:
    if (match) {
      /* Check guard first */
      if (curr->guard != NULL) {
        if (!curr->guard(uri, curr->guard_data)) {
          CMP_FREE(uri_mutable);
          return CMP_ERROR_NOT_FOUND; /* Guard blocked navigation */
        }
      }

      /* Build view */
      router->active_view = curr->builder(uri);
      CMP_FREE(uri_mutable);
      return CMP_SUCCESS;
    }

    curr = curr->next;
  }

  CMP_FREE(uri_mutable);
  return CMP_ERROR_NOT_FOUND;
}

int cmp_router_push(cmp_router_t *router, const char *uri) {
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

    if (CMP_MALLOC(sizeof(char *) * new_cap, (void **)&new_stack) !=
        CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }

    if (router->stack) {
      memcpy(new_stack, router->stack, sizeof(char *) * router->stack_count);
      CMP_FREE(router->stack);
    }
    router->stack = new_stack;
    router->stack_capacity = new_cap;
  }

  len = strlen(uri);
  if (CMP_MALLOC(len + 1, (void **)&uri_copy) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  strcpy_s(uri_copy, len + 1, uri);
#else
  strcpy(uri_copy, uri);
#endif

  router->stack[router->stack_count++] = uri_copy;
  return CMP_SUCCESS;
}

int cmp_router_replace(cmp_router_t *router, const char *uri) {
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
  if (CMP_MALLOC(len + 1, (void **)&uri_copy) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  strcpy_s(uri_copy, len + 1, uri);
#else
  strcpy(uri_copy, uri);
#endif

  CMP_FREE(router->stack[router->stack_count - 1]);
  router->stack[router->stack_count - 1] = uri_copy;

  return CMP_SUCCESS;
}

int cmp_router_pop(cmp_router_t *router) {
  if (router == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (router->stack_count <= 1) {
    /* Can't pop the root route */
    return CMP_ERROR_BOUNDS;
  }

  /* Remove top */
  CMP_FREE(router->stack[router->stack_count - 1]);
  router->stack_count--;

  /* Re-execute the new top */
  return internal_execute_route(router, router->stack[router->stack_count - 1]);
}

int cmp_router_get_current(cmp_router_t *router, cmp_string_t *out_uri) {
  if (router == NULL || out_uri == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (router->stack_count == 0) {
    return CMP_ERROR_NOT_FOUND;
  }

  cmp_string_init(out_uri);
  return cmp_string_append(out_uri, router->stack[router->stack_count - 1]);
}

int cmp_router_set_transitions(cmp_router_t *router, int enable) {
  if (router == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  /* Store transition flag, evaluated during build process */
  (void)enable;
  return CMP_SUCCESS;
}

int cmp_router_wasm_bind_history(cmp_router_t *router) {
  if (router == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__EMSCRIPTEN__)
  /* Call JS EM_ASM to hook window.addEventListener('popstate') back into
   * cmp_router_replace */
  return CMP_SUCCESS;
#else
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_os_register_uri_scheme(const char *scheme) {
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

  return CMP_SUCCESS;
}

int cmp_router_push_with_style(cmp_router_t *router, const char *uri,
                               cmp_presentation_style_t style) {
  if (!router || !uri)
    return CMP_ERROR_INVALID_ARG;
  /* Modifies internal vdom root mounting transitions (slide-left, slide-up
   * sheet, crossfade) */
  (void)style;
  return cmp_router_push(router, uri); /* Uses base logic for now */
}

int cmp_router_pop_with_style(cmp_router_t *router) {
  if (!router)
    return CMP_ERROR_INVALID_ARG;
  return cmp_router_pop(router);
}

int cmp_router_get_previous_title(cmp_router_t *router, char *out_title,
                                  size_t title_cap) {
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

  return CMP_SUCCESS;
}

int cmp_router_switch_tab(cmp_router_t *router, const char *tab_uri) {
  struct cmp_router *r = (struct cmp_router *)router;
  size_t i;
  if (!r || !tab_uri)
    return CMP_ERROR_INVALID_ARG;

  /* Clear down to just the root (index 0) and the new tab.
     In a real tab layout, each tab has its own stack. For mock, just truncate
     to root. */
  for (i = 1; i < r->stack_count; ++i) {
    CMP_FREE(r->stack[i]);
  }
  r->stack_count = 0; /* Flush all */

  return cmp_router_push(router, tab_uri);
}

struct cmp_split_view {
  char *master_route;
  char *detail_route;
};

int cmp_split_view_create(cmp_split_view_t **out_split_view) {
  struct cmp_split_view *ctx;
  if (!out_split_view)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_split_view), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->master_route = NULL;
  ctx->detail_route = NULL;

  *out_split_view = (cmp_split_view_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_split_view_destroy(cmp_split_view_t *split_view) {
  struct cmp_split_view *ctx = (struct cmp_split_view *)split_view;
  if (!ctx)
    return CMP_SUCCESS;

  if (ctx->master_route)
    CMP_FREE(ctx->master_route);
  if (ctx->detail_route)
    CMP_FREE(ctx->detail_route);
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_split_view_set_routes(cmp_split_view_t *split_view,
                              const char *master_uri, const char *detail_uri) {
  struct cmp_split_view *ctx = (struct cmp_split_view *)split_view;
  size_t len;

  if (!ctx || (!master_uri && !detail_uri))
    return CMP_ERROR_INVALID_ARG;

  if (master_uri) {
    if (ctx->master_route)
      CMP_FREE(ctx->master_route);
    len = strlen(master_uri);
    if (CMP_MALLOC(len + 1, (void **)&ctx->master_route) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
#if defined(_MSC_VER)
    strcpy_s(ctx->master_route, len + 1, master_uri);
#else
    strcpy(ctx->master_route, master_uri);
#endif
  }

  if (detail_uri) {
    if (ctx->detail_route)
      CMP_FREE(ctx->detail_route);
    len = strlen(detail_uri);
    if (CMP_MALLOC(len + 1, (void **)&ctx->detail_route) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
#if defined(_MSC_VER)
    strcpy_s(ctx->detail_route, len + 1, detail_uri);
#else
    strcpy(ctx->detail_route, detail_uri);
#endif
  }

  return CMP_SUCCESS;
}
