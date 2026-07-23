/* clang-format off */
#include "ui_tabs_base.h"
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

static const char *ui_tabs_base_default_css =
    ".ui-tabs { display: flex; flex-direction: column; } "
    ".ui-tablist { display: flex; flex-direction: row; } "
    ".ui-tabpanels { display: flex; flex-direction: column; flex: 1; }";

/** \brief ui_tab_entry */
struct ui_tab_entry {
  char *id;
  struct ui_dom_node *header_node;
  struct ui_dom_node *panel_node;
};

/** \brief ui_tabs_base */
struct ui_tabs_base {
  struct ui_component *component;
  struct ui_dom_node *tablist_node;
  struct ui_dom_node *panels_node;

  struct ui_tab_entry *tabs;
  int tab_count;
  int tab_capacity;

  int active_index;
  ui_tabs_on_change_t on_change;
  void *user_data;
  struct ui_signal *active_index_signal;
};

enum ui_error ui_tabs_base_create(struct ui_tabs_base **out_tabs) {
  struct ui_tabs_base *tabs;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_dom_node *tablist_node = NULL;
  struct ui_dom_node *panels_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_tabs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tabs = (struct ui_tabs_base *)UI_MALLOC(sizeof(struct ui_tabs_base));
  if (!tabs) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  tabs->component = NULL;
  tabs->tablist_node = NULL;
  tabs->panels_node = NULL;
  tabs->tabs = NULL;
  tabs->tab_count = 0;
  tabs->tab_capacity = 0;
  tabs->active_index = -1;
  tabs->on_change = NULL;
  tabs->user_data = NULL;

