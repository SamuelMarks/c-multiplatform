/* clang-format off */
#include "../include/ui_event.h"
#include "../include/ui_error.h"
#include "../include/ui_layout.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_focus_manager.h"
#include <stddef.h>
/* clang-format on */

static void trigger_event(const struct ui_dom_node *node,
                          const struct ui_event *event) {
  struct ui_dom_event_listener *listener;
  listener = node->listeners;
  while (listener) {
    if (listener->type == event->type && listener->handler) {
      listener->handler((struct ui_dom_node *)node, event, listener->user_data);
    }
    listener = listener->next;
  }
}

static enum ui_error hit_test(const struct ui_layout_node *node, float x,
                              float y, const struct ui_layout_node **out_node) {
  *out_node = NULL;
  if (x >= node->x && x <= node->x + node->width && y >= node->y &&
      y <= node->y + node->height) {
    *out_node = node;
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_event_dispatch(const struct ui_layout_node *layout_root,
                                const struct ui_event *event,
                                struct ui_mouse_state *state, void *user_data);

enum ui_error ui_event_dispatch(const struct ui_layout_node *layout_root,
                                const struct ui_event *event,
                                struct ui_mouse_state *state, void *user_data) {
  struct ui_focus_manager *focus_mgr = (struct ui_focus_manager *)user_data;
  if (!layout_root || !event || !state)
    return UI_ERROR_NONE;

  if (event->type == UI_EVENT_MOUSE_MOVE ||
      event->type == UI_EVENT_MOUSE_DOWN || event->type == UI_EVENT_MOUSE_UP ||
      event->type == UI_EVENT_MOUSE_WHEEL) {

    const struct ui_layout_node *hit = NULL;
    hit_test(layout_root, (float)event->event_data.mouse.x,
             (float)event->event_data.mouse.y, &hit);

    if (event->type == UI_EVENT_MOUSE_MOVE) {
      if (hit != state->hovered_node) {
        if (state->hovered_node && state->hovered_node->dom_node) {
          struct ui_event ev_leave = *event;
          ev_leave.type = UI_EVENT_MOUSE_LEAVE;
          trigger_event(state->hovered_node->dom_node, &ev_leave);
        }
        state->hovered_node = (struct ui_layout_node *)hit;
        if (state->hovered_node && state->hovered_node->dom_node) {
          struct ui_event ev_enter = *event;
          ev_enter.type = UI_EVENT_MOUSE_ENTER;
          trigger_event(state->hovered_node->dom_node, &ev_enter);
        }
      }
      if (hit && hit->dom_node) {
        trigger_event(hit->dom_node, event);
      }
    } else if (event->type == UI_EVENT_MOUSE_DOWN) {
      if (hit && hit->dom_node) {
        trigger_event(hit->dom_node, event);
      }
      state->active_node = (struct ui_layout_node *)hit;
      if (hit && event->event_data.mouse.button == 1) {
        struct ui_event ev_ctx = *event;
        ev_ctx.type = UI_EVENT_CONTEXT_MENU;
        trigger_event(hit->dom_node, &ev_ctx);
      }
    } else if (event->type == UI_EVENT_MOUSE_UP) {
      if (hit && hit->dom_node) {
        trigger_event(hit->dom_node, event);
      }
      if (hit && hit == state->active_node &&
          event->event_data.mouse.button == 0) {
        struct ui_event ev_click;
        state->click_count++;
        ev_click = *event;
        ev_click.type = UI_EVENT_CLICK;
        trigger_event(hit->dom_node, &ev_click);
        if (state->click_count == 2) {
          struct ui_event ev_dbl;
          ev_dbl = *event;
          ev_dbl.type = UI_EVENT_DBLCLICK;
          trigger_event(hit->dom_node, &ev_dbl);
          state->click_count = 0;
        }
      }
    } else if (event->type == UI_EVENT_MOUSE_WHEEL) {
      if (hit && hit->dom_node) {
        trigger_event(hit->dom_node, event);
      }
    }
  } else if (event->type == UI_EVENT_KEY_DOWN ||
             event->type == UI_EVENT_KEY_UP ||
             event->type == UI_EVENT_KEY_PRESS) {
    struct ui_dom_node *focused = NULL;
    if (focus_mgr) {
      ui_focus_manager_get_focused_node(focus_mgr, &focused);
    }
    if (focused) {
      trigger_event(focused, event);
    }
    if (event->type == UI_EVENT_KEY_DOWN &&
        event->event_data.keyboard.key_code == UI_KEY_TAB && focus_mgr &&
        layout_root->dom_node) {
      int forward = !(event->event_data.keyboard.modifiers & UI_MODIFIER_SHIFT);
      ui_focus_manager_advance(
          focus_mgr, (struct ui_dom_node *)layout_root->dom_node, forward);
    }
  }
  return UI_ERROR_NONE;
}
