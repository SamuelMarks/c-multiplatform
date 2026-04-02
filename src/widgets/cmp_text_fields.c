/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_text_field {
  cmp_keyboard_type_t keyboard_type;
  cmp_return_key_type_t return_key_type;
  int is_secure;
  cmp_auto_capitalization_t auto_cap;
  int spellcheck_enabled;
  cmp_ui_node_t *accessory_node; /* ref to custom toolbar node */

  /* Text Editing State */
  char *text_buffer;
  size_t text_capacity;
  size_t text_length;
  size_t caret_position;
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

  /* Initialize empty text buffer */
  ctx->text_capacity = 32;
  if (CMP_MALLOC(ctx->text_capacity, (void **)&ctx->text_buffer) !=
      CMP_SUCCESS) {
    CMP_FREE(ctx);
    return CMP_ERROR_OOM;
  }
  ctx->text_buffer[0] = '\0';
  ctx->text_length = 0;
  ctx->caret_position = 0;

  *out_field = (cmp_text_field_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_text_field_destroy(cmp_text_field_t *field_opaque) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx)
    return CMP_SUCCESS;

  if (ctx->text_buffer) {
    CMP_FREE(ctx->text_buffer);
  }

  /* Accessory node lifecycle is managed by layout engine */
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_text_field_insert_text(cmp_text_field_t *field_opaque,
                               const char *text) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  size_t insert_len;
  size_t new_len;
  if (!ctx || !text)
    return CMP_ERROR_INVALID_ARG;

  insert_len = strlen(text);
  if (insert_len == 0)
    return CMP_SUCCESS;

  new_len = ctx->text_length + insert_len;
  if (new_len + 1 > ctx->text_capacity) {
    size_t new_cap = ctx->text_capacity * 2;
    char *new_buf;
    if (new_cap < new_len + 1)
      new_cap = new_len + 1;
    if (CMP_MALLOC(new_cap, (void **)&new_buf) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
    memcpy(new_buf, ctx->text_buffer, ctx->text_length + 1);
    CMP_FREE(ctx->text_buffer);
    ctx->text_buffer = new_buf;
    ctx->text_capacity = new_cap;
  }

  /* Shift text forward to make room for insertion */
  memmove(ctx->text_buffer + ctx->caret_position + insert_len,
          ctx->text_buffer + ctx->caret_position,
          ctx->text_length - ctx->caret_position + 1);

  /* Copy new text */
  memcpy(ctx->text_buffer + ctx->caret_position, text, insert_len);

  ctx->text_length = new_len;
  ctx->caret_position += insert_len;

  return CMP_SUCCESS;
}

int cmp_text_field_delete_backward(cmp_text_field_t *field_opaque) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->caret_position > 0) {
    /* Basic 1-byte deletion. Real implementation needs UTF-8 cluster analysis
     */
    size_t del_len = 1;
    ctx->caret_position -= del_len;
    memmove(ctx->text_buffer + ctx->caret_position,
            ctx->text_buffer + ctx->caret_position + del_len,
            ctx->text_length - ctx->caret_position - del_len + 1);
    ctx->text_length -= del_len;
  }
  return CMP_SUCCESS;
}

int cmp_text_field_set_caret_position(cmp_text_field_t *field_opaque,
                                      size_t pos) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  if (pos > ctx->text_length)
    pos = ctx->text_length;
  ctx->caret_position = pos;
  return CMP_SUCCESS;
}

int cmp_text_field_get_caret_position(const cmp_text_field_t *field_opaque,
                                      size_t *out_pos) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx || !out_pos)
    return CMP_ERROR_INVALID_ARG;
  *out_pos = ctx->caret_position;
  return CMP_SUCCESS;
}

int cmp_text_field_get_text(const cmp_text_field_t *field_opaque,
                            const char **out_text) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx || !out_text)
    return CMP_ERROR_INVALID_ARG;
  *out_text = ctx->text_buffer;
  return CMP_SUCCESS;
}

int cmp_text_field_handle_event(cmp_text_field_t *field_opaque,
                                const cmp_event_t *event) {
  struct cmp_text_field *ctx = (struct cmp_text_field *)field_opaque;
  if (!ctx || !event)
    return CMP_ERROR_INVALID_ARG;

  if (event->type == 2 /* KEYBOARD */ && event->action == CMP_ACTION_DOWN) {
    /* Backspace key code assumed 8 for example */
    if (event->source_id == 8) {
      cmp_text_field_delete_backward(field_opaque);
    } else if (event->source_id >= 32 && event->source_id <= 126) {
      char buf[2];
      buf[0] = (char)event->source_id;
      buf[1] = '\0';
      cmp_text_field_insert_text(field_opaque, buf);
    }
  }
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
