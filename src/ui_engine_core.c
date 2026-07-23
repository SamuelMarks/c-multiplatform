/* clang-format off */
#include <stddef.h>
#include "../include/ui_engine.h"
#include "../include/ui_execution_context.h"
#include "../include/ui_thread_pool.h"
#include "../include/ui_tick_engine.h"
#include "../include/ui_reactor.h"
#include "../include/ui_timer.h"
#if defined(__EMSCRIPTEN__)
#include "../include/ui_web_bridge.h"
#endif
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_engine {
  struct ui_tick_engine *tick_engine;
  struct ui_thread_pool *thread_pool;
  struct ui_reactor *reactor;
  struct ui_timer *timer;
};

enum ui_error ui_engine_create(const struct ui_engine_config *config,
                               struct ui_engine **out_engine) {
  enum ui_error rc = UI_ERROR_NONE;
  struct ui_engine *engine = NULL;

  if (!config || !out_engine) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  engine = (struct ui_engine *)UI_MALLOC(sizeof(struct ui_engine));
  if (!engine) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  engine->tick_engine = NULL;
  engine->thread_pool = NULL;
  engine->reactor = NULL;
  engine->timer = NULL;

  rc = ui_tick_engine_create(&engine->tick_engine);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

#ifndef UI_SINGLE_THREADED
  rc = ui_thread_pool_create(config->num_threads > 0 ? config->num_threads : 1,
                             &engine->thread_pool);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
#else
  engine->thread_pool = NULL; /* No thread pool in single threaded mode */
#endif

  rc = ui_reactor_create(&engine->reactor);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_timer_create_monotonic(&engine->timer);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  *out_engine = engine;
  engine = NULL;

cleanup:
  if (engine) {

    if (engine->reactor) {
      ui_reactor_destroy(engine->reactor);
    }
    if (engine->thread_pool) {
#ifndef UI_SINGLE_THREADED
      ui_thread_pool_destroy(engine->thread_pool);
#endif
    }
    if (engine->tick_engine) {
      ui_tick_engine_destroy(engine->tick_engine);
    }
    UI_FREE(engine);
  }
  return rc;
}

enum ui_error ui_engine_destroy(struct ui_engine *engine) {
  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (engine->timer) {
    ui_timer_destroy(engine->timer);
  }
  if (engine->reactor) {
    ui_reactor_destroy(engine->reactor);
  }
#ifndef UI_SINGLE_THREADED
  if (engine->thread_pool) {
    ui_thread_pool_destroy(engine->thread_pool);
  }
#endif
  if (engine->tick_engine) {
    ui_tick_engine_destroy(engine->tick_engine);
  }

  UI_FREE(engine);
  return UI_ERROR_NONE;
}

enum ui_error ui_engine_tick(struct ui_engine *engine) {
  enum ui_error rc = UI_ERROR_NONE;

  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_reactor_poll(engine->reactor, 0); /* Non-blocking poll */
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_tick_engine_tick(engine->tick_engine);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

#if defined(__EMSCRIPTEN__)
  rc = ui_web_bridge_flush();
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
#endif

  return UI_ERROR_NONE;
}
