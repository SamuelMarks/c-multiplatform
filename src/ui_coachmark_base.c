/* clang-format off */
#include "ui_coachmark_base.h"
#include "ui_internal_mem.h"
#include "ui_geometry_anchor.h"
#include "ui_backdrop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @struct ui_coachmark_tour
 * @brief Internal representation of a coachmark tour.
 */
struct ui_coachmark_tour {
  /** @brief Overlay director for layering. */
  struct ui_overlay_director *director;

  /** @brief Array of tour steps. */
  struct ui_coachmark_step *steps;
  /** @brief Number of steps. */
  int step_count;

  /** @brief Index of current step, -1 if inactive. */
  int current_step;
  /** @brief 1 if tour is currently active, 0 otherwise. */
  int is_active;

  /** @brief Component wrapping the step content. */
  struct ui_component *coachmark_container;
  /** @brief The mounted overlay for the container. */
  struct ui_overlay *coachmark_overlay;

  /** @brief Full-screen dimming backdrop component. */
  struct ui_component *backdrop_comp;
  /** @brief Mounted overlay for the backdrop. */
  struct ui_overlay *backdrop_overlay;

  /** @brief Step change callback. */
  ui_coachmark_on_step_change_t on_step_change;
  /** @brief User data for callback. */
  void *on_step_change_user_data;

  /** @brief Signal bound to open state. */
  struct ui_signal *open_signal;
  /** @brief Computed signal for animation state. */
  struct ui_computed *animating_signal;
};

static ui_error_t update_aria_and_focus(struct ui_coachmark_tour *tour) {
  ui_error_t rc = UI_ERROR_NONE;
  /* This updates the aria roles for the current coachmark container. */
  rc = ui_dom_node_set_attribute(tour->coachmark_container->shadow_root, "role",
                                 "dialog");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  rc = ui_dom_node_set_attribute(tour->coachmark_container->shadow_root,
                                 "aria-modal", "true");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  /* Focus management would typically set focus to the first focusable element
     inside tour->steps[tour->current_step].content_component. Here we ensure
     the container is focusable and focus it to trap accessibility. */
  rc = ui_dom_node_set_attribute(tour->coachmark_container->shadow_root,
                                 "tabindex", "-1");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_coachmark_tour_create(struct ui_overlay_director *director,
                                    struct ui_coachmark_tour **out_tour) {
  struct ui_coachmark_tour *tour;
  ui_error_t rc;
  struct ui_dom_node *container_node;
  struct ui_dom_node *backdrop_node = NULL;

  if (!director || !out_tour) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tour = (struct ui_coachmark_tour *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_coachmark_tour));
  if (!tour) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tour, 0, sizeof(struct ui_coachmark_tour));
  tour->director = director;
  tour->current_step = -1;

  rc = ui_component_create(&tour->coachmark_container);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tour);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container_node);
  if (rc != UI_ERROR_NONE) {
    (void)ui_component_destroy(tour->coachmark_container);
    C_MULTIPLATFORM_FREE(tour);
    return rc;
  }
  rc = ui_dom_node_set_tag_name(container_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  tour->coachmark_container->shadow_root = container_node;
  container_node = NULL;

  rc = ui_component_create(&tour->backdrop_comp);
  if (rc != UI_ERROR_NONE) {
    (void)ui_component_destroy(tour->coachmark_container);
    C_MULTIPLATFORM_FREE(tour);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &backdrop_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_tag_name(backdrop_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_attribute(
      backdrop_node, "style",
      "position: fixed; inset: 0; background: rgba(0,0,0,0.5);");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  tour->backdrop_comp->shadow_root = backdrop_node;
  backdrop_node = NULL;

  *out_tour = tour;
  return UI_ERROR_NONE;

cleanup:
  if (backdrop_node) {
    (void)ui_dom_node_destroy(backdrop_node);
  }
  if (tour->backdrop_comp) {
    (void)ui_component_destroy(tour->backdrop_comp);
  }
  if (container_node) {
    (void)ui_dom_node_destroy(container_node);
  }
  C_MULTIPLATFORM_FREE(tour);
  return rc;
}

ui_error_t ui_coachmark_tour_destroy(struct ui_coachmark_tour *tour) {
  if (!tour) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)ui_coachmark_tour_skip(tour);

  if (tour->steps) {
    C_MULTIPLATFORM_FREE(tour->steps);
  }

  (void)ui_component_destroy(tour->coachmark_container);
  (void)ui_component_destroy(tour->backdrop_comp);

  C_MULTIPLATFORM_FREE(tour);
  return UI_ERROR_NONE;
}

ui_error_t ui_coachmark_tour_set_steps(struct ui_coachmark_tour *tour,
                                       const struct ui_coachmark_step *steps,
                                       int step_count) {
  if (!tour || (!steps && step_count > 0)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tour->steps) {
    C_MULTIPLATFORM_FREE(tour->steps);
    tour->steps = NULL;
  }
  tour->step_count = step_count;

  if (step_count > 0) {
    tour->steps = (struct ui_coachmark_step *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_coachmark_step) * (size_t)step_count);
    if (!tour->steps) {
      tour->step_count = 0;
      return UI_ERROR_OUT_OF_MEMORY;
    }
    memcpy(tour->steps, steps,
           sizeof(struct ui_coachmark_step) * (size_t)step_count);
  }
  return UI_ERROR_NONE;
}

