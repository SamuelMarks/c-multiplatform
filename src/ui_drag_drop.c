/* clang-format off */
#include "ui_drag_drop.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define UI_DRAG_DROP_DEFAULT_THRESHOLD 5

struct ui_drag_drop_context {
  int drag_threshold;
  enum ui_drag_state state;

  int start_x;
  int start_y;
  int current_x;
  int current_y;

  int drag_item_id;
  int drag_source_list_id;
  int drag_item_width;
  int drag_item_height;
  int drag_item_offset_x;
  int drag_item_offset_y;

  struct ui_drag_list *lists;
  int list_capacity;
  int list_count;

  int drop_occurred;
  int drop_item_id;
  int drop_from_list;
  int drop_to_list;
  int drop_to_index;

  struct ui_drag_placeholder placeholder;

  int active_pointer_id;
  int pointer_is_down;
};

ui_error_t ui_drag_drop_create(struct ui_drag_drop_context **out_ctx) {
  struct ui_drag_drop_context *ctx;

  if (!out_ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ctx = (struct ui_drag_drop_context *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_drag_drop_context));
  if (!ctx) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(ctx, 0, sizeof(struct ui_drag_drop_context));
  ctx->drag_threshold = UI_DRAG_DROP_DEFAULT_THRESHOLD;
  ctx->active_pointer_id = -1;
  ctx->list_capacity = 4;

  ctx->lists = (struct ui_drag_list *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_drag_list) * ctx->list_capacity);
  if (!ctx->lists) {
    C_MULTIPLATFORM_FREE(ctx);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  *out_ctx = ctx;
  return UI_ERROR_NONE;
}

ui_error_t ui_drag_drop_destroy(struct ui_drag_drop_context *ctx) {
  int i;

  if (!ctx) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < ctx->list_count; i++) {
    if (ctx->lists[i].items) {
      C_MULTIPLATFORM_FREE(ctx->lists[i].items);
    }
  }

  C_MULTIPLATFORM_FREE(ctx->lists);

  C_MULTIPLATFORM_FREE(ctx);
  return UI_ERROR_NONE;
}

ui_error_t ui_drag_drop_set_drag_threshold(struct ui_drag_drop_context *ctx,
                                           int distance) {
  if (!ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ctx->drag_threshold = distance;
  return UI_ERROR_NONE;
}

ui_error_t ui_drag_drop_clear_lists(struct ui_drag_drop_context *ctx) {
  int i;

  if (!ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < ctx->list_count; i++) {
    if (ctx->lists[i].items) {
      C_MULTIPLATFORM_FREE(ctx->lists[i].items);
      ctx->lists[i].items = NULL;
    }
  }

  ctx->list_count = 0;
  return UI_ERROR_NONE;
}

ui_error_t ui_drag_drop_add_list(struct ui_drag_drop_context *ctx,
                                 const struct ui_drag_list *list) {
  struct ui_drag_list *new_list;

  if (!ctx || !list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (list->item_count > 0 && !list->items) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ctx->list_count >= ctx->list_capacity) {
    int new_cap = ctx->list_capacity * 2;
    struct ui_drag_list *new_lists =
        (struct ui_drag_list *)C_MULTIPLATFORM_REALLOC(
            ctx->lists, sizeof(struct ui_drag_list) * new_cap);
    if (!new_lists) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    ctx->lists = new_lists;
    ctx->list_capacity = new_cap;
  }

  new_list = &ctx->lists[ctx->list_count++];
  *new_list = *list;
  new_list->items = NULL;

  if (list->item_count > 0 && list->items) {
    new_list->items = (struct ui_drag_item *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_drag_item) * list->item_count);
    if (!new_list->items) {
      ctx->list_count--; /* Revert count on failure */
      return UI_ERROR_OUT_OF_MEMORY;
    }
    memcpy(new_list->items, list->items,
           sizeof(struct ui_drag_item) * list->item_count);
  }

  return UI_ERROR_NONE;
}

