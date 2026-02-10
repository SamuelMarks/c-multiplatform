#ifndef M3_ROUTER_H
#define M3_ROUTER_H

/**
 * @file m3_router.h
 * @brief URI parsing and navigation router for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_core.h"

/**
 * @brief String slice descriptor.
 */
typedef struct M3UriSlice {
  const char *data; /**< Slice data pointer (may be NULL when length is 0). */
  m3_usize length;  /**< Slice length in bytes. */
} M3UriSlice;

/**
 * @brief Parsed URI components.
 */
typedef struct M3Uri {
  M3UriSlice scheme;    /**< Scheme without "://". */
  M3UriSlice authority; /**< Authority (host[:port]). */
  M3UriSlice path;      /**< Path component. */
  M3UriSlice query;     /**< Query string without '?'. */
  M3UriSlice fragment;  /**< Fragment without '#'. */
} M3Uri;

/**
 * @brief Parse a URI into components.
 * @param uri URI string (null-terminated).
 * @param out_uri Receives the parsed components.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_uri_parse(const char *uri, M3Uri *out_uri);

/**
 * @brief Find a query parameter value by key.
 * @param uri Parsed URI.
 * @param key Null-terminated key string.
 * @param out_value Receives the value slice (empty if key has no value).
 * @param out_found Receives M3_TRUE if the key is found.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_uri_query_find(const M3Uri *uri, const char *key,
                                     M3UriSlice *out_value, M3Bool *out_found);

/**
 * @brief Route parameter pair.
 */
typedef struct M3RouteParam {
  M3UriSlice key;   /**< Parameter name. */
  M3UriSlice value; /**< Parameter value. */
} M3RouteParam;

/**
 * @brief Route builder callback.
 * @param ctx Route context pointer.
 * @param path Matched path string.
 * @param out_component Receives the created component pointer.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3RouteBuildFn)(void *ctx, const char *path,
                                     void **out_component);

/**
 * @brief Route destroy callback.
 * @param ctx Route context pointer.
 * @param component Component pointer to destroy.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3RouteDestroyFn)(void *ctx, void *component);

/**
 * @brief Route descriptor.
 */
typedef struct M3Route {
  const char *pattern;      /**< Route pattern string. */
  M3RouteBuildFn build;     /**< Component build callback. */
  M3RouteDestroyFn destroy; /**< Component destroy callback (optional). */
  void *ctx;                /**< Route context pointer. */
} M3Route;

/**
 * @brief Route stack entry.
 */
typedef struct M3RouteEntry {
  const M3Route *route; /**< Matched route. */
  char *path;           /**< Owned path string. */
  void *component;      /**< Built component pointer. */
} M3RouteEntry;

/**
 * @brief Router configuration.
 */
typedef struct M3RouterConfig {
  const M3Allocator
      *allocator;        /**< Allocator to use; NULL uses default allocator. */
  const M3Route *routes; /**< Route table. */
  m3_usize route_count;  /**< Number of routes. */
  m3_usize stack_capacity; /**< Maximum navigation stack size. */
} M3RouterConfig;

/**
 * @brief Router instance.
 */
typedef struct M3Router {
  M3Allocator allocator;   /**< Allocator used by the router. */
  const M3Route *routes;   /**< Route table. */
  m3_usize route_count;    /**< Number of routes. */
  M3RouteEntry *stack;     /**< Navigation stack. */
  m3_usize stack_capacity; /**< Stack capacity. */
  m3_usize stack_size;     /**< Current stack size. */
} M3Router;

/**
 * @brief Initialize a router.
 * @param router Router instance.
 * @param config Router configuration.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_init(M3Router *router,
                                  const M3RouterConfig *config);

/**
 * @brief Shut down a router and release its resources.
 * @param router Router instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_shutdown(M3Router *router);

/**
 * @brief Navigate to a path.
 * @param router Router instance.
 * @param path Path string.
 * @param out_component Receives the created component pointer (optional).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_navigate(M3Router *router, const char *path,
                                      void **out_component);

/**
 * @brief Navigate to a URI.
 * @param router Router instance.
 * @param uri URI string.
 * @param out_component Receives the created component pointer (optional).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_navigate_uri(M3Router *router, const char *uri,
                                          void **out_component);

/**
 * @brief Check whether back navigation is possible.
 * @param router Router instance.
 * @param out_can_back Receives M3_TRUE if back is possible.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_can_back(const M3Router *router,
                                      M3Bool *out_can_back);

/**
 * @brief Navigate back in the stack.
 * @param router Router instance.
 * @param out_component Receives the new current component (optional).
 * @return M3_OK on success, M3_ERR_NOT_FOUND if no back entry, or a failure
 * code.
 */
M3_API int M3_CALL m3_router_back(M3Router *router, void **out_component);

/**
 * @brief Get the current route entry.
 * @param router Router instance.
 * @param out_path Receives the current path (optional).
 * @param out_component Receives the current component (optional).
 * @return M3_OK on success, M3_ERR_NOT_FOUND if stack is empty, or a failure
 * code.
 */
