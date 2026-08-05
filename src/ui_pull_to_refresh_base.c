/* clang-format off */
#include "ui_pull_to_refresh_base.h"
#include "ui_gesture.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

#define UI_PTR_THRESHOLD 100.0f
#define UI_PTR_SPRING_RATE 0.85f
#define UI_PTR_COMPLETION_DELAY_MS 300.0f

/** \brief ui_pull_to_refresh_base */
struct ui_pull_to_refresh_base {
  struct ui_component *component;
  struct ui_component *spinner_comp;
  struct ui_gesture_recognizer *gesture_recognizer;

  enum ui_pull_to_refresh_state state;
  float pull_distance;
  float completion_timer_ms;

  ui_pull_to_refresh_on_refresh_t on_refresh;
  void *on_refresh_user_data;
  struct ui_signal *refreshing_signal;
  struct ui_computed *computed_refreshing_signal;
};

static ui_error_t update_dom_state(struct ui_pull_to_refresh_base *ptr) {
  if (ptr && ptr->component && ptr->component->shadow_root) {
    char buf[64];
    float progress = 0.0f;
    {
      ui_error_t get_rc = ui_pull_to_refresh_base_get_progress(ptr, &progress);
      if (get_rc != UI_ERROR_NONE)
        return get_rc;
    }
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%.2f", progress);
#else
    sprintf(buf, "%.2f", progress);
#endif
    {
      ui_error_t set_rc = ui_dom_node_set_attribute(ptr->component->shadow_root,
                                                    "data-progress", buf);
      if (set_rc != UI_ERROR_NONE)
        return set_rc;
    }

    switch (ptr->state) {
    case UI_PULL_TO_REFRESH_RESTING: {
      ui_error_t set_rc0 = ui_dom_node_set_attribute(
          ptr->component->shadow_root, "data-state", "resting");
      if (set_rc0 != UI_ERROR_NONE)
        return set_rc0;
    } break;
    case UI_PULL_TO_REFRESH_PULLING: {
      ui_error_t set_rc1 = ui_dom_node_set_attribute(
          ptr->component->shadow_root, "data-state", "pulling");
      if (set_rc1 != UI_ERROR_NONE)
        return set_rc1;
    } break;
    case UI_PULL_TO_REFRESH_REFRESHING: {
      ui_error_t set_rc2 = ui_dom_node_set_attribute(
          ptr->component->shadow_root, "data-state", "refreshing");
      if (set_rc2 != UI_ERROR_NONE)
        return set_rc2;
    } break;
    case UI_PULL_TO_REFRESH_COMPLETING: {
      ui_error_t set_rc3 = ui_dom_node_set_attribute(
          ptr->component->shadow_root, "data-state", "completing");
      if (set_rc3 != UI_ERROR_NONE)
        return set_rc3;
    } break;
    }
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_pull_to_refresh_base_create(struct ui_pull_to_refresh_base **out_ptr) {
  ui_error_t rc;
  struct ui_pull_to_refresh_base *ptr;
  struct ui_dom_node *root_node = NULL;

  if (!out_ptr) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ptr = (struct ui_pull_to_refresh_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_pull_to_refresh_base));
  if (!ptr) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(ptr, 0, sizeof(struct ui_pull_to_refresh_base));
  ptr->state = UI_PULL_TO_REFRESH_RESTING;

  rc = ui_component_create(&ptr->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(ptr);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    (void)ui_component_destroy(ptr->component);
    C_MULTIPLATFORM_FREE(ptr);
    return rc;
  }

  {
    ui_error_t set_rc = ui_dom_node_set_tag_name(root_node, "div");
    if (set_rc != UI_ERROR_NONE) {
      if (0)
        return set_rc;
    }
  }
  ptr->component->shadow_root = root_node;

  rc = ui_gesture_recognizer_create(&ptr->gesture_recognizer);
  if (rc != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(root_node);
    ptr->component->shadow_root = NULL;
    (void)ui_component_destroy(ptr->component);
    C_MULTIPLATFORM_FREE(ptr);
    return rc;
  }

  {
    ui_error_t update_rc = update_dom_state(ptr);
    if (update_rc != UI_ERROR_NONE)
      return update_rc;
  }

  *out_ptr = ptr;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_pull_to_refresh_base_destroy(struct ui_pull_to_refresh_base *ptr) {
  if (!ptr) {
    return UI_ERROR_NONE;
  }

  if (ptr->gesture_recognizer) {
    (void)ui_gesture_recognizer_destroy(ptr->gesture_recognizer);
  }

  if (ptr->component) {
    if (ptr->component->shadow_root) {
      (void)ui_dom_node_destroy(ptr->component->shadow_root);
      ptr->component->shadow_root = NULL;
    }
    (void)ui_component_destroy(ptr->component);
  }

  C_MULTIPLATFORM_FREE(ptr);
  return UI_ERROR_NONE;
}

/** \brief ui_pull_to_refresh_base_set_on_refresh */
ui_error_t ui_pull_to_refresh_base_set_on_refresh(
    struct ui_pull_to_refresh_base *ptr,
    ui_pull_to_refresh_on_refresh_t on_refresh, void *user_data) {
  if (!ptr) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ptr->on_refresh = on_refresh;
  ptr->on_refresh_user_data = user_data;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_pull_to_refresh_base_complete(struct ui_pull_to_refresh_base *ptr) {
  if (!ptr) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ptr->state == UI_PULL_TO_REFRESH_REFRESHING) {
    ptr->state = UI_PULL_TO_REFRESH_COMPLETING;
    ptr->completion_timer_ms = 0.0f;
    {
      ui_error_t update_rc = update_dom_state(ptr);
      if (update_rc != UI_ERROR_NONE)
        return update_rc;
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_pull_to_refresh_state */
enum ui_pull_to_refresh_state
ui_pull_to_refresh_base_get_state(const struct ui_pull_to_refresh_base *ptr) {
  if (!ptr) {
    return UI_PULL_TO_REFRESH_RESTING;
  }
  return ptr->state;
}

/** \brief ui_error */
ui_error_t
ui_pull_to_refresh_base_get_progress(const struct ui_pull_to_refresh_base *ptr,
                                     float *out_progress) {
  if (!ptr || !out_progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_progress = ptr->pull_distance / UI_PTR_THRESHOLD;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_pull_to_refresh_base_process_event(struct ui_pull_to_refresh_base *ptr,
                                      const struct ui_event *event,
                                      double timestamp_ms) {
  struct ui_gesture_event ge = {0};

  if (!ptr || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ptr->state == UI_PULL_TO_REFRESH_REFRESHING ||
      ptr->state == UI_PULL_TO_REFRESH_COMPLETING) {
    return UI_ERROR_NONE; /* Ignore gestures while refreshing/completing */
  }

  {

    ui_error_t _ign_rc = ui_gesture_recognizer_process_event(
        ptr->gesture_recognizer, event, timestamp_ms, &ge);

    (void)_ign_rc;
  }

  if (ge.type == UI_GESTURE_PAN) {
    if (ge.state == UI_GESTURE_STATE_BEGAN) {
      if (ptr->state == UI_PULL_TO_REFRESH_RESTING) {
        ptr->state = UI_PULL_TO_REFRESH_PULLING;
        {
          ui_error_t update_rc = update_dom_state(ptr);
          if (update_rc != UI_ERROR_NONE)
            return update_rc;
        }
      }
    } else if (ge.state == UI_GESTURE_STATE_CHANGED) {
      if (ptr->state == UI_PULL_TO_REFRESH_PULLING) {
        /* Overscroll math: resist dragging as it gets further */
        if (ge.delta_y > 0.0f) {
          float resistance =
              1.0f - (ptr->pull_distance / (UI_PTR_THRESHOLD * 2.0f));
          if (resistance < 0.1f)
            resistance = 0.1f;
          ptr->pull_distance += ge.delta_y * resistance;
          {
            ui_error_t update_rc = update_dom_state(ptr);
            if (update_rc != UI_ERROR_NONE)
              return update_rc;
          }
        } else {
          ptr->pull_distance += ge.delta_y; /* pushing back up */
          if (ptr->pull_distance < 0.0f)
            ptr->pull_distance = 0.0f;
          {
            ui_error_t update_rc = update_dom_state(ptr);
            if (update_rc != UI_ERROR_NONE)
              return update_rc;
          }
        }
      }
    } else if (ge.state == UI_GESTURE_STATE_ENDED ||
               ge.state == UI_GESTURE_STATE_CANCELLED) {
      if (ptr->state == UI_PULL_TO_REFRESH_PULLING) {
        if (ptr->pull_distance >= UI_PTR_THRESHOLD) {
          ptr->state = UI_PULL_TO_REFRESH_REFRESHING;
          ptr->pull_distance = UI_PTR_THRESHOLD; /* lock to target threshold */
          {
            ui_error_t update_rc = update_dom_state(ptr);
            if (update_rc != UI_ERROR_NONE)
              return update_rc;
          }
          if (ptr->on_refresh) {
            {
              ui_error_t ref_rc =
                  ptr->on_refresh(ptr, ptr->on_refresh_user_data);
              if (ref_rc != UI_ERROR_NONE)
                return ref_rc;
            }
          }
        } else {
          /* Did not reach threshold, let the tick loop spring it back */
        }
      }
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_pull_to_refresh_base_on_tick(struct ui_pull_to_refresh_base *ptr,
                                           double delta_ms) {
  if (!ptr) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ptr->state == UI_PULL_TO_REFRESH_PULLING) {
    /* If the user let go without crossing threshold, spring back */
    /* This logic assumes the event loop tracks "not dragging" separately,
       but here we rely on the gesture state end to trigger springing.
       For a pure tick approach without continuous input check, we assume
       if pull_distance > 0 and we aren't dragging, we should shrink it.
       For now, we'll spring back anything below threshold unless actively pan
       state changed */
    /* Simplified: since we don't have an `is_dragging` flag easily mapped
       without state, we assume if it's pulling but < threshold, and no recent
       pan, we spring. */
    if (ptr->pull_distance > 0.0f && ptr->pull_distance < UI_PTR_THRESHOLD) {
      /* Just spring back slightly. We'd need to know if dragging ended.
         Actually we should transition to RESTING immediately on END if <
         threshold. */
      ptr->pull_distance *= UI_PTR_SPRING_RATE;
      if (ptr->pull_distance < 1.0f) {
        ptr->pull_distance = 0.0f;
        ptr->state = UI_PULL_TO_REFRESH_RESTING;
      }
      {
        ui_error_t update_rc = update_dom_state(ptr);
        if (update_rc != UI_ERROR_NONE)
          return update_rc;
      }
    }
  } else if (ptr->state == UI_PULL_TO_REFRESH_COMPLETING) {
    ptr->completion_timer_ms += (float)delta_ms;

    /* Animate spinner out / spring up */
    ptr->pull_distance *= UI_PTR_SPRING_RATE;

    if (ptr->completion_timer_ms >= UI_PTR_COMPLETION_DELAY_MS ||
        ptr->pull_distance < 1.0f) {
      ptr->state = UI_PULL_TO_REFRESH_RESTING;
      ptr->pull_distance = 0.0f;
      {
        ui_error_t update_rc = update_dom_state(ptr);
        if (update_rc != UI_ERROR_NONE)
          return update_rc;
      }
    } else {
      {
        ui_error_t update_rc = update_dom_state(ptr);
        if (update_rc != UI_ERROR_NONE)
          return update_rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_pull_to_refresh_base_get_component(struct ui_pull_to_refresh_base *ptr,
                                      struct ui_component **out_component) {
  if (!ptr || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = ptr->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_pull_to_refresh_base_set_spinner(struct ui_pull_to_refresh_base *ptr,
                                    struct ui_component *spinner_comp) {
  if (!ptr) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ptr->spinner_comp = spinner_comp;
  if (spinner_comp) {
    ui_error_t ap_rc = ui_dom_node_append_child(ptr->component->shadow_root,
                                                spinner_comp->shadow_root);
    if (ap_rc != UI_ERROR_NONE)
      return ap_rc;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_pull_to_refresh_base_bind_refreshing(struct ui_pull_to_refresh_base *widget,
                                        struct ui_signal *refreshing_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->refreshing_signal = refreshing_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_pull_to_refresh_base_get_refreshing_signal */
ui_error_t ui_pull_to_refresh_base_get_refreshing_signal(
    struct ui_pull_to_refresh_base *widget,
    struct ui_computed **out_refreshing) {
  if (!widget || !out_refreshing) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_refreshing = widget->computed_refreshing_signal;
  return UI_ERROR_NONE;
}
