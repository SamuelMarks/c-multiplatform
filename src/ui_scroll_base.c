/**
 * @file ui_scroll_base.c
 * @brief ui_scroll_base.c implementation.
 */
/*
 * \file ui_scroll_base.c
 * \brief Implementation of the UI Scroll Base component.
 */

/* clang-format off */
#include "ui_scroll_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/* \brief Default CSS stylesheet for the scroll base */
/** @brief Default CSS stylesheet */
static const char *ui_scroll_base_default_css =
    "div { "
    "overflow: auto; "
    "background: var(--scroll-bg, transparent); "
    "padding: var(--scroll-padding, 0); "
    "} "
    "div::-webkit-scrollbar { "
    "width: 8px; height: 8px; "
    "} "
    "div::-webkit-scrollbar-track { "
    "background: var(--scroll-track-bg, transparent); "
    "} "
    "div::-webkit-scrollbar-thumb { "
    "background: var(--scroll-thumb-bg, #bbb); "
    "border-radius: var(--scroll-thumb-radius, 4px); "
    "}";

/**
 * @struct ui_scroll_base
 * \brief Internal structure representing a scroll base component.
 */
struct ui_scroll_base {
  struct ui_component *component; /**< Underlying DOM container */
  struct ui_gesture_recognizer *gesture_recognizer; /**< Gesture recognizer */
  float scroll_x;                  /**< Current horizontal scroll */
  float scroll_y;                  /**< Current vertical scroll */
  float content_width;             /**< Content width */
  float content_height;            /**< Content height */
  float viewport_width;            /**< Viewport width */
  float viewport_height;           /**< Viewport height */
  ui_scroll_on_change_t on_change; /**< Scroll change callback */
  void *user_data;                 /**< Callback user data */
  struct ui_signal *data_signal;   /**< Bound data signal */
};

/**
 * \brief Updates DOM state to reflect the scroll position.
 *
 * \param scroll The scroll area component.
 */
/**
 * @brief update_dom_state.
 * @param scroll Parameter scroll.
 * @return Return value.
 */
static void update_dom_state(struct ui_scroll_base *scroll) {
  (void)scroll;
  /* You might map scroll positions to CSS variables or inline styles */
}

/**
 * \brief Creates a new unstyled scroll area base component.
 *
 * \param out_scroll Pointer to receive the allocated scroll base.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_base_create(struct ui_scroll_base **out_scroll) {
  struct ui_scroll_base *scroll;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_scroll) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scroll = (struct ui_scroll_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_scroll_base));
  if (!scroll) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  scroll->component = NULL;
  scroll->gesture_recognizer = NULL;
  scroll->scroll_x = 0.0f;
  scroll->scroll_y = 0.0f;
  scroll->content_width = 0.0f;
  scroll->content_height = 0.0f;
  scroll->viewport_width = 0.0f;
  scroll->viewport_height = 0.0f;
  scroll->on_change = NULL;
  scroll->user_data = NULL;

  rc = ui_component_create(&scroll->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_gesture_recognizer_create(&scroll->gesture_recognizer);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Set role and tabindex to make it scrollable/focusable */
  rc = ui_dom_node_set_attribute(root_node, "role", "region");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "tabindex", "0");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_scroll_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  {
    ui_error_t _ign_rc =
        ui_component_set_default_style(scroll->component, default_style);
    (void)_ign_rc;
  }

  scroll->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

  *out_scroll = scroll;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  if (scroll->gesture_recognizer) {
    (void)ui_gesture_recognizer_destroy(scroll->gesture_recognizer);
  }
  if (scroll->component) {
    (void)ui_component_destroy(scroll->component);
  }
  C_MULTIPLATFORM_FREE(scroll);
  return rc;
}

