/* clang-format off */
#include "cmp.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

int cmp_string_init(cmp_string_t *str) {
  if (str == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  str->data = NULL;
  str->length = 0;
  str->capacity = 0;

  return CMP_SUCCESS;
}

int cmp_string_append(cmp_string_t *str, const char *append) {
  size_t append_len;
  size_t new_len;
  char *new_data;

  size_t new_capacity;

  if (str == NULL || append == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  append_len = strlen(append);
  if (append_len == 0) {
    return CMP_SUCCESS;
  }

  new_len = str->length + append_len;

  if (new_len + 1 > str->capacity) {
    new_capacity = str->capacity == 0 ? 32 : str->capacity * 2;
    while (new_capacity < new_len + 1) {
      new_capacity *= 2;
    }

    if (str->data == NULL) {
      if (CMP_MALLOC(new_capacity, (void **)&new_data) != CMP_SUCCESS) {
        return CMP_ERROR_OOM;
      }
    } else {
      if (CMP_MALLOC(new_capacity, (void **)&new_data) != CMP_SUCCESS) {
        return CMP_ERROR_OOM;
      }
#if defined(_MSC_VER)
      memcpy_s(new_data, new_capacity, str->data, str->length + 1);
#else
      memcpy(new_data, str->data, str->length + 1);
#endif
      CMP_FREE(str->data);
    }

    str->data = new_data;
    str->capacity = new_capacity;
  }

#if defined(_MSC_VER)
  strcpy_s(str->data + str->length, str->capacity - str->length, append);
#else
  strcpy(str->data + str->length, append);
#endif

  str->length = new_len;

  return CMP_SUCCESS;
}

int cmp_string_destroy(cmp_string_t *str) {
  if (str == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (str->data != NULL) {
    CMP_FREE(str->data);
    str->data = NULL;
  }

  str->length = 0;
  str->capacity = 0;

  return CMP_SUCCESS;
}