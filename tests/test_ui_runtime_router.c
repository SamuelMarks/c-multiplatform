/* clang-format off */
#include "ui_runtime_router.h"
#include "ui_runtime_builder.h"
#include "ui_runtime_schema.h"
#include "ui_component_registry.h"
#include "ui_dynamic_context.h"
#include "ui_app_state_registry.h"
#include "ui_arena.h"
#include "ui_dom_node.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_mock_strcpy_fail;

static ui_error_t headless_factory(void **out_instance,
                                   struct ui_dom_node **out_dom_node) {
  *out_instance = (void *)0x1234;
  *out_dom_node = NULL;
  return UI_ERROR_NONE;
}

int main(void) {
  struct ui_arena *arena = NULL;
  struct ui_arena *small_arena = NULL;
  struct ui_runtime_router *router = NULL;
  struct ui_runtime_router *heap_router = NULL;
  struct ui_runtime_router *small_router = NULL;
  struct ui_component_registry *registry = NULL;
  struct ui_dynamic_context *ctx = NULL;
  struct ui_app_state_registry *app_state = NULL;
  struct ui_dom_node *mount_host = NULL;
  struct ui_runtime_node home_node;
  struct ui_runtime_node survey_node;
  struct ui_runtime_node extra_node;
  struct ui_runtime_node headless_node;
  struct ui_component_vtable headless_vt;
  struct ui_runtime_app_manifest manifest;
  struct ui_runtime_route_def route_def1;
  struct ui_runtime_route_def route_def2;
  struct ui_runtime_route_def route_def_incomplete;
  struct ui_runtime_route_def route_def_missing_node;
  const char *curr_route = NULL;
  ui_error_t rc;
  int i;

  rc = ui_arena_create(65536, &arena);
  assert(rc == UI_ERROR_NONE);

  rc = ui_component_registry_get_default(&registry);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_create(arena, &ctx);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_create(arena, &app_state);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &mount_host);
  assert(rc == UI_ERROR_NONE);

  /* 1. NULL & Invalid Argument checks */
  rc = ui_runtime_router_create(NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_runtime_router_destroy(NULL);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_router_register_route(NULL, "/test", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_runtime_router_create(arena, &router);
  assert(rc == UI_ERROR_NONE);

  memset(&home_node, 0, sizeof(home_node));
  home_node.id = "screen_home";
  home_node.type = "row";

  memset(&survey_node, 0, sizeof(survey_node));
  survey_node.id = "screen_survey";
  survey_node.type = "column";

  memset(&extra_node, 0, sizeof(extra_node));
  extra_node.id = "screen_extra";
  extra_node.type = "container";

  rc = ui_runtime_router_register_route(router, NULL, &home_node);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_router_register_route(router, "/home", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_runtime_router_load_manifest(NULL, &manifest);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_router_load_manifest(router, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_runtime_router_navigate(NULL, "/home", mount_host, registry, ctx,
                                  app_state);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_router_navigate(router, NULL, mount_host, registry, ctx,
                                  app_state);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_router_navigate(router, "/home", NULL, registry, ctx,
                                  app_state);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_runtime_router_pop(NULL, mount_host, registry, ctx, app_state);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_router_pop(router, NULL, registry, ctx, app_state);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_runtime_router_get_current_route(NULL, &curr_route);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_router_get_current_route(router, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Current route before any navigation returns NOT_FOUND */
  rc = ui_runtime_router_get_current_route(router, &curr_route);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* Pop with empty history returns NOT_FOUND */
  rc = ui_runtime_router_pop(router, mount_host, registry, ctx, app_state);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* 2. Route Registration & Duplicate Updates */
  rc = ui_runtime_router_register_route(router, "/home", &home_node);
  assert(rc == UI_ERROR_NONE);

  /* Update duplicate at head */
  rc = ui_runtime_router_register_route(router, "/home", &home_node);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_router_register_route(router, "/survey", &survey_node);
  assert(rc == UI_ERROR_NONE);

  /* Update duplicate after head (curr = curr->next) */
  rc = ui_runtime_router_register_route(router, "/home", &home_node);
  assert(rc == UI_ERROR_NONE);

  /* 3. Navigation & Screen Switching & History Pop */
  /* Navigate to /home */
  rc = ui_runtime_router_navigate(router, "/home", mount_host, registry, ctx,
                                  app_state);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_router_get_current_route(router, &curr_route);
  assert(rc == UI_ERROR_NONE);
  assert(strcmp(curr_route, "/home") == 0);
  assert(mount_host->first_child != NULL);

  /* Pop with only 1 history entry returns NOT_FOUND */
  rc = ui_runtime_router_pop(router, mount_host, registry, ctx, app_state);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* Navigate to /survey (clears mount_host children and mounts survey) */
  rc = ui_runtime_router_navigate(router, "/survey", mount_host, registry, ctx,
                                  app_state);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_router_get_current_route(router, &curr_route);
  assert(rc == UI_ERROR_NONE);
  assert(strcmp(curr_route, "/survey") == 0);

  /* Pop back to /home on arena router */
  rc = ui_runtime_router_pop(router, mount_host, registry, ctx, app_state);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_router_get_current_route(router, &curr_route);
  assert(rc == UI_ERROR_NONE);
  assert(strcmp(curr_route, "/home") == 0);

  /* Navigate to invalid path */
  rc = ui_runtime_router_navigate(router, "/not_found", mount_host, registry,
                                  ctx, app_state);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* Route with headless (NULL) DOM node to cover if (screen_dom) false path */
  memset(&headless_vt, 0, sizeof(headless_vt));
  headless_vt.factory = headless_factory;
  rc =
      ui_component_registry_register(registry, "headless_widget", &headless_vt);
  assert(rc == UI_ERROR_NONE);

  memset(&headless_node, 0, sizeof(headless_node));
  headless_node.id = "screen_headless";
  headless_node.type = "headless_widget";

  rc = ui_runtime_router_register_route(router, "/headless", &headless_node);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_router_navigate(router, "/headless", mount_host, registry,
                                  ctx, app_state);
  assert(rc == UI_ERROR_NONE);

  /* 4. Manifest Loading */
  memset(&manifest, 0, sizeof(manifest));
  memset(&route_def1, 0, sizeof(route_def1));
  memset(&route_def2, 0, sizeof(route_def2));
  memset(&route_def_incomplete, 0, sizeof(route_def_incomplete));
  memset(&route_def_missing_node, 0, sizeof(route_def_missing_node));

  route_def1.path = "/page1";
  route_def1.root_node = &home_node;
  route_def1.next = &route_def_incomplete;

  /* Incomplete route missing root_node and path */
  route_def_incomplete.path = NULL;
  route_def_incomplete.root_node = NULL;
  route_def_incomplete.next = &route_def_missing_node;

  /* Route with valid path but NULL root_node (covers branch 2 of r->path &&
   * r->root_node) */
  route_def_missing_node.path = "/missing_node";
  route_def_missing_node.root_node = NULL;
  route_def_missing_node.next = &route_def2;

  route_def2.path = "/page2";
  route_def2.root_node = &survey_node;
  route_def2.next = NULL;

  manifest.routes = &route_def1;

  rc = ui_runtime_router_load_manifest(router, &manifest);
  assert(rc == UI_ERROR_NONE);

  /* Load manifest with route registration failure on fresh router */
  {
    struct ui_runtime_router *fresh_router = NULL;
    rc = ui_runtime_router_create(arena, &fresh_router);
    assert(rc == UI_ERROR_NONE);
    g_mock_strcpy_fail = 1;
    rc = ui_runtime_router_load_manifest(fresh_router, &manifest);
    g_mock_strcpy_fail = 0;
    assert(rc == UI_ERROR_UNKNOWN);
  }

  /* 5. Heap-allocated router lifecycle, error paths, and pop */
  /* OOM on router create */
  g_malloc_fail_countdown = 0;
  rc = ui_runtime_router_create(NULL, &heap_router);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  rc = ui_runtime_router_create(NULL, &heap_router);
  assert(rc == UI_ERROR_NONE);

  /* OOM on route entry */
  g_malloc_fail_countdown = 0;
  rc = ui_runtime_router_register_route(heap_router, "/h_oom", &home_node);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* OOM on route path */
  g_malloc_fail_countdown = 1;
  rc = ui_runtime_router_register_route(heap_router, "/h_oom", &home_node);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* UI_STRCPY fail on route */
  g_mock_strcpy_fail = 1;
  rc = ui_runtime_router_register_route(heap_router, "/h_strcpy", &home_node);
  g_mock_strcpy_fail = 0;
  assert(rc == UI_ERROR_UNKNOWN);

  /* Register valid routes on heap router */
  rc = ui_runtime_router_register_route(heap_router, "/heap_home", &home_node);
  assert(rc == UI_ERROR_NONE);
  rc = ui_runtime_router_register_route(heap_router, "/heap_survey",
                                        &survey_node);
  assert(rc == UI_ERROR_NONE);

  /* OOM countdown loop over navigate on heap router to hit all intermediate
   * fail points */
  for (i = 0; i < 15; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_runtime_router_navigate(heap_router, "/heap_home", mount_host,
                                    registry, ctx, app_state);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      break;
    }
  }

  /* UI_STRCPY fail on history during navigate */
  g_mock_strcpy_fail = 1;
  rc = ui_runtime_router_navigate(heap_router, "/heap_home", mount_host,
                                  registry, ctx, app_state);
  g_mock_strcpy_fail = 0;
  assert(rc == UI_ERROR_UNKNOWN);

  /* Successful navigations on heap router */
  rc = ui_runtime_router_navigate(heap_router, "/heap_home", mount_host,
                                  registry, ctx, app_state);
  assert(rc == UI_ERROR_NONE);
  rc = ui_runtime_router_navigate(heap_router, "/heap_survey", mount_host,
                                  registry, ctx, app_state);
  assert(rc == UI_ERROR_NONE);

  /* Pop on heap router (frees popped->path and popped) */
  rc = ui_runtime_router_pop(heap_router, mount_host, registry, ctx, app_state);
  assert(rc == UI_ERROR_NONE);

  /* Destroy heap router (frees remaining history and routes) */
  rc = ui_runtime_router_destroy(heap_router);
  assert(rc == UI_ERROR_NONE);

  /* 6. Arena exhaustion paths */
  rc = ui_arena_create(16, &small_arena);
  assert(rc == UI_ERROR_NONE);
  g_malloc_fail_countdown = 0;
  rc = ui_runtime_router_create(small_arena, &small_router);
  g_malloc_fail_countdown = -1;
  assert(rc != UI_ERROR_NONE);
  rc = ui_arena_destroy(small_arena);
  assert(rc == UI_ERROR_NONE);

  /* Arena route entry allocation failure (line 150) */
  rc = ui_arena_create(32, &small_arena);
  assert(rc == UI_ERROR_NONE);
  rc = ui_runtime_router_create(small_arena, &small_router);
  assert(rc == UI_ERROR_NONE);
  /* router takes 32 bytes, small_arena block is full: next alloc fails */
  g_malloc_fail_countdown = 0;
  rc = ui_runtime_router_register_route(small_router, "/overflow_route",
                                        &home_node);
  g_malloc_fail_countdown = -1;
  assert(rc != UI_ERROR_NONE);
  rc = ui_arena_destroy(small_arena);
  assert(rc == UI_ERROR_NONE);

  /* Arena route path allocation failure (line 154) */
  {
    char long_path[300];
    memset(long_path, 'r', sizeof(long_path) - 1);
    long_path[sizeof(long_path) - 1] = '\0';

    rc = ui_arena_create(64, &small_arena);
    assert(rc == UI_ERROR_NONE);
    rc = ui_runtime_router_create(small_arena, &small_router);
    assert(rc == UI_ERROR_NONE);

    /* Entry fits in 64 bytes, but 300-byte path fails on arena alloc */
    g_malloc_fail_countdown = 0;
    rc = ui_runtime_router_register_route(small_router, long_path, &home_node);
    g_malloc_fail_countdown = -1;
    assert(rc != UI_ERROR_NONE);

    /* UI_STRCPY failure on arena router route */
    g_mock_strcpy_fail = 1;
    rc = ui_runtime_router_register_route(router, "/fail_arena_route",
                                          &home_node);
    g_mock_strcpy_fail = 0;
    assert(rc == UI_ERROR_UNKNOWN);

    /* UI_STRCPY failure on arena router history */
    g_mock_strcpy_fail = 1;
    rc = ui_runtime_router_navigate(router, "/home", mount_host, registry, ctx,
                                    app_state);
    g_mock_strcpy_fail = 0;
    assert(rc == UI_ERROR_UNKNOWN);

    rc = ui_arena_destroy(small_arena);
    assert(rc == UI_ERROR_NONE);
  }

  /* Arena history entry and path allocation failure during navigate */
  {
    struct ui_arena *hist_arena = NULL;
    struct ui_runtime_router *hist_router = NULL;
    void *dummy = NULL;
    char long_nav_path[300];

    rc = ui_arena_create(64, &hist_arena);
    assert(rc == UI_ERROR_NONE);
    rc = ui_runtime_router_create(hist_arena, &hist_router);
    assert(rc == UI_ERROR_NONE);
    rc = ui_runtime_router_register_route(hist_router, "/headless",
                                          &headless_node);
    assert(rc == UI_ERROR_NONE);

    /* Exhaust current block so next allocation needs a new block */
    rc = ui_arena_alloc(hist_arena, 64, 8, &dummy);
    assert(rc == UI_ERROR_NONE);

    /* Hist entry alloc fails on malloc (line 285) */
    g_malloc_fail_countdown = 0;
    rc = ui_runtime_router_navigate(hist_router, "/headless", mount_host,
                                    registry, ctx, app_state);
    g_malloc_fail_countdown = -1;
    assert(rc != UI_ERROR_NONE);
    rc = ui_arena_destroy(hist_arena);
    assert(rc == UI_ERROR_NONE);

    /* Hist path alloc fails on malloc (line 289) */
    memset(long_nav_path, 'a', sizeof(long_nav_path) - 1);
    long_nav_path[0] = '/';
    long_nav_path[sizeof(long_nav_path) - 1] = '\0';

    rc = ui_arena_create(64, &hist_arena);
    assert(rc == UI_ERROR_NONE);
    rc = ui_runtime_router_create(hist_arena, &hist_router);
    assert(rc == UI_ERROR_NONE);
    rc = ui_runtime_router_register_route(hist_router, long_nav_path,
                                          &headless_node);
    assert(rc == UI_ERROR_NONE);

    /* Exhaust current block */
    rc = ui_arena_alloc(hist_arena, 64, 8, &dummy);
    assert(rc == UI_ERROR_NONE);

    /* Countdown 1: hist entry gets new block, 300-byte path requires second
     * block and fails */
    g_malloc_fail_countdown = 1;
    rc = ui_runtime_router_navigate(hist_router, long_nav_path, mount_host,
                                    registry, ctx, app_state);
    g_malloc_fail_countdown = -1;
    assert(rc != UI_ERROR_NONE);
    rc = ui_arena_destroy(hist_arena);
    assert(rc == UI_ERROR_NONE);
  }

  /* 7. Teardown */
  rc = ui_dom_node_destroy(mount_host);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_router_destroy(router);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_destroy(ctx);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_destroy(app_state);
  assert(rc == UI_ERROR_NONE);

  rc = ui_arena_destroy(arena);
  assert(rc == UI_ERROR_NONE);

  printf("test_ui_runtime_router passed\n");
  return 0;
}
