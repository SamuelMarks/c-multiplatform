/**
 * @file ui_sidenav_base.c
 * @brief ui_sidenav_base.c implementation.
 */
/* clang-format off */
#include "ui_sidenav_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include "ui_backdrop.h"
#include <stddef.h>
/* clang-format on */

/*
 * \file ui_sidenav_base.c
 * \brief Sidenav base component implementation.
 */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/** @brief Default CSS stylesheet */
static const char ui_sidenav_base_default_css[] = {
    58,  104, 111, 115, 116, 32,  123, 32,  100, 105, 115, 112, 108, 97,  121,
    58,  32,  102, 108, 101, 120, 59,  32,  111, 118, 101, 114, 102, 108, 111,
    119, 58,  32,  104, 105, 100, 100, 101, 110, 59,  32,  112, 111, 115, 105,
    116, 105, 111, 110, 58,  32,  114, 101, 108, 97,  116, 105, 118, 101, 59,
    32,  119, 105, 100, 116, 104, 58,  32,  49,  48,  48,  37,  59,  32,  104,
    101, 105, 103, 104, 116, 58,  32,  49,  48,  48,  37,  59,  32,  125, 32,
    46,  100, 114, 97,  119, 101, 114, 32,  123, 32,  100, 105, 115, 112, 108,
    97,  121, 58,  32,  102, 108, 101, 120, 59,  32,  102, 108, 101, 120, 45,
    100, 105, 114, 101, 99,  116, 105, 111, 110, 58,  32,  99,  111, 108, 117,
    109, 110, 59,  32,  112, 111, 115, 105, 116, 105, 111, 110, 58,  32,  97,
    98,  115, 111, 108, 117, 116, 101, 59,  32,  116, 111, 112, 58,  32,  48,
    59,  32,  98,  111, 116, 116, 111, 109, 58,  32,  48,  59,  32,  122, 45,
    105, 110, 100, 101, 120, 58,  32,  49,  48,  59,  32,  98,  97,  99,  107,
    103, 114, 111, 117, 110, 100, 58,  32,  118, 97,  114, 40,  45,  45,  115,
    105, 100, 101, 110, 97,  118, 45,  98,  103, 44,  32,  35,  102, 102, 102,
    41,  59,  32,  98,  111, 120, 45,  115, 104, 97,  100, 111, 119, 58,  32,
    118, 97,  114, 40,  45,  45,  115, 105, 100, 101, 110, 97,  118, 45,  115,
    104, 97,  100, 111, 119, 44,  32,  48,  32,  56,  112, 120, 32,  49,  48,
    112, 120, 32,  114, 103, 98,  97,  40,  48,  44,  48,  44,  48,  44,  48,
    46,  49,  52,  41,  41,  59,  32,  116, 114, 97,  110, 115, 105, 116, 105,
    111, 110, 58,  32,  116, 114, 97,  110, 115, 102, 111, 114, 109, 32,  48,
    46,  51,  115, 32,  101, 97,  115, 101, 59,  32,  125, 32,  46,  100, 114,
    97,  119, 101, 114, 91,  100, 97,  116, 97,  45,  112, 111, 115, 105, 116,
    105, 111, 110, 61,  34,  115, 116, 97,  114, 116, 34,  93,  32,  123, 32,
    108, 101, 102, 116, 58,  32,  48,  59,  32,  116, 114, 97,  110, 115, 102,
    111, 114, 109, 58,  32,  116, 114, 97,  110, 115, 108, 97,  116, 101, 88,
    40,  45,  49,  48,  48,  37,  41,  59,  32,  125, 32,  46,  100, 114, 97,
    119, 101, 114, 91,  100, 97,  116, 97,  45,  112, 111, 115, 105, 116, 105,
    111, 110, 61,  34,  101, 110, 100, 34,  93,  32,  123, 32,  114, 105, 103,
    104, 116, 58,  32,  48,  59,  32,  116, 114, 97,  110, 115, 102, 111, 114,
    109, 58,  32,  116, 114, 97,  110, 115, 108, 97,  116, 101, 88,  40,  49,
    48,  48,  37,  41,  59,  32,  125, 32,  46,  100, 114, 97,  119, 101, 114,
    91,  100, 97,  116, 97,  45,  111, 112, 101, 110, 61,  34,  116, 114, 117,
    101, 34,  93,  32,  123, 32,  116, 114, 97,  110, 115, 102, 111, 114, 109,
    58,  32,  116, 114, 97,  110, 115, 108, 97,  116, 101, 88,  40,  48,  41,
    59,  32,  125, 32,  46,  100, 114, 97,  119, 101, 114, 91,  100, 97,  116,
    97,  45,  109, 111, 100, 101, 61,  34,  115, 105, 100, 101, 34,  93,  32,
    123, 32,  112, 111, 115, 105, 116, 105, 111, 110, 58,  32,  114, 101, 108,
    97,  116, 105, 118, 101, 59,  32,  98,  111, 120, 45,  115, 104, 97,  100,
    111, 119, 58,  32,  110, 111, 110, 101, 59,  32,  98,  111, 114, 100, 101,
    114, 45,  114, 105, 103, 104, 116, 58,  32,  49,  112, 120, 32,  115, 111,
    108, 105, 100, 32,  118, 97,  114, 40,  45,  45,  115, 105, 100, 101, 110,
    97,  118, 45,  98,  111, 114, 100, 101, 114, 44,  32,  35,  99,  99,  99,
    41,  59,  32,  125, 32,  46,  100, 114, 97,  119, 101, 114, 91,  100, 97,
    116, 97,  45,  109, 111, 100, 101, 61,  34,  115, 105, 100, 101, 34,  93,
    91,  100, 97,  116, 97,  45,  112, 111, 115, 105, 116, 105, 111, 110, 61,
    34,  101, 110, 100, 34,  93,  32,  123, 32,  98,  111, 114, 100, 101, 114,
    45,  114, 105, 103, 104, 116, 58,  32,  110, 111, 110, 101, 59,  32,  98,
    111, 114, 100, 101, 114, 45,  108, 101, 102, 116, 58,  32,  49,  112, 120,
    32,  115, 111, 108, 105, 100, 32,  118, 97,  114, 40,  45,  45,  115, 105,
    100, 101, 110, 97,  118, 45,  98,  111, 114, 100, 101, 114, 44,  32,  35,
    99,  99,  99,  41,  59,  32,  125, 32,  46,  100, 114, 97,  119, 101, 114,
    91,  100, 97,  116, 97,  45,  109, 111, 100, 101, 61,  34,  112, 117, 115,
    104, 34,  93,  32,  123, 32,  112, 111, 115, 105, 116, 105, 111, 110, 58,
    32,  114, 101, 108, 97,  116, 105, 118, 101, 59,  32,  125, 32,  46,  109,
    97,  105, 110, 45,  99,  111, 110, 116, 101, 110, 116, 32,  123, 32,  102,
    108, 101, 120, 58,  32,  49,  32,  49,  32,  97,  117, 116, 111, 59,  32,
    112, 111, 115, 105, 116, 105, 111, 110, 58,  32,  114, 101, 108, 97,  116,
    105, 118, 101, 59,  32,  122, 45,  105, 110, 100, 101, 120, 58,  32,  49,
    59,  32,  100, 105, 115, 112, 108, 97,  121, 58,  32,  98,  108, 111, 99,
    107, 59,  32,  111, 118, 101, 114, 102, 108, 111, 119, 58,  32,  97,  117,
    116, 111, 59,  32,  125, 0};

