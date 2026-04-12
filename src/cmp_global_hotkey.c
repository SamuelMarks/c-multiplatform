/* clang-format off */
#include "cmp_global_hotkey.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_global_hotkey {
  int next_id;
};

int cmp_global_hotkey_create(cmp_global_hotkey_t **out_hotkey) {
  cmp_global_hotkey_t *hk;
  if (!out_hotkey) {
    return -1;
  }
  hk = (cmp_global_hotkey_t *)malloc(sizeof(cmp_global_hotkey_t));
  if (!hk) {
    return -2;
  }
  hk->next_id = 1;
  *out_hotkey = hk;
  return 0;
}

int cmp_global_hotkey_destroy(cmp_global_hotkey_t *hotkey) {
  if (!hotkey) {
    return -1;
  }
  free(hotkey);
  return 0;
}

int cmp_global_hotkey_register(cmp_global_hotkey_t *hotkey, int key_code,
                               int modifiers, int *out_id) {
  if (!hotkey || !out_id || key_code <= 0 || modifiers < 0) {
    return -1;
  }
  *out_id = hotkey->next_id++;
  return 0;
}

int cmp_global_hotkey_unregister(cmp_global_hotkey_t *hotkey, int id) {
  if (!hotkey || id <= 0) {
    return -1;
  }
  return 0;
}
