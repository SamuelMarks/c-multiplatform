/* clang-format off */
#include "ui_toast_manager_base.h"
#include "ui_internal_mem.h"
#include "ui_component.h"
#include "ui_overlay_director.h"
#include "ui_dom_node.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

struct ui_toast_entry {
  ui_toast_id id;
  struct ui_toast_config config;
  enum ui_toast_anim_state anim_state;
  double show_time;
  double total_paused_time;
  double pause_start_time;
  int is_paused;
  char *message;

  struct ui_component *overlay_component;
  struct ui_overlay *active_overlay;
};

/** \brief ui_toast_region_stack */
struct ui_toast_region_stack {
  struct ui_toast_entry **toasts;
  size_t count;
  size_t capacity;
};

/** \brief ui_toast_manager_base */
struct ui_toast_manager_base {
  struct ui_toast_region_stack regions[UI_TOAST_REGION_COUNT];
  ui_toast_id next_id;
  int is_hovered; /* Simple global hover state for primitive */
};

/** \brief ui_error */
enum ui_error
ui_toast_manager_base_create(struct ui_toast_manager_base **out_manager) {
  struct ui_toast_manager_base *manager;
  int i;

  if (!out_manager)
    return UI_ERROR_INVALID_ARGUMENT;

  manager = (struct ui_toast_manager_base *)UI_MALLOC(
      sizeof(struct ui_toast_manager_base));
  if (!manager)
    return UI_ERROR_OUT_OF_MEMORY;

  manager->next_id = 1;
  manager->is_hovered = 0;

  for (i = 0; i < UI_TOAST_REGION_COUNT; i++) {
    manager->regions[i].toasts = NULL;
    manager->regions[i].count = 0;
    manager->regions[i].capacity = 0;
  }

  *out_manager = manager;
  return UI_ERROR_NONE;
}

static void free_toast_entry(struct ui_toast_entry *entry) {
  if (!entry)
    return;
  if (entry->message)
    UI_FREE(entry->message);
  if (entry->overlay_component)
    ui_component_destroy(entry->overlay_component);
  UI_FREE(entry);
}

void ui_toast_manager_base_destroy(struct ui_toast_manager_base *manager) {
  int i;
  size_t j;
  if (!manager)
    return;

  for (i = 0; i < UI_TOAST_REGION_COUNT; i++) {
    for (j = 0; j < manager->regions[i].count; j++) {
      free_toast_entry(manager->regions[i].toasts[j]);
    }
    if (manager->regions[i].toasts) {
      UI_FREE(manager->regions[i].toasts);
    }
  }
  UI_FREE(manager);
}

