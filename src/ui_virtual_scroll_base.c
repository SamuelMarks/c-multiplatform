/**
 * @file ui_virtual_scroll_base.c
 * @brief Implementation of the virtual scroll base component.
 */

/* clang-format off */
#include "ui_virtual_scroll_base.h"
#include "ui_internal_mem.h"
#include "ui_dom_node.h"
#include "ui_error.h"
#include <stddef.h>
#include <stdio.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/**
 * @struct ui_virtual_scroll_base
 * @brief Internal state for the virtual scroll base component.
 */
struct ui_virtual_scroll_base {
  struct ui_virtual_scroll_config config;
  size_t item_count;
  float viewport_width;
  float viewport_height;

  struct ui_dom_node *container;

  struct ui_dom_node **active_nodes;
  size_t *active_node_indices;
  size_t num_active_nodes;
  size_t max_active_nodes;
  float *cached_prefix_heights;
  size_t cached_prefix_heights_count;
  struct ui_computed *data_signal;
};

ui_error_t ui_virtual_scroll_base_create(
    struct ui_virtual_scroll_base **out_virtual_scroll,
    const struct ui_virtual_scroll_config *config) {
  struct ui_virtual_scroll_base *vs;

  if (!out_virtual_scroll || !config)
    return UI_ERROR_INVALID_ARGUMENT;

  if (config->strategy == UI_VIRTUAL_SCROLL_FIXED_SIZE &&
      config->fixed_item_size <= 0.0f)
    return UI_ERROR_INVALID_ARGUMENT;

  if (config->strategy == UI_VIRTUAL_SCROLL_VARIABLE_SIZE &&
      !config->get_item_size)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!config->create_node || !config->update_node)
    return UI_ERROR_INVALID_ARGUMENT;

  vs = (struct ui_virtual_scroll_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_virtual_scroll_base));
  if (!vs)
    return UI_ERROR_OUT_OF_MEMORY;

  vs->config = *config;
  vs->item_count = 0;
  vs->viewport_width = 0.0f;
  vs->viewport_height = 0.0f;
  vs->container = NULL;
  vs->active_nodes = NULL;
  vs->active_node_indices = NULL;
  vs->num_active_nodes = 0;
  vs->max_active_nodes = 0;
  vs->cached_prefix_heights = NULL;
  vs->cached_prefix_heights_count = 0;

  *out_virtual_scroll = vs;
  return UI_ERROR_NONE;
}

ui_error_t ui_virtual_scroll_base_destroy(struct ui_virtual_scroll_base *vs) {
  if (!vs)
    return UI_ERROR_NONE;

  if (vs->active_nodes)
    C_MULTIPLATFORM_FREE(vs->active_nodes);
  if (vs->active_node_indices)
    C_MULTIPLATFORM_FREE(vs->active_node_indices);
  if (vs->cached_prefix_heights)
    C_MULTIPLATFORM_FREE(vs->cached_prefix_heights);

  C_MULTIPLATFORM_FREE(vs);
  return UI_ERROR_NONE;
}

