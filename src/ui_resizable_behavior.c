/* clang-format off */
#include "ui_resizable_behavior.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_resizable_behavior {
  unsigned int allowed_edges;
  int min_w;
  int min_h;
  int max_w;
  int max_h;

  ui_resizable_on_resize_t on_resize;
  void *user_data;

  int is_dragging;
  unsigned int active_edges;
  int start_x;
  int start_y;
  int start_w;
  int start_h;
};

/** \brief ui_error */
enum ui_error
ui_resizable_behavior_create(struct ui_resizable_behavior **out_behavior) {
  struct ui_resizable_behavior *b;

  if (!out_behavior) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  b = (struct ui_resizable_behavior *)UI_MALLOC(
      sizeof(struct ui_resizable_behavior));
  if (!b) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  b->allowed_edges = UI_RESIZABLE_EDGE_ALL;
  b->min_w = -1;
  b->min_h = -1;
  b->max_w = -1;
  b->max_h = -1;
  b->on_resize = NULL;
  b->user_data = NULL;

  b->is_dragging = 0;
  b->active_edges = 0;
  b->start_x = 0;
  b->start_y = 0;
  b->start_w = 0;
  b->start_h = 0;

  *out_behavior = b;
  return UI_ERROR_NONE;
}

void ui_resizable_behavior_destroy(struct ui_resizable_behavior *behavior) {
  if (!behavior)
    return;
  UI_FREE(behavior);
}

/** \brief ui_error */
enum ui_error
ui_resizable_behavior_configure(struct ui_resizable_behavior *behavior,
                                unsigned int edges, int min_width,
                                int min_height, int max_width, int max_height) {
  if (!behavior)
    return UI_ERROR_INVALID_ARGUMENT;
  behavior->allowed_edges = edges;
  behavior->min_w = min_width;
  behavior->min_h = min_height;
  behavior->max_w = max_width;
  behavior->max_h = max_height;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_resizable_behavior_set_on_resize(struct ui_resizable_behavior *behavior,
                                    ui_resizable_on_resize_t on_resize,
                                    void *user_data) {
  if (!behavior)
    return UI_ERROR_INVALID_ARGUMENT;
  behavior->on_resize = on_resize;
  behavior->user_data = user_data;
  return UI_ERROR_NONE;
}

/** \brief ui_resizable_behavior_process_event */
enum ui_error ui_resizable_behavior_process_event(
    struct ui_resizable_behavior *behavior, const struct ui_event *event,
    int current_width, int current_height, int hit_test_thickness) {

  int mx = 0, my = 0;
  int is_down = 0, is_up = 0, is_move = 0;

  if (!behavior || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (event->type == UI_EVENT_MOUSE_DOWN) {
    mx = event->event_data.mouse.x;
    my = event->event_data.mouse.y;
    is_down = 1;
  } else if (event->type == UI_EVENT_MOUSE_UP) {
    is_up = 1;
  } else if (event->type == UI_EVENT_MOUSE_MOVE) {
    mx = event->event_data.mouse.x;
    my = event->event_data.mouse.y;
    is_move = 1;
  } else {
    return UI_ERROR_NONE; /* Ignored event */
  }

  if (is_down && !behavior->is_dragging) {
    unsigned int hit = 0;
    if (mx < hit_test_thickness)
      hit |= UI_RESIZABLE_EDGE_LEFT;
    if (mx > current_width - hit_test_thickness)
      hit |= UI_RESIZABLE_EDGE_RIGHT;
    if (my < hit_test_thickness)
      hit |= UI_RESIZABLE_EDGE_TOP;
    if (my > current_height - hit_test_thickness)
      hit |= UI_RESIZABLE_EDGE_BOTTOM;

    hit &= behavior->allowed_edges;

    if (hit) {
      behavior->is_dragging = 1;
      behavior->active_edges = hit;
      behavior->start_x = mx;
      behavior->start_y = my;
      behavior->start_w = current_width;
      behavior->start_h = current_height;
    }
  } else if (is_move && behavior->is_dragging) {
    int dx = mx - behavior->start_x;
    int dy = my - behavior->start_y;
    int new_w = behavior->start_w;
    int new_h = behavior->start_h;

    if (behavior->active_edges & UI_RESIZABLE_EDGE_RIGHT) {
      new_w += dx;
    } else if (behavior->active_edges & UI_RESIZABLE_EDGE_LEFT) {
      new_w -= dx;
    }

    if (behavior->active_edges & UI_RESIZABLE_EDGE_BOTTOM) {
      new_h += dy;
    } else if (behavior->active_edges & UI_RESIZABLE_EDGE_TOP) {
      new_h -= dy;
    }

    if (behavior->min_w != -1 && new_w < behavior->min_w)
      new_w = behavior->min_w;
    if (behavior->max_w != -1 && new_w > behavior->max_w)
      new_w = behavior->max_w;
    if (behavior->min_h != -1 && new_h < behavior->min_h)
      new_h = behavior->min_h;
    if (behavior->max_h != -1 && new_h > behavior->max_h)
      new_h = behavior->max_h;

    if (behavior->on_resize &&
        (new_w != current_width || new_h != current_height)) {
      behavior->on_resize(new_w, new_h, behavior->user_data);
    }
  } else if (is_up) {
    behavior->is_dragging = 0;
    behavior->active_edges = 0;
  }

  return UI_ERROR_NONE;
}
