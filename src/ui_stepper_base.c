/**
 * @file ui_stepper_base.c
 * @brief ui_stepper_base.c implementation.
 */
/* clang-format off */
#include "ui_stepper_base.h"
#include "ui_aria.h"
#include "ui_css_parser.h"
#include "ui_internal_mem.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
/* clang-format on */

/*
 * \file ui_stepper_base.c
 * \brief Stepper base component implementation.
 */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

#ifdef UI_TEST_MOCK_ALLOC
int g_stepper_mock_fail = 0;
int g_stepper_mock_append_fail_target = 0;
int g_stepper_mock_set_attr_fail_target = 0;
int g_stepper_mock_remove_attr_fail_target = 0;
static int g_stepper_append_counter = 0;
static int g_stepper_set_attr_counter = 0;
static int g_stepper_remove_attr_counter = 0;

/**
 * @brief mock_stepper_dom_node_append_child.
 * @param parent Parent node.
 * @param child Child node.
 * @return Return value.
 */
static ui_error_t
mock_stepper_dom_node_append_child(struct ui_dom_node *parent,
                                   struct ui_dom_node *child) {
  if (g_stepper_mock_append_fail_target > 0) {
    if (++g_stepper_append_counter == g_stepper_mock_append_fail_target) {
      g_stepper_append_counter = 0;
      return UI_ERROR_UNKNOWN;
    }
  }
  return ui_dom_node_append_child(parent, child);
}
/** @cond */
#define ui_dom_node_append_child mock_stepper_dom_node_append_child
/** @endcond */

/**
 * @brief mock_stepper_component_set_default_style.
 * @param comp Component.
 * @param style Stylesheet.
 * @return Return value.
 */