/** @brief Default CSS stylesheet for backdrop */
static const char *ui_sidenav_backdrop_css =
    ":host { "
    "position: absolute; "
    "top: 0; left: 0; right: 0; bottom: 0; "
    "background: var(--sidenav-backdrop-color, rgba(0,0,0,0.6)); "
    "z-index: 5; "
    "}";

/**
 * @struct ui_sidenav_base
 * \brief ui_sidenav_base structure.
 * \details Internal state for the sidenav base component.
 */
struct ui_sidenav_base {
  struct ui_component *component; /**< component */

  struct ui_dom_node *root_node;   /**< root_node */
  struct ui_dom_node *drawer_node; /**< drawer_node */
  struct ui_dom_node *main_node;   /**< main_node */

  struct ui_component *drawer_content; /**< drawer_content */
  struct ui_component *main_content;   /**< main_content */

  enum ui_sidenav_mode mode;         /**< mode */
  enum ui_sidenav_position position; /**< position */
  int is_open;                       /**< is_open */

  struct ui_overlay_director *director;    /**< director */
  struct ui_backdrop *backdrop_logic;      /**< backdrop_logic */
  struct ui_component *backdrop_component; /**< backdrop_component */
  struct ui_overlay *backdrop_overlay;     /**< backdrop_overlay */

