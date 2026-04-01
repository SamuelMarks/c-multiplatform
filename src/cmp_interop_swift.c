/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

int cmp_interop_mock_init(void) {
  /* Placeholder for C to Swift ABI stability hooks */
  return CMP_SUCCESS;
}
/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

int cmp_interop_cfstring_bridge(void *cf_string_ref) {
  if (!cf_string_ref)
    return CMP_ERROR_INVALID_ARG;
  /* Placeholder for CFStringRef -> NSString -> Swift String bridge validation
   */
  return CMP_SUCCESS;
}

int cmp_interop_allocate_retained_object(void **out_object)
    CMP_RETURNS_RETAINED {
  if (!out_object)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(int), out_object) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  /* ARC memory management mock: object is returned retained (+1) */
  return CMP_SUCCESS;
}

int cmp_interop_release_object(void *object) {
  if (object)
    CMP_FREE(object);
  /* CFRelease / ARC boundary mock */
  return CMP_SUCCESS;
}
