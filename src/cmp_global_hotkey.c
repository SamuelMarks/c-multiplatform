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
  int rc = CMP_SUCCESS;
  cmp_global_hotkey_t *hk = NULL;

  if (!out_hotkey) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_global_hotkey_create: Invalid argument "
              "(out_hotkey=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_global_hotkey_t), (void **)&hk);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_global_hotkey_create: Out of memory\n");
    return rc;
  }

  hk->next_id = 1;
  *out_hotkey = hk;
  return rc;
}

/**
 * @brief cmp_global_hotkey_destroy
 *
 * @param hotkey Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_global_hotkey_destroy(cmp_global_hotkey_t *hotkey) {
  int rc = CMP_SUCCESS;

  if (!hotkey) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_global_hotkey_destroy: Invalid argument (hotkey=NULL)\n");
    return rc;
  }

  rc = CMP_FREE(hotkey);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_global_hotkey_destroy: CMP_FREE failed\n");
    return rc;
  }

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
  int rc = CMP_SUCCESS;

  if (!hotkey || !out_id || key_code <= 0 || modifiers < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_global_hotkey_register: Invalid argument\n");
    return rc;
  }

  *out_id = hotkey->next_id++;
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
  int rc = CMP_SUCCESS;

  if (!hotkey || id <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_global_hotkey_unregister: Invalid argument\n");
    return rc;
  }

  return rc;
}