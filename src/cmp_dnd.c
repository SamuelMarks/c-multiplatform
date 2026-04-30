/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_dnd {
  char **payloads;
  size_t payload_count;
  size_t payload_capacity;
  cmp_dnd_op_t operation;
};

/**
 * @brief cmp_dnd_create
 *
 * @param out_dnd Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dnd_create(cmp_dnd_t **out_dnd) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_dnd_t *dnd = NULL;

  if (out_dnd == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dnd_create: Invalid argument (out_dnd=NULL): %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_dnd_t), (void **)&dnd);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dnd_create: Out of memory: %s\n", err_str);

    return rc;
  }

  memset(dnd, 0, sizeof(cmp_dnd_t));
  dnd->operation = CMP_DND_OP_NONE;
  *out_dnd = dnd;
  cmp_log_debug("cmp_dnd_create: Successfully created drag/drop context\n");
  return rc;
}

/**
 * @brief cmp_dnd_destroy
 *
 * @param dnd_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dnd_destroy(cmp_dnd_t *dnd_opaque) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_dnd *dnd = (struct cmp_dnd *)dnd_opaque;
  size_t i;

  if (dnd == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dnd_destroy: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (dnd->payloads != NULL) {
    for (i = 0; i < dnd->payload_count; i++) {
      if (dnd->payloads[i] != NULL) {
        rc = CMP_FREE(dnd->payloads[i]);
        if (rc != CMP_SUCCESS) {
          cmp_log_debug("cmp_dnd_destroy: CMP_FREE payload str failed\n");
        }
      }
    }
    rc = CMP_FREE(dnd->payloads);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_dnd_destroy: CMP_FREE payloads array failed\n");
    }
  }

  rc = CMP_FREE(dnd);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_dnd_destroy: CMP_FREE ctx failed\n");
  }

  cmp_log_debug("cmp_dnd_destroy: Successfully destroyed drag/drop context\n");
  return rc;
}

/**
 * @brief cmp_dnd_set_payload_text
 *
 * @param dnd_opaque Parameter description.
 * @param text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dnd_set_payload_text(cmp_dnd_t *dnd_opaque, const char *text) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_dnd *dnd = (struct cmp_dnd *)dnd_opaque;
  size_t len;
  size_t i;

  if (dnd == NULL || text == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dnd_set_payload_text: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (dnd->payloads != NULL) {
    for (i = 0; i < dnd->payload_count; i++) {
      if (dnd->payloads[i] != NULL) {
        rc = CMP_FREE(dnd->payloads[i]);
        if (rc != CMP_SUCCESS) {
          cmp_log_debug(
              "cmp_dnd_set_payload_text: CMP_FREE failed on old payload\n");
        }
      }
    }
    dnd->payload_count = 0;
  } else {
    dnd->payload_capacity = 4;
    rc = CMP_MALLOC(dnd->payload_capacity * sizeof(char *),
                    (void **)&dnd->payloads);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_dnd_set_payload_text: Out of memory allocating "
                    "payloads array: %s\n",
                    err_str);

      return rc;
    }
  }

  len = strlen(text);
  rc = CMP_MALLOC(len + 1, (void **)&dnd->payloads[0]);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_dnd_set_payload_text: Out of memory allocating string: %s\n",
        err_str);

    return rc;
  }
#if defined(_MSC_VER)
  if (strcpy_s(dnd->payloads[0], len + 1, text) != 0) {
    CMP_FREE(dnd->payloads[0]);
    dnd->payloads[0] = NULL;
    cmp_log_debug("cmp_dnd_set_payload_text: strcpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(dnd->payloads[0], text);
#endif
  dnd->payload_count = 1;

  cmp_log_debug("cmp_dnd_set_payload_text: Set primary payload text\n");
  return rc;
}

/**
 * @brief cmp_dnd_get_payload_text
 *
 * @param dnd_opaque Parameter description.
 * @param out_text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dnd_get_payload_text(const cmp_dnd_t *dnd_opaque, char **out_text) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  const struct cmp_dnd *dnd = (const struct cmp_dnd *)dnd_opaque;
  size_t len;

  if (dnd == NULL || out_text == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dnd_get_payload_text: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (dnd->payload_count == 0 || dnd->payloads[0] == NULL) {
    *out_text = NULL;
    return rc;
  }

  len = strlen(dnd->payloads[0]);
  rc = CMP_MALLOC(len + 1, (void **)out_text);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dnd_get_payload_text: Out of memory: %s\n", err_str);

    return rc;
  }
#if defined(_MSC_VER)
  if (strcpy_s(*out_text, len + 1, dnd->payloads[0]) != 0) {
    CMP_FREE(*out_text);
    *out_text = NULL;
    cmp_log_debug("cmp_dnd_get_payload_text: strcpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(*out_text, dnd->payloads[0]);
#endif

  cmp_log_debug("cmp_dnd_get_payload_text: Fetched primary payload text\n");
  return rc;
}

/**
 * @brief cmp_dnd_set_operation
 *
 * @param dnd_opaque Parameter description.
 * @param op Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dnd_set_operation(cmp_dnd_t *dnd_opaque, cmp_dnd_op_t op) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_dnd *dnd = (struct cmp_dnd *)dnd_opaque;

  if (dnd == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dnd_set_operation: Invalid argument: %s\n", err_str);

    return rc;
  }

  dnd->operation = op;
  cmp_log_debug("cmp_dnd_set_operation: Op set to %d\n", (int)op);

  return rc;
}

/**
 * @brief cmp_dnd_get_operation
 *
 * @param dnd_opaque Parameter description.
 * @param out_op Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dnd_get_operation(const cmp_dnd_t *dnd_opaque, cmp_dnd_op_t *out_op) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  const struct cmp_dnd *dnd = (const struct cmp_dnd *)dnd_opaque;

  if (dnd == NULL || out_op == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dnd_get_operation: Invalid argument: %s\n", err_str);

    return rc;
  }

  *out_op = dnd->operation;
  cmp_log_debug("cmp_dnd_get_operation: Read op\n");

  return rc;
}

/**
 * @brief cmp_dnd_evaluate_lift_animation
 *
 * @param progress Parameter description.
 * @param out_scale Parameter description.
 * @param out_shadow_opacity Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dnd_evaluate_lift_animation(float progress, float *out_scale,
                                    float *out_shadow_opacity) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (out_scale == NULL || out_shadow_opacity == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dnd_evaluate_lift_animation: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  if (progress < 0.0f)
    progress = 0.0f;
  if (progress > 1.0f)
    progress = 1.0f;

  /* HIG: Visually lift, scale up to 105% */
  *out_scale = 1.0f + (0.05f * progress);
  /* Deep diffuse shadow drops in */
  *out_shadow_opacity = 0.4f * progress;

  cmp_log_debug("cmp_dnd_evaluate_lift_animation: Evaluated lift curves\n");

  return rc;
}

