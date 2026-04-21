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

int cmp_dnd_create(cmp_dnd_t **out_dnd) {
  int rc = CMP_SUCCESS;
  cmp_dnd_t *dnd = NULL;

  if (!out_dnd) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dnd_create: Invalid argument (out_dnd=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_dnd_t), (void **)&dnd);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_dnd_create: Out of memory\n");
    return rc;
  }

  memset(dnd, 0, sizeof(cmp_dnd_t));
  dnd->operation = CMP_DND_OP_NONE;
  *out_dnd = dnd;
  return rc;
}

int cmp_dnd_destroy(cmp_dnd_t *dnd_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_dnd *dnd = (struct cmp_dnd *)dnd_opaque;
  size_t i;

  if (!dnd) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dnd_destroy: Invalid argument (dnd=NULL)\n");
    return rc;
  }

  if (dnd->payloads) {
    for (i = 0; i < dnd->payload_count; i++) {
      if (dnd->payloads[i]) {
        CMP_FREE(dnd->payloads[i]);
      }
    }
    CMP_FREE(dnd->payloads);
  }
  CMP_FREE(dnd);
  return rc;
}

int cmp_dnd_set_payload_text(cmp_dnd_t *dnd_opaque, const char *text) {
  int rc = CMP_SUCCESS;
  struct cmp_dnd *dnd = (struct cmp_dnd *)dnd_opaque;
  size_t len;
  size_t i;

  if (!dnd || !text) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dnd_set_payload_text: Invalid argument\n");
    return rc;
  }

  if (dnd->payloads) {
    for (i = 0; i < dnd->payload_count; i++) {
      if (dnd->payloads[i]) {
        CMP_FREE(dnd->payloads[i]);
      }
    }
    dnd->payload_count = 0;
  } else {
    dnd->payload_capacity = 4;
    rc = CMP_MALLOC(dnd->payload_capacity * sizeof(char *),
                    (void **)&dnd->payloads);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_dnd_set_payload_text: Out of memory allocating "
                "payloads\n");
      return rc;
    }
  }

  len = strlen(text);
  rc = CMP_MALLOC(len + 1, (void **)&dnd->payloads[0]);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_dnd_set_payload_text: Out of memory allocating string\n");
    return rc;
  }

#if defined(_MSC_VER)
  strcpy_s(dnd->payloads[0], len + 1, text);
#else
  strcpy(dnd->payloads[0], text);
#endif
  dnd->payload_count = 1;
  return rc;
}

int cmp_dnd_get_payload_text(const cmp_dnd_t *dnd_opaque, char **out_text) {
  int rc = CMP_SUCCESS;
  const struct cmp_dnd *dnd = (const struct cmp_dnd *)dnd_opaque;
  size_t len;

  if (!dnd || !out_text) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dnd_get_payload_text: Invalid argument\n");
    return rc;
  }

  if (dnd->payload_count == 0 || !dnd->payloads[0]) {
    *out_text = NULL;
    return rc;
  }

  len = strlen(dnd->payloads[0]);
  rc = CMP_MALLOC(len + 1, (void **)out_text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_dnd_get_payload_text: Out of memory\n");
    return rc;
  }

#if defined(_MSC_VER)
  strcpy_s(*out_text, len + 1, dnd->payloads[0]);
#else
  strcpy(*out_text, dnd->payloads[0]);
#endif
  return rc;
}

int cmp_dnd_set_operation(cmp_dnd_t *dnd_opaque, cmp_dnd_op_t op) {
  int rc = CMP_SUCCESS;
  struct cmp_dnd *dnd = (struct cmp_dnd *)dnd_opaque;

  if (!dnd) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dnd_set_operation: Invalid argument (dnd=NULL)\n");
    return rc;
  }

  dnd->operation = op;
  return rc;
}

