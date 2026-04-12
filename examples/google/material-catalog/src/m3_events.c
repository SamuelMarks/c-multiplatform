/* clang-format off */
#include "m3_events.h"
#include <math.h>
/* clang-format on */

int m3_event_engine_init(m3_event_engine_t *engine) {
  if (!engine) {
    return 1;
  }
  engine->active_modality = CMP_EVENT_TYPE_MOUSE;
  engine->hovered_node = NULL;
  engine->focused_node = NULL;
  engine->pressed_node = NULL;
  engine->is_focus_ring_visible = 0;
  engine->is_dragging = 0;
  engine->start_x = 0.0f;
  engine->start_y = 0.0f;
  engine->current_x = 0.0f;
  engine->current_y = 0.0f;
  engine->velocity_x = 0.0f;
  engine->velocity_y = 0.0f;
  engine->last_event_time_ms = 0;
  engine->gamepad_connected = 0;
  return 0;
}

int m3_event_engine_cleanup(m3_event_engine_t *engine) {
  if (!engine) {
    return 1;
  }
  /* Memory is managed globally, just clear references */
  engine->hovered_node = NULL;
  engine->focused_node = NULL;
  engine->pressed_node = NULL;
  return 0;
}

int m3_event_process(m3_event_engine_t *engine, material_catalog_state_t *state,
                     const cmp_event_t *event, uint32_t current_time_ms) {
  uint32_t dt_ms;
  float dx;
  float dy;

  if (!engine || !state || !event) {
    return 1;
  }

  /* Update global active modality */
  engine->active_modality = event->type;

  /* Focus ring is strictly for keyboard traversal */
  if (event->type == CMP_EVENT_TYPE_KEYBOARD) {
    engine->is_focus_ring_visible = 1;
  } else if (event->type == CMP_EVENT_TYPE_MOUSE ||
             event->type == CMP_EVENT_TYPE_TOUCH) {
    if (event->action == CMP_ACTION_DOWN) {
      engine->is_focus_ring_visible = 0;
    }
  }

  /* Touch / Drag state management */
  if (event->type == CMP_EVENT_TYPE_MOUSE ||
      event->type == CMP_EVENT_TYPE_TOUCH) {
    if (event->action == CMP_ACTION_DOWN) {
      engine->is_dragging = 1;
      engine->start_x = (float)event->x;
      engine->start_y = (float)event->y;
      engine->current_x = (float)event->x;
      engine->current_y = (float)event->y;
      engine->velocity_x = 0.0f;
      engine->velocity_y = 0.0f;
      engine->last_event_time_ms = current_time_ms;
    } else if (event->action == CMP_ACTION_MOVE && engine->is_dragging) {
      dt_ms = current_time_ms - engine->last_event_time_ms;
      if (dt_ms > 0) {
        float alpha = 0.2f;
        float new_vx;
        float new_vy;
        dx = (float)event->x - engine->current_x;
        dy = (float)event->y - engine->current_y;

        /* Basic exponential moving average for velocity to smooth out spikes */

        alpha = 0.2f;
        new_vx = dx / (float)dt_ms;
        new_vy = dy / (float)dt_ms;

        engine->velocity_x =
            engine->velocity_x * (1.0f - alpha) + new_vx * alpha;
        engine->velocity_y =
            engine->velocity_y * (1.0f - alpha) + new_vy * alpha;
      }
      engine->current_x = (float)event->x;
      engine->current_y = (float)event->y;
      engine->last_event_time_ms = current_time_ms;
    } else if (event->action == CMP_ACTION_UP ||
               event->action == CMP_ACTION_CANCEL) {
      engine->is_dragging = 0;
    }
  }

  return 0;
}

int m3_event_get_active_modality(const m3_event_engine_t *engine,
                                 uint32_t *out_modality) {
  if (!engine || !out_modality) {
    return 1;
  }
  *out_modality = engine->active_modality;
  return 0;
}

int m3_event_check_touch_slop(const m3_event_engine_t *engine, float dpi_scale,
                              int *out_exceeded) {
  float dx;
  float dy;
  float distance_sq;
  float slop_px;

  if (!engine || !out_exceeded) {
    return 1;
  }

  if (!engine->is_dragging) {
    *out_exceeded = 0;
    return 0;
  }

  dx = engine->current_x - engine->start_x;
  dy = engine->current_y - engine->start_y;
  distance_sq = (dx * dx) + (dy * dy);

  slop_px = M3_TOUCH_SLOP_DP * dpi_scale;

  if (distance_sq >= (slop_px * slop_px)) {
    *out_exceeded = 1;
  } else {
    *out_exceeded = 0;
  }

  return 0;
}

int m3_event_get_velocity(const m3_event_engine_t *engine, float *out_vx,
                          float *out_vy) {
  if (!engine || !out_vx || !out_vy) {
    return 1;
  }
  *out_vx = engine->velocity_x;
  *out_vy = engine->velocity_y;
  return 0;
}

int m3_event_set_focus(m3_event_engine_t *engine, cmp_ui_node_t *node) {
  if (!engine) {
    return 1;
  }
  engine->focused_node = node;
  return 0;
}
