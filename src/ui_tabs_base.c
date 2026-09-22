/**
 * @file ui_tabs_base.c
 * @brief Implementation of the tabs base component.
 */
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

#ifdef UI_TEST_MOCK_ALLOC
int g_tabs_mock_fail = 0;
int g_tabs_mock_append_fail_target = 0;
int g_tabs_mock_set_attr_fail_target = 0;
int g_tabs_mock_remove_attr_fail_target = 0;
static int g_tabs_append_counter = 0;
static int g_tabs_set_attr_counter = 0;
static int g_tabs_remove_attr_counter = 0;

/**
 * @brief mock_tabs_dom_node_append_child.
 * @param parent Parent node.
 * @param child Child node.
 * @return Return value.
 */
static ui_error_t mock_tabs_dom_node_append_child(struct ui_dom_node *parent,
                                                  struct ui_dom_node *child) {
  if (g_tabs_mock_append_fail_target > 0) {
    if (++g_tabs_append_counter == g_tabs_mock_append_fail_target) {
      g_tabs_append_counter = 0;
      return UI_ERROR_UNKNOWN;
    }
  }
  return ui_dom_node_append_child(parent, child);
}
/** @cond */
#define ui_dom_node_append_child mock_tabs_dom_node_append_child
/** @endcond */

/**
 * @brief mock_tabs_component_set_default_style.
 * @param comp Component.
 * @param style Stylesheet.
 * @return Return value.
 */
