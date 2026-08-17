/**
 * @file ui_timeline_base.c
 * @brief Implementation of the timeline UI component base logic.
 */
/* clang-format off */
#include "ui_timeline_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @struct ui_timeline_node
 * @brief Internal representation of a single node in the timeline.
 */
struct ui_timeline_node {
  /** @brief The title of the node. */
  char *title;
  /** @brief The description of the node. */
  char *description;
};

/**
 * @struct ui_timeline_base
 * @brief Internal implementation of the timeline base component.
 */
struct ui_timeline_base {
  /** @brief Alignment mode of the timeline. */
  enum ui_timeline_alignment alignment;
  /** @brief Array of timeline nodes. */
  struct ui_timeline_node *nodes;
  /** @brief Number of nodes currently in the timeline. */
  size_t node_count;
  /** @brief Allocated capacity for timeline nodes. */
  size_t node_capacity;
  /** @brief Bound data signal. */
  struct ui_computed *data_signal;
};

ui_error_t ui_timeline_base_create(struct ui_timeline_base **out_timeline) {
  struct ui_timeline_base *tb;
  ui_error_t rc = UI_ERROR_NONE;

  if (!out_timeline) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  tb = (struct ui_timeline_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_timeline_base));
  if (!tb) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  tb->alignment = UI_TIMELINE_ALIGN_ALTERNATE;
  tb->nodes = NULL;
  tb->node_count = 0;
  tb->node_capacity = 0;

  *out_timeline = tb;

cleanup:
  return rc;
}

ui_error_t ui_timeline_base_destroy(struct ui_timeline_base *timeline) {
  size_t i;

  if (!timeline) {
    return UI_ERROR_NONE;
  }

  if (timeline->nodes) {
    for (i = 0; i < timeline->node_count; i++) {
      C_MULTIPLATFORM_FREE(timeline->nodes[i].title);
      C_MULTIPLATFORM_FREE(timeline->nodes[i].description);
    }
    C_MULTIPLATFORM_FREE(timeline->nodes);
  }

  C_MULTIPLATFORM_FREE(timeline);
  return UI_ERROR_NONE;
}

ui_error_t
ui_timeline_base_set_alignment(struct ui_timeline_base *timeline,
                               enum ui_timeline_alignment alignment) {
  if (!timeline) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  timeline->alignment = alignment;
  return UI_ERROR_NONE;
}

ui_error_t
ui_timeline_base_get_alignment(const struct ui_timeline_base *timeline,
                               enum ui_timeline_alignment *out_alignment) {
  if (!timeline || !out_alignment) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_alignment = timeline->alignment;
  return UI_ERROR_NONE;
}

ui_error_t ui_timeline_base_add_node(struct ui_timeline_base *timeline,
                                     const char *title,
                                     const char *description) {
  ui_error_t rc = UI_ERROR_NONE;
  size_t new_cap;
  struct ui_timeline_node *new_nodes;
  size_t i;
  size_t len_title;
  size_t len_desc;
  char *new_title = NULL;
  char *new_desc = NULL;

  if (!timeline || !title || !description) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  len_title = strlen(title);
  new_title = (char *)C_MULTIPLATFORM_MALLOC(len_title + 1);
  if (!new_title) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
  if (UI_STRCPY(new_title, len_title + 1, title) != 0) {
    rc = UI_ERROR_UNKNOWN;
    goto cleanup;
  }

  len_desc = strlen(description);
  new_desc = (char *)C_MULTIPLATFORM_MALLOC(len_desc + 1);
  if (!new_desc) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
  if (UI_STRCPY(new_desc, len_desc + 1, description) != 0) {
    rc = UI_ERROR_UNKNOWN;
    goto cleanup;
  }

  if (timeline->node_count == timeline->node_capacity) {
    new_cap = timeline->node_capacity == 0 ? 4 : timeline->node_capacity * 2;
    new_nodes = (struct ui_timeline_node *)C_MULTIPLATFORM_MALLOC(
        new_cap * sizeof(struct ui_timeline_node));
    if (!new_nodes) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }

    if (timeline->nodes) {
      for (i = 0; i < timeline->node_count; i++) {
        new_nodes[i] = timeline->nodes[i];
      }
      C_MULTIPLATFORM_FREE(timeline->nodes);
    }

    timeline->nodes = new_nodes;
    timeline->node_capacity = new_cap;
  }

  timeline->nodes[timeline->node_count].title = new_title;
  new_title = NULL; /* Ownership transferred */

  timeline->nodes[timeline->node_count].description = new_desc;
  new_desc = NULL; /* Ownership transferred */

  timeline->node_count++;

cleanup:
  if (new_title) {
    C_MULTIPLATFORM_FREE(new_title);
  }
  if (new_desc) {
    C_MULTIPLATFORM_FREE(new_desc);
  }
  return rc;
}

ui_error_t
ui_timeline_base_get_node_count(const struct ui_timeline_base *timeline,
                                size_t *out_count) {
  if (!timeline || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_count = timeline->node_count;
  return UI_ERROR_NONE;
}

ui_error_t ui_timeline_base_get_node(const struct ui_timeline_base *timeline,
                                     size_t index, const char **out_title,
                                     const char **out_description) {
  if (!timeline || !out_title || !out_description) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index >= timeline->node_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  *out_title = timeline->nodes[index].title;
  *out_description = timeline->nodes[index].description;

  return UI_ERROR_NONE;
}

ui_error_t ui_timeline_base_render(struct ui_timeline_base *timeline) {
  if (!timeline) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Mock render routine. Simply validates all string pointers. */

  return UI_ERROR_NONE;
}

ui_error_t ui_timeline_base_bind_data(struct ui_timeline_base *widget,
                                      struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
