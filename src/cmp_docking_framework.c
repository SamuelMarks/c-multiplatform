/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_docking_framework {
  cmp_tool_panel_t **panels;
  size_t count;
  size_t capacity;
};

int cmp_docking_framework_create(cmp_docking_framework_t **out_docking) {
  cmp_docking_framework_t *dock;

  if (!out_docking) {
    return CMP_ERROR_INVALID_ARG;
  }

  dock = (cmp_docking_framework_t *)malloc(sizeof(cmp_docking_framework_t));
  if (!dock) {
    return CMP_ERROR_OOM;
  }

  dock->capacity = 16;
  dock->count = 0;
  dock->panels =
      (cmp_tool_panel_t **)malloc(dock->capacity * sizeof(cmp_tool_panel_t *));
  if (!dock->panels) {
    free(dock);
    return CMP_ERROR_OOM;
  }

  *out_docking = dock;
  return CMP_SUCCESS;
}

int cmp_docking_framework_destroy(cmp_docking_framework_t *docking) {
  size_t i;
  if (!docking) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < docking->count; i++) {
    free(docking->panels[i]);
  }
  free(docking->panels);
  free(docking);

  return CMP_SUCCESS;
}

int cmp_docking_framework_register_panel(cmp_docking_framework_t *docking,
                                         const char *id, const char *title,
                                         cmp_pane_type_t default_pane) {
  cmp_tool_panel_t *panel;
  cmp_tool_panel_t **new_array;
  size_t i;

  if (!docking || !id || !title) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < docking->count; i++) {
    if (strcmp(docking->panels[i]->id, id) == 0) {
      /* Already registered */
      return CMP_SUCCESS;
    }
  }

  if (docking->count == docking->capacity) {
    docking->capacity *= 2;
    new_array = (cmp_tool_panel_t **)realloc(
        docking->panels, docking->capacity * sizeof(cmp_tool_panel_t *));
    if (!new_array) {
      return CMP_ERROR_OOM;
    }
    docking->panels = new_array;
  }

  panel = (cmp_tool_panel_t *)malloc(sizeof(cmp_tool_panel_t));
  if (!panel) {
    return CMP_ERROR_OOM;
  }

  strncpy(panel->id, id, sizeof(panel->id) - 1);
  panel->id[sizeof(panel->id) - 1] = '\0';

  strncpy(panel->title, title, sizeof(panel->title) - 1);
  panel->title[sizeof(panel->title) - 1] = '\0';

  panel->state = CMP_PANEL_STATE_DOCKED;
  panel->floating_x = 0.0f;
  panel->floating_y = 0.0f;
  panel->width = 250.0f;
  panel->height = 300.0f;
  panel->docked_pane = default_pane;

  docking->panels[docking->count++] = panel;
  return CMP_SUCCESS;
}

static cmp_tool_panel_t *find_panel(const cmp_docking_framework_t *docking,
                                    const char *id) {
  size_t i;
  for (i = 0; i < docking->count; i++) {
    if (strcmp(docking->panels[i]->id, id) == 0) {
      return docking->panels[i];
    }
  }
  return NULL;
}

int cmp_docking_framework_float_panel(cmp_docking_framework_t *docking,
                                      const char *id, float x, float y) {
  cmp_tool_panel_t *panel;

  if (!docking || !id) {
    return CMP_ERROR_INVALID_ARG;
  }

  panel = find_panel(docking, id);
  if (!panel) {
    return CMP_ERROR_NOT_FOUND;
  }

  panel->state = CMP_PANEL_STATE_FLOATING;
  panel->floating_x = x;
  panel->floating_y = y;

  return CMP_SUCCESS;
}

int cmp_docking_framework_dock_panel(cmp_docking_framework_t *docking,
                                     const char *id, cmp_pane_type_t pane) {
  cmp_tool_panel_t *panel;

  if (!docking || !id) {
    return CMP_ERROR_INVALID_ARG;
  }

  panel = find_panel(docking, id);
  if (!panel) {
    return CMP_ERROR_NOT_FOUND;
  }

  panel->state = CMP_PANEL_STATE_DOCKED;
  panel->docked_pane = pane;

  return CMP_SUCCESS;
}

int cmp_docking_framework_get_panel(const cmp_docking_framework_t *docking,
                                    const char *id,
                                    cmp_tool_panel_t **out_panel) {
  cmp_tool_panel_t *panel;

  if (!docking || !id || !out_panel) {
    return CMP_ERROR_INVALID_ARG;
  }

  panel = find_panel(docking, id);
  if (!panel) {
    return CMP_ERROR_NOT_FOUND;
  }

  *out_panel = panel;
  return CMP_SUCCESS;
}