  ui_sidenav_on_close_t on_close;        /**< on_close */
  void *user_data;                       /**< user_data */
  struct ui_signal *active_index_signal; /**< active_index_signal */
};

/**
 * \brief Updates the DOM state for the sidenav.
 * \param sidenav The sidenav component.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief update_dom_state.
 * @param sidenav Parameter sidenav.
 * @return Return value.
 */
static ui_error_t update_dom_state(struct ui_sidenav_base *sidenav) {
  const char *mode_str = "over";
  const char *pos_str = "start";

  if (sidenav->mode == UI_SIDENAV_MODE_PUSH)
    mode_str = "push";
  else if (sidenav->mode == UI_SIDENAV_MODE_SIDE)
    mode_str = "side";

  if (sidenav->position == UI_SIDENAV_POSITION_END)
    pos_str = "end";

/** @cond */
#define UI_DOM_SET_ATTR_IGNORE(n, a, v) ui_dom_node_set_attribute((n), (a), (v))
/** @endcond */
/** @cond */
#define UI_DOM_REM_ATTR_IGNORE(n, a) ui_dom_node_remove_attribute((n), (a))
/** @endcond */
/** @cond */
#define UI_DOM_APP_CHILD_IGNORE(p, c) ui_dom_node_append_child((p), (c))
  /** @endcond */

  (void)UI_DOM_SET_ATTR_IGNORE(sidenav->drawer_node, "data-mode", mode_str);
  (void)UI_DOM_SET_ATTR_IGNORE(sidenav->drawer_node, "data-position", pos_str);

  if (sidenav->is_open) {
    (void)UI_DOM_SET_ATTR_IGNORE(sidenav->drawer_node, "data-open", "true");
    (void)UI_DOM_SET_ATTR_IGNORE(sidenav->root_node, "data-mode", mode_str);
    (void)UI_DOM_SET_ATTR_IGNORE(sidenav->root_node, "data-open", "true");
    (void)UI_DOM_SET_ATTR_IGNORE(sidenav->root_node, "data-position", pos_str);
  } else {
    (void)UI_DOM_REM_ATTR_IGNORE(sidenav->drawer_node, "data-open");
    (void)UI_DOM_REM_ATTR_IGNORE(sidenav->root_node, "data-open");
  }

  /* Adjust DOM ordering for SIDE/PUSH modes so flex layout works properly
   * without absolute positioning */
  if ((sidenav->mode == UI_SIDENAV_MODE_SIDE ||
       sidenav->mode == UI_SIDENAV_MODE_PUSH) &&
      sidenav->is_open) {
    if (sidenav->position == UI_SIDENAV_POSITION_START) {
      (void)UI_DOM_APP_CHILD_IGNORE(sidenav->root_node, sidenav->drawer_node);
      (void)UI_DOM_APP_CHILD_IGNORE(sidenav->root_node, sidenav->main_node);
    } else {
      (void)UI_DOM_APP_CHILD_IGNORE(sidenav->root_node, sidenav->main_node);
      (void)UI_DOM_APP_CHILD_IGNORE(sidenav->root_node, sidenav->drawer_node);
    }
  } else {
    /* Default overlay rendering order (drawer on top) */
    (void)UI_DOM_APP_CHILD_IGNORE(sidenav->root_node, sidenav->main_node);
    (void)UI_DOM_APP_CHILD_IGNORE(sidenav->root_node, sidenav->drawer_node);
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Mounts the backdrop for the sidenav.
 * \param sidenav The sidenav component.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief mount_backdrop.
 * @param sidenav Parameter sidenav.
 * @return Return value.
 */
static ui_error_t mount_backdrop(struct ui_sidenav_base *sidenav) {
  ui_error_t rc;
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
    rc = ui_dom_node_set_tag_name(b_root, "div");
    if (rc != UI_ERROR_NONE)
      return rc;

    rc = ui_css_parse_stylesheet(ui_sidenav_backdrop_css, &b_style);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    {
      ui_error_t set_rc =
          ui_component_set_default_style(sidenav->backdrop_component, b_style);
      (void)set_rc;
    }

    sidenav->backdrop_component->shadow_root = b_root;
  }

  /* Mount at z-index 5, which is below the drawer's z-index 10 but above main
   * content */
  return ui_overlay_director_mount_component(sidenav->director,
                                             sidenav->backdrop_component, 5,
                                             &sidenav->backdrop_overlay);
}

/**
 * \brief Unmounts the backdrop for the sidenav.
 * \param sidenav The sidenav component.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief unmount_backdrop.
 * @param sidenav Parameter sidenav.
 * @return Return value.
 */
static ui_error_t unmount_backdrop(struct ui_sidenav_base *sidenav) {
  if (sidenav->director && sidenav->backdrop_overlay) {
    ui_error_t rc = ui_overlay_director_unmount(sidenav->director,
                                                sidenav->backdrop_overlay);
    if (rc != UI_ERROR_NONE)
      return rc;
    sidenav->backdrop_overlay = NULL;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Creates a new sidenav base component.
 * \param out_sidenav Pointer to store the created component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_create(struct ui_sidenav_base **out_sidenav) {
  struct ui_sidenav_base *sidenav;
  ui_error_t rc;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_sidenav)
    return UI_ERROR_INVALID_ARGUMENT;

  sidenav = (struct ui_sidenav_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_sidenav_base));
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
  rc = ui_dom_node_set_tag_name(sidenav->root_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sidenav->main_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_tag_name(sidenav->main_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(sidenav->main_node, "class", "main-content");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_append_child(sidenav->root_node, sidenav->main_node);
  (void)rc;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sidenav->drawer_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_tag_name(sidenav->drawer_node, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(sidenav->drawer_node, "class", "drawer");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_append_child(sidenav->root_node, sidenav->drawer_node);
  (void)rc;

  rc = ui_css_parse_stylesheet(ui_sidenav_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  {

    ui_error_t _ign_rc =
        ui_component_set_default_style(sidenav->component, default_style);

    (void)_ign_rc;
  }

  sidenav->component->shadow_root = sidenav->root_node;
  sidenav->mode = UI_SIDENAV_MODE_OVER;
  sidenav->position = UI_SIDENAV_POSITION_START;

/** @cond */
#define UI_UPDATE_DOM_STATE_IGNORE(s) update_dom_state((s))
  /** @endcond */
  (void)UI_UPDATE_DOM_STATE_IGNORE(sidenav);

  *out_sidenav = sidenav;
  return UI_ERROR_NONE;

cleanup: {
  ui_error_t rc_cleanup = ui_dom_node_destroy(sidenav->root_node);
  if (rc_cleanup != UI_ERROR_NONE) {
    (void)rc_cleanup; /* Avoid override */
  }
}
  {
    ui_error_t rc_cleanup = ui_backdrop_destroy(sidenav->backdrop_logic);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(sidenav->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  C_MULTIPLATFORM_FREE(sidenav);
  return rc;
}

/**
 * \brief Destroys a sidenav base component.
 * \param sidenav The component to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_destroy(struct ui_sidenav_base *sidenav) {
  if (!sidenav)
    return UI_ERROR_NONE;
  {
    ui_error_t rc = unmount_backdrop(sidenav);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(sidenav->backdrop_component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_backdrop_destroy(sidenav->backdrop_logic);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(sidenav->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  C_MULTIPLATFORM_FREE(sidenav);
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the mode of the sidenav.
 * \param sidenav The sidenav component.
 * \param mode The mode to set.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_set_mode(struct ui_sidenav_base *sidenav,
                                    enum ui_sidenav_mode mode) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  sidenav->mode = mode;
/** @cond */
#define UI_UPDATE_DOM_STATE_IGNORE(s) update_dom_state((s))
  /** @endcond */
  (void)UI_UPDATE_DOM_STATE_IGNORE(sidenav);
  if (sidenav->mode != UI_SIDENAV_MODE_OVER && sidenav->is_open) {
    ui_error_t rc = unmount_backdrop(sidenav);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (sidenav->mode == UI_SIDENAV_MODE_OVER && sidenav->is_open) {
    ui_error_t rc = mount_backdrop(sidenav);
    (void)rc;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the position of the sidenav.
 * \param sidenav The sidenav component.
 * \param position The position to set.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_set_position(struct ui_sidenav_base *sidenav,
                                        enum ui_sidenav_position position) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  sidenav->position = position;
/** @cond */
#define UI_UPDATE_DOM_STATE_IGNORE(s) update_dom_state((s))
  /** @endcond */
  (void)UI_UPDATE_DOM_STATE_IGNORE(sidenav);
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the drawer content component.
 * \param sidenav The sidenav component.
 * \param content The drawer content component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_set_drawer_content(struct ui_sidenav_base *sidenav,
                                              struct ui_component *content) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  sidenav->drawer_content = content;
  if (content) {
    return ui_component_mount(content, sidenav->drawer_node);
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the main content component.
 * \param sidenav The sidenav component.
 * \param content The main content component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_set_main_content(struct ui_sidenav_base *sidenav,
                                            struct ui_component *content) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  sidenav->main_content = content;
  if (content) {
    return ui_component_mount(content, sidenav->main_node);
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Sets whether the sidenav is open.
 * \param sidenav The sidenav component.
 * \param is_open 1 to open, 0 to close.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_set_open(struct ui_sidenav_base *sidenav,
                                    int is_open) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  if (sidenav->is_open == is_open)
    return UI_ERROR_NONE;

  sidenav->is_open = is_open;
/** @cond */
#define UI_UPDATE_DOM_STATE_IGNORE(s) update_dom_state((s))
  /** @endcond */
  (void)UI_UPDATE_DOM_STATE_IGNORE(sidenav);

  if (is_open && (sidenav->mode == UI_SIDENAV_MODE_OVER ||
                  sidenav->mode == UI_SIDENAV_MODE_PUSH)) {
    rc = mount_backdrop(sidenav);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else {
    rc = unmount_backdrop(sidenav);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Checks if the sidenav is open.
 * \param sidenav The sidenav component.
 * \param out_is_open Pointer to store the result.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_is_open(const struct ui_sidenav_base *sidenav,
                                   int *out_is_open) {
  if (!sidenav || !out_is_open)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_is_open = sidenav->is_open;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the overlay director.
 * \param sidenav The sidenav component.
 * \param director The overlay director.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_sidenav_base_set_overlay_director(struct ui_sidenav_base *sidenav,
                                     struct ui_overlay_director *director) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  sidenav->director = director;
  if (sidenav->is_open && sidenav->mode == UI_SIDENAV_MODE_OVER) {
    ui_error_t rc = mount_backdrop(sidenav);
    (void)rc;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the on close callback.
 * \param sidenav The sidenav component.
 * \param on_close The callback function.
 * \param user_data User data for the callback.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_set_on_close(struct ui_sidenav_base *sidenav,
                                        ui_sidenav_on_close_t on_close,
                                        void *user_data) {
  if (!sidenav)
    return UI_ERROR_INVALID_ARGUMENT;
  sidenav->on_close = on_close;
  sidenav->user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * \brief Processes an event for the sidenav.
 * \param sidenav The sidenav component.
 * \param event The event to process.
 * \param timestamp_ms The timestamp.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_process_event(struct ui_sidenav_base *sidenav,
                                         const struct ui_event *event,
                                         double timestamp_ms) {
  int should_dismiss = 0;

  (void)timestamp_ms;

  if (!sidenav || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!sidenav->is_open || sidenav->mode != UI_SIDENAV_MODE_OVER) {
    return UI_ERROR_NONE;
  }

  /* For headless testing, pass dummy drawer bounds. In a real engine, layout
   * tree geometry would be used. */
  {
    ui_error_t _ign_rc =
        ui_backdrop_process_event(sidenav->backdrop_logic, event, 0.0f, 0.0f,
                                  0.0f, 0.0f, &should_dismiss);
    (void)_ign_rc;
  }

  if (should_dismiss) {
    if (sidenav->on_close) {
      ui_error_t close_rc = sidenav->on_close(sidenav, sidenav->user_data);
      if (close_rc != UI_ERROR_NONE)
        return close_rc;
    } else {
      ui_error_t open_rc = ui_sidenav_base_set_open(sidenav, 0);
      if (open_rc != UI_ERROR_NONE)
        return open_rc;
    }
  }

  return UI_ERROR_NONE;
}
/**
 * \brief Gets the base component for the sidenav.
 * \param sidenav The sidenav component.
 * \param out_component Pointer to store the component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_get_component(struct ui_sidenav_base *sidenav,
                                         struct ui_component **out_component) {
  if (!sidenav || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = sidenav->component;
  return UI_ERROR_NONE;
}

/**
 * \brief Binds the active index to a signal.
 * \param widget The sidenav component.
 * \param signal The signal to bind.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_sidenav_base_bind_active_index(struct ui_sidenav_base *widget,
                                             struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