static ui_error_t
mock_tabs_component_set_default_style(struct ui_component *comp,
                                      struct ui_css_stylesheet *style) {
  if (g_tabs_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_set_default_style(comp, style);
}
/** @cond */
#define ui_component_set_default_style mock_tabs_component_set_default_style
/** @endcond */

/**
 * @brief mock_tabs_dom_node_destroy.
 * @param node Node.
 * @return Return value.
 */
static ui_error_t mock_tabs_dom_node_destroy(struct ui_dom_node *node) {
  if (g_tabs_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_destroy(node);
}
/** @cond */
#define ui_dom_node_destroy mock_tabs_dom_node_destroy
/** @endcond */

/**
 * @brief mock_tabs_component_destroy.
 * @param comp Component.
 * @return Return value.
 */
static ui_error_t mock_tabs_component_destroy(struct ui_component *comp) {
  if (g_tabs_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_destroy(comp);
}
/** @cond */
#define ui_component_destroy mock_tabs_component_destroy
/** @endcond */

/**
 * @brief mock_tabs_dom_node_set_attribute.
 * @param node Node.
 * @param name Name.
 * @param val Value.
 * @return Return value.
 */
static ui_error_t mock_tabs_dom_node_set_attribute(struct ui_dom_node *node,
                                                   const char *name,
                                                   const char *val) {
  if (g_tabs_mock_set_attr_fail_target > 0) {
    if (++g_tabs_set_attr_counter == g_tabs_mock_set_attr_fail_target) {
      g_tabs_set_attr_counter = 0;
      return UI_ERROR_UNKNOWN;
    }
  }
  return ui_dom_node_set_attribute(node, name, val);
}
/** @cond */
#define ui_dom_node_set_attribute mock_tabs_dom_node_set_attribute
/** @endcond */

/**
 * @brief mock_tabs_dom_node_remove_attribute.
 * @param node Node.
 * @param name Name.
 * @return Return value.
 */
static ui_error_t mock_tabs_dom_node_remove_attribute(struct ui_dom_node *node,
                                                      const char *name) {
  if (g_tabs_mock_remove_attr_fail_target > 0) {
    if (++g_tabs_remove_attr_counter == g_tabs_mock_remove_attr_fail_target) {
      g_tabs_remove_attr_counter = 0;
      return UI_ERROR_UNKNOWN;
    }
  }
  return ui_dom_node_remove_attribute(node, name);
}
/** @cond */
#define ui_dom_node_remove_attribute mock_tabs_dom_node_remove_attribute
/** @endcond */
#endif

/** @brief Default CSS stylesheet */
static const char *ui_tabs_base_default_css =
    ".ui-tabs { display: flex; flex-direction: column; } "
    ".ui-tablist { display: flex; flex-direction: row; } "
    ".ui-tabpanels { display: flex; flex-direction: column; flex: 1; }";

/**
 * @struct ui_tab_entry
 * @struct ui_tab_entry
 * @brief Internal representation of a single tab.
 */
struct ui_tab_entry {
  /* @brief The tab identifier. */
  char *id; /**< id */
  /* @brief The DOM node for the tab header. */
  struct ui_dom_node *header_node; /**< header_node */
  /* @brief The DOM node for the tab panel. */
  struct ui_dom_node *panel_node; /**< panel_node */
};

/**
 * @struct ui_tabs_base
 * @struct ui_tabs_base
 * @brief Internal implementation of the tabs base component.
 */
struct ui_tabs_base {
  /* @brief The underlying UI component. */
  struct ui_component *component; /**< component */
  /* @brief The tab list container DOM node. */
  struct ui_dom_node *tablist_node; /**< tablist_node */
  /* @brief The panels container DOM node. */
  struct ui_dom_node *panels_node; /**< panels_node */

  /* @brief Array of tab entries. */
  struct ui_tab_entry *tabs; /**< tabs */
  /* @brief Number of active tabs. */
  int tab_count; /**< tab_count */
  /* @brief Allocated capacity for tabs. */
  int tab_capacity; /**< tab_capacity */

  /* @brief The active tab index. */
  int active_index; /**< active_index */
  /* @brief On change callback. */
  ui_tabs_on_change_t on_change; /**< on_change */
  /* @brief User data for callback. */
  void *user_data; /**< user_data */
  /* @brief Signal bound for active index. */
  struct ui_signal *active_index_signal; /**< active_index_signal */
};

ui_error_t ui_tabs_base_create(struct ui_tabs_base **out_tabs) {
  struct ui_tabs_base *tabs;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_dom_node *tablist_node = NULL;
  struct ui_dom_node *panels_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_tabs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tabs = (struct ui_tabs_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_tabs_base));
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
  {
    ui_error_t dom_rc = ui_dom_node_set_tag_name(root_node, "div");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }
  {
    ui_error_t dom_rc =
        ui_dom_node_set_attribute(root_node, "class", "ui-tabs");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &tablist_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  {
    ui_error_t dom_rc = ui_dom_node_set_tag_name(tablist_node, "div");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }
  {
    ui_error_t dom_rc =
        ui_dom_node_set_attribute(tablist_node, "role", "tablist");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }
  {
    ui_error_t dom_rc =
        ui_dom_node_set_attribute(tablist_node, "class", "ui-tablist");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &panels_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  {
    ui_error_t dom_rc = ui_dom_node_set_tag_name(panels_node, "div");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }
  {
    ui_error_t dom_rc =
        ui_dom_node_set_attribute(panels_node, "class", "ui-tabpanels");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }

  rc = ui_dom_node_append_child(root_node, tablist_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  tabs->tablist_node = tablist_node;

  rc = ui_dom_node_append_child(root_node, panels_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
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
    ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  if (tabs->component) {
    ui_error_t rc_cleanup = ui_component_destroy(tabs->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  C_MULTIPLATFORM_FREE(tabs);
  return rc;
}

ui_error_t ui_tabs_base_destroy(struct ui_tabs_base *tabs) {
  int i;
  ui_error_t rc = UI_ERROR_NONE;
  if (!tabs) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < tabs->tab_count; i++) {
    C_MULTIPLATFORM_FREE(tabs->tabs[i].id);
  }
  C_MULTIPLATFORM_FREE(tabs->tabs);

  if (tabs->component) {
    ui_error_t rc_cleanup = ui_component_destroy(tabs->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  C_MULTIPLATFORM_FREE(tabs);
  return rc;
}

/**
 * @brief Duplicates a string using multiplatform malloc.
 * @param src The source string.
 * @param out_str Pointer to receive the duplicated string.
 * @return UI_ERROR_NONE on success, or an error code.
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
 * @brief Formats an identifier by combining a prefix and a suffix.
 * @param buf The output buffer.
 * @param buf_size The size of the output buffer.
 * @param prefix The prefix string.
 * @param suffix The suffix string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
static ui_error_t format_id(char *buf, size_t buf_size, const char *prefix,
                            const char *suffix) {
#if !defined(_MSC_VER)
  size_t unused_size;
#endif
#ifdef UI_TEST_MOCK_ALLOC
  if (g_tabs_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_tabs_mock_fail == 5) {
    static int fid_count = 0;
    if (++fid_count == 2) {
      fid_count = 0;
      return UI_ERROR_UNKNOWN;
    }
  }
#endif
#if defined(_MSC_VER)
  sprintf_s(buf, buf_size, "%s-%s", prefix, suffix);
#else
  unused_size = buf_size;
  buf_size = unused_size;
  sprintf(buf, "%s-%s", prefix, suffix);
#endif
  return UI_ERROR_NONE;
}

ui_error_t ui_tabs_base_add_tab(struct ui_tabs_base *tabs, const char *tab_id,
                                struct ui_dom_node *title_node,
                                struct ui_dom_node *panel_node) {
  struct ui_tab_entry *new_tabs;
  char tab_node_id[256];
  char panel_node_id[256];
  ui_error_t rc;

  if (!tabs || !tab_id || !title_node || !panel_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tabs->tab_count >= tabs->tab_capacity) {
    int new_cap = tabs->tab_capacity == 0 ? 4 : tabs->tab_capacity * 2;
    new_tabs = (struct ui_tab_entry *)C_MULTIPLATFORM_REALLOC(
        tabs->tabs, (size_t)new_cap * sizeof(struct ui_tab_entry));
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

  {
    char *tmp = NULL;
    ui_error_t err = duplicate_string(tab_id, &tmp);
    if (err != UI_ERROR_NONE)
      return err;
    tabs->tabs[tabs->tab_count].id = tmp;
  }

  /** @cond */
  /** @endcond */

  /* Setup title_node ARIA/role attributes */
  rc = ui_dom_node_set_attribute(title_node, "role", "tab");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(title_node, "id", tab_node_id);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(title_node, "aria-controls", panel_node_id);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Setup panel_node ARIA/role attributes */
  rc = ui_dom_node_set_attribute(panel_node, "role", "tabpanel");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(panel_node, "id", panel_node_id);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(panel_node, "aria-labelledby", tab_node_id);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Default state: hidden/inactive if not the first tab */
  if (tabs->tab_count == 0) {
    rc = ui_dom_node_set_attribute(title_node, "aria-selected", "true");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(title_node, "tabindex", "0");
    if (rc != UI_ERROR_NONE)
      return rc;
  } else {
    rc = ui_dom_node_set_attribute(title_node, "aria-selected", "false");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(title_node, "tabindex", "-1");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(panel_node, "hidden", "true");
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  rc = ui_dom_node_append_child(tabs->tablist_node, title_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_dom_node_append_child(tabs->panels_node, panel_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  tabs->tabs[tabs->tab_count].header_node = title_node;
  tabs->tabs[tabs->tab_count].panel_node = panel_node;

  if (tabs->tab_count == 0) {
    tabs->active_index = 0;
  }

  tabs->tab_count++;

  return UI_ERROR_NONE;
}

ui_error_t ui_tabs_base_set_active_index(struct ui_tabs_base *tabs, int index) {
  int i;
  ui_error_t rc;

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
      rc = ui_dom_node_set_attribute(entry->header_node, "aria-selected",
                                     "true");
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = ui_dom_node_set_attribute(entry->header_node, "tabindex", "0");
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = ui_dom_node_remove_attribute(entry->panel_node, "hidden");
      if (rc != UI_ERROR_NONE)
        return rc;
    } else {
      rc = ui_dom_node_set_attribute(entry->header_node, "aria-selected",
                                     "false");
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = ui_dom_node_set_attribute(entry->header_node, "tabindex", "-1");
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = ui_dom_node_set_attribute(entry->panel_node, "hidden", "true");
      if (rc != UI_ERROR_NONE)
        return rc;
    }
  }

  tabs->active_index = index;

  if (tabs->on_change) {
    ui_error_t oc_rc = tabs->on_change(tabs, index, tabs->user_data);
    if (oc_rc != UI_ERROR_NONE)
      return oc_rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_tabs_base_get_active_index(const struct ui_tabs_base *tabs,
                                         int *out_index) {
  if (!tabs || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_index = tabs->active_index;
  return UI_ERROR_NONE;
}

ui_error_t ui_tabs_base_set_on_change(struct ui_tabs_base *tabs,
                                      ui_tabs_on_change_t on_change,
                                      void *user_data) {
  if (!tabs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  tabs->on_change = on_change;
  tabs->user_data = user_data;
  return UI_ERROR_NONE;
}

ui_error_t ui_tabs_base_process_event(struct ui_tabs_base *tabs,
                                      const struct ui_event *event,
                                      double timestamp_ms) {
  int next_index;

  if (timestamp_ms > 0.0) {
  }

  if (!tabs || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tabs->tab_count == 0) {
    return UI_ERROR_NONE;
  }

  if (event->type == UI_EVENT_KEY_DOWN) {
    ui_error_t set_rc;
    const struct ui_keyboard_event *kb_event = &event->event_data.keyboard;

    if (kb_event->key_code == UI_KEY_RIGHT) {
      next_index = (tabs->active_index + 1) % tabs->tab_count;
      set_rc = ui_tabs_base_set_active_index(tabs, next_index);
      if (set_rc != UI_ERROR_NONE)
        return set_rc;
    } else if (kb_event->key_code == UI_KEY_LEFT) {
      next_index = tabs->active_index - 1;
      if (next_index < 0) {
        next_index = tabs->tab_count - 1;
      }
      set_rc = ui_tabs_base_set_active_index(tabs, next_index);
      if (set_rc != UI_ERROR_NONE)
        return set_rc;
    } else if (kb_event->key_code == UI_KEY_HOME) {
      set_rc = ui_tabs_base_set_active_index(tabs, 0);
      if (set_rc != UI_ERROR_NONE)
        return set_rc;
    } else if (kb_event->key_code == UI_KEY_END) {
      set_rc = ui_tabs_base_set_active_index(tabs, tabs->tab_count - 1);
      if (set_rc != UI_ERROR_NONE)
        return set_rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_tabs_base_get_component(struct ui_tabs_base *tabs,
                                      struct ui_component **out_component) {
  if (!tabs || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = tabs->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_tabs_base_bind_active_index(struct ui_tabs_base *widget,
                                          struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
