#ifndef CMP_ROUTER_H
#define CMP_ROUTER_H

/**
 * @file cmp_router.h
 * @brief URI parsing and navigation router for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"

/**
 * @brief String slice descriptor.
 */
typedef struct CMPUriSlice {
  const char *data; /**< Slice data pointer (may be NULL when length is 0). */
  cmp_usize length;  /**< Slice length in bytes. */
} CMPUriSlice;

/**
 * @brief Parsed URI components.
 */
typedef struct CMPUri {
  CMPUriSlice scheme;    /**< Scheme without "://". */
  CMPUriSlice authority; /**< Authority (host[:port]). */
  CMPUriSlice path;      /**< Path component. */
  CMPUriSlice query;     /**< Query string without '?'. */
  CMPUriSlice fragment;  /**< Fragment without '#'. */
} CMPUri;

/**
 * @brief Parse a URI into components.
 * @param uri URI string (null-terminated).
 * @param out_uri Receives the parsed components.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_uri_parse(const char *uri, CMPUri *out_uri);

/**
 * @brief Find a query parameter value by key.
 * @param uri Parsed URI.
 * @param key Null-terminated key string.
 * @param out_value Receives the value slice (empty if key has no value).
 * @param out_found Receives CMP_TRUE if the key is found.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_uri_query_find(const CMPUri *uri, const char *key,
                                     CMPUriSlice *out_value, CMPBool *out_found);

/**
 * @brief Route parameter pair.
 */
typedef struct CMPRouteParam {
  CMPUriSlice key;   /**< Parameter name. */
  CMPUriSlice value; /**< Parameter value. */
} CMPRouteParam;

/**
 * @brief Route builder callback.
 * @param ctx Route context pointer.
 * @param path Matched path string.
 * @param out_component Receives the created component pointer.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPRouteBuildFn)(void *ctx, const char *path,
                                     void **out_component);

/**
 * @brief Route destroy callback.
 * @param ctx Route context pointer.
 * @param component Component pointer to destroy.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPRouteDestroyFn)(void *ctx, void *component);

/**
 * @brief Route descriptor.
 */
typedef struct CMPRoute {
  const char *pattern;      /**< Route pattern string. */
  CMPRouteBuildFn build;     /**< Component build callback. */
  CMPRouteDestroyFn destroy; /**< Component destroy callback (optional). */
  void *ctx;                /**< Route context pointer. */
} CMPRoute;

/**
 * @brief Route stack entry.
 */
typedef struct CMPRouteEntry {
  const CMPRoute *route; /**< Matched route. */
  char *path;           /**< Owned path string. */
  void *component;      /**< Built component pointer. */
} CMPRouteEntry;

/**
 * @brief Router configuration.
 */
typedef struct CMPRouterConfig {
  const CMPAllocator
      *allocator;        /**< Allocator to use; NULL uses default allocator. */
  const CMPRoute *routes; /**< Route table. */
  cmp_usize route_count;  /**< Number of routes. */
  cmp_usize stack_capacity; /**< Maximum navigation stack size. */
} CMPRouterConfig;

/**
 * @brief Router instance.
 */
typedef struct CMPRouter {
  CMPAllocator allocator;   /**< Allocator used by the router. */
  const CMPRoute *routes;   /**< Route table. */
  cmp_usize route_count;    /**< Number of routes. */
  CMPRouteEntry *stack;     /**< Navigation stack. */
  cmp_usize stack_capacity; /**< Stack capacity. */
  cmp_usize stack_size;     /**< Current stack size. */
} CMPRouter;

/**
 * @brief Initialize a router.
 * @param router Router instance.
 * @param config Router configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_init(CMPRouter *router,
                                  const CMPRouterConfig *config);

/**
 * @brief Shut down a router and release its resources.
 * @param router Router instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_shutdown(CMPRouter *router);

/**
 * @brief Navigate to a path.
 * @param router Router instance.
 * @param path Path string.
 * @param out_component Receives the created component pointer (optional).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_navigate(CMPRouter *router, const char *path,
                                      void **out_component);

/**
 * @brief Navigate to a URI.
 * @param router Router instance.
 * @param uri URI string.
 * @param out_component Receives the created component pointer (optional).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_navigate_uri(CMPRouter *router, const char *uri,
                                          void **out_component);

/**
 * @brief Check whether back navigation is possible.
 * @param router Router instance.
 * @param out_can_back Receives CMP_TRUE if back is possible.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_can_back(const CMPRouter *router,
                                      CMPBool *out_can_back);

/**
 * @brief Navigate back in the stack.
 * @param router Router instance.
 * @param out_component Receives the new current component (optional).
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND if no back entry, or a failure
 * code.
 */
CMP_API int CMP_CALL cmp_router_back(CMPRouter *router, void **out_component);

