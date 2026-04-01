/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_editable {
  char *text_buffer;
  size_t capacity;
  size_t length;
};

int cmp_editable_create(cmp_editable_t **out_editable) {
  struct cmp_editable *editable;

  if (!out_editable)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_editable), (void **)&editable) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(editable, 0, sizeof(struct cmp_editable));

  editable->capacity = 256;
  if (CMP_MALLOC(editable->capacity, (void **)&editable->text_buffer) !=
      CMP_SUCCESS) {
    CMP_FREE(editable);
    return CMP_ERROR_OOM;
  }
  editable->text_buffer[0] = '\0';

  *out_editable = (cmp_editable_t *)editable;
  return CMP_SUCCESS;
}

int cmp_editable_destroy(cmp_editable_t *editable) {
  struct cmp_editable *internal_editable = (struct cmp_editable *)editable;
  if (!internal_editable)
    return CMP_ERROR_INVALID_ARG;

  if (internal_editable->text_buffer) {
    CMP_FREE(internal_editable->text_buffer);
  }
  CMP_FREE(internal_editable);
  return CMP_SUCCESS;
}

int cmp_editable_insert_text(cmp_editable_t *editable, const char *text) {
  struct cmp_editable *internal_editable = (struct cmp_editable *)editable;
  size_t text_len;

  if (!internal_editable || !text)
    return CMP_ERROR_INVALID_ARG;

  text_len = strlen(text);
  if (text_len == 0)
    return CMP_SUCCESS;

  if (internal_editable->length + text_len >= internal_editable->capacity) {
    /* Reallocate */
    size_t new_capacity = internal_editable->capacity * 2;
    char *new_buffer = NULL;

    while (internal_editable->length + text_len >= new_capacity) {
      new_capacity *= 2;
    }

    /* Simulating realloc with malloc/copy/free */
    if (CMP_MALLOC(new_capacity, (void **)&new_buffer) != CMP_SUCCESS)
      return CMP_ERROR_OOM;

    memcpy(new_buffer, internal_editable->text_buffer,
           internal_editable->length + 1);
    CMP_FREE(internal_editable->text_buffer);
    internal_editable->text_buffer = new_buffer;
    internal_editable->capacity = new_capacity;
  }

  strcpy(internal_editable->text_buffer + internal_editable->length, text);
  internal_editable->length += text_len;

  return CMP_SUCCESS;
}
