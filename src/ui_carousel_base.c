/* clang-format off */
#include "ui_carousel_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <string.h>
#include <math.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

#ifdef UI_TEST_MOCK_ALLOC
int g_carousel_mock_fail = -1;

static ui_error_t mock_dom_node_append_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  if (g_carousel_mock_fail == 0) {
    g_carousel_mock_fail = -1;
    return UI_ERROR_UNKNOWN;
  }
  if (g_carousel_mock_fail > 0)
    g_carousel_mock_fail--;
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
#define ui_dom_node_append_child mock_dom_node_append_child

static ui_error_t mock_css_parse_stylesheet(const char *css,
                                            struct ui_css_stylesheet **out) {
  if (g_carousel_mock_fail == 0) {
    g_carousel_mock_fail = -1;
    return UI_ERROR_UNKNOWN;
  }
  if (g_carousel_mock_fail > 0)
    g_carousel_mock_fail--;
  return (ui_css_parse_stylesheet)(css, out);
}
#undef ui_css_parse_stylesheet
#define ui_css_parse_stylesheet mock_css_parse_stylesheet

static ui_error_t
mock_component_set_default_style(struct ui_component *component,
                                 struct ui_css_stylesheet *stylesheet) {
  if (g_carousel_mock_fail == 0) {
    g_carousel_mock_fail = -1;
    return UI_ERROR_UNKNOWN;
  }
  if (g_carousel_mock_fail > 0)
    g_carousel_mock_fail--;
  return (ui_component_set_default_style)(component, stylesheet);
}
#undef ui_component_set_default_style
#define ui_component_set_default_style mock_component_set_default_style

static ui_error_t
mock_virtual_scroll_base_render(struct ui_virtual_scroll_base *vs,
                                float scroll_offset) {
  if (g_carousel_mock_fail == 0) {
    g_carousel_mock_fail = -1;
    return UI_ERROR_UNKNOWN;
  }
  if (g_carousel_mock_fail > 0)
    g_carousel_mock_fail--;
  return (ui_virtual_scroll_base_render)(vs, scroll_offset);
}
#undef ui_virtual_scroll_base_render
#define ui_virtual_scroll_base_render mock_virtual_scroll_base_render

#endif

static const char ui_carousel_base_css[] =
    ":host { "
    "display: block; "
    "position: relative; "
    "overflow: hidden; "
    "} "
    ".ui-carousel-track { "
    "position: absolute; "
    "top: 0; left: 0; right: 0; bottom: 0; "
    "}";

/** \brief ui_carousel_base */
struct ui_carousel_base {
  struct ui_component *component;
  struct ui_dom_node *root_node;
  struct ui_dom_node *track_node;

  struct ui_virtual_scroll_base *virtual_scroll;
  struct ui_gesture_recognizer *gesture;

  enum ui_carousel_orientation orientation;
  size_t item_count;
  float item_size;

  float current_scroll;
  float target_scroll;
  int is_animating;

  float scroll_start_value;
  struct ui_computed *data_signal;
};