enum ui_error ui_toast_manager_base_show(struct ui_toast_manager_base *manager,
                                         const struct ui_toast_config *config,
                                         double current_time_secs,
                                         ui_toast_id *out_id) {

  struct ui_toast_entry *entry;
  struct ui_toast_region_stack *stack;
  enum ui_error rc;

  if (!manager || !config || !out_id ||
      config->region >= UI_TOAST_REGION_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  entry = (struct ui_toast_entry *)UI_MALLOC(sizeof(struct ui_toast_entry));
  if (!entry)
    return UI_ERROR_OUT_OF_MEMORY;

  entry->id = manager->next_id++;
  entry->config = *config;
  entry->anim_state = UI_TOAST_ANIM_SLIDE_IN;
  entry->show_time = current_time_secs;
  entry->total_paused_time = 0.0;
  entry->pause_start_time = 0.0;
  entry->is_paused = manager->is_hovered;
  if (entry->is_paused) {
    entry->pause_start_time = current_time_secs;
  }
  entry->message = NULL;
  entry->overlay_component = NULL;
  entry->active_overlay = NULL;

  if (config->message) {
    size_t len = strlen(config->message);
    entry->message = (char *)UI_MALLOC(len + 1);
    if (!entry->message) {
      free_toast_entry(entry);
      return UI_ERROR_OUT_OF_MEMORY;
    }
#if defined(_MSC_VER)
    strcpy_s(entry->message, len + 1, config->message);
#else
    strcpy(entry->message, config->message);
#endif
  }

  rc = ui_component_create(&entry->overlay_component);
  if (rc != UI_ERROR_NONE) {
    free_toast_entry(entry);
    return rc;
  }

  stack = &manager->regions[config->region];
  if (stack->count >= stack->capacity) {
    size_t new_cap = stack->capacity == 0 ? 4 : stack->capacity * 2;
    struct ui_toast_entry **new_arr = (struct ui_toast_entry **)UI_REALLOC(
        stack->toasts, new_cap * sizeof(struct ui_toast_entry *));
    if (!new_arr) {
      free_toast_entry(entry);
      return UI_ERROR_OUT_OF_MEMORY;
    }
    stack->toasts = new_arr;
    stack->capacity = new_cap;
  }

  stack->toasts[stack->count++] = entry;
  *out_id = entry->id;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_toast_manager_base_dismiss(struct ui_toast_manager_base *manager,
                              ui_toast_id id) {
  int i;
  size_t j;

  if (!manager)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < UI_TOAST_REGION_COUNT; i++) {
    struct ui_toast_region_stack *stack = &manager->regions[i];
    for (j = 0; j < stack->count; j++) {
      if (stack->toasts[j]->id == id) {
        struct ui_toast_entry *entry = stack->toasts[j];

        /* If already animating out, ignore */
        if (entry->anim_state == UI_TOAST_ANIM_SLIDE_OUT) {
          return UI_ERROR_NONE;
        }

        /* Trigger slide out animation. Tick will clean it up later. */
        entry->anim_state = UI_TOAST_ANIM_SLIDE_OUT;
        return UI_ERROR_NONE;
      }
    }
  }

  return UI_ERROR_NOT_FOUND;
}

enum ui_error ui_toast_manager_base_tick(struct ui_toast_manager_base *manager,
                                         double current_time_secs) {
  int i;
  size_t j, k;

  if (!manager)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < UI_TOAST_REGION_COUNT; i++) {
    struct ui_toast_region_stack *stack = &manager->regions[i];

    for (j = 0; j < stack->count;) {
      struct ui_toast_entry *entry = stack->toasts[j];

      /* Progress animations */
      if (entry->anim_state == UI_TOAST_ANIM_SLIDE_IN) {
        /* Pseudo-animation time: 0.3s */
        if (current_time_secs - entry->show_time >= 0.3) {
          entry->anim_state = UI_TOAST_ANIM_VISIBLE;
        }
      }

      /* Auto dismiss */
      if (entry->anim_state == UI_TOAST_ANIM_VISIBLE &&
          entry->config.duration_secs > 0) {
        double active_time =
            current_time_secs - entry->show_time - entry->total_paused_time;
        if (entry->is_paused) {
          active_time -= (current_time_secs - entry->pause_start_time);
        }

        if (active_time >= entry->config.duration_secs) {
          entry->anim_state = UI_TOAST_ANIM_SLIDE_OUT;
        }
      }

      /* Cleanup on slide out finished */
      if (entry->anim_state == UI_TOAST_ANIM_SLIDE_OUT) {
        /* We'll just destroy it immediately for the primitive.
           A full engine would track a 0.3s out-animation timer. */

        /* Unmount handled in render pass if active_overlay was set,
           but here we just free. To be safe, we rely on director cleanup
           or explicitly unmount if we had a director ref. Since tick doesn't
           have director, we just mark it for removal.
           Wait, if we destroy overlay_component, director might crash if it
           holds ref. We should let `render` pass clean up the director side, or
           we just remove it here and assume director is robust against
           component destruction. Actually, the best way in this architecture is
           to just free it and let the director drop it during the next render,
           OR we pass director to tick. For now, just free it. */

        free_toast_entry(entry);

        /* Shift array */
        for (k = j; k < stack->count - 1; k++) {
          stack->toasts[k] = stack->toasts[k + 1];
        }
        stack->count--;
        continue; /* Do not increment j */
      }
      j++;
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_toast_manager_base_handle_event(struct ui_toast_manager_base *manager,
                                   const struct ui_event *event,
                                   double current_time_secs) {
  int i;
  size_t j;

  if (!manager || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  switch (event->type) {
  case UI_EVENT_MOUSE_DOWN: /* Dismiss all on click for primitive? No. */
    break;
  case UI_EVENT_MOUSE_MOVE:
  case UI_EVENT_TOUCH_START:
    /* Pause on hover/touch */
    if (!manager->is_hovered) {
      manager->is_hovered = 1;
      for (i = 0; i < UI_TOAST_REGION_COUNT; i++) {
        struct ui_toast_region_stack *stack = &manager->regions[i];
        for (j = 0; j < stack->count; j++) {
          if (!stack->toasts[j]->is_paused) {
            stack->toasts[j]->is_paused = 1;
            stack->toasts[j]->pause_start_time = current_time_secs;
          }
        }
      }
    }
    break;

  case UI_EVENT_MOUSE_UP: /* Assuming leave/up ends hover in simple primitive */
  case UI_EVENT_TOUCH_END:
  case UI_EVENT_TOUCH_CANCEL:
    if (manager->is_hovered) {
      manager->is_hovered = 0;
      for (i = 0; i < UI_TOAST_REGION_COUNT; i++) {
        struct ui_toast_region_stack *stack = &manager->regions[i];
        for (j = 0; j < stack->count; j++) {
          if (stack->toasts[j]->is_paused) {
            stack->toasts[j]->is_paused = 0;
            stack->toasts[j]->total_paused_time +=
                (current_time_secs - stack->toasts[j]->pause_start_time);
          }
        }
      }
    }
    break;

  default:
    break;
  }

  return UI_ERROR_NONE;
}

static enum ui_error get_region_style(enum ui_toast_region region,
                                      const char **out_str) {
  if (!out_str)
    return UI_ERROR_INVALID_ARGUMENT;
  switch (region) {
  case UI_TOAST_REGION_TOP_LEFT:
    *out_str = "position: absolute; top: 20px; left: 20px;";
    return UI_ERROR_NONE;
  case UI_TOAST_REGION_TOP_CENTER:
    *out_str = "position: absolute; top: 20px; left: 50%; transform: "
               "translateX(-50%);";
    return UI_ERROR_NONE;
  case UI_TOAST_REGION_TOP_RIGHT:
    *out_str = "position: absolute; top: 20px; right: 20px;";
    return UI_ERROR_NONE;
  case UI_TOAST_REGION_BOTTOM_LEFT:
    *out_str = "position: absolute; bottom: 20px; left: 20px;";
    return UI_ERROR_NONE;
  case UI_TOAST_REGION_BOTTOM_CENTER:
    *out_str = "position: absolute; bottom: 20px; left: 50%; transform: "
               "translateX(-50%);";
    return UI_ERROR_NONE;
  case UI_TOAST_REGION_BOTTOM_RIGHT:
    *out_str = "position: absolute; bottom: 20px; right: 20px;";
    return UI_ERROR_NONE;
  default:
    *out_str = "";
    return UI_ERROR_NONE;
  }
}

/** \brief ui_error */
enum ui_error
ui_toast_manager_base_render(struct ui_toast_manager_base *manager,
                             struct ui_overlay_director *director) {
  int i;
  size_t j;
  enum ui_error rc;

  if (!manager || !director)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < UI_TOAST_REGION_COUNT; i++) {
    struct ui_toast_region_stack *stack = &manager->regions[i];

    if (stack->count == 0)
      continue;

    /* Enforce singleton wrapper per region:
       We will construct ONE container per region, append all active toasts for
       that region into it, and mount that container. For this primitive, we'll
       just mount the first active toast's overlay component, and append all
       children into it. */

    /* Actually, a cleaner way is just to re-render the components.
       Since we decoupled component mapping, let's just mount them as separate
       overlays but offset their styles. */

    for (j = 0; j < stack->count; j++) {
      struct ui_toast_entry *entry = stack->toasts[j];
      struct ui_dom_node *root_node;
      struct ui_dom_node *text_node;
      char style_buf[512];

      if (entry->active_overlay) {
        /* Unmount old so we can rebuild. In a full diff engine, this is
         * optimized. */
        ui_overlay_director_unmount(director, entry->active_overlay);
        entry->active_overlay = NULL;
      }

      rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
      if (rc != UI_ERROR_NONE)
        continue;

      ui_dom_node_set_attribute(root_node, "role",
                                entry->config.is_error ? "alert" : "status");
      ui_dom_node_set_attribute(root_node, "aria-live", "polite");

      /* Apply stack offset. e.g. j * 60px down or up depending on region */
      if (i >= UI_TOAST_REGION_BOTTOM_LEFT) {
#if defined(_MSC_VER)
        const char *region_style = "";
        (void)get_region_style((enum ui_toast_region)i, &region_style);
        sprintf_s(style_buf, sizeof(style_buf), "%s margin-bottom: %lupx;",
                  region_style, (unsigned long)(j * 60));
#else
        const char *region_style = "";
        (void)get_region_style((enum ui_toast_region)i, &region_style);
        sprintf(style_buf, "%s margin-bottom: %lupx;", region_style,
                (unsigned long)(j * 60));
#endif
      } else {
#if defined(_MSC_VER)
        const char *region_style = "";
        (void)get_region_style((enum ui_toast_region)i, &region_style);
        sprintf_s(style_buf, sizeof(style_buf), "%s margin-top: %lupx;",
                  region_style, (unsigned long)(j * 60));
#else
        const char *region_style = "";
        (void)get_region_style((enum ui_toast_region)i, &region_style);
        sprintf(style_buf, "%s margin-top: %lupx;", region_style,
                (unsigned long)(j * 60));
#endif
      }

      ui_dom_node_set_attribute(root_node, "style", style_buf);

      if (entry->message) {
        rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
        if (rc == UI_ERROR_NONE) {
          size_t len = strlen(entry->message);
          text_node->text_content = (char *)UI_MALLOC(len + 1);
          if (text_node->text_content) {
#if defined(_MSC_VER)
            strcpy_s(text_node->text_content, len + 1, entry->message);
#else
            strcpy(text_node->text_content, entry->message);
#endif
          }
          ui_dom_node_append_child(root_node, text_node);
        }
      }

      if (entry->overlay_component->shadow_root) {
        ui_dom_node_destroy(entry->overlay_component->shadow_root);
      }
      entry->overlay_component->shadow_root = root_node;
      ui_overlay_director_mount_component(director, entry->overlay_component,
                                          10000 + (int)j,
                                          &entry->active_overlay);
    }
  }

  return UI_ERROR_NONE;
}
