#include "cmpc/cmp_router.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAllocator {
  cmp_usize alloc_calls;
  cmp_usize free_calls;
  cmp_usize realloc_calls;
  cmp_usize fail_alloc_on;
  cmp_usize fail_free_on;
} TestAllocator;

static int test_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  TestAllocator *alloc;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on != 0 && alloc->alloc_calls == alloc->fail_alloc_on) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = malloc(size);
  if (*out_ptr == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  return CMP_OK;
}

static int test_realloc(void *ctx, void *ptr, cmp_usize size, void **out_ptr) {
  TestAllocator *alloc;
  void *mem;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->realloc_calls += 1;
  mem = realloc(ptr, size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  *out_ptr = mem;
  return CMP_OK;
}

static int test_free(void *ctx, void *ptr) {
  TestAllocator *alloc;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_on != 0 && alloc->free_calls == alloc->fail_free_on) {
    return CMP_ERR_UNKNOWN;
  }

  free(ptr);
  return CMP_OK;
}

static int test_alloc_null(void *ctx, cmp_usize size, void **out_ptr) {
  CMP_UNUSED(ctx);
  CMP_UNUSED(size);

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;
  return CMP_OK;
}

static void test_allocator_init(TestAllocator *alloc) {
  alloc->alloc_calls = 0;
  alloc->free_calls = 0;
  alloc->realloc_calls = 0;
  alloc->fail_alloc_on = 0;
  alloc->fail_free_on = 0;
}

static int slice_equals(const CMPUriSlice *slice, const char *text) {
  cmp_usize len;
  cmp_usize i;

  if (slice == NULL || text == NULL) {
    return 0;
  }

  len = 0;
  while (text[len] != '\0') {
    len += 1;
  }

  if (slice->length != len) {
    return 0;
  }

  for (i = 0; i < len; ++i) {
    if (slice->data[i] != text[i]) {
      return 0;
    }
  }

  return 1;
}

typedef struct RouteCtx {
  int build_called;
  int destroy_called;
  int build_fail;
  int destroy_fail;
} RouteCtx;

