/* clang-format off */
#include "ui_stepper_base.h"
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

static const char *ui_stepper_base_default_css =
    ".ui-stepper { display: flex; flex-direction: column; } "
    ".ui-stepper-header { display: flex; flex-direction: row; } "
    ".ui-stepper-content { display: flex; flex-direction: column; flex: 1; }";

/** \brief ui_stepper_step_entry */
struct ui_stepper_step_entry {
  char *id;
  struct ui_dom_node *header_node;
  struct ui_dom_node *content_node;
  enum ui_stepper_step_state explicit_state;
};

/** \brief ui_stepper_base */
struct ui_stepper_base {
  struct ui_component *component;
  struct ui_dom_node *header_container_node;
  struct ui_dom_node *content_container_node;

  struct ui_stepper_step_entry *steps;
  int step_count;
  int step_capacity;

  int active_index;
  enum ui_stepper_mode mode;

  ui_stepper_validate_t validate_hook;
  void *user_data;
  struct ui_signal *active_index_signal;
};

enum ui_error ui_stepper_base_create(struct ui_stepper_base **out_stepper) {
  struct ui_stepper_base *stepper;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_dom_node *header_node = NULL;
  struct ui_dom_node *content_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_stepper) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  stepper = (struct ui_stepper_base *)UI_MALLOC(sizeof(struct ui_stepper_base));
  if (!stepper) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  stepper->component = NULL;
  stepper->header_container_node = NULL;
  stepper->content_container_node = NULL;
  stepper->steps = NULL;
  stepper->step_count = 0;
  stepper->step_capacity = 0;
  stepper->active_index = -1;
  stepper->mode = UI_STEPPER_MODE_LINEAR;
  stepper->validate_hook = NULL;
  stepper->user_data = NULL;

  rc = ui_component_create(&stepper->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(root_node, "div");
  ui_dom_node_set_attribute(root_node, "class", "ui-stepper");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &header_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(header_node, "div");
  ui_dom_node_set_attribute(header_node, "class", "ui-stepper-header");
  ui_dom_node_set_attribute(header_node, "role", "tablist");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(content_node, "div");
  ui_dom_node_set_attribute(content_node, "class", "ui-stepper-content");

  rc = ui_dom_node_append_child(root_node, header_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  stepper->header_container_node = header_node;

  rc = ui_dom_node_append_child(root_node, content_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  stepper->content_container_node = content_node;

  rc = ui_css_parse_stylesheet(ui_stepper_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_component_set_default_style(stepper->component, default_style);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(default_style);
    goto cleanup;
  }

  stepper->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component */

  *out_stepper = stepper;
  return UI_ERROR_NONE;

cleanup:
  if (root_node)
    ui_dom_node_destroy(root_node);
  else {
    if (header_node && !stepper->header_container_node)
      ui_dom_node_destroy(header_node);
    if (content_node && !stepper->content_container_node)
      ui_dom_node_destroy(content_node);
  }
  if (stepper->component)
    ui_component_destroy(stepper->component);
  UI_FREE(stepper);
  return rc;
}

void ui_stepper_base_destroy(struct ui_stepper_base *stepper) {
  int i;
  if (!stepper)
    return;

  for (i = 0; i < stepper->step_count; i++) {
    UI_FREE(stepper->steps[i].id);
  }
  if (stepper->steps) {
    UI_FREE(stepper->steps);
  }

  if (stepper->component) {
    ui_component_destroy(stepper->component);
  }

  UI_FREE(stepper);
}

enum ui_error ui_stepper_base_set_mode(struct ui_stepper_base *stepper,
                                       enum ui_stepper_mode mode) {
  if (!stepper)
    return UI_ERROR_INVALID_ARGUMENT;
  stepper->mode = mode;
  return UI_ERROR_NONE;
}

enum ui_error ui_stepper_base_set_validate_hook(struct ui_stepper_base *stepper,
                                                ui_stepper_validate_t hook,
                                                void *user_data) {
  if (!stepper)
    return UI_ERROR_INVALID_ARGUMENT;
  stepper->validate_hook = hook;
  stepper->user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error duplicate_string(const char *src, char **out_copy) {
  size_t len;
  char *dst;
  *out_copy = NULL;
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
  *out_copy = dst;
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

static enum ui_error
apply_step_state_attributes(struct ui_stepper_base *stepper, int index) {
  struct ui_stepper_step_entry *entry = &stepper->steps[index];
  enum ui_stepper_step_state effective_state = entry->explicit_state;

  if (index == stepper->active_index) {
    effective_state = UI_STEPPER_STEP_STATE_ACTIVE;
  }

  ui_dom_node_remove_attribute(entry->header_node, "data-state");

  switch (effective_state) {
  case UI_STEPPER_STEP_STATE_ACTIVE:
    ui_dom_node_set_attribute(entry->header_node, "aria-selected", "true");
    ui_dom_node_set_attribute(entry->header_node, "data-state", "active");
    ui_dom_node_remove_attribute(entry->content_node, "hidden");
    break;
  case UI_STEPPER_STEP_STATE_COMPLETED:
    ui_dom_node_set_attribute(entry->header_node, "aria-selected", "false");
    ui_dom_node_set_attribute(entry->header_node, "data-state", "completed");
    ui_dom_node_set_attribute(entry->content_node, "hidden", "true");
    break;
  case UI_STEPPER_STEP_STATE_ERROR:
    ui_dom_node_set_attribute(entry->header_node, "aria-selected", "false");
    ui_dom_node_set_attribute(entry->header_node, "data-state", "error");
    ui_dom_node_set_attribute(entry->content_node, "hidden", "true");
    break;
  default:
  case UI_STEPPER_STEP_STATE_DEFAULT:
    ui_dom_node_set_attribute(entry->header_node, "aria-selected", "false");
    ui_dom_node_set_attribute(entry->header_node, "data-state", "default");
    ui_dom_node_set_attribute(entry->content_node, "hidden", "true");
    break;
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_stepper_base_add_step(struct ui_stepper_base *stepper,
                                       const char *step_id,
                                       struct ui_dom_node *header_node,
                                       struct ui_dom_node *content_node) {
  enum ui_error rc;
  struct ui_stepper_step_entry *new_steps;
  char tab_node_id[256];
  char panel_node_id[256];

  if (!stepper || !step_id || !header_node || !content_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (stepper->step_count >= stepper->step_capacity) {
    int new_cap = stepper->step_capacity == 0 ? 4 : stepper->step_capacity * 2;
    new_steps = (struct ui_stepper_step_entry *)UI_REALLOC(
        stepper->steps, new_cap * sizeof(struct ui_stepper_step_entry));
    if (!new_steps) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    stepper->steps = new_steps;
    stepper->step_capacity = new_cap;
  }

  rc = format_id(tab_node_id, sizeof(tab_node_id), step_id, "step-hdr");
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = format_id(panel_node_id, sizeof(panel_node_id), step_id, "step-cnt");
  if (rc != UI_ERROR_NONE)
    return rc;

  ui_dom_node_set_attribute(header_node, "role", "tab");
  ui_dom_node_set_attribute(header_node, "id", tab_node_id);
  ui_dom_node_set_attribute(header_node, "aria-controls", panel_node_id);

  ui_dom_node_set_attribute(content_node, "role", "tabpanel");
  ui_dom_node_set_attribute(content_node, "id", panel_node_id);
  ui_dom_node_set_attribute(content_node, "aria-labelledby", tab_node_id);

  rc = ui_dom_node_append_child(stepper->header_container_node, header_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_dom_node_append_child(stepper->content_container_node, content_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  {
    char *tmp = NULL;
    enum ui_error err = duplicate_string(step_id, &tmp);
    if (err != UI_ERROR_NONE)
      return err;
    stepper->steps[stepper->step_count].id = tmp;
  }
  if (!stepper->steps[stepper->step_count].id) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  stepper->steps[stepper->step_count].header_node = header_node;
  stepper->steps[stepper->step_count].content_node = content_node;
  stepper->steps[stepper->step_count].explicit_state =
      UI_STEPPER_STEP_STATE_DEFAULT;

  if (stepper->step_count == 0) {
    stepper->active_index = 0;
  }

  (void)apply_step_state_attributes(stepper, stepper->step_count);

  stepper->step_count++;

  return UI_ERROR_NONE;
}

enum ui_error ui_stepper_base_set_active_index(struct ui_stepper_base *stepper,
                                               int index) {
  int i;

  if (!stepper)
    return UI_ERROR_INVALID_ARGUMENT;

  if (index < 0 || index >= stepper->step_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  if (stepper->active_index == index) {
    return UI_ERROR_NONE;
  }

  if (stepper->mode == UI_STEPPER_MODE_LINEAR) {
    if (index > stepper->active_index) {
      for (i = stepper->active_index; i < index; i++) {
        if (stepper->validate_hook) {
          if (!stepper->validate_hook(stepper, i, stepper->user_data)) {
            return UI_ERROR_UNKNOWN;
          }
        }
        /* Automatically mark as completed if moving forward */
        stepper->steps[i].explicit_state = UI_STEPPER_STEP_STATE_COMPLETED;
      }
    }
  }

  stepper->active_index = index;

  for (i = 0; i < stepper->step_count; i++) {
    (void)apply_step_state_attributes(stepper, i);
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_stepper_base_get_active_index(const struct ui_stepper_base *stepper,
                                 int *out_index) {
  if (!stepper || !out_index)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_index = stepper->active_index;
  return UI_ERROR_NONE;
}

enum ui_error ui_stepper_base_set_step_state(struct ui_stepper_base *stepper,
                                             int index,
                                             enum ui_stepper_step_state state) {
  if (!stepper)
    return UI_ERROR_INVALID_ARGUMENT;
  if (index < 0 || index >= stepper->step_count)
    return UI_ERROR_OUT_OF_BOUNDS;

  stepper->steps[index].explicit_state = state;
  (void)apply_step_state_attributes(stepper, index);

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_stepper_base_get_step_state(const struct ui_stepper_base *stepper, int index,
                               enum ui_stepper_step_state *out_state) {
  if (!stepper || !out_state)
    return UI_ERROR_INVALID_ARGUMENT;
  if (index < 0 || index >= stepper->step_count)
    return UI_ERROR_OUT_OF_BOUNDS;

  if (index == stepper->active_index) {
    *out_state = UI_STEPPER_STEP_STATE_ACTIVE;
  } else {
    *out_state = stepper->steps[index].explicit_state;
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_stepper_base_next_step(struct ui_stepper_base *stepper) {
  if (!stepper)
    return UI_ERROR_INVALID_ARGUMENT;
  if (stepper->active_index + 1 >= stepper->step_count) {
    return UI_ERROR_OUT_OF_BOUNDS; /* Already at the end */
  }

  return ui_stepper_base_set_active_index(stepper, stepper->active_index + 1);
}

enum ui_error ui_stepper_base_prev_step(struct ui_stepper_base *stepper) {
  if (!stepper)
    return UI_ERROR_INVALID_ARGUMENT;
  if (stepper->active_index - 1 < 0) {
    return UI_ERROR_OUT_OF_BOUNDS; /* Already at the beginning */
  }

  return ui_stepper_base_set_active_index(stepper, stepper->active_index - 1);
}
/** \brief ui_error */
enum ui_error
ui_stepper_base_get_component(struct ui_stepper_base *stepper,
                              struct ui_component **out_component) {
  if (!stepper || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = stepper->component;
  return UI_ERROR_NONE;
}

enum ui_error ui_stepper_base_bind_active_index(struct ui_stepper_base *widget,
                                                struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
