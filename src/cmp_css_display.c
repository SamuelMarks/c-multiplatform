/* clang-format off */
#include "cmp_css_display.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int cmp_strdup_safe(const char *src, char **dst) {
  size_t len;
  if (!dst)
    return -1;
  if (!src) {
    *dst = NULL;
    return 0;
  }
  len = strlen(src);
  *dst = (char *)malloc(len + 1);
  if (!*dst)
    return -1;
#if defined(_MSC_VER)
  strcpy_s(*dst, len + 1, src);
#else
  strcpy(*dst, src);
#endif
  return 0;
}

int cmp_prop_float_advanced_init(cmp_prop_float_advanced_t *adv, int defer,
                                 const char *reference) {
  int rc;
  if (!adv)
    return -1;
  adv->defer = defer;
  if (reference) {
    rc = cmp_strdup_safe(reference, &adv->reference);
    if (rc != 0)
      return rc;
  } else {
    adv->reference = NULL;
  }
  return 0;
}

int cmp_prop_float_advanced_free(cmp_prop_float_advanced_t *adv) {
  if (!adv)
    return -1;
  if (adv->reference) {
    free(adv->reference);
    adv->reference = NULL;
  }
  return 0;
}