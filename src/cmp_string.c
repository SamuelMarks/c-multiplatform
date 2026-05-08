/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(_MSC_VER)
#define MEMCPY_S memcpy_s
#define STRCPY_S strcpy_s
#define STRTOK_S strtok_s
#else
#define MEMCPY_S(dest, dest_size, src, count) memcpy((dest), (src), (count))
#define STRCPY_S(dest, dest_size, src) strcpy((dest), (src))
#define STRTOK_S(str, delim, ctx) strtok_r((str), (delim), (ctx))
#endif

/* clang-format on */

/**
 * @brief Initializes a string object.
 *
 * @param str Pointer to the string object to initialize.
 * @return int Returns 0 on success, or an error code on failure.
 */
int cmp_string_init(cmp_string_t *str) {
  int rc = CMP_SUCCESS;
  if (str == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_string_init: str is NULL\n");
    return rc;
  }

  str->data = NULL;
  str->length = 0;
  str->capacity = 0;

  return rc;
}

/**
 * @brief Appends a C-string to the string object.
 *
 * @param str Pointer to the string object.
 * @param append The null-terminated C-string to append.
 * @return int Returns 0 on success, or an error code on failure.
 */
int cmp_string_append(cmp_string_t *str, const char *append) {
  int rc = CMP_SUCCESS;
  size_t append_len;
  size_t new_len;
  char *new_data;
  size_t new_capacity;

  if (str == NULL || append == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_string_append: str or append is NULL\n");
    return rc;
  }

  append_len = strlen(append);
  if (append_len == 0) {
    return rc;
  }

  /* Detect overflow on length */
  if (str->length > ((size_t)-1) - append_len) {
    rc = CMP_ERROR_BOUNDS;
    LOG_DEBUG("cmp_string_append: length overflow\n");
    return rc;
  }

  new_len = str->length + append_len;

  if (new_len + 1 > str->capacity) {
    new_capacity = str->capacity == 0 ? 32 : str->capacity * 2;
    /* Handle overflow during capacity doubling */
    if (new_capacity < str->capacity) {
      new_capacity = ((size_t)-1);
    }

    while (new_capacity < new_len + 1) {
      if (new_capacity > ((size_t)-1) / 2) {
        new_capacity = ((size_t)-1);
        if (new_capacity < new_len + 1) {
          rc = CMP_ERROR_BOUNDS;
          LOG_DEBUG("cmp_string_append: capacity bounds limit\n");
          return rc;
        }
        break;
      }
      new_capacity *= 2;
    }

    rc = CMP_MALLOC(new_capacity, (void **)&new_data);
    if (rc != CMP_SUCCESS || new_data == NULL) {
      rc = CMP_ERROR_OOM;
      LOG_DEBUG("cmp_string_append: CMP_MALLOC failed\n");
      return rc;
    }

    if (str->data != NULL) {
      MEMCPY_S(new_data, new_capacity, str->data, str->length + 1);
      rc = CMP_FREE(str->data);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_string_append: CMP_FREE failed\n");
        return rc;
      }
    }

    str->data = new_data;
    str->capacity = new_capacity;
  }

  STRCPY_S(str->data + str->length, str->capacity - str->length, append);
  str->length = new_len;

  return rc;
}

/**
 * @brief Destroys the string object and frees allocated memory.
 *
 * @param str Pointer to the string object to destroy.
 * @return int Returns 0 on success, or an error code on failure.
 */
int cmp_string_destroy(cmp_string_t *str) {
  int rc = CMP_SUCCESS;
  if (str == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_string_destroy: str is NULL\n");
    return rc;
  }

  if (str->data != NULL) {
    rc = CMP_FREE(str->data);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_string_destroy: CMP_FREE failed\n");
      return rc;
    }
    str->data = NULL;
  }

  str->length = 0;
  str->capacity = 0;

  return rc;
}

/**
 * @brief Thread-safe string tokenization.
 *
 * @param str The string to tokenize, or NULL to continue previous tokenization.
 * @param delim The delimiter characters.
 * @param saveptr Pointer used to maintain state between calls.
 * @param out_tok Pointer to a char* where the token pointer will be stored.
 * @return int Returns 0 on success, or an error code on failure. Returns
 * CMP_ERROR_NOT_FOUND when there are no more tokens.
 */
int cmp_strtok_r(char *str, const char *delim, char **saveptr, char **out_tok) {
  int rc = CMP_SUCCESS;
#if defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
  char *end;
#endif

  if (saveptr == NULL || out_tok == NULL || delim == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_strtok_r: null arguments provided\n");
    return rc;
  }

#if defined(_MSC_VER)
  if (str == NULL && (saveptr == NULL || *saveptr == NULL)) {
    *out_tok = NULL;
    rc = CMP_ERROR_NOT_FOUND;
    return rc;
  }
  *out_tok = strtok_s(str, delim, saveptr);
#elif defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
  if (str == NULL) {
    str = *saveptr;
  }
  if (str == NULL) {
    *out_tok = NULL;
    rc = CMP_ERROR_NOT_FOUND;
    return rc;
  }
  str += strspn(str, delim);
  if (*str == '\0') {
    *saveptr = NULL;
    *out_tok = NULL;
    rc = CMP_ERROR_NOT_FOUND;
    return rc;
  }
  end = str + strcspn(str, delim);
  if (*end == '\0') {
    *saveptr = NULL;
  } else {
    *end = '\0';
    *saveptr = end + 1;
  }
  *out_tok = str;
#else
  *out_tok = strtok_r(str, delim, saveptr);
#endif

  if (*out_tok == NULL) {
    rc = CMP_ERROR_NOT_FOUND;
    return rc;
  }

  return rc;
}