static void update_placeholder(struct ui_drag_drop_context *ctx) {
  int drag_center_x =
      ctx->current_x - ctx->drag_item_offset_x + ctx->drag_item_width / 2;
  int drag_center_y =
      ctx->current_y - ctx->drag_item_offset_y + ctx->drag_item_height / 2;
  int i, j;

  ctx->placeholder.active = 0;

  for (i = 0; i < ctx->list_count; i++) {
    struct ui_drag_list *list = &ctx->lists[i];

    if (drag_center_x >= list->x && drag_center_x < list->x + list->width &&
        drag_center_y >= list->y && drag_center_y < list->y + list->height) {

      ctx->placeholder.active = 1;
      ctx->placeholder.list_id = list->list_id;
      ctx->placeholder.width = ctx->drag_item_width;
      ctx->placeholder.height = ctx->drag_item_height;

      if (list->orientation == UI_DRAG_LIST_ORIENTATION_VERTICAL) {
        for (j = 0; j < list->item_count; j++) {
          struct ui_drag_item *item = &list->items[j];
          int item_mid_y = item->y + item->height / 2;
          if (drag_center_y < item_mid_y) {
            ctx->placeholder.index = j;
            ctx->placeholder.x = item->x;
            ctx->placeholder.y = item->y;
            return;
          }
        }

        ctx->placeholder.index = list->item_count;
        if (list->item_count > 0) {
          ctx->placeholder.x = list->items[list->item_count - 1].x;
          ctx->placeholder.y = list->items[list->item_count - 1].y +
                               list->items[list->item_count - 1].height;
        } else {
          ctx->placeholder.x = list->x;
          ctx->placeholder.y = list->y;
        }
      } else {
        for (j = 0; j < list->item_count; j++) {
          struct ui_drag_item *item = &list->items[j];
          int item_mid_x = item->x + item->width / 2;
          if (drag_center_x < item_mid_x) {
            ctx->placeholder.index = j;
            ctx->placeholder.x = item->x;
            ctx->placeholder.y = item->y;
            return;
          }
        }

        ctx->placeholder.index = list->item_count;
        if (list->item_count > 0) {
          ctx->placeholder.x = list->items[list->item_count - 1].x +
                               list->items[list->item_count - 1].width;
          ctx->placeholder.y = list->items[list->item_count - 1].y;
        } else {
          ctx->placeholder.x = list->x;
          ctx->placeholder.y = list->y;
        }
      }
      return;
    }
  }
  return;
}

static void handle_pointer_down(struct ui_drag_drop_context *ctx,
                                int pointer_id, int x, int y) {
  int i, j;

  if (ctx->state != UI_DRAG_STATE_IDLE) {
    return;
  }

  ctx->active_pointer_id = pointer_id;
  ctx->pointer_is_down = 1;
  ctx->start_x = x;
  ctx->start_y = y;
  ctx->current_x = x;
  ctx->current_y = y;

  for (i = 0; i < ctx->list_count; i++) {
    struct ui_drag_list *list = &ctx->lists[i];
    for (j = 0; j < list->item_count; j++) {
      struct ui_drag_item *item = &list->items[j];
      if (x >= item->x && x < item->x + item->width && y >= item->y &&
          y < item->y + item->height) {

        ctx->state = UI_DRAG_STATE_PENDING;
        ctx->drag_item_id = item->item_id;
        ctx->drag_source_list_id = list->list_id;
        ctx->drag_item_width = item->width;
        ctx->drag_item_height = item->height;
        ctx->drag_item_offset_x = x - item->x;
        ctx->drag_item_offset_y = y - item->y;
        return;
      }
    }
  }
  return;
}

static void handle_pointer_move(struct ui_drag_drop_context *ctx,
                                int pointer_id, int x, int y) {
  if (ctx->active_pointer_id != pointer_id)
    return;

  ctx->current_x = x;
  ctx->current_y = y;

  if (ctx->state == UI_DRAG_STATE_PENDING) {
    int dx = x - ctx->start_x;
    int dy = y - ctx->start_y;
    if (dx * dx + dy * dy >= ctx->drag_threshold * ctx->drag_threshold) {
      ctx->state = UI_DRAG_STATE_DRAGGING;
    }
  }

  if (ctx->state == UI_DRAG_STATE_DRAGGING) {
    update_placeholder(ctx);
  }
  return;
}

static void handle_pointer_up(struct ui_drag_drop_context *ctx,
                              int pointer_id) {
  if (ctx->active_pointer_id != pointer_id) {
    return;
  }

  if (ctx->state == UI_DRAG_STATE_DRAGGING && ctx->placeholder.active) {
    ctx->drop_occurred = 1;
    ctx->drop_item_id = ctx->drag_item_id;
    ctx->drop_from_list = ctx->drag_source_list_id;
    ctx->drop_to_list = ctx->placeholder.list_id;
    ctx->drop_to_index = ctx->placeholder.index;
  }

  ctx->state = UI_DRAG_STATE_IDLE;
  ctx->pointer_is_down = 0;
  ctx->active_pointer_id = -1;
  ctx->placeholder.active = 0;
  return;
}

