/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_docking_framework {
  cmp_tool_panel_t **panels;
  size_t count;
  size_t capacity;
};

int cmp_docking_framework_create(cmp_docking_framework_t **out_docking) {
  int rc = CMP_SUCCESS;
  cmp_docking_framework_t *dock = NULL;

  if (!out_docking) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_docking_framework_create: Invalid argument\n");
    return rc;
  }

  dock = (cmp_docking_framework_t *)malloc(sizeof(cmp_docking_framework_t));
  if (!dock) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_docking_framework_create: Out of memory\n");
    return rc;
  }

  dock->capacity = 16;
  dock->count = 0;
  dock->panels =
      (cmp_tool_panel_t **)malloc(dock->capacity * sizeof(cmp_tool_panel_t *));
  if (!dock->panels) {
    free(dock);
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_docking_framework_create: Out of memory allocating "
              "panels\n");
    return rc;
  }

  *out_docking = dock;
  return rc;
}

int cmp_docking_framework_destroy(cmp_docking_framework_t *docking) {
  int rc = CMP_SUCCESS;
  size_t i;

  if (!docking) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_docking_framework_destroy: Invalid argument\n");
    return rc;
  }

  for (i = 0; i < docking->count; i++) {
    free(docking->panels[i]);
  }
  free(docking->panels);
  free(docking);

  return rc;
}

int cmp_docking_framework_register_panel(cmp_docking_framework_t *docking,
                                         const char *id, const char *title,
                                         cmp_pane_type_t default_pane) {
  int rc = CMP_SUCCESS;
  cmp_tool_panel_t *panel = NULL;
  cmp_tool_panel_t **new_array = NULL;
  size_t i;

  if (!docking || !id || !title) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_docking_framework_register_panel: Invalid argument\n");
    return rc;
  }

  for (i = 0; i < docking->count; i++) {
    if (strcmp(docking->panels[i]->id, id) == 0) {
      /* Already registered */
      return rc;
    }
  }

  if (docking->count == docking->capacity) {
    docking->capacity *= 2;
    new_array = (cmp_tool_panel_t **)realloc(
        docking->panels, docking->capacity * sizeof(cmp_tool_panel_t *));
    if (!new_array) {
      rc = CMP_ERROR_OOM;
      LOG_DEBUG("Error in cmp_docking_framework_register_panel: Out of memory "
                "reallocating array\n");
      return rc;
    }
    docking->panels = new_array;
  }

  panel = (cmp_tool_panel_t *)malloc(sizeof(cmp_tool_panel_t));
  if (!panel) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_docking_framework_register_panel: Out of memory "
              "allocating panel\n");
    return rc;
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
  return rc;
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
  int rc = CMP_SUCCESS;
  cmp_tool_panel_t *panel = NULL;

  if (!docking || !id) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_docking_framework_float_panel: Invalid argument\n");
    return rc;
  }

  panel = find_panel(docking, id);
  if (!panel) {
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_docking_framework_float_panel: Panel not found\n");
    return rc;
  }

  panel->state = CMP_PANEL_STATE_FLOATING;
  panel->floating_x = x;
  panel->floating_y = y;

  return rc;
}

int cmp_docking_framework_dock_panel(cmp_docking_framework_t *docking,
                                     const char *id, cmp_pane_type_t pane) {
  int rc = CMP_SUCCESS;
  cmp_tool_panel_t *panel = NULL;

  if (!docking || !id) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_docking_framework_dock_panel: Invalid argument\n");
    return rc;
  }

  panel = find_panel(docking, id);
  if (!panel) {
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_docking_framework_dock_panel: Panel not found\n");
    return rc;
  }

  panel->state = CMP_PANEL_STATE_DOCKED;
  panel->docked_pane = pane;

  return rc;
}

int cmp_docking_framework_get_panel(const cmp_docking_framework_t *docking,
                                    const char *id,
                                    cmp_tool_panel_t **out_panel) {
  int rc = CMP_SUCCESS;
  cmp_tool_panel_t *panel = NULL;

  if (!docking || !id || !out_panel) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_docking_framework_get_panel: Invalid argument\n");
    return rc;
  }

  panel = find_panel(docking, id);
  if (!panel) {
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("Error in cmp_docking_framework_get_panel: Panel not found\n");
    return rc;
  }

  *out_panel = panel;
  return rc;
}
