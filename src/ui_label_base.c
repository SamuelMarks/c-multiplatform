/**
 * @file ui_label_base.c
 * @brief ui_label_base.c implementation.
 */
/*
 * @file ui_label_base.c
 * @brief Implementation of the UI label base component.
 */
/* clang-format off */
#include "ui_label_base.h"
#include "ui_internal_mem.h"
#include "ui_aria.h"
#include "ui_css_parser.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

/**
 * @struct ui_label_base
 * @struct ui_label_base
 * @brief Represents a label component, capable of targeting another DOM node.
 */
struct ui_label_base {
  struct ui_component *component;  /**< component */
  char *target_id;                 /**< target_id */
  struct ui_dom_node *target_node; /**< target_node */
  struct ui_signal *text_signal;   /**< text_signal */
};

/**
 * @brief Creates a new label base component.
 * @param[out] out_label Pointer to store the created label.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_label_base_create(struct ui_label_base **out_label) {
  struct ui_label_base *lbl;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;

  if (!out_label) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  lbl = (struct ui_label_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_label_base));
  if (!lbl) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  lbl->component = NULL;
  lbl->target_id = NULL;
  lbl->target_node = NULL;

  rc = ui_component_create(&lbl->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "label");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  lbl->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

  *out_label = lbl;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  if (lbl->component) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(lbl->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  C_MULTIPLATFORM_FREE(lbl);
  return rc;
}

/**
 * @brief Destroys a label base component.
 * @param[in,out] label The label component to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_label_base_destroy(struct ui_label_base *label) {
  if (!label) {
    return UI_ERROR_NONE;
  }

  if (label->target_id) {
    C_MULTIPLATFORM_FREE(label->target_id);
  }

  {
    ui_error_t rc_cleanup = ui_component_destroy(label->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  C_MULTIPLATFORM_FREE(label);
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the "for" attribute of the label, identifying the target element
 * by ID.
 * @param[in,out] label The label component.
 * @param[in] target_id The ID of the target element.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_label_base_set_for(struct ui_label_base *label,
                                 const char *target_id) {
  size_t len;

  if (!label) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (label->target_id) {
    C_MULTIPLATFORM_FREE(label->target_id);
    label->target_id = NULL;
  }

  if (target_id) {
    len = strlen(target_id);
    label->target_id = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
    if (!label->target_id) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
#if defined(_MSC_VER)
    strcpy_s(label->target_id, len + 1, target_id);
#else
    UI_STRCPY(label->target_id, sizeof(label->target_id), target_id);
#endif

    if (label->component && label->component->shadow_root) {
      ui_dom_node_set_attribute(label->component->shadow_root, "for",
                                target_id);
    }
  } else {
    if (label->component && label->component->shadow_root) {
      ui_dom_node_remove_attribute(label->component->shadow_root, "for");
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Directly associates the label with a target DOM node.
 * @param[in,out] label The label component.
 * @param[in,out] target_node The target DOM node.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_label_base_set_target_node(struct ui_label_base *label,
                                         struct ui_dom_node *target_node) {
  if (!label) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  label->target_node = target_node;
  return UI_ERROR_NONE;
}

/**
 * @brief Processes UI events for the label (e.g., clicking the label to focus
 * the target).
 * @param[in,out] label The label component.
 * @param[in] event The UI event.
 * @param[in] timestamp_ms Event timestamp in milliseconds.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_label_base_process_event(struct ui_label_base *label,
                                       const struct ui_event *event,
                                       double timestamp_ms) {
  /* In a real engine, we'd fire a click event on target_node, or search for
   * target_id globally if target_node is null. This acts as a stub to be
   * elaborated upon. */
  if (!label || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Just a placeholder for actual dispatch logic to the bound node. */
  (void)timestamp_ms;

  return UI_ERROR_NONE;
}

/**
 * @brief Gets the underlying DOM component of the label.
 * @param[in] label The label component.
 * @param[out] out_component Pointer to store the DOM component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_label_base_get_component(struct ui_label_base *label,
                                       struct ui_component **out_component) {
  if (!label || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = label->component;
  return UI_ERROR_NONE;
}

/**
 * @brief Binds the label's text content to a reactive signal.
 * @param[in,out] widget The label component.
 * @param[in,out] signal The signal providing the text.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_label_base_bind_text(struct ui_label_base *widget,
                                   struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = signal;
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
ui_error_t ui_test_label_base_set_for_no_component(void);

ui_error_t ui_test_label_base_set_for_no_component(void) {
  struct ui_label_base *lbl = NULL;
  {
    ui_error_t rc_cleanup = ui_label_base_create(&lbl);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(lbl->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  lbl->component = NULL;
  {
    ui_error_t rc_cleanup = ui_label_base_set_for(lbl, "fail-target-2");
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_label_base_set_for(lbl, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_label_base_destroy(lbl);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return UI_ERROR_NONE;
}
#endif
