/*
 * \file ui_router.c
 * \brief Implementation of the UI Router component.
 */

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
#endif
/* clang-format on */

/* \brief Initial capacity for router stacks and route arrays */
#define UI_ROUTER_INITIAL_CAPACITY 8

/**
 * @struct ui_route_param
 * \brief Key-value pair for route parameters and queries.
 */
struct ui_route_param {
  char *key;   /**< Parameter name */
  char *value; /**< Parameter value */
};

/**
 * @struct ui_route_request
 * \brief Internal structure representing a route request.
 */
struct ui_route_request {
  char *path;                     /**< Matched path */
  struct ui_route_param *params;  /**< Path parameters */
  size_t params_size;             /**< Number of path parameters */
  struct ui_route_param *queries; /**< Query parameters */
  size_t queries_size;            /**< Number of query parameters */
  void *state;                    /**< Passed state */
};

/**
 * @struct ui_route
 * \brief Internal structure mapping a route pattern to a factory.
 */
struct ui_route {
  char *pattern;              /**< Route pattern string */
  ui_route_factory_t factory; /**< Screen factory function */
  void *user_data;            /**< Factory user data */
};

/**
 * @struct ui_router
 * \brief Internal structure representing a router manager.
 */
struct ui_router {
  struct ui_component **stack; /**< Stack of active screens */
  size_t stack_capacity;       /**< Capacity of screen stack */
  size_t stack_size;           /**< Current number of screens */

  struct ui_route *routes; /**< Array of registered routes */
  size_t routes_capacity;  /**< Capacity of routes array */
  size_t routes_size;      /**< Current number of routes */
};

/*
 * \brief Frees a route request and its parameters.
 *
 * \param req The request to free.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t request_free(struct ui_route_request *req) {
  size_t i;
  if (!req)
    return UI_ERROR_NONE;
  if (req->path)
    C_MULTIPLATFORM_FREE(req->path);
  if (req->params) {
    for (i = 0; i < req->params_size; ++i) {
      if (req->params[i].key)
        C_MULTIPLATFORM_FREE(req->params[i].key);
      if (req->params[i].value)
        C_MULTIPLATFORM_FREE(req->params[i].value);
    }
    C_MULTIPLATFORM_FREE(req->params);
  }
  if (req->queries) {
    for (i = 0; i < req->queries_size; ++i) {
      if (req->queries[i].key)
        C_MULTIPLATFORM_FREE(req->queries[i].key);
      if (req->queries[i].value)
        C_MULTIPLATFORM_FREE(req->queries[i].value);
    }
    C_MULTIPLATFORM_FREE(req->queries);
  }
  C_MULTIPLATFORM_FREE(req);
  return UI_ERROR_NONE;
}

/*
 * \brief Copies a string up to n characters safely.
 *
 * \param src Source string.
 * \param n Max characters to copy.
 * \param out_str Pointer to receive the allocated string.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t internal_strndup(const char *src, size_t n, char **out_str) {
  char *copy;

  if (!src || !out_str)
    return UI_ERROR_INVALID_ARGUMENT;

  copy = (char *)C_MULTIPLATFORM_MALLOC(n + 1);
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

/*
 * \brief Appends a key-value pair to a parameter array.
 *
 * \param params Pointer to the array of parameters.
 * \param size Pointer to the size of the array.
 * \param key Key string.
 * \param key_len Length of key string.
 * \param value Value string.
 * \param val_len Length of value string.
 * \return 1 on success, 0 on failure.
 */
static int add_param(struct ui_route_param **params, size_t *size,
                     const char *key, size_t key_len, const char *value,
                     size_t val_len) {
  struct ui_route_param *new_params;
  char *k = NULL, *v = NULL;

  if (key_len > 0) {
    if (internal_strndup(key, key_len, &k) != UI_ERROR_NONE)
      return 0;
  } else {
    k = C_MULTIPLATFORM_STRDUP("");
    if (!k)
      return 0;
  }

  if (val_len > 0) {
    if (internal_strndup(value, val_len, &v) != UI_ERROR_NONE) {
      C_MULTIPLATFORM_FREE(k);
      return 0;
    }
  } else {
    v = C_MULTIPLATFORM_STRDUP("");
    if (!v) {
      C_MULTIPLATFORM_FREE(k);
      return 0;
    }
  }

  new_params = (struct ui_route_param *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_route_param) * (size_t)(*size + 1));
  if (!new_params) {
    C_MULTIPLATFORM_FREE(k);
    C_MULTIPLATFORM_FREE(v);
    return 0;
  }

  if (*size > 0) {
    memcpy(new_params, *params,
           sizeof(struct ui_route_param) * (size_t)(*size));
    C_MULTIPLATFORM_FREE(*params);
  }

  new_params[*size].key = k;
  new_params[*size].value = v;
  *params = new_params;
  (*size)++;
  return 1;
}

