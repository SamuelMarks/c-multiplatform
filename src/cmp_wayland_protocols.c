/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_wayland_protocols {
  int fractional_scale_bound;
  int xdg_decoration_bound;
};

/**
 * @brief cmp_wayland_protocols_create
 *
 * @param out_protocols Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_wayland_protocols_create(cmp_wayland_protocols_t **out_protocols) {
  cmp_wayland_protocols_t *p = NULL;
  int rc = CMP_SUCCESS;

  if (!out_protocols) {
    LOG_DEBUG("cmp_wayland_protocols_create: out_protocols is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_wayland_protocols_t), (void **)&p);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_wayland_protocols_create: OOM\n");

    return rc;
  }

  p->fractional_scale_bound = 0;
  p->xdg_decoration_bound = 0;
  *out_protocols = p;

  return rc;
}

/**
 * @brief cmp_wayland_protocols_destroy
 *
 * @param protocols Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_wayland_protocols_destroy(cmp_wayland_protocols_t *protocols) {
  int rc = CMP_SUCCESS;

  if (!protocols) {
    LOG_DEBUG("cmp_wayland_protocols_destroy: protocols is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(protocols);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_wayland_protocols_destroy: CMP_FREE failed\n");

    return rc;
  }
  return rc;
}

/**
 * @brief cmp_wayland_protocols_bind
 *
 * @param protocols Parameter description.
 * @param window Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_wayland_protocols_bind(cmp_wayland_protocols_t *protocols,
                               cmp_window_t *window) {
  int rc = CMP_SUCCESS;
  if (!protocols || !window) {
    LOG_DEBUG("cmp_wayland_protocols_bind: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Mock: In real application this uses wayland-client and parses globals. */
  protocols->fractional_scale_bound = 1;
  protocols->xdg_decoration_bound = 1;

  return rc;
}