M3_API int M3_CALL m3_router_get_current(const M3Router *router,
                                         const char **out_path,
                                         void **out_component);

/**
 * @brief Clear the navigation stack.
 * @param router Router instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_clear(M3Router *router);

#ifdef M3_TESTING
/**
 * @brief Override the maximum C-string length for router helpers.
 * @param max_len Maximum allowed length (0 disables override).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_set_cstr_limit(m3_usize max_len);

/**
 * @brief Force router C-string length checks to fail on a given call count.
 * @param call_index Call index to fail on (0 disables failure).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_set_cstr_fail_after(m3_u32 call_index);

/**
 * @brief Test hook to force slice equality failure.
 * @param enable Whether to force failure.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_set_slice_equals_fail(M3Bool enable);

/**
 * @brief Test wrapper for route matching.
 * @param pattern Route pattern.
 * @param path Path string.
 * @param params Output parameter array (optional).
 * @param max_params Parameter array capacity.
 * @param out_param_count Receives number of captured params.
 * @param out_match Receives M3_TRUE if match succeeded.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_route_test_match(const char *pattern, const char *path,
                                       M3RouteParam *params,
                                       m3_usize max_params,
                                       m3_usize *out_param_count,
                                       M3Bool *out_match);

/**
 * @brief Test wrapper for router C-string length helper.
 * @param cstr Null-terminated string.
 * @param out_len Receives length.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_cstrlen(const char *cstr, m3_usize *out_len);

/**
 * @brief Test wrapper for router slice setter.
 * @param slice Slice to populate (may be NULL).
 * @param data Slice data pointer.
 * @param length Slice length.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_slice_set(M3UriSlice *slice, const char *data,
                                            m3_usize length);

/**
 * @brief Test wrapper for slice equality helper.
 * @param slice Slice to compare.
 * @param key Null-terminated key.
 * @param key_len Key length.
 * @param out_equal Receives comparison result.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_slice_equals(const M3UriSlice *slice,
                                               const char *key,
                                               m3_usize key_len,
                                               M3Bool *out_equal);

/**
 * @brief Test wrapper for trimming trailing slashes.
 * @param str Path pointer.
 * @param len Length pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_trim_trailing(const char **str,
                                                m3_usize *len);

/**
 * @brief Test wrapper for segment scanning.
 * @param str Path string.
 * @param len Length.
 * @param index In/out index.
 * @param out_seg Receives segment pointer.
 * @param out_len Receives segment length.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_next_segment(const char *str, m3_usize len,
                                               m3_usize *index,
                                               const char **out_seg,
                                               m3_usize *out_len);

/**
 * @brief Test wrapper for segment lookahead.
 * @param str Path string.
 * @param len Length.
 * @param index Start index.
 * @param out_has_more Receives M3_TRUE if more segments exist.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_has_more_segments(const char *str,
                                                    m3_usize len,
                                                    m3_usize index,
                                                    M3Bool *out_has_more);

/**
 * @brief Test wrapper for low-level route matching with explicit lengths.
 * @param pattern Pattern string.
 * @param pattern_len Pattern length.
 * @param path Path string.
 * @param path_len Path length.
 * @param params Output parameter array.
 * @param max_params Parameter array capacity.
 * @param out_param_count Receives parameter count.
 * @param out_match Receives match result.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_match_len(
    const char *pattern, m3_usize pattern_len, const char *path,
    m3_usize path_len, M3RouteParam *params, m3_usize max_params,
    m3_usize *out_param_count, M3Bool *out_match);

/**
 * @brief Test wrapper for validating route patterns.
 * @param pattern Pattern string.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_validate_pattern(const char *pattern);

/**
 * @brief Test wrapper for releasing a route entry.
 * @param router Router instance.
 * @param entry Entry to release.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_release_entry(M3Router *router,
                                                M3RouteEntry *entry);

/**
 * @brief Test wrapper for finding a route entry.
 * @param router Router instance.
 * @param path Path to match.
 * @param path_len Length of the path in bytes.
 * @param out_route Receives the matched route.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_find_route(const M3Router *router,
                                             const char *path,
                                             m3_usize path_len,
                                             const M3Route **out_route);

/**
 * @brief Test wrapper for copying a path.
 * @param router Router instance.
 * @param path Path string.
 * @param path_len Path length.
 * @param out_copy Receives allocated copy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_copy_path(M3Router *router, const char *path,
                                            m3_usize path_len, char **out_copy);

/**
 * @brief Test wrapper for navigating with explicit path length.
 * @param router Router instance.
 * @param path Path string.
 * @param path_len Path length.
 * @param out_component Receives component pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_router_test_navigate_path_len(M3Router *router,
                                                    const char *path,
                                                    m3_usize path_len,
                                                    void **out_component);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_ROUTER_H */
