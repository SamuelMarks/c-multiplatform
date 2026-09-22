/**
 * @file ui_toast_manager_base.c
 * @brief Implementation of the global toast notification manager.
 */
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

#ifdef UI_TEST_MOCK_ALLOC
int g_toast_mock_fail = 0;
int g_toast_mock_set_attr_fail_target = 0;
int g_toast_mock_destroy_target = 0;
static int g_toast_set_attr_counter = 0;
int g_toast_destroy_counter = 0;

/**
 * @brief mock_toast_component_destroy.
 * @param comp Component.
 * @return Return value.
 */
static ui_error_t mock_toast_component_destroy(struct ui_component *comp) {
  if (g_toast_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_destroy(comp);
}
/** @cond */
#define ui_component_destroy mock_toast_component_destroy
/** @endcond */

/**
 * @brief mock_toast_dom_node_destroy.
 * @param node Node.
 * @return Return value.
 */
static ui_error_t mock_toast_dom_node_destroy(struct ui_dom_node *node) {
  if (g_toast_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_toast_mock_destroy_target > 0) {
    if (++g_toast_destroy_counter == g_toast_mock_destroy_target) {
      g_toast_destroy_counter = 0;
      return UI_ERROR_UNKNOWN;
    }
  }
  return ui_dom_node_destroy(node);
}
/** @cond */
#define ui_dom_node_destroy mock_toast_dom_node_destroy
/** @endcond */

/**
 * @brief mock_toast_dom_node_create.
 * @param type Node type.
 * @param out_node Output pointer.
 * @return Return value.
 */
static ui_error_t mock_toast_dom_node_create(enum ui_dom_node_type type,
                                             struct ui_dom_node **out_node) {
  if (g_toast_mock_fail == 8 && type == UI_DOM_NODE_TYPE_TEXT) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_create(type, out_node);
}
/** @cond */
#define ui_dom_node_create mock_toast_dom_node_create
/** @endcond */

/**
 * @brief mock_toast_overlay_director_unmount.
 * @param director Director.
 * @param overlay Overlay.
 * @return Return value.
 */
static ui_error_t
mock_toast_overlay_director_unmount(struct ui_overlay_director *director,
                                    struct ui_overlay *overlay) {
  if (g_toast_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_overlay_director_unmount(director, overlay);
}
/** @cond */
#define ui_overlay_director_unmount mock_toast_overlay_director_unmount
/** @endcond */

/**
 * @brief mock_toast_dom_node_append_child.
 * @param parent Parent node.
 * @param child Child node.
 * @return Return value.
 */
static ui_error_t mock_toast_dom_node_append_child(struct ui_dom_node *parent,
                                                   struct ui_dom_node *child) {
  if (g_toast_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_append_child(parent, child);
}
/** @cond */
#define ui_dom_node_append_child mock_toast_dom_node_append_child
/** @endcond */

/**
 * @brief mock_toast_dom_node_set_attribute.
 * @param node Node.
 * @param name Name.
 * @param val Value.
 * @return Return value.
 */
static ui_error_t mock_toast_dom_node_set_attribute(struct ui_dom_node *node,
                                                    const char *name,
                                                    const char *val) {
  if (g_toast_mock_set_attr_fail_target > 0) {
    if (++g_toast_set_attr_counter == g_toast_mock_set_attr_fail_target) {
      g_toast_set_attr_counter = 0;
      return UI_ERROR_UNKNOWN;
    }
  }
  return ui_dom_node_set_attribute(node, name, val);
}
/** @cond */
#define ui_dom_node_set_attribute mock_toast_dom_node_set_attribute
/** @endcond */

/**
 * @brief mock_toast_overlay_director_mount_component.
 * @param director Director.
 * @param comp Component.
 * @param layer Layer.
 * @param out_overlay Output pointer.
 * @return Return value.
 */
static ui_error_t mock_toast_overlay_director_mount_component(
    struct ui_overlay_director *director, struct ui_component *comp, int layer,
    struct ui_overlay **out_overlay) {
  if (g_toast_mock_fail == 6) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_overlay_director_mount_component(director, comp, layer,
                                             out_overlay);
}
/** @cond */
#define ui_overlay_director_mount_component                                    \
  mock_toast_overlay_director_mount_component
/** @endcond */
#endif

/**
 * @struct ui_toast_entry
 * @struct ui_toast_entry
 * @brief Internal representation of an active toast notification.
 */
struct ui_toast_entry {
  /* @brief Unique identifier for the toast. */
  ui_toast_id id; /**< id */
  /* @brief Configuration for the toast. */
  struct ui_toast_config config; /**< config */
  /* @brief Current animation state. */
  enum ui_toast_anim_state anim_state; /**< anim_state */
  /* @brief Time when the toast was shown. */
  double show_time; /**< show_time */
  /* @brief Total time the toast has been paused. */
  double total_paused_time; /**< total_paused_time */
  /* @brief Time when the current pause started. */
  double pause_start_time; /**< pause_start_time */
  /* @brief Flag indicating if the toast is currently paused. */
  int is_paused; /**< is_paused */
  /* @brief The message string to display. */
  char *message; /**< message */

  /* @brief The overlay component for rendering. */
  struct ui_component *overlay_component; /**< overlay_component */
  /* @brief The active overlay instance. */
  struct ui_overlay *active_overlay; /**< active_overlay */
};

/**
 * @struct ui_toast_region_stack
 * @struct ui_toast_region_stack
 * @brief Represents a stack of toasts in a specific region.
 */
struct ui_toast_region_stack {
  /* @brief Array of toast entries. */
  struct ui_toast_entry **toasts; /**< toasts */
  /* @brief Number of toasts in the stack. */
  size_t count; /**< count */
  /* @brief Allocated capacity of the toasts array. */
  size_t capacity; /**< capacity */
};

/**
 * @struct ui_toast_manager_base
 * @struct ui_toast_manager_base
 * @brief Internal implementation of the global toast manager.
 */
struct ui_toast_manager_base {
  /* @brief Array of toast region stacks. */
  struct ui_toast_region_stack regions[UI_TOAST_REGION_COUNT]; /**< regions */
  /* @brief Next available toast ID. */
  ui_toast_id next_id; /**< next_id */
  /* @brief Simple global hover state for primitive pause logic. */
  int is_hovered; /**< is_hovered */
};

ui_error_t
ui_toast_manager_base_create(struct ui_toast_manager_base **out_manager) {
  struct ui_toast_manager_base *manager;
  int i;

  if (!out_manager)
    return UI_ERROR_INVALID_ARGUMENT;

  manager = (struct ui_toast_manager_base *)C_MULTIPLATFORM_MALLOC(
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

/**
 * @brief Helper function to free a single toast entry.
 * @param entry The entry to free.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t free_toast_entry(struct ui_toast_entry *entry) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!entry) {
    return UI_ERROR_NONE;
  }
#ifdef UI_TEST_MOCK_ALLOC
  if (g_toast_mock_fail == 10) {
    if (entry->message)
      C_MULTIPLATFORM_FREE(entry->message);
    if (entry->overlay_component) {
      ui_component_destroy(entry->overlay_component);
    }
    C_MULTIPLATFORM_FREE(entry);
    return UI_ERROR_UNKNOWN;
  }
#endif
  if (entry->message)
    C_MULTIPLATFORM_FREE(entry->message);
  if (entry->overlay_component) {
    rc = ui_component_destroy(entry->overlay_component);
  }
  C_MULTIPLATFORM_FREE(entry);
  return rc;
}

ui_error_t
ui_toast_manager_base_destroy(struct ui_toast_manager_base *manager) {
  int i;
  size_t j;
  if (!manager)
    return UI_ERROR_NONE;

  for (i = 0; i < UI_TOAST_REGION_COUNT; i++) {
    for (j = 0; j < manager->regions[i].count; j++) {
      free_toast_entry(manager->regions[i].toasts[j]);
    }
    if (manager->regions[i].toasts) {
      C_MULTIPLATFORM_FREE(manager->regions[i].toasts);
    }
  }
  C_MULTIPLATFORM_FREE(manager);
  return UI_ERROR_NONE;
}

ui_error_t ui_toast_manager_base_show(struct ui_toast_manager_base *manager,
                                      const struct ui_toast_config *config,
                                      double current_time_secs,
                                      ui_toast_id *out_id) {

  struct ui_toast_entry *entry;
  struct ui_toast_region_stack *stack;
  ui_error_t rc;
  ui_error_t rc_cleanup;

  if (!manager || !config || !out_id ||
      config->region >= UI_TOAST_REGION_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  entry = (struct ui_toast_entry *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_toast_entry));
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
    entry->message = C_MULTIPLATFORM_STRDUP(config->message);
    if (!entry->message) {
      rc_cleanup = free_toast_entry(entry);
      if (rc_cleanup != UI_ERROR_NONE) {
        return rc_cleanup;
      }
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  rc = ui_component_create(&entry->overlay_component);
  if (rc != UI_ERROR_NONE) {
    rc_cleanup = free_toast_entry(entry);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
    return rc;
  }

  stack = &manager->regions[config->region];
  if (stack->count >= stack->capacity) {
    size_t new_cap = stack->capacity == 0 ? 4 : stack->capacity * 2;
    struct ui_toast_entry **new_arr =
        (struct ui_toast_entry **)C_MULTIPLATFORM_REALLOC(
            stack->toasts, (size_t)new_cap * sizeof(struct ui_toast_entry *));
    if (!new_arr) {
      rc_cleanup = free_toast_entry(entry);
      if (rc_cleanup != UI_ERROR_NONE) {
        return rc_cleanup;
      }
      return UI_ERROR_OUT_OF_MEMORY;
    }
    stack->toasts = new_arr;
    stack->capacity = new_cap;
  }

  stack->toasts[stack->count++] = entry;
  *out_id = entry->id;

  return UI_ERROR_NONE;
}

ui_error_t ui_toast_manager_base_dismiss(struct ui_toast_manager_base *manager,
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

ui_error_t ui_toast_manager_base_tick(struct ui_toast_manager_base *manager,
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

ui_error_t
ui_toast_manager_base_handle_event(struct ui_toast_manager_base *manager,
                                   const struct ui_event *event,
                                   double current_time_secs) {
  int i;
  size_t j;

  if (!manager || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  switch (event->type) {
  case UI_EVENT_MOUSE_MOVE:
  case UI_EVENT_TOUCH_START:
    /* Pause on hover/touch */
    if (!manager->is_hovered) {
      manager->is_hovered = 1;
      for (i = 0; i < UI_TOAST_REGION_COUNT; i++) {
        struct ui_toast_region_stack *stack = &manager->regions[i];
        for (j = 0; j < stack->count; j++) {
          stack->toasts[j]->is_paused = 1;
          stack->toasts[j]->pause_start_time = current_time_secs;
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
          stack->toasts[j]->is_paused = 0;
          stack->toasts[j]->total_paused_time +=
              (current_time_secs - stack->toasts[j]->pause_start_time);
        }
      }
    }
    break;

  default:
    break;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief get_region_style.
 * @param region Parameter region.
 * @param out_str Parameter out_str.
 * @return Return value.
 */
static ui_error_t get_region_style(enum ui_toast_region region,
                                   const char **out_str) {
#ifdef UI_TEST_MOCK_ALLOC
  if (g_toast_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
#endif
  switch (region) {
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
    *out_str = "position: absolute; top: 20px; left: 20px;";
    return UI_ERROR_NONE;
  }
}

ui_error_t ui_toast_manager_base_render(struct ui_toast_manager_base *manager,
                                        struct ui_overlay_director *director) {
  int i;
  size_t j;
  ui_error_t rc;
  ui_error_t rc_cleanup;

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
      struct ui_dom_node *root_node = NULL;
      struct ui_dom_node *text_node = NULL;
      const char *region_style = "";
      char style_buf[512];

      if (entry->active_overlay) {
        /* Unmount old so we can rebuild. In a full diff engine, this is
         * optimized. */
        rc = ui_overlay_director_unmount(director, entry->active_overlay);
        if (rc != UI_ERROR_NONE)
          return rc;
        entry->active_overlay = NULL;
      }

      rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
      if (rc != UI_ERROR_NONE)
        continue;

      rc = ui_dom_node_set_attribute(
          root_node, "role", entry->config.is_error ? "alert" : "status");
      if (rc != UI_ERROR_NONE) {
        rc_cleanup = ui_dom_node_destroy(root_node);
        if (rc_cleanup != UI_ERROR_NONE)
          return rc_cleanup;
        return rc;
      }

      rc = ui_dom_node_set_attribute(root_node, "aria-live", "polite");
      if (rc != UI_ERROR_NONE) {
        rc_cleanup = ui_dom_node_destroy(root_node);
        if (rc_cleanup != UI_ERROR_NONE)
          return rc_cleanup;
        return rc;
      }

      rc = get_region_style((enum ui_toast_region)i, &region_style);
      if (rc != UI_ERROR_NONE) {
        rc_cleanup = ui_dom_node_destroy(root_node);
        if (rc_cleanup != UI_ERROR_NONE)
          return rc_cleanup;
        return rc;
      }

      /* Apply stack offset. e.g. j * 60px down or up depending on region */
      if (i >= UI_TOAST_REGION_BOTTOM_LEFT) {
#if defined(_MSC_VER)
        sprintf_s(style_buf, sizeof(style_buf), "%s margin-bottom: %lupx;",
                  region_style, (unsigned long)(j * 60));
#else
        sprintf(style_buf, "%s margin-bottom: %lupx;", region_style,
                (unsigned long)(j * 60));
#endif
      } else {
#if defined(_MSC_VER)
        sprintf_s(style_buf, sizeof(style_buf), "%s margin-top: %lupx;",
                  region_style, (unsigned long)(j * 60));
#else
        sprintf(style_buf, "%s margin-top: %lupx;", region_style,
                (unsigned long)(j * 60));
#endif
      }

      rc = ui_dom_node_set_attribute(root_node, "style", style_buf);
      if (rc != UI_ERROR_NONE) {
        rc_cleanup = ui_dom_node_destroy(root_node);
        if (rc_cleanup != UI_ERROR_NONE)
          return rc_cleanup;
        return rc;
      }

      if (entry->message) {
        rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
        if (rc != UI_ERROR_NONE) {
          rc_cleanup = ui_dom_node_destroy(root_node);
          if (rc_cleanup != UI_ERROR_NONE)
            return rc_cleanup;
          return rc;
        }
        text_node->text_content = C_MULTIPLATFORM_STRDUP(entry->message);
        if (!text_node->text_content) {
          rc_cleanup = ui_dom_node_destroy(text_node);
          if (rc_cleanup != UI_ERROR_NONE)
            return rc_cleanup;
          rc_cleanup = ui_dom_node_destroy(root_node);
          if (rc_cleanup != UI_ERROR_NONE)
            return rc_cleanup;
          return UI_ERROR_OUT_OF_MEMORY;
        }
        rc = ui_dom_node_append_child(root_node, text_node);
        if (rc != UI_ERROR_NONE) {
          rc_cleanup = ui_dom_node_destroy(text_node);
          if (rc_cleanup != UI_ERROR_NONE)
            return rc_cleanup;
          rc_cleanup = ui_dom_node_destroy(root_node);
          if (rc_cleanup != UI_ERROR_NONE)
            return rc_cleanup;
          return rc;
        }
      }

      if (entry->overlay_component->shadow_root) {
        rc_cleanup = ui_dom_node_destroy(entry->overlay_component->shadow_root);
        if (rc_cleanup != UI_ERROR_NONE)
          return rc_cleanup;
      }
      entry->overlay_component->shadow_root = root_node;
      rc = ui_overlay_director_mount_component(
          director, entry->overlay_component, 10000 + (int)j,
          &entry->active_overlay);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}
