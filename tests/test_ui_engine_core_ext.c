/* clang-format off */
#include <stdio.h>
#include "../include/ui_engine.h"
#include "../include/ui_error.h"
#include "../src/ui_internal_mem.h"

/* The engine struct inside ui_engine_core.c */
struct ui_engine {
  struct ui_tick_engine *tick_engine;
  struct ui_thread_pool *thread_pool;
  struct ui_reactor *reactor;
  struct ui_timer *timer;
};

int main() {
    struct ui_engine engine;
    engine.tick_engine = NULL;
    engine.thread_pool = NULL;
    engine.reactor = NULL;
    engine.timer = NULL;
    /* If ui_reactor_poll receives NULL reactor, it returns UI_ERROR_INVALID_ARGUMENT. */
    if (ui_engine_tick(&engine) != UI_ERROR_INVALID_ARGUMENT) return 1;

    return 0;
}
