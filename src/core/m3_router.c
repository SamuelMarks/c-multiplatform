#include "m3/m3_router.h"

#include <string.h>

#ifdef M3_TESTING
static m3_usize g_router_cstr_limit_override = 0;
static M3Bool g_router_force_slice_equals_fail = M3_FALSE;
static m3_u32 g_router_cstr_fail_after = 0u;
static m3_u32 g_router_cstr_call_count = 0u;
#endif

static const char g_router_root_path[] = "/";

static m3_usize m3_router_usize_max_value(void) {
  return (m3_usize) ~(m3_usize)0;
}

static m3_usize m3_router_cstr_limit(void) {
#ifdef M3_TESTING
  if (g_router_cstr_limit_override != 0) {
    return g_router_cstr_limit_override;
  }
#endif
  return m3_router_usize_max_value();
}

static int m3_router_cstrlen(const char *cstr, m3_usize *out_len) {
  m3_usize max_len;
  m3_usize len;

  if (cstr == NULL || out_len == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

#ifdef M3_TESTING
  if (g_router_cstr_fail_after != 0u) {
    g_router_cstr_call_count += 1u;
    if (g_router_cstr_call_count >= g_router_cstr_fail_after) {
      return M3_ERR_OVERFLOW;
    }
  }
#endif

  max_len = m3_router_cstr_limit();
  len = 0;

  while (cstr[len] != '\0') {
    if (len == max_len) {
      return M3_ERR_OVERFLOW;
    }
    len += 1;
  }

  *out_len = len;
  return M3_OK;
}

static void m3_router_slice_set(M3UriSlice *slice, const char *data,
                                m3_usize length) {
  if (slice == NULL) {
    return;
  }

  if (length == 0) {
    slice->data = NULL;
    slice->length = 0;
  } else {
    slice->data = data;
    slice->length = length;
  }
}

int M3_CALL m3_uri_parse(const char *uri, M3Uri *out_uri) {
  m3_usize len;
  m3_usize i;
  m3_usize scheme_end;
  m3_usize authority_start;
  m3_usize authority_end;
  m3_usize path_start;
  m3_usize path_end;
  m3_usize query_start;
  m3_usize query_end;
  m3_usize fragment_start;
  int rc;

  if (uri == NULL || out_uri == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_router_cstrlen(uri, &len);
  if (rc != M3_OK) {
    return rc;
  }

  m3_router_slice_set(&out_uri->scheme, NULL, 0);
  m3_router_slice_set(&out_uri->authority, NULL, 0);
  m3_router_slice_set(&out_uri->path, NULL, 0);
  m3_router_slice_set(&out_uri->query, NULL, 0);
  m3_router_slice_set(&out_uri->fragment, NULL, 0);

  if (len == 0) {
    return M3_OK;
  }

  scheme_end = 0;
  authority_start = 0;
  authority_end = 0;
  path_start = 0;
  path_end = len;
  query_start = len;
  query_end = len;
  fragment_start = len;

  for (i = 0; i + 2 < len; ++i) {
    if (uri[i] == ':' && uri[i + 1] == '/' && uri[i + 2] == '/') {
      scheme_end = i;
      authority_start = i + 3;
      path_start = authority_start;
      break;
    }
  }

  if (scheme_end > 0) {
    m3_router_slice_set(&out_uri->scheme, uri, scheme_end);
    for (i = authority_start; i < len; ++i) {
      if (uri[i] == '/' || uri[i] == '?' || uri[i] == '#') {
        authority_end = i;
        break;
      }
    }
    if (authority_end == 0) {
      authority_end = len;
    }
    m3_router_slice_set(&out_uri->authority, uri + authority_start,
                        authority_end - authority_start);
    path_start = authority_end;
  } else {
    path_start = 0;
  }

  for (i = path_start; i < len; ++i) {
    if (uri[i] == '?') {
      path_end = i;
      query_start = i + 1;
      break;
    }
    if (uri[i] == '#') {
      path_end = i;
      fragment_start = i + 1;
      break;
    }
  }

  if (query_start < len) {
    for (i = query_start; i < len; ++i) {
      if (uri[i] == '#') {
        query_end = i;
        fragment_start = i + 1;
        break;
      }
    }
    if (query_end == len) {
      query_end = len;
    }
  }

  if (fragment_start < len && query_start == len) {
    fragment_start = path_end + 1;
  }

  if (path_end > path_start) {
    m3_router_slice_set(&out_uri->path, uri + path_start,
                        path_end - path_start);
  }
  if (query_start < len) {
    m3_router_slice_set(&out_uri->query, uri + query_start,
                        query_end - query_start);
  }
  if (fragment_start < len) {
    m3_router_slice_set(&out_uri->fragment, uri + fragment_start,
                        len - fragment_start);
  }

  return M3_OK;
}

static int m3_router_slice_equals(const M3UriSlice *slice, const char *key,
                                  m3_usize key_len, M3Bool *out_equal) {
  if (slice == NULL || key == NULL || out_equal == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (g_router_force_slice_equals_fail) {
    return M3_ERR_UNKNOWN;
  }
#endif

  if (slice->length != key_len) {
    *out_equal = M3_FALSE;
    return M3_OK;
  }

  if (slice->length == 0) {
    *out_equal = M3_TRUE;
    return M3_OK;
  }

  if (memcmp(slice->data, key, (size_t)slice->length) == 0) {
    *out_equal = M3_TRUE;
  } else {
    *out_equal = M3_FALSE;
  }
  return M3_OK;
}

int M3_CALL m3_uri_query_find(const M3Uri *uri, const char *key,
                              M3UriSlice *out_value, M3Bool *out_found) {
  m3_usize key_len;
  m3_usize start;
  m3_usize end;
  m3_usize eq_pos;
  M3UriSlice key_slice;
  M3Bool equal;
  int rc;

  if (uri == NULL || key == NULL || out_value == NULL || out_found == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_router_cstrlen(key, &key_len);
  if (rc != M3_OK) {
    return rc;
  }
  if (key_len == 0) {
    return M3_ERR_RANGE;
  }

  out_value->data = NULL;
  out_value->length = 0;
  *out_found = M3_FALSE;

  if (uri->query.length == 0) {
    return M3_OK;
  }

  start = 0;
  while (start <= uri->query.length) {
    end = start;
    while (end < uri->query.length && uri->query.data[end] != '&') {
      end += 1;
    }

    eq_pos = start;
    while (eq_pos < end && uri->query.data[eq_pos] != '=') {
      eq_pos += 1;
    }

    m3_router_slice_set(&key_slice, uri->query.data + start, eq_pos - start);
    rc = m3_router_slice_equals(&key_slice, key, key_len, &equal);
    if (rc != M3_OK) {
      return rc;
    }
    if (equal == M3_TRUE) {
      if (eq_pos < end && uri->query.data[eq_pos] == '=') {
        m3_router_slice_set(out_value, uri->query.data + eq_pos + 1,
                            end - eq_pos - 1);
      } else {
        m3_router_slice_set(out_value, NULL, 0);
      }
      *out_found = M3_TRUE;
      return M3_OK;
    }

    if (end >= uri->query.length) {
      break;
    }
    start = end + 1;
  }

  return M3_OK;
}

static void m3_route_trim_trailing(const char **str, m3_usize *len) {
  m3_usize length;

  if (str == NULL || len == NULL) {
    return;
  }

  length = *len;
  while (length > 1 && (*str)[length - 1] == '/') {
    length -= 1;
  }
  *len = length;
}

static void m3_route_next_segment(const char *str, m3_usize len,
                                  m3_usize *index, const char **out_seg,
                                  m3_usize *out_len) {
  m3_usize i;
  m3_usize start;

  if (out_seg != NULL) {
    *out_seg = NULL;
  }
  if (out_len != NULL) {
    *out_len = 0;
  }

  if (str == NULL || index == NULL || out_seg == NULL || out_len == NULL) {
    return;
  }

  i = *index;
  while (i < len && str[i] == '/') {
    i += 1;
  }
  if (i >= len) {
    *index = i;
    return;
  }

  start = i;
  while (i < len && str[i] != '/') {
    i += 1;
  }

  *out_seg = str + start;
  *out_len = i - start;
  *index = i;
}

static M3Bool m3_route_has_more_segments(const char *str, m3_usize len,
                                         m3_usize index) {
  m3_usize i;

  if (str == NULL) {
    return M3_FALSE;
  }

  i = index;
  while (i < len && str[i] == '/') {
    i += 1;
  }

  return (i < len) ? M3_TRUE : M3_FALSE;
}

static int m3_route_match_len(const char *pattern, m3_usize pattern_len,
                              const char *path, m3_usize path_len,
                              M3RouteParam *params, m3_usize max_params,
                              m3_usize *out_param_count, M3Bool *out_match) {
  const char *pat_str;
  const char *path_str;
  m3_usize pat_len;
  m3_usize p_idx;
  m3_usize s_idx;
  m3_usize param_count;

  if (pattern == NULL || path == NULL || out_match == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (params == NULL && out_param_count != NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  param_count = 0;
  *out_match = M3_FALSE;
  if (out_param_count != NULL) {
    *out_param_count = 0;
  }

  pat_str = pattern;
  path_str = path;
  pat_len = pattern_len;

  if (pat_len == 0) {
    pat_str = g_router_root_path;
    pat_len = 1;
  }

  if (path_len == 0) {
    path_str = g_router_root_path;
    path_len = 1;
  }

  m3_route_trim_trailing(&pat_str, &pat_len);
  m3_route_trim_trailing(&path_str, &path_len);

  p_idx = 0;
  s_idx = 0;

  for (;;) {
    const char *pat_seg;
    const char *path_seg;
    m3_usize pat_seg_len;
    m3_usize path_seg_len;
    M3Bool has_more;

    m3_route_next_segment(pat_str, pat_len, &p_idx, &pat_seg, &pat_seg_len);
    m3_route_next_segment(path_str, path_len, &s_idx, &path_seg, &path_seg_len);

    if (pat_seg == NULL && path_seg == NULL) {
      *out_match = M3_TRUE;
      if (out_param_count != NULL) {
        *out_param_count = param_count;
      }
      return M3_OK;
    }
    if (pat_seg == NULL) {
      return M3_OK;
    }
    if (path_seg == NULL) {
      if (pat_seg_len == 1 && pat_seg[0] == '*') {
        has_more = m3_route_has_more_segments(pat_str, pat_len, p_idx);
        if (has_more == M3_TRUE) {
          return M3_ERR_INVALID_ARGUMENT;
        }
        *out_match = M3_TRUE;
        if (out_param_count != NULL) {
          *out_param_count = param_count;
        }
      }
      return M3_OK;
    }

    if (pat_seg_len == 1 && pat_seg[0] == '*') {
      has_more = m3_route_has_more_segments(pat_str, pat_len, p_idx);
      if (has_more == M3_TRUE) {
        return M3_ERR_INVALID_ARGUMENT;
      }
      *out_match = M3_TRUE;
      if (out_param_count != NULL) {
        *out_param_count = param_count;
      }
      return M3_OK;
    }

    if (pat_seg_len > 0 && pat_seg[0] == ':') {
      if (pat_seg_len == 1) {
        return M3_ERR_INVALID_ARGUMENT;
      }
      if (params != NULL) {
        if (param_count >= max_params) {
          return M3_ERR_OVERFLOW;
        }
        params[param_count].key.data = pat_seg + 1;
        params[param_count].key.length = pat_seg_len - 1;
        params[param_count].value.data = path_seg;
        params[param_count].value.length = path_seg_len;
        param_count += 1;
      }
      continue;
    }

    if (pat_seg_len != path_seg_len) {
      return M3_OK;
    }
    if (memcmp(pat_seg, path_seg, (size_t)pat_seg_len) != 0) {
      return M3_OK;
    }
  }
}

static int m3_route_validate_pattern(const char *pattern) {
  m3_usize len;
  const char *pat_str;
  m3_usize pat_len;
  m3_usize index;
  const char *seg;
  m3_usize seg_len;
  M3Bool has_more;
  int rc;

  if (pattern == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_router_cstrlen(pattern, &len);
  if (rc != M3_OK) {
    return rc;
  }

  if (len == 0) {
    return M3_OK;
  }

  pat_str = pattern;
  pat_len = len;
  m3_route_trim_trailing(&pat_str, &pat_len);

  index = 0;
  for (;;) {
    m3_route_next_segment(pat_str, pat_len, &index, &seg, &seg_len);
    if (seg == NULL) {
      return M3_OK;
    }
    if (seg_len == 1 && seg[0] == '*') {
      has_more = m3_route_has_more_segments(pat_str, pat_len, index);
      if (has_more == M3_TRUE) {
        return M3_ERR_INVALID_ARGUMENT;
      }
      return M3_OK;
    }
    if (seg_len > 0 && seg[0] == ':') {
      if (seg_len == 1) {
        return M3_ERR_INVALID_ARGUMENT;
      }
    }
  }
}

static int m3_router_release_entry(M3Router *router, M3RouteEntry *entry) {
  int rc;
  int rc_next;

  if (router == NULL || entry == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = M3_OK;

  if (entry->route != NULL && entry->route->destroy != NULL) {
    rc_next = entry->route->destroy(entry->route->ctx, entry->component);
    if (rc == M3_OK) {
      rc = rc_next;
    }
  }

  if (entry->path != NULL) {
    rc_next = router->allocator.free(router->allocator.ctx, entry->path);
    if (rc == M3_OK) {
      rc = rc_next;
    }
  }

  entry->route = NULL;
  entry->path = NULL;
  entry->component = NULL;
  return rc;
}

static int m3_router_find_route(const M3Router *router, const char *path,
                                m3_usize path_len, const M3Route **out_route) {
  m3_usize i;
  M3Bool match; /* GCOVR_EXCL_LINE */
  m3_usize pattern_len;
  int rc;

  if (router == NULL || out_route == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (router->route_count == 0 || router->routes == NULL) {
    return M3_ERR_NOT_FOUND;
  }

  for (i = 0; i < router->route_count; ++i) {
    const M3Route *route; /* GCOVR_EXCL_LINE */

    route = &router->routes[i];
    if (route->pattern == NULL || route->build == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    rc = m3_route_validate_pattern(route->pattern);
    if (rc != M3_OK) {
      return rc;
    }

    rc = m3_router_cstrlen(route->pattern, &pattern_len);
    if (rc != M3_OK) {
      return rc;
    }

    match = M3_FALSE;
    rc = m3_route_match_len(route->pattern, pattern_len, path, path_len, NULL,
                            0, NULL, &match);
    if (rc != M3_OK) {
      return rc;
    }
    if (match == M3_TRUE) {
      *out_route = route;
      return M3_OK;
    }
  }

  return M3_ERR_NOT_FOUND;
}

static int m3_router_copy_path(M3Router *router, const char *path,
                               m3_usize path_len,
                               char **out_copy) { /* GCOVR_EXCL_LINE */
  m3_usize alloc_size;                            /* GCOVR_EXCL_LINE */
  void *mem;                                      /* GCOVR_EXCL_LINE */
  int rc;

  if (router == NULL || path == NULL || out_copy == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (path_len > m3_router_usize_max_value() - 1) {
    return M3_ERR_OVERFLOW;
  }

  alloc_size = path_len + 1;
  mem = NULL;
  rc = router->allocator.alloc(router->allocator.ctx, alloc_size, &mem);
  if (rc != M3_OK) {
    return rc;
  }
  if (mem == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  if (path_len > 0) {
    memcpy(mem, path, (size_t)path_len);
  }
  ((char *)mem)[path_len] = '\0';
  *out_copy = (char *)mem;
  return M3_OK;
}

static int m3_router_navigate_path_len(M3Router *router, const char *path,
                                       m3_usize path_len,
                                       void **out_component) {
  const M3Route *route;
  char *path_copy;
  void *component;
  int rc;

  if (router == NULL || path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (router->stack_size > 0 && router->stack != NULL) {
    const char *current_path;
    m3_usize current_len;

    current_path = router->stack[router->stack_size - 1].path;
    if (current_path != NULL) {
      rc = m3_router_cstrlen(current_path, &current_len);
      if (rc != M3_OK) {
        return rc;
      }
      if (current_len == path_len &&
          memcmp(current_path, path, (size_t)path_len) == 0) {
        if (out_component != NULL) {
          *out_component = router->stack[router->stack_size - 1].component;
        }
        return M3_OK;
      }
    }
  }

  if (router->stack_capacity == 0 || router->stack == NULL) {
    return M3_ERR_OVERFLOW;
  }
  if (router->stack_size >= router->stack_capacity) {
    return M3_ERR_OVERFLOW;
  }

  rc = m3_router_find_route(router, path, path_len, &route);
  if (rc != M3_OK) {
    return rc;
  }

  path_copy = NULL;
  rc = m3_router_copy_path(router, path, path_len, &path_copy);
  if (rc != M3_OK) {
    return rc;
  }

  component = NULL;
  rc = route->build(route->ctx, path_copy, &component);
  if (rc != M3_OK) {
    router->allocator.free(router->allocator.ctx, path_copy);
    return rc;
  }

  router->stack[router->stack_size].route = route;
  router->stack[router->stack_size].path = path_copy;
  router->stack[router->stack_size].component = component;
  router->stack_size += 1;

  if (out_component != NULL) {
    *out_component = component;
  }

  return M3_OK;
}

int M3_CALL m3_router_init(M3Router *router, const M3RouterConfig *config) {
  m3_usize i;
  m3_usize alloc_size;
  void *mem;
  int rc;

  if (router == NULL || config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (config->route_count > 0 && config->routes == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (config->stack_capacity > 0) {
    if (config->stack_capacity >
        m3_router_usize_max_value() / sizeof(M3RouteEntry)) {
      return M3_ERR_OVERFLOW;
    }
  }

  if (config->allocator != NULL) {
    router->allocator = *config->allocator;
  } else {
    rc = m3_get_default_allocator(&router->allocator);
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (config->routes != NULL) {
    for (i = 0; i < config->route_count; ++i) {
      if (config->routes[i].pattern == NULL ||
          config->routes[i].build == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
      }
      rc = m3_route_validate_pattern(config->routes[i].pattern);
      if (rc != M3_OK) {
        return rc;
      }
    }
  }

  router->routes = config->routes;
  router->route_count = config->route_count;
  router->stack_capacity = config->stack_capacity;
  router->stack_size = 0;
  router->stack = NULL;

  if (router->stack_capacity == 0) {
    return M3_OK;
  }

  alloc_size = router->stack_capacity * sizeof(M3RouteEntry);
  mem = NULL;
  rc = router->allocator.alloc(router->allocator.ctx, alloc_size, &mem);
  if (rc != M3_OK) {
    return rc;
  }
  if (mem == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }
  memset(mem, 0, (size_t)alloc_size);
  router->stack = (M3RouteEntry *)mem;
  return M3_OK;
}

int M3_CALL m3_router_shutdown(M3Router *router) {
  int rc;
  int free_rc;

  if (router == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_router_clear(router);

  if (router->stack != NULL) {
    free_rc = router->allocator.free(router->allocator.ctx, router->stack);
    if (free_rc != M3_OK) {
      if (rc == M3_OK) {
        rc = free_rc;
      }
      return rc;
    }
    router->stack = NULL;
    router->stack_capacity = 0;
  }

  router->stack_size = 0;
  router->routes = NULL;
  router->route_count = 0;
  return rc;
}

int M3_CALL m3_router_navigate(M3Router *router, const char *path,
                               void **out_component) {
  m3_usize path_len;
  int rc;

  if (router == NULL || path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_router_cstrlen(path, &path_len);
  if (rc != M3_OK) {
    return rc;
  }

  if (path_len == 0) {
    path = g_router_root_path;
    path_len = 1;
  }

  return m3_router_navigate_path_len(router, path, path_len, out_component);
}

int M3_CALL m3_router_navigate_uri(M3Router *router, const char *uri,
                                   void **out_component) {
  M3Uri parsed;
  const char *path;
  m3_usize path_len;
  int rc;

  if (router == NULL || uri == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_uri_parse(uri, &parsed);
  if (rc != M3_OK) {
    return rc;
  }

  if (parsed.path.length == 0) {
    path = g_router_root_path;
    path_len = 1;
  } else {
    path = parsed.path.data;
    path_len = parsed.path.length;
  }

  return m3_router_navigate_path_len(router, path, path_len, out_component);
}

int M3_CALL m3_router_can_back(const M3Router *router, M3Bool *out_can_back) {
  if (router == NULL || out_can_back == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_can_back = (router->stack_size > 1) ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_router_back(M3Router *router, void **out_component) {
  int rc;

  if (router == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (router->stack_size < 2) {
    return M3_ERR_NOT_FOUND;
  }

  rc = m3_router_release_entry(router, &router->stack[router->stack_size - 1]);
  router->stack_size -= 1;

  if (out_component != NULL) {
    *out_component = router->stack[router->stack_size - 1].component;
  }
  return rc;
}

int M3_CALL m3_router_get_current(const M3Router *router, const char **out_path,
                                  void **out_component) {
  const M3RouteEntry *entry;

  if (router == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (out_path == NULL && out_component == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (router->stack_size == 0) {
    return M3_ERR_NOT_FOUND;
  }

  entry = &router->stack[router->stack_size - 1];
  if (out_path != NULL) {
    *out_path = entry->path;
  }
  if (out_component != NULL) {
    *out_component = entry->component;
  }
  return M3_OK;
}

int M3_CALL m3_router_clear(M3Router *router) {
  int rc;
  int entry_rc;

  if (router == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = M3_OK;
  while (router->stack_size > 0) {
    entry_rc =
        m3_router_release_entry(router, &router->stack[router->stack_size - 1]);
    router->stack_size -= 1;
    if (rc == M3_OK && entry_rc != M3_OK) {
      rc = entry_rc;
    }
  }

  return rc;
}

#ifdef M3_TESTING
int M3_CALL m3_router_test_set_cstr_limit(m3_usize max_len) {
  g_router_cstr_limit_override = max_len;
  return M3_OK;
}

int M3_CALL m3_router_test_set_cstr_fail_after(m3_u32 call_index) {
  g_router_cstr_fail_after = call_index;
  g_router_cstr_call_count = 0u;
  return M3_OK;
}

int M3_CALL m3_router_test_set_slice_equals_fail(M3Bool enable) {
  g_router_force_slice_equals_fail = enable ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_route_test_match(const char *pattern, const char *path,
                                M3RouteParam *params, m3_usize max_params,
                                m3_usize *out_param_count, M3Bool *out_match) {
  m3_usize pattern_len;
  m3_usize path_len;
  int rc;

  if (pattern == NULL || path == NULL || out_match == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_router_cstrlen(pattern, &pattern_len);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_router_cstrlen(path, &path_len);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_route_match_len(pattern, pattern_len, path, path_len, params,
                            max_params, out_param_count, out_match);
}

int M3_CALL m3_router_test_cstrlen(const char *cstr, m3_usize *out_len) {
  return m3_router_cstrlen(cstr, out_len);
}

int M3_CALL m3_router_test_slice_set(M3UriSlice *slice, const char *data,
                                     m3_usize length) {
  m3_router_slice_set(slice, data, length);
  return M3_OK;
}

int M3_CALL m3_router_test_slice_equals(const M3UriSlice *slice,
                                        const char *key, m3_usize key_len,
                                        M3Bool *out_equal) {
  return m3_router_slice_equals(slice, key, key_len, out_equal);
}

int M3_CALL m3_router_test_trim_trailing(const char **str, m3_usize *len) {
  m3_route_trim_trailing(str, len);
  return M3_OK;
}

int M3_CALL m3_router_test_next_segment(const char *str, m3_usize len,
                                        m3_usize *index, const char **out_seg,
                                        m3_usize *out_len) {
  m3_route_next_segment(str, len, index, out_seg, out_len);
  return M3_OK;
}

int M3_CALL m3_router_test_has_more_segments(const char *str, m3_usize len,
                                             m3_usize index,
                                             M3Bool *out_has_more) {
  if (out_has_more == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_has_more = m3_route_has_more_segments(str, len, index);
  return M3_OK;
}

int M3_CALL m3_router_test_match_len(
    const char *pattern, m3_usize pattern_len, const char *path,
    m3_usize path_len, M3RouteParam *params, m3_usize max_params,
    m3_usize *out_param_count, /* GCOVR_EXCL_LINE */
    M3Bool *out_match) {
  return m3_route_match_len(pattern, pattern_len, path, path_len, params,
                            max_params, out_param_count, out_match);
}

int M3_CALL m3_router_test_validate_pattern(const char *pattern) {
  return m3_route_validate_pattern(pattern);
}

int M3_CALL m3_router_test_find_route(const M3Router *router, const char *path,
                                      m3_usize path_len,
                                      const M3Route **out_route) {
  return m3_router_find_route(router, path, path_len, out_route);
}

int M3_CALL m3_router_test_release_entry(M3Router *router,
                                         M3RouteEntry *entry) {
  return m3_router_release_entry(router, entry);
}

int M3_CALL m3_router_test_copy_path(M3Router *router, const char *path,
                                     m3_usize path_len, char **out_copy) {
  return m3_router_copy_path(router, path, path_len, out_copy);
}

int M3_CALL m3_router_test_navigate_path_len(M3Router *router, const char *path,
                                             m3_usize path_len,
                                             void **out_component) {
  return m3_router_navigate_path_len(router, path, path_len, out_component);
}
#endif