/**
 * @brief cmp_dnd_add_item_to_stack
 *
 * @param dnd_opaque Parameter description.
 * @param additional_payload_text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dnd_add_item_to_stack(cmp_dnd_t *dnd_opaque,
                              const char *additional_payload_text) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_dnd *dnd = (struct cmp_dnd *)dnd_opaque;
  size_t len;
  size_t new_cap;
  char **new_payloads = NULL;

  if (dnd == NULL || additional_payload_text == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dnd_add_item_to_stack: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (dnd->payloads == NULL || dnd->payload_count >= dnd->payload_capacity) {
    new_cap = dnd->payload_capacity == 0 ? 4 : dnd->payload_capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(char *), (void **)&new_payloads);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_dnd_add_item_to_stack: Out of memory allocating "
                    "payloads array: %s\n",
                    err_str);

      return rc;
    }
    if (dnd->payloads != NULL) {
      memcpy(new_payloads, dnd->payloads, dnd->payload_count * sizeof(char *));
      rc = CMP_FREE(dnd->payloads);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug("cmp_dnd_add_item_to_stack: CMP_FREE old array failed\n");
      }
    }
    dnd->payloads = new_payloads;
    dnd->payload_capacity = new_cap;
  }

  len = strlen(additional_payload_text);
  rc = CMP_MALLOC(len + 1, (void **)&dnd->payloads[dnd->payload_count]);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_dnd_add_item_to_stack: Out of memory allocating string: %s\n",
        err_str);

    return rc;
  }
#if defined(_MSC_VER)
  if (strcpy_s(dnd->payloads[dnd->payload_count], len + 1,
               additional_payload_text) != 0) {
    CMP_FREE(dnd->payloads[dnd->payload_count]);
    cmp_log_debug("cmp_dnd_add_item_to_stack: strcpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(dnd->payloads[dnd->payload_count], additional_payload_text);
#endif

  dnd->payload_count++;
  cmp_log_debug("cmp_dnd_add_item_to_stack: Appended item to drag stack\n");
  return rc;
}

/**
 * @brief cmp_dnd_get_stack_count
 *
 * @param dnd_opaque Parameter description.
 * @param out_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dnd_get_stack_count(const cmp_dnd_t *dnd_opaque, size_t *out_count) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  const struct cmp_dnd *dnd = (const struct cmp_dnd *)dnd_opaque;

  if (dnd == NULL || out_count == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dnd_get_stack_count: Invalid argument: %s\n", err_str);

    return rc;
  }

  *out_count = dnd->payload_count;

  return rc;
}

/**
 * @brief cmp_dnd_evaluate_drop_target_highlight
 *
 * @param is_hovered Parameter description.
 * @param out_expansion_scale Parameter description.
 * @param out_bg_overlay_opacity Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dnd_evaluate_drop_target_highlight(int is_hovered,
                                           float *out_expansion_scale,
                                           float *out_bg_overlay_opacity) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (out_expansion_scale == NULL || out_bg_overlay_opacity == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_dnd_evaluate_drop_target_highlight: Invalid argument: %s\n",
        err_str);

    return rc;
  }

  if (is_hovered) {
    *out_expansion_scale = 1.02f;    /* Slight pop */
    *out_bg_overlay_opacity = 0.15f; /* Darken/highlight bg */
  } else {
    *out_expansion_scale = 1.0f;
    *out_bg_overlay_opacity = 0.0f;
  }

  cmp_log_debug("cmp_dnd_evaluate_drop_target_highlight: Evaluated target "
                "highlight curves\n");

  return rc;
}

/**
 * @brief cmp_dnd_evaluate_spring_loading
 *
 * @param hover_duration_ms Parameter description.
 * @param out_should_trigger Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dnd_evaluate_spring_loading(float hover_duration_ms,
                                    int *out_should_trigger) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (out_should_trigger == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_dnd_evaluate_spring_loading: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  /* HIG: Typical spring loading threshold is ~1000ms */
  if (hover_duration_ms >= 1000.0f) {
    *out_should_trigger = 1;
  } else {
    *out_should_trigger = 0;
  }

  cmp_log_debug(
      "cmp_dnd_evaluate_spring_loading: Evaluated spring-load timing\n");

  return rc;
}
