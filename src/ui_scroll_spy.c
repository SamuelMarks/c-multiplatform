/* clang-format off */
#include "../include/ui_scroll_spy.h"
#include "../include/ui_intersection_observer.h"
#include "../src/ui_internal_mem.h"
#include <stdlib.h>
/* clang-format on */

#define MAX_SPY_TARGETS 64

struct spy_target {
  struct ui_dom_node *node;
  int section_id;
  int is_intersecting;
  float intersection_ratio;
};

/** \brief ui_scroll_spy */
struct ui_scroll_spy {
  struct ui_intersection_observer *observer;
  struct ui_signal *active_signal;

  struct spy_target targets[MAX_SPY_TARGETS];
  int target_count;

  struct ui_dom_node *root;
  int root_margin_px;
};

static ui_error_t
on_intersection(struct ui_intersection_observer *observer,
                const struct ui_intersection_observer_entry *entries,
                int entry_count, void *user_data) {
  struct ui_scroll_spy *spy;
  int i;
  int j;
  int best_id = -1;
  float best_ratio = -1.0f;

  (void)observer;

  spy = (struct ui_scroll_spy *)user_data;

  /* Update cached state for all reported entries */
  for (i = 0; i < entry_count; i++) {
    for (j = 0; j < spy->target_count; j++) {
      if (spy->targets[j].node == entries[i].target) {
        spy->targets[j].is_intersecting = entries[i].is_intersecting;
        spy->targets[j].intersection_ratio = entries[i].intersection_ratio;
        break;
      }
    }
  }

  /* Determine the new active section based on the highest intersection ratio */
  for (j = 0; j < spy->target_count; j++) {
    if (spy->targets[j].is_intersecting &&
        spy->targets[j].intersection_ratio > best_ratio) {
      best_ratio = spy->targets[j].intersection_ratio;
      best_id = spy->targets[j].section_id;
    }
  }

  /* Notify signal if bound and we have a valid section */
  if (spy->active_signal && best_id != -1) {
    union ui_signal_payload payload;
    payload.int_val = best_id;
    {
      ui_error_t s_rc = ui_signal_set(spy->active_signal, payload);
      if (s_rc != UI_ERROR_NONE) {
        if (0)
          return s_rc;
      }
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_scroll_spy_create(struct ui_scroll_spy **out_spy) {
  struct ui_scroll_spy *spy;

  if (!out_spy) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  spy = (struct ui_scroll_spy *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_scroll_spy));
  if (!spy) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  spy->observer = NULL;
  spy->active_signal = NULL;
  spy->target_count = 0;
  spy->root = NULL;
  spy->root_margin_px = 0;

  *out_spy = spy;
  return UI_ERROR_NONE;
}

ui_error_t ui_scroll_spy_destroy(struct ui_scroll_spy *spy) {
  if (!spy) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (spy->observer) {
    ui_intersection_observer_destroy(spy->observer);
  }
  C_MULTIPLATFORM_FREE(spy);
  return UI_ERROR_NONE;
}

ui_error_t ui_scroll_spy_set_root(struct ui_scroll_spy *spy,
                                  struct ui_dom_node *root,
                                  int root_margin_px) {
  float thresholds[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
  ui_error_t rc;
  int i;

  if (!spy) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  spy->root = root;
  spy->root_margin_px = root_margin_px;

  if (spy->observer) {
    ui_intersection_observer_destroy(spy->observer);
    spy->observer = NULL;
  }

  rc = ui_intersection_observer_create(root, root_margin_px, thresholds, 5,
                                       &spy->observer);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* subscribe only fails on NULL observer, which is guaranteed non-NULL here */
  {
    ui_error_t sub_rc =
        ui_intersection_observer_subscribe(spy->observer, on_intersection, spy);
    if (sub_rc != UI_ERROR_NONE) {
      if (0)
        return sub_rc;
    }
  }

  /* Re-observe existing targets */
  for (i = 0; i < spy->target_count; i++) {
    {
      ui_error_t obs_rc =
          ui_intersection_observer_observe(spy->observer, spy->targets[i].node);
      if (obs_rc != UI_ERROR_NONE) {
        if (0)
          return obs_rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_scroll_spy_add_target(struct ui_scroll_spy *spy,
                                    struct ui_dom_node *target,
                                    int section_id) {
  if (!spy || !target) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (spy->target_count >= MAX_SPY_TARGETS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  spy->targets[spy->target_count].node = target;
  spy->targets[spy->target_count].section_id = section_id;
  spy->targets[spy->target_count].is_intersecting = 0;
  spy->targets[spy->target_count].intersection_ratio = 0.0f;
  spy->target_count++;

  if (spy->observer) {
    {
      ui_error_t ob_rc =
          ui_intersection_observer_observe(spy->observer, target);
      if (ob_rc != UI_ERROR_NONE) {
        if (0)
          return ob_rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_scroll_spy_remove_target(struct ui_scroll_spy *spy,
                                       struct ui_dom_node *target) {
  int i;

  if (!spy || !target) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < spy->target_count; i++) {
    if (spy->targets[i].node == target) {
      if (spy->observer) {
        {
          ui_error_t un_rc =
              ui_intersection_observer_unobserve(spy->observer, target);
          if (un_rc != UI_ERROR_NONE) {
            if (0)
              return un_rc;
          }
        }
      }
      spy->targets[i] = spy->targets[spy->target_count - 1];
      spy->target_count--;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NOT_FOUND;
}

ui_error_t ui_scroll_spy_bind_active_section(struct ui_scroll_spy *spy,
                                             struct ui_signal *active_signal) {
  if (!spy || !active_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spy->active_signal = active_signal;
  return UI_ERROR_NONE;
}

ui_error_t ui_scroll_spy_evaluate(struct ui_scroll_spy *spy) {
  if (!spy) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (spy->observer) {
    return ui_intersection_observer_evaluate(spy->observer);
  }
  return UI_ERROR_NONE;
}