static int route_build(void *ctx, const char *path, void **out_component) {
  RouteCtx *route_ctx;

  if (ctx == NULL || path == NULL || out_component == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  route_ctx = (RouteCtx *)ctx;
  route_ctx->build_called += 1;
  if (route_ctx->build_fail) {
    return CMP_ERR_UNKNOWN;
  }

  *out_component = ctx;
  return CMP_OK;
}

static int route_destroy(void *ctx, void *component) {
  RouteCtx *route_ctx;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  route_ctx = (RouteCtx *)ctx;
  route_ctx->destroy_called += 1;
  if (route_ctx->destroy_fail) {
    return CMP_ERR_UNKNOWN;
  }

  CMP_UNUSED(component);
  return CMP_OK;
}

int main(void) {
  CMPUri uri;
  CMPUriSlice value;
  CMPUriSlice current_slice;
  CMPUriSlice temp_slice;
  CMPBool found;
  CMPBool match;
  CMPBool has_more;
  cmp_usize param_count;
  cmp_usize max_value;
  cmp_usize overflow_capacity;
  CMPRouteParam params[2];
  TestAllocator alloc;
  CMPAllocator cmp_alloc;
  CMPAllocator null_alloc;
  CMPRouter router;
  CMPRouter router_copy;
  CMPRouter router_manual;
  CMPRouterConfig config;
  RouteCtx home_ctx;
  RouteCtx user_ctx;
  RouteCtx file_ctx;
  CMPRoute routes[3];
  CMPRoute bad_routes[1];
  CMPRouteEntry stack_entries[1];
  const CMPRoute *found_route;
  const char *current_path;
  char *path_copy;
  void *component;
  CMPBool can_back;

  memset(&router_copy, 0, sizeof(router_copy));
  memset(&router_manual, 0, sizeof(router_manual));

  CMP_TEST_EXPECT(cmp_uri_parse(NULL, &uri), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_uri_parse("x", NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_router_test_cstrlen(NULL, &max_value),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_test_cstrlen("x", NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_router_test_slice_set(NULL, "x", 1));
  temp_slice.data = "bar";
  temp_slice.length = 3;
  CMP_TEST_EXPECT(cmp_router_test_slice_equals(NULL, "bar", 3, &match),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_test_slice_equals(&temp_slice, NULL, 3, &match),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_test_slice_equals(&temp_slice, "bar", 3, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  temp_slice.data = NULL;
  temp_slice.length = 0;
  CMP_TEST_OK(cmp_router_test_slice_equals(&temp_slice, "", 0, &match));
  CMP_TEST_ASSERT(match == CMP_TRUE);
  temp_slice.data = "bar";
  temp_slice.length = 3;
  CMP_TEST_OK(cmp_router_test_slice_equals(&temp_slice, "foo", 3, &match));
  CMP_TEST_ASSERT(match == CMP_FALSE);
  CMP_TEST_OK(cmp_router_test_trim_trailing(NULL, NULL));
  {
    cmp_usize idx;
    const char *seg;
    cmp_usize seg_len;

    idx = 0;
    seg = NULL;
    seg_len = 0;
    CMP_TEST_OK(cmp_router_test_next_segment(NULL, 0, &idx, &seg, &seg_len));
  }
  CMP_TEST_OK(cmp_router_test_has_more_segments(NULL, 0, 0, &has_more));
  CMP_TEST_ASSERT(has_more == CMP_FALSE);
  CMP_TEST_EXPECT(
      cmp_router_test_match_len(NULL, 0, "/", 1, NULL, 0, NULL, &match),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_test_validate_pattern(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_router_test_validate_pattern(""));
  CMP_TEST_EXPECT(cmp_router_test_validate_pattern("/users/:"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_test_release_entry(NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_test_navigate_path_len(NULL, "/", 1, &component),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_test_has_more_segments(NULL, 0, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  found_route = NULL;
  CMP_TEST_EXPECT(cmp_router_test_find_route(NULL, "/", 1, &found_route),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_test_find_route(&router_manual, "/", 1, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_router_test_find_route(&router_manual, "/", 1, &found_route),
      CMP_ERR_NOT_FOUND);
  CMP_TEST_OK(
      cmp_uri_parse("app://example.com/path/one?foo=bar&flag#frag", &uri));
  CMP_TEST_ASSERT(slice_equals(&uri.scheme, "app"));
  CMP_TEST_ASSERT(slice_equals(&uri.authority, "example.com"));
  CMP_TEST_ASSERT(slice_equals(&uri.path, "/path/one"));
  CMP_TEST_ASSERT(slice_equals(&uri.query, "foo=bar&flag"));
  CMP_TEST_ASSERT(slice_equals(&uri.fragment, "frag"));

  CMP_TEST_OK(cmp_uri_query_find(&uri, "foo", &value, &found));
  CMP_TEST_ASSERT(found == CMP_TRUE);
  CMP_TEST_ASSERT(slice_equals(&value, "bar"));
  CMP_TEST_OK(cmp_uri_query_find(&uri, "flag", &value, &found));
  CMP_TEST_ASSERT(found == CMP_TRUE);
  CMP_TEST_ASSERT(value.length == 0);
  CMP_TEST_OK(cmp_uri_query_find(&uri, "missing", &value, &found));
  CMP_TEST_ASSERT(found == CMP_FALSE);
#ifdef CMP_TESTING
  CMP_TEST_OK(cmp_router_test_set_slice_equals_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_uri_query_find(&uri, "foo", &value, &found),
                  CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_router_test_set_slice_equals_fail(CMP_FALSE));
#endif

  CMP_TEST_OK(cmp_uri_parse("app://example.com/path#frag", &uri));
  CMP_TEST_ASSERT(slice_equals(&uri.path, "/path"));
  CMP_TEST_ASSERT(slice_equals(&uri.fragment, "frag"));

  CMP_TEST_OK(cmp_uri_parse("app://example.com/path?bar=baz", &uri));
  CMP_TEST_OK(cmp_uri_query_find(&uri, "foo", &value, &found));
  CMP_TEST_ASSERT(found == CMP_FALSE);

  CMP_TEST_EXPECT(cmp_uri_query_find(NULL, "foo", &value, &found),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_uri_query_find(&uri, NULL, &value, &found),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_uri_query_find(&uri, "foo", NULL, &found),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_uri_query_find(&uri, "foo", &value, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_uri_query_find(&uri, "", &value, &found), CMP_ERR_RANGE);

  CMP_TEST_OK(cmp_uri_parse("app://example.com/path", &uri));
  CMP_TEST_OK(cmp_uri_query_find(&uri, "foo", &value, &found));
  CMP_TEST_ASSERT(found == CMP_FALSE);

  CMP_TEST_OK(cmp_router_test_set_cstr_limit(3));
  CMP_TEST_EXPECT(cmp_uri_query_find(&uri, "abcd", &value, &found),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_EXPECT(cmp_router_test_validate_pattern("abcd"), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_router_test_set_cstr_limit(0));

  {
    CMPRouter find_router;
    CMPRoute route;

    memset(&find_router, 0, sizeof(find_router));
    route.pattern = "/home";
    route.build = route_build;
    route.destroy = route_destroy;
    route.ctx = &home_ctx;
    find_router.routes = &route;
    find_router.route_count = 1;

    CMP_TEST_OK(cmp_router_test_set_cstr_limit(1));
    CMP_TEST_EXPECT(
        cmp_router_test_find_route(&find_router, "/home", 5, &found_route),
        CMP_ERR_OVERFLOW);
    CMP_TEST_OK(cmp_router_test_set_cstr_limit(0));

    CMP_TEST_OK(cmp_router_test_set_cstr_fail_after(2u));
    CMP_TEST_EXPECT(
        cmp_router_test_find_route(&find_router, "/home", 5, &found_route),
        CMP_ERR_OVERFLOW);
    CMP_TEST_OK(cmp_router_test_set_cstr_fail_after(0u));

    CMP_TEST_EXPECT(
        cmp_router_test_find_route(&find_router, NULL, 0, &found_route),
        CMP_ERR_INVALID_ARGUMENT);
  }

  {
    CMPRouter find_router;
    CMPRoute routes_local[2];

    memset(&find_router, 0, sizeof(find_router));
    memset(routes_local, 0, sizeof(routes_local));
    routes_local[0].pattern = "/home";
    routes_local[0].build = route_build;
    routes_local[0].destroy = route_destroy;
    routes_local[0].ctx = &home_ctx;
    routes_local[1].pattern = "/users/:id";
    routes_local[1].build = route_build;
    routes_local[1].destroy = route_destroy;
    routes_local[1].ctx = &user_ctx;
    find_router.routes = routes_local;
    find_router.route_count = 2;

    found_route = NULL;
    CMP_TEST_OK(
        cmp_router_test_find_route(&find_router, "/home", 5, &found_route));
    CMP_TEST_ASSERT(found_route == &routes_local[0]);
    CMP_TEST_EXPECT(
        cmp_router_test_find_route(&find_router, "/missing", 8, &found_route),
        CMP_ERR_NOT_FOUND);

    routes_local[0].build = NULL;
    CMP_TEST_EXPECT(
        cmp_router_test_find_route(&find_router, "/home", 5, &found_route),
        CMP_ERR_INVALID_ARGUMENT);
    routes_local[0].build = route_build;
    routes_local[0].pattern = "/bad/:";
    CMP_TEST_EXPECT(
        cmp_router_test_find_route(&find_router, "/bad", 4, &found_route),
        CMP_ERR_INVALID_ARGUMENT);
    routes_local[0].pattern = NULL;
    CMP_TEST_EXPECT(
        cmp_router_test_find_route(&find_router, "/home", 5, &found_route),
        CMP_ERR_INVALID_ARGUMENT);
  }

  {
    CMPRouter local_router;
    CMPRouteEntry local_stack[1];

    memset(&local_router, 0, sizeof(local_router));
    local_router.stack = local_stack;
    local_router.stack_capacity = 1;
    local_router.stack_size = 1;
    local_stack[0].path = "/overflow";
    local_stack[0].component = NULL;

    CMP_TEST_OK(cmp_router_test_set_cstr_limit(1));
    CMP_TEST_EXPECT(cmp_router_test_navigate_path_len(&local_router, "/next", 5,
                                                      &component),
                    CMP_ERR_OVERFLOW);
    CMP_TEST_OK(cmp_router_test_set_cstr_limit(0));
  }

  CMP_TEST_OK(cmp_uri_parse("/local/path", &uri));
  CMP_TEST_ASSERT(uri.scheme.length == 0);
  CMP_TEST_ASSERT(uri.authority.length == 0);
  CMP_TEST_ASSERT(slice_equals(&uri.path, "/local/path"));

  CMP_TEST_OK(cmp_uri_parse("noscheme", &uri));
  CMP_TEST_ASSERT(uri.scheme.length == 0);
  CMP_TEST_ASSERT(slice_equals(&uri.path, "noscheme"));

  CMP_TEST_OK(cmp_uri_parse("app://host", &uri));
  CMP_TEST_ASSERT(slice_equals(&uri.scheme, "app"));
  CMP_TEST_ASSERT(slice_equals(&uri.authority, "host"));
  CMP_TEST_ASSERT(uri.path.length == 0);

  CMP_TEST_OK(cmp_uri_parse("", &uri));
  CMP_TEST_ASSERT(uri.path.length == 0);

  CMP_TEST_EXPECT(cmp_route_test_match(NULL, "/a", NULL, 0, NULL, &match),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_route_test_match("/a", NULL, NULL, 0, NULL, &match),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_route_test_match("/a", "/a", NULL, 0, &param_count, &match),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_route_test_match("", "", NULL, 0, NULL, &match));
  CMP_TEST_ASSERT(match == CMP_TRUE);
  CMP_TEST_OK(cmp_route_test_match("*", "", NULL, 0, NULL, &match));
  CMP_TEST_ASSERT(match == CMP_TRUE);
  CMP_TEST_OK(cmp_route_test_match("*", "/files", NULL, 0, NULL, &match));
  CMP_TEST_ASSERT(match == CMP_TRUE);

  CMP_TEST_OK(cmp_route_test_match("/home", "/home", NULL, 0, NULL, &match));
  CMP_TEST_ASSERT(match == CMP_TRUE);
  CMP_TEST_OK(cmp_route_test_match("/home/", "/home", NULL, 0, NULL, &match));
  CMP_TEST_ASSERT(match == CMP_TRUE);
  CMP_TEST_OK(cmp_route_test_match("/home", "/home/", NULL, 0, NULL, &match));
  CMP_TEST_ASSERT(match == CMP_TRUE);

  CMP_TEST_OK(cmp_route_test_match("/users/:id", "/users/42", params, 2,
                                   &param_count, &match));
  CMP_TEST_ASSERT(match == CMP_TRUE);
  CMP_TEST_ASSERT(param_count == 1);
  CMP_TEST_ASSERT(slice_equals(&params[0].key, "id"));
  CMP_TEST_ASSERT(slice_equals(&params[0].value, "42"));

  CMP_TEST_EXPECT(cmp_route_test_match("/users/:", "/users/42", params, 2,
                                       &param_count, &match),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_route_test_match("/files/*/more", "/files/x", params, 2,
                                       &param_count, &match),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_route_test_match("/files/*/more", "/files", params, 2,
                                       &param_count, &match),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(
      cmp_route_test_match("/files/*", "/files/a/b/c", NULL, 0, NULL, &match));
  CMP_TEST_ASSERT(match == CMP_TRUE);
  CMP_TEST_OK(cmp_route_test_match("/files/*", "/files/a/b", params, 2,
                                   &param_count, &match));
  CMP_TEST_ASSERT(match == CMP_TRUE);
  CMP_TEST_ASSERT(param_count == 0);
  CMP_TEST_OK(cmp_route_test_match("/files/*", "/files", params, 2,
                                   &param_count, &match));
  CMP_TEST_ASSERT(match == CMP_TRUE);
  CMP_TEST_ASSERT(param_count == 0);

  CMP_TEST_EXPECT(cmp_route_test_match("/users/:id", "/users/42", params, 0,
                                       &param_count, &match),
                  CMP_ERR_OVERFLOW);

  CMP_TEST_OK(cmp_route_test_match("/a/b", "/a", NULL, 0, NULL, &match));
  CMP_TEST_ASSERT(match == CMP_FALSE);
  CMP_TEST_OK(cmp_route_test_match("/a", "/a/b", NULL, 0, NULL, &match));
  CMP_TEST_ASSERT(match == CMP_FALSE);

  test_allocator_init(&alloc);
  cmp_alloc.ctx = &alloc;
  cmp_alloc.alloc = test_alloc;
  cmp_alloc.realloc = test_realloc;
  cmp_alloc.free = test_free;

  memset(&config, 0, sizeof(config));
  CMP_TEST_EXPECT(cmp_router_init(NULL, &config), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_init(&router, NULL), CMP_ERR_INVALID_ARGUMENT);

  config.allocator = &cmp_alloc;
  config.routes = NULL;
  config.route_count = 1;
  config.stack_capacity = 1;
  CMP_TEST_EXPECT(cmp_router_init(&router, &config), CMP_ERR_INVALID_ARGUMENT);

  routes[0].pattern = NULL;
  routes[0].build = route_build;
  routes[0].destroy = route_destroy;
  routes[0].ctx = &home_ctx;
  config.routes = routes;
  config.route_count = 1;
  CMP_TEST_EXPECT(cmp_router_init(&router, &config), CMP_ERR_INVALID_ARGUMENT);

  routes[0].pattern = "/";
  routes[0].build = NULL;
  CMP_TEST_EXPECT(cmp_router_init(&router, &config), CMP_ERR_INVALID_ARGUMENT);

  routes[0].build = route_build;
  routes[0].pattern = "/files/*/more";
  CMP_TEST_EXPECT(cmp_router_init(&router, &config), CMP_ERR_INVALID_ARGUMENT);

#ifdef CMP_TESTING
  config.allocator = NULL;
  routes[0].pattern = "/";
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_router_init(&router, &config), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));
  config.allocator = &cmp_alloc;
#endif

  null_alloc.ctx = &alloc;
  null_alloc.alloc = test_alloc_null;
  null_alloc.realloc = test_realloc;
  null_alloc.free = test_free;
  config.allocator = &null_alloc;
  routes[0].pattern = "/";
  CMP_TEST_EXPECT(cmp_router_init(&router, &config), CMP_ERR_OUT_OF_MEMORY);
  config.allocator = &cmp_alloc;

  max_value = (cmp_usize) ~(cmp_usize)0;
  overflow_capacity = max_value / sizeof(CMPRouteEntry);
  if (overflow_capacity < max_value) {
    overflow_capacity += 1;
  }
  config.stack_capacity = overflow_capacity;
  CMP_TEST_EXPECT(cmp_router_init(&router, &config), CMP_ERR_OVERFLOW);

  config.stack_capacity = 1;
  alloc.fail_alloc_on = 1;
  CMP_TEST_EXPECT(cmp_router_init(&router, &config), CMP_ERR_OUT_OF_MEMORY);
  alloc.fail_alloc_on = 0;

  home_ctx.build_called = 0;
  home_ctx.destroy_called = 0;
  home_ctx.build_fail = 0;
  home_ctx.destroy_fail = 0;
  user_ctx = home_ctx;
  file_ctx = home_ctx;

  routes[0].pattern = "/";
  routes[0].build = route_build;
  routes[0].destroy = route_destroy;
  routes[0].ctx = &home_ctx;
  routes[1].pattern = "/users/:id";
  routes[1].build = route_build;
  routes[1].destroy = route_destroy;
  routes[1].ctx = &user_ctx;
  routes[2].pattern = "/files/*";
  routes[2].build = route_build;
  routes[2].destroy = route_destroy;
  routes[2].ctx = &file_ctx;

  config.routes = routes;
  config.route_count = 3;
  config.stack_capacity = 3;
  CMP_TEST_OK(cmp_router_init(&router, &config));

  CMP_TEST_EXPECT(cmp_router_test_copy_path(NULL, "/", 1, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  router_copy.allocator = cmp_alloc;
  CMP_TEST_EXPECT(
      cmp_router_test_copy_path(&router_copy, "/", max_value, &path_copy),
      CMP_ERR_OVERFLOW);
  router_copy.allocator = null_alloc;
  CMP_TEST_EXPECT(cmp_router_test_copy_path(&router_copy, "/", 1, &path_copy),
                  CMP_ERR_OUT_OF_MEMORY);

  CMP_TEST_OK(cmp_get_default_allocator(&router_manual.allocator));
  router_manual.routes = bad_routes;
  router_manual.route_count = 1;
  router_manual.stack = stack_entries;
  router_manual.stack_capacity = 1;
  router_manual.stack_size = 0;

  bad_routes[0].pattern = NULL;
  bad_routes[0].build = route_build;
  bad_routes[0].destroy = route_destroy;
  bad_routes[0].ctx = &home_ctx;
  CMP_TEST_EXPECT(cmp_router_navigate(&router_manual, "/", &component),
                  CMP_ERR_INVALID_ARGUMENT);

  bad_routes[0].pattern = "/abcd";
  CMP_TEST_OK(cmp_router_test_set_cstr_limit(3));
  CMP_TEST_EXPECT(cmp_router_navigate(&router_manual, "/", &component),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_router_test_set_cstr_limit(0));

  bad_routes[0].pattern = "/files/*/more";
  CMP_TEST_EXPECT(cmp_router_navigate(&router_manual, "/", &component),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_router_navigate(NULL, "/", &component),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_navigate(&router, NULL, &component),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_get_current(&router, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_router_navigate_uri(NULL, "app://example.com", &component),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_navigate_uri(&router, NULL, &component),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_can_back(NULL, &can_back),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_can_back(&router, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_back(NULL, &component), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_get_current(NULL, &current_path, &component),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_router_clear(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_router_navigate(&router, "/missing", &component),
                  CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(cmp_router_navigate(&router, "", &component));
  CMP_TEST_ASSERT(component == &home_ctx);
  CMP_TEST_OK(cmp_router_get_current(&router, &current_path, &component));
  current_slice.data = current_path;
  current_slice.length = 1;
  CMP_TEST_ASSERT(slice_equals(&current_slice, "/"));

  CMP_TEST_OK(
      cmp_router_navigate_uri(&router, "app://example.com", &component));
  CMP_TEST_ASSERT(component == &home_ctx);

  CMP_TEST_OK(cmp_router_navigate_uri(&router, "app://example.com/users/42?x=1",
                                      &component));
  CMP_TEST_ASSERT(component == &user_ctx);
  CMP_TEST_OK(cmp_router_can_back(&router, &can_back));
  CMP_TEST_ASSERT(can_back == CMP_TRUE);

  CMP_TEST_OK(cmp_router_back(&router, &component));
  CMP_TEST_ASSERT(component == &home_ctx);
  CMP_TEST_ASSERT(user_ctx.destroy_called == 1);
  CMP_TEST_OK(cmp_router_can_back(&router, &can_back));
  CMP_TEST_ASSERT(can_back == CMP_FALSE);
  CMP_TEST_EXPECT(cmp_router_back(&router, &component), CMP_ERR_NOT_FOUND);

  user_ctx.build_fail = 1;
  CMP_TEST_EXPECT(cmp_router_navigate(&router, "/users/99", &component),
                  CMP_ERR_UNKNOWN);
  user_ctx.build_fail = 0;

  alloc.fail_alloc_on = alloc.alloc_calls + 1;
  CMP_TEST_EXPECT(cmp_router_navigate(&router, "/users/77", &component),
                  CMP_ERR_OUT_OF_MEMORY);
  alloc.fail_alloc_on = 0;

  CMP_TEST_OK(cmp_router_navigate(&router, "/users/77", &component));
  CMP_TEST_OK(cmp_router_navigate(&router, "/files/a/b", &component));
  CMP_TEST_EXPECT(cmp_router_navigate(&router, "/users/88", &component),
                  CMP_ERR_OVERFLOW);

  file_ctx.destroy_fail = 1;
  CMP_TEST_EXPECT(cmp_router_back(&router, &component), CMP_ERR_UNKNOWN);
  CMP_TEST_ASSERT(component == &user_ctx);
  CMP_TEST_OK(cmp_router_get_current(&router, &current_path, NULL));
  CMP_TEST_ASSERT(current_path != NULL);
  file_ctx.destroy_fail = 0;

  alloc.fail_free_on = alloc.free_calls + 1;
  CMP_TEST_EXPECT(cmp_router_back(&router, &component), CMP_ERR_UNKNOWN);
  CMP_TEST_ASSERT(component == &home_ctx);
  alloc.fail_free_on = 0;
  CMP_TEST_EXPECT(cmp_router_back(&router, &component), CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(cmp_router_clear(&router));
  CMP_TEST_EXPECT(cmp_router_get_current(&router, &current_path, NULL),
                  CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(cmp_router_navigate(&router, "/users/11", &component));
  user_ctx.destroy_fail = 1;
  CMP_TEST_EXPECT(cmp_router_clear(&router), CMP_ERR_UNKNOWN);
  user_ctx.destroy_fail = 0;
  CMP_TEST_OK(cmp_router_clear(&router));

  CMP_TEST_EXPECT(cmp_router_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);
  alloc.fail_free_on = alloc.free_calls + 1;
  CMP_TEST_EXPECT(cmp_router_shutdown(&router), CMP_ERR_UNKNOWN);
  alloc.fail_free_on = 0;
  CMP_TEST_OK(cmp_router_shutdown(&router));

  config.stack_capacity = 0;
  CMP_TEST_OK(cmp_router_init(&router, &config));
  CMP_TEST_EXPECT(cmp_router_navigate(&router, "/", &component),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_router_shutdown(&router));

  CMP_TEST_OK(cmp_router_test_set_cstr_limit(3));
  CMP_TEST_EXPECT(cmp_router_navigate(&router, "abcd", &component),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_EXPECT(cmp_router_navigate_uri(&router, "abcd", &component),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_EXPECT(cmp_route_test_match("abcd", "/a", NULL, 0, NULL, &match),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_EXPECT(cmp_route_test_match("a", "/abcd", NULL, 0, NULL, &match),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_EXPECT(cmp_uri_parse("abcd", &uri), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_router_test_set_cstr_limit(0));

  return 0;
}
