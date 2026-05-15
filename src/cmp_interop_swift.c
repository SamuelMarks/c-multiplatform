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

  /* Validate struct alignment for C to Swift ABI stability */
  size_t ptr_size = sizeof(void *);
  if (ptr_size != 4 && ptr_size != 8) {
    rc = CMP_ERROR_GENERAL;
    cmp_log_debug(
        "cmp_interop_mock_init: Unexpected pointer size for ABI %zu\n",
        ptr_size);
    return rc;
  }

  cmp_log_debug(
      "cmp_interop_mock_init: Verified architecture ABI layout sizes\n");

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

  /* Verify alignment formatting, typical core foundation objects are 8/16 byte
   * aligned */
  if ((size_t)cf_string_ref % sizeof(void *) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    cmp_log_debug(
        "cmp_interop_cfstring_bridge: Unaligned CFStringRef detected\n");
    return rc;
  }

  cmp_log_debug("cmp_interop_cfstring_bridge: Bridged valid swift pointer\n");

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
    return rc;
  }

  /* ARC memory management mock: Allocate space large enough to hold a ref count
     and the actual object pointer */
  rc = CMP_MALLOC(sizeof(void *) * 2, out_object);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_interop_allocate_retained_object: Out of memory\n");
    return rc;
  }

  /* Set mock retain count to 1 */
  ((size_t *)(*out_object))[0] = 1;

  cmp_log_debug(
      "cmp_interop_allocate_retained_object: Mocked +1 retained object\n");

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
    /* Verify mock ARC ref count */
    size_t ref_count = ((size_t *)object)[0];
    if (ref_count == 0) {
      /* Underflow! */
      cmp_log_debug("cmp_interop_release_object: Double release detected!\n");
      return CMP_ERROR_GENERAL;
    }

    ((size_t *)object)[0]--;

    if (((size_t *)object)[0] == 0) {
      rc = CMP_FREE(object);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Error in cmp_interop_release_object: CMP_FREE failed\n");
      }
    }
  }

  return rc;
}
