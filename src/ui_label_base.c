/* clang-format off */
#include "ui_label_base.h"
#include "ui_internal_mem.h"
#include "ui_aria.h"
#include "ui_css_parser.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

struct ui_label_base {
  struct ui_component *component;
  char *target_id;
  struct ui_dom_node *target_node;
  struct ui_signal *text_signal;
};

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
    (void)ui_dom_node_destroy(root_node);
  }
  if (lbl->component) {
    (void)ui_component_destroy(lbl->component);
  }
  C_MULTIPLATFORM_FREE(lbl);
  return rc;
}

ui_error_t ui_label_base_destroy(struct ui_label_base *label) {
  if (!label) {
    return UI_ERROR_NONE;
  }

  if (label->target_id) {
    C_MULTIPLATFORM_FREE(label->target_id);
  }

  (void)ui_component_destroy(label->component);

  C_MULTIPLATFORM_FREE(label);
  return UI_ERROR_NONE;
}

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
    strcpy(label->target_id, target_id);
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

ui_error_t ui_label_base_set_target_node(struct ui_label_base *label,
                                         struct ui_dom_node *target_node) {
  if (!label) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  label->target_node = target_node;
  return UI_ERROR_NONE;
}

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

ui_error_t ui_label_base_get_component(struct ui_label_base *label,
                                       struct ui_component **out_component) {
  if (!label || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = label->component;
  return UI_ERROR_NONE;
}

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
  (void)ui_label_base_create(&lbl);
  (void)ui_component_destroy(lbl->component);
  lbl->component = NULL;
  (void)ui_label_base_set_for(lbl, "fail-target-2");
  (void)ui_label_base_set_for(lbl, NULL);
  (void)ui_label_base_destroy(lbl);
  return UI_ERROR_NONE;
}
#endif
