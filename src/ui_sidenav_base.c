/* clang-format off */
#include "ui_sidenav_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include "ui_backdrop.h"
#include <stddef.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_sidenav_base_default_css =
    ":host { "
    "display: flex; "
    "overflow: hidden; "
    "position: relative; "
    "width: 100%; "
    "height: 100%; "
    "} "
    ".drawer { "
    "display: flex; "
    "flex-direction: column; "
    "position: absolute; "
    "top: 0; bottom: 0; "
    "z-index: 10; "
    "background: var(--sidenav-bg, #fff); "
    "box-shadow: var(--sidenav-shadow, 0 8px 10px rgba(0,0,0,0.14)); "
    "transition: transform 0.3s ease; "
    "} "
    ".drawer[data-position=\"start\"] { left: 0; transform: translateX(-100%); "
    "} "
    ".drawer[data-position=\"end\"] { right: 0; transform: translateX(100%); } "
    ".drawer[data-open=\"true\"] { transform: translateX(0); } "
    ".drawer[data-mode=\"side\"] { "
    "position: relative; "
    "box-shadow: none; "
    "border-right: 1px solid var(--sidenav-border, #ccc); "
    "} "
    ".drawer[data-mode=\"side\"][data-position=\"end\"] { "
    "border-right: none; "
    "border-left: 1px solid var(--sidenav-border, #ccc); "
    "} "
    ".drawer[data-mode=\"push\"] { "
    "position: relative; "
    "} "
    ".main-content { "
    "flex: 1 1 auto; "
    "position: relative; "
    "z-index: 1; "
    "display: block; "
    "overflow: auto; "
    "}";

static const char *ui_sidenav_backdrop_css =
    ":host { "
    "position: absolute; "
    "top: 0; left: 0; right: 0; bottom: 0; "
    "background: var(--sidenav-backdrop-color, rgba(0,0,0,0.6)); "
    "z-index: 5; "
    "}";

/** \brief ui_sidenav_base */
struct ui_sidenav_base {
  struct ui_component *component;

  struct ui_dom_node *root_node;
  struct ui_dom_node *drawer_node;
  struct ui_dom_node *main_node;

  struct ui_component *drawer_content;
  struct ui_component *main_content;

  enum ui_sidenav_mode mode;
  enum ui_sidenav_position position;
  int is_open;

  struct ui_overlay_director *director;
  struct ui_backdrop *backdrop_logic;
  struct ui_component *backdrop_component;
  struct ui_overlay *backdrop_overlay;

  ui_sidenav_on_close_t on_close;
  void *user_data;
  struct ui_signal *active_index_signal;
};

static enum ui_error update_dom_state(struct ui_sidenav_base *sidenav) {
  const char *mode_str = "over";
  const char *pos_str = "start";

  if (!sidenav || !sidenav->drawer_node)
    return UI_ERROR_NONE;

  if (sidenav->mode == UI_SIDENAV_MODE_PUSH)
    mode_str = "push";
  else if (sidenav->mode == UI_SIDENAV_MODE_SIDE)
    mode_str = "side";

  if (sidenav->position == UI_SIDENAV_POSITION_END)
    pos_str = "end";

  ui_dom_node_set_attribute(sidenav->drawer_node, "data-mode", mode_str);
  ui_dom_node_set_attribute(sidenav->drawer_node, "data-position", pos_str);

  if (sidenav->is_open) {
    ui_dom_node_set_attribute(sidenav->drawer_node, "data-open", "true");
    ui_dom_node_set_attribute(sidenav->root_node, "data-mode", mode_str);
    ui_dom_node_set_attribute(sidenav->root_node, "data-open", "true");
    ui_dom_node_set_attribute(sidenav->root_node, "data-position", pos_str);
  } else {
    ui_dom_node_remove_attribute(sidenav->drawer_node, "data-open");
    ui_dom_node_remove_attribute(sidenav->root_node, "data-open");
  }