/*
 * \brief Matches a URL against a route pattern, extracting parameters.
 *
 * \param pattern The route pattern.
 * \param url The requested URL.
 * \param out_req Pointer to receive the populated request if matched.
 * \param out_match Set to 1 if matched, 0 otherwise.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t match_route(const char *pattern, const char *url,
                              struct ui_route_request **out_req,
                              int *out_match) {
  const char *p = pattern;
  const char *t = url;
  const char *param_start = NULL;
  const char *val_start = NULL;
  struct ui_route_request *req;

  if (!pattern || !url || !out_req || !out_match)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_match = 0;

  req = (struct ui_route_request *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_route_request));
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
        (void)request_free(req);
        return UI_ERROR_NONE;
      }
    } else if (*p == *t) {
      p++;
      t++;
    } else {
      (void)request_free(req);
      return UI_ERROR_NONE;
    }
  }

  /* Pattern must be consumed, t must be at end or at '?' */
  if (*p != '\0') {
    (void)request_free(req);
    return UI_ERROR_NONE;
  }

  if (*t != '\0' && *t != '?') {
    (void)request_free(req);
    return UI_ERROR_NONE;
  }

  {
    const char *path_end = t;
    if (internal_strndup(url, (size_t)(path_end - url), &req->path) !=
        UI_ERROR_NONE) {
      (void)request_free(req);
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
        (void)request_free(req);
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

/*
 * \brief Gets a path parameter (e.g. from "/settings/:id") by name.
 *
 * \param req The route request.
 * \param param_name The name of the parameter (e.g. "id").
 * \param out_param Pointer to receive the parameter value, or NULL if not
 * found.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_route_request_get_param(const struct ui_route_request *req,
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
  return UI_ERROR_NONE;
}

/*
 * \brief Gets a query string parameter (e.g. from "?tab=2") by name.
 *
 * \param req The route request.
 * \param query_name The name of the query parameter (e.g. "tab").
 * \param out_query Pointer to receive the query value, or NULL if not found.
 * \return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if not present,
 *         or an appropriate error code.
 */
ui_error_t ui_route_request_get_query(const struct ui_route_request *req,
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

/*
 * \brief Gets the exact path string that was requested (excluding query
 * string).
 *
 * \param req The route request.
 * \param out_path Pointer to receive the path string.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_route_request_get_path(const struct ui_route_request *req,
                                     const char **out_path) {
  if (!req || !out_path)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_path = req->path;
  return UI_ERROR_NONE;
}

/*
 * \brief Gets the custom state pointer passed during navigation.
 *
 * \param req The route request.
 * \param out_state Pointer to receive the state, or NULL if none.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_route_request_get_state(const struct ui_route_request *req,
                                      void **out_state) {
  if (!req || !out_state)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_state = req->state;
  return UI_ERROR_NONE;
}

/*
 * \brief Creates a new screen manager (router) navigation stack.
 *
 * \param out_router Pointer to receive the allocated router.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_router_create(struct ui_router **out_router) {
  struct ui_router *router;

  if (!out_router) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  router = (struct ui_router *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_router));
  if (!router) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  router->stack_capacity = UI_ROUTER_INITIAL_CAPACITY;
  router->stack_size = 0;
  router->stack = (struct ui_component **)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_component *) * (size_t)router->stack_capacity);

  if (!router->stack) {
    C_MULTIPLATFORM_FREE(router);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  router->routes_capacity = UI_ROUTER_INITIAL_CAPACITY;
  router->routes_size = 0;
  router->routes = (struct ui_route *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_route) * (size_t)router->routes_capacity);

  if (!router->routes) {
    C_MULTIPLATFORM_FREE(router->stack);
    C_MULTIPLATFORM_FREE(router);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  *out_router = router;
  return UI_ERROR_NONE;
}

/*
 * \brief Destroys a router and all screens in its stack.
 *
 * \param router The router to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_router_destroy(struct ui_router *router) {
  size_t i;
  if (!router) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < router->stack_size; ++i) {
    (void)ui_component_destroy(router->stack[i]);
  }

  C_MULTIPLATFORM_FREE(router->stack);

  for (i = 0; i < router->routes_size; ++i) {
    C_MULTIPLATFORM_FREE(router->routes[i].pattern);
  }

  C_MULTIPLATFORM_FREE(router->routes);

  C_MULTIPLATFORM_FREE(router);
  return UI_ERROR_NONE;
}

/*
 * \brief Registers a route pattern mapping to a screen factory.
 *
 * \param router The router instance.
 * \param pattern The route pattern (e.g., "/settings/profile/:id").
 * \param factory The factory function to build the screen.
 * \param user_data Opaque data passed to the factory.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_router_add_route(struct ui_router *router, const char *pattern,
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
    new_routes = (struct ui_route *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_route) * (size_t)new_capacity);
    if (!new_routes) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    memcpy(new_routes, router->routes,
           sizeof(struct ui_route) * (size_t)router->routes_size);
    C_MULTIPLATFORM_FREE(router->routes);
    router->routes = new_routes;
    router->routes_capacity = new_capacity;
  }

  len = strlen(pattern);
  p = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
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

/*
 * \brief Navigates to a specific URL with an optional state payload.
 *
 * \param router The router instance.
 * \param path The URL path to navigate to.
 * \param state Opaque user state (e.g. form group pointer).
 * \return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if no matching route is
 * found, or an appropriate error code.
 */
ui_error_t ui_router_navigate_with_state(struct ui_router *router,
                                         const char *path, void *state) {
  size_t i;
  struct ui_component *screen = NULL;
  struct ui_route_request *req = NULL;
  ui_error_t rc;

  if (!router || !path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < router->routes_size; i++) {
    int is_match = 0;
    (void)match_route(router->routes[i].pattern, path, &req, &is_match);
    if (is_match) {
      req->state = state;
      rc = router->routes[i].factory(req, router->routes[i].user_data, &screen);
      (void)request_free(req);
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

/*
 * \brief Navigates to a specific URL by matching it against registered routes
 * and pushing the resulting screen.
 *
 * \param router The router instance.
 * \param path The URL path to navigate to (e.g., "/settings/profile/123").
 * \return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if no matching route is
 * found, or an appropriate error code.
 */
ui_error_t ui_router_navigate(struct ui_router *router, const char *path) {
  return ui_router_navigate_with_state(router, path, NULL);
}

/*
 * \brief Pushes a new screen component onto the navigation stack.
 *
 * \param router The router.
 * \param screen The screen component to push.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_router_push(struct ui_router *router,
                          struct ui_component *screen) {
  struct ui_component **new_stack;
  size_t new_capacity;

  if (!router || !screen) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (router->stack_size >= router->stack_capacity) {
    new_capacity = router->stack_capacity * 2;
    new_stack = (struct ui_component **)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_component *) * (size_t)new_capacity);
    if (!new_stack) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    memcpy(new_stack, router->stack,
           sizeof(struct ui_component *) * (size_t)router->stack_size);
    C_MULTIPLATFORM_FREE(router->stack);
    router->stack = new_stack;
    router->stack_capacity = new_capacity;
  }

  router->stack[router->stack_size++] = screen;

  return UI_ERROR_NONE;
}

/*
 * \brief Pops the top screen from the navigation stack and destroys it.
 *
 * \param router The router.
 * \return UI_ERROR_NONE on success, UI_ERROR_QUEUE_EMPTY if the stack is empty.
 */
ui_error_t ui_router_pop(struct ui_router *router) {
  if (!router) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (router->stack_size == 0) {
    return UI_ERROR_QUEUE_EMPTY;
  }

  router->stack_size--;
  (void)ui_component_destroy(router->stack[router->stack_size]);
  router->stack[router->stack_size] = NULL;

  return UI_ERROR_NONE;
}

/*
 * \brief Replaces the current top screen with a new screen component.
 *
 * \param router The router.
 * \param screen The new screen component.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_router_replace(struct ui_router *router,
                             struct ui_component *screen) {
  if (!router || !screen) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (router->stack_size > 0) {
    (void)ui_component_destroy(router->stack[router->stack_size - 1]);
    router->stack[router->stack_size - 1] = screen;
  } else {
    return ui_router_push(router, screen);
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Gets the current top screen from the navigation stack.
 *
 * \param router The router.
 * \param out_current Pointer to receive the current screen component, or NULL
 * if the stack is empty.
 * \return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if empty.
 */
ui_error_t ui_router_get_current(struct ui_router *router,
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

/*
 * \brief Processes an OS event, looking for deep link events to automatically
 * navigate.
 *
 * \param router The router instance.
 * \param event The event to process.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_router_process_event(struct ui_router *router,
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

/*
 * \brief Installs OS-level integration for the router (e.g., HTML5 History API
 * for Emscripten).
 *
 * \param router The router instance.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_router_install_os_hooks(struct ui_router *router) {
  if (!router) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(__EMSCRIPTEN__)
  /* Emscripten-specific setup would go here if needed */
  (void)router;
#endif

  return UI_ERROR_NONE;
}
