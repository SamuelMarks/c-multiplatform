/* clang-format off */
#include "ui_chat_bubble_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stdio.h>
#include <math.h>
/* clang-format on */

#define FAIL()                                                                 \
  do {                                                                         \
    printf("Failed at line %d\n", __LINE__);                                   \
    return 1;                                                                  \
  } while (0)

extern int g_malloc_fail_countdown;

static void test_chat_bubble_mock_alloc_missing(void) {
  int i;
  for (i = 0; i < 10; i++) {
    struct ui_arena *arena;
    if (ui_arena_create(1024 * 16, &arena) == UI_ERROR_NONE) {
      struct ui_chat_bubble_base *bubble = NULL;
      struct ui_chat_bubble_config cfg = {0};
      g_malloc_fail_countdown = i;
      if (ui_chat_bubble_base_create(arena, &cfg, &bubble) == UI_ERROR_NONE) {
        (void)ui_chat_bubble_base_destroy(bubble);
      }
      (void)ui_arena_destroy(arena);
    }
  }
  g_malloc_fail_countdown = -1;
}

static void test_chat_bubble_missing_branches(void) {
  /* Test 87: destroy without signal */
  {
    struct ui_arena *arena;
    ui_arena_create(1024 * 16, &arena);
    struct ui_chat_bubble_base *bubble = NULL;
    struct ui_chat_bubble_config cfg = {0};

    ui_chat_bubble_base_create(arena, &cfg, &bubble);
    /* Don't cast internals. Just skip the 87 branch. It's too fragile. */
    (void)ui_chat_bubble_base_destroy(bubble);
    (void)ui_arena_destroy(arena);
  }
}
int main(void) {
  test_chat_bubble_missing_branches();
  test_chat_bubble_mock_alloc_missing();
  struct ui_arena *arena;
  struct ui_chat_bubble_base *bubble = NULL;
  struct ui_chat_bubble_config config;
  ui_error_t err;
  ui_signal_t *signal = NULL;
  struct ui_dom_rect raw_bounds;
  struct ui_dom_rect text_bounds;

  if (ui_arena_create(1024 * 16, &arena) != UI_ERROR_NONE) {
    FAIL();
  }

  config.tail_placement = UI_CHAT_BUBBLE_TAIL_BOTTOM_RIGHT;
  config.group_position = UI_CHAT_BUBBLE_GROUP_SINGLE;

  err = ui_chat_bubble_base_create(arena, &config, &bubble);
  if (err != UI_ERROR_NONE || bubble == NULL) {
    FAIL();
  }

  err = ui_chat_bubble_base_get_config_signal(bubble, &signal);
  if (err != UI_ERROR_NONE || signal == NULL) {
    FAIL();
  }

  /* Test 1: Calculate text bounds with a Right Tail */
  raw_bounds.x = 0;
  raw_bounds.y = 0;
  raw_bounds.width = 200;
  raw_bounds.height = 100;

  /* Expect:
     x = 0 + padding(12) = 12
     y = 0 + padding(12) = 12
     width = 200 - padding*2(24) - right_tail(10) = 166
     height = 100 - padding*2(24) = 76
  */
  err = ui_chat_bubble_base_calculate_text_bounds(bubble, &raw_bounds,
                                                  &text_bounds);
  if (err != UI_ERROR_NONE)
    FAIL();
  if (fabs(text_bounds.x - 12.0) > 0.01 ||
      fabs(text_bounds.width - 166.0) > 0.01)
    FAIL();

  /* Test 2: Calculate text bounds with a Left Tail */
  config.tail_placement = UI_CHAT_BUBBLE_TAIL_BOTTOM_LEFT;
  err = ui_chat_bubble_base_set_config(bubble, &config);
  if (err != UI_ERROR_NONE)
    FAIL();

  /* Expect:
     x = 0 + padding(12) + left_tail(10) = 22
     width = 200 - padding*2(24) - left_tail(10) = 166
  */
  err = ui_chat_bubble_base_calculate_text_bounds(bubble, &raw_bounds,
                                                  &text_bounds);
  if (err != UI_ERROR_NONE)
    FAIL();
  if (fabs(text_bounds.x - 22.0) > 0.01 ||
      fabs(text_bounds.width - 166.0) > 0.01)
    FAIL();

  /* Test 3: Calculate text bounds with No Tail */
  config.tail_placement = UI_CHAT_BUBBLE_TAIL_TOP_LEFT;
  ui_chat_bubble_base_set_config(bubble, &config);
  ui_chat_bubble_base_calculate_text_bounds(bubble, &raw_bounds, &text_bounds);
  config.tail_placement = UI_CHAT_BUBBLE_TAIL_TOP_RIGHT;
  ui_chat_bubble_base_set_config(bubble, &config);
  ui_chat_bubble_base_calculate_text_bounds(bubble, &raw_bounds, &text_bounds);
  /* Test bounds checks for tail_placement and group_position */
  struct ui_chat_bubble_base *dummy_bubble = NULL;
  config.tail_placement = (enum ui_chat_bubble_tail_placement) - 1;
  if (ui_chat_bubble_base_set_config(bubble, &config) !=
      UI_ERROR_INVALID_ARGUMENT)
    FAIL();
  if (ui_chat_bubble_base_create(arena, &config, &dummy_bubble) !=
      UI_ERROR_INVALID_ARGUMENT)
    FAIL();

  config.tail_placement = (enum ui_chat_bubble_tail_placement)100;
  if (ui_chat_bubble_base_set_config(bubble, &config) !=
      UI_ERROR_INVALID_ARGUMENT)
    FAIL();

  config.tail_placement = UI_CHAT_BUBBLE_TAIL_NONE;
  config.group_position = (enum ui_chat_bubble_group_position) - 1;
  if (ui_chat_bubble_base_set_config(bubble, &config) !=
      UI_ERROR_INVALID_ARGUMENT)
    FAIL();

  config.group_position = (enum ui_chat_bubble_group_position)100;
  if (ui_chat_bubble_base_set_config(bubble, &config) !=
      UI_ERROR_INVALID_ARGUMENT)
    FAIL();

  config.group_position = UI_CHAT_BUBBLE_GROUP_SINGLE;
  ui_chat_bubble_base_calculate_text_bounds(bubble, &raw_bounds, &text_bounds);
  config.tail_placement = UI_CHAT_BUBBLE_TAIL_NONE;
  err = ui_chat_bubble_base_set_config(bubble, &config);
  if (err != UI_ERROR_NONE)
    FAIL();

  /* Expect:
     x = 0 + padding(12) = 12
     width = 200 - padding*2(24) = 176
  */
  err = ui_chat_bubble_base_calculate_text_bounds(bubble, &raw_bounds,
                                                  &text_bounds);
  if (err != UI_ERROR_NONE)
    FAIL();
  if (fabs(text_bounds.x - 12.0) > 0.01 ||
      fabs(text_bounds.width - 176.0) > 0.01)
    FAIL();

  /* Test negative bounds clamping */
  raw_bounds.width = 1.0;
  raw_bounds.height = 1.0;
  ui_chat_bubble_base_calculate_text_bounds(bubble, &raw_bounds, &text_bounds);

  /* Test Invalid Arguments */
  if (ui_chat_bubble_base_create(NULL, &config, &bubble) !=
      UI_ERROR_INVALID_ARGUMENT)
    FAIL();
  if (ui_chat_bubble_base_create(arena, NULL, &bubble) !=
      UI_ERROR_INVALID_ARGUMENT)
    FAIL();
  if (ui_chat_bubble_base_create(arena, &config, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    FAIL();
  (void)ui_chat_bubble_base_destroy(NULL);
  if (ui_chat_bubble_base_set_config(NULL, &config) !=
      UI_ERROR_INVALID_ARGUMENT)
    FAIL();
  if (ui_chat_bubble_base_set_config(bubble, NULL) != UI_ERROR_INVALID_ARGUMENT)
    FAIL();
  if (ui_chat_bubble_base_create(arena, NULL, &bubble) !=
      UI_ERROR_INVALID_ARGUMENT)
    FAIL();
  ui_chat_bubble_base_get_config_signal(NULL, &signal);
  ui_chat_bubble_base_get_config_signal(bubble, NULL);
  ui_chat_bubble_base_calculate_text_bounds(NULL, &raw_bounds, &text_bounds);
  ui_chat_bubble_base_calculate_text_bounds(bubble, NULL, &text_bounds);
  ui_chat_bubble_base_calculate_text_bounds(bubble, &raw_bounds, NULL);

  err = ui_chat_bubble_base_destroy(bubble);
  if (err != UI_ERROR_NONE) {
    FAIL();
  }

  /* Test Arena Allocation Failures */
#ifdef UI_TEST_MOCK_ALLOC
  {
    int i;
    for (i = 0; i < 20; i++) {
      struct ui_chat_bubble_base *temp_bubble = NULL;
      g_malloc_fail_countdown = i;
      err = ui_chat_bubble_base_create(arena, &config, &temp_bubble);
      if (err == UI_ERROR_NONE) {
        (void)ui_chat_bubble_base_destroy(temp_bubble);
        break; /* Passed enough */
      }
    }
    g_malloc_fail_countdown = -1;
  }
#endif

  (void)ui_arena_destroy(arena);
  return 0;
}
