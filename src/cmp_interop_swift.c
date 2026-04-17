/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

int cmp_interop_mock_init(void) {
  int rc = CMP_SUCCESS;
  /* Placeholder for C to Swift ABI stability hooks */
  return rc;
}

int cmp_interop_cfstring_bridge(void *cf_string_ref) {
  int rc = CMP_SUCCESS;
  if (!cf_string_ref) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_interop_cfstring_bridge: Invalid argument\n");
    return rc;
  }
  /* Placeholder for CFStringRef -> NSString -> Swift String bridge validation
   */
  return rc;
}

int cmp_interop_allocate_retained_object(void **out_object)
    CMP_RETURNS_RETAINED {
  int rc = CMP_SUCCESS;

  if (!out_object) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_interop_allocate_retained_object: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(int), out_object);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_interop_allocate_retained_object: Out of memory\n");
    return rc;
  }

  /* ARC memory management mock: object is returned retained (+1) */
  return rc;
}

int cmp_interop_release_object(void *object) {
  int rc = CMP_SUCCESS;
  if (object) {
    CMP_FREE(object);
  }
  /* CFRelease / ARC boundary mock */
  return rc;
}
