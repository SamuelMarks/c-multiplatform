#include "m3/m3_router.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAllocator {
  m3_usize alloc_calls;
  m3_usize free_calls;
  m3_usize realloc_calls;
  m3_usize fail_alloc_on;
  m3_usize fail_free_on;
} TestAllocator;

static int test_alloc(void *ctx, m3_usize size, void **out_ptr) {
  TestAllocator *alloc;

  if (ctx == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on != 0 && alloc->alloc_calls == alloc->fail_alloc_on) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = malloc(size);
  if (*out_ptr == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }
  return M3_OK;
}

static int test_realloc(void *ctx, void *ptr, m3_usize size, void **out_ptr) {
  TestAllocator *alloc;
  void *mem;

  if (ctx == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->realloc_calls += 1;
  mem = realloc(ptr, size);
  if (mem == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }
  *out_ptr = mem;
  return M3_OK;
}

static int test_free(void *ctx, void *ptr) {
  TestAllocator *alloc;

  if (ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_on != 0 && alloc->free_calls == alloc->fail_free_on) {
    return M3_ERR_UNKNOWN;
  }

  free(ptr);
  return M3_OK;
}

static int test_alloc_null(void *ctx, m3_usize size, void **out_ptr) {
  M3_UNUSED(ctx);
  M3_UNUSED(size);

  if (out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;
  return M3_OK;
}

static void test_allocator_init(TestAllocator *alloc) {
  alloc->alloc_calls = 0;
  alloc->free_calls = 0;
  alloc->realloc_calls = 0;
  alloc->fail_alloc_on = 0;
  alloc->fail_free_on = 0;
}

static int slice_equals(const M3UriSlice *slice, const char *text) {
  m3_usize len;
  m3_usize i;

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
    return M3_ERR_INVALID_ARGUMENT;
  }

  route_ctx = (RouteCtx *)ctx;
  route_ctx->build_called += 1;
  if (route_ctx->build_fail) {
    return M3_ERR_UNKNOWN;
  }

  *out_component = ctx;
  return M3_OK;
}

static int route_destroy(void *ctx, void *component) {
  RouteCtx *route_ctx;

  if (ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  route_ctx = (RouteCtx *)ctx;
  route_ctx->destroy_called += 1;
  if (route_ctx->destroy_fail) {
    return M3_ERR_UNKNOWN;
  }

  M3_UNUSED(component);
  return M3_OK;
}

int main(void) {
  M3Uri uri;
  M3UriSlice value;
  M3UriSlice current_slice;
  M3UriSlice temp_slice;
  M3Bool found;
  M3Bool match;
  M3Bool has_more;
  m3_usize param_count;
  m3_usize max_value;
  m3_usize overflow_capacity;
  M3RouteParam params[2];
  TestAllocator alloc;
  M3Allocator m3_alloc;
  M3Allocator null_alloc;
  M3Router router;
  M3Router router_copy;
  M3Router router_manual;
  M3RouterConfig config;
  RouteCtx home_ctx;
  RouteCtx user_ctx;
  RouteCtx file_ctx;
  M3Route routes[3];
  M3Route bad_routes[1];
  M3RouteEntry stack_entries[1];
  const M3Route *found_route;
  const char *current_path;
  char *path_copy;
  void *component;
  M3Bool can_back;

  memset(&router_copy, 0, sizeof(router_copy));
  memset(&router_manual, 0, sizeof(router_manual));

  M3_TEST_EXPECT(m3_uri_parse(NULL, &uri), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_uri_parse("x", NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_router_test_cstrlen(NULL, &max_value),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_test_cstrlen("x", NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_router_test_slice_set(NULL, "x", 1));
  temp_slice.data = "bar";
  temp_slice.length = 3;
  M3_TEST_EXPECT(m3_router_test_slice_equals(NULL, "bar", 3, &match),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_test_slice_equals(&temp_slice, NULL, 3, &match),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_test_slice_equals(&temp_slice, "bar", 3, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  temp_slice.data = NULL;
  temp_slice.length = 0;
  M3_TEST_OK(m3_router_test_slice_equals(&temp_slice, "", 0, &match));
  M3_TEST_ASSERT(match == M3_TRUE);
  temp_slice.data = "bar";
  temp_slice.length = 3;
  M3_TEST_OK(m3_router_test_slice_equals(&temp_slice, "foo", 3, &match));
  M3_TEST_ASSERT(match == M3_FALSE);
  M3_TEST_OK(m3_router_test_trim_trailing(NULL, NULL));
  {
    m3_usize idx;
    const char *seg;
    m3_usize seg_len;

    idx = 0;
    seg = NULL;
    seg_len = 0;
    M3_TEST_OK(m3_router_test_next_segment(NULL, 0, &idx, &seg, &seg_len));
  }
  M3_TEST_OK(m3_router_test_has_more_segments(NULL, 0, 0, &has_more));
  M3_TEST_ASSERT(has_more == M3_FALSE);
  M3_TEST_EXPECT(
      m3_router_test_match_len(NULL, 0, "/", 1, NULL, 0, NULL, &match),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_test_validate_pattern(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_router_test_validate_pattern(""));
  M3_TEST_EXPECT(m3_router_test_validate_pattern("/users/:"),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_test_release_entry(NULL, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_test_navigate_path_len(NULL, "/", 1, &component),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_test_has_more_segments(NULL, 0, 0, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  found_route = NULL;
  M3_TEST_EXPECT(m3_router_test_find_route(NULL, "/", 1, &found_route),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_test_find_route(&router_manual, "/", 1, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_router_test_find_route(&router_manual, "/", 1, &found_route),
      M3_ERR_NOT_FOUND);
  M3_TEST_OK(
      m3_uri_parse("app://example.com/path/one?foo=bar&flag#frag", &uri));
  M3_TEST_ASSERT(slice_equals(&uri.scheme, "app"));
  M3_TEST_ASSERT(slice_equals(&uri.authority, "example.com"));
  M3_TEST_ASSERT(slice_equals(&uri.path, "/path/one"));
  M3_TEST_ASSERT(slice_equals(&uri.query, "foo=bar&flag"));
  M3_TEST_ASSERT(slice_equals(&uri.fragment, "frag"));

  M3_TEST_OK(m3_uri_query_find(&uri, "foo", &value, &found));
  M3_TEST_ASSERT(found == M3_TRUE);
  M3_TEST_ASSERT(slice_equals(&value, "bar"));
  M3_TEST_OK(m3_uri_query_find(&uri, "flag", &value, &found));
  M3_TEST_ASSERT(found == M3_TRUE);
  M3_TEST_ASSERT(value.length == 0);
  M3_TEST_OK(m3_uri_query_find(&uri, "missing", &value, &found));
  M3_TEST_ASSERT(found == M3_FALSE);
#ifdef M3_TESTING
  M3_TEST_OK(m3_router_test_set_slice_equals_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_uri_query_find(&uri, "foo", &value, &found),
                 M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_router_test_set_slice_equals_fail(M3_FALSE));
#endif

  M3_TEST_OK(m3_uri_parse("app://example.com/path#frag", &uri));
  M3_TEST_ASSERT(slice_equals(&uri.path, "/path"));
  M3_TEST_ASSERT(slice_equals(&uri.fragment, "frag"));

  M3_TEST_OK(m3_uri_parse("app://example.com/path?bar=baz", &uri));
  M3_TEST_OK(m3_uri_query_find(&uri, "foo", &value, &found));
  M3_TEST_ASSERT(found == M3_FALSE);

  M3_TEST_EXPECT(m3_uri_query_find(NULL, "foo", &value, &found),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_uri_query_find(&uri, NULL, &value, &found),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_uri_query_find(&uri, "foo", NULL, &found),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_uri_query_find(&uri, "foo", &value, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_uri_query_find(&uri, "", &value, &found), M3_ERR_RANGE);

  M3_TEST_OK(m3_uri_parse("app://example.com/path", &uri));
  M3_TEST_OK(m3_uri_query_find(&uri, "foo", &value, &found));
  M3_TEST_ASSERT(found == M3_FALSE);

  M3_TEST_OK(m3_router_test_set_cstr_limit(3));
  M3_TEST_EXPECT(m3_uri_query_find(&uri, "abcd", &value, &found),
                 M3_ERR_OVERFLOW);
  M3_TEST_EXPECT(m3_router_test_validate_pattern("abcd"), M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_router_test_set_cstr_limit(0));

  {
    M3Router find_router;
    M3Route route;

    memset(&find_router, 0, sizeof(find_router));
    route.pattern = "/home";
    route.build = route_build;
    route.destroy = route_destroy;
    route.ctx = &home_ctx;
    find_router.routes = &route;
    find_router.route_count = 1;

    M3_TEST_OK(m3_router_test_set_cstr_limit(1));
    M3_TEST_EXPECT(
        m3_router_test_find_route(&find_router, "/home", 5, &found_route),
        M3_ERR_OVERFLOW);
    M3_TEST_OK(m3_router_test_set_cstr_limit(0));

    M3_TEST_OK(m3_router_test_set_cstr_fail_after(2u));
    M3_TEST_EXPECT(
        m3_router_test_find_route(&find_router, "/home", 5, &found_route),
        M3_ERR_OVERFLOW);
    M3_TEST_OK(m3_router_test_set_cstr_fail_after(0u));

    M3_TEST_EXPECT(
        m3_router_test_find_route(&find_router, NULL, 0, &found_route),
        M3_ERR_INVALID_ARGUMENT);
  }

  {
    M3Router find_router;
    M3Route routes_local[2];

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
    M3_TEST_OK(
        m3_router_test_find_route(&find_router, "/home", 5, &found_route));
    M3_TEST_ASSERT(found_route == &routes_local[0]);
    M3_TEST_EXPECT(
        m3_router_test_find_route(&find_router, "/missing", 8, &found_route),
        M3_ERR_NOT_FOUND);

    routes_local[0].build = NULL;
    M3_TEST_EXPECT(
        m3_router_test_find_route(&find_router, "/home", 5, &found_route),
        M3_ERR_INVALID_ARGUMENT);
    routes_local[0].build = route_build;
    routes_local[0].pattern = "/bad/:";
    M3_TEST_EXPECT(
        m3_router_test_find_route(&find_router, "/bad", 4, &found_route),
        M3_ERR_INVALID_ARGUMENT);
    routes_local[0].pattern = NULL;
    M3_TEST_EXPECT(
        m3_router_test_find_route(&find_router, "/home", 5, &found_route),
        M3_ERR_INVALID_ARGUMENT);
  }

  {
    M3Router local_router;
    M3RouteEntry local_stack[1];

    memset(&local_router, 0, sizeof(local_router));
    local_router.stack = local_stack;
    local_router.stack_capacity = 1;
    local_router.stack_size = 1;
    local_stack[0].path = "/overflow";
    local_stack[0].component = NULL;

    M3_TEST_OK(m3_router_test_set_cstr_limit(1));
    M3_TEST_EXPECT(
        m3_router_test_navigate_path_len(&local_router, "/next", 5, &component),
        M3_ERR_OVERFLOW);
    M3_TEST_OK(m3_router_test_set_cstr_limit(0));
  }

  M3_TEST_OK(m3_uri_parse("/local/path", &uri));
  M3_TEST_ASSERT(uri.scheme.length == 0);
  M3_TEST_ASSERT(uri.authority.length == 0);
  M3_TEST_ASSERT(slice_equals(&uri.path, "/local/path"));

  M3_TEST_OK(m3_uri_parse("noscheme", &uri));
  M3_TEST_ASSERT(uri.scheme.length == 0);
  M3_TEST_ASSERT(slice_equals(&uri.path, "noscheme"));

  M3_TEST_OK(m3_uri_parse("app://host", &uri));
  M3_TEST_ASSERT(slice_equals(&uri.scheme, "app"));
  M3_TEST_ASSERT(slice_equals(&uri.authority, "host"));
  M3_TEST_ASSERT(uri.path.length == 0);

  M3_TEST_OK(m3_uri_parse("", &uri));
  M3_TEST_ASSERT(uri.path.length == 0);

  M3_TEST_EXPECT(m3_route_test_match(NULL, "/a", NULL, 0, NULL, &match),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_route_test_match("/a", NULL, NULL, 0, NULL, &match),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_route_test_match("/a", "/a", NULL, 0, &param_count, &match),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_route_test_match("", "", NULL, 0, NULL, &match));
  M3_TEST_ASSERT(match == M3_TRUE);
  M3_TEST_OK(m3_route_test_match("*", "", NULL, 0, NULL, &match));
  M3_TEST_ASSERT(match == M3_TRUE);
  M3_TEST_OK(m3_route_test_match("*", "/files", NULL, 0, NULL, &match));
  M3_TEST_ASSERT(match == M3_TRUE);

  M3_TEST_OK(m3_route_test_match("/home", "/home", NULL, 0, NULL, &match));
  M3_TEST_ASSERT(match == M3_TRUE);
  M3_TEST_OK(m3_route_test_match("/home/", "/home", NULL, 0, NULL, &match));
  M3_TEST_ASSERT(match == M3_TRUE);
  M3_TEST_OK(m3_route_test_match("/home", "/home/", NULL, 0, NULL, &match));
  M3_TEST_ASSERT(match == M3_TRUE);

  M3_TEST_OK(m3_route_test_match("/users/:id", "/users/42", params, 2,
                                 &param_count, &match));
  M3_TEST_ASSERT(match == M3_TRUE);
  M3_TEST_ASSERT(param_count == 1);
  M3_TEST_ASSERT(slice_equals(&params[0].key, "id"));
  M3_TEST_ASSERT(slice_equals(&params[0].value, "42"));

  M3_TEST_EXPECT(m3_route_test_match("/users/:", "/users/42", params, 2,
                                     &param_count, &match),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_route_test_match("/files/*/more", "/files/x", params, 2,
                                     &param_count, &match),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_route_test_match("/files/*/more", "/files", params, 2,
                                     &param_count, &match),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(
      m3_route_test_match("/files/*", "/files/a/b/c", NULL, 0, NULL, &match));
  M3_TEST_ASSERT(match == M3_TRUE);
  M3_TEST_OK(m3_route_test_match("/files/*", "/files/a/b", params, 2,
                                 &param_count, &match));
  M3_TEST_ASSERT(match == M3_TRUE);
  M3_TEST_ASSERT(param_count == 0);
  M3_TEST_OK(m3_route_test_match("/files/*", "/files", params, 2, &param_count,
                                 &match));
  M3_TEST_ASSERT(match == M3_TRUE);
  M3_TEST_ASSERT(param_count == 0);

  M3_TEST_EXPECT(m3_route_test_match("/users/:id", "/users/42", params, 0,
                                     &param_count, &match),
                 M3_ERR_OVERFLOW);

  M3_TEST_OK(m3_route_test_match("/a/b", "/a", NULL, 0, NULL, &match));
  M3_TEST_ASSERT(match == M3_FALSE);
  M3_TEST_OK(m3_route_test_match("/a", "/a/b", NULL, 0, NULL, &match));
  M3_TEST_ASSERT(match == M3_FALSE);

  test_allocator_init(&alloc);
  m3_alloc.ctx = &alloc;
  m3_alloc.alloc = test_alloc;
  m3_alloc.realloc = test_realloc;
  m3_alloc.free = test_free;

  memset(&config, 0, sizeof(config));
  M3_TEST_EXPECT(m3_router_init(NULL, &config), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_init(&router, NULL), M3_ERR_INVALID_ARGUMENT);

  config.allocator = &m3_alloc;
  config.routes = NULL;
  config.route_count = 1;
  config.stack_capacity = 1;
  M3_TEST_EXPECT(m3_router_init(&router, &config), M3_ERR_INVALID_ARGUMENT);

  routes[0].pattern = NULL;
  routes[0].build = route_build;
  routes[0].destroy = route_destroy;
  routes[0].ctx = &home_ctx;
  config.routes = routes;
  config.route_count = 1;
  M3_TEST_EXPECT(m3_router_init(&router, &config), M3_ERR_INVALID_ARGUMENT);

  routes[0].pattern = "/";
  routes[0].build = NULL;
  M3_TEST_EXPECT(m3_router_init(&router, &config), M3_ERR_INVALID_ARGUMENT);

  routes[0].build = route_build;
  routes[0].pattern = "/files/*/more";
  M3_TEST_EXPECT(m3_router_init(&router, &config), M3_ERR_INVALID_ARGUMENT);

#ifdef M3_TESTING
  config.allocator = NULL;
  routes[0].pattern = "/";
  M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_router_init(&router, &config), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));
  config.allocator = &m3_alloc;
#endif

  null_alloc.ctx = &alloc;
  null_alloc.alloc = test_alloc_null;
  null_alloc.realloc = test_realloc;
  null_alloc.free = test_free;
  config.allocator = &null_alloc;
  routes[0].pattern = "/";
  M3_TEST_EXPECT(m3_router_init(&router, &config), M3_ERR_OUT_OF_MEMORY);
  config.allocator = &m3_alloc;

  max_value = (m3_usize) ~(m3_usize)0;
  overflow_capacity = max_value / sizeof(M3RouteEntry);
  if (overflow_capacity < max_value) {
    overflow_capacity += 1;
  }
  config.stack_capacity = overflow_capacity;
  M3_TEST_EXPECT(m3_router_init(&router, &config), M3_ERR_OVERFLOW);

  config.stack_capacity = 1;
  alloc.fail_alloc_on = 1;
  M3_TEST_EXPECT(m3_router_init(&router, &config), M3_ERR_OUT_OF_MEMORY);
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
  M3_TEST_OK(m3_router_init(&router, &config));

  M3_TEST_EXPECT(m3_router_test_copy_path(NULL, "/", 1, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  router_copy.allocator = m3_alloc;
  M3_TEST_EXPECT(
      m3_router_test_copy_path(&router_copy, "/", max_value, &path_copy),
      M3_ERR_OVERFLOW);
  router_copy.allocator = null_alloc;
  M3_TEST_EXPECT(m3_router_test_copy_path(&router_copy, "/", 1, &path_copy),
                 M3_ERR_OUT_OF_MEMORY);

  M3_TEST_OK(m3_get_default_allocator(&router_manual.allocator));
  router_manual.routes = bad_routes;
  router_manual.route_count = 1;
  router_manual.stack = stack_entries;
  router_manual.stack_capacity = 1;
  router_manual.stack_size = 0;

  bad_routes[0].pattern = NULL;
  bad_routes[0].build = route_build;
  bad_routes[0].destroy = route_destroy;
  bad_routes[0].ctx = &home_ctx;
  M3_TEST_EXPECT(m3_router_navigate(&router_manual, "/", &component),
                 M3_ERR_INVALID_ARGUMENT);

  bad_routes[0].pattern = "/abcd";
  M3_TEST_OK(m3_router_test_set_cstr_limit(3));
  M3_TEST_EXPECT(m3_router_navigate(&router_manual, "/", &component),
                 M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_router_test_set_cstr_limit(0));

  bad_routes[0].pattern = "/files/*/more";
  M3_TEST_EXPECT(m3_router_navigate(&router_manual, "/", &component),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_router_navigate(NULL, "/", &component),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_navigate(&router, NULL, &component),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_get_current(&router, NULL, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_navigate_uri(NULL, "app://example.com", &component),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_navigate_uri(&router, NULL, &component),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_can_back(NULL, &can_back), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_can_back(&router, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_back(NULL, &component), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_get_current(NULL, &current_path, &component),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_router_clear(NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_router_navigate(&router, "/missing", &component),
                 M3_ERR_NOT_FOUND);

  M3_TEST_OK(m3_router_navigate(&router, "", &component));
  M3_TEST_ASSERT(component == &home_ctx);
  M3_TEST_OK(m3_router_get_current(&router, &current_path, &component));
  current_slice.data = current_path;
  current_slice.length = 1;
  M3_TEST_ASSERT(slice_equals(&current_slice, "/"));

  M3_TEST_OK(m3_router_navigate_uri(&router, "app://example.com", &component));
  M3_TEST_ASSERT(component == &home_ctx);

  M3_TEST_OK(m3_router_navigate_uri(&router, "app://example.com/users/42?x=1",
                                    &component));
  M3_TEST_ASSERT(component == &user_ctx);
  M3_TEST_OK(m3_router_can_back(&router, &can_back));
  M3_TEST_ASSERT(can_back == M3_TRUE);

  M3_TEST_OK(m3_router_back(&router, &component));
  M3_TEST_ASSERT(component == &home_ctx);
  M3_TEST_ASSERT(user_ctx.destroy_called == 1);
  M3_TEST_OK(m3_router_can_back(&router, &can_back));
  M3_TEST_ASSERT(can_back == M3_FALSE);
  M3_TEST_EXPECT(m3_router_back(&router, &component), M3_ERR_NOT_FOUND);

  user_ctx.build_fail = 1;
  M3_TEST_EXPECT(m3_router_navigate(&router, "/users/99", &component),
                 M3_ERR_UNKNOWN);
  user_ctx.build_fail = 0;

  alloc.fail_alloc_on = alloc.alloc_calls + 1;
  M3_TEST_EXPECT(m3_router_navigate(&router, "/users/77", &component),
                 M3_ERR_OUT_OF_MEMORY);
  alloc.fail_alloc_on = 0;

  M3_TEST_OK(m3_router_navigate(&router, "/users/77", &component));
  M3_TEST_OK(m3_router_navigate(&router, "/files/a/b", &component));
  M3_TEST_EXPECT(m3_router_navigate(&router, "/users/88", &component),
                 M3_ERR_OVERFLOW);

  file_ctx.destroy_fail = 1;
  M3_TEST_EXPECT(m3_router_back(&router, &component), M3_ERR_UNKNOWN);
  M3_TEST_ASSERT(component == &user_ctx);
  M3_TEST_OK(m3_router_get_current(&router, &current_path, NULL));
  M3_TEST_ASSERT(current_path != NULL);
  file_ctx.destroy_fail = 0;

  alloc.fail_free_on = alloc.free_calls + 1;
  M3_TEST_EXPECT(m3_router_back(&router, &component), M3_ERR_UNKNOWN);
  M3_TEST_ASSERT(component == &home_ctx);
  alloc.fail_free_on = 0;
  M3_TEST_EXPECT(m3_router_back(&router, &component), M3_ERR_NOT_FOUND);

  M3_TEST_OK(m3_router_clear(&router));
  M3_TEST_EXPECT(m3_router_get_current(&router, &current_path, NULL),
                 M3_ERR_NOT_FOUND);

  M3_TEST_OK(m3_router_navigate(&router, "/users/11", &component));
  user_ctx.destroy_fail = 1;
  M3_TEST_EXPECT(m3_router_clear(&router), M3_ERR_UNKNOWN);
  user_ctx.destroy_fail = 0;
  M3_TEST_OK(m3_router_clear(&router));

  M3_TEST_EXPECT(m3_router_shutdown(NULL), M3_ERR_INVALID_ARGUMENT);
  alloc.fail_free_on = alloc.free_calls + 1;
  M3_TEST_EXPECT(m3_router_shutdown(&router), M3_ERR_UNKNOWN);
  alloc.fail_free_on = 0;
  M3_TEST_OK(m3_router_shutdown(&router));

  config.stack_capacity = 0;
  M3_TEST_OK(m3_router_init(&router, &config));
  M3_TEST_EXPECT(m3_router_navigate(&router, "/", &component), M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_router_shutdown(&router));

  M3_TEST_OK(m3_router_test_set_cstr_limit(3));
  M3_TEST_EXPECT(m3_router_navigate(&router, "abcd", &component),
                 M3_ERR_OVERFLOW);
  M3_TEST_EXPECT(m3_router_navigate_uri(&router, "abcd", &component),
                 M3_ERR_OVERFLOW);
  M3_TEST_EXPECT(m3_route_test_match("abcd", "/a", NULL, 0, NULL, &match),
                 M3_ERR_OVERFLOW);
  M3_TEST_EXPECT(m3_route_test_match("a", "/abcd", NULL, 0, NULL, &match),
                 M3_ERR_OVERFLOW);
  M3_TEST_EXPECT(m3_uri_parse("abcd", &uri), M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_router_test_set_cstr_limit(0));

  return 0;
}
