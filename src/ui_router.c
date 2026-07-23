/* clang-format off */
#include "ui_web_bridge.h"
#include "ui_router.h"
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif
#include "ui_internal_mem.h"
#include "ui_event.h"

#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <emscripten/html5.h>
/* clang-format on */
#endif

#define UI_ROUTER_INITIAL_CAPACITY 8

struct ui_route_param {
  char *key;
  char *value;
};

/** \brief ui_route_request */
struct ui_route_request {
  char *path;
  struct ui_route_param *params;
  size_t params_size;
  struct ui_route_param *queries;
  size_t queries_size;
  void *state;
};

/** \brief ui_route */
struct ui_route {
  char *pattern;
  ui_route_factory_t factory;
  void *user_data;
};

/** \brief ui_router */
struct ui_router {
  struct ui_component **stack;
  size_t stack_capacity;
  size_t stack_size;

  struct ui_route *routes;
  size_t routes_capacity;
  size_t routes_size;
};

static void request_free(struct ui_route_request *req) {
  size_t i;
  if (!req)
    return;
  if (req->path)
    UI_FREE(req->path);
  if (req->params) {
    for (i = 0; i < req->params_size; ++i) {
      if (req->params[i].key)
        UI_FREE(req->params[i].key);
      if (req->params[i].value)
        UI_FREE(req->params[i].value);
    }
    UI_FREE(req->params);
  }
  if (req->queries) {
    for (i = 0; i < req->queries_size; ++i) {
      if (req->queries[i].key)
        UI_FREE(req->queries[i].key);
      if (req->queries[i].value)
        UI_FREE(req->queries[i].value);
    }
    UI_FREE(req->queries);
  }
  UI_FREE(req);
}

static enum ui_error internal_strdup(const char *src, char **out_str) {
  size_t len;
  char *copy;

  if (!src || !out_str)
    return UI_ERROR_INVALID_ARGUMENT;

  len = strlen(src);
  copy = (char *)UI_MALLOC(len + 1);
  if (!copy)
    return UI_ERROR_OUT_OF_MEMORY;

#if defined(_MSC_VER)
  strcpy_s(copy, len + 1, src);
#else
  strcpy(copy, src);
#endif

  *out_str = copy;
  return UI_ERROR_NONE;
}

static enum ui_error internal_strndup(const char *src, size_t n,
                                      char **out_str) {
  char *copy;

  if (!src || !out_str)
    return UI_ERROR_INVALID_ARGUMENT;

  copy = (char *)UI_MALLOC(n + 1);
  if (!copy)
    return UI_ERROR_OUT_OF_MEMORY;

#if defined(_MSC_VER)
  strncpy_s(copy, n + 1, src, n);
#else
  UI_STRNCPY(copy, n + 1, src, n);
  copy[n] = '\0';
#endif

  *out_str = copy;
  return UI_ERROR_NONE;
}

static int add_param(struct ui_route_param **params, size_t *size,
                     const char *key, size_t key_len, const char *value,
                     size_t val_len) {
  struct ui_route_param *new_params;
  char *k = NULL, *v = NULL;

  if (key_len > 0) {
    if (internal_strndup(key, key_len, &k) != UI_ERROR_NONE)
      return UI_ERROR_NONE;
  } else {
    if (internal_strdup("", &k) != UI_ERROR_NONE)
      return UI_ERROR_NONE;
  }

  if (val_len > 0) {
    if (internal_strndup(value, val_len, &v) != UI_ERROR_NONE) {
      UI_FREE(k);
      return UI_ERROR_NONE;
    }
  } else {
    if (internal_strdup("", &v) != UI_ERROR_NONE) {
      UI_FREE(k);
      return UI_ERROR_NONE;
    }
  }

  new_params = (struct ui_route_param *)UI_MALLOC(
      sizeof(struct ui_route_param) * (*size + 1));
  if (!new_params) {
    UI_FREE(k);
    UI_FREE(v);
    return UI_ERROR_NONE;
  }

  if (*size > 0) {
    memcpy(new_params, *params, sizeof(struct ui_route_param) * (*size));
    UI_FREE(*params);
  }

  new_params[*size].key = k;
  new_params[*size].value = v;
  *params = new_params;
  (*size)++;
  return 1;
}

