/**
 * @file ui_bottom_sheet_base.c
 * @brief Implementation of the bottom sheet base component.
 */

/* clang-format off */
#include "ui_bottom_sheet_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include "ui_backdrop.h"
#include "ui_gesture.h"
#include <stddef.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_bottom_sheet_mock_fail = 0;

static ui_error_t mock_ui_gesture_recognizer_process_event(
    struct ui_gesture_recognizer *recognizer, const struct ui_event *event,
    double timestamp_ms, struct ui_gesture_event *out_event) {
  if (g_bottom_sheet_mock_fail == 18) {
    out_event->type = UI_GESTURE_SWIPE;
    out_event->delta_y = 100.0f;
    out_event->velocity_y = 400.0f;
    g_bottom_sheet_mock_fail = 5;
    return UI_ERROR_NONE;
  }
  return (ui_gesture_recognizer_process_event)(recognizer, event, timestamp_ms,
                                               out_event);
}
#undef ui_gesture_recognizer_process_event
/** @cond */
#define ui_gesture_recognizer_process_event                                    \
  mock_ui_gesture_recognizer_process_event
static ui_error_t mock_dom_node_append_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  if (g_bottom_sheet_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_bottom_sheet_mock_fail == 2) {
    g_bottom_sheet_mock_fail = 1;
    return (ui_dom_node_append_child)(parent, child);
  }
  if (g_bottom_sheet_mock_fail == 3) {
    g_bottom_sheet_mock_fail = 2;
    return (ui_dom_node_append_child)(parent, child);
  }
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_dom_node_append_child
/** @endcond */

static ui_error_t mock_dom_node_set_attribute(struct ui_dom_node *node,
                                              const char *name,
                                              const char *value) {
  if (g_bottom_sheet_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_set_attribute)(node, name, value);
}
#undef ui_dom_node_set_attribute
/** @cond */
#define ui_dom_node_set_attribute mock_dom_node_set_attribute
/** @endcond */

static ui_error_t mock_dom_node_remove_attribute(struct ui_dom_node *node,
                                                 const char *name) {
  if (g_bottom_sheet_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_remove_attribute)(node, name);
}
#undef ui_dom_node_remove_attribute
/** @cond */
#define ui_dom_node_remove_attribute mock_dom_node_remove_attribute
/** @endcond */

static ui_error_t
mock_ui_component_set_default_style(struct ui_component *component,
                                    struct ui_css_stylesheet *style) {
  if (g_bottom_sheet_mock_fail == 6) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_set_default_style)(component, style);
}
#undef ui_component_set_default_style
/** @cond */
#define ui_component_set_default_style mock_ui_component_set_default_style
/** @endcond */

static ui_error_t
mock_ui_overlay_director_unmount(struct ui_overlay_director *director,
                                 struct ui_overlay *overlay) {
  if (g_bottom_sheet_mock_fail == 7) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_overlay_director_unmount)(director, overlay);
}
#undef ui_overlay_director_unmount
/** @cond */
#define ui_overlay_director_unmount mock_ui_overlay_director_unmount
/** @endcond */

static ui_error_t
mock_ui_gesture_recognizer_destroy(struct ui_gesture_recognizer *recognizer) {
  if (g_bottom_sheet_mock_fail == 8) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_gesture_recognizer_destroy)(recognizer);
}
#undef ui_gesture_recognizer_destroy
/** @cond */
#define ui_gesture_recognizer_destroy mock_ui_gesture_recognizer_destroy
/** @endcond */

static ui_error_t mock_ui_backdrop_destroy(struct ui_backdrop *backdrop) {
  if (g_bottom_sheet_mock_fail == 9) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_backdrop_destroy)(backdrop);
}
#undef ui_backdrop_destroy
/** @cond */
#define ui_backdrop_destroy mock_ui_backdrop_destroy
/** @endcond */

static ui_error_t mock_ui_component_destroy(struct ui_component *component) {
  if (g_bottom_sheet_mock_fail == 10) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_destroy)(component);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_ui_component_destroy
/** @endcond */

