/* clang-format off */
#include "../include/ui_web_bridge.h"
#include "../include/ui_asset_streamer.h"
#include "../include/ui_promise.h"
#include "../include/ui_thread_pool.h"
#include "../include/ui_execution_context.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

int main(void) {
  ui_error_t err;

  printf("Testing Wasm error paths (bridge failures)...\n");

  /* Force OOM during command buffer allocation */
  g_malloc_fail_countdown = 0;
  err = ui_web_bridge_create_node(1, "div");
  if (err != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected UI_ERROR_OUT_OF_MEMORY for bridge create, got %d\n", err);
    return 1;
  }

  g_malloc_fail_countdown = -1; /* reset */

  printf("Testing OOM during asset streamer request...\n");
  {
    struct ui_thread_pool *pool = NULL;
    struct ui_execution_context *ctx = NULL;
    struct ui_asset_streamer *streamer = NULL;
    struct ui_promise *promise = NULL;

    ui_thread_pool_create(1, &pool);
    ui_execution_context_create(&ctx);
    ui_asset_streamer_create(pool, ctx, &streamer);

    g_malloc_fail_countdown = 1; /* allow promise create, fail on task alloc */
    err = ui_asset_streamer_request(streamer, "test.png", UI_ASSET_TYPE_BINARY,
                                    &promise);

    if (err != UI_ERROR_OUT_OF_MEMORY) {
      printf("Expected UI_ERROR_OUT_OF_MEMORY for asset request, got %d\n",
             err);
      return 1;
    }

    g_malloc_fail_countdown = -1;
    (void)ui_asset_streamer_destroy(streamer);
    (void)ui_execution_context_destroy(ctx);
    ui_thread_pool_destroy(pool);
  }

  printf("test_ui_wasm_error_paths passed\n");
  return 0;
}
