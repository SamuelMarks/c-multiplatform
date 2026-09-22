/**
 * @file ui_menu_base.c
 * @brief Implementation of the UI menu base component.
 * @details Manages menu states, submenus, and keyboard navigation.
 */
/* clang-format off */
#include "ui_menu_base.h"
#include "ui_aria.h"
#include "ui_css_parser.h"
#include "ui_internal_mem.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

#ifdef UI_TEST_MOCK_ALLOC
int g_menu_mock_fail = 0;
int g_menu_mock_attr_countdown = -1;
int g_menu_mock_append_countdown = -1;

static ui_error_t mock_menu_dom_node_set_attribute(struct ui_dom_node *node,
                                                   const char *name,
                                                   const char *value) {
  if (g_menu_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_menu_mock_attr_countdown == 0) {
    g_menu_mock_attr_countdown = -1;
    return UI_ERROR_UNKNOWN;
  }
  if (g_menu_mock_attr_countdown > 0) {
    g_menu_mock_attr_countdown--;
  }
  return (ui_dom_node_set_attribute)(node, name, value);
}
#undef ui_dom_node_set_attribute
/** @cond */
#define ui_dom_node_set_attribute mock_menu_dom_node_set_attribute
/** @endcond */

static ui_error_t
mock_menu_css_parse_stylesheet(const char *css,
                               struct ui_css_stylesheet **out_sheet) {
  if (g_menu_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_css_parse_stylesheet)(css, out_sheet);
}
#undef ui_css_parse_stylesheet
/** @cond */
#define ui_css_parse_stylesheet mock_menu_css_parse_stylesheet
/** @endcond */

static ui_error_t
mock_menu_component_set_default_style(struct ui_component *comp,
                                      struct ui_css_stylesheet *sheet) {
  if (g_menu_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_set_default_style)(comp, sheet);
}
#undef ui_component_set_default_style
/** @cond */
#define ui_component_set_default_style mock_menu_component_set_default_style
/** @endcond */

static ui_error_t mock_menu_dom_node_append_child(struct ui_dom_node *parent,
                                                  struct ui_dom_node *child) {
  if (g_menu_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_menu_dom_node_append_child
/** @endcond */

static ui_error_t mock_menu_overlay_mount(struct ui_overlay_director *director,
                                          struct ui_component *comp,
                                          int z_index,
                                          struct ui_overlay **out_handle) {
  if (g_menu_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_overlay_director_mount_component)(director, comp, z_index,
                                               out_handle);
}
#undef ui_overlay_director_mount_component
/** @cond */
#define ui_overlay_director_mount_component mock_menu_overlay_mount
/** @endcond */

static ui_error_t
mock_menu_overlay_unmount(struct ui_overlay_director *director,
                          struct ui_overlay *handle) {
  if (g_menu_mock_fail == 6) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_overlay_director_unmount)(director, handle);
}
#undef ui_overlay_director_unmount
/** @cond */
#define ui_overlay_director_unmount mock_menu_overlay_unmount
/** @endcond */
#endif

/** @brief Default CSS stylesheet for menus */
static const char *ui_menu_base_default_css =
    ".ui-menu { display: flex; flex-direction: column; position: absolute; "
    "background-color: #fff; border: 1px solid #ccc; z-index: 1000; } "
    ".ui-menu-item { padding: 4px 8px; cursor: pointer; display: flex; "
    "justify-content: space-between; } "
    ".ui-menu-item[data-active=\"true\"] { background-color: #eee; }";

/**
 * @struct ui_menu_item_entry
 * @brief Structure tracking a single item in a menu.
 */
struct ui_menu_item_entry {
  char *id;                     /**< Item ID */
  struct ui_dom_node *node;     /**< DOM node associated with the item */
  struct ui_menu_base *submenu; /**< Submenu linked to this item, if any */
};

/**
 * @struct ui_menu_base
 * @brief State and DOM mapping for a menu, supporting nested submenus.
 */
struct ui_menu_base {
  struct ui_component *component;       /**< UI component */
  struct ui_dom_node *container_node;   /**< Container DOM node */
  struct ui_overlay *overlay_handle;    /**< Overlay handle when open */
  struct ui_overlay_director *director; /**< Overlay director */

  struct ui_menu_item_entry *items; /**< Menu items array */
  int item_count;                   /**< Current number of items */
  int item_capacity;                /**< Allocated capacity of items */

  int active_index; /**< Currently active index for keyboard nav */
  int is_open;      /**< Indicates if the menu is open */
  int last_x;       /**< Last known X coordinate */
  int last_y;       /**< Last known Y coordinate */

  struct ui_menu_base *parent_menu; /**< Parent menu reference */

  ui_menu_on_action_t on_action;         /**< Action callback */
  void *user_data;                       /**< User data for callback */
  struct ui_signal *active_index_signal; /**< Signal for the active index */
};

/**
 * @brief Creates a new menu base component.
 * @param[out] out_menu Pointer to store the created menu.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_create(struct ui_menu_base **out_menu) {
  struct ui_menu_base *menu;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_menu)
    return UI_ERROR_INVALID_ARGUMENT;

  menu = (struct ui_menu_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_menu_base));
  if (!menu)
    return UI_ERROR_OUT_OF_MEMORY;

  menu->component = NULL;
  menu->container_node = NULL;
  menu->overlay_handle = NULL;
  menu->director = NULL;
  menu->items = NULL;
  menu->item_count = 0;
  menu->item_capacity = 0;
  menu->active_index = -1;
  menu->is_open = 0;
  menu->last_x = 0;
  menu->last_y = 0;
  menu->parent_menu = NULL;
  menu->on_action = NULL;
  menu->user_data = NULL;

  rc = ui_component_create(&menu->component);
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
  rc = ui_dom_node_set_attribute(root_node, "class", "ui-menu");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_attribute(root_node, "role", "menu");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  menu->container_node = root_node;

  rc = ui_css_parse_stylesheet(ui_menu_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_set_default_style(menu->component, default_style);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(default_style);
    goto cleanup;
  }

  menu->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component */

  *out_menu = menu;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    ui_dom_node_destroy(root_node);
  }
  if (menu->component) {
    ui_component_destroy(menu->component);
  }
  C_MULTIPLATFORM_FREE(menu);
  return rc;
}

