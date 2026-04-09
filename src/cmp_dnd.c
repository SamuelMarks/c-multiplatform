/* clang-format off */
#include "cmp.h"

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
  cmp_dnd_t *dnd;
  if (!out_dnd) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_dnd_t), (void **)&dnd) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
  memset(dnd, 0, sizeof(cmp_dnd_t));
  dnd->operation = CMP_DND_OP_NONE;
  *out_dnd = dnd;
  return CMP_SUCCESS;
}

int cmp_dnd_destroy(cmp_dnd_t *dnd_opaque) {
  struct cmp_dnd *dnd = (struct cmp_dnd *)dnd_opaque;
  size_t i;
  if (!dnd) {
    return CMP_ERROR_INVALID_ARG;
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
  return CMP_SUCCESS;
}

int cmp_dnd_set_payload_text(cmp_dnd_t *dnd_opaque, const char *text) {
  struct cmp_dnd *dnd = (struct cmp_dnd *)dnd_opaque;
  size_t len;
  size_t i;
  if (!dnd || !text) {
    return CMP_ERROR_INVALID_ARG;
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
    if (CMP_MALLOC(dnd->payload_capacity * sizeof(char *),
                   (void **)&dnd->payloads) != CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }
  }

  len = strlen(text);
  if (CMP_MALLOC(len + 1, (void **)&dnd->payloads[0]) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(dnd->payloads[0], len + 1, text);
#else
  strcpy(dnd->payloads[0], text);
#endif
  dnd->payload_count = 1;
  return CMP_SUCCESS;
}

int cmp_dnd_get_payload_text(const cmp_dnd_t *dnd_opaque, char **out_text) {
  const struct cmp_dnd *dnd = (const struct cmp_dnd *)dnd_opaque;
  size_t len;
  if (!dnd || !out_text) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (dnd->payload_count == 0 || !dnd->payloads[0]) {
    *out_text = NULL;
    return CMP_SUCCESS;
  }
  len = strlen(dnd->payloads[0]);
  if (CMP_MALLOC(len + 1, (void **)out_text) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(*out_text, len + 1, dnd->payloads[0]);
#else
  strcpy(*out_text, dnd->payloads[0]);
#endif
  return CMP_SUCCESS;
}

int cmp_dnd_set_operation(cmp_dnd_t *dnd_opaque, cmp_dnd_op_t op) {
  struct cmp_dnd *dnd = (struct cmp_dnd *)dnd_opaque;
  if (!dnd) {
    return CMP_ERROR_INVALID_ARG;
  }
  dnd->operation = op;
  return CMP_SUCCESS;
}

int cmp_dnd_get_operation(const cmp_dnd_t *dnd_opaque, cmp_dnd_op_t *out_op) {
  const struct cmp_dnd *dnd = (const struct cmp_dnd *)dnd_opaque;
  if (!dnd || !out_op) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_op = dnd->operation;
  return CMP_SUCCESS;
}

int cmp_dnd_evaluate_lift_animation(float progress, float *out_scale,
                                    float *out_shadow_opacity) {
  if (!out_scale || !out_shadow_opacity)
    return CMP_ERROR_INVALID_ARG;

  if (progress < 0.0f)
    progress = 0.0f;
  if (progress > 1.0f)
    progress = 1.0f;

  /* HIG: Visually lift, scale up to 105% */
  *out_scale = 1.0f + (0.05f * progress);
  /* Deep diffuse shadow drops in */
  *out_shadow_opacity = 0.4f * progress;

  return CMP_SUCCESS;
}

int cmp_dnd_add_item_to_stack(cmp_dnd_t *dnd_opaque,
                              const char *additional_payload_text) {
  struct cmp_dnd *dnd = (struct cmp_dnd *)dnd_opaque;
  size_t len;

  if (!dnd || !additional_payload_text)
    return CMP_ERROR_INVALID_ARG;

  if (!dnd->payloads || dnd->payload_count >= dnd->payload_capacity) {
    size_t new_cap = dnd->payload_capacity == 0 ? 4 : dnd->payload_capacity * 2;
    char **new_payloads;
    if (CMP_MALLOC(new_cap * sizeof(char *), (void **)&new_payloads) !=
        CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }
    if (dnd->payloads) {
      memcpy(new_payloads, dnd->payloads, dnd->payload_count * sizeof(char *));
      CMP_FREE(dnd->payloads);
    }
    dnd->payloads = new_payloads;
    dnd->payload_capacity = new_cap;
  }

  len = strlen(additional_payload_text);
  if (CMP_MALLOC(len + 1, (void **)&dnd->payloads[dnd->payload_count]) !=
      CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  strcpy_s(dnd->payloads[dnd->payload_count], len + 1, additional_payload_text);
#else
  strcpy(dnd->payloads[dnd->payload_count], additional_payload_text);
#endif

  dnd->payload_count++;

  return CMP_SUCCESS;
}

int cmp_dnd_get_stack_count(const cmp_dnd_t *dnd_opaque, size_t *out_count) {
  const struct cmp_dnd *dnd = (const struct cmp_dnd *)dnd_opaque;

  if (!dnd || !out_count)
    return CMP_ERROR_INVALID_ARG;

  *out_count = dnd->payload_count;
  return CMP_SUCCESS;
}

int cmp_dnd_evaluate_drop_target_highlight(int is_hovered,
                                           float *out_expansion_scale,
                                           float *out_bg_overlay_opacity) {
  if (!out_expansion_scale || !out_bg_overlay_opacity)
    return CMP_ERROR_INVALID_ARG;

  if (is_hovered) {
    *out_expansion_scale = 1.02f;    /* Slight pop */
    *out_bg_overlay_opacity = 0.15f; /* Darken/highlight bg */
  } else {
    *out_expansion_scale = 1.0f;
    *out_bg_overlay_opacity = 0.0f;
  }

  return CMP_SUCCESS;
}

int cmp_dnd_evaluate_spring_loading(float hover_duration_ms,
                                    int *out_should_trigger) {
  if (!out_should_trigger)
    return CMP_ERROR_INVALID_ARG;

  /* HIG: Typical spring loading threshold is ~1000ms */
  if (hover_duration_ms >= 1000.0f) {
    *out_should_trigger = 1;
  } else {
    *out_should_trigger = 0;
  }
  return CMP_SUCCESS;
}
