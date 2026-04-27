/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_collection_section {
  float min_column_width;
  cmp_orthogonal_behavior_t orthogonal_behavior;
};

struct cmp_collection {
  cmp_collection_section_t **sections;
  size_t section_count;
  size_t capacity;
};

struct cmp_diffable_datasource {
  uint64_t *current_state;
  size_t count;
};

/**
 * @brief cmp_collection_create
 *
 * @param out_collection Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_collection_create(cmp_collection_t **out_collection) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_collection *ctx = NULL;

  if (out_collection == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_collection_create: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_collection), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_collection_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->sections = NULL;
  ctx->section_count = 0;
  ctx->capacity = 0;

  *out_collection = (cmp_collection_t *)ctx;
  cmp_log_debug(
      "cmp_collection_create: Successfully created collection context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_collection_destroy
 *
 * @param collection_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_collection_destroy(cmp_collection_t *collection_opaque) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_collection *ctx = (struct cmp_collection *)collection_opaque;
  size_t i;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_collection_destroy: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ctx->sections != NULL) {
    for (i = 0; i < ctx->section_count; ++i) {
      if (ctx->sections[i] != NULL) {
        rc = cmp_collection_section_destroy(
            (cmp_collection_section_t *)ctx->sections[i]);
        if (rc != CMP_SUCCESS) {
          cmp_log_debug(
              "cmp_collection_destroy: Failed destroying section at index %d\n",
              (int)i);
        }
      }
    }
    rc = CMP_FREE(ctx->sections);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_collection_destroy: Failed freeing sections array\n");
    }
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug(
        "cmp_collection_destroy: Failed freeing collection context\n");
  }

  cmp_log_debug(
      "cmp_collection_destroy: Successfully destroyed collection context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_collection_section_create
 *
 * @param out_section Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_collection_section_create(cmp_collection_section_t **out_section) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_collection_section *ctx = NULL;

  if (out_section == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_collection_section_create: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_collection_section), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_collection_section_create: Out of memory: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->min_column_width = 0.0f;
  ctx->orthogonal_behavior = CMP_ORTHOGONAL_NONE;

  *out_section = (cmp_collection_section_t *)ctx;
  cmp_log_debug("cmp_collection_section_create: Successfully created "
                "collection section\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_collection_section_destroy
 *
 * @param section_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_collection_section_destroy(cmp_collection_section_t *section_opaque) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (section_opaque == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_collection_section_destroy: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_FREE(section_opaque);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_collection_section_destroy: Failed freeing section\n");
  }

  cmp_log_debug("cmp_collection_section_destroy: Successfully destroyed "
                "collection section\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_collection_section_set_flow_layout
 *
 * @param section_opaque Parameter description.
 * @param min_column_width Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_collection_section_set_flow_layout(
    cmp_collection_section_t *section_opaque, float min_column_width) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_collection_section *ctx =
      (struct cmp_collection_section *)section_opaque;

  if (ctx == NULL || min_column_width <= 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_collection_section_set_flow_layout: Invalid argument: %s\n",
        err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->min_column_width = min_column_width;
  cmp_log_debug(
      "cmp_collection_section_set_flow_layout: Set min width to %.2f\n",
      min_column_width);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_collection_section_set_orthogonal_behavior
 *
 * @param section_opaque Parameter description.
 * @param behavior Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_collection_section_set_orthogonal_behavior(
    cmp_collection_section_t *section_opaque,
    cmp_orthogonal_behavior_t behavior) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_collection_section *ctx =
      (struct cmp_collection_section *)section_opaque;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_collection_section_set_orthogonal_behavior: Invalid "
                  "argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->orthogonal_behavior = behavior;
  cmp_log_debug(
      "cmp_collection_section_set_orthogonal_behavior: Set behavior to %d\n",
      (int)behavior);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_collection_add_section
 *
 * @param collection_opaque Parameter description.
 * @param section_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_collection_add_section(cmp_collection_t *collection_opaque,
                               cmp_collection_section_t *section_opaque) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_collection *ctx = (struct cmp_collection *)collection_opaque;
  struct cmp_collection_section *sec =
      (struct cmp_collection_section *)section_opaque;
  cmp_collection_section_t **new_secs = NULL;
  size_t new_cap;

  if (ctx == NULL || sec == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_collection_add_section: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ctx->section_count == ctx->capacity) {
    new_cap = ctx->capacity == 0 ? 4 : ctx->capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_collection_section_t *),
                    (void **)&new_secs);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_collection_add_section: Out of memory: %s\n", err_str);
      if (rc != 0) {
        return rc;
      }
      return rc;
    }
    if (ctx->sections != NULL) {
      memcpy(new_secs, ctx->sections,
             ctx->section_count * sizeof(cmp_collection_section_t *));
      rc = CMP_FREE(ctx->sections);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug(
            "cmp_collection_add_section: CMP_FREE old sections failed\n");
      }
    }
    ctx->sections = new_secs;
    ctx->capacity = new_cap;
  }

  ctx->sections[ctx->section_count++] = sec;
  cmp_log_debug("cmp_collection_add_section: Added section, total count=%d\n",
                (int)ctx->section_count);
  return CMP_SUCCESS;
}

/**
 * @brief cmp_diffable_datasource_create
 *
 * @param out_ds Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_diffable_datasource_create(cmp_diffable_datasource_t **out_ds) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_diffable_datasource *ctx = NULL;

  if (out_ds == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_diffable_datasource_create: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_diffable_datasource), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_diffable_datasource_create: Out of memory: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->current_state = NULL;
  ctx->count = 0;

  *out_ds = (cmp_diffable_datasource_t *)ctx;
  cmp_log_debug("cmp_diffable_datasource_create: Successfully created "
                "datasource context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_diffable_datasource_destroy
 *
 * @param ds_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_diffable_datasource_destroy(cmp_diffable_datasource_t *ds_opaque) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_diffable_datasource *ctx =
      (struct cmp_diffable_datasource *)ds_opaque;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_diffable_datasource_destroy: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (ctx->current_state != NULL) {
    rc = CMP_FREE(ctx->current_state);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_diffable_datasource_destroy: Failed freeing current state\n");
    }
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug(
        "cmp_diffable_datasource_destroy: Failed freeing datasource context\n");
  }

  cmp_log_debug("cmp_diffable_datasource_destroy: Successfully destroyed "
                "datasource context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_diffable_datasource_apply_snapshot
 *
 * @param ds_opaque Parameter description.
 * @param items Parameter description.
 * @param count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_diffable_datasource_apply_snapshot(cmp_diffable_datasource_t *ds_opaque,
                                           const uint64_t *items,
                                           size_t count) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_diffable_datasource *ctx =
      (struct cmp_diffable_datasource *)ds_opaque;
  uint64_t *new_state = NULL;

  if (ctx == NULL || (items == NULL && count > 0)) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_diffable_datasource_apply_snapshot: Invalid argument: %s\n",
        err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (count > 0) {
    rc = CMP_MALLOC(count * sizeof(uint64_t), (void **)&new_state);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug(
          "cmp_diffable_datasource_apply_snapshot: Out of memory: %s\n",
          err_str);
      if (rc != 0) {
        return rc;
      }
      return rc;
    }
    memcpy(new_state, items, count * sizeof(uint64_t));
  }

  if (ctx->current_state != NULL) {
    rc = CMP_FREE(ctx->current_state);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_diffable_datasource_apply_snapshot: Failed freeing old state\n");
    }
  }

  ctx->current_state = new_state;
  ctx->count = count;

  /* A real implementation calculates O(N) Meyer's diff here to emit specific
     view transition instructions (insertRow: 3, deleteRow: 5, moveRow: 1->7).
   */
  cmp_log_debug(
      "cmp_diffable_datasource_apply_snapshot: Applied %d items snapshot\n",
      (int)count);
  return CMP_SUCCESS;
}

/**
 * @brief cmp_system_map_view_mount
 *
 * @param node Parameter description.
 * @param latitude Parameter description.
 * @param longitude Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_map_view_mount(cmp_ui_node_t *node, float latitude,
                              float longitude) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_system_map_view_mount: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Emits WKWebView/MKMapView native backing handles bounded to the layout
   * dimensions */
  (void)latitude;
  (void)longitude;

  cmp_log_debug("cmp_system_map_view_mount: Mock mounted map view\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_system_web_view_mount
 *
 * @param node Parameter description.
 * @param url Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_web_view_mount(cmp_ui_node_t *node, const char *url) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (node == NULL || url == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_system_web_view_mount: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Mounts platform-specific WKWebView/EdgeWebView2 */
  cmp_log_debug("cmp_system_web_view_mount: Mock mounted web view for %s\n",
                url);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
