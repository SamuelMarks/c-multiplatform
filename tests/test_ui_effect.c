/* clang-format off */
#include "../include/ui_effect.h"
#include "../include/ui_error.h"
#include "../include/ui_arena.h"
#include "../include/ui_reactor.h"
#include <stdio.h>

/* Pre-include headers so include guards trigger */
#include "../src/ui_reactive_graph.h"

int g_mock_graph_set_fail = 0;
static ui_error_t mock_graph_set(struct ui_reactive_node *n, struct ui_reactive_node **p) {
  if (g_mock_graph_set_fail == 1 && p != NULL) return UI_ERROR_INVALID_ARGUMENT;
  if (g_mock_graph_set_fail == 2 && p == NULL) return UI_ERROR_INVALID_ARGUMENT;
  return ui_reactive_graph_set_current_node(n, p);
}
#define ui_reactive_graph_set_current_node mock_graph_set

#include "../src/ui_effect.c"

/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t failing_effect(void *user_data) {
  return UI_ERROR_INVALID_ARGUMENT;
}
static ui_error_t dummy_effect(void *user_data) {
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

    /* Test fallback when schedule fails */
    g_malloc_fail_countdown = 0;
    ui_effect_on_notify(eff2);
    g_malloc_fail_countdown = -1;

    g_malloc_fail_countdown = 0;
    ui_effect_create(NULL, dummy_effect, &data2, reactor, &eff2);
    g_malloc_fail_countdown = -1;
    ui_reactor_poll(reactor, 0);
    (void)ui_effect_destroy(eff2);
  }

  (void)ui_effect_destroy(eff);
  ui_reactor_destroy(reactor);

  /* Test effect fn failure */
  {
    ui_effect_t *eff_fail = NULL;
    if (ui_effect_create(NULL, failing_effect, NULL, NULL, &eff_fail) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }

  /* Test mock graph set fail on enter */
  {
    ui_effect_t *eff_fail2 = NULL;
    g_mock_graph_set_fail = 1;
    if (ui_effect_create(NULL, dummy_effect, &my_data, NULL, &eff_fail2) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    g_mock_graph_set_fail = 0;
  }

  /* Test mock graph set fail on exit */
  {
    ui_effect_t *eff_fail3 = NULL;
    g_mock_graph_set_fail = 2;
    if (ui_effect_create(NULL, dummy_effect, &my_data, NULL, &eff_fail3) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    g_mock_graph_set_fail = 0;
  }

  /* Null tests */
  ui_effect_create(NULL, NULL, NULL, NULL, NULL);
  ui_effect_create(NULL, dummy_effect, NULL, NULL, NULL);
  ui_effect_create(NULL, NULL, NULL, NULL, &eff);
  (void)ui_effect_destroy(NULL);
  ui_effect_evaluate(NULL);

  {
    ui_effect_t mock_eff = {0};
    ui_effect_evaluate(&mock_eff);
  }

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
    (void)ui_effect_destroy(eff);

    /* Exhaust the arena */
    ui_arena_alloc(arena, 256, 8, &dummy);

    g_malloc_fail_countdown = 0;
    if (ui_effect_create(arena, dummy_effect, &my_data, NULL, &eff) !=
        UI_ERROR_OUT_OF_MEMORY) {
      printf("Failed to get OOM for arena effect\n");
      return 1;
    }
    g_malloc_fail_countdown = -1;
    (void)ui_arena_destroy(arena);
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