int cmp_dnd_get_operation(const cmp_dnd_t *dnd_opaque, cmp_dnd_op_t *out_op) {
  int rc = CMP_SUCCESS;
  const struct cmp_dnd *dnd = (const struct cmp_dnd *)dnd_opaque;

  if (!dnd || !out_op) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dnd_get_operation: Invalid argument\n");
    return rc;
  }

  *out_op = dnd->operation;
  return rc;
}

int cmp_dnd_evaluate_lift_animation(float progress, float *out_scale,
                                    float *out_shadow_opacity) {
  int rc = CMP_SUCCESS;

  if (!out_scale || !out_shadow_opacity) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dnd_evaluate_lift_animation: Invalid argument\n");
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

  return rc;
}

int cmp_dnd_add_item_to_stack(cmp_dnd_t *dnd_opaque,
                              const char *additional_payload_text) {
  int rc = CMP_SUCCESS;
  struct cmp_dnd *dnd = (struct cmp_dnd *)dnd_opaque;
  size_t len;
  size_t new_cap;
  char **new_payloads = NULL;

  if (!dnd || !additional_payload_text) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dnd_add_item_to_stack: Invalid argument\n");
    return rc;
  }

  if (!dnd->payloads || dnd->payload_count >= dnd->payload_capacity) {
    new_cap = dnd->payload_capacity == 0 ? 4 : dnd->payload_capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(char *), (void **)&new_payloads);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_dnd_add_item_to_stack: Out of memory allocating "
                "payloads\n");
      return rc;
    }
    if (dnd->payloads) {
      memcpy(new_payloads, dnd->payloads, dnd->payload_count * sizeof(char *));
      CMP_FREE(dnd->payloads);
    }
    dnd->payloads = new_payloads;
    dnd->payload_capacity = new_cap;
  }

  len = strlen(additional_payload_text);
  rc = CMP_MALLOC(len + 1, (void **)&dnd->payloads[dnd->payload_count]);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_dnd_add_item_to_stack: Out of memory allocating "
              "string\n");
    return rc;
  }

#if defined(_MSC_VER)
  strcpy_s(dnd->payloads[dnd->payload_count], len + 1, additional_payload_text);
#else
  strcpy(dnd->payloads[dnd->payload_count], additional_payload_text);
#endif

  dnd->payload_count++;

  return rc;
}

int cmp_dnd_get_stack_count(const cmp_dnd_t *dnd_opaque, size_t *out_count) {
  int rc = CMP_SUCCESS;
  const struct cmp_dnd *dnd = (const struct cmp_dnd *)dnd_opaque;

  if (!dnd || !out_count) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dnd_get_stack_count: Invalid argument\n");
    return rc;
  }

  *out_count = dnd->payload_count;
  return rc;
}

int cmp_dnd_evaluate_drop_target_highlight(int is_hovered,
                                           float *out_expansion_scale,
                                           float *out_bg_overlay_opacity) {
  int rc = CMP_SUCCESS;

  if (!out_expansion_scale || !out_bg_overlay_opacity) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_dnd_evaluate_drop_target_highlight: Invalid argument\n");
    return rc;
  }

  if (is_hovered) {
    *out_expansion_scale = 1.02f;    /* Slight pop */
    *out_bg_overlay_opacity = 0.15f; /* Darken/highlight bg */
  } else {
    *out_expansion_scale = 1.0f;
    *out_bg_overlay_opacity = 0.0f;
  }

  return rc;
}

int cmp_dnd_evaluate_spring_loading(float hover_duration_ms,
                                    int *out_should_trigger) {
  int rc = CMP_SUCCESS;

  if (!out_should_trigger) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_dnd_evaluate_spring_loading: Invalid argument\n");
    return rc;
  }

  /* HIG: Typical spring loading threshold is ~1000ms */
  if (hover_duration_ms >= 1000.0f) {
    *out_should_trigger = 1;
  } else {
    *out_should_trigger = 0;
  }
  return rc;
}
