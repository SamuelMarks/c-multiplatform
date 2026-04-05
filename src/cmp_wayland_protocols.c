/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_wayland_protocols {
  int fractional_scale_bound;
  int xdg_decoration_bound;
};

int cmp_wayland_protocols_create(cmp_wayland_protocols_t **out_protocols) {
  cmp_wayland_protocols_t *p;
  if (!out_protocols)
    return CMP_ERROR_INVALID_ARG;

  p = (cmp_wayland_protocols_t *)malloc(sizeof(cmp_wayland_protocols_t));
  if (!p)
    return CMP_ERROR_OOM;

  p->fractional_scale_bound = 0;
  p->xdg_decoration_bound = 0;
  *out_protocols = p;

  return CMP_SUCCESS;
}

int cmp_wayland_protocols_destroy(cmp_wayland_protocols_t *protocols) {
  if (!protocols)
    return CMP_ERROR_INVALID_ARG;
  free(protocols);
  return CMP_SUCCESS;
}

int cmp_wayland_protocols_bind(cmp_wayland_protocols_t *protocols,
                               cmp_window_t *window) {
  if (!protocols || !window)
    return CMP_ERROR_INVALID_ARG;

  /* Mock: In real application this uses wayland-client and parses globals. */
  protocols->fractional_scale_bound = 1;
  protocols->xdg_decoration_bound = 1;

  return CMP_SUCCESS;
}