ui_error_t
ui_virtual_scroll_base_set_item_count(struct ui_virtual_scroll_base *vs,
                                      size_t count) {
  if (!vs)
    return UI_ERROR_INVALID_ARGUMENT;

  vs->item_count = count;

  if (vs->config.strategy == UI_VIRTUAL_SCROLL_VARIABLE_SIZE) {
    float *new_cache = (float *)C_MULTIPLATFORM_REALLOC(
        vs->cached_prefix_heights, (count + 1) * sizeof(float));
    if (!new_cache && count > 0)
      return UI_ERROR_OUT_OF_MEMORY;
    vs->cached_prefix_heights = new_cache;
    vs->cached_prefix_heights_count = count + 1;

    if (count > 0) {
      size_t i;
      float total = 0.0f;
      vs->cached_prefix_heights[0] = 0.0f;
      for (i = 0; i < count; i++) {
        total += vs->config.get_item_size(i, vs->config.user_data);
        vs->cached_prefix_heights[i + 1] = total;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
ui_virtual_scroll_base_set_viewport_size(struct ui_virtual_scroll_base *vs,
                                         float width, float height) {
  if (!vs)
    return UI_ERROR_INVALID_ARGUMENT;

  if (width < 0.0f)
    width = 0.0f;
  if (height < 0.0f)
    height = 0.0f;

  vs->viewport_width = width;
  vs->viewport_height = height;

  return UI_ERROR_NONE;
}

ui_error_t
ui_virtual_scroll_base_get_total_height(const struct ui_virtual_scroll_base *vs,
                                        float *out_height) {
  if (!vs || !out_height)
    return UI_ERROR_INVALID_ARGUMENT;

  if (vs->item_count == 0) {
    *out_height = 0.0f;
    return UI_ERROR_NONE;
  }

  if (vs->config.strategy == UI_VIRTUAL_SCROLL_FIXED_SIZE) {
    *out_height = (float)vs->item_count * vs->config.fixed_item_size;
  } else {
    if (vs->cached_prefix_heights &&
        vs->cached_prefix_heights_count > vs->item_count) {
      *out_height = vs->cached_prefix_heights[vs->item_count];
    } else {
      *out_height = 0.0f;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_virtual_scroll_base_get_visible_range(
    const struct ui_virtual_scroll_base *vs, float scroll_y,
    size_t *out_start_index, size_t *out_end_index, float *out_offset_y) {
  size_t start = 0, end = 0;
  float offset_y = 0.0f;
  size_t low, high, mid;

  if (!vs || !out_start_index || !out_end_index || !out_offset_y)
    return UI_ERROR_INVALID_ARGUMENT;

  if (vs->item_count == 0) {
    *out_start_index = 0;
    *out_end_index = 0;
    *out_offset_y = 0.0f;
    return UI_ERROR_NONE;
  }

  if (scroll_y < 0.0f)
    scroll_y = 0.0f;

  if (vs->config.strategy == UI_VIRTUAL_SCROLL_FIXED_SIZE) {
    start = (size_t)(scroll_y / vs->config.fixed_item_size);
    if (start >= vs->item_count) {
      start = vs->item_count - 1;
    }
    offset_y = (float)start * vs->config.fixed_item_size;
    end = start + (size_t)(vs->viewport_height / vs->config.fixed_item_size) +
          2; /* +2 for buffer */
    if (end >= vs->item_count) {
      end = vs->item_count - 1;
    }
  } else {
    /* Binary search on cached prefix heights */
    low = 0;
    high = vs->item_count;

    while (low < high) {
      mid = low + (high - low) / 2;
      if (vs->cached_prefix_heights[mid + 1] <= scroll_y) {
        low = mid + 1;
      } else {
        high = mid;
      }
    }
    start = low;
    if (start >= vs->item_count) {
      start = vs->item_count - 1;
    }
    offset_y = vs->cached_prefix_heights[start];

    end = start;
    while (end < vs->item_count && vs->cached_prefix_heights[end + 1] <
                                       scroll_y + vs->viewport_height) {
      end++;
    }
    /* Add a small buffer */
    if (end + 1 < vs->item_count)
      end++;

    if (end >= vs->item_count) {
      end = vs->item_count - 1;
    }
  }

  *out_start_index = start;
  *out_end_index = end;
  *out_offset_y = offset_y;

  return UI_ERROR_NONE;
}

ui_error_t ui_virtual_scroll_base_render(struct ui_virtual_scroll_base *vs,
                                         float scroll_y) {
  size_t start, end, visible_count, i;
  float offset_y;
  ui_error_t rc;
  char style_buf[128];
  struct ui_dom_node **new_nodes = NULL;
  size_t *new_indices = NULL;

  if (!vs || !vs->container)
    return UI_ERROR_INVALID_ARGUMENT;

  ui_virtual_scroll_base_get_visible_range(vs, scroll_y, &start, &end,
                                           &offset_y);

  if (vs->item_count == 0) {
    /* Hide all active nodes */
    for (i = 0; i < vs->num_active_nodes; i++) {
      ui_dom_node_set_attribute(vs->active_nodes[i], "style", "display: none;");
    }
    return UI_ERROR_NONE;
  }

  visible_count = end - start + 1;

  if (visible_count > vs->max_active_nodes) {
    new_nodes = (struct ui_dom_node **)C_MULTIPLATFORM_REALLOC(
        vs->active_nodes, visible_count * sizeof(struct ui_dom_node *));
    if (!new_nodes)
      return UI_ERROR_OUT_OF_MEMORY;
    vs->active_nodes = new_nodes;

    new_indices = (size_t *)C_MULTIPLATFORM_REALLOC(
        vs->active_node_indices, visible_count * sizeof(size_t));
    if (!new_indices)
      return UI_ERROR_OUT_OF_MEMORY;
    vs->active_node_indices = new_indices;

    for (i = vs->max_active_nodes; i < visible_count; i++) {
      rc = vs->config.create_node(start + i, &vs->active_nodes[i],
                                  vs->config.user_data);
      if (rc != UI_ERROR_NONE) {
        /* In a complete engine we'd handle cleanup better here, but UI_ERROR
         * propagation is key */
        return rc;
      }
      ui_dom_node_append_child(vs->container, vs->active_nodes[i]);
      vs->active_node_indices[i] = (size_t)-1; /* Invalid index */
    }
    vs->max_active_nodes = visible_count;
  }

  vs->num_active_nodes = visible_count;

  for (i = 0; i < visible_count; i++) {
    size_t item_index = start + i;
    struct ui_dom_node *node = vs->active_nodes[i];

    if (vs->active_node_indices[i] != item_index) {
      rc = vs->config.update_node(item_index, node, vs->config.user_data);
      if (rc != UI_ERROR_NONE)
        return rc;
      vs->active_node_indices[i] = item_index;
    }

#if defined(_MSC_VER)
    if (vs->config.orientation == UI_VIRTUAL_SCROLL_ORIENTATION_HORIZONTAL) {
      sprintf_s(
          style_buf, sizeof(style_buf),
          "position: absolute; left: %fpx; height: 100%%; display: block;",
          offset_y);
    } else {
      sprintf_s(style_buf, sizeof(style_buf),
                "position: absolute; top: %fpx; width: 100%%; display: block;",
                offset_y);
    }
#else
    if (vs->config.orientation == UI_VIRTUAL_SCROLL_ORIENTATION_HORIZONTAL) {
      sprintf(style_buf,
              "position: absolute; left: %fpx; height: 100%%; display: block;",
              offset_y);
    } else {
      sprintf(style_buf,
              "position: absolute; top: %fpx; width: 100%%; display: block;",
              offset_y);
    }
#endif

    ui_dom_node_set_attribute(node, "style", style_buf);

    if (vs->config.strategy == UI_VIRTUAL_SCROLL_FIXED_SIZE) {
      offset_y += vs->config.fixed_item_size;
    } else {
      offset_y += vs->config.get_item_size(item_index, vs->config.user_data);
    }
  }

  /* Hide the unused active nodes to essentially "recycle" them */
  for (i = visible_count; i < vs->max_active_nodes; i++) {
    ui_dom_node_set_attribute(vs->active_nodes[i], "style", "display: none;");
    vs->active_node_indices[i] = (size_t)-1;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_virtual_scroll_base_mount(struct ui_virtual_scroll_base *vs,
                                        struct ui_dom_node *container) {
  if (!vs || !container)
    return UI_ERROR_INVALID_ARGUMENT;

  vs->container = container;
  return UI_ERROR_NONE;
}

ui_error_t
ui_virtual_scroll_base_bind_data(struct ui_virtual_scroll_base *widget,
                                 struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