  /* Adjust DOM ordering for SIDE/PUSH modes so flex layout works properly
   * without absolute positioning */
  if ((sidenav->mode == UI_SIDENAV_MODE_SIDE ||
       sidenav->mode == UI_SIDENAV_MODE_PUSH) &&
      sidenav->is_open) {
    if (sidenav->position == UI_SIDENAV_POSITION_START) {
      ui_dom_node_append_child(sidenav->root_node, sidenav->drawer_node);
      ui_dom_node_append_child(sidenav->root_node, sidenav->main_node);
    } else {
      ui_dom_node_append_child(sidenav->root_node, sidenav->main_node);
      ui_dom_node_append_child(sidenav->root_node, sidenav->drawer_node);
    }
  } else {
    /* Default overlay rendering order (drawer on top) */
    ui_dom_node_append_child(sidenav->root_node, sidenav->main_node);
    ui_dom_node_append_child(sidenav->root_node, sidenav->drawer_node);
  }
  return UI_ERROR_NONE;
}

static enum ui_error mount_backdrop(struct ui_sidenav_base *sidenav) {
  enum ui_error rc;
  if (!sidenav->director || sidenav->backdrop_overlay)
    return UI_ERROR_NONE;

  if (!sidenav->backdrop_component) {
    struct ui_css_stylesheet *b_style = NULL;
    struct ui_dom_node *b_root = NULL;

    rc = ui_component_create(&sidenav->backdrop_component);
    if (rc != UI_ERROR_NONE)
      return rc;

    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &b_root);
    if (rc != UI_ERROR_NONE)
      return rc;
    ui_dom_node_set_tag_name(b_root, "div");

    rc = ui_css_parse_stylesheet(ui_sidenav_backdrop_css, &b_style);
    if (rc == UI_ERROR_NONE) {
      ui_component_set_default_style(sidenav->backdrop_component, b_style);
    }

    sidenav->backdrop_component->shadow_root = b_root;
  }

  /* Mount at z-index 5, which is below the drawer's z-index 10 but above main
   * content */
  return ui_overlay_director_mount_component(sidenav->director,
                                             sidenav->backdrop_component, 5,
                                             &sidenav->backdrop_overlay);
}

