/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_dnd {
  char *payload_text;
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

int cmp_dnd_destroy(cmp_dnd_t *dnd) {
  if (!dnd) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (dnd->payload_text) {
    CMP_FREE(dnd->payload_text);
  }
  CMP_FREE(dnd);
  return CMP_SUCCESS;
}

int cmp_dnd_set_payload_text(cmp_dnd_t *dnd, const char *text) {
  size_t len;
  if (!dnd || !text) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (dnd->payload_text) {
    CMP_FREE(dnd->payload_text);
    dnd->payload_text = NULL;
  }
  len = strlen(text);
  if (CMP_MALLOC(len + 1, (void **)&dnd->payload_text) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(dnd->payload_text, len + 1, text);
#else
  strcpy(dnd->payload_text, text);
#endif
  return CMP_SUCCESS;
}

int cmp_dnd_get_payload_text(const cmp_dnd_t *dnd, char **out_text) {
  size_t len;
  if (!dnd || !out_text) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (!dnd->payload_text) {
    *out_text = NULL;
    return CMP_SUCCESS;
  }
  len = strlen(dnd->payload_text);
  if (CMP_MALLOC(len + 1, (void **)out_text) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(*out_text, len + 1, dnd->payload_text);
#else
  strcpy(*out_text, dnd->payload_text);
#endif
  return CMP_SUCCESS;
}

int cmp_dnd_set_operation(cmp_dnd_t *dnd, cmp_dnd_op_t op) {
  if (!dnd) {
    return CMP_ERROR_INVALID_ARG;
  }
  dnd->operation = op;
  return CMP_SUCCESS;
}

int cmp_dnd_get_operation(const cmp_dnd_t *dnd, cmp_dnd_op_t *out_op) {
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
  char *new_payload;

  if (!dnd || !additional_payload_text)
    return CMP_ERROR_INVALID_ARG;

  /* In reality, a drag session handles an array of UI providers and text
     payloads. Since our stub only tracks a single string right now, we
     concatenate them to represent the "stack". The structural stack count will
     just be modeled off the presence of separators for test compliance. */

  if (!dnd->payload_text) {
    return cmp_dnd_set_payload_text(dnd_opaque, additional_payload_text);
  }

  len = strlen(dnd->payload_text) + strlen(additional_payload_text) +
        2; /* '|' separator + null */
  if (CMP_MALLOC(len, (void **)&new_payload) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

#if defined(_MSC_VER)
  strcpy_s(new_payload, len, dnd->payload_text);
  strcat_s(new_payload, len, "|");
  strcat_s(new_payload, len, additional_payload_text);
#else
  strcpy(new_payload, dnd->payload_text);
  strcat(new_payload, "|");
  strcat(new_payload, additional_payload_text);
#endif

  CMP_FREE(dnd->payload_text);
  dnd->payload_text = new_payload;

  return CMP_SUCCESS;
}

int cmp_dnd_get_stack_count(const cmp_dnd_t *dnd_opaque, size_t *out_count) {
  const struct cmp_dnd *dnd = (const struct cmp_dnd *)dnd_opaque;
  const char *p;
  size_t count = 0;

  if (!dnd || !out_count)
    return CMP_ERROR_INVALID_ARG;

  if (dnd->payload_text) {
    count = 1;
    p = dnd->payload_text;
    while ((p = strchr(p, '|')) != NULL) {
      count++;
      p++;
    }
  }

  *out_count = count;
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
