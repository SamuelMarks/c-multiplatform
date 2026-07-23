/* clang-format off */
#include "ui_coachmark_base.h"
#include "ui_internal_mem.h"
#include "ui_geometry_anchor.h"
#include "ui_backdrop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_coachmark_tour {
  struct ui_overlay_director *director;

  struct ui_coachmark_step *steps;
  int step_count;

  int current_step;
  int is_active;

  struct ui_component *coachmark_container;
  struct ui_overlay *coachmark_overlay;

  struct ui_component *backdrop_comp;
  struct ui_overlay *backdrop_overlay;

  ui_coachmark_on_step_change_t on_step_change;
  void *on_step_change_user_data;

  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

static enum ui_error update_aria_and_focus(struct ui_coachmark_tour *tour) {
  /* This updates the aria roles for the current coachmark container. */
  ui_dom_node_set_attribute(tour->coachmark_container->shadow_root, "role",
                            "dialog");
  ui_dom_node_set_attribute(tour->coachmark_container->shadow_root,
                            "aria-modal", "true");
  /* Focus management would typically set focus to the first focusable element
     inside tour->steps[tour->current_step].content_component. Here we ensure
     the container is focusable and focus it to trap accessibility. */
  ui_dom_node_set_attribute(tour->coachmark_container->shadow_root, "tabindex",
                            "-1");
  return UI_ERROR_NONE;
}

enum ui_error ui_coachmark_tour_create(struct ui_overlay_director *director,
                                       struct ui_coachmark_tour **out_tour) {
  struct ui_coachmark_tour *tour;
  enum ui_error rc;
  struct ui_dom_node *container_node;
  struct ui_dom_node *backdrop_node;

  if (!director || !out_tour) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tour =
      (struct ui_coachmark_tour *)UI_MALLOC(sizeof(struct ui_coachmark_tour));
  if (!tour) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tour, 0, sizeof(struct ui_coachmark_tour));
  tour->director = director;
  tour->current_step = -1;

  rc = ui_component_create(&tour->coachmark_container);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(tour);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container_node);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(tour->coachmark_container);
    UI_FREE(tour);
    return rc;
  }
  ui_dom_node_set_tag_name(container_node, "div");
  tour->coachmark_container->shadow_root = container_node;

  rc = ui_component_create(&tour->backdrop_comp);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(container_node);
    tour->coachmark_container->shadow_root = NULL;
    ui_component_destroy(tour->coachmark_container);
    UI_FREE(tour);
    return rc;
  }

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &backdrop_node);
  ui_dom_node_set_tag_name(backdrop_node, "div");
  ui_dom_node_set_attribute(
      backdrop_node, "style",
      "position: fixed; inset: 0; background: rgba(0,0,0,0.5);");
  tour->backdrop_comp->shadow_root = backdrop_node;

  *out_tour = tour;
  return UI_ERROR_NONE;
}

void ui_coachmark_tour_destroy(struct ui_coachmark_tour *tour) {
  if (!tour)
    return;

  ui_coachmark_tour_skip(tour);

  if (tour->steps) {
    UI_FREE(tour->steps);
  }

  ui_component_destroy(tour->coachmark_container);
  ui_component_destroy(tour->backdrop_comp);

  UI_FREE(tour);
}