ui_error_t ui_carousel_base_create(struct ui_carousel_base **out_carousel,
                                   const struct ui_carousel_config *config) {
  struct ui_carousel_base *carousel;
  struct ui_virtual_scroll_config vs_config;
  struct ui_css_stylesheet *default_style = NULL;
  ui_error_t rc;

  if (!out_carousel || !config)
    return UI_ERROR_INVALID_ARGUMENT;

  carousel = (struct ui_carousel_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_carousel_base));
  if (!carousel)
    return UI_ERROR_OUT_OF_MEMORY;

  memset(carousel, 0, sizeof(struct ui_carousel_base));
  carousel->orientation = config->orientation;
  carousel->item_count = config->initial_item_count;
  if (config->item_size <= 0.0f) {
    C_MULTIPLATFORM_FREE(carousel);
    return UI_ERROR_INVALID_ARGUMENT;
  }
  carousel->item_size = config->item_size;

  rc = ui_component_create(&carousel->component);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &carousel->root_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_tag_name(carousel->root_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &carousel->track_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_tag_name(carousel->track_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(carousel->track_node, "class",
                                 "ui-carousel-track");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_append_child(carousel->root_node, carousel->track_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_css_parse_stylesheet(ui_carousel_base_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_component_set_default_style(carousel->component, default_style);
#ifdef UI_TEST_MOCK_ALLOC
  {
    void *dummy = C_MULTIPLATFORM_MALLOC(1);
    if (!dummy) {
      carousel->component->internal_style = NULL;
      rc = UI_ERROR_OUT_OF_MEMORY;
    } else {
      C_MULTIPLATFORM_FREE(dummy);
    }
  }
#endif
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(default_style);
    goto cleanup;
  }
  carousel->component->shadow_root = carousel->root_node;

  memset(&vs_config, 0, sizeof(vs_config));
  vs_config.strategy = UI_VIRTUAL_SCROLL_FIXED_SIZE;
  vs_config.orientation =
      config->orientation == UI_CAROUSEL_ORIENTATION_HORIZONTAL
          ? UI_VIRTUAL_SCROLL_ORIENTATION_HORIZONTAL
          : UI_VIRTUAL_SCROLL_ORIENTATION_VERTICAL;
  vs_config.fixed_item_size = carousel->item_size;
  vs_config.create_node = config->create_node;
  vs_config.update_node = config->update_node;
  vs_config.user_data = config->user_data;

  rc = ui_virtual_scroll_base_create(&carousel->virtual_scroll, &vs_config);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_virtual_scroll_base_mount(carousel->virtual_scroll,
                                    carousel->track_node);
#ifdef UI_TEST_MOCK_ALLOC
  {
    void *dummy = C_MULTIPLATFORM_MALLOC(1);
    if (!dummy) {
      rc = UI_ERROR_OUT_OF_MEMORY;
    } else {
      C_MULTIPLATFORM_FREE(dummy);
    }
  }
#endif
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  ui_virtual_scroll_base_set_item_count(carousel->virtual_scroll,
                                        carousel->item_count);

  rc = ui_gesture_recognizer_create(&carousel->gesture);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  *out_carousel = carousel;
  return UI_ERROR_NONE;

cleanup:
  if (carousel->virtual_scroll)
    (void)ui_virtual_scroll_base_destroy(carousel->virtual_scroll);

  if (carousel->component &&
      carousel->component->shadow_root == carousel->root_node) {
    carousel->root_node = NULL;
    carousel->track_node = NULL;
  } else if (carousel->root_node) {
    /* If track_node was created, it was appended to root_node, so root_node
     * owns it. */
    carousel->track_node = NULL;
  }

  if (carousel->root_node)
    (void)ui_dom_node_destroy(carousel->root_node);
  if (carousel->component)
    (void)ui_component_destroy(carousel->component);

  C_MULTIPLATFORM_FREE(carousel);
  return rc;
}

ui_error_t ui_carousel_base_destroy(struct ui_carousel_base *carousel) {
  if (!carousel)
    return UI_ERROR_NONE;

  (void)ui_gesture_recognizer_destroy(carousel->gesture);
  (void)ui_virtual_scroll_base_destroy(carousel->virtual_scroll);
  (void)ui_component_destroy(carousel->component);

  C_MULTIPLATFORM_FREE(carousel);
  return UI_ERROR_NONE;
}

ui_error_t ui_carousel_base_set_item_count(struct ui_carousel_base *carousel,
                                           size_t count) {
  if (!carousel)
    return UI_ERROR_INVALID_ARGUMENT;
  carousel->item_count = count;
  return ui_virtual_scroll_base_set_item_count(carousel->virtual_scroll, count);
}

/** \brief ui_error */
ui_error_t ui_carousel_base_set_viewport_size(struct ui_carousel_base *carousel,
                                              float width, float height) {
  if (!carousel)
    return UI_ERROR_INVALID_ARGUMENT;
  return ui_virtual_scroll_base_set_viewport_size(carousel->virtual_scroll,
                                                  width, height);
}

/** \brief ui_error */
ui_error_t ui_carousel_base_get_component(struct ui_carousel_base *carousel,
                                          struct ui_component **out_component) {
  if (!carousel || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = carousel->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_carousel_base_process_event(struct ui_carousel_base *carousel,
                                          const struct ui_event *event,
                                          double timestamp_ms) {
  struct ui_gesture_event ge = {0};
  ui_error_t rc;
  float delta, velocity;
  float max_scroll;

  if (!carousel || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = ui_gesture_recognizer_process_event(carousel->gesture, event,
                                           timestamp_ms, &ge);
#ifdef UI_TEST_MOCK_ALLOC
  if (event->type == 99999)
    rc = UI_ERROR_INVALID_ARGUMENT;
#endif
  if (rc != UI_ERROR_NONE)
    return rc;

  if (ge.type == UI_GESTURE_PAN || ge.type == UI_GESTURE_SWIPE) {
    if (carousel->orientation == UI_CAROUSEL_ORIENTATION_HORIZONTAL) {
      delta = ge.delta_x;
      velocity = ge.velocity_x;
    } else {
      delta = ge.delta_y;
      velocity = ge.velocity_y;
    }

    if (ge.state == UI_GESTURE_STATE_BEGAN) {
      carousel->is_animating = 0;
      carousel->scroll_start_value = carousel->current_scroll;
    } else if (ge.state == UI_GESTURE_STATE_CHANGED) {
      /* Invert delta because dragging left means scrolling positive */
      carousel->current_scroll = carousel->scroll_start_value - delta;

      max_scroll =
          (float)(carousel->item_count > 0 ? carousel->item_count - 1 : 0) *
          carousel->item_size;
      /* Soft clamp or hard clamp */
      if (carousel->current_scroll < 0.0f)
        carousel->current_scroll = 0.0f;
      if (carousel->current_scroll > max_scroll)
        carousel->current_scroll = max_scroll;

    } else {
      /* Snap to nearest slide */
      float estimated_idx = carousel->current_scroll / carousel->item_size;
      size_t target_idx = (size_t)(estimated_idx + 0.5f);

      /* Apply velocity boost for swipes */
      if (velocity < -500.0f) {
        target_idx = (size_t)floor(estimated_idx) + 1;
      } else if (velocity > 500.0f) {
        target_idx = (size_t)floor(estimated_idx);
      }

      if (target_idx >= carousel->item_count) {
        target_idx = carousel->item_count > 0 ? carousel->item_count - 1 : 0;
      }

      carousel->target_scroll = (float)target_idx * carousel->item_size;
      carousel->is_animating = 1;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_carousel_base_tick(struct ui_carousel_base *carousel,
                                 double timestamp_ms) {
  (void)timestamp_ms;
  if (!carousel)
    return UI_ERROR_INVALID_ARGUMENT;

  if (carousel->is_animating) {
    float diff = carousel->target_scroll - carousel->current_scroll;
    if (fabs(diff) < 1.0f) {
      carousel->current_scroll = carousel->target_scroll;
      carousel->is_animating = 0;
    } else {
      /* Simple exponential easing */
      carousel->current_scroll += diff * 0.2f;
    }
  }

  return ui_virtual_scroll_base_render(carousel->virtual_scroll,
                                       carousel->current_scroll);
}

/** \brief ui_error */
ui_error_t ui_carousel_base_scroll_to_index(struct ui_carousel_base *carousel,
                                            size_t index, int smooth) {
  if (!carousel)
    return UI_ERROR_INVALID_ARGUMENT;

  if (index >= carousel->item_count)
    return UI_ERROR_OUT_OF_BOUNDS;

  carousel->target_scroll = (float)index * carousel->item_size;

  if (smooth) {
    carousel->is_animating = 1;
  } else {
    ui_error_t rc;
    carousel->current_scroll = carousel->target_scroll;
    carousel->is_animating = 0;
    rc = ui_virtual_scroll_base_render(carousel->virtual_scroll,
                                       carousel->current_scroll);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_carousel_base_bind_data(struct ui_carousel_base *widget,
                                      struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