static enum ui_error match_route(const char *pattern, const char *url,
                                 struct ui_route_request **out_req,
                                 int *out_match) {
  const char *p = pattern;
  const char *t = url;
  const char *param_start = NULL;
  const char *val_start = NULL;
  struct ui_route_request *req;
  if (!out_match)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_match = 0;

  req = (struct ui_route_request *)UI_MALLOC(sizeof(struct ui_route_request));
  if (!req)
    return UI_ERROR_OUT_OF_MEMORY;
  req->path = NULL;
  req->params = NULL;
  req->params_size = 0;
  req->queries = NULL;
  req->queries_size = 0;
  req->state = NULL;

  while (*p && *t && *t != '?') {
    if (*p == ':') {
      p++;
      param_start = p;
      while (*p && *p != '/') {
        p++;
      }
      val_start = t;
      while (*t && *t != '/' && *t != '?') {
        t++;
      }
      if (!add_param(&req->params, &req->params_size, param_start,
                     (size_t)(p - param_start), val_start,
                     (size_t)(t - val_start))) {
        request_free(req);
        return UI_ERROR_NONE;
      }
    } else if (*p == *t) {
      p++;
      t++;
    } else {
      request_free(req);
      return UI_ERROR_NONE;
    }
  }

  /* Pattern must be consumed, t must be at end or at '?' */
  if (*p != '\0') {
    request_free(req);
    return UI_ERROR_NONE;
  }

  if (*t != '\0' && *t != '?') {
    request_free(req);
    return UI_ERROR_NONE;
  }

  {
    const char *path_end = t;
    if (internal_strndup(url, (size_t)(path_end - url), &req->path) !=
        UI_ERROR_NONE) {
      request_free(req);
      return UI_ERROR_NONE;
    }
  }

  if (*t == '?') {
    t++;
    while (*t) {
      const char *k_start = t;
      const char *k_end = NULL;
      const char *v_start = NULL;
      const char *v_end = NULL;

      while (*t && *t != '=' && *t != '&') {
        t++;
      }
      k_end = t;

      if (*t == '=') {
        t++;
        v_start = t;
        while (*t && *t != '&') {
          t++;
        }
        v_end = t;
      } else {
        v_start = t;
        v_end = t;
      }

      if (!add_param(&req->queries, &req->queries_size, k_start,
                     (size_t)(k_end - k_start), v_start,
                     (size_t)(v_end - v_start))) {
        request_free(req);
        return UI_ERROR_NONE;
      }

      if (*t == '&') {
        t++;
      }
    }
  }

  *out_req = req;
  *out_match = 1;
  return UI_ERROR_NONE;
}

enum ui_error ui_route_request_get_param(const struct ui_route_request *req,
                                         const char *param_name,
                                         const char **out_param) {
  size_t i;
  if (!req || !param_name || !out_param)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_param = NULL;
  for (i = 0; i < req->params_size; ++i) {
    if (strcmp(req->params[i].key, param_name) == 0) {
      *out_param = req->params[i].value;
      return UI_ERROR_NONE;
    }
  }
  return UI_ERROR_NOT_FOUND;
}

enum ui_error ui_route_request_get_query(const struct ui_route_request *req,
                                         const char *query_name,
                                         const char **out_query) {
  size_t i;
  if (!req || !query_name || !out_query)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_query = NULL;
  for (i = 0; i < req->queries_size; ++i) {
    if (strcmp(req->queries[i].key, query_name) == 0) {
      *out_query = req->queries[i].value;
      return UI_ERROR_NONE;
    }
  }
  return UI_ERROR_NOT_FOUND;
}

enum ui_error ui_route_request_get_path(const struct ui_route_request *req,
                                        const char **out_path) {
  if (!req || !out_path)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_path = req->path;
  return UI_ERROR_NONE;
}

enum ui_error ui_router_create(struct ui_router **out_router) {
  struct ui_router *router;

  if (!out_router) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  router = (struct ui_router *)UI_MALLOC(sizeof(struct ui_router));
  if (!router) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  router->stack_capacity = UI_ROUTER_INITIAL_CAPACITY;
  router->stack_size = 0;
  router->stack = (struct ui_component **)UI_MALLOC(
      sizeof(struct ui_component *) * router->stack_capacity);