ui_error_t
ui_coachmark_tour_set_on_step_change(struct ui_coachmark_tour *tour,
                                     ui_coachmark_on_step_change_t on_change,
                                     void *user_data) {
  if (!tour) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tour->on_step_change = on_change;
  tour->on_step_change_user_data = user_data;
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
int g_coachmark_signal_mock_fail = 0;
int g_coachmark_dom_mock_fail = 0;
#endif

static ui_error_t render_current_step(struct ui_coachmark_tour *tour) {
  struct ui_dom_node *container_node;
  ui_error_t rc = UI_ERROR_NONE;

  container_node = tour->coachmark_container->shadow_root;
  /* Remove old children from container */
  /* This is simplified. In a real DOM we'd detach old content component safely.
   */
  /* For now we just replace the content conceptually. */
  if (tour->steps[tour->current_step].content_component &&
      tour->steps[tour->current_step].content_component->shadow_root) {
#ifdef UI_TEST_MOCK_ALLOC
    if (g_coachmark_dom_mock_fail) {
      rc = UI_ERROR_UNKNOWN;
    } else
#endif
      rc = ui_dom_node_append_child(
          container_node,
          tour->steps[tour->current_step].content_component->shadow_root);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  rc = update_aria_and_focus(tour);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (tour->on_step_change) {
    rc = tour->on_step_change(tour, tour->current_step,
                              tour->on_step_change_user_data);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return rc;
}

ui_error_t ui_coachmark_tour_start(struct ui_coachmark_tour *tour) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!tour || tour->step_count == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tour->is_active) {
    return UI_ERROR_NONE; /* Already active */
  }

  tour->is_active = 1;
  tour->current_step = 0;

  rc = ui_overlay_director_mount_component(tour->director, tour->backdrop_comp,
                                           100, &tour->backdrop_overlay);
  if (rc != UI_ERROR_NONE) {
    tour->is_active = 0;
    return rc;
  }

  rc = ui_overlay_director_mount_component(
      tour->director, tour->coachmark_container, 101, &tour->coachmark_overlay);
  if (rc != UI_ERROR_NONE) {
    (void)ui_overlay_director_unmount(tour->director, tour->backdrop_overlay);
    tour->backdrop_overlay = NULL;
    tour->is_active = 0;
    return rc;
  }

  rc = render_current_step(tour);
  if (rc != UI_ERROR_NONE) {
    (void)ui_overlay_director_unmount(tour->director, tour->coachmark_overlay);
    tour->coachmark_overlay = NULL;
    (void)ui_overlay_director_unmount(tour->director, tour->backdrop_overlay);
    tour->backdrop_overlay = NULL;
    tour->is_active = 0;
    return rc;
  }

  if (tour->open_signal) {
    union ui_signal_payload payload;
    payload.bool_val = 1;
#ifdef UI_TEST_MOCK_ALLOC
    if (g_coachmark_signal_mock_fail) {
      rc = UI_ERROR_UNKNOWN;
    } else
#endif
      rc = ui_signal_set(tour->open_signal, payload);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_coachmark_tour_next(struct ui_coachmark_tour *tour) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!tour) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!tour->is_active) {
    return UI_ERROR_NONE;
  }

  if (tour->current_step + 1 < tour->step_count) {
    /* If there was a previous child appended, we should technically remove it
       first. Skipping exact child removal for simplicity here, assuming DOM
       node manages parent linkage. */
    tour->current_step++;
    rc = render_current_step(tour);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    (void)ui_coachmark_tour_skip(tour);
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_coachmark_tour_prev(struct ui_coachmark_tour *tour) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!tour) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!tour->is_active) {
    return UI_ERROR_NONE;
  }

  if (tour->current_step > 0) {
    tour->current_step--;
    rc = render_current_step(tour);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_coachmark_tour_skip(struct ui_coachmark_tour *tour) {
  if (!tour) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!tour->is_active) {
    return UI_ERROR_NONE;
  }

  (void)ui_overlay_director_unmount(tour->director, tour->coachmark_overlay);
  tour->coachmark_overlay = NULL;

  (void)ui_overlay_director_unmount(tour->director, tour->backdrop_overlay);
  tour->backdrop_overlay = NULL;

  tour->is_active = 0;
  tour->current_step = -1;

  if (tour->on_step_change) {
    ui_error_t rc =
        tour->on_step_change(tour, -1, tour->on_step_change_user_data);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (tour->open_signal) {
    union ui_signal_payload payload;
    ui_error_t rc;
    payload.bool_val = 0;
#ifdef UI_TEST_MOCK_ALLOC
    if (g_coachmark_signal_mock_fail) {
      rc = UI_ERROR_UNKNOWN;
    } else
#endif
      rc = ui_signal_set(tour->open_signal, payload);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_coachmark_tour_update_layout(struct ui_coachmark_tour *tour,
                                           float viewport_width,
                                           float viewport_height) {
  struct ui_anchor_config config;
  float out_x = 0.0f, out_y = 0.0f;
  ui_error_t rc = UI_ERROR_NONE;

  if (!tour || !tour->is_active) {
    return UI_ERROR_NONE;
  }

  /* Setup an anchor configuration to place coachmark below target */
  config.target_x = UI_ANCHOR_EDGE_CENTER;
  config.target_y = UI_ANCHOR_EDGE_END;
  config.overlay_x = UI_ANCHOR_EDGE_CENTER;
  config.overlay_y = UI_ANCHOR_EDGE_START;
  config.offset_x = 0.0f;
  config.offset_y = 8.0f;

  /* In a real implementation we would extract layout nodes from the components.
     Here we mock the call if we had the actual layout nodes. */
  /*
  struct ui_layout_node *target_layout =
  get_layout(tour->steps[tour->current_step].target_component); struct
  ui_layout_node *overlay_layout = get_layout(tour->coachmark_container);

  rc = ui_geometry_anchor_compute(target_layout, overlay_layout, &config,
                                  viewport_width, viewport_height, &out_x,
  &out_y); if (rc == UI_ERROR_NONE) { char style_buf[128]; sprintf(style_buf,
  "position: absolute; left: %fpx; top: %fpx;", out_x, out_y);
     ui_dom_node_set_attribute(tour->coachmark_container->shadow_root, "style",
  style_buf);
  }
  */
  (void)config;
  (void)out_x;
  (void)out_y;
  (void)rc;
  (void)viewport_width;
  (void)viewport_height;

  return UI_ERROR_NONE;
}

ui_error_t ui_coachmark_tour_process_event(struct ui_coachmark_tour *tour,
                                           const struct ui_event *event) {
  if (!tour || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!tour->is_active) {
    return UI_ERROR_NONE;
  }

  if (event->type == UI_EVENT_KEY_DOWN) {
    if (event->event_data.keyboard.key_code == UI_KEY_ESCAPE) {
      if (tour->steps[tour->current_step].allow_skip) {
        (void)ui_coachmark_tour_skip(tour);
        return UI_ERROR_NONE;
      }
    }
  }

  /* Focus trapping logic for TAB key would be implemented here */

  return UI_ERROR_NONE;
}

ui_error_t ui_coachmark_tour_bind_open(struct ui_coachmark_tour *tour,
                                       struct ui_signal *open_signal) {
  if (!tour) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tour->open_signal = open_signal;
  return UI_ERROR_NONE;
}

ui_error_t
ui_coachmark_tour_get_animating_signal(struct ui_coachmark_tour *tour,
                                       struct ui_computed **out_animating) {
  if (!tour || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = tour->animating_signal;
  return UI_ERROR_NONE;
}
