/* clang-format off */
#include "ui_section_index_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

static const char *ui_section_index_default_css =
    "div.section-index { "
    "display: flex; "
    "flex-direction: column; "
    "align-items: center; "
    "justify-content: center; "
    "touch-action: none; "
    "}"
    "div.section-index-item { "
    "font-size: var(--section-index-font-size, 11px); "
    "color: var(--section-index-color, #000); "
    "user-select: none; "
    "}";

/** \brief ui_section_index_base */
struct ui_section_index_base {
  struct ui_component *component;
  struct ui_dom_node **item_nodes;
  size_t count;
  int active_idx;
  struct ui_computed *data_signal;
};

/** \brief ui_error */
ui_error_t
ui_section_index_base_create(struct ui_section_index_base **out_index) {
  struct ui_section_index_base *index;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  index = (struct ui_section_index_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_section_index_base));
  if (!index) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  index->component = NULL;
  index->item_nodes = NULL;
  index->count = 0;
  index->active_idx = -1;

  rc = ui_component_create(&index->component);
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

  rc = ui_dom_node_set_attribute(root_node, "class", "section-index");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_section_index_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  {

    ui_error_t _ign_rc =
        ui_component_set_default_style(index->component, default_style);

    (void)_ign_rc;
  }

  index->component->shadow_root = root_node;
  root_node = NULL;

  *out_index = index;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  (void)ui_component_destroy(index->component);
  C_MULTIPLATFORM_FREE(index);
  return rc;
}

ui_error_t ui_section_index_base_destroy(struct ui_section_index_base *index) {
  if (!index) {
    return UI_ERROR_NONE;
  }

  if (index->item_nodes) {
    C_MULTIPLATFORM_FREE(index->item_nodes);
  }

  (void)ui_component_destroy(index->component);
  C_MULTIPLATFORM_FREE(index);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_section_index_base_get_component(struct ui_section_index_base *index,
                                    struct ui_component **out_component) {
  if (!index || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = index->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_section_index_base_set_sections(struct ui_section_index_base *index,
                                   const char **sections, size_t count) {
  size_t i;
  ui_error_t rc;

  if (!index || (!sections && count > 0)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Clear existing items */
  if (index->item_nodes) {
    for (i = 0; i < index->count; ++i) {
      ui_dom_node_remove_child(index->component->shadow_root,
                               index->item_nodes[i]);
    }
    C_MULTIPLATFORM_FREE(index->item_nodes);
    index->item_nodes = NULL;
  }
  index->count = 0;
  index->active_idx = -1;

  if (count == 0) {
    return UI_ERROR_NONE;
  }

  index->item_nodes = (struct ui_dom_node **)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_dom_node *) * count);
  if (!index->item_nodes) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  for (i = 0; i < count; ++i) {
    struct ui_dom_node *node = NULL;
    struct ui_dom_node *text_node = NULL;

    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
    if (rc == UI_ERROR_NONE) {
      rc = ui_dom_node_set_tag_name(node, "div");
    }
    if (rc == UI_ERROR_NONE) {
      rc = ui_dom_node_set_attribute(node, "class", "section-index-item");
    }
    if (rc == UI_ERROR_NONE) {
      rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    }
    if (rc == UI_ERROR_NONE) {
      rc = ui_dom_node_set_text_content(text_node, sections[i]);
    }
    if (rc == UI_ERROR_NONE) {
      rc = ui_dom_node_append_child(node, text_node);
    } else if (text_node) {
      (void)ui_dom_node_destroy(text_node);
    }

    if (rc == UI_ERROR_NONE) {
      rc = ui_dom_node_append_child(index->component->shadow_root, node);
    }

    if (rc != UI_ERROR_NONE) {
      size_t j;
      if (node) {
        (void)ui_dom_node_destroy(node);
      }
      for (j = 0; j < i; ++j) {
        ui_dom_node_remove_child(index->component->shadow_root,
                                 index->item_nodes[j]);
      }
      C_MULTIPLATFORM_FREE(index->item_nodes);
      index->item_nodes = NULL;
      return rc;
    }

    index->item_nodes[i] = node;
  }

  index->count = count;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_section_index_base_set_active_section(struct ui_section_index_base *index,
                                         int active_idx) {
  if (!index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (active_idx >= (int)index->count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  if (index->active_idx >= 0) {
    {
      ui_error_t rm_rc = ui_dom_node_remove_attribute(
          index->item_nodes[index->active_idx], "data-active");
      if (rm_rc != UI_ERROR_NONE) {
        return rm_rc;
      }
    }
  }

  index->active_idx = active_idx;

  if (index->active_idx >= 0) {
    {
      ui_error_t sa_rc = ui_dom_node_set_attribute(
          index->item_nodes[index->active_idx], "data-active", "true");
      if (sa_rc != UI_ERROR_NONE) {
        return sa_rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_section_index_base_bind_data(struct ui_section_index_base *widget,
                                           struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
