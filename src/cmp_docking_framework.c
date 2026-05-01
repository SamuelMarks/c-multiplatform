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

/**
 * @brief Creates a new docking framework context.
 *
 * @param out_docking Pointer to store the newly created docking context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_docking_framework_create(cmp_docking_framework_t **out_docking) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_docking_framework_t *dock = NULL;

  if (out_docking == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_docking_framework_create: Invalid argument "
                  "(out_docking=NULL): %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_docking_framework_t), (void **)&dock);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_docking_framework_create: Out of memory: %s\n", err_str);

    return rc;
  }

  dock->capacity = 16;
  dock->count = 0;

  rc = CMP_MALLOC(dock->capacity * sizeof(cmp_tool_panel_t *),
                  (void **)&dock->panels);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_docking_framework_create: Out of memory allocating "
                  "panels: %s\n",
                  err_str);

    rc = CMP_FREE(dock);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_docking_framework_create: CMP_FREE recovery failed\n");
    }
    return CMP_ERROR_OOM;
  }

  *out_docking = dock;
  cmp_log_debug(
      "cmp_docking_framework_create: Successfully created docking framework\n");
  return rc;
}

/**
 * @brief Destroys a docking framework context and frees its resources.
 *
 * @param docking Pointer to the docking framework context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_docking_framework_destroy(cmp_docking_framework_t *docking) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t i;

  if (docking == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_docking_framework_destroy: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  if (docking->panels != NULL) {
    for (i = 0; i < docking->count; i++) {
      if (docking->panels[i] != NULL) {
        rc = CMP_FREE(docking->panels[i]);
        if (rc != CMP_SUCCESS) {
          cmp_log_debug(
              "cmp_docking_framework_destroy: CMP_FREE panel failed\n");
        }
      }
    }
    rc = CMP_FREE(docking->panels);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_docking_framework_destroy: CMP_FREE panels array failed\n");
    }
  }

  rc = CMP_FREE(docking);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_docking_framework_destroy: CMP_FREE context failed\n");
  }

  cmp_log_debug("cmp_docking_framework_destroy: Successfully destroyed docking "
                "framework\n");
  return rc;
}

/**
 * @brief Registers a new tool panel with the docking framework.
 *
 * @param docking Pointer to the docking framework context.
 * @param id Unique identifier string for the panel.
 * @param title The display title for the panel.
 * @param default_pane The default pane to dock the panel into.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_docking_framework_register_panel(cmp_docking_framework_t *docking,
                                         const char *id, const char *title,
                                         cmp_pane_type_t default_pane) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_tool_panel_t *panel = NULL;
  cmp_tool_panel_t **new_array = NULL;
  size_t new_cap;
  size_t i;

  if (docking == NULL || id == NULL || title == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_docking_framework_register_panel: Invalid argument: %s\n",
        err_str);

    return rc;
  }

  for (i = 0; i < docking->count; i++) {
    if (docking->panels[i] != NULL && strcmp(docking->panels[i]->id, id) == 0) {
      /* Already registered */
      cmp_log_debug(
          "cmp_docking_framework_register_panel: Panel already registered\n");
      return rc;
    }
  }

  if (docking->count == docking->capacity) {
    new_cap = docking->capacity == 0 ? 8 : docking->capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_tool_panel_t *), (void **)&new_array);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_docking_framework_register_panel: Out of memory "
                    "reallocating array: %s\n",
                    err_str);

      return rc;
    }
    if (docking->panels != NULL) {
      memcpy(new_array, docking->panels,
             docking->count * sizeof(cmp_tool_panel_t *));
      rc = CMP_FREE(docking->panels);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug("cmp_docking_framework_register_panel: CMP_FREE old "
                      "array failed\n");
      }
    }
    docking->panels = new_array;
    docking->capacity = new_cap;
  }

  rc = CMP_MALLOC(sizeof(cmp_tool_panel_t), (void **)&panel);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_docking_framework_register_panel: Out of memory "
                  "allocating panel: %s\n",
                  err_str);

    return rc;
  }