/**
 * @brief Get the current route entry.
 * @param router Router instance.
 * @param out_path Receives the current path (optional).
 * @param out_component Receives the current component (optional).
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND if stack is empty, or a failure
 * code.
 */
CMP_API int CMP_CALL cmp_router_get_current(const CMPRouter *router,
                                         const char **out_path,
                                         void **out_component);

/**
 * @brief Clear the navigation stack.
 * @param router Router instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_clear(CMPRouter *router);

#ifdef CMP_TESTING
/**
 * @brief Override the maximum C-string length for router helpers.
 * @param max_len Maximum allowed length (0 disables override).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_set_cstr_limit(cmp_usize max_len);

/**
 * @brief Force router C-string length checks to fail on a given call count.
 * @param call_index Call index to fail on (0 disables failure).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_set_cstr_fail_after(cmp_u32 call_index);

/**
 * @brief Test hook to force slice equality failure.
 * @param enable Whether to force failure.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_set_slice_equals_fail(CMPBool enable);

/**
 * @brief Test wrapper for route matching.
 * @param pattern Route pattern.
 * @param path Path string.
 * @param params Output parameter array (optional).
 * @param max_params Parameter array capacity.
 * @param out_param_count Receives number of captured params.
 * @param out_match Receives CMP_TRUE if match succeeded.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_route_test_match(const char *pattern, const char *path,
                                       CMPRouteParam *params,
                                       cmp_usize max_params,
                                       cmp_usize *out_param_count,
                                       CMPBool *out_match);

/**
 * @brief Test wrapper for router C-string length helper.
 * @param cstr Null-terminated string.
 * @param out_len Receives length.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_cstrlen(const char *cstr, cmp_usize *out_len);

/**
 * @brief Test wrapper for router slice setter.
 * @param slice Slice to populate (may be NULL).
 * @param data Slice data pointer.
 * @param length Slice length.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_slice_set(CMPUriSlice *slice, const char *data,
                                            cmp_usize length);

/**
 * @brief Test wrapper for slice equality helper.
 * @param slice Slice to compare.
 * @param key Null-terminated key.
 * @param key_len Key length.
 * @param out_equal Receives comparison result.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_slice_equals(const CMPUriSlice *slice,
                                               const char *key,
                                               cmp_usize key_len,
                                               CMPBool *out_equal);

/**
 * @brief Test wrapper for trimming trailing slashes.
 * @param str Path pointer.
 * @param len Length pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_trim_trailing(const char **str,
                                                cmp_usize *len);

/**
 * @brief Test wrapper for segment scanning.
 * @param str Path string.
 * @param len Length.
 * @param index In/out index.
 * @param out_seg Receives segment pointer.
 * @param out_len Receives segment length.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_next_segment(const char *str, cmp_usize len,
                                               cmp_usize *index,
                                               const char **out_seg,
                                               cmp_usize *out_len);

/**
 * @brief Test wrapper for segment lookahead.
 * @param str Path string.
 * @param len Length.
 * @param index Start index.
 * @param out_has_more Receives CMP_TRUE if more segments exist.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_has_more_segments(const char *str,
                                                    cmp_usize len,
                                                    cmp_usize index,
                                                    CMPBool *out_has_more);

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
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_match_len(
    const char *pattern, cmp_usize pattern_len, const char *path,
    cmp_usize path_len, CMPRouteParam *params, cmp_usize max_params,
    cmp_usize *out_param_count, CMPBool *out_match);

/**
 * @brief Test wrapper for validating route patterns.
 * @param pattern Pattern string.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_validate_pattern(const char *pattern);

/**
 * @brief Test wrapper for releasing a route entry.
 * @param router Router instance.
 * @param entry Entry to release.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_release_entry(CMPRouter *router,
                                                CMPRouteEntry *entry);

/**
 * @brief Test wrapper for finding a route entry.
 * @param router Router instance.
 * @param path Path to match.
 * @param path_len Length of the path in bytes.
 * @param out_route Receives the matched route.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_find_route(const CMPRouter *router,
                                             const char *path,
                                             cmp_usize path_len,
                                             const CMPRoute **out_route);

/**
 * @brief Test wrapper for copying a path.
 * @param router Router instance.
 * @param path Path string.
 * @param path_len Path length.
 * @param out_copy Receives allocated copy.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_copy_path(CMPRouter *router, const char *path,
                                            cmp_usize path_len, char **out_copy);

/**
 * @brief Test wrapper for navigating with explicit path length.
 * @param router Router instance.
 * @param path Path string.
 * @param path_len Path length.
 * @param out_component Receives component pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_test_navigate_path_len(CMPRouter *router,
                                                    const char *path,
                                                    cmp_usize path_len,
                                                    void **out_component);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_ROUTER_H */