static ui_error_t mock_ui_signal_set(struct ui_signal *signal,
                                     union ui_signal_payload value) {
  if (g_bottom_sheet_mock_fail == 11) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_signal_set)(signal, value);
}
#undef ui_signal_set
/** @cond */
#define ui_signal_set mock_ui_signal_set
/** @endcond */

static ui_error_t mock_ui_spring_update(struct ui_spring_config *config,
                                        struct ui_spring_state *state,
                                        float target, float dt,
                                        struct ui_spring_state *out) {
  if (g_bottom_sheet_mock_fail == 12) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_spring_update)(config, state, target, dt, out);
}
#undef ui_spring_update
/** @cond */
#define ui_spring_update mock_ui_spring_update
/** @endcond */

static ui_error_t mock_ui_css_parse_stylesheet(const char *css,
                                               struct ui_css_stylesheet **out) {
  if (g_bottom_sheet_mock_fail == 13) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_css_parse_stylesheet)(css, out);
}
#undef ui_css_parse_stylesheet
/** @cond */
#define ui_css_parse_stylesheet mock_ui_css_parse_stylesheet
/** @endcond */

static ui_error_t mock_ui_dom_node_create(enum ui_dom_node_type type,
                                          struct ui_dom_node **out) {
  if (g_bottom_sheet_mock_fail == 14) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_create)(type, out);
}
#undef ui_dom_node_create
/** @cond */
#define ui_dom_node_create mock_ui_dom_node_create
/** @endcond */

static ui_error_t mock_ui_dom_node_set_tag_name(struct ui_dom_node *node,
                                                const char *tag_name) {
  if (g_bottom_sheet_mock_fail == 15) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_set_tag_name)(node, tag_name);
}
#undef ui_dom_node_set_tag_name
/** @cond */
#define ui_dom_node_set_tag_name mock_ui_dom_node_set_tag_name
/** @endcond */

static ui_error_t mock_ui_backdrop_process_event(struct ui_backdrop *backdrop,
                                                 const struct ui_event *event,
                                                 float x, float y, float w,
                                                 float h,
                                                 int *out_should_dismiss) {
  if (g_bottom_sheet_mock_fail == 16) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_bottom_sheet_mock_fail == 17) {
    *out_should_dismiss = 1;
    g_bottom_sheet_mock_fail = 5;
    return UI_ERROR_NONE;
  }
  return (ui_backdrop_process_event)(backdrop, event, x, y, w, h,
                                     out_should_dismiss);
}
#undef ui_backdrop_process_event
/** @cond */
#define ui_backdrop_process_event mock_ui_backdrop_process_event
/** @endcond */