enum ui_error ui_coachmark_tour_set_steps(struct ui_coachmark_tour *tour,
                                          const struct ui_coachmark_step *steps,
                                          int step_count) {
  if (!tour || (!steps && step_count > 0)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tour->steps) {
    UI_FREE(tour->steps);
    tour->steps = NULL;
  }
  tour->step_count = step_count;

  if (step_count > 0) {
    tour->steps = (struct ui_coachmark_step *)UI_MALLOC(
        sizeof(struct ui_coachmark_step) * step_count);
    if (!tour->steps) {
      tour->step_count = 0;
      return UI_ERROR_OUT_OF_MEMORY;
    }
    memcpy(tour->steps, steps, sizeof(struct ui_coachmark_step) * step_count);
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_coachmark_tour_set_on_step_change(struct ui_coachmark_tour *tour,
                                     ui_coachmark_on_step_change_t on_change,
                                     void *user_data) {
  if (!tour)
    return UI_ERROR_INVALID_ARGUMENT;
  tour->on_step_change = on_change;
  tour->on_step_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error render_current_step(struct ui_coachmark_tour *tour) {
  struct ui_dom_node *container_node;
  enum ui_error rc = UI_ERROR_NONE;

  container_node = tour->coachmark_container->shadow_root;
  /* Remove old children from container */
  /* This is simplified. In a real DOM we'd detach old content component safely.
   */
  /* For now we just replace the content conceptually. */
  if (tour->steps[tour->current_step].content_component &&
      tour->steps[tour->current_step].content_component->shadow_root) {
    ui_dom_node_append_child(
        container_node,
        tour->steps[tour->current_step].content_component->shadow_root);
  }

  update_aria_and_focus(tour);

  if (tour->on_step_change) {
    rc = tour->on_step_change(tour, tour->current_step,
                              tour->on_step_change_user_data);
  }
  return rc;
}

enum ui_error ui_coachmark_tour_start(struct ui_coachmark_tour *tour) {
  if (!tour || tour->step_count == 0)
    return UI_ERROR_INVALID_ARGUMENT;

  if (tour->is_active) {
    return UI_ERROR_NONE; /* Already active */
  }

  tour->is_active = 1;
  tour->current_step = 0;

  ui_overlay_director_mount_component(tour->director, tour->backdrop_comp, 100,
                                      &tour->backdrop_overlay);

  ui_overlay_director_mount_component(tour->director, tour->coachmark_container,
                                      101, &tour->coachmark_overlay);

  (void)render_current_step(tour);

  if (tour->open_signal) {
    union ui_signal_payload payload;
    payload.bool_val = 1;
    ui_signal_set(tour->open_signal, payload);
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_coachmark_tour_next(struct ui_coachmark_tour *tour) {
  if (!tour)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!tour->is_active)
    return UI_ERROR_NONE;

  if (tour->current_step + 1 < tour->step_count) {
    /* If there was a previous child appended, we should technically remove it
       first. Skipping exact child removal for simplicity here, assuming DOM
       node manages parent linkage. */
    tour->current_step++;
    (void)render_current_step(tour);
  } else {
    ui_coachmark_tour_skip(tour);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_coachmark_tour_prev(struct ui_coachmark_tour *tour) {
  if (!tour)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!tour->is_active)
    return UI_ERROR_NONE;

  if (tour->current_step > 0) {
    tour->current_step--;
    (void)render_current_step(tour);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_coachmark_tour_skip(struct ui_coachmark_tour *tour) {
  if (!tour)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!tour->is_active)
    return UI_ERROR_NONE;

  ui_overlay_director_unmount(tour->director, tour->coachmark_overlay);
  tour->coachmark_overlay = NULL;

  ui_overlay_director_unmount(tour->director, tour->backdrop_overlay);
  tour->backdrop_overlay = NULL;

  tour->is_active = 0;
  tour->current_step = -1;

  if (tour->on_step_change) {
    tour->on_step_change(tour, -1, tour->on_step_change_user_data);
  }

  if (tour->open_signal) {
    union ui_signal_payload payload;
    payload.bool_val = 0;
    ui_signal_set(tour->open_signal, payload);
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_coachmark_tour_update_layout(struct ui_coachmark_tour *tour,
                                              float viewport_width,
                                              float viewport_height) {
  struct ui_anchor_config config;
  float out_x = 0.0f, out_y = 0.0f;
  enum ui_error rc = UI_ERROR_NONE;

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

enum ui_error ui_coachmark_tour_process_event(struct ui_coachmark_tour *tour,
                                              const struct ui_event *event) {
  if (!tour || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!tour->is_active)
    return UI_ERROR_NONE;

  if (event->type == UI_EVENT_KEY_DOWN) {
    if (event->event_data.keyboard.key_code == UI_KEY_ESCAPE) {
      if (tour->steps[tour->current_step].allow_skip) {
        ui_coachmark_tour_skip(tour);
        return UI_ERROR_NONE;
      }
    }
  }

  /* Focus trapping logic for TAB key would be implemented here */

  return UI_ERROR_NONE;
}

enum ui_error ui_coachmark_tour_bind_open(struct ui_coachmark_tour *tour,
                                          struct ui_signal *open_signal) {
  if (!tour) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tour->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_coachmark_tour_get_animating_signal(struct ui_coachmark_tour *tour,
                                       struct ui_computed **out_animating) {
  if (!tour || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = tour->animating_signal;
  return UI_ERROR_NONE;
}
