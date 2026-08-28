/**
 * @file ui_dockable_layout_base.c
 * @brief ui_dockable_layout_base.c implementation.
 */
/* clang-format off */
#include "ui_dockable_layout_base.h"
#include "ui_component.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

/** @brief internal */
#define MAX_PANELS 128

/**
 * @struct layout_node
 * @struct layout_node
 * @brief Internal node representation for a docked panel.
 */
struct layout_node {
  int panel_id;           /**< ID of the panel */
  int target_panel_id;    /**< ID of the target panel it is docked to */
  enum ui_dock_edge edge; /**< Edge it is docked to */
  int in_use;             /**< 1 if node is in use */
};

/**
 * @struct ui_dockable_layout_base
 * @struct ui_dockable_layout_base
 * @brief Internal representation of a dockable layout.
 */
struct ui_dockable_layout_base {
  struct ui_component *component;        /**< The base component */
  struct ui_drag_drop_context *drag_ctx; /**< Drag and drop context */

  struct layout_node nodes[MAX_PANELS]; /**< Array of layout nodes */
};

/**
 * @brief ui_dockable_layout_base_create.
 * @param out_layout Parameter out_layout.
 * @return Return value.
 */
ui_error_t
ui_dockable_layout_base_create(struct ui_dockable_layout_base **out_layout) {
  struct ui_dockable_layout_base *layout;
  ui_error_t rc;
  int i;

  if (!out_layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  layout = (struct ui_dockable_layout_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_dockable_layout_base));
  if (!layout) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&layout->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(layout);
    return rc;
  }

  layout->drag_ctx = NULL;

  for (i = 0; i < MAX_PANELS; i++) {
    layout->nodes[i].in_use = 0;
  }

  *out_layout = layout;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_dockable_layout_base_destroy.
 * @param layout Parameter layout.
 * @return Return value.
 */
ui_error_t
ui_dockable_layout_base_destroy(struct ui_dockable_layout_base *layout) {
  if (!layout) {
    return UI_ERROR_NONE;
  }
  (void)ui_component_destroy(layout->component);
  C_MULTIPLATFORM_FREE(layout);
  return UI_ERROR_NONE;
}

/**
 * @brief ui_dockable_layout_base_get_component.
 * @param layout Parameter layout.
 * @param out_component Parameter out_component.
 * @return Return value.
 */
ui_error_t
ui_dockable_layout_base_get_component(struct ui_dockable_layout_base *layout,
                                      struct ui_component **out_component) {
  if (!layout || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = layout->component;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_dockable_layout_base_dock_panel.
 * @param layout Parameter layout.
 * @param panel_id Parameter panel_id.
 * @param target_panel_id Parameter target_panel_id.
 * @param edge Parameter edge.
 * @return Return value.
 */
ui_error_t
ui_dockable_layout_base_dock_panel(struct ui_dockable_layout_base *layout,
                                   int panel_id, int target_panel_id,
                                   enum ui_dock_edge edge) {
  int i;
  int free_idx = -1;

  if (!layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Find existing or free slot */
  for (i = 0; i < MAX_PANELS; i++) {
    if (layout->nodes[i].in_use && layout->nodes[i].panel_id == panel_id) {
      free_idx = i;
      break;
    }
    if (!layout->nodes[i].in_use && free_idx == -1) {
      free_idx = i;
    }
  }

  if (free_idx == -1) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  layout->nodes[free_idx].panel_id = panel_id;
  layout->nodes[free_idx].target_panel_id = target_panel_id;
  layout->nodes[free_idx].edge = edge;
  layout->nodes[free_idx].in_use = 1;

  return UI_ERROR_NONE;
}

/**
 * @brief ui_dockable_layout_base_remove_panel.
 * @param layout Parameter layout.
 * @param panel_id Parameter panel_id.
 * @return Return value.
 */
ui_error_t
ui_dockable_layout_base_remove_panel(struct ui_dockable_layout_base *layout,
                                     int panel_id) {
  int i;
  if (!layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < MAX_PANELS; i++) {
    if (layout->nodes[i].in_use && layout->nodes[i].panel_id == panel_id) {
      layout->nodes[i].in_use = 0;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_dockable_layout_base_serialize.
 * @param layout Parameter layout.
 * @param out_buffer Parameter out_buffer.
 * @param buffer_size Parameter buffer_size.
 * @return Return value.
 */
ui_error_t
ui_dockable_layout_base_serialize(struct ui_dockable_layout_base *layout,
                                  char *out_buffer, size_t buffer_size) {
  int i;
  int offset = 0;

  if (!layout || !out_buffer || buffer_size == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_buffer[0] = '\0';

  for (i = 0; i < MAX_PANELS; i++) {
    if (layout->nodes[i].in_use) {
      char tmp[64];
      size_t len;
#if defined(_MSC_VER)
      sprintf_s(tmp, sizeof(tmp), "P:%d,T:%d,E:%d;", layout->nodes[i].panel_id,
                layout->nodes[i].target_panel_id, (int)layout->nodes[i].edge);
#else
      sprintf(tmp, "P:%d,T:%d,E:%d;", layout->nodes[i].panel_id,
              layout->nodes[i].target_panel_id, (int)layout->nodes[i].edge);
#endif
      len = strlen(tmp);
      if ((size_t)offset + len < buffer_size) {
        memcpy(&out_buffer[offset], tmp, len);
        offset += (int)len;
        out_buffer[offset] = '\0';
      }
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_dockable_layout_base_deserialize.
 * @param layout Parameter layout.
 * @param buffer Parameter buffer.
 * @return Return value.
 */
ui_error_t
ui_dockable_layout_base_deserialize(struct ui_dockable_layout_base *layout,
                                    const char *buffer) {
  int i;
  if (!layout || !buffer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < MAX_PANELS; i++) {
    layout->nodes[i].in_use = 0;
  }

  /* Basic mock deserialization for testing. A real one would split by ';' and
   * parse */
  {
    int p, t, e;
    const char *ptr = buffer;
    while (*ptr) {
      const char *next_semi;
#if defined(_MSC_VER)
      if (sscanf_s(ptr, "P:%d,T:%d,E:%d;", &p, &t, &e) == 3) {
#else
      if (sscanf(ptr, "P:%d,T:%d,E:%d;", &p, &t, &e) == 3) {
#endif
        ui_error_t dock_rc = ui_dockable_layout_base_dock_panel(
            layout, p, t, (enum ui_dock_edge)e);
        if (dock_rc != UI_ERROR_NONE) {
          return dock_rc;
        }
      }
      next_semi = strchr(ptr, ';');
      if (next_semi) {
        ptr = next_semi + 1;
      } else {
        break;
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_dockable_layout_base_integrate_drag_drop.
 * @param layout Parameter layout.
 * @param drag_ctx Parameter drag_ctx.
 * @return Return value.
 */
ui_error_t ui_dockable_layout_base_integrate_drag_drop(
    struct ui_dockable_layout_base *layout,
    struct ui_drag_drop_context *drag_ctx) {
  if (!layout || !drag_ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  layout->drag_ctx = drag_ctx;
  return UI_ERROR_NONE;
}