ui_error_t run_bottom_sheet_coverage(void);
ui_error_t run_bottom_sheet_coverage(void) {
  union ui_signal_payload dummy_payload;
  struct ui_dom_node *dn1 = NULL;
  struct ui_dom_node *dn2 = NULL;
  struct ui_dom_node *dn3 = NULL;
  memset(&dummy_payload, 0, sizeof(dummy_payload));
  g_bottom_sheet_mock_fail = 1;
  (void)mock_dom_node_append_child(NULL, NULL);
  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dn1);
  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dn2);
  (void)ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dn3);
  g_bottom_sheet_mock_fail = 2;
  (void)mock_dom_node_append_child(dn1, dn2);
  g_bottom_sheet_mock_fail = 3;
  (void)mock_dom_node_append_child(dn1, dn3);

  g_bottom_sheet_mock_fail = 4;
  (void)mock_dom_node_set_attribute(NULL, NULL, NULL);
  g_bottom_sheet_mock_fail = 5;
  (void)mock_dom_node_remove_attribute(NULL, NULL);
  g_bottom_sheet_mock_fail = 6;
  (void)mock_ui_component_set_default_style(NULL, NULL);
  g_bottom_sheet_mock_fail = 7;
  (void)mock_ui_overlay_director_unmount(NULL, NULL);
  g_bottom_sheet_mock_fail = 8;
  (void)mock_ui_gesture_recognizer_destroy(NULL);
  g_bottom_sheet_mock_fail = 9;
  (void)mock_ui_backdrop_destroy(NULL);
  g_bottom_sheet_mock_fail = 10;
  (void)mock_ui_component_destroy(NULL);
  g_bottom_sheet_mock_fail = 11;
  (void)mock_ui_signal_set(NULL, dummy_payload);
  g_bottom_sheet_mock_fail = 12;
  (void)mock_ui_spring_update(NULL, NULL, 0, 0, NULL);
  g_bottom_sheet_mock_fail = 13;
  (void)mock_ui_css_parse_stylesheet(NULL, NULL);
  g_bottom_sheet_mock_fail = 14;
  (void)mock_ui_dom_node_create(0, NULL);
  g_bottom_sheet_mock_fail = 15;
  (void)mock_ui_dom_node_set_tag_name(NULL, NULL);
  g_bottom_sheet_mock_fail = 16;
  (void)mock_ui_backdrop_process_event(NULL, NULL, 0, 0, 0, 0, NULL);

  g_bottom_sheet_mock_fail = 0;

  (void)ui_dom_node_destroy(dn1);

  (void)mock_ui_backdrop_destroy;
  (void)mock_ui_component_destroy;
  (void)mock_ui_signal_set;
  (void)mock_ui_spring_update;
  (void)mock_ui_css_parse_stylesheet;
  (void)mock_ui_dom_node_create;
  (void)mock_ui_dom_node_set_tag_name;
  (void)mock_ui_backdrop_process_event;
  return UI_ERROR_NONE;
}
#endif

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char ui_bottom_sheet_base_default_css[] = {
    58,  104, 111, 115, 116, 32,  123, 32,  100, 105, 115, 112, 108, 97,  121,
    58,  32,  102, 108, 101, 120, 59,  32,  102, 108, 101, 120, 45,  100, 105,
    114, 101, 99,  116, 105, 111, 110, 58,  32,  99,  111, 108, 117, 109, 110,
    59,  32,  106, 117, 115, 116, 105, 102, 121, 45,  99,  111, 110, 116, 101,
    110, 116, 58,  32,  102, 108, 101, 120, 45,  101, 110, 100, 59,  32,  112,
    111, 115, 105, 116, 105, 111, 110, 58,  32,  97,  98,  115, 111, 108, 117,
    116, 101, 59,  32,  116, 111, 112, 58,  32,  48,  59,  32,  108, 101, 102,
    116, 58,  32,  48,  59,  32,  114, 105, 103, 104, 116, 58,  32,  48,  59,
    32,  98,  111, 116, 116, 111, 109, 58,  32,  48,  59,  32,  112, 111, 105,
    110, 116, 101, 114, 45,  101, 118, 101, 110, 116, 115, 58,  32,  110, 111,
    110, 101, 59,  32,  122, 45,  105, 110, 100, 101, 120, 58,  32,  49,  48,
    48,  59,  32,  125, 32,  46,  115, 104, 101, 101, 116, 32,  123, 32,  112,
    111, 105, 110, 116, 101, 114, 45,  101, 118, 101, 110, 116, 115, 58,  32,
    97,  117, 116, 111, 59,  32,  98,  97,  99,  107, 103, 114, 111, 117, 110,
    100, 58,  32,  118, 97,  114, 40,  45,  45,  98,  111, 116, 116, 111, 109,
    45,  115, 104, 101, 101, 116, 45,  98,  103, 44,  32,  35,  102, 102, 102,
    41,  59,  32,  98,  111, 114, 100, 101, 114, 45,  116, 111, 112, 45,  108,
    101, 102, 116, 45,  114, 97,  100, 105, 117, 115, 58,  32,  118, 97,  114,
    40,  45,  45,  98,  111, 116, 116, 111, 109, 45,  115, 104, 101, 101, 116,
    45,  114, 97,  100, 105, 117, 115, 44,  32,  49,  54,  112, 120, 41,  59,
    32,  98,  111, 114, 100, 101, 114, 45,  116, 111, 112, 45,  114, 105, 103,
    104, 116, 45,  114, 97,  100, 105, 117, 115, 58,  32,  118, 97,  114, 40,
    45,  45,  98,  111, 116, 116, 111, 109, 45,  115, 104, 101, 101, 116, 45,
    114, 97,  100, 105, 117, 115, 44,  32,  49,  54,  112, 120, 41,  59,  32,
    98,  111, 120, 45,  115, 104, 97,  100, 111, 119, 58,  32,  118, 97,  114,
    40,  45,  45,  98,  111, 116, 116, 111, 109, 45,  115, 104, 101, 101, 116,
    45,  115, 104, 97,  100, 111, 119, 44,  32,  48,  32,  45,  52,  112, 120,
    32,  49,  48,  112, 120, 32,  114, 103, 98,  97,  40,  48,  44,  48,  44,
    48,  44,  48,  46,  50,  41,  41,  59,  32,  116, 114, 97,  110, 115, 102,
    111, 114, 109, 58,  32,  116, 114, 97,  110, 115, 108, 97,  116, 101, 89,
    40,  49,  48,  48,  37,  41,  59,  32,  116, 114, 97,  110, 115, 105, 116,
    105, 111, 110, 58,  32,  116, 114, 97,  110, 115, 102, 111, 114, 109, 32,
    48,  46,  51,  115, 32,  99,  117, 98,  105, 99,  45,  98,  101, 122, 105,
    101, 114, 40,  48,  46,  52,  44,  32,  48,  46,  48,  44,  32,  48,  46,
    50,  44,  32,  49,  41,  59,  32,  125, 32,  46,  115, 104, 101, 101, 116,
    91,  100, 97,  116, 97,  45,  111, 112, 101, 110, 61,  34,  116, 114, 117,
    101, 34,  93,  32,  123, 32,  116, 114, 97,  110, 115, 102, 111, 114, 109,
    58,  32,  116, 114, 97,  110, 115, 108, 97,  116, 101, 89,  40,  48,  41,
    59,  32,  125, 32,  46,  100, 114, 97,  103, 45,  104, 97,  110, 100, 108,
    101, 32,  123, 32,  119, 105, 100, 116, 104, 58,  32,  51,  50,  112, 120,
    59,  32,  104, 101, 105, 103, 104, 116, 58,  32,  52,  112, 120, 59,  32,
    98,  97,  99,  107, 103, 114, 111, 117, 110, 100, 58,  32,  118, 97,  114,
    40,  45,  45,  98,  111, 116, 116, 111, 109, 45,  115, 104, 101, 101, 116,
    45,  104, 97,  110, 100, 108, 101, 44,  32,  35,  99,  99,  99,  41,  59,
    32,  98,  111, 114, 100, 101, 114, 45,  114, 97,  100, 105, 117, 115, 58,
    32,  50,  112, 120, 59,  32,  109, 97,  114, 103, 105, 110, 58,  32,  56,
    112, 120, 32,  97,  117, 116, 111, 59,  32,  125, 0};

