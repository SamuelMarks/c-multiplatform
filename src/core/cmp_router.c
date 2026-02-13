#include "cmpc/cmp_router.h"

#include <string.h>

#ifdef CMP_TESTING
static cmp_usize g_router_cstr_limit_override = 0;
static CMPBool g_router_force_slice_equals_fail = CMP_FALSE;
static cmp_u32 g_router_cstr_fail_after = 0u;
static cmp_u32 g_router_cstr_call_count = 0u;
#endif

static const char g_router_root_path[] = "/";

static cmp_usize cmp_router_usize_max_value(void) {
  return (cmp_usize) ~(cmp_usize)0;
}

static cmp_usize cmp_router_cstr_limit(void) {
#ifdef CMP_TESTING
  if (g_router_cstr_limit_override != 0) {
    return g_router_cstr_limit_override;
  }
#endif
  return cmp_router_usize_max_value();
}

static int cmp_router_cstrlen(const char *cstr, cmp_usize *out_len) {
  cmp_usize max_len;
  cmp_usize len;

  if (cstr == NULL || out_len == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#ifdef CMP_TESTING
  if (g_router_cstr_fail_after != 0u) {
    g_router_cstr_call_count += 1u;
    if (g_router_cstr_call_count >= g_router_cstr_fail_after) {
      return CMP_ERR_OVERFLOW;
    }
  }
#endif

  max_len = cmp_router_cstr_limit();
  len = 0;

  while (cstr[len] != '\0') {
    if (len == max_len) {
      return CMP_ERR_OVERFLOW;
    }
    len += 1;
  }

  *out_len = len;
  return CMP_OK;
}

static void cmp_router_slice_set(CMPUriSlice *slice, const char *data,
                                 cmp_usize length) {
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

int CMP_CALL cmp_uri_parse(const char *uri, CMPUri *out_uri) {
  cmp_usize len;
  cmp_usize i;
  cmp_usize scheme_end;
  cmp_usize authority_start;
  cmp_usize authority_end;
  cmp_usize path_start;
  cmp_usize path_end;
  cmp_usize query_start;
  cmp_usize query_end;
  cmp_usize fragment_start;
  int rc;

  if (uri == NULL || out_uri == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_router_cstrlen(uri, &len);
  if (rc != CMP_OK) {
    return rc;
  }

  cmp_router_slice_set(&out_uri->scheme, NULL, 0);
  cmp_router_slice_set(&out_uri->authority, NULL, 0);
  cmp_router_slice_set(&out_uri->path, NULL, 0);
  cmp_router_slice_set(&out_uri->query, NULL, 0);
  cmp_router_slice_set(&out_uri->fragment, NULL, 0);

  if (len == 0) {
    return CMP_OK;
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
    cmp_router_slice_set(&out_uri->scheme, uri, scheme_end);
    for (i = authority_start; i < len; ++i) {
      if (uri[i] == '/' || uri[i] == '?' || uri[i] == '#') {
        authority_end = i;
        break;
      }
    }
    if (authority_end == 0) {
      authority_end = len;
    }
    cmp_router_slice_set(&out_uri->authority, uri + authority_start,
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
    cmp_router_slice_set(&out_uri->path, uri + path_start,
                         path_end - path_start);
  }
  if (query_start < len) {
    cmp_router_slice_set(&out_uri->query, uri + query_start,
                         query_end - query_start);
  }
  if (fragment_start < len) {
    cmp_router_slice_set(&out_uri->fragment, uri + fragment_start,
                         len - fragment_start);
  }

  return CMP_OK;
}

static int cmp_router_slice_equals(const CMPUriSlice *slice, const char *key,
                                   cmp_usize key_len, CMPBool *out_equal) {
  if (slice == NULL || key == NULL || out_equal == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_router_force_slice_equals_fail) {
    return CMP_ERR_UNKNOWN;
  }
#endif

  if (slice->length != key_len) {
    *out_equal = CMP_FALSE;
    return CMP_OK;
  }

  if (slice->length == 0) {
    *out_equal = CMP_TRUE;
    return CMP_OK;
  }

  if (memcmp(slice->data, key, (size_t)slice->length) == 0) {
    *out_equal = CMP_TRUE;
  } else {
    *out_equal = CMP_FALSE;
  }
  return CMP_OK;
}

int CMP_CALL cmp_uri_query_find(const CMPUri *uri, const char *key,
                                CMPUriSlice *out_value, CMPBool *out_found) {
  cmp_usize key_len;
  cmp_usize start;
  cmp_usize end;
  cmp_usize eq_pos;
  CMPUriSlice key_slice;
  CMPBool equal;
  int rc;

  if (uri == NULL || key == NULL || out_value == NULL || out_found == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_router_cstrlen(key, &key_len);
  if (rc != CMP_OK) {
    return rc;
  }
  if (key_len == 0) {
    return CMP_ERR_RANGE;
  }

  out_value->data = NULL;
  out_value->length = 0;
  *out_found = CMP_FALSE;

  if (uri->query.length == 0) {
    return CMP_OK;
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

    cmp_router_slice_set(&key_slice, uri->query.data + start, eq_pos - start);
    rc = cmp_router_slice_equals(&key_slice, key, key_len, &equal);
    if (rc != CMP_OK) {
      return rc;
    }
    if (equal == CMP_TRUE) {
      if (eq_pos < end && uri->query.data[eq_pos] == '=') {
        cmp_router_slice_set(out_value, uri->query.data + eq_pos + 1,
                             end - eq_pos - 1);
      } else {
        cmp_router_slice_set(out_value, NULL, 0);
      }
      *out_found = CMP_TRUE;
      return CMP_OK;
    }

    if (end >= uri->query.length) {
      break;
    }
    start = end + 1;
  }

  return CMP_OK;
}

static void cmp_route_trim_trailing(const char **str, cmp_usize *len) {
  cmp_usize length;

  if (str == NULL || len == NULL) {
    return;
  }

  length = *len;
  while (length > 1 && (*str)[length - 1] == '/') {
    length -= 1;
  }
  *len = length;
}

static void cmp_route_next_segment(const char *str, cmp_usize len,
                                   cmp_usize *index, const char **out_seg,
                                   cmp_usize *out_len) {
  cmp_usize i;
  cmp_usize start;

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

static CMPBool cmp_route_has_more_segments(const char *str, cmp_usize len,
                                           cmp_usize index) {
  cmp_usize i;

  if (str == NULL) {
    return CMP_FALSE;
  }

  i = index;
  while (i < len && str[i] == '/') {
    i += 1;
  }

  return (i < len) ? CMP_TRUE : CMP_FALSE;
}

static int cmp_route_match_len(const char *pattern, cmp_usize pattern_len,
                               const char *path, cmp_usize path_len,
                               CMPRouteParam *params, cmp_usize max_params,
                               cmp_usize *out_param_count, CMPBool *out_match) {
  const char *pat_str;
  const char *path_str;
  cmp_usize pat_len;
  cmp_usize p_idx;
  cmp_usize s_idx;
  cmp_usize param_count;

  if (pattern == NULL || path == NULL || out_match == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (params == NULL && out_param_count != NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  param_count = 0;
  *out_match = CMP_FALSE;
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

  cmp_route_trim_trailing(&pat_str, &pat_len);
  cmp_route_trim_trailing(&path_str, &path_len);

  p_idx = 0;
  s_idx = 0;

  for (;;) {
    const char *pat_seg;
    const char *path_seg;
    cmp_usize pat_seg_len;
    cmp_usize path_seg_len;
    CMPBool has_more;

    cmp_route_next_segment(pat_str, pat_len, &p_idx, &pat_seg, &pat_seg_len);
    cmp_route_next_segment(path_str, path_len, &s_idx, &path_seg,
                           &path_seg_len);

    if (pat_seg == NULL && path_seg == NULL) {
      *out_match = CMP_TRUE;
      if (out_param_count != NULL) {
        *out_param_count = param_count;
      }
      return CMP_OK;
    }
    if (pat_seg == NULL) {
      return CMP_OK;
    }
    if (path_seg == NULL) {
      if (pat_seg_len == 1 && pat_seg[0] == '*') {
        has_more = cmp_route_has_more_segments(pat_str, pat_len, p_idx);
        if (has_more == CMP_TRUE) {
          return CMP_ERR_INVALID_ARGUMENT;
        }
        *out_match = CMP_TRUE;
        if (out_param_count != NULL) {
          *out_param_count = param_count;
        }
      }
      return CMP_OK;
    }

    if (pat_seg_len == 1 && pat_seg[0] == '*') {
      has_more = cmp_route_has_more_segments(pat_str, pat_len, p_idx);
      if (has_more == CMP_TRUE) {
        return CMP_ERR_INVALID_ARGUMENT;
      }
      *out_match = CMP_TRUE;
      if (out_param_count != NULL) {
        *out_param_count = param_count;
      }
      return CMP_OK;
    }

    if (pat_seg_len > 0 && pat_seg[0] == ':') {
      if (pat_seg_len == 1) {
        return CMP_ERR_INVALID_ARGUMENT;
      }
      if (params != NULL) {
        if (param_count >= max_params) {
          return CMP_ERR_OVERFLOW;
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
      return CMP_OK;
    }
    if (memcmp(pat_seg, path_seg, (size_t)pat_seg_len) != 0) {
      return CMP_OK;
    }
  }
}

static int cmp_route_validate_pattern(const char *pattern) {
  cmp_usize len;
  const char *pat_str;
  cmp_usize pat_len;
  cmp_usize index;
  const char *seg;
  cmp_usize seg_len;
  CMPBool has_more;
  int rc;

  if (pattern == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_router_cstrlen(pattern, &len);
  if (rc != CMP_OK) {
    return rc;
  }

  if (len == 0) {
    return CMP_OK;
  }

  pat_str = pattern;
  pat_len = len;
  cmp_route_trim_trailing(&pat_str, &pat_len);

  index = 0;
  for (;;) {
    cmp_route_next_segment(pat_str, pat_len, &index, &seg, &seg_len);
    if (seg == NULL) {
      return CMP_OK;
    }
    if (seg_len == 1 && seg[0] == '*') {
      has_more = cmp_route_has_more_segments(pat_str, pat_len, index);
      if (has_more == CMP_TRUE) {
        return CMP_ERR_INVALID_ARGUMENT;
      }
      return CMP_OK;
    }
    if (seg_len > 0 && seg[0] == ':') {
      if (seg_len == 1) {
        return CMP_ERR_INVALID_ARGUMENT;
      }
    }
  }
}

static int cmp_router_release_entry(CMPRouter *router, CMPRouteEntry *entry) {
  int rc;
  int rc_next;

  if (router == NULL || entry == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = CMP_OK;

  if (entry->route != NULL && entry->route->destroy != NULL) {
    rc_next = entry->route->destroy(entry->route->ctx, entry->component);
    if (rc == CMP_OK) {
      rc = rc_next;
    }
  }

  if (entry->path != NULL) {
    rc_next = router->allocator.free(router->allocator.ctx, entry->path);
    if (rc == CMP_OK) {
      rc = rc_next;
    }
  }

  entry->route = NULL;
  entry->path = NULL;
  entry->component = NULL;
  return rc;
}

static int cmp_router_find_route(const CMPRouter *router, const char *path,
                                 cmp_usize path_len,
                                 const CMPRoute **out_route) {
  cmp_usize i;
  CMPBool match; /* GCOVR_EXCL_LINE */
  cmp_usize pattern_len;
  int rc;

  if (router == NULL || out_route == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (router->route_count == 0 || router->routes == NULL) {
    return CMP_ERR_NOT_FOUND;
  }

  for (i = 0; i < router->route_count; ++i) {
    const CMPRoute *route; /* GCOVR_EXCL_LINE */

    route = &router->routes[i];
    if (route->pattern == NULL || route->build == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
    rc = cmp_route_validate_pattern(route->pattern);
    if (rc != CMP_OK) {
      return rc;
    }

    rc = cmp_router_cstrlen(route->pattern, &pattern_len);
    if (rc != CMP_OK) {
      return rc;
    }

    match = CMP_FALSE;
    rc = cmp_route_match_len(route->pattern, pattern_len, path, path_len, NULL,
                             0, NULL, &match);
    if (rc != CMP_OK) {
      return rc;
    }
    if (match == CMP_TRUE) {
      *out_route = route;
      return CMP_OK;
    }
  }

  return CMP_ERR_NOT_FOUND;
}

static int cmp_router_copy_path(CMPRouter *router, const char *path,
                                cmp_usize path_len,
                                char **out_copy) { /* GCOVR_EXCL_LINE */
  cmp_usize alloc_size;                            /* GCOVR_EXCL_LINE */
  void *mem;                                       /* GCOVR_EXCL_LINE */
  int rc;

  if (router == NULL || path == NULL || out_copy == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (path_len > cmp_router_usize_max_value() - 1) {
    return CMP_ERR_OVERFLOW;
  }

  alloc_size = path_len + 1;
  mem = NULL;
  rc = router->allocator.alloc(router->allocator.ctx, alloc_size, &mem);
  if (rc != CMP_OK) {
    return rc;
  }
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  if (path_len > 0) {
    memcpy(mem, path, (size_t)path_len);
  }
  ((char *)mem)[path_len] = '\0';
  *out_copy = (char *)mem;
  return CMP_OK;
}

static int cmp_router_navigate_path_len(CMPRouter *router, const char *path,
                                        cmp_usize path_len,
                                        void **out_component) {
  const CMPRoute *route;
  char *path_copy;
  void *component;
  int rc;

  if (router == NULL || path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (router->stack_size > 0 && router->stack != NULL) {
    const char *current_path;
    cmp_usize current_len;

    current_path = router->stack[router->stack_size - 1].path;
    if (current_path != NULL) {
      rc = cmp_router_cstrlen(current_path, &current_len);
      if (rc != CMP_OK) {
        return rc;
      }
      if (current_len == path_len &&
          memcmp(current_path, path, (size_t)path_len) == 0) {
        if (out_component != NULL) {
          *out_component = router->stack[router->stack_size - 1].component;
        }
        return CMP_OK;
      }
    }
  }

  if (router->stack_capacity == 0 || router->stack == NULL) {
    return CMP_ERR_OVERFLOW;
  }
  if (router->stack_size >= router->stack_capacity) {
    return CMP_ERR_OVERFLOW;
  }

  rc = cmp_router_find_route(router, path, path_len, &route);
  if (rc != CMP_OK) {
    return rc;
  }

  path_copy = NULL;
  rc = cmp_router_copy_path(router, path, path_len, &path_copy);
  if (rc != CMP_OK) {
    return rc;
  }

  component = NULL;
  rc = route->build(route->ctx, path_copy, &component);
  if (rc != CMP_OK) {
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

  return CMP_OK;
}

int CMP_CALL cmp_router_init(CMPRouter *router, const CMPRouterConfig *config) {
  cmp_usize i;
  cmp_usize alloc_size;
  void *mem;
  int rc;

  if (router == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (config->route_count > 0 && config->routes == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (config->stack_capacity > 0) {
    if (config->stack_capacity >
        cmp_router_usize_max_value() / sizeof(CMPRouteEntry)) {
      return CMP_ERR_OVERFLOW;
    }
  }

  if (config->allocator != NULL) {
    router->allocator = *config->allocator;
  } else {
    rc = cmp_get_default_allocator(&router->allocator);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  if (config->routes != NULL) {
    for (i = 0; i < config->route_count; ++i) {
      if (config->routes[i].pattern == NULL ||
          config->routes[i].build == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
      }
      rc = cmp_route_validate_pattern(config->routes[i].pattern);
      if (rc != CMP_OK) {
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
    return CMP_OK;
  }

  alloc_size = router->stack_capacity * sizeof(CMPRouteEntry);
  mem = NULL;
  rc = router->allocator.alloc(router->allocator.ctx, alloc_size, &mem);
  if (rc != CMP_OK) {
    return rc;
  }
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  memset(mem, 0, (size_t)alloc_size);
  router->stack = (CMPRouteEntry *)mem;
  return CMP_OK;
}

int CMP_CALL cmp_router_shutdown(CMPRouter *router) {
  int rc;
  int free_rc;

  if (router == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_router_clear(router);

  if (router->stack != NULL) {
    free_rc = router->allocator.free(router->allocator.ctx, router->stack);
    if (free_rc != CMP_OK) {
      if (rc == CMP_OK) {
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

int CMP_CALL cmp_router_navigate(CMPRouter *router, const char *path,
                                 void **out_component) {
  cmp_usize path_len;
  int rc;

  if (router == NULL || path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_router_cstrlen(path, &path_len);
  if (rc != CMP_OK) {
    return rc;
  }

  if (path_len == 0) {
    path = g_router_root_path;
    path_len = 1;
  }

  return cmp_router_navigate_path_len(router, path, path_len, out_component);
}

int CMP_CALL cmp_router_navigate_uri(CMPRouter *router, const char *uri,
                                     void **out_component) {
  CMPUri parsed;
  const char *path;
  cmp_usize path_len;
  int rc;

  if (router == NULL || uri == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_uri_parse(uri, &parsed);
  if (rc != CMP_OK) {
    return rc;
  }

  if (parsed.path.length == 0) {
    path = g_router_root_path;
    path_len = 1;
  } else {
    path = parsed.path.data;
    path_len = parsed.path.length;
  }

  return cmp_router_navigate_path_len(router, path, path_len, out_component);
}

int CMP_CALL cmp_router_can_back(const CMPRouter *router,
                                 CMPBool *out_can_back) {
  if (router == NULL || out_can_back == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_can_back = (router->stack_size > 1) ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_router_back(CMPRouter *router, void **out_component) {
  int rc;

  if (router == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (router->stack_size < 2) {
    return CMP_ERR_NOT_FOUND;
  }

  rc = cmp_router_release_entry(router, &router->stack[router->stack_size - 1]);
  router->stack_size -= 1;

  if (out_component != NULL) {
    *out_component = router->stack[router->stack_size - 1].component;
  }
  return rc;
}

int CMP_CALL cmp_router_get_current(const CMPRouter *router,
                                    const char **out_path,
                                    void **out_component) {
  const CMPRouteEntry *entry;

  if (router == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (out_path == NULL && out_component == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (router->stack_size == 0) {
    return CMP_ERR_NOT_FOUND;
  }

  entry = &router->stack[router->stack_size - 1];
  if (out_path != NULL) {
    *out_path = entry->path;
  }
  if (out_component != NULL) {
    *out_component = entry->component;
  }
  return CMP_OK;
}

int CMP_CALL cmp_router_clear(CMPRouter *router) {
  int rc;
  int entry_rc;

  if (router == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = CMP_OK;
  while (router->stack_size > 0) {
    entry_rc = cmp_router_release_entry(router,
                                        &router->stack[router->stack_size - 1]);
    router->stack_size -= 1;
    if (rc == CMP_OK && entry_rc != CMP_OK) {
      rc = entry_rc;
    }
  }

  return rc;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_router_test_set_cstr_limit(cmp_usize max_len) {
  g_router_cstr_limit_override = max_len;
  return CMP_OK;
}

int CMP_CALL cmp_router_test_set_cstr_fail_after(cmp_u32 call_index) {
  g_router_cstr_fail_after = call_index;
  g_router_cstr_call_count = 0u;
  return CMP_OK;
}

int CMP_CALL cmp_router_test_set_slice_equals_fail(CMPBool enable) {
  g_router_force_slice_equals_fail = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_route_test_match(const char *pattern, const char *path,
                                  CMPRouteParam *params, cmp_usize max_params,
                                  cmp_usize *out_param_count,
                                  CMPBool *out_match) {
  cmp_usize pattern_len;
  cmp_usize path_len;
  int rc;

  if (pattern == NULL || path == NULL || out_match == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_router_cstrlen(pattern, &pattern_len);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_router_cstrlen(path, &path_len);
  if (rc != CMP_OK) {
    return rc;
  }

  return cmp_route_match_len(pattern, pattern_len, path, path_len, params,
                             max_params, out_param_count, out_match);
}

int CMP_CALL cmp_router_test_cstrlen(const char *cstr, cmp_usize *out_len) {
  return cmp_router_cstrlen(cstr, out_len);
}

int CMP_CALL cmp_router_test_slice_set(CMPUriSlice *slice, const char *data,
                                       cmp_usize length) {
  cmp_router_slice_set(slice, data, length);
  return CMP_OK;
}

int CMP_CALL cmp_router_test_slice_equals(const CMPUriSlice *slice,
                                          const char *key, cmp_usize key_len,
                                          CMPBool *out_equal) {
  return cmp_router_slice_equals(slice, key, key_len, out_equal);
}

int CMP_CALL cmp_router_test_trim_trailing(const char **str, cmp_usize *len) {
  cmp_route_trim_trailing(str, len);
  return CMP_OK;
}

int CMP_CALL cmp_router_test_next_segment(const char *str, cmp_usize len,
                                          cmp_usize *index,
                                          const char **out_seg,
                                          cmp_usize *out_len) {
  cmp_route_next_segment(str, len, index, out_seg, out_len);
  return CMP_OK;
}

int CMP_CALL cmp_router_test_has_more_segments(const char *str, cmp_usize len,
                                               cmp_usize index,
                                               CMPBool *out_has_more) {
  if (out_has_more == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_has_more = cmp_route_has_more_segments(str, len, index);
  return CMP_OK;
}

int CMP_CALL cmp_router_test_match_len(
    const char *pattern, cmp_usize pattern_len, const char *path,
    cmp_usize path_len, CMPRouteParam *params, cmp_usize max_params,
    cmp_usize *out_param_count, /* GCOVR_EXCL_LINE */
    CMPBool *out_match) {
  return cmp_route_match_len(pattern, pattern_len, path, path_len, params,
                             max_params, out_param_count, out_match);
}

int CMP_CALL cmp_router_test_validate_pattern(const char *pattern) {
  return cmp_route_validate_pattern(pattern);
}

int CMP_CALL cmp_router_test_find_route(const CMPRouter *router,
                                        const char *path, cmp_usize path_len,
                                        const CMPRoute **out_route) {
  return cmp_router_find_route(router, path, path_len, out_route);
}

int CMP_CALL cmp_router_test_release_entry(CMPRouter *router,
                                           CMPRouteEntry *entry) {
  return cmp_router_release_entry(router, entry);
}

int CMP_CALL cmp_router_test_copy_path(CMPRouter *router, const char *path,
                                       cmp_usize path_len, char **out_copy) {
  return cmp_router_copy_path(router, path, path_len, out_copy);
}

int CMP_CALL cmp_router_test_navigate_path_len(CMPRouter *router,
                                               const char *path,
                                               cmp_usize path_len,
                                               void **out_component) {
  return cmp_router_navigate_path_len(router, path, path_len, out_component);
}
#endif
