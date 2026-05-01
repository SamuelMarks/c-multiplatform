/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

/**
 * @brief cmp_interop_mock_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_interop_mock_init(void) {
  int rc = CMP_SUCCESS;
  /* Placeholder for C to Swift ABI stability hooks */

  return rc;
}

/**
 * @brief cmp_interop_cfstring_bridge
 *
 * @param cf_string_ref Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
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

/**
 * @brief cmp_interop_allocate_retained_object
 *
 * @param out_object Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_interop_allocate_retained_object(void **out_object)
    CMP_RETURNS_RETAINED {
  int rc = CMP_SUCCESS;
  if (!out_object) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_interop_allocate_retained_object: Invalid argument\n");
    if (rc != CMP_SUCCESS) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(int), out_object);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_interop_allocate_retained_object: Out of memory\n");
    if (rc != CMP_SUCCESS) {
      return rc;
    }
    return rc;
  }

  /* ARC memory management mock: object is returned retained (+1) */
  if (rc != CMP_SUCCESS) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_interop_release_object
 *
 * @param object Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_interop_release_object(void *object) {
  int rc = CMP_SUCCESS;
  if (object) {
    rc = CMP_FREE(object);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_interop_release_object: CMP_FREE failed\n");
    }
  }
  /* CFRelease / ARC boundary mock */

  return rc;
}
