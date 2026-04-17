/* clang-format off */
#include "cmp_global_hotkey.h"
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_global_hotkey {
  int next_id;
};

int cmp_global_hotkey_create(cmp_global_hotkey_t **out_hotkey) {
  int rc = CMP_SUCCESS;
  cmp_global_hotkey_t *hk = NULL;

  if (!out_hotkey) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_global_hotkey_create: Invalid argument "
              "(out_hotkey=NULL)\n");
    return rc;
  }

  hk = (cmp_global_hotkey_t *)malloc(sizeof(cmp_global_hotkey_t));
  if (!hk) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_global_hotkey_create: Out of memory\n");
    return rc;
  }

  hk->next_id = 1;
  *out_hotkey = hk;
  return rc;
}

int cmp_global_hotkey_destroy(cmp_global_hotkey_t *hotkey) {
  int rc = CMP_SUCCESS;

  if (!hotkey) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_global_hotkey_destroy: Invalid argument (hotkey=NULL)\n");
    return rc;
  }

  free(hotkey);
  return rc;
}

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

int cmp_global_hotkey_unregister(cmp_global_hotkey_t *hotkey, int id) {
  int rc = CMP_SUCCESS;

  if (!hotkey || id <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_global_hotkey_unregister: Invalid argument\n");
    return rc;
  }

  return rc;
}
