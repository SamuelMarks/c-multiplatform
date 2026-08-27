/**
 * @file ui_titlebar_base.c
 * @brief Implementation of the custom titlebar base component logic.
 */
/* clang-format off */
#include "ui_titlebar_base.h"
#include "ui_arena.h"
#include <stddef.h>
/* clang-format on */

/**
 * @struct ui_titlebar_btn_rect
 * @struct ui_titlebar_btn_rect
 * @brief Internal representation of a button hit area in the titlebar.
 */
struct ui_titlebar_btn_rect {
  /* @brief The type of button for this rect. */
  enum ui_titlebar_hit_test_result btn_type; /**< btn_type */
  /* @brief X coordinate of the rect. */
  float x; /**< x */
  /* @brief Y coordinate of the rect. */
  float y; /**< y */
  /* @brief Width of the rect. */
  float w; /**< w */
  /* @brief Height of the rect. */
  float h; /**< h */
  /* @brief Pointer to the next button rect in the linked list. */
  struct ui_titlebar_btn_rect *next; /**< next */
};

/**
 * @struct ui_titlebar_base
 * @struct ui_titlebar_base
 * @brief Internal implementation of the titlebar base component.
 */
struct ui_titlebar_base {
  /* @brief Memory arena for allocations. */
  struct ui_arena *arena; /**< arena */
  /* @brief The titlebar configuration. */
  struct ui_titlebar_config config; /**< config */
  /* @brief Linked list of button hit areas. */
  struct ui_titlebar_btn_rect *buttons; /**< buttons */
};

ui_error_t ui_titlebar_base_create(struct ui_arena *arena,
                                   const struct ui_titlebar_config *config,
                                   struct ui_titlebar_base **out_titlebar) {
  void *ptr;

  if (!arena || !config || !out_titlebar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_arena_alloc(arena, sizeof(struct ui_titlebar_base), 8, &ptr);

  *out_titlebar = (struct ui_titlebar_base *)ptr;
  (*out_titlebar)->arena = arena;
  (*out_titlebar)->config = *config;
  (*out_titlebar)->buttons = NULL;

  return UI_ERROR_NONE;
}

ui_error_t ui_titlebar_base_destroy(struct ui_titlebar_base *titlebar) {
  if (!titlebar)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_NONE;
}

ui_error_t
ui_titlebar_base_add_button_rect(struct ui_titlebar_base *titlebar,
                                 enum ui_titlebar_hit_test_result btn_type,
                                 float x, float y, float w, float h) {
  void *ptr;
  struct ui_titlebar_btn_rect *rect;

  if (!titlebar)
    return UI_ERROR_INVALID_ARGUMENT;

  ui_arena_alloc(titlebar->arena, sizeof(struct ui_titlebar_btn_rect), 8, &ptr);

  rect = (struct ui_titlebar_btn_rect *)ptr;
  rect->btn_type = btn_type;
  rect->x = x;
  rect->y = y;
  rect->w = w;
  rect->h = h;

  rect->next = titlebar->buttons;
  titlebar->buttons = rect;

  return UI_ERROR_NONE;
}

ui_error_t
ui_titlebar_base_hit_test(struct ui_titlebar_base *titlebar, float x, float y,
                          enum ui_titlebar_hit_test_result *out_result) {
  struct ui_titlebar_btn_rect *curr;

  if (!titlebar || !out_result)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_result = UI_TITLEBAR_HIT_TEST_NONE;

  /* Check if inside overall titlebar area */
  if (y < 0.0f || y > titlebar->config.height) {
    return UI_ERROR_NONE;
  }

  /* Check specific buttons */
  curr = titlebar->buttons;
  while (curr != NULL) {
    if (x >= curr->x && x <= (curr->x + curr->w) && y >= curr->y &&
        y <= (curr->y + curr->h)) {
      *out_result = curr->btn_type;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  /* If no buttons were hit but we are in the titlebar, it's a drag area */
  if (titlebar->config.draggable) {
    *out_result = UI_TITLEBAR_HIT_TEST_DRAG_AREA;
  }

  return UI_ERROR_NONE;
}