  rc = ui_component_create(&tabs->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  ui_dom_node_set_tag_name(root_node, "div");
  ui_dom_node_set_attribute(root_node, "class", "ui-tabs");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &tablist_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  ui_dom_node_set_tag_name(tablist_node, "div");
  ui_dom_node_set_attribute(tablist_node, "role", "tablist");
  ui_dom_node_set_attribute(tablist_node, "class", "ui-tablist");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &panels_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  ui_dom_node_set_tag_name(panels_node, "div");
  ui_dom_node_set_attribute(panels_node, "class", "ui-tabpanels");

  rc = ui_dom_node_append_child(root_node, tablist_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  tabs->tablist_node = tablist_node;

  rc = ui_dom_node_append_child(root_node, panels_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  tabs->panels_node = panels_node;

  rc = ui_css_parse_stylesheet(ui_tabs_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_set_default_style(tabs->component, default_style);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(default_style);
    goto cleanup;
  }

  tabs->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component */

  *out_tabs = tabs;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    ui_dom_node_destroy(root_node);
  } else {
    if (tablist_node && !tabs->tablist_node)
      ui_dom_node_destroy(tablist_node);
    if (panels_node && !tabs->panels_node)
      ui_dom_node_destroy(panels_node);
  }
  if (tabs->component) {
    ui_component_destroy(tabs->component);
  }
  UI_FREE(tabs);
  return rc;
}

void ui_tabs_base_destroy(struct ui_tabs_base *tabs) {
  int i;
  if (!tabs) {
    return;
  }

  for (i = 0; i < tabs->tab_count; i++) {
    UI_FREE(tabs->tabs[i].id);
  }
  if (tabs->tabs) {
    UI_FREE(tabs->tabs);
  }

  if (tabs->component) {
    ui_component_destroy(tabs->component);
  }

  UI_FREE(tabs);
}

static enum ui_error duplicate_string(const char *src, char **out_str) {
  size_t len;
  char *dst;
  if (!out_str)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_str = NULL;
  if (!src)
    return UI_ERROR_NONE;
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

static enum ui_error format_id(char *buf, size_t buf_size, const char *prefix,
                               const char *suffix) {
#if defined(_MSC_VER)
  if (sprintf_s(buf, buf_size, "%s-%s", prefix, suffix) < 0) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
#else
  if (sprintf(buf, "%s-%s", prefix, suffix) >= (int)buf_size) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
#endif
  return UI_ERROR_NONE;
}

enum ui_error ui_tabs_base_add_tab(struct ui_tabs_base *tabs,
                                   const char *tab_id,
                                   struct ui_dom_node *title_node,
                                   struct ui_dom_node *panel_node) {
  enum ui_error rc;
  struct ui_tab_entry *new_tabs;
  char tab_node_id[256];
  char panel_node_id[256];

  if (!tabs || !tab_id || !title_node || !panel_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tabs->tab_count >= tabs->tab_capacity) {
    int new_cap = tabs->tab_capacity == 0 ? 4 : tabs->tab_capacity * 2;
    new_tabs = (struct ui_tab_entry *)UI_REALLOC(
        tabs->tabs, new_cap * sizeof(struct ui_tab_entry));
    if (!new_tabs) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    tabs->tabs = new_tabs;
    tabs->tab_capacity = new_cap;
  }

  rc = format_id(tab_node_id, sizeof(tab_node_id), tab_id, "tab");
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = format_id(panel_node_id, sizeof(panel_node_id), tab_id, "panel");
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Setup title_node ARIA/role attributes */
  ui_dom_node_set_attribute(title_node, "role", "tab");
  ui_dom_node_set_attribute(title_node, "id", tab_node_id);
  ui_dom_node_set_attribute(title_node, "aria-controls", panel_node_id);

  /* Setup panel_node ARIA/role attributes */
  ui_dom_node_set_attribute(panel_node, "role", "tabpanel");
  ui_dom_node_set_attribute(panel_node, "id", panel_node_id);
  ui_dom_node_set_attribute(panel_node, "aria-labelledby", tab_node_id);

  /* Default state: hidden/inactive if not the first tab */
  if (tabs->tab_count == 0) {
    ui_dom_node_set_attribute(title_node, "aria-selected", "true");
    ui_dom_node_set_attribute(title_node, "tabindex", "0");
  } else {
    ui_dom_node_set_attribute(title_node, "aria-selected", "false");
    ui_dom_node_set_attribute(title_node, "tabindex", "-1");
    ui_dom_node_set_attribute(panel_node, "hidden", "true");
  }

  rc = ui_dom_node_append_child(tabs->tablist_node, title_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_dom_node_append_child(tabs->panels_node, panel_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = duplicate_string(tab_id, &tabs->tabs[tabs->tab_count].id);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (!tabs->tabs[tabs->tab_count].id) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  tabs->tabs[tabs->tab_count].header_node = title_node;
  tabs->tabs[tabs->tab_count].panel_node = panel_node;

  if (tabs->tab_count == 0) {
    tabs->active_index = 0;
  }

  tabs->tab_count++;

  return UI_ERROR_NONE;
}

enum ui_error ui_tabs_base_set_active_index(struct ui_tabs_base *tabs,
                                            int index) {
  int i;

  if (!tabs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index < 0 || index >= tabs->tab_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  if (tabs->active_index == index) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < tabs->tab_count; i++) {
    struct ui_tab_entry *entry = &tabs->tabs[i];
    if (i == index) {
      ui_dom_node_set_attribute(entry->header_node, "aria-selected", "true");
      ui_dom_node_set_attribute(entry->header_node, "tabindex", "0");
      ui_dom_node_remove_attribute(entry->panel_node, "hidden");
    } else {
      ui_dom_node_set_attribute(entry->header_node, "aria-selected", "false");
      ui_dom_node_set_attribute(entry->header_node, "tabindex", "-1");
      ui_dom_node_set_attribute(entry->panel_node, "hidden", "true");
    }
  }

  tabs->active_index = index;

  if (tabs->on_change) {
    tabs->on_change(tabs, index, tabs->user_data);
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_tabs_base_get_active_index(const struct ui_tabs_base *tabs,
                                            int *out_index) {
  if (!tabs || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_index = tabs->active_index;
  return UI_ERROR_NONE;
}

enum ui_error ui_tabs_base_set_on_change(struct ui_tabs_base *tabs,
                                         ui_tabs_on_change_t on_change,
                                         void *user_data) {
  if (!tabs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tabs->on_change = on_change;
  tabs->user_data = user_data;
  return UI_ERROR_NONE;
}

enum ui_error ui_tabs_base_process_event(struct ui_tabs_base *tabs,
                                         const struct ui_event *event,
                                         double timestamp_ms) {
  int next_index;

  (void)timestamp_ms;

  if (!tabs || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tabs->tab_count == 0) {
    return UI_ERROR_NONE;
  }

  if (event->type == UI_EVENT_KEY_DOWN) {
    const struct ui_keyboard_event *kb_event = &event->event_data.keyboard;

    if (kb_event->key_code == UI_KEY_RIGHT) {
      next_index = (tabs->active_index + 1) % tabs->tab_count;
      ui_tabs_base_set_active_index(tabs, next_index);
    } else if (kb_event->key_code == UI_KEY_LEFT) {
      next_index = tabs->active_index - 1;
      if (next_index < 0) {
        next_index = tabs->tab_count - 1;
      }
      ui_tabs_base_set_active_index(tabs, next_index);
    } else if (kb_event->key_code == UI_KEY_HOME) {
      ui_tabs_base_set_active_index(tabs, 0);
    } else if (kb_event->key_code == UI_KEY_END) {
      ui_tabs_base_set_active_index(tabs, tabs->tab_count - 1);
    }
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_tabs_base_get_component(struct ui_tabs_base *tabs,
                                         struct ui_component **out_component) {
  if (!tabs || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = tabs->component;
  return UI_ERROR_NONE;
}

enum ui_error ui_tabs_base_bind_active_index(struct ui_tabs_base *widget,
                                             struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