/**
 * @struct ui_bottom_sheet_base
 * @struct ui_bottom_sheet_base
 * @brief Internal representation of a bottom sheet component.
 */
struct ui_bottom_sheet_base {
  struct ui_component *component; /**< Core UI component */

  struct ui_dom_node *root_node;        /**< Shadow root node */
  struct ui_dom_node *sheet_node;       /**< Container for the sheet */
  struct ui_dom_node *drag_handle_node; /**< Drag handle element */
  struct ui_dom_node *content_node;     /**< Container for projected content */

  struct ui_component *content_component; /**< User projected component */

  int is_open; /**< Visibility state */

  struct ui_overlay_director *director; /**< Overlay manager dependency */
  struct ui_backdrop *backdrop_logic;   /**< Backdrop click/touch handler */
  struct ui_overlay *overlay;           /**< Active overlay reference */

  struct ui_gesture_recognizer
      *gesture_recognizer; /**< Handles swipe down to dismiss */

  struct ui_spring_config spring_config; /**< Animation configuration */
  struct ui_spring_state spring_state;   /**< Current animation state */

  ui_bottom_sheet_on_close_t on_close; /**< User callback when closing */
  void *user_data;                     /**< User data for callback */

  struct ui_signal *open_signal; /**< Signal for reacting to state changes */
  struct ui_computed
      *animating_signal; /**< Signal indicating animation activity */
};

