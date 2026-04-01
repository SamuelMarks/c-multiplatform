/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

static void *dummy_route_handler(const char *route_params) {
  (void)route_params;
  return NULL;
}

TEST test_state_restoration_lifecycle(void) {
  cmp_state_restoration_ctx_t *ctx = NULL;
  cmp_router_t *router = NULL;
  void *blob = NULL;
  size_t size = 0;
  cmp_string_t result = {0};

  ASSERT_EQ(CMP_SUCCESS, cmp_state_restoration_ctx_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_create(&router));

  /* Setup a router */
  ASSERT_EQ(CMP_SUCCESS, cmp_router_register(router, "/app/item/123",
                                             dummy_route_handler, NULL, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_push(router, "/app/item/123"));

  /* Isolate Window Scene on iPadOS */
  ASSERT_EQ(CMP_SUCCESS, cmp_state_restoration_set_scene_id(ctx, "Scene_A"));

  /* Encode state */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_state_restoration_encode(ctx, router, &blob, &size));
  ASSERT_NEQ(NULL, blob);
  ASSERT_GT(size, 0);

  /* Reset router state as if app was killed */
  cmp_router_destroy(router);
  ASSERT_EQ(CMP_SUCCESS, cmp_router_create(&router));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_register(router, "/app/item/123",
                                             dummy_route_handler, NULL, NULL));

  /* Decode state from OS */
  ASSERT_EQ(CMP_SUCCESS, cmp_state_restoration_decode(ctx, router, blob, size));

  /* Verify deep link restoration worked */
  ASSERT_EQ(CMP_SUCCESS, cmp_router_get_current(router, &result));
  ASSERT_STR_EQ("/app/item/123", result.data);

  cmp_string_destroy(&result);
  if (blob)
    CMP_FREE(blob);

  ASSERT_EQ(CMP_SUCCESS, cmp_router_destroy(router));
  ASSERT_EQ(CMP_SUCCESS, cmp_state_restoration_ctx_destroy(ctx));
  PASS();
}

TEST test_universal_link_handler(void) {
  cmp_state_restoration_ctx_t *ctx = NULL;
  cmp_router_t *router = NULL;
  cmp_string_t result = {0};

  ASSERT_EQ(CMP_SUCCESS, cmp_state_restoration_ctx_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_create(&router));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_register(router, "/product/456",
                                             dummy_route_handler, NULL, NULL));

  /* Simulate NSUserActivity continuation from Safari universal link */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_deep_link_handle_universal_link(ctx, "/product/456", router));

  ASSERT_EQ(CMP_SUCCESS, cmp_router_get_current(router, &result));
  ASSERT_STR_EQ("/product/456", result.data);

  cmp_string_destroy(&result);
  ASSERT_EQ(CMP_SUCCESS, cmp_router_destroy(router));
  ASSERT_EQ(CMP_SUCCESS, cmp_state_restoration_ctx_destroy(ctx));
  PASS();
}

TEST test_null_args(void) {
  cmp_state_restoration_ctx_t *ctx = NULL;
  cmp_router_t *router = NULL;
  void *blob = NULL;
  size_t size = 0;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_state_restoration_ctx_create(NULL));
  cmp_state_restoration_ctx_create(&ctx);
  cmp_router_create(&router);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_deep_link_handle_universal_link(NULL, "/a", router));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_deep_link_handle_universal_link(ctx, NULL, router));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_deep_link_handle_universal_link(ctx, "/a", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_state_restoration_encode(NULL, router, &blob, &size));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_state_restoration_encode(ctx, NULL, &blob, &size));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_state_restoration_encode(ctx, router, NULL, &size));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_state_restoration_encode(ctx, router, &blob, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_state_restoration_decode(NULL, router, "a", 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_state_restoration_decode(ctx, NULL, "a", 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_state_restoration_decode(ctx, router, NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_state_restoration_decode(ctx, router, "a", 0));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_state_restoration_set_scene_id(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_state_restoration_set_scene_id(ctx, NULL));

  cmp_state_restoration_ctx_destroy(ctx);
  cmp_router_destroy(router);
  PASS();
}

SUITE(deep_link_suite) {
  RUN_TEST(test_state_restoration_lifecycle);
  RUN_TEST(test_universal_link_handler);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(deep_link_suite);
  GREATEST_MAIN_END();
}
