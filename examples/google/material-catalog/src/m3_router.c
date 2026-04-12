/* clang-format off */
#include "m3_router.h"
#include <string.h>
/* clang-format on */

int m3_router_init(m3_router_engine_t *engine) {
  if (!engine) {
    return 1;
  }
  engine->stack_size = 0;
  engine->os_back_intercepted = 0;
  return 0;
}

int m3_router_cleanup(m3_router_engine_t *engine) {
  if (!engine) {
    return 1;
  }
  engine->stack_size = 0;
  return 0;
}

int m3_router_push(m3_router_engine_t *engine, catalog_screen_id_t screen_id,
                   int component_id, int example_index, void *args,
                   m3_route_transition_t transition) {
  m3_route_t *route;
  if (!engine) {
    return 1;
  }
  if (engine->stack_size >= CATALOG_MAX_BACK_STACK) {
    return 1; /* Stack overflow */
  }

  route = &engine->stack[engine->stack_size++];
  route->screen_id = screen_id;
  route->component_id = component_id;
  route->example_index = example_index;
  route->args = args;
  route->transition = transition;
  route->saved_scroll_x = 0.0f;
  route->saved_scroll_y = 0.0f;

  return 0;
}

int m3_router_pop(m3_router_engine_t *engine) {
  if (!engine) {
    return 1;
  }
  if (engine->stack_size <= 1) {
    return 1; /* Cannot pop root */
  }

  engine->stack_size--;
  return 0;
}

int m3_router_replace(m3_router_engine_t *engine, catalog_screen_id_t screen_id,
                      int component_id, int example_index, void *args,
                      m3_route_transition_t transition) {
  m3_route_t *route;
  if (!engine) {
    return 1;
  }
  if (engine->stack_size == 0) {
    return m3_router_push(engine, screen_id, component_id, example_index, args,
                          transition);
  }

  route = &engine->stack[engine->stack_size - 1];
  route->screen_id = screen_id;
  route->component_id = component_id;
  route->example_index = example_index;
  route->args = args;
  route->transition = transition;
  route->saved_scroll_x = 0.0f;
  route->saved_scroll_y = 0.0f;

  return 0;
}

int m3_router_reset_to(m3_router_engine_t *engine,
                       catalog_screen_id_t screen_id) {
  if (!engine) {
    return 1;
  }
  engine->stack_size = 0;
  return m3_router_push(engine, screen_id, 0, 0, NULL, M3_TRANSITION_NONE);
}

int m3_router_can_pop(const m3_router_engine_t *engine, int *out_can_pop) {
  if (!engine || !out_can_pop) {
    return 1;
  }
  *out_can_pop = (engine->stack_size > 1) ? 1 : 0;
  return 0;
}

int m3_router_save_scroll_state(m3_router_engine_t *engine, float scroll_x,
                                float scroll_y) {
  if (!engine || engine->stack_size == 0) {
    return 1;
  }
  engine->stack[engine->stack_size - 1].saved_scroll_x = scroll_x;
  engine->stack[engine->stack_size - 1].saved_scroll_y = scroll_y;
  return 0;
}

int m3_router_get_scroll_state(const m3_router_engine_t *engine,
                               float *out_scroll_x, float *out_scroll_y) {
  if (!engine || !out_scroll_x || !out_scroll_y || engine->stack_size == 0) {
    return 1;
  }
  *out_scroll_x = engine->stack[engine->stack_size - 1].saved_scroll_x;
  *out_scroll_y = engine->stack[engine->stack_size - 1].saved_scroll_y;
  return 0;
}

int m3_router_handle_deep_link(m3_router_engine_t *engine, const char *url) {
  /* Example implementation:
     cmp://catalog/button/filled
     For simplicity, parsing mock urls based on typical M3 navigation structure
   */

  if (!engine || !url) {
    return 1;
  }

  if (strncmp(url, "cmp://catalog/", 14) != 0) {
    return 1; /* Not a valid deep link for this app */
  }

  if (strcmp(url, "cmp://catalog/home") == 0) {
    return m3_router_reset_to(engine, CATALOG_SCREEN_HOME);
  } else if (strncmp(url, "cmp://catalog/component/", 24) == 0) {
    /* MOCK: Convert remainder to int component_id */
    int component_id = 0; /* Fallback for test */
    return m3_router_push(engine, CATALOG_SCREEN_COMPONENT_DETAILS,
                          component_id, 0, NULL, M3_TRANSITION_SHARED_AXIS_Z);
  }

  return 1;
}