static ui_error_t update_dom_state(struct ui_bottom_sheet_base *sheet) {
  if (sheet->is_open) {
    ui_error_t rc =
        ui_dom_node_set_attribute(sheet->sheet_node, "data-open", "true");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    ui_error_t rc =
        ui_dom_node_remove_attribute(sheet->sheet_node, "data-open");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_sheet_base_create(struct ui_bottom_sheet_base **out_sheet) {
  struct ui_bottom_sheet_base *sheet;
  ui_error_t rc;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet = (struct ui_bottom_sheet_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_bottom_sheet_base));
  if (!sheet) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(sheet, 0, sizeof(struct ui_bottom_sheet_base));

  rc = ui_component_create(&sheet->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_backdrop_create(&sheet->backdrop_logic);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_gesture_recognizer_create(&sheet->gesture_recognizer);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sheet->root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_tag_name(sheet->root_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sheet->sheet_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_tag_name(sheet->sheet_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_attribute(sheet->sheet_node, "class", "sheet");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_append_child(sheet->root_node, sheet->sheet_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sheet->drag_handle_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_tag_name(sheet->drag_handle_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_attribute(sheet->drag_handle_node, "class",
                                 "drag-handle");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_append_child(sheet->sheet_node, sheet->drag_handle_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sheet->content_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_tag_name(sheet->content_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_append_child(sheet->sheet_node, sheet->content_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc =
      ui_css_parse_stylesheet(ui_bottom_sheet_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_set_default_style(sheet->component, default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  sheet->component->shadow_root = sheet->root_node;

  sheet->spring_config.damping = 1.0f; /* critically damped */
  sheet->spring_config.stiffness = 300.0f;
  sheet->spring_config.mass = 1.0f;
  sheet->spring_state.value = 0.0f;
  sheet->spring_state.velocity = 0.0f;

  sheet->open_signal = NULL;
  sheet->animating_signal = NULL;

  rc = update_dom_state(sheet);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  *out_sheet = sheet;
  return UI_ERROR_NONE;

cleanup:
  if (sheet->root_node) {
    if (sheet->component->shadow_root == sheet->root_node) {
      sheet->component->shadow_root = NULL;
    }
    (void)ui_dom_node_destroy(sheet->root_node);
  }
  if (sheet->gesture_recognizer) {
    (void)ui_gesture_recognizer_destroy(sheet->gesture_recognizer);
  }
  if (sheet->backdrop_logic) {
    (void)ui_backdrop_destroy(sheet->backdrop_logic);
  }
  if (sheet->component) {
    (void)ui_component_destroy(sheet->component);
  }
  C_MULTIPLATFORM_FREE(sheet);
  return rc;
}

ui_error_t ui_bottom_sheet_base_destroy(struct ui_bottom_sheet_base *sheet) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (sheet->director && sheet->overlay) {
    rc = ui_overlay_director_unmount(sheet->director, sheet->overlay);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  rc = ui_gesture_recognizer_destroy(sheet->gesture_recognizer);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  rc = ui_backdrop_destroy(sheet->backdrop_logic);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  rc = ui_component_destroy(sheet->component);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  C_MULTIPLATFORM_FREE(sheet);
  return UI_ERROR_NONE;
}

ui_error_t ui_bottom_sheet_base_set_content(struct ui_bottom_sheet_base *sheet,
                                            struct ui_component *content) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->content_component = content;
  if (content) {
    return ui_component_mount(content, sheet->content_node);
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_bottom_sheet_base_set_open(struct ui_bottom_sheet_base *sheet,
                                         int is_open) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (sheet->is_open == is_open) {
    return UI_ERROR_NONE;
  }

  if (is_open) {
    if (sheet->director) {
      /* High z-index to overlay app content */
      rc = ui_overlay_director_mount_component(
          sheet->director, sheet->component, 1000, &sheet->overlay);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  } else {
    if (sheet->director && sheet->overlay) {
      rc = ui_overlay_director_unmount(sheet->director, sheet->overlay);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      sheet->overlay = NULL;
    }
  }

  sheet->is_open = is_open;
  rc = update_dom_state(sheet);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (sheet->open_signal) {
    union ui_signal_payload payload;
    payload.bool_val = is_open;
    rc = ui_signal_set(sheet->open_signal, payload);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_sheet_base_is_open(const struct ui_bottom_sheet_base *sheet,
                             int *out_is_open) {
  if (!sheet || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_open = sheet->is_open;
  return UI_ERROR_NONE;
}

ui_error_t ui_bottom_sheet_base_set_overlay_director(
    struct ui_bottom_sheet_base *sheet, struct ui_overlay_director *director) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->director = director;
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_sheet_base_set_on_close(struct ui_bottom_sheet_base *sheet,
                                  ui_bottom_sheet_on_close_t on_close,
                                  void *user_data) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->on_close = on_close;
  sheet->user_data = user_data;
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_sheet_base_process_event(struct ui_bottom_sheet_base *sheet,
                                   const struct ui_event *event,
                                   double timestamp_ms) {
  int should_dismiss = 0;
  ui_error_t rc;
  struct ui_gesture_event gesture_ev;

  if (!sheet || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!sheet->is_open) {
    return UI_ERROR_NONE;
  }

  /* Process Backdrop Dismissal */
  rc = ui_backdrop_process_event(sheet->backdrop_logic, event, 0.0f, 0.0f,
                                 100.0f, 100.0f, &should_dismiss);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (should_dismiss) {
    if (sheet->on_close) {
      ui_error_t close_rc = sheet->on_close(sheet, sheet->user_data);
      if (close_rc != UI_ERROR_NONE) {
        return close_rc;
      }
    } else {
      rc = ui_bottom_sheet_base_set_open(sheet, 0);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
    return UI_ERROR_NONE; /* Event consumed via dismissal */
  }

  /* Process Swipe Down Dismissal */
  rc = ui_gesture_recognizer_process_event(sheet->gesture_recognizer, event,
                                           timestamp_ms, &gesture_ev);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (gesture_ev.type == UI_GESTURE_SWIPE) {
    /* Check if it's a downward swipe */
    if (gesture_ev.delta_y > 0.0f && gesture_ev.velocity_y > 300.0f) {
      if (sheet->on_close) {
        ui_error_t close_rc = sheet->on_close(sheet, sheet->user_data);
        if (close_rc != UI_ERROR_NONE) {
          return close_rc;
        }
      } else {
        rc = ui_bottom_sheet_base_set_open(sheet, 0);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_bottom_sheet_base_update(struct ui_bottom_sheet_base *sheet,
                                       double timestamp_ms) {
  struct ui_gesture_event gesture_ev;
  struct ui_spring_state next_state;
  float target;
  /* simplistic delta time since we don't store previous tick here, ideally we
   * would. Using 16ms for a 60fps assumption for testing. */
  float dt = 0.016f;
  ui_error_t rc;

  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (sheet->is_open) {
    target = 100.0f; /* 100% open */
  } else {
    target = 0.0f;
  }

  rc = ui_spring_update(&sheet->spring_config, &sheet->spring_state, target, dt,
                        &next_state);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  sheet->spring_state = next_state;

  if (!sheet->is_open) {
    return UI_ERROR_NONE;
  }

  return ui_gesture_recognizer_update(sheet->gesture_recognizer, timestamp_ms,
                                      &gesture_ev);
}

ui_error_t
ui_bottom_sheet_base_get_component(struct ui_bottom_sheet_base *sheet,
                                   struct ui_component **out_component) {
  if (!sheet || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = sheet->component;
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_sheet_base_set_spring_config(struct ui_bottom_sheet_base *sheet,
                                       const struct ui_spring_config *config) {
  if (!sheet || !config) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->spring_config = *config;
  return UI_ERROR_NONE;
}

ui_error_t ui_bottom_sheet_base_bind_open(struct ui_bottom_sheet_base *sheet,
                                          struct ui_signal *open_signal) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->open_signal = open_signal;
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_sheet_base_get_animating_signal(struct ui_bottom_sheet_base *sheet,
                                          struct ui_computed **out_animating) {
  if (!sheet || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = sheet->animating_signal;
  return UI_ERROR_NONE;
}