/**
 * @brief Destroys a menu base component.
 * @param[in,out] menu The menu to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_destroy(struct ui_menu_base *menu) {
  int i;
  if (!menu)
    return UI_ERROR_NONE;

  ui_menu_base_close(menu);

  for (i = 0; i < menu->item_count; i++) {
    C_MULTIPLATFORM_FREE(menu->items[i].id);
  }
  if (menu->items) {
    C_MULTIPLATFORM_FREE(menu->items);
  }

  ui_component_destroy(menu->component);

  C_MULTIPLATFORM_FREE(menu);
  return UI_ERROR_NONE;
}

/**
 * @brief Helper to dynamically duplicate a string.
 * @param[in] src The string to duplicate.
 * @param[out] out_str Pointer to store the newly allocated copy.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t duplicate_string(const char *src, char **out_str) {
  size_t len;
  char *dst;
  *out_str = NULL;
  len = strlen(src);
  dst = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!dst)
    return UI_ERROR_OUT_OF_MEMORY;
#if defined(_MSC_VER)
  strcpy_s(dst, len + 1, src);
#else
  strcpy(dst, src);
#endif
  *out_str = dst;
  return UI_ERROR_NONE;
}

/**
 * @brief Adds an item to the menu, optionally marking it as a submenu trigger.
 * @param[in,out] menu The menu to modify.
 * @param[in] item_id The logical ID of the item.
 * @param[in,out] label_node The DOM node representing the visual item.
 * @param[in,out] submenu Optional submenu component to open when this item is
 * active.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_add_item(struct ui_menu_base *menu, const char *item_id,
                                 struct ui_dom_node *label_node,
                                 struct ui_menu_base *submenu) {
  ui_error_t rc;
  struct ui_menu_item_entry *new_items;

  if (!menu || !item_id || !label_node)
    return UI_ERROR_INVALID_ARGUMENT;

  if (menu->item_count >= menu->item_capacity) {
    int new_cap = menu->item_capacity == 0 ? 4 : menu->item_capacity * 2;
    new_items = (struct ui_menu_item_entry *)C_MULTIPLATFORM_REALLOC(
        menu->items, (size_t)new_cap * sizeof(struct ui_menu_item_entry));
    if (!new_items)
      return UI_ERROR_OUT_OF_MEMORY;
    menu->items = new_items;
    menu->item_capacity = new_cap;
  }

  rc = ui_dom_node_set_attribute(label_node, "role", "menuitem");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(label_node, "class", "ui-menu-item");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(label_node, "tabindex", "-1");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(label_node, "data-active", "false");
  if (rc != UI_ERROR_NONE)
    return rc;

  if (submenu) {
    rc = ui_dom_node_set_attribute(label_node, "aria-haspopup", "menu");
    if (rc != UI_ERROR_NONE)
      return rc;
    submenu->parent_menu = menu;
  }

  rc = ui_dom_node_append_child(menu->container_node, label_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = duplicate_string(item_id, &menu->items[menu->item_count].id);
  if (rc != UI_ERROR_NONE)
    return rc;

  menu->items[menu->item_count].node = label_node;
  menu->items[menu->item_count].submenu = submenu;

  if (menu->item_count == 0) {
    menu->active_index = 0;
    rc = ui_dom_node_set_attribute(label_node, "data-active", "true");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(label_node, "tabindex", "0");
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  menu->item_count++;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the callback invoked when a menu item is triggered.
 * @param[in,out] menu The menu.
 * @param[in] on_action The callback function.
 * @param[in] user_data User data for the callback.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_set_on_action(struct ui_menu_base *menu,
                                      ui_menu_on_action_t on_action,
                                      void *user_data) {
  if (!menu)
    return UI_ERROR_INVALID_ARGUMENT;
  menu->on_action = on_action;
  menu->user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief Opens the menu at a specific screen coordinate via an overlay
 * director.
 * @param[in,out] menu The menu to open.
 * @param[in,out] director The overlay director managing the mount.
 * @param[in] x The X coordinate.
 * @param[in] y The Y coordinate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_open_at(struct ui_menu_base *menu,
                                struct ui_overlay_director *director, int x,
                                int y) {
  char style_buf[128];

  if (!menu || !director)
    return UI_ERROR_INVALID_ARGUMENT;

  if (menu->is_open)
    return UI_ERROR_NONE;

#if defined(_MSC_VER)
  sprintf_s(style_buf, sizeof(style_buf), "left: %dpx; top: %dpx;", x, y);
#else
  sprintf(style_buf, "left: %dpx; top: %dpx;", x, y);
#endif

  {
    ui_error_t attr_rc =
        ui_dom_node_set_attribute(menu->container_node, "style", style_buf);
    if (attr_rc != UI_ERROR_NONE) {
      return attr_rc;
    }
  }

  menu->director = director;
  menu->is_open = 1;
  menu->last_x = x;
  menu->last_y = y;

  return ui_overlay_director_mount_component(director, menu->component, 1000,
                                             &menu->overlay_handle);
}

/**
 * @brief Closes the menu and all its submenus.
 * @param[in,out] menu The menu to close.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_close(struct ui_menu_base *menu) {
  int i;
  ui_error_t rc = UI_ERROR_NONE;
  if (!menu)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!menu->is_open)
    return UI_ERROR_NONE;

  /* Recursively close submenus */
  for (i = 0; i < menu->item_count; i++) {
    if (menu->items[i].submenu) {
      ui_error_t close_rc = ui_menu_base_close(menu->items[i].submenu);
      if (close_rc != UI_ERROR_NONE && rc == UI_ERROR_NONE) {
        rc = close_rc;
      }
    }
  }

  {
    ui_error_t rc_cleanup =
        ui_overlay_director_unmount(menu->director, menu->overlay_handle);
    if (rc_cleanup != UI_ERROR_NONE && rc == UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  menu->overlay_handle = NULL;

  menu->is_open = 0;
  menu->active_index = -1;
  return rc;
}

/**
 * @brief Checks if the menu is currently open.
 * @param[in] menu The menu.
 * @param[out] out_is_open Set to 1 if open, 0 otherwise.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_is_open(const struct ui_menu_base *menu,
                                int *out_is_open) {
  if (!menu || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_open = menu->is_open;
  return UI_ERROR_NONE;
}

/**
 * @brief Intercepts a context menu event to potentially show this menu.
 * @param[in,out] menu The menu to show.
 * @param[in,out] director The overlay director to attach to.
 * @param[in] event The UI event representing the context menu request.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_menu_base_intercept_context_menu(struct ui_menu_base *menu,
                                    struct ui_overlay_director *director,
                                    const struct ui_event *event) {
  if (!menu || !director || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  return UI_ERROR_NONE;
}

/**
 * @brief Internal helper to update the visually active item index.
 * @param[in,out] menu The menu.
 * @param[in] new_index The new active index.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t update_active_index(struct ui_menu_base *menu,
                                      int new_index) {
  ui_error_t rc;
  if (menu->active_index >= 0) {
    rc = ui_dom_node_set_attribute(menu->items[menu->active_index].node,
                                   "data-active", "false");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(menu->items[menu->active_index].node,
                                   "tabindex", "-1");
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  menu->active_index = new_index;
  rc = ui_dom_node_set_attribute(menu->items[menu->active_index].node,
                                 "data-active", "true");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(menu->items[menu->active_index].node,
                                 "tabindex", "0");
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

/**
 * @brief Processes an incoming UI event (keyboard navigation) for the menu.
 * @param[in,out] menu The menu.
 * @param[in] event The event to process.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_process_event(struct ui_menu_base *menu,
                                      const struct ui_event *event) {
  int i;

  if (!menu || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!menu->is_open)
    return UI_ERROR_NONE;

  if (menu->item_count == 0)
    return UI_ERROR_NONE;

  /* First pass event to open submenus */
  for (i = 0; i < menu->item_count; i++) {
    if (menu->items[i].submenu) {
      if (menu->items[i].submenu->is_open) {
        /* If submenu handles it, we might want to return, but let's just
           process. Actually, we should return if submenu is open, so it takes
           focus. */
        ui_menu_base_process_event(menu->items[i].submenu, event);
        return UI_ERROR_NONE;
      }
    }
  }

  if (event->type == UI_EVENT_KEY_DOWN) {
    int key = event->event_data.keyboard.key_code;

    if (key == UI_KEY_DOWN) {
      int next = (menu->active_index + 1) % menu->item_count;
      ui_error_t up_rc = update_active_index(menu, next);
      if (up_rc != UI_ERROR_NONE) {
        return up_rc;
      }
    } else if (key == UI_KEY_UP) {
      int prev = menu->active_index - 1;
      ui_error_t up_rc;
      if (prev < 0)
        prev = menu->item_count - 1;
      up_rc = update_active_index(menu, prev);
      if (up_rc != UI_ERROR_NONE) {
        return up_rc;
      }
    } else if (key == UI_KEY_RIGHT) {
      if (menu->active_index >= 0) {
        struct ui_menu_base *sub = menu->items[menu->active_index].submenu;
        if (sub) {
          /* Compute an offset for the cascading menu (e.g. +100x, +24y per
           * index) */
          ui_error_t open_rc =
              ui_menu_base_open_at(sub, menu->director, menu->last_x + 100,
                                   menu->last_y + (menu->active_index * 24));
          if (open_rc != UI_ERROR_NONE) {
            return open_rc;
          }
        }
      }
    } else if (key == UI_KEY_LEFT) {
      if (menu->parent_menu) {
        ui_error_t close_rc = ui_menu_base_close(menu);
        if (close_rc != UI_ERROR_NONE) {
          return close_rc;
        }
      }
    } else if (key == UI_KEY_ENTER || key == UI_KEY_SPACE) {
      if (menu->active_index >= 0) {
        struct ui_menu_base *sub = menu->items[menu->active_index].submenu;
        if (sub) {
          ui_error_t open_rc =
              ui_menu_base_open_at(sub, menu->director, menu->last_x + 100,
                                   menu->last_y + (menu->active_index * 24));
          if (open_rc != UI_ERROR_NONE) {
            return open_rc;
          }
        } else {
          if (menu->on_action) {
            ui_error_t action_rc = menu->on_action(
                menu, menu->items[menu->active_index].id, menu->user_data);
            if (action_rc != UI_ERROR_NONE) {
              return action_rc;
            }
          }
          /* Close the entire menu hierarchy */
          {
            struct ui_menu_base *root = menu;
            ui_error_t close_rc;
            while (root->parent_menu) {
              root = root->parent_menu;
            }
            close_rc = ui_menu_base_close(root);
            if (close_rc != UI_ERROR_NONE) {
              return close_rc;
            }
          }
        }
      }
    } else if (key == UI_KEY_ESCAPE) {
      ui_error_t close_rc;
      close_rc = ui_menu_base_close(menu);
      if (close_rc != UI_ERROR_NONE) {
        return close_rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the underlying generic DOM component.
 * @param[in] menu The menu.
 * @param[out] out_component Pointer to store the DOM component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_get_component(struct ui_menu_base *menu,
                                      struct ui_component **out_component) {
  if (!menu || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = menu->component;
  return UI_ERROR_NONE;
}

/**
 * @brief Binds the active index of the menu to a reactive signal.
 * @param[in,out] widget The menu.
 * @param[in,out] signal The signal representing the active index.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_menu_base_bind_active_index(struct ui_menu_base *widget,
                                          struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