#if defined(_MSC_VER)
  if (strncpy_s(panel->id, sizeof(panel->id), id, _TRUNCATE) != 0) {
    CMP_FREE(panel);
    cmp_log_debug(
        "cmp_docking_framework_register_panel: strncpy_s failed (id)\n");
    return CMP_ERROR_GENERAL;
  }
  if (strncpy_s(panel->title, sizeof(panel->title), title, _TRUNCATE) != 0) {
    CMP_FREE(panel);
    cmp_log_debug(
        "cmp_docking_framework_register_panel: strncpy_s failed (title)\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strncpy(panel->id, id, sizeof(panel->id) - 1);
  panel->id[sizeof(panel->id) - 1] = '\0';

  strncpy(panel->title, title, sizeof(panel->title) - 1);
  panel->title[sizeof(panel->title) - 1] = '\0';
#endif

  panel->state = CMP_PANEL_STATE_DOCKED;
  panel->floating_x = 0.0f;
  panel->floating_y = 0.0f;
  panel->width = 250.0f;
  panel->height = 300.0f;
  panel->docked_pane = default_pane;

  docking->panels[docking->count++] = panel;
  cmp_log_debug(
      "cmp_docking_framework_register_panel: Registered panel successfully\n");
  return rc;
}

/**
 * @brief Internal helper to locate a registered panel by its ID.
 *
 * @param docking Pointer to the docking framework context.
 * @param id The unique identifier string of the panel.
 * @param out_panel Pointer to store the found panel pointer.
 * @return Returns 0 on success, or an error code on failure.
 */
static int find_panel(const cmp_docking_framework_t *docking, const char *id,
                      cmp_tool_panel_t **out_panel) {
  int rc = CMP_SUCCESS;
  rc = CMP_ERROR_NOT_FOUND;
  size_t i;

  if (out_panel == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_panel = NULL;

  for (i = 0; i < docking->count; i++) {
    if (docking->panels[i] != NULL && strcmp(docking->panels[i]->id, id) == 0) {
      *out_panel = docking->panels[i];
      return CMP_SUCCESS;
    }
  }

  return rc;
}

/**
 * @brief Floats a panel at specific coordinates on the screen.
 *
 * @param docking Pointer to the docking framework context.
 * @param id The unique identifier string of the panel to float.
 * @param x The target X coordinate for the floated panel.
 * @param y The target Y coordinate for the floated panel.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_docking_framework_float_panel(cmp_docking_framework_t *docking,
                                      const char *id, float x, float y) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_tool_panel_t *panel = NULL;

  if (docking == NULL || id == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_docking_framework_float_panel: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = find_panel(docking, id, &panel);
  if (rc != CMP_SUCCESS || panel == NULL) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_docking_framework_float_panel: Panel not found: %s\n",
                  err_str);
    return CMP_ERROR_NOT_FOUND;
  }

  panel->state = CMP_PANEL_STATE_FLOATING;
  panel->floating_x = x;
  panel->floating_y = y;

  cmp_log_debug("cmp_docking_framework_float_panel: Repositioned panel to "
                "floating state\n");
  return rc;
}

/**
 * @brief Docks a panel into a specified pane.
 *
 * @param docking Pointer to the docking framework context.
 * @param id The unique identifier string of the panel to dock.
 * @param pane The target pane to dock the panel into.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_docking_framework_dock_panel(cmp_docking_framework_t *docking,
                                     const char *id, cmp_pane_type_t pane) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_tool_panel_t *panel = NULL;

  if (docking == NULL || id == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_docking_framework_dock_panel: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = find_panel(docking, id, &panel);
  if (rc != CMP_SUCCESS || panel == NULL) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_docking_framework_dock_panel: Panel not found: %s\n",
                  err_str);
    return CMP_ERROR_NOT_FOUND;
  }

  panel->state = CMP_PANEL_STATE_DOCKED;
  panel->docked_pane = pane;

  cmp_log_debug(
      "cmp_docking_framework_dock_panel: Docked panel onto target pane\n");
  return rc;
}

/**
 * @brief Retrieves a pointer to a registered panel by its ID.
 *
 * @param docking Pointer to the docking framework context.
 * @param id The unique identifier string of the panel to retrieve.
 * @param out_panel Pointer to store the retrieved panel pointer.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_docking_framework_get_panel(const cmp_docking_framework_t *docking,
                                    const char *id,
                                    cmp_tool_panel_t **out_panel) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (docking == NULL || id == NULL || out_panel == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_docking_framework_get_panel: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = find_panel(docking, id, out_panel);
  if (rc != CMP_SUCCESS || *out_panel == NULL) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_docking_framework_get_panel: Panel not found: %s\n",
                  err_str);
    return CMP_ERROR_NOT_FOUND;
  }

  cmp_log_debug(
      "cmp_docking_framework_get_panel: Retrieved internal panel reference\n");
  return rc;
}