static enum ui_error unmount_backdrop(struct ui_sidenav_base *sidenav) {
  if (sidenav->director && sidenav->backdrop_overlay) {
    ui_overlay_director_unmount(sidenav->director, sidenav->backdrop_overlay);
    sidenav->backdrop_overlay = NULL;
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_sidenav_base_create(struct ui_sidenav_base **out_sidenav) {
  struct ui_sidenav_base *sidenav;
  enum ui_error rc;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_sidenav)
    return UI_ERROR_INVALID_ARGUMENT;

  sidenav = (struct ui_sidenav_base *)UI_MALLOC(sizeof(struct ui_sidenav_base));
  if (!sidenav)
    return UI_ERROR_OUT_OF_MEMORY;

  memset(sidenav, 0, sizeof(struct ui_sidenav_base));

  rc = ui_component_create(&sidenav->component);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_backdrop_create(&sidenav->backdrop_logic);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sidenav->root_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(sidenav->root_node, "div");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sidenav->main_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(sidenav->main_node, "div");
  ui_dom_node_set_attribute(sidenav->main_node, "class", "main-content");
  ui_dom_node_append_child(sidenav->root_node, sidenav->main_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sidenav->drawer_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(sidenav->drawer_node, "div");
  ui_dom_node_set_attribute(sidenav->drawer_node, "class", "drawer");
  ui_dom_node_append_child(sidenav->root_node, sidenav->drawer_node);

  rc = ui_css_parse_stylesheet(ui_sidenav_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_component_set_default_style(sidenav->component, default_style);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(default_style);
    goto cleanup;
  }

  sidenav->component->shadow_root = sidenav->root_node;
  sidenav->mode = UI_SIDENAV_MODE_OVER;
  sidenav->position = UI_SIDENAV_POSITION_START;

  (void)update_dom_state(sidenav);

  *out_sidenav = sidenav;
  return UI_ERROR_NONE;

cleanup:
  if (sidenav->root_node)
    ui_dom_node_destroy(sidenav->root_node);
  if (sidenav->backdrop_logic)
    ui_backdrop_destroy(sidenav->backdrop_logic);
  if (sidenav->component)
    ui_component_destroy(sidenav->component);
  UI_FREE(sidenav);
  return rc;
}

enum ui_error ui_sidenav_base_destroy(struct ui_sidenav_base *sidenav) {
  if (!sidenav)
    return UI_ERROR_NONE;
  (void)unmount_backdrop(sidenav);
  if (sidenav->backdrop_component)
    ui_component_destroy(sidenav->backdrop_component);
  if (sidenav->backdrop_logic)
    ui_backdrop_destroy(sidenav->backdrop_logic);
  if (sidenav->component)
    ui_component_destroy(sidenav->component);
  UI_FREE(sidenav);
  return UI_ERROR_NONE;
}

enum ui_error ui_sidenav_base_set_mode(struct ui_sidenav_base *sidenav,
                                       enum ui_sidenav_mode mode) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  sidenav->mode = mode;
  (void)update_dom_state(sidenav);
  if (sidenav->mode != UI_SIDENAV_MODE_OVER && sidenav->is_open) {
    (void)unmount_backdrop(sidenav);
  } else if (sidenav->mode == UI_SIDENAV_MODE_OVER && sidenav->is_open) {
    mount_backdrop(sidenav);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_sidenav_base_set_position(struct ui_sidenav_base *sidenav,
                                           enum ui_sidenav_position position) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  sidenav->position = position;
  (void)update_dom_state(sidenav);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_sidenav_base_set_drawer_content(struct ui_sidenav_base *sidenav,
                                   struct ui_component *content) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  sidenav->drawer_content = content;
  if (content) {
    return ui_component_mount(content, sidenav->drawer_node);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_sidenav_base_set_main_content(struct ui_sidenav_base *sidenav,
                                               struct ui_component *content) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  sidenav->main_content = content;
  if (content) {
    return ui_component_mount(content, sidenav->main_node);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_sidenav_base_set_open(struct ui_sidenav_base *sidenav,
                                       int is_open) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  if (sidenav->is_open == is_open)
    return UI_ERROR_NONE;

  sidenav->is_open = is_open;
  (void)update_dom_state(sidenav);

  if (is_open && sidenav->mode == UI_SIDENAV_MODE_OVER) {
    mount_backdrop(sidenav);
  } else {
    (void)unmount_backdrop(sidenav);
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_sidenav_base_is_open(const struct ui_sidenav_base *sidenav,
                                      int *out_is_open) {
  if (!sidenav || !out_is_open)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_is_open = sidenav->is_open;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_sidenav_base_set_overlay_director(struct ui_sidenav_base *sidenav,
                                     struct ui_overlay_director *director) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  sidenav->director = director;
  if (sidenav->is_open && sidenav->mode == UI_SIDENAV_MODE_OVER) {
    mount_backdrop(sidenav);
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_sidenav_base_set_on_close(struct ui_sidenav_base *sidenav,
                                           ui_sidenav_on_close_t on_close,
                                           void *user_data) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  sidenav->on_close = on_close;
  sidenav->user_data = user_data;
  return UI_ERROR_NONE;
}

enum ui_error ui_sidenav_base_process_event(struct ui_sidenav_base *sidenav,
                                            const struct ui_event *event,
                                            double timestamp_ms) {
  int should_dismiss = 0;
  enum ui_error rc;

  (void)timestamp_ms;

  if (!sidenav || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!sidenav->is_open || sidenav->mode != UI_SIDENAV_MODE_OVER ||
      !sidenav->backdrop_logic) {
    return UI_ERROR_NONE;
  }

  /* For headless testing, pass dummy drawer bounds. In a real engine, layout
   * tree geometry would be used. */
  rc = ui_backdrop_process_event(sidenav->backdrop_logic, event, 0.0f, 0.0f,
                                 0.0f, 0.0f, &should_dismiss);
  if (rc != UI_ERROR_NONE)
    return rc;

  if (should_dismiss) {
    if (sidenav->on_close) {
      sidenav->on_close(sidenav, sidenav->user_data);
    } else {
      ui_sidenav_base_set_open(sidenav, 0);
    }
  }

  return UI_ERROR_NONE;
}
/** \brief ui_error */
enum ui_error
ui_sidenav_base_get_component(struct ui_sidenav_base *sidenav,
                              struct ui_component **out_component) {
  if (!sidenav || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = sidenav->component;
  return UI_ERROR_NONE;
}

enum ui_error ui_sidenav_base_bind_active_index(struct ui_sidenav_base *widget,
                                                struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
