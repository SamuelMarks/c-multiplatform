/* clang-format off */
#include "../include/ui_effect.h"
#include "../include/ui_error.h"
#include "../include/ui_arena.h"
#include "../include/ui_reactor.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static enum ui_error dummy_effect(void *user_data) {
  int *cnt = (int *)user_data;
  (*cnt)++;
  return UI_ERROR_NONE;
}

static int test_effect(void) {
  ui_effect_t *eff = NULL;
  struct ui_reactor *reactor = NULL;
  int my_data = 0;

  if (ui_effect_create(NULL, dummy_effect, &my_data, NULL, &eff) !=
      UI_ERROR_NONE)
    return 1;
  /* effect_fn is called once on creation */
  if (my_data != 1)
    return 1;

  ui_reactor_create(&reactor);
  {
    ui_effect_t *eff2 = NULL;
    int data2 = 0;
    ui_effect_create(NULL, dummy_effect, &data2, reactor, &eff2);
    /* Should have scheduled to reactor instead of running directly if reactor
       logic allows it. Actually ui_reactor_schedule pushes it to reactor queue.
       It will run on reactor dispatch. */
    ui_reactor_poll(reactor, 0);
    g_malloc_fail_countdown = 0;
    ui_effect_create(NULL, dummy_effect, &data2, reactor, &eff2);
    g_malloc_fail_countdown = -1;
    ui_reactor_poll(reactor, 0);
    ui_effect_destroy(eff2);
  }

  ui_effect_destroy(eff);
  ui_reactor_destroy(reactor);

  /* Null tests */
  ui_effect_create(NULL, NULL, NULL, NULL, NULL);
  ui_effect_create(NULL, dummy_effect, NULL, NULL, NULL);
  ui_effect_destroy(NULL);

  /* Malloc fails */
  g_malloc_fail_countdown = 0;
  if (ui_effect_create(NULL, dummy_effect, &my_data, NULL, &eff) !=
      UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  /* Arena */
  {
    struct ui_arena *arena;
    void *dummy;
    ui_arena_create(256, &arena);

    ui_effect_create(arena, dummy_effect, &my_data, NULL, &eff);
    ui_effect_destroy(eff);

    /* Exhaust the arena */
    ui_arena_alloc(arena, 256, 8, &dummy);

    g_malloc_fail_countdown = 0;
    if (ui_effect_create(arena, dummy_effect, &my_data, NULL, &eff) !=
        UI_ERROR_OUT_OF_MEMORY) {
      printf("Failed to get OOM for arena effect\n");
      return 1;
    }
    g_malloc_fail_countdown = -1;
    ui_arena_destroy(arena);
  }

  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_effect();
  if (failed)
    return 1;
  return 0;
}
