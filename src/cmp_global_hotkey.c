/* clang-format off */
#include "cmp_global_hotkey.h"
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_global_hotkey {
  int next_id;
};

/**
 * @brief cmp_global_hotkey_create
 *
 * @param out_hotkey Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_global_hotkey_create(cmp_global_hotkey_t **out_hotkey) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_global_hotkey_t *hk = NULL;

  if (out_hotkey == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_global_hotkey_create: Invalid argument (out_hotkey=NULL): %s\n",
        err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_global_hotkey_t), (void **)&hk);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_global_hotkey_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  hk->next_id = 1;
  *out_hotkey = hk;
  cmp_log_debug(
      "cmp_global_hotkey_create: Successfully created global hotkey context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_global_hotkey_destroy
 *
 * @param hotkey Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_global_hotkey_destroy(cmp_global_hotkey_t *hotkey) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (hotkey == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_global_hotkey_destroy: Invalid argument: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_FREE(hotkey);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_global_hotkey_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug("cmp_global_hotkey_destroy: Successfully destroyed global "
                "hotkey context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_global_hotkey_register
 *
 * @param hotkey Parameter description.
 * @param key_code Parameter description.
 * @param modifiers Parameter description.
 * @param out_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_global_hotkey_register(cmp_global_hotkey_t *hotkey, int key_code,
                               int modifiers, int *out_id) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (hotkey == NULL || out_id == NULL || key_code <= 0 || modifiers < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_global_hotkey_register: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  *out_id = hotkey->next_id++;
  cmp_log_debug("cmp_global_hotkey_register: Registered hotkey id=%d\n",
                *out_id);
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
 * @brief cmp_global_hotkey_unregister
 *
 * @param hotkey Parameter description.
 * @param id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_global_hotkey_unregister(cmp_global_hotkey_t *hotkey, int id) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (hotkey == NULL || id <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_global_hotkey_unregister: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  cmp_log_debug("cmp_global_hotkey_unregister: Unregistered hotkey id=%d\n",
                id);
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