/**
 * \brief Destroys a scroll area component.
 *
 * \param scroll The scroll area to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_base_destroy(struct ui_scroll_base *scroll) {
  if (!scroll)
    return UI_ERROR_NONE;
  (void)ui_gesture_recognizer_destroy(scroll->gesture_recognizer);
  (void)ui_component_destroy(scroll->component);
  C_MULTIPLATFORM_FREE(scroll);
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the scroll position.
 *
 * \param scroll The scroll area.
 * \param x The horizontal scroll position.
 * \param y The vertical scroll position.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_base_set_scroll_pos(struct ui_scroll_base *scroll, float x,
                                         float y) {
  float max_x, max_y;

  if (!scroll)
    return UI_ERROR_INVALID_ARGUMENT;

  max_x = scroll->content_width - scroll->viewport_width;
  max_y = scroll->content_height - scroll->viewport_height;

  if (max_x < 0.0f)
    max_x = 0.0f;
  if (max_y < 0.0f)
    max_y = 0.0f;

  if (x < 0.0f)
    x = 0.0f;
  if (y < 0.0f)
    y = 0.0f;

  if (x > max_x)
    x = max_x;
  if (y > max_y)
    y = max_y;

  if (scroll->scroll_x != x || scroll->scroll_y != y) {
    scroll->scroll_x = x;
    scroll->scroll_y = y;

    update_dom_state(scroll);
    if (scroll->on_change) {
      {
        ui_error_t cb_rc = scroll->on_change(
            scroll, scroll->scroll_x, scroll->scroll_y, scroll->user_data);
        if (cb_rc != UI_ERROR_NONE) {
          return cb_rc;
        }
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Gets the horizontal scroll position.
 *
 * \param scroll The scroll area.
 * \param out_x Pointer to receive the horizontal scroll position.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_base_get_scroll_x(const struct ui_scroll_base *scroll,
                                       float *out_x) {
  if (!scroll || !out_x)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_x = scroll->scroll_x;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the vertical scroll position.
 *
 * \param scroll The scroll area.
 * \param out_y Pointer to receive the vertical scroll position.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_base_get_scroll_y(const struct ui_scroll_base *scroll,
                                       float *out_y) {
  if (!scroll || !out_y)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_y = scroll->scroll_y;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the content dimensions for overflow calculations.
 *
 * \param scroll The scroll area.
 * \param width The content width.
 * \param height The content height.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_base_set_content_size(struct ui_scroll_base *scroll,
                                           float width, float height) {
  if (!scroll)
    return UI_ERROR_INVALID_ARGUMENT;

  scroll->content_width = width;
  scroll->content_height = height;

  return ui_scroll_base_set_scroll_pos(scroll, scroll->scroll_x,
                                       scroll->scroll_y);
}

/**
 * \brief Sets the viewport dimensions for overflow calculations.
 *
 * \param scroll The scroll area.
 * \param width The viewport width.
 * \param height The viewport height.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_base_set_viewport_size(struct ui_scroll_base *scroll,
                                            float width, float height) {
  if (!scroll)
    return UI_ERROR_INVALID_ARGUMENT;

  scroll->viewport_width = width;
  scroll->viewport_height = height;

  return ui_scroll_base_set_scroll_pos(scroll, scroll->scroll_x,
                                       scroll->scroll_y);
}

/**
 * \brief Sets the change handler for the scroll area.
 *
 * \param scroll The scroll area.
 * \param on_change The callback to invoke when scrolling occurs.
 * \param user_data Opaque user data passed to the callback.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_base_set_on_change(struct ui_scroll_base *scroll,
                                        ui_scroll_on_change_t on_change,
                                        void *user_data) {
  if (!scroll)
    return UI_ERROR_INVALID_ARGUMENT;

  scroll->on_change = on_change;
  scroll->user_data = user_data;

  return UI_ERROR_NONE;
}

/**
 * \brief Processes an incoming input event to trigger scrolling (e.g., mouse
 * wheel, touch drag).
 *
 * \param scroll The scroll area.
 * \param event The input event.
 * \param timestamp_ms Current time in milliseconds.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_base_process_event(struct ui_scroll_base *scroll,
                                        const struct ui_event *event,
                                        double timestamp_ms) {
  (void)timestamp_ms;
  if (!scroll || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (event->type == UI_EVENT_MOUSE_WHEEL) {
    float new_x = scroll->scroll_x + event->event_data.mouse.wheel_x * 20.0f;
    float new_y = scroll->scroll_y + event->event_data.mouse.wheel_y * 20.0f;
    return ui_scroll_base_set_scroll_pos(scroll, new_x, new_y);
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * \param scroll The scroll area.
 * \param out_component Pointer to receive the underlying component.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_base_get_component(struct ui_scroll_base *scroll,
                                        struct ui_component **out_component) {
  if (!scroll || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = scroll->component;
  return UI_ERROR_NONE;
}

/**
 * \brief Binds the data property.
 *
 * \param widget The scroll area component.
 * \param signal The signal to bind to.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_base_bind_data(struct ui_scroll_base *widget,
                                    struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
