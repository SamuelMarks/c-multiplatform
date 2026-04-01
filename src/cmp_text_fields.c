/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_text_field {
  cmp_keyboard_type_t keyboard_type;
  cmp_return_key_type_t return_key_type;
  int is_secure;
  cmp_auto_capitalization_t auto_cap;
  int spellcheck_enabled;
  cmp_ui_node_t *accessory_node; /* ref to custom toolbar node */
};

struct cmp_rich_text_view {
  cmp_data_detector_types_t detectors;
};

int cmp_text_field_create(cmp_text_field_t **out_field) {
  struct cmp_text_field *ctx;
  if (!out_field)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_text_field), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->keyboard_type = CMP_KEYBOARD_TYPE_DEFAULT;
  ctx->return_key_type = CMP_RETURN_KEY_DEFAULT;
  ctx->is_secure = 0;
  ctx->auto_cap = CMP_AUTO_CAPITALIZATION_SENTENCES; /* Typical OS default */
  ctx->spellcheck_enabled = 1;
  ctx->accessory_node = NULL;

  *out_field = (cmp_text_field_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_text_field_destroy(cmp_text_field_t *field_opaque) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx)
    return CMP_SUCCESS;

  /* Accessory node lifecycle is managed by layout engine */
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_text_field_set_keyboard_type(cmp_text_field_t *field_opaque,
                                     cmp_keyboard_type_t type) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->keyboard_type = type;
  return CMP_SUCCESS;
}

int cmp_text_field_set_return_key_type(cmp_text_field_t *field_opaque,
                                       cmp_return_key_type_t type) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->return_key_type = type;
  return CMP_SUCCESS;
}

int cmp_text_field_set_secure_text_entry(cmp_text_field_t *field_opaque,
                                         int secure) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->is_secure = secure;
  if (secure) {
    /* HIG overrides when secure is active */
    ctx->auto_cap = CMP_AUTO_CAPITALIZATION_NONE;
    ctx->spellcheck_enabled = 0;
  }

  return CMP_SUCCESS;
}

int cmp_text_field_set_auto_capitalization(cmp_text_field_t *field_opaque,
                                           cmp_auto_capitalization_t cap) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->auto_cap = cap;
  return CMP_SUCCESS;
}

int cmp_text_field_set_spellcheck_enabled(cmp_text_field_t *field_opaque,
                                          int enabled) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->spellcheck_enabled = enabled;
  return CMP_SUCCESS;
}

int cmp_text_field_set_input_accessory_view(cmp_text_field_t *field_opaque,
                                            cmp_ui_node_t *accessory_node) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->accessory_node = accessory_node;
  return CMP_SUCCESS;
}

int cmp_rich_text_view_create(cmp_rich_text_view_t **out_view) {
  struct cmp_rich_text_view *ctx;
  if (!out_view)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_rich_text_view), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->detectors = CMP_DATA_DETECTOR_NONE;

  *out_view = (cmp_rich_text_view_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_rich_text_view_destroy(cmp_rich_text_view_t *view_opaque) {
  if (view_opaque)
    CMP_FREE(view_opaque);
  return CMP_SUCCESS;
}

int cmp_rich_text_view_set_data_detectors(cmp_rich_text_view_t *view_opaque,
                                          cmp_data_detector_types_t flags) {
  struct cmp_rich_text_view *ctx = (struct cmp_rich_text_view *)view_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->detectors = flags;
  return CMP_SUCCESS;
}
