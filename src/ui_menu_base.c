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

static const char *ui_menu_base_default_css =
    ".ui-menu { display: flex; flex-direction: column; position: absolute; "
    "background-color: #fff; border: 1px solid #ccc; z-index: 1000; } "
    ".ui-menu-item { padding: 4px 8px; cursor: pointer; display: flex; "
    "justify-content: space-between; } "
    ".ui-menu-item[data-active=\"true\"] { background-color: #eee; }";

/** \brief ui_menu_item_entry */
struct ui_menu_item_entry {
  char *id;
  struct ui_dom_node *node;
  struct ui_menu_base *submenu;
};

/** \brief ui_menu_base */
struct ui_menu_base {
  struct ui_component *component;
  struct ui_dom_node *container_node;
  struct ui_overlay *overlay_handle;
  struct ui_overlay_director *director;

  struct ui_menu_item_entry *items;
  int item_count;
  int item_capacity;

  int active_index;
  int is_open;
  int last_x;
  int last_y;

  struct ui_menu_base *parent_menu;

  ui_menu_on_action_t on_action;
  void *user_data;
  struct ui_signal *active_index_signal;
};

enum ui_error ui_menu_base_create(struct ui_menu_base **out_menu) {
  struct ui_menu_base *menu;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_menu)
    return UI_ERROR_INVALID_ARGUMENT;

  menu = (struct ui_menu_base *)UI_MALLOC(sizeof(struct ui_menu_base));
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
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(root_node, "div");
  ui_dom_node_set_attribute(root_node, "class", "ui-menu");
  ui_dom_node_set_attribute(root_node, "role", "menu");

  menu->container_node = root_node;

  rc = ui_css_parse_stylesheet(ui_menu_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  (void)ui_component_set_default_style(menu->component, default_style);

  menu->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component */

  *out_menu = menu;
  return UI_ERROR_NONE;

cleanup:
  if (root_node)
    ui_dom_node_destroy(root_node);
  if (menu->component)
    ui_component_destroy(menu->component);
  UI_FREE(menu);
  return rc;
}

void ui_menu_base_destroy(struct ui_menu_base *menu) {
  int i;
  if (!menu)
    return;

  ui_menu_base_close(menu);

  for (i = 0; i < menu->item_count; i++) {
    UI_FREE(menu->items[i].id);
  }
  if (menu->items) {
    UI_FREE(menu->items);
  }

  if (menu->component) {
    ui_component_destroy(menu->component);
  }

  UI_FREE(menu);
}

static enum ui_error duplicate_string(const char *src, char **out_str) {
  size_t len;
  char *dst;
  *out_str = NULL;
  len = strlen(src);
  dst = (char *)UI_MALLOC(len + 1);
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

enum ui_error ui_menu_base_add_item(struct ui_menu_base *menu,
                                    const char *item_id,
                                    struct ui_dom_node *label_node,
                                    struct ui_menu_base *submenu) {
  enum ui_error rc;
  struct ui_menu_item_entry *new_items;

  if (!menu || !item_id || !label_node)
    return UI_ERROR_INVALID_ARGUMENT;

  if (menu->item_count >= menu->item_capacity) {
    int new_cap = menu->item_capacity == 0 ? 4 : menu->item_capacity * 2;
    new_items = (struct ui_menu_item_entry *)UI_REALLOC(
        menu->items, new_cap * sizeof(struct ui_menu_item_entry));
    if (!new_items)
      return UI_ERROR_OUT_OF_MEMORY;
    menu->items = new_items;
    menu->item_capacity = new_cap;
  }

  ui_dom_node_set_attribute(label_node, "role", "menuitem");
  ui_dom_node_set_attribute(label_node, "class", "ui-menu-item");
  ui_dom_node_set_attribute(label_node, "tabindex", "-1");
  ui_dom_node_set_attribute(label_node, "data-active", "false");

  if (submenu) {
    ui_dom_node_set_attribute(label_node, "aria-haspopup", "menu");
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
    ui_dom_node_set_attribute(label_node, "data-active", "true");
    ui_dom_node_set_attribute(label_node, "tabindex", "0");
  }

  menu->item_count++;
  return UI_ERROR_NONE;
}

enum ui_error ui_menu_base_set_on_action(struct ui_menu_base *menu,
                                         ui_menu_on_action_t on_action,
                                         void *user_data) {
  if (!menu)
    return UI_ERROR_INVALID_ARGUMENT;
  menu->on_action = on_action;
  menu->user_data = user_data;
  return UI_ERROR_NONE;
}

enum ui_error ui_menu_base_open_at(struct ui_menu_base *menu,
                                   struct ui_overlay_director *director, int x,
                                   int y) {
  char style_buf[128];

  if (!menu || !director)
    return UI_ERROR_INVALID_ARGUMENT;

  if (menu->is_open)
    return UI_ERROR_NONE;

#if defined(_MSC_VER)
  (void)sprintf_s(style_buf, sizeof(style_buf), "left: %dpx; top: %dpx;", x, y);
#else
  (void)sprintf(style_buf, "left: %dpx; top: %dpx;", x, y);
#endif

  ui_dom_node_set_attribute(menu->container_node, "style", style_buf);

  menu->director = director;
  menu->is_open = 1;
  menu->last_x = x;
  menu->last_y = y;

  return ui_overlay_director_mount_component(director, menu->component, 1000,
                                             &menu->overlay_handle);
}

enum ui_error ui_menu_base_close(struct ui_menu_base *menu) {
  int i;
  if (!menu)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!menu->is_open)
    return UI_ERROR_NONE;