static void handle_pointer_cancel(struct ui_drag_drop_context *ctx,
                                  int pointer_id) {
  if (ctx->active_pointer_id != pointer_id) {
    return;
  }
  ctx->state = UI_DRAG_STATE_IDLE;
  ctx->pointer_is_down = 0;
  ctx->active_pointer_id = -1;
  ctx->placeholder.active = 0;
  return;
}

ui_error_t ui_drag_drop_process_event(struct ui_drag_drop_context *ctx,
                                      const struct ui_event *event) {
  if (!ctx || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  switch (event->type) {
  case UI_EVENT_MOUSE_DOWN:
    if (event->event_data.mouse.button == 0) {
      handle_pointer_down(ctx, 0, event->event_data.mouse.x,
                          event->event_data.mouse.y);
    }
    break;
  case UI_EVENT_MOUSE_MOVE: {
    handle_pointer_move(ctx, 0, event->event_data.mouse.x,
                        event->event_data.mouse.y);
  } break;
  case UI_EVENT_MOUSE_UP:
    if (event->event_data.mouse.button == 0) {
      handle_pointer_up(ctx, 0);
    }
    break;
  case UI_EVENT_TOUCH_START:
    if (event->event_data.touch.num_points > 0) {
      handle_pointer_down(ctx, event->event_data.touch.points[0].id,
                          event->event_data.touch.points[0].x,
                          event->event_data.touch.points[0].y);
    }
    break;
  case UI_EVENT_TOUCH_MOVE:
    if (event->event_data.touch.num_points > 0) {
      handle_pointer_move(ctx, event->event_data.touch.points[0].id,
                          event->event_data.touch.points[0].x,
                          event->event_data.touch.points[0].y);
    }
    break;
  case UI_EVENT_TOUCH_END:
    if (event->event_data.touch.num_points > 0) {
      handle_pointer_up(ctx, event->event_data.touch.points[0].id);
    }
    break;
  case UI_EVENT_TOUCH_CANCEL:
    if (event->event_data.touch.num_points > 0) {
      handle_pointer_cancel(ctx, event->event_data.touch.points[0].id);
    }
    break;
  default:
    break;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_drag_drop_get_state(const struct ui_drag_drop_context *ctx,
                                  enum ui_drag_state *out_state) {
  if (!ctx || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_state = ctx->state;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_drag_drop_get_dragged_item(const struct ui_drag_drop_context *ctx,
                                         int *out_item_id,
                                         int *out_source_list_id,
                                         int *out_current_x,
                                         int *out_current_y) {

  if (!ctx || !out_item_id || !out_source_list_id || !out_current_x ||
      !out_current_y) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ctx->state != UI_DRAG_STATE_DRAGGING) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_item_id = ctx->drag_item_id;
  *out_source_list_id = ctx->drag_source_list_id;
  *out_current_x = ctx->current_x - ctx->drag_item_offset_x;
  *out_current_y = ctx->current_y - ctx->drag_item_offset_y;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_drag_drop_get_placeholder(const struct ui_drag_drop_context *ctx,
                             struct ui_drag_placeholder *out_placeholder) {
  if (!ctx || !out_placeholder) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_placeholder = ctx->placeholder;
  return UI_ERROR_NONE;
}

ui_error_t ui_drag_drop_get_drop_event(struct ui_drag_drop_context *ctx,
                                       int *out_dropped, int *out_item_id,
                                       int *out_from_list, int *out_to_list,
                                       int *out_to_index) {

  if (!ctx || !out_dropped || !out_item_id || !out_from_list || !out_to_list ||
      !out_to_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_dropped = ctx->drop_occurred;
  if (ctx->drop_occurred) {
    *out_item_id = ctx->drop_item_id;
    *out_from_list = ctx->drop_from_list;
    *out_to_list = ctx->drop_to_list;
    *out_to_index = ctx->drop_to_index;
    ctx->drop_occurred = 0; /* Clear event */
  }

  return UI_ERROR_NONE;
}