  if (!router->stack) {
    UI_FREE(router);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  router->routes_capacity = UI_ROUTER_INITIAL_CAPACITY;
  router->routes_size = 0;
  router->routes = (struct ui_route *)UI_MALLOC(sizeof(struct ui_route) *
                                                router->routes_capacity);

  if (!router->routes) {
    UI_FREE(router->stack);
    UI_FREE(router);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  *out_router = router;
  return UI_ERROR_NONE;
}

void ui_router_destroy(struct ui_router *router) {
  size_t i;
  if (!router) {
    return;
  }

  for (i = 0; i < router->stack_size; ++i) {
    if (router->stack[i]) {
      ui_component_destroy(router->stack[i]);
    }
  }

  if (router->stack) {
    UI_FREE(router->stack);
  }

  for (i = 0; i < router->routes_size; ++i) {
    if (router->routes[i].pattern) {
      UI_FREE(router->routes[i].pattern);
    }
  }

  if (router->routes) {
    UI_FREE(router->routes);
  }

  UI_FREE(router);
}

enum ui_error ui_router_add_route(struct ui_router *router, const char *pattern,
                                  ui_route_factory_t factory, void *user_data) {
  size_t len;
  char *p;
  struct ui_route *new_routes;
  size_t new_capacity;

  if (!router || !pattern || !factory) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (router->routes_size >= router->routes_capacity) {
    new_capacity = router->routes_capacity * 2;
    new_routes =
        (struct ui_route *)UI_MALLOC(sizeof(struct ui_route) * new_capacity);
    if (!new_routes) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    memcpy(new_routes, router->routes,
           sizeof(struct ui_route) * router->routes_size);
    UI_FREE(router->routes);
    router->routes = new_routes;
    router->routes_capacity = new_capacity;
  }

  len = strlen(pattern);
  p = (char *)UI_MALLOC(len + 1);
  if (!p) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  strcpy_s(p, len + 1, pattern);
#else
  strcpy(p, pattern);
#endif

  router->routes[router->routes_size].pattern = p;
  router->routes[router->routes_size].factory = factory;
  router->routes[router->routes_size].user_data = user_data;
  router->routes_size++;

  return UI_ERROR_NONE;
}

enum ui_error ui_router_navigate_with_state(struct ui_router *router,
                                            const char *path, void *state) {
  size_t i;
  struct ui_component *screen = NULL;
  struct ui_route_request *req = NULL;
  enum ui_error rc;

  if (!router || !path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < router->routes_size; i++) {
    int is_match = 0;
    (void)match_route(router->routes[i].pattern, path, &req, &is_match);
    if (is_match) {
      req->state = state;
      rc = router->routes[i].factory(req, router->routes[i].user_data, &screen);
      request_free(req);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

#if defined(__EMSCRIPTEN__)
      {
        char script[1024];
#if defined(_MSC_VER)
        sprintf_s(
            script, sizeof(script),
            "if(window.history) window.history.pushState(null, '', '%s');",
            path);
#else
        /* Using safe formatting */
        sprintf(script,
                "if(window.history) window.history.pushState(null, '', '%s');",
                path);
#endif
        emscripten_run_script(script);
      }
#endif

      return ui_router_push(router, screen);
    }
  }

  return UI_ERROR_NOT_FOUND;
}

enum ui_error ui_router_push(struct ui_router *router,
                             struct ui_component *screen) {
  struct ui_component **new_stack;
  size_t new_capacity;

  if (!router || !screen) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (router->stack_size >= router->stack_capacity) {
    new_capacity = router->stack_capacity * 2;
    new_stack = (struct ui_component **)UI_MALLOC(
        sizeof(struct ui_component *) * new_capacity);
    if (!new_stack) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    memcpy(new_stack, router->stack,
           sizeof(struct ui_component *) * router->stack_size);
    UI_FREE(router->stack);
    router->stack = new_stack;
    router->stack_capacity = new_capacity;
  }

  router->stack[router->stack_size++] = screen;

  return UI_ERROR_NONE;
}

enum ui_error ui_router_pop(struct ui_router *router) {
  if (!router) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (router->stack_size == 0) {
    return UI_ERROR_QUEUE_EMPTY;
  }

  router->stack_size--;
  ui_component_destroy(router->stack[router->stack_size]);
  router->stack[router->stack_size] = NULL;

  return UI_ERROR_NONE;
}

enum ui_error ui_router_replace(struct ui_router *router,
                                struct ui_component *screen) {
  if (!router || !screen) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (router->stack_size > 0) {
    ui_component_destroy(router->stack[router->stack_size - 1]);
    router->stack[router->stack_size - 1] = screen;
  } else {
    return ui_router_push(router, screen);
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_router_get_current(struct ui_router *router,
                                    struct ui_component **out_current) {
  if (!router || !out_current) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (router->stack_size == 0) {
    *out_current = NULL;
    return UI_ERROR_NOT_FOUND;
  }

  *out_current = router->stack[router->stack_size - 1];
  return UI_ERROR_NONE;
}

enum ui_error ui_router_process_event(struct ui_router *router,
                                      const struct ui_event *event) {
  if (!router || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (event->type == UI_EVENT_OS_DEEP_LINK) {
    return ui_router_navigate(router, event->event_data.deep_link.uri);
  }

  /* Other routing events could be handled here */
  return UI_ERROR_NONE;
}

#if defined(__EMSCRIPTEN__)

#endif

enum ui_error ui_router_install_os_hooks(struct ui_router *router) {
  if (!router) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(__EMSCRIPTEN__)
  (void)router;
#endif

  return UI_ERROR_NONE;
}

enum ui_error ui_route_request_get_state(const struct ui_route_request *req,
                                         void **out_state) {
  if (!req || !out_state)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_state = req->state;
  return UI_ERROR_NONE;
}

enum ui_error ui_router_navigate(struct ui_router *router, const char *path) {
  return ui_router_navigate_with_state(router, path, NULL);
}
