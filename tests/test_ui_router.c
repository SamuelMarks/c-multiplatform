/* clang-format off */
#include "ui_router.h"
#include "ui_event.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static enum ui_error mock_factory_success(const struct ui_route_request *req,
                                          void *user_data,
                                          struct ui_component **out_screen) {
  int *call_count = (int *)user_data;
  const char *id;
  const char *tab;

  if (call_count)
    (*call_count)++;

  if (ui_route_request_get_param(req, "id", &id) != UI_ERROR_NONE)
    id = NULL;
  if (ui_route_request_get_query(req, "tab", &tab) != UI_ERROR_NONE)
    tab = NULL;

  /* Optional id matching */
  if (id && strcmp(id, "123") != 0) {
    return UI_ERROR_INVALID_ARGUMENT;
    return UI_ERROR_NONE;
  }

  /* Optional tab matching */
  if (tab && strcmp(tab, "2") != 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_component_create(out_screen);
  return UI_ERROR_NONE;
}

static enum ui_error mock_factory_fail(const struct ui_route_request *req,
                                       void *user_data,
                                       struct ui_component **out_screen) {
  return UI_ERROR_UNKNOWN;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_router *router = NULL;
  struct ui_component *screen1 = NULL;
  struct ui_component *screen2 = NULL;
  struct ui_component *screen3 = NULL;
  struct ui_component *current = NULL;
  struct ui_event ev;
  enum ui_error rc;
  int factory_calls = 0;

  printf("Testing invalid arguments...\n");
  if (ui_router_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_router_destroy(NULL); /* Should not crash */
  if (ui_router_add_route(NULL, "/a", mock_factory_success, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_router_create(&router);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_router_add_route(router, NULL, mock_factory_success, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_router_add_route(router, "/a", NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_router_navigate(NULL, "/a") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_router_navigate(router, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_router_push(NULL, screen1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_router_push(router, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_router_pop(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_router_replace(NULL, screen1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_router_replace(router, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    struct ui_component *tmp_comp;
    if (ui_router_get_current(NULL, &tmp_comp) == UI_ERROR_NONE)
      return 1;
  }
  if (ui_router_process_event(NULL, &ev) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_router_process_event(router, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_router_install_os_hooks(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    const char *tmp_str;
    if (ui_route_request_get_param(NULL, "a", &tmp_str) == UI_ERROR_NONE)
      return 1;
    if (ui_route_request_get_query(NULL, "a", &tmp_str) == UI_ERROR_NONE)
      return 1;
    if (ui_route_request_get_path(NULL, &tmp_str) == UI_ERROR_NONE)
      return 1;
  }

  rc = ui_component_create(&screen1);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create screen1\n");
    return 1;
  }

  rc = ui_component_create(&screen2);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create screen2\n");
    return 1;
  }

  rc = ui_component_create(&screen3);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create screen3\n");
    return 1;
  }

  printf("Testing ui_router_push...\n");
  rc = ui_router_push(router, screen1);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to push screen1\n");
    return 1;
  }

  rc = ui_router_get_current(router, &current);
  if (rc != UI_ERROR_NONE || current != screen1) {
    printf("Expected screen1 to be current\n");
    return 1;
  }

  rc = ui_router_push(router, screen2);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to push screen2\n");
    return 1;
  }

  rc = ui_router_get_current(router, &current);
  if (rc != UI_ERROR_NONE || current != screen2) {
    printf("Expected screen2 to be current\n");
    return 1;
  }

  printf("Testing ui_router_replace...\n");
  rc = ui_router_replace(router, screen3);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to replace with screen3\n");
    return 1;
  }

  rc = ui_router_get_current(router, &current);
  if (rc != UI_ERROR_NONE || current != screen3) {
    printf("Expected screen3 to be current\n");
    return 1;
  }

  printf("Testing ui_router_pop...\n");
  rc = ui_router_pop(router);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to pop screen3\n");
    return 1;
  }

  rc = ui_router_get_current(router, &current);
  if (rc != UI_ERROR_NONE || current != screen1) {
    printf("Expected screen1 to be current after pop\n");
    return 1;
  }

  rc = ui_router_pop(router);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to pop screen1\n");
    return 1;
  }

  rc = ui_router_get_current(router, &current);
  if (rc == UI_ERROR_NONE && current != NULL) {
    printf("Expected NULL current after popping last screen\n");
    return 1;
  }

  rc = ui_router_pop(router);
  if (rc != UI_ERROR_QUEUE_EMPTY) {
    printf("Expected pop to fail on empty router\n");
    return 1;
  }

  /* Replace on empty stack behaves like push */
  ui_component_create(&screen1);
  rc = ui_router_replace(router, screen1);
  if (rc != UI_ERROR_NONE ||
      ui_router_get_current(router, &current) != UI_ERROR_NONE ||
      current != screen1)
    return 1;

  printf("Testing ui_router_add_route...\n");
  rc = ui_router_add_route(router, "/settings/profile/:id",
                           mock_factory_success, &factory_calls);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to add route\n");
    return 1;
  }

  rc = ui_router_add_route(router, "/fail", mock_factory_fail, NULL);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to add fail route\n");
    return 1;
  }

  /* Add complex route to test URL param parsing branches */
  rc = ui_router_add_route(router, "/search", mock_factory_success,
                           &factory_calls);
  if (rc != UI_ERROR_NONE)
    return 1;

  printf("Testing ui_router_navigate...\n");
  rc = ui_router_navigate(router, "/settings/profile/123?tab=2");
  if (rc != UI_ERROR_NONE) {
    printf("Failed to navigate to matched route\n");
    return 1;
  }

  if (factory_calls != 1) {
    printf("Expected factory to be called once\n");
    return 1;
  }

  rc = ui_router_get_current(router, &current);
  if (rc != UI_ERROR_NONE || current == NULL) {
    printf("Expected a screen to be pushed\n");
    return 1;
  }

  rc = ui_router_navigate(router, "/not/found");
  if (rc != UI_ERROR_NOT_FOUND) {
    printf("Expected NOT_FOUND error for invalid route\n");
    return 1;
  }

  rc = ui_router_navigate(router, "/fail");
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Expected factory error to be percolated\n");
    return 1;
  }

  /* Testing query string parsing branches */
  /* Query with just & or empty */
  factory_calls = 0;
  rc = ui_router_navigate(router, "/search?q=&b");
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Testing deep link event */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_OS_DEEP_LINK;
#if defined(_MSC_VER)
  strcpy_s(ev.event_data.deep_link.uri, sizeof(ev.event_data.deep_link.uri),
           "/search?tab=2");
#else
  strcpy(ev.event_data.deep_link.uri, "/search?tab=2");
#endif
  rc = ui_router_process_event(router, &ev);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Other event shouldn't do anything */
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_router_process_event(router, &ev);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* OS Hooks */
  ui_router_install_os_hooks(router);

  printf("Testing ui_router_destroy...\n");
  ui_router_destroy(router);

  return 0;
}

static int run_oom_tests(void) {
  struct ui_router *router = NULL;
  enum ui_error err;
  int i;
  struct ui_component *screen = NULL;

  printf("Running router OOM tests...\n");

  /* Creation OOM */
  for (i = 0; i < 7; i++) {
    g_malloc_fail_countdown = i;
    err = ui_router_create(&router);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_OUT_OF_MEMORY) {
      continue;
    } else if (err == UI_ERROR_NONE) {
      ui_router_destroy(router);
      break;
    } else {
      return 1;
    }
  }

  ui_router_create(&router);
  ui_component_create(&screen);

  /* Push reallocation OOM */
  /* Force stack growth (initial capacity is 8) */
  for (i = 0; i < 8; i++) {
    struct ui_component *s;
    ui_component_create(&s);
    ui_router_push(router, s);
  }

  g_malloc_fail_countdown = 0;
  err = ui_router_push(router, screen);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  /* Route add reallocation OOM */
  for (i = 0; i < 8; i++) {
    ui_router_add_route(router, "/a", mock_factory_success, NULL);
  }

  g_malloc_fail_countdown = 0;
  err = ui_router_add_route(router, "/b", mock_factory_success, NULL);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  /* Route add string OOM */
  g_malloc_fail_countdown = 0;
  err = ui_router_add_route(router, "/b", mock_factory_success, NULL);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  /* Route string parsing OOMs */
  ui_router_add_route(router, "/p/:id", mock_factory_success, NULL);

  for (i = 0; i < 15; i++) {
    g_malloc_fail_countdown = i;
    err = ui_router_navigate(router, "/p/123?a=b&c=d");
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_OUT_OF_MEMORY) {
      continue;
    } else if (err == UI_ERROR_NONE) {
      break;
    } else {
      /* Can hit UI_ERROR_NOT_FOUND or UI_ERROR_UNKNOWN if factory fails due to
       * missing ID */
      if (err != UI_ERROR_NOT_FOUND && err != UI_ERROR_UNKNOWN &&
          err != UI_ERROR_INVALID_ARGUMENT) {
        return 1;
      }
    }
  }

  ui_router_destroy(router);
  /* Note screen was not successfully pushed, so we clean it up */
  ui_component_destroy(screen);

  return 0;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All test_ui_router passed.\n");
  return 0;
}