static ui_error_t
mock_stepper_component_set_default_style(struct ui_component *comp,
                                         struct ui_css_stylesheet *style) {
  if (g_stepper_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_set_default_style(comp, style);
}
/** @cond */
#define ui_component_set_default_style mock_stepper_component_set_default_style
/** @endcond */

/**
 * @brief mock_stepper_dom_node_destroy.
 * @param node Node.
 * @return Return value.
 */
static ui_error_t mock_stepper_dom_node_destroy(struct ui_dom_node *node) {
  if (g_stepper_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_dom_node_destroy(node);
}
/** @cond */
#define ui_dom_node_destroy mock_stepper_dom_node_destroy
/** @endcond */

/**
 * @brief mock_stepper_component_destroy.
 * @param comp Component.
 * @return Return value.
 */
static ui_error_t mock_stepper_component_destroy(struct ui_component *comp) {
  if (g_stepper_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_destroy(comp);
}
/** @cond */
#define ui_component_destroy mock_stepper_component_destroy
/** @endcond */

/**
 * @brief mock_stepper_dom_node_set_attribute.
 * @param node Node.
 * @param name Name.
 * @param val Value.
 * @return Return value.
 */
static ui_error_t mock_stepper_dom_node_set_attribute(struct ui_dom_node *node,
                                                      const char *name,
                                                      const char *val) {
  if (g_stepper_mock_set_attr_fail_target > 0) {
    if (++g_stepper_set_attr_counter == g_stepper_mock_set_attr_fail_target) {
      g_stepper_set_attr_counter = 0;
      return UI_ERROR_UNKNOWN;
    }
  }
  return ui_dom_node_set_attribute(node, name, val);
}
/** @cond */
#define ui_dom_node_set_attribute mock_stepper_dom_node_set_attribute
/** @endcond */

/**
 * @brief mock_stepper_dom_node_remove_attribute.
 * @param node Node.
 * @param name Name.
 * @return Return value.
 */
static ui_error_t
mock_stepper_dom_node_remove_attribute(struct ui_dom_node *node,
                                       const char *name) {
  if (g_stepper_mock_remove_attr_fail_target > 0) {
    if (++g_stepper_remove_attr_counter ==
        g_stepper_mock_remove_attr_fail_target) {
      g_stepper_remove_attr_counter = 0;
      return UI_ERROR_UNKNOWN;
    }
  }
  return ui_dom_node_remove_attribute(node, name);
}
/** @cond */
#define ui_dom_node_remove_attribute mock_stepper_dom_node_remove_attribute
/** @endcond */
#endif

/** @brief Default CSS stylesheet */
static const char *ui_stepper_base_default_css =
    ".ui-stepper { display: flex; flex-direction: column; } "
    ".ui-stepper-header { display: flex; flex-direction: row; } "
    ".ui-stepper-content { display: flex; flex-direction: column; flex: 1; }";

/**
 * @struct ui_stepper_step_entry
 * \brief ui_stepper_step_entry structure.
 * \details Internal state for a stepper step.
 */
struct ui_stepper_step_entry {
  char *id;                                  /**< id */
  struct ui_dom_node *header_node;           /**< header_node */
  struct ui_dom_node *content_node;          /**< content_node */
  enum ui_stepper_step_state explicit_state; /**< explicit_state */
};

/**
 * @struct ui_stepper_base
 * \brief ui_stepper_base structure.
 * \details Internal state for the stepper base component.
 */
struct ui_stepper_base {
  struct ui_component *component;             /**< component */
  struct ui_dom_node *header_container_node;  /**< header_container_node */
  struct ui_dom_node *content_container_node; /**< content_container_node */

  struct ui_stepper_step_entry *steps; /**< steps */
  int step_count;                      /**< step_count */
  int step_capacity;                   /**< step_capacity */

  int active_index;          /**< active_index */
  enum ui_stepper_mode mode; /**< mode */

  ui_stepper_validate_t validate_hook;   /**< validate_hook */
  void *user_data;                       /**< user_data */
  struct ui_signal *active_index_signal; /**< active_index_signal */
};

/**
 * \brief Creates a new stepper base component.
 * \param out_stepper Pointer to store the component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_stepper_base_create(struct ui_stepper_base **out_stepper) {
  struct ui_stepper_base *stepper;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_dom_node *header_node = NULL;
  struct ui_dom_node *content_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_stepper) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  stepper = (struct ui_stepper_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_stepper_base));
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
  {
    ui_error_t dom_rc = ui_dom_node_set_tag_name(root_node, "div");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }
  {
    ui_error_t dom_rc =
        ui_dom_node_set_attribute(root_node, "class", "ui-stepper");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &header_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  {
    ui_error_t dom_rc = ui_dom_node_set_tag_name(header_node, "div");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }
  {
    ui_error_t dom_rc =
        ui_dom_node_set_attribute(header_node, "class", "ui-stepper-header");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }
  {
    ui_error_t dom_rc =
        ui_dom_node_set_attribute(header_node, "role", "tablist");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  {
    ui_error_t dom_rc = ui_dom_node_set_tag_name(content_node, "div");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }
  {
    ui_error_t dom_rc =
        ui_dom_node_set_attribute(content_node, "class", "ui-stepper-content");
    if (dom_rc != UI_ERROR_NONE)
      return dom_rc;
  }

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
  if (root_node) {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  if (stepper->component) {
    ui_error_t rc_cleanup = ui_component_destroy(stepper->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  C_MULTIPLATFORM_FREE(stepper);
  return rc;
}

/**
 * \brief Destroys a stepper base component.
 * \param stepper The component to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_stepper_base_destroy(struct ui_stepper_base *stepper) {
  int i;
  ui_error_t rc = UI_ERROR_NONE;
  if (!stepper)
    return UI_ERROR_NONE;

  for (i = 0; i < stepper->step_count; i++) {
    C_MULTIPLATFORM_FREE(stepper->steps[i].id);
  }
  C_MULTIPLATFORM_FREE(stepper->steps);

  if (stepper->component) {
    ui_error_t rc_cleanup = ui_component_destroy(stepper->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  C_MULTIPLATFORM_FREE(stepper);
  return rc;
}

/**
 * \brief Sets the mode of the stepper.
 * \param stepper The stepper component.
 * \param mode The mode to set.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_stepper_base_set_mode(struct ui_stepper_base *stepper,
                                    enum ui_stepper_mode mode) {
  if (!stepper)
    return UI_ERROR_INVALID_ARGUMENT;
  stepper->mode = mode;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the validation hook for the stepper.
 * \param stepper The stepper component.
 * \param hook The hook function.
 * \param user_data User data for the hook.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_stepper_base_set_validate_hook(struct ui_stepper_base *stepper,
                                             ui_stepper_validate_t hook,
                                             void *user_data) {
  if (!stepper)
    return UI_ERROR_INVALID_ARGUMENT;
  stepper->validate_hook = hook;
  stepper->user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * \brief Duplicates a string.
 * \param src The source string.
 * \param out_copy Pointer to store the copy.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief duplicate_string.
 * @param src Parameter src.
 * @param out_copy Parameter out_copy.
 * @return Return value.
 */
static ui_error_t duplicate_string(const char *src, char **out_copy) {
  char *dst;
  *out_copy = NULL;
  dst = C_MULTIPLATFORM_STRDUP(src);
  if (!dst)
    return UI_ERROR_OUT_OF_MEMORY;
  *out_copy = dst;
  return UI_ERROR_NONE;
}

/**
 * \brief Formats an ID string.
 * \param buf The buffer to write to.
 * \param buf_size The size of the buffer.
 * \param prefix The prefix.
 * \param suffix The suffix.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief format_id.
 * @param buf Parameter buf.
 * @param buf_size Parameter buf_size.
 * @param prefix Parameter prefix.
 * @param suffix Parameter suffix.
 * @return Return value.
 */
static ui_error_t format_id(char *buf, size_t buf_size, const char *prefix,
                            const char *suffix) {
#if !defined(_MSC_VER)
  size_t unused_size;
#endif
#ifdef UI_TEST_MOCK_ALLOC
  if (g_stepper_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_stepper_mock_fail == 5) {
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

/**
 * \brief Applies state attributes to a step.
 * \param stepper The stepper component.
 * \param index The step index.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief apply_step_state_attributes.
 * @param stepper Parameter stepper.
 * @param index Parameter index.
 * @return Return value.
 */
static ui_error_t apply_step_state_attributes(struct ui_stepper_base *stepper,
                                              int index) {
  struct ui_stepper_step_entry *entry = &stepper->steps[index];
  enum ui_stepper_step_state effective_state = entry->explicit_state;
  ui_error_t rc;

  if (index == stepper->active_index) {
    effective_state = UI_STEPPER_STEP_STATE_ACTIVE;
  }

  rc = ui_dom_node_remove_attribute(entry->header_node, "data-state");
  if (rc != UI_ERROR_NONE)
    return rc;

  switch (effective_state) {
  case UI_STEPPER_STEP_STATE_ACTIVE:
    rc = ui_dom_node_set_attribute(entry->header_node, "aria-selected", "true");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(entry->header_node, "data-state", "active");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_remove_attribute(entry->content_node, "hidden");
    if (rc != UI_ERROR_NONE)
      return rc;
    break;
  case UI_STEPPER_STEP_STATE_COMPLETED:
    rc =
        ui_dom_node_set_attribute(entry->header_node, "aria-selected", "false");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(entry->header_node, "data-state",
                                   "completed");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(entry->content_node, "hidden", "true");
    if (rc != UI_ERROR_NONE)
      return rc;
    break;
  case UI_STEPPER_STEP_STATE_ERROR:
    rc =
        ui_dom_node_set_attribute(entry->header_node, "aria-selected", "false");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(entry->header_node, "data-state", "error");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(entry->content_node, "hidden", "true");
    if (rc != UI_ERROR_NONE)
      return rc;
    break;
  default:
  case UI_STEPPER_STEP_STATE_DEFAULT:
    rc =
        ui_dom_node_set_attribute(entry->header_node, "aria-selected", "false");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(entry->header_node, "data-state", "default");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_dom_node_set_attribute(entry->content_node, "hidden", "true");
    if (rc != UI_ERROR_NONE)
      return rc;
    break;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Adds a step to the stepper.
 * \param stepper The stepper component.
 * \param step_id The ID of the step.
 * \param header_node The header node.
 * \param content_node The content node.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_stepper_base_add_step(struct ui_stepper_base *stepper,
                                    const char *step_id,
                                    struct ui_dom_node *header_node,
                                    struct ui_dom_node *content_node) {
  struct ui_stepper_step_entry *new_steps;
  char tab_node_id[256];
  char panel_node_id[256];
  ui_error_t rc;

  if (!stepper || !step_id || !header_node || !content_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (stepper->step_count >= stepper->step_capacity) {
    int new_cap = stepper->step_capacity == 0 ? 4 : stepper->step_capacity * 2;
    new_steps = (struct ui_stepper_step_entry *)C_MULTIPLATFORM_REALLOC(
        stepper->steps, (size_t)new_cap * sizeof(struct ui_stepper_step_entry));
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

  rc = ui_dom_node_set_attribute(header_node, "role", "tab");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(header_node, "id", tab_node_id);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(header_node, "aria-controls", panel_node_id);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_dom_node_set_attribute(content_node, "role", "tabpanel");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(content_node, "id", panel_node_id);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(content_node, "aria-labelledby", tab_node_id);
  if (rc != UI_ERROR_NONE)
    return rc;

  {
    char *tmp = NULL;
    ui_error_t err = duplicate_string(step_id, &tmp);
    if (err != UI_ERROR_NONE)
      return err;
    stepper->steps[stepper->step_count].id = tmp;
  }

  rc = ui_dom_node_append_child(stepper->header_container_node, header_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_dom_node_append_child(stepper->content_container_node, content_node);
  if (rc != UI_ERROR_NONE)
    return rc;

  stepper->steps[stepper->step_count].header_node = header_node;
  stepper->steps[stepper->step_count].content_node = content_node;
  stepper->steps[stepper->step_count].explicit_state =
      UI_STEPPER_STEP_STATE_DEFAULT;

  if (stepper->step_count == 0) {
    stepper->active_index = 0;
  }

  rc = apply_step_state_attributes(stepper, stepper->step_count);
  if (rc != UI_ERROR_NONE)
    return rc;

  stepper->step_count++;

  return UI_ERROR_NONE;
}

/**
 * \brief Sets the active step index.
 * \param stepper The stepper component.
 * \param index The index to activate.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_stepper_base_set_active_index(struct ui_stepper_base *stepper,
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

  stepper->active_index = index;

  for (i = 0; i < stepper->step_count; i++) {
    ui_error_t rc_step = apply_step_state_attributes(stepper, i);
    if (rc_step != UI_ERROR_NONE)
      return rc_step;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Gets the current active step index.
 * \param stepper The stepper component.
 * \param out_index Pointer to store the index.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_stepper_base_get_active_index(const struct ui_stepper_base *stepper,
                                 int *out_index) {
  if (!stepper || !out_index)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_index = stepper->active_index;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the state of a step.
 * \param stepper The stepper component.
 * \param index The step index.
 * \param state The state to set.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_stepper_base_set_step_state(struct ui_stepper_base *stepper,
                                          int index,
                                          enum ui_stepper_step_state state) {
  if (!stepper)
    return UI_ERROR_INVALID_ARGUMENT;
  if (index < 0 || index >= stepper->step_count)
    return UI_ERROR_OUT_OF_BOUNDS;

  stepper->steps[index].explicit_state = state;
  return apply_step_state_attributes(stepper, index);
}

/**
 * \brief Gets the current state of a step.
 * \param stepper The stepper component.
 * \param index The step index.
 * \param out_state Pointer to store the state.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
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

/**
 * \brief Moves to the next step.
 * \param stepper The stepper component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_stepper_base_next_step(struct ui_stepper_base *stepper) {
  if (!stepper)
    return UI_ERROR_INVALID_ARGUMENT;
  if (stepper->active_index + 1 >= stepper->step_count) {
    return UI_ERROR_OUT_OF_BOUNDS; /* Already at the end */
  }

  return ui_stepper_base_set_active_index(stepper, stepper->active_index + 1);
}

/**
 * \brief Moves to the previous step.
 * \param stepper The stepper component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_stepper_base_prev_step(struct ui_stepper_base *stepper) {
  if (!stepper)
    return UI_ERROR_INVALID_ARGUMENT;
  if (stepper->active_index - 1 < 0) {
    return UI_ERROR_OUT_OF_BOUNDS; /* Already at the beginning */
  }

  return ui_stepper_base_set_active_index(stepper, stepper->active_index - 1);
}
/**
 * \brief Gets the base component for the stepper.
 * \param stepper The stepper component.
 * \param out_component Pointer to store the component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_stepper_base_get_component(struct ui_stepper_base *stepper,
                                         struct ui_component **out_component) {
  if (!stepper || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = stepper->component;
  return UI_ERROR_NONE;
}

/**
 * \brief Binds the active index state to a signal.
 * \param widget The stepper component.
 * \param signal The signal to bind.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_stepper_base_bind_active_index(struct ui_stepper_base *widget,
                                             struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
