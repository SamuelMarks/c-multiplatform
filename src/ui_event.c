/**
 * @file ui_event.c
 * @brief ui_event.c implementation.
 */
/* clang-format off */
#include "../include/ui_event.h"
#include "../include/ui_error.h"
#include "../include/ui_layout.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_focus_manager.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief trigger_event.
 * @param node Parameter node.
 * @param event Parameter event.
 * @return Return value.
 */
static ui_error_t trigger_event(const struct ui_dom_node *node,
                                const struct ui_event *event) {
  struct ui_dom_event_listener *listener;
  if (!node)
    return UI_ERROR_NONE;
  listener = node->listeners;
  while (listener) {
    if (listener->type == event->type) {
      {
        ui_error_t h_rc = listener->handler((struct ui_dom_node *)node, event,
                                            listener->user_data);
        if (h_rc != UI_ERROR_NONE) {
          return h_rc;
        }
      }
    }
    listener = listener->next;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief hit_test.
 * @param node Parameter node.
 * @param x Parameter x.
 * @param y Parameter y.
 * @param out_node Parameter out_node.
 * @return Return value.
 */
static void hit_test(const struct ui_layout_node *node, float x, float y,
                     const struct ui_layout_node **out_node) {
  *out_node = NULL;
  if (x >= node->x && x <= node->x + node->width && y >= node->y &&
      y <= node->y + node->height) {
    *out_node = node;
  }
}

ui_error_t ui_event_dispatch(const struct ui_layout_node *layout_root,
                             const struct ui_event *event,
                             struct ui_mouse_state *state, void *user_data);

/**
 * @brief ui_event_dispatch.
 * @param layout_root Parameter layout_root.
 * @param event Parameter event.
 * @param state Parameter state.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
ui_error_t ui_event_dispatch(const struct ui_layout_node *layout_root,
                             const struct ui_event *event,
                             struct ui_mouse_state *state, void *user_data) {
  struct ui_focus_manager *focus_mgr = (struct ui_focus_manager *)user_data;
  if (!layout_root || !event || !state)
    return UI_ERROR_NONE;

  if (event->type == UI_EVENT_MOUSE_MOVE ||
      event->type == UI_EVENT_MOUSE_DOWN || event->type == UI_EVENT_MOUSE_UP ||
      event->type == UI_EVENT_MOUSE_WHEEL) {

    const struct ui_layout_node *hit = NULL;
    {
      hit_test(layout_root, (float)event->event_data.mouse.x,
               (float)event->event_data.mouse.y, &hit);
    }

    if (event->type == UI_EVENT_MOUSE_MOVE) {
      if (hit != state->hovered_node) {
        if (state->hovered_node && state->hovered_node->dom_node) {
          struct ui_event ev_leave = *event;
          ev_leave.type = UI_EVENT_MOUSE_LEAVE;
          {
            ui_error_t trig_rc1 =
                trigger_event(state->hovered_node->dom_node, &ev_leave);
            if (trig_rc1 != UI_ERROR_NONE) {
              return trig_rc1;
            }
          }
        }
        state->hovered_node = (struct ui_layout_node *)hit;
        if (state->hovered_node && state->hovered_node->dom_node) {
          struct ui_event ev_enter = *event;
          ev_enter.type = UI_EVENT_MOUSE_ENTER;
          {
            ui_error_t trig_rc2 =
                trigger_event(state->hovered_node->dom_node, &ev_enter);
            if (trig_rc2 != UI_ERROR_NONE) {
              return trig_rc2;
            }
          }
        }
      }
      if (hit && hit->dom_node) {
        {
          ui_error_t trig_rc3 = trigger_event(hit->dom_node, event);
          if (trig_rc3 != UI_ERROR_NONE) {
            return trig_rc3;
          }
        }
      }
    } else if (event->type == UI_EVENT_MOUSE_DOWN) {
      if (hit && hit->dom_node) {
        {
          ui_error_t trig_rc4 = trigger_event(hit->dom_node, event);
          if (trig_rc4 != UI_ERROR_NONE) {
            return trig_rc4;
          }
        }
      }
      state->active_node = (struct ui_layout_node *)hit;
      if (hit && event->event_data.mouse.button == 1) {
        struct ui_event ev_ctx = *event;
        ev_ctx.type = UI_EVENT_CONTEXT_MENU;
        {
          ui_error_t trig_rc5 = trigger_event(hit->dom_node, &ev_ctx);
          if (trig_rc5 != UI_ERROR_NONE) {
            return trig_rc5;
          }
        }
      }
    } else if (event->type == UI_EVENT_MOUSE_UP) {
      if (hit && hit->dom_node) {
        {
          ui_error_t trig_rc6 = trigger_event(hit->dom_node, event);
          if (trig_rc6 != UI_ERROR_NONE) {
            return trig_rc6;
          }
        }
      }
      if (hit && hit == state->active_node &&
          event->event_data.mouse.button == 0) {
        struct ui_event ev_click;
        state->click_count++;
        ev_click = *event;
        ev_click.type = UI_EVENT_CLICK;
        {
          ui_error_t trig_rc7 = trigger_event(hit->dom_node, &ev_click);
          if (trig_rc7 != UI_ERROR_NONE) {
            return trig_rc7;
          }
        }
        if (state->click_count == 2) {
          struct ui_event ev_dbl;
          ev_dbl = *event;
          ev_dbl.type = UI_EVENT_DBLCLICK;
          {
            ui_error_t trig_rc8 = trigger_event(hit->dom_node, &ev_dbl);
            if (trig_rc8 != UI_ERROR_NONE) {
              return trig_rc8;
            }
          }
          state->click_count = 0;
        }
      }
    } else {
      if (hit && hit->dom_node) {
        {
          ui_error_t trig_rc9 = trigger_event(hit->dom_node, event);
          if (trig_rc9 != UI_ERROR_NONE) {
            return trig_rc9;
          }
        }
      }
    }
  } else if (event->type == UI_EVENT_KEY_DOWN ||
             event->type == UI_EVENT_KEY_UP ||
             event->type == UI_EVENT_KEY_PRESS) {
    struct ui_dom_node *focused = NULL;
    if (focus_mgr) {
      {
        ui_error_t rc_cleanup =
            ui_focus_manager_get_focused_node(focus_mgr, &focused);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
    if (focused) {
      {
        ui_error_t trig_rc10 = trigger_event(focused, event);
        if (trig_rc10 != UI_ERROR_NONE) {
          return trig_rc10;
        }
      }
    }
    if (event->type == UI_EVENT_KEY_DOWN &&
        event->event_data.keyboard.key_code == UI_KEY_TAB && focus_mgr &&
        layout_root->dom_node) {
      int forward = !(event->event_data.keyboard.modifiers & UI_MODIFIER_SHIFT);
      {
        ui_error_t rc_cleanup = ui_focus_manager_advance(
            focus_mgr, (struct ui_dom_node *)layout_root->dom_node, forward);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
  }
  return UI_ERROR_NONE;
}