  /* Recursively close submenus */
  for (i = 0; i < menu->item_count; i++) {
    if (menu->items[i].submenu) {
      ui_menu_base_close(menu->items[i].submenu);
    }
  }

  if (menu->director && menu->overlay_handle) {
    ui_overlay_director_unmount(menu->director, menu->overlay_handle);
    menu->overlay_handle = NULL;
  }

  menu->is_open = 0;
  return UI_ERROR_NONE;
}

enum ui_error ui_menu_base_is_open(const struct ui_menu_base *menu,
                                   int *out_is_open) {
  if (!menu || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_open = menu->is_open;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_menu_base_intercept_context_menu(struct ui_menu_base *menu,
                                    struct ui_overlay_director *director,
                                    const struct ui_event *event) {
  if (!menu || !director || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  return UI_ERROR_NONE;
}

static enum ui_error update_active_index(struct ui_menu_base *menu,
                                         int new_index) {
  if (menu->active_index >= 0 && menu->active_index < menu->item_count) {
    ui_dom_node_set_attribute(menu->items[menu->active_index].node,
                              "data-active", "false");
    ui_dom_node_set_attribute(menu->items[menu->active_index].node, "tabindex",
                              "-1");
  }
  menu->active_index = new_index;
  if (menu->active_index >= 0 && menu->active_index < menu->item_count) {
    ui_dom_node_set_attribute(menu->items[menu->active_index].node,
                              "data-active", "true");
    ui_dom_node_set_attribute(menu->items[menu->active_index].node, "tabindex",
                              "0");
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_menu_base_process_event(struct ui_menu_base *menu,
                                         const struct ui_event *event) {
  int i;
  int handled = 0;

  if (!menu || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!menu->is_open)
    return UI_ERROR_NONE;

  /* First pass event to open submenus */
  for (i = 0; i < menu->item_count; i++) {
    int is_open = 0;
    if (menu->items[i].submenu) {
      ui_menu_base_is_open(menu->items[i].submenu, &is_open);
      if (is_open) {
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
      update_active_index(menu, next);
      handled = 1;
    } else if (key == UI_KEY_UP) {
      int prev = menu->active_index - 1;
      if (prev < 0)
        prev = menu->item_count - 1;
      update_active_index(menu, prev);
      handled = 1;
    } else if (key == UI_KEY_RIGHT) {
      if (menu->active_index >= 0 && menu->active_index < menu->item_count) {
        struct ui_menu_base *sub = menu->items[menu->active_index].submenu;
        if (sub) {
          /* Compute an offset for the cascading menu (e.g. +100x, +24y per
           * index) */
          ui_menu_base_open_at(sub, menu->director, menu->last_x + 100,
                               menu->last_y + (menu->active_index * 24));
        }
      }
      handled = 1;
    } else if (key == UI_KEY_LEFT) {
      if (menu->parent_menu) {
        ui_menu_base_close(menu);
      }
      handled = 1;
    } else if (key == UI_KEY_ENTER || key == UI_KEY_SPACE) {
      if (menu->active_index >= 0 && menu->active_index < menu->item_count) {
        struct ui_menu_base *sub = menu->items[menu->active_index].submenu;
        if (sub) {
          ui_menu_base_open_at(sub, menu->director, menu->last_x + 100,
                               menu->last_y + (menu->active_index * 24));
        } else {
          if (menu->on_action) {
            menu->on_action(menu, menu->items[menu->active_index].id,
                            menu->user_data);
          }
          /* Close the entire menu hierarchy */
          {
            struct ui_menu_base *root = menu;
            while (root->parent_menu) {
              root = root->parent_menu;
            }
            ui_menu_base_close(root);
          }
        }
      }
      handled = 1;
    } else if (key == UI_KEY_ESCAPE) {
      ui_menu_base_close(menu);
      handled = 1;
    }
  }

  (void)handled;
  return UI_ERROR_NONE;
}
enum ui_error ui_menu_base_get_component(struct ui_menu_base *menu,
                                         struct ui_component **out_component) {
  if (!menu || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = menu->component;
  return UI_ERROR_NONE;
}

enum ui_error ui_menu_base_bind_active_index(struct ui_menu_base *widget,
                                             